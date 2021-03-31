#include "triggerhandler.h"

#include "defaults.h"
#include "utility/log.h"
#include "utility/utility.h"

#include <sstream>

#include <regex>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ldap.h>
#include <sasl/sasl.h>
#include <utility>

namespace muonpi {

namespace Trigger {

    auto Detector::Setting::to_string(char delimiter) const -> std::string
    {
        std::ostringstream stream;
        if (delimiter == 0) {
            stream << username << station;
        } else {
            stream << username << delimiter << station << delimiter;
        }
        switch (type) {
        case Trigger::Detector::Setting::Offline:
            stream << "offline";
            break;
        case Trigger::Detector::Setting::Online:
            stream << "online";
            break;
        case Trigger::Detector::Setting::Unreliable:
            stream << "unreliable";
            break;
        case Trigger::Detector::Setting::Reliable:
            stream << "reliable";
            break;
        case Trigger::Detector::Setting::Invalid:
            stream << "invalid";
            break;
        }
        return stream.str();
    }

    auto Detector::Setting::id() const -> std::size_t
    {
        return std::hash<std::string> {}(to_string());
    }

    auto Detector::Setting::from_string(const std::string& string) -> Setting
    {
        MessageParser parser { string, ' ' };

        if (parser.size() != 3) {
            return Setting {};
        }

        Setting trigger;
        trigger.username = parser[0];
        trigger.station = parser[1];
        if (parser[2] == "offline") {
            trigger.type = Trigger::Detector::Setting::Offline;
        } else if (parser[2] == "online") {
            trigger.type = Trigger::Detector::Setting::Online;
        } else if (parser[2] == "unreliable") {
            trigger.type = Trigger::Detector::Setting::Unreliable;
        } else if (parser[2] == "reliable") {
            trigger.type = Trigger::Detector::Setting::Reliable;
        } else {
            return Setting {};
        }
        return trigger;
    }

}

namespace Ldap {

    auto my_sasl_interact(LDAP* ld, unsigned flags, void* defaults, void* in) -> int;

    struct authdata {
        const char* username;
        const char* authname;
        const char* password;
    };

    auto my_sasl_interact(LDAP* ld, unsigned /*flags*/, void* defaults, void* in) -> int
    {
        auto* auth = static_cast<authdata*>(defaults);

        auto* interact = static_cast<sasl_interact_t*>(in);
        if (ld == nullptr) {
            return LDAP_PARAM_ERROR;
        }

        while (interact->id != SASL_CB_LIST_END) {

            const char* dflt = interact->defresult;

            switch (interact->id) {
            case SASL_CB_GETREALM:
                dflt = nullptr;
                break;
            case SASL_CB_USER:
                dflt = auth->username;
                break;
            case SASL_CB_AUTHNAME:
                dflt = auth->authname;
                break;
            case SASL_CB_PASS:
                dflt = auth->password;
                break;
            default:
                muonpi::Log::warning() << "unknown ldap parameter" + std::to_string(interact->id);
            }
            interact->result = ((dflt != nullptr) && (*dflt != 0)) ? dflt : "";
            interact->len = strlen(static_cast<char*>(const_cast<void*>(interact->result)));

            interact++;
        }
        return LDAP_SUCCESS;
    }
}

TriggerHandler::TriggerHandler(Sink::Base<Trigger::Detector::Action>& sink, Config::Ldap ldap_config, Config::Trigger trigger_config)
    : Source::Base<Trigger::Detector::Action> { sink }
    , m_ldap { std::move(ldap_config) }
    , m_trigger { std::move(trigger_config) }
{
    load();

    rest::handler handler {};
    handler.matches = [](std::string_view path) { return path == "trigger"; };
    handler.authenticate = [this](rest::request /*request*/, std::string_view username, std::string_view password) { return authenticate(username, password); };
    handler.handle = [this](rest::request request, std::queue<std::string> /*path*/) { return handle(std::move(request)); };
    handler.requires_auth = true;

    set_handler(std::move(handler));
}

TriggerHandler::~TriggerHandler()
{
    save();
}

auto TriggerHandler::authenticate(std::string_view user, std::string_view pw) -> bool
{
    LDAP* ldap { nullptr };
    auto code = ldap_initialize(&ldap, m_ldap.server.c_str());
    if (code != LDAP_SUCCESS) {
        Log::warning() << "Could not connect to ldap: " + std::string { ldap_err2string(code) };
        return false;
    }

    const int protocol { LDAP_VERSION3 };

    if (ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &protocol) != LDAP_OPT_SUCCESS) {
        Log::warning() << "Could not set ldap options.";
        return false;
    }
    {

        berval credentials {};
        credentials.bv_len = m_ldap.login.password.size();
        credentials.bv_val = const_cast<char*>(m_ldap.login.password.c_str());

        code = ldap_sasl_bind_s(ldap, m_ldap.login.bind_dn.c_str(),
            nullptr, &credentials, nullptr,
            nullptr, nullptr);

        if (code != LDAP_SUCCESS) {
            Log::warning() << "Could not bind to ldap: " + std::string { ldap_err2string(code) } + " " + std::to_string(code);
            ldap_unbind_ext_s(ldap, nullptr, nullptr);
            return false;
        }
    }

