#ifndef LOG_H
#define LOG_H

#include "application.h"
#include "defaults.h"

#include "utility/configuration.h"

#include <iostream>
#include <sstream>
#include <string>

namespace muonpi::log {

enum Level : int {
    Emergency,
    Alert,
    Critical,
    Error,
    Info,
    Warning,
    Notice,
    Debug
};

template <Level L>
class logger {
public:
    template <typename T>
    auto operator<<(T content) -> logger<L>&
    {
        m_stream << content;
        return *this;
    }

    logger(int exit_code)
        : m_exit_code { std::move(exit_code) }
    {
    }

    logger() = default;

    ~logger()
    {
        if (L <= (Level::Info + config::singleton()->meta.verbosity)) {
            std::cerr << to_string() << m_stream.str() + "\n"
                      << std::flush;
        }
        if (L <= Level::Critical) {
            application::shutdown(std::move(m_exit_code));
        }
    }

private:
    std::ostringstream m_stream {};
    int m_exit_code { 0 };

    [[nodiscard]] auto to_string() -> std::string
    {
        switch (L) {
        case Level::Debug:
            return "Debug: ";
        case Level::Info:
            return "";
        case Level::Notice:
            return "Notice: ";
        case Level::Warning:
            return "Warning: ";
        case Level::Error:
            return "Error: ";
        case Level::Critical:
            return "Critical: ";
        case Level::Alert:
            return "Alert: ";
        case Level::Emergency:
            return "Emergency: ";
        }
        return {};
    }
};

[[nodiscard]] auto debug() -> logger<Level::Debug>;
[[nodiscard]] auto info() -> logger<Level::Info>;
[[nodiscard]] auto notice() -> logger<Level::Notice>;
[[nodiscard]] auto warning() -> logger<Level::Warning>;
[[nodiscard]] auto error() -> logger<Level::Error>;
[[nodiscard]] auto critical(int exit_code = 1) -> logger<Level::Critical>;
[[nodiscard]] auto alert(int exit_code = 1) -> logger<Level::Alert>;
[[nodiscard]] auto emergency(int exit_code = 1) -> logger<Level::Emergency>;

}

#endif // LOG_H
