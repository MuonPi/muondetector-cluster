#ifndef DETECTORTRACKER_H
#define DETECTORTRACKER_H

#include "threadrunner.h"

#include <map>
#include <memory>
#include <queue>


namespace MuonPi {

class Event;
class LogMessage;
template <typename T>
class AbstractSource;
class Detector;
class StateSupervisor;

class DetectorTracker : public ThreadRunner
{
public:
    DetectorTracker(std::vector<std::shared_ptr<AbstractSource<LogMessage>>> log_sources, StateSupervisor& supervisor);

    /**
     * @brief accept Check if an event is accepted
     * @param event The event to check
     * @return true if the event belongs to a known detector and the detector is reliable
     */
    [[nodiscard]] auto accept(Event &event) const -> bool;

    /**
     * @brief factor The current maximum factor
     * @return maximum factor between all detectors
     */
    [[nodiscard]] auto factor() const -> float;


    [[nodiscard]] auto get(std::size_t hash) const -> std::shared_ptr<Detector>;
protected:

    /**
     * @brief process Process a log message. Hands the message over to a detector, if none exists, creates a new one.
     * @param log The log message to check
     */
    void process(const LogMessage& log);

    /**
     * @brief step reimplemented from ThreadRunner
     * @return true if the step succeeded.
     */
    [[nodiscard]] auto step() -> int override;

private:
    StateSupervisor& m_supervisor;

    std::vector<std::shared_ptr<AbstractSource<LogMessage>>> m_log_sources {};

    std::map<std::size_t, std::shared_ptr<Detector>> m_detectors {};

    std::queue<std::size_t> m_delete_detectors {};

    float m_factor { 1.0 };
};

}

#endif // DETECTORTRACKER_H
