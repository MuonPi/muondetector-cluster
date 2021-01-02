#ifndef TRIGGERHANDLER_H
#define TRIGGERHANDLER_H

#include "sink/base.h"
#include "source/base.h"

#include "messages/trigger.h"

#include <restbed>
#include <future>

namespace restbed {
typedef std::shared_ptr<Session> session_ptr;
typedef std::function<void(const restbed::session_ptr)> callback;
}

namespace MuonPi {


class TriggerHandler : public Sink::Base<DetectorTrigger>, public Source::Base<DetectorTrigger>
{
public:
    TriggerHandler(Sink::Base<DetectorTrigger>& sink);

    ~TriggerHandler() override;

    void get(DetectorTrigger trigger) override;

private:
    void save();
    void load();

    void handle_authentication(const restbed::session_ptr session, const restbed::callback& callback);

    [[nodiscard]] auto authenticate(const std::string& user, const std::string& pw) -> bool;

    void handle_post(const restbed::session_ptr session);
    void handle_get(const restbed::session_ptr session);
    void handle_delete(const restbed::session_ptr session);


    std::shared_ptr<restbed::Resource> m_resource { std::make_shared<restbed::Resource>()};
    std::shared_ptr<restbed::SSLSettings> m_ssl_settings { std::make_shared<restbed::SSLSettings>()};
    std::shared_ptr<restbed::Settings> m_settings { std::make_shared<restbed::Settings>()};

    restbed::Service m_service {};

    std::map<std::size_t, DetectorTrigger> m_detector_trigger {};

    std::future<void> m_future;
};

}

#endif // TRIGGERHANDLER_H