#ifndef GEOHASH_H
#define GEOHASH_H

#include <string>


namespace MuonPi {
/**
 * @brief The GeoHash class
 * a class with a static function to generate a geo hash string from given geographic ccordinates
 */
class GeoHash
{
public:
    GeoHash()=delete;
	static std::string hashFromCoordinates(double lon, double lat, std::size_t precision=6);
};

	
} // namespace MuonPi

#endif // GEOHASH_H