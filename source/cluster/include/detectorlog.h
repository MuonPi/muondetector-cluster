#ifndef DETECTORLOG_H
#define DETECTORLOG_H

#include <chrono>

namespace MuonPi {

struct Location {
    double lat { 0.0 };
    double lon { 0.0 };
    double h { 0.0 };
    double prec { std::numeric_limits<double>::max() };
    double dop { std::numeric_limits<double>::min() };

    static constexpr double maximum_prec { 1.0 };
    static constexpr double maximum_dop { 1.0 };
};

/**
 * @brief The DetectorLog class
 */
class DetectorLog
{
public:

    /**
     * @brief DetectorLog
     * @param hash The hash of the detector identifier
     * @param location The detector Location information
     */
    DetectorLog(std::size_t hash, Location location);

    DetectorLog() noexcept;

    DetectorLog(const DetectorLog& other);
    DetectorLog(DetectorLog&& other);

    /**
     * @brief hash
     * @return The hash of the detector for this event
     */
    [[nodiscard]] auto hash() const noexcept -> std::size_t;

    /**
     * @brief location The location of the detector from this log message
     * @return The location data
     */
    [[nodiscard]] auto location() const -> Location;

    /**
     * @brief time The time this log message arrived
     * @return The arrival time
     */
    [[nodiscard]] auto time() const -> std::chrono::system_clock::time_point;

    [[nodiscard]] auto valid() const -> bool;

private:
    std::size_t m_hash { 0 };
    Location m_location {};
    std::chrono::system_clock::time_point m_time { std::chrono::system_clock::now() };

    bool m_valid { true };
};
}

#endif // DETECTORLOG_H