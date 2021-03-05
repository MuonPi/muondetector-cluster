#ifndef EVENT_H
#define EVENT_H

#include "defaults.h"
#include "messages/detectorinfo.h"
#include "messages/userinfo.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace MuonPi {

class Detector;

struct TimeBase {
    double factor { 0.0 };
    std::chrono::steady_clock::duration base {};
};

/**
 * @brief The Event class
 *  class, which is used as an interface for single events and combined events
 */
class Event {
public:
    struct Data {
        std::string user {};
        std::string station_id {};
        std::int_fast64_t start {};
        std::int_fast64_t end {};
        std::uint32_t time_acc {};
        std::uint16_t ublox_counter {};
        std::uint8_t fix {};
        std::uint8_t utc {};
        std::uint8_t gnss_time_grid {};
    };

    Event(std::size_t hash, Data data) noexcept;

    Event(Event event, bool foreign) noexcept;

    Event() noexcept;

    virtual ~Event() noexcept;

    void set_detector_info(Location location, /*Time time,*/ UserInfo user);

    auto location() const -> Location;

    auto user_info() const -> UserInfo;

    /**
     * @brief start
     * @return The starting time of the event
     */
    [[nodiscard]] auto start() const noexcept -> std::int_fast64_t;

    /**
     * @brief duration
     * @return The duration of the event
     */
    [[nodiscard]] auto duration() const noexcept -> std::int_fast64_t;

    /**
     * @brief end
     * @return The end time of the event
     */
    [[nodiscard]] auto end() const noexcept -> std::int_fast64_t;

    /**
     * @brief hash
     * @return The hash of the detector for this event
     */
    [[nodiscard]] auto hash() const noexcept -> std::size_t;

    /**
     * @brief n
     * @return The number of events of this event. 1 for a default event
     */
    [[nodiscard]] auto n() const noexcept -> std::size_t;

    /**
      * @brief Get the list of events.
      * @return The list of events contained in this combined event
      */
    [[nodiscard]] auto events() const -> const std::vector<Event>&;

    /**
     * @brief add_event Adds an event to the CombinedEvent.
     * @param event The event to add. In the case that the abstract event is a combined event, the child events will be added instead of their combination.
     */
    void add_event(Event event) noexcept;

    /**
     * @brief valid
     * @return true when the event is valid
     */
    [[nodiscard]] auto valid() const -> bool;

    /**
     * @brief data get the data
     * @return The data struct of the event
     */
    [[nodiscard]] auto data() const -> Data;

    /**
     * @brief set_data Sets the data for the event
     * @param data The new data to use
     */
    void set_data(const Data& data);

private:
    std::size_t m_n { 1 };
    std::vector<Event> m_events {};
    std::uint64_t m_hash {};
    bool m_valid { true };

    Data m_data {};

    Location m_location {};
    UserInfo m_user_info {};
};
}

#endif // EVENT_H