    LDAPMessage* result = nullptr;
    code = ldap_search_ext_s(ldap, "ou=users,dc=muonpi,dc=org", LDAP_SCOPE_ONELEVEL, ("(&(objectClass=inetOrgPerson)(memberof=cn=trigger,ou=groups,dc=muonpi,dc=org)(uid=" + std::string { user } + "))").c_str(), nullptr, 0, nullptr, nullptr, nullptr, LDAP_NO_LIMIT, &result);

    if (code != LDAP_SUCCESS) {
        Log::warning() << "Could not search in ldap: " + std::string { ldap_err2string(code) };
        return false;
    }

    if (ldap_count_entries(ldap, result) < 1) {
        Log::warning() << "No search results.";
        return false;
    }

    std::string bind_dn { "uid=" + std::string { user } + ",ou=users,dc=muonpi,dc=org" };

    berval credentials {};
    credentials.bv_len = pw.size();
    credentials.bv_val = const_cast<char*>(std::string { pw }.c_str());

    code = ldap_sasl_bind_s(ldap, bind_dn.c_str(),
        nullptr, &credentials, nullptr,
        nullptr, nullptr);

    if (code != LDAP_SUCCESS) {
        Log::warning() << "Could not bind to ldap: " + std::string { ldap_err2string(code) } + " " + std::to_string(code);
        ldap_unbind_ext_s(ldap, nullptr, nullptr);
        return false;
    }

    ldap_unbind_ext_s(ldap, nullptr, nullptr);
    sasl_done();
    sasl_client_init(nullptr);
    return true;
}

auto TriggerHandler::handle(rest::request request) -> rest::response_type
{
    std::string body { request.req.body() };

    if (request.req.method() == rest::http::verb::get) {
        MessageParser parser { body, ' ' };

        std::ostringstream stream {};
        std::size_t n { 0 };
        if (parser.size() == 1) {
            for (auto& [hash, trigger] : m_detector_trigger) {
                if (trigger.username == parser[0]) {
                    n++;
                    stream << trigger.to_string(' ') << '\n';
                }
            }
        } else if (parser.size() == 2) {
            for (auto& [hash, trigger] : m_detector_trigger) {
                if ((trigger.username == parser[0]) && (trigger.station == parser[1])) {
                    n++;
                    stream << trigger.to_string(' ') << '\n';
                }
            }
        } else if (parser.size() == 3) {
            std::size_t hash = std::hash<std::string> {}(parser[0] + parser[1] + parser[2]);

            if (m_detector_trigger.find(hash) == m_detector_trigger.end()) {
                return request.response<rest::http::status::not_found>("No trigger found with search term");
            }
            return request.response<rest::http::status::ok>(body);
        }
        if (n == 0) {
            return request.response<rest::http::status::not_found>("No trigger found with search term");
        }

        return request.response<rest::http::status::ok>(stream.str());

    } else if (request.req.method() == rest::http::verb::post) {
        auto trigger { Trigger::Detector::Setting::from_string(body) };

        if (trigger.type == Trigger::Detector::Setting::Invalid) {
            return request.response<rest::http::status::bad_request>("Invalid trigger");
        }

        std::size_t hash = trigger.id();

        if (m_detector_trigger.find(hash) != m_detector_trigger.end()) {
            return request.response<rest::http::status::already_reported>("Trigger already set");
        }

        m_detector_trigger[hash] = trigger;
        put(Trigger::Detector::Action { Trigger::Detector::Action::Activate, trigger });

        Log::debug() << "Setting up new trigger: '" + trigger.to_string() + "'";
        save();

        return request.response<rest::http::status::created>("Trigger created");
    } else if (request.req.method() == rest::http::verb::delete_) {
        auto trigger { Trigger::Detector::Setting::from_string(body) };

        if (trigger.type == Trigger::Detector::Setting::Invalid) {
            return request.response<rest::http::status::bad_request>("Invalid trigger");
        }

        std::size_t hash = trigger.id();

        if (m_detector_trigger.find(hash) == m_detector_trigger.end()) {
            return request.response<rest::http::status::not_found>("No trigger found with search term");
        }

        m_detector_trigger.erase(hash);

        put(Trigger::Detector::Action { Trigger::Detector::Action::Deactivate, trigger });

        Log::debug() << "Removing trigger: '" + body + "'";
        save();
        return request.response<rest::http::status::ok>("");
    }
    return request.response<rest::http::status::not_implemented>("Method not implemented");
}

void TriggerHandler::save()
{
    std::ofstream out { m_trigger.save_file };
    if (!out.is_open()) {
        Log::warning() << "Could not save trigger.";
        return;
    }
    Log::info() << "Saving trigger.";
    for (auto& [hash, trigger] : m_detector_trigger) {
        out << trigger.to_string(' ') << '\n';
    }
    out.close();
}

void TriggerHandler::load()
{
    std::ifstream in { m_trigger.save_file };
    if (!in.is_open()) {
        Log::warning() << "Could not load trigger.";
        return;
    }
    Log::info() << "Loading trigger.";
    for (std::string line; std::getline(in, line);) {

        auto trigger { Trigger::Detector::Setting::from_string(line) };

        if (trigger.type == Trigger::Detector::Setting::Invalid) {
            continue;
        }

        const std::size_t hash = trigger.id();

        if (m_detector_trigger.find(hash) != m_detector_trigger.end()) {
            continue;
        }

        m_detector_trigger[hash] = trigger;

        put(Trigger::Detector::Action { Trigger::Detector::Action::Activate, trigger });
    }
}

}
