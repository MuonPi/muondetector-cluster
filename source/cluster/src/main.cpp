#include "log.h"
#include "core.h"
#include "detectortracker.h"

#include "mqttsource.h"
#include "mqttlink.h"
#include "mqttsink.h"
#include "statesupervisor.h"

#define CLUSTER_RUN_SERVER

#ifdef CLUSTER_RUN_SERVER
#include "databasesink.h"
#include "databaselink.h"
#else
#endif

#include "asciieventsink.h"
#include "asciilogsink.h"

#include "detectorsummary.h"
#include "clusterlog.h"

#include <csignal>
#include <functional>

void signal_handler(int signal);


static std::function<void(int)> shutdown_handler;

void signal_handler(int signal)
{
    shutdown_handler(signal);
}

auto main() -> int
{
    MuonPi::Log::Log::singleton()->add_sink(std::make_shared<MuonPi::Log::StreamSink>(std::cerr));
    MuonPi::Log::Log::singleton()->add_sink(std::make_shared<MuonPi::Log::SyslogSink>());


    MuonPi::MqttLink::LoginData login;
	login.username = "";
    login.password = "";
    login.station_id = "";
    MuonPi::MqttLink mqtt_link {login, "116.202.96.181", 1883};

    if (!mqtt_link.wait_for(MuonPi::MqttLink::Status::Connected)) {
        return -1;
    }

    auto event_source { std::make_shared<MuonPi::MqttSource<MuonPi::Event>>(mqtt_link.subscribe("muonpi/data/#")) };
    auto log_source { std::make_shared<MuonPi::MqttSource<MuonPi::DetectorInfo>>(mqtt_link.subscribe("muonpi/log/#")) };

    auto ascii_clusterlog_sink { std::make_shared<MuonPi::AsciiLogSink<MuonPi::ClusterLog>>(std::cout) };
    auto ascii_detectorlog_sink { std::make_shared<MuonPi::AsciiLogSink<MuonPi::DetectorSummary>>(std::cout) };
    auto ascii_event_sink { std::make_shared<MuonPi::AsciiEventSink>(std::cout) };

/*
	MuonPi::MqttLink sink_link {login, "116.202.96.181", 1883};

    if (!sink_link.wait_for(MuonPi::MqttLink::Status::Connected)) {
        return -1;
    }
*/
#ifdef CLUSTER_RUN_SERVER
    MuonPi::DatabaseLink db_link {"", {"", ""}, ""};

    auto event_sink { std::make_shared<MuonPi::DatabaseSink<MuonPi::Event>>(db_link) };
    auto clusterlog_sink { std::make_shared<MuonPi::DatabaseSink<MuonPi::ClusterLog>>(db_link) };
    auto detectorlog_sink { std::make_shared<MuonPi::DatabaseSink<MuonPi::DetectorSummary>>(db_link) };

#else

    auto event_sink { std::make_shared<MuonPi::MqttSink<MuonPi::Event>>(mqtt_link.publish("muonpi/l1data")) };
    auto clusterlog_sink { std::make_shared<MuonPi::MqttSink<MuonPi::ClusterLog>>(mqtt_link.publish("muonpi/cluster")) };
    auto detectorlog_sink { std::make_shared<MuonPi::MqttSink<MuonPi::DetectorSummary>>(mqtt_link.publish("muonpi/cluster")) };

#endif

    auto mqtt_broadcast_sink { std::make_shared<MuonPi::MqttSink<MuonPi::Event>>(mqtt_link.publish("muonpi/events")) };

	MuonPi::StateSupervisor supervisor{{ascii_clusterlog_sink, clusterlog_sink}};
    MuonPi::DetectorTracker detector_tracker{{log_source}, {ascii_detectorlog_sink, detectorlog_sink}, supervisor};
    MuonPi::Core core{{ascii_event_sink, event_sink, mqtt_broadcast_sink}, {event_source}, detector_tracker, supervisor};

#ifndef CLUSTER_RUN_SERVER
//    supervisor.add_thread(&mqtt_link);
#endif
    supervisor.add_thread(&detector_tracker);
    supervisor.add_thread(&mqtt_link);
    supervisor.add_thread(detectorlog_sink.get());
    supervisor.add_thread(log_source.get());
    supervisor.add_thread(event_source.get());
    supervisor.add_thread(ascii_event_sink.get());
    supervisor.add_thread(ascii_clusterlog_sink.get());
    supervisor.add_thread(ascii_detectorlog_sink.get());


    shutdown_handler = [&](int signal) {
        if (
                   (signal == SIGINT)
                || (signal == SIGTERM)
                || (signal == SIGHUP)
                ) {
            MuonPi::Log::notice()<<"Received signal: " + std::to_string(signal) + ". Exiting.";
            supervisor.stop();
        }
        if (
                   signal == SIGSEGV
                ) {
            MuonPi::Log::critical()<<"Received signal: " + std::to_string(signal) + ". Exiting.";
            supervisor.stop();
            core.stop();
        }
    };

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGHUP, signal_handler);
    std::signal(SIGSEGV, signal_handler);

    core.start_synchronuos();

    return core.wait();
}
