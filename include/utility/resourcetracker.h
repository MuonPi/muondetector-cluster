#ifndef RESOURCETRACKER_H
#define RESOURCETRACKER_H

#include <cstdint>

namespace muonpi {

class ResourceTracker {
public:
    struct Data {
        float process_cpu_load {};
        float system_cpu_load {};
        float memory_usage {};
    };

    [[nodiscard]] auto get_data() -> Data;

private:
    struct {
        std::uint64_t total_time_last {};
        std::uint64_t process_time_last {};
        std::uint64_t system_time_last {};
    } m_cpu {};

    bool m_first { true };
};
}

#endif // RESOURCETRACKER_H
