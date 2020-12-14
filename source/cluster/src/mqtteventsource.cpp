#include "mqtteventsource.h"

#include "event.h"
#include "utility.h"
#include "log.h"

namespace MuonPi {

MqttEventSource::MqttEventSource(Subscribers subscribers)
    : AbstractSource<Event>{}
    , m_link { std::move(subscribers) }
{
    start();
}

MqttEventSource::~MqttEventSource() = default;


auto MqttEventSource::pre_run() -> int
{
    if ((m_link.single == nullptr) || (m_link.combined == nullptr)) {
        return -1;
    }
    return 0;
}

auto MqttEventSource::step() -> int
{
    if (m_link.single->has_message()) {
        MqttLink::Message msg = m_link.single->get_message();
        MessageParser topic { msg.topic, '/'};
        MessageParser content { msg.content, ' '};
        if ((topic.size() == 4) && (content.size() >= 2)) {
//            Log::info()<<"Got data from " + topic[2] + topic[3];
            std::size_t hash {std::hash<std::string>{}(topic[2] + topic[3])};

            std::string ts_string = content[0];
            std::uint64_t ts = static_cast<std::uint64_t>(std::stod(ts_string, nullptr) * 1.0e9);

            std::chrono::system_clock::time_point start = std::chrono::system_clock::time_point(std::chrono::nanoseconds(ts));

            ts_string = content[1];
//            ts = std::stoull(ts_string, nullptr);
            ts = static_cast<std::uint64_t>(std::stod(ts_string, nullptr) * 1.0e9);

            std::chrono::system_clock::time_point end = std::chrono::system_clock::time_point(std::chrono::nanoseconds(ts));

            std::uint64_t id {hash & 0xFFFFFFFF00000000 + 0x00000000FFFFFFFF & ts};

            push_item(std::make_unique<Event>(Event(hash,id,start,end)));
        }
    }
    if (m_link.combined->has_message()) {
        MqttLink::Message msg = m_link.combined->get_message();
        MessageParser topic { msg.topic, '/'};
        MessageParser content { msg.content, ' '};
//        std::unique_ptr<Event> event { nullptr };
        // todo: parsing of message
//        push_item(std::move(event));
    }
    std::this_thread::sleep_for(std::chrono::microseconds{50});
    return 0;
}

}
