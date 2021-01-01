#ifndef DETECTORINFO_H
#define DETECTORINFO_H

#include "userinfo.h"
#include "mqttlink.h"

#include <chrono>

namespace MuonPi {

struct Location {
    double lat { 0.0 };
    double lon { 0.0 };
    double h { 0.0 };
    double v_acc { 0.0 };
    double h_acc { 0.0 };
    double dop { 0.0 };
};

struct Time
{
    double accuracy { 0.0 };
    double dop { 0.0 };
};

/**
 * @brief The DetectorInfo class
 */
class DetectorInfo /*: public MqttLink::BaseMessage*/
{
public:

    /**
     * @brief DetectorInfo
     * @param hash The hash of the detector identifier
     * @param user_info The user info object
     * @param location The detector location information
     */
    DetectorInfo(std::size_t hash, /* std::string msg_time,*/ UserInfo user_info, Location location);

    DetectorInfo() noexcept;

    DetectorInfo(const DetectorInfo& other);
    DetectorInfo(DetectorInfo&& other);

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

//    [[nodiscard]] auto valid() const -> bool;

    /**
     * @brief data Accesses the user info from the object
     * @return the UserInfo struct
     */
    [[nodiscard]] auto user_info() const -> UserInfo;


    std::size_t m_hash { 0 };
    Location m_location {};
    Time m_time_info {};
    UserInfo m_userinfo {};


private:
    std::chrono::system_clock::time_point m_time { std::chrono::system_clock::now() };

    bool m_valid { true };
};
}

#endif // DETECTORINFO_H