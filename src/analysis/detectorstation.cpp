#include "analysis/detectorstation.h"
#include "messages/event.h"
#include "supervision/state.h"
#include "utility/log.h"
#include "utility/utility.h"

#include "supervision/station.h"

namespace muonpi {

constexpr double LIGHTSPEED { 0.299 }; //< velocity of light in m/ns
constexpr double MAX_TIMING_ERROR { 1000. }; //< max allowable timing error in nanoseconds
constexpr double MAX_LOCATION_ERROR { MAX_TIMING_ERROR * LIGHTSPEED }; //< max allowable location error in meter
constexpr double stddev_factor { 0.75 };

void detector_station::enable()
{
    set_status(Status::Created);
}

detector_station::detector_station(const detector_info_t<location_t>& initial_log, supervision::station& stationsupervisor)
    : m_location { initial_log.get<location_t>() }
    , m_hash { initial_log.hash }
    , m_userinfo { initial_log.userinfo }
    , m_stationsupervisor { stationsupervisor }
{
}

detector_station::detector_station(const std::string& serialised, supervision::station& stationsupervisor, bool stale)
    : m_stationsupervisor { stationsupervisor }
{
    message_parser in { serialised, ' ' };
    if (in.size() < 10) {
        m_status = Status::Deleted;
        return;
    }
    m_hash = std::stoul(in[0], nullptr);
    m_userinfo.username = in[1];
    m_userinfo.station_id = in[2];
    if (in[3] == "created") {
        m_status = Status::Created;
    } else if (in[3] == "deleted") {
        m_status = Status::Deleted;
    } else if ((in[3] != "reliable") || (stale)) {
        m_status = Status::Unreliable;
    } else {
        m_status = Status::Reliable;
    }

    m_location.lat = std::stod(in[4], nullptr);
    m_location.lon = std::stod(in[5], nullptr);
    m_location.h = std::stod(in[6], nullptr);
    m_location.h_acc = std::stod(in[7], nullptr);
    m_location.v_acc = std::stod(in[8], nullptr);
    m_location.dop = std::stod(in[9], nullptr);
}

auto detector_station::serialise() const -> std::string
{
    std::ostringstream out {};
    out << m_hash << ' ' << m_userinfo.username << ' ' << m_userinfo.station_id << ' ';
    switch (m_status) {
    case Status::Created:
        out << "created";
        break;
    case Status::Deleted:
        out << "deleted";
        break;
    case Status::Reliable:
        out << "reliable";
        break;
    case Status::Unreliable:
        out << "unreliable";
        break;
    }
    out << ' ' << m_location.lat << ' ' << m_location.lon << ' ' << m_location.h << ' ' << m_location.h_acc << ' ' << m_location.v_acc << ' ' << m_location.dop;

    return out.str();
}

auto detector_station::process(const event_t& event) -> bool
{
    m_current_rate.increase_counter();
    m_mean_rate.increase_counter();
    m_current_data.incoming++;

    const std::uint16_t current_ublox_counter = event.ublox_counter;
    if (!m_initial) {
        std::uint16_t difference { static_cast<uint16_t>(current_ublox_counter - m_last_ublox_counter) };

        if (current_ublox_counter <= m_last_ublox_counter) {
            difference = current_ublox_counter + (std::numeric_limits<std::uint16_t>::max() - m_last_ublox_counter);
        }
        m_current_data.ublox_counter_progress += difference;
    } else {
        m_initial = false;
    }
    m_last_ublox_counter = current_ublox_counter;

    double pulselength { static_cast<double>(event.end - event.start) };
    if ((pulselength > 0.) && (pulselength < 1e6)) {
        m_pulselength.add(pulselength);
    }
    m_time_acc.add(event.time_acc);
    m_reliability_time_acc.add(event.time_acc);

    if (event.time_acc > (MAX_TIMING_ERROR * 100)) {
        set_status(Status::Unreliable);
    }

    return (event.time_acc <= MAX_TIMING_ERROR) && (event.fix == 1);
}

void detector_station::process(const detector_info_t<location_t>& info)
{
    m_last_log = std::chrono::system_clock::now();
    m_location = info.get<location_t>();
    check_reliability();
}

void detector_station::set_status(Status status)
{
    if (m_status != status) {
        m_stationsupervisor.detector_status(m_hash, status);
    }
    m_status = status;
}

auto detector_station::is(Status status) const -> bool
{
    return m_status == status;
}

auto detector_station::factor() const -> double
{
    return m_factor;
}

void detector_station::check_reliability()
{
    const double loc_precision { m_location.dop * std::sqrt((m_location.h_acc * m_location.h_acc + m_location.v_acc * m_location.v_acc)) };
    if ((loc_precision > MAX_LOCATION_ERROR) || (m_reliability_time_acc.mean() > MAX_TIMING_ERROR) || (m_mean_rate.stddev() > (m_mean_rate.mean() * stddev_factor))) {
        set_status(Status::Unreliable);
    } else {
        set_status(Status::Reliable);
    }
}

void detector_station::step(const std::chrono::system_clock::time_point& now)
{
    auto diff { now - std::chrono::system_clock::time_point { m_last_log } };
    if (diff > s_log_interval) {
        if (diff > s_quit_interval) {
            set_status(Status::Deleted);
            return;
        }
        set_status(Status::Unreliable);

    } else {
        check_reliability();
    }

    if (m_current_rate.step(now)) {
        m_mean_rate.step(now);
        if (m_current_rate.mean() < (m_mean_rate.mean() - m_mean_rate.stddev())) {
            m_factor = ((m_mean_rate.mean() - m_current_rate.mean()) / (m_mean_rate.stddev()) + 1.0) * 2.0;
        } else {
            m_factor = 1.0;
        }
    }
}

auto detector_station::current_log_data() -> detector_summary_t
{
    m_current_data.mean_eventrate = m_current_rate.mean();
    m_current_data.stddev_eventrate = m_current_rate.stddev();
    m_current_data.mean_pulselength = m_pulselength.mean();
    m_current_data.mean_time_acc = m_time_acc.mean();

    if (m_current_data.ublox_counter_progress == 0) {
        m_current_data.deadtime = 1.;
    } else {
        m_current_data.deadtime = 1. - static_cast<double>(m_current_data.incoming) / static_cast<double>(m_current_data.ublox_counter_progress);
    }
    detector_summary_t log { m_current_data };
    log.hash = m_hash;
    log.userinfo = m_userinfo;
    m_current_data.incoming = 0;
    m_current_data.ublox_counter_progress = 0;
    return log;
}

auto detector_station::change_log_data() -> detector_summary_t
{
    auto summary { current_log_data() };
    summary.change = 1;
    return summary;
}

auto detector_station::user_info() const -> userinfo_t
{
    return m_userinfo;
}

auto detector_station::location() const -> location_t
{
    return m_location;
}

}
