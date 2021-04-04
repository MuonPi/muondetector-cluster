﻿#include "analysis/stationcoincidence.h"

#include "supervision/station.h"

#include "utility/coordinatemodel.h"
#include "utility/utility.h"

#include <algorithm>
#include <fstream>

namespace muonpi {


station_coincidence::station_coincidence(std::string data_directory, supervision::station& stationsupervisor)
    : thread_runner {"station_coincidence"}
    , m_stationsupervisor { stationsupervisor }
    , m_data_directory { std::move(data_directory) }
{
    start();
    reset();
}

auto station_coincidence::step() -> int
{
    std::this_thread::sleep_for(s_sample_time);
    save();
    return 0;
}

void station_coincidence::get(event_t event)
{
    if (event.n() == 0) {
        return;
    }

    for (std::size_t i { 0 }; i < (event.n() - 1); i++) {
        const std::size_t first_h { event.events.at(i).hash };
        auto condition_1 = [&](const auto& iterator){return iterator.first.hash() == first_h;};
        auto it_1 { std::find_if(m_stations.begin(), m_stations.end(), condition_1) };
        if (it_1 == m_stations.end()) {
            const auto& [userinfo, location] {m_stationsupervisor.get_station(first_h)};
            add_station(userinfo, location);
        }
        const std::size_t first { static_cast<std::size_t>(std::distance(m_stations.begin(), it_1)) };
        const auto first_t { event.events.at(i).start };
        for (std::size_t j { i + 1 }; j < event.n(); j++) {
            const std::size_t second_h { event.events.at(j).hash };
            auto condition_2 = [&](const auto& iterator){return iterator.first.hash() == second_h;};
            auto it_2 { std::find_if(m_stations.begin(), m_stations.end(), condition_2) };
            if (it_2 == m_stations.end()) {
                const auto& [userinfo, location] {m_stationsupervisor.get_station(first_h)};
                add_station(userinfo, location);
            }
            const std::size_t second { static_cast<std::size_t>(std::distance(m_stations.begin(), it_2)) };
            const auto second_t { event.events.at(j).start };
            if (second_h > first_h) {
                m_data.at(std::max(first, second), std::min(first, second)).hist.add(static_cast<std::int32_t>(first_t - second_t));
            } else {
                m_data.at(std::max(first, second), std::min(first, second)).hist.add(static_cast<std::int32_t>(second_t - first_t));
            }
        }
    }
}

void station_coincidence::save()
{
    std::map<std::size_t, userinfo_t> stations{};
    std::ofstream header_file { m_data_directory + "/stations" };
    for (const auto& [userinfo, location]: m_stations) {
        header_file<<std::hex<<userinfo.hash()<<' '<<userinfo.username<<' '<<userinfo.station_id<<'\n';
        stations.emplace(userinfo.hash(), userinfo);
    }
    header_file.close();

    for (const auto& data: m_data.data()) {
        std::ofstream data_file { m_data_directory + "/" + stations[data.first].station_id + "_" + stations[data.second].station_id + ".dat" };
        for (const auto& bin: data.hist.qualified_bins()) {
            data_file<<((bin.lower + bin.upper) * 0.5)<<' '<<bin.count<<'\n';
        }
        data_file.close();
    }
    reset();
}

void station_coincidence::reset()
{
    m_stations.clear();
    m_data.reset();

    for (const auto& [userinfo, location]: m_stationsupervisor.get_stations()) {
        add_station(userinfo, location);
    }
}

void station_coincidence::add_station(const userinfo_t& userinfo, const location_t& location)
{
    const auto x { m_data.increase() };
    if (x > 0) {
        coordinate::geodetic<double> first {location.lat, location.lon, location.h};
        for (std::size_t y { 0 }; y < x; y++) {
            const auto& [user, loc] { m_stations.at(y) };
            const auto time_of_flight { coordinate::transformation<double, coordinate::WGS84>::straight_distance(first, {loc.lat, loc.lon, loc.h}) / s_c };
            const std::int32_t bin_width { static_cast<std::int32_t>(std::clamp((2.0 * time_of_flight) / static_cast<double>(s_bins), 1.0, s_total_width / static_cast<double>(s_bins))) };
            const std::int32_t min { bin_width * - static_cast<std::int32_t>(s_bins * 0.5) };
            const std::int32_t max { bin_width * static_cast<std::int32_t>(s_bins * 0.5) };
            m_data.emplace(x, y, {userinfo.hash(), user.hash(), histogram<s_bins, std::int32_t, std::uint16_t>{min, max}});
        }
    }
    m_stations.emplace_back(std::make_pair(userinfo, location));
}

}