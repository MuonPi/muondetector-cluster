#include "utility/threadrunner.h"

#include <utility>

#include "utility/log.h"
#include "utility/scopeguard.h"

namespace muonpi {

thread_runner::thread_runner(std::string name, bool use_custom_run)
    : m_use_custom_run { use_custom_run }
    , m_name { std::move(name) }
{
}

thread_runner::~thread_runner() = default;

void thread_runner::stop(int exit_code)
{
    m_run = false;
    m_quit = true;
    m_exit_code = exit_code;
    m_condition.notify_all();
    on_stop();
}

void thread_runner::join()
{
    if ((m_thread != nullptr) && m_thread->joinable()) {
        m_thread->join();
        m_thread.reset();
    }
    if (m_run_future.valid()) {
        m_run_future.wait();
    }
}

auto thread_runner::step() -> int
{
    return 0;
}

auto thread_runner::pre_run() -> int
{
    return 0;
}

auto thread_runner::post_run() -> int
{
    return 0;
}

auto thread_runner::custom_run() -> int
{
    return 0;
}

void thread_runner::on_stop()
{
}

auto thread_runner::wait() -> int
{
    if (!m_run_future.valid()) {
        return -1;
    }
    join();
    return m_run_future.get();
}

auto thread_runner::state() -> State
{
    return m_state;
}

auto thread_runner::run() -> int
{
    m_state = State::Initialising;
    State& state { m_state };
    bool clean { false };
    const scope_guard state_guard { [&state, &clean] {
        if (clean) {
            state = State::Stopped;
        } else {
            state = State::Error;
        }
    } };

    try {
        log::debug() << "Starting thread " << m_name;
        int pre_result { pre_run() };
        if (pre_result != 0) {
            return pre_result;
        }

        if ((m_thread != nullptr)) {
            log::debug() << "setting name for thread " << m_name;
            auto handle { m_thread->native_handle() };
            const auto result { pthread_setname_np(handle, m_name.c_str()) };
            if (result != 0) {
                log::debug() << "couldn't set name of thread " << m_name << " (" << ((result == ERANGE) ? std::string { "ERANGE" } : std::to_string(result)) << ")";
            }
        }
        m_state = State::Running;
        if (m_use_custom_run) {
            int result { custom_run() };
            if (result != 0) {
                return result;
            }
        } else {
            while (m_run) {
                int result { step() };
                if (result != 0) {
                    log::warning() << "Thread " << m_name << " Stopped.";
                    return result;
                }
            }
        }
        m_state = State::Finalising;
        log::debug() << "Stopping thread " << m_name;
        clean = true;
        return post_run() + m_exit_code;
    } catch (std::exception& e) {
        log::error() << "Thread " << m_name << "Got an uncaught exception: " << e.what();
        return -1;
    } catch (...) {
        log::error() << "Thread " << m_name << "Got an uncaught exception.";
        return -1;
    }
}

void thread_runner::exec()
{

    std::promise<int> promise {};
    m_run_future = promise.get_future();
    int value = run();
    promise.set_value(value);
}

void thread_runner::finish()
{
    stop();
    join();
}

auto thread_runner::name() -> std::string
{
    return m_name;
}

auto thread_runner::state_string() -> std::string
{
    switch (m_state) {
    case State::Error:
        return "Error";
    case State::Stopped:
        return "Stopped";
    case State::Initial:
        return "Initial";
    case State::Initialising:
        return "Initialising";
    case State::Running:
        return "Running";
    case State::Finalising:
        return "Finalising";
    }
    return {};
}

void thread_runner::start()
{
    if ((m_state > State::Initial) || (m_thread != nullptr)) {
        log::info() << "Thread " << m_name << " already running, refusing to start.";
        return;
    }
    m_thread = std::make_unique<std::thread>(&thread_runner::exec, this);
}

void thread_runner::start_synchronuos()
{
    if (m_state > State::Initial) {
        return;
    }
    exec();
}

} // namespace muonpi
