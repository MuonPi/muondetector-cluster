#ifndef USERINFO_H
#define USERINFO_H

#include <string>

namespace muonpi {

struct userinfo_t {
    std::string username {};
    std::string station_id {};

    /**
     * @brief site_id Creates a site_id string from the username and the station id.
     * @return The site id as string
     */
    [[nodiscard]] inline auto site_id() const -> std::string { return username + station_id; }

    /**
     * @brief hash Create a hashed version of the site_id
     * @return A size_t containting the hash
     */
    [[nodiscard]] inline auto hash() const -> std::size_t { return std::hash<std::string> {}(site_id()); }
};
}
#endif // USERINFO_H
