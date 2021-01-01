#ifndef ABSTRACTEVENTSOURCE_H
#define ABSTRACTEVENTSOURCE_H

#include "threadrunner.h"

#include <future>
#include <memory>
#include <atomic>
#include <queue>

namespace MuonPi {

// +++ Forward declarations
class Event;
// --- Forward declarations

template <typename T>
/**
 * @brief The AbstractSource class
 * Represents a canonical Source for items of type T.
 */
class AbstractSource : public ThreadRunner
{
public:
    AbstractSource();
    /**
     * @brief ~AbstractSource The destructor. If this gets called while the event loop is still running, it will tell the loop to finish and wait for it to be done.
     */
    virtual ~AbstractSource() override;

    /**
      * @brief next_item gets the next available Event from the internal item buffer.
      * @return the next available Event if available, a nullptr otherwise.
      */
    [[nodiscard]] auto next_item() -> T;

    /**
     * @brief has_item Whether the source has items available
     * @return true if there are items to get
     */
    [[nodiscard]] auto has_items() const -> bool;

    /**
     * @brief size Get the number of items currently in the queue
     * @return number of items in the queue
     */
    [[nodiscard]] auto size() const -> std::size_t;

protected:
    /**
     * @brief push_item pushes an item into the source
     * @param item The item to push
     */
    void push_item(T item);

private:

    std::atomic<bool> m_has_items { false };

    std::atomic<std::size_t> m_size { 0 };

    std::queue<T> m_queue {};

    std::mutex m_mutex {};

};



template <typename T>
AbstractSource<T>::AbstractSource()
    : ThreadRunner{"AbstractSource"}
{}

template <typename T>
AbstractSource<T>::~AbstractSource() = default;

template <typename T>
auto AbstractSource<T>::next_item() -> T
{
    if (!has_items()) {
        return {};
    }
    std::scoped_lock<std::mutex> lock {m_mutex};
    auto item {std::move(m_queue.front())};
    m_queue.pop();
    m_size = m_size - 1;
    if (m_queue.size() == 0) {
        m_has_items = false;
    }
    return item;
}

template <typename T>
void AbstractSource<T>::push_item(T item)
{
    std::scoped_lock<std::mutex> lock {m_mutex};
    m_queue.push(std::move(item));
    m_has_items = true;
    m_size = m_size + 1;
}

template <typename T>
auto AbstractSource<T>::has_items() const -> bool
{
    return m_has_items;
}


template <typename T>
auto AbstractSource<T>::size() const -> std::size_t
{
    return m_size;
}


}

#endif // ABSTRACTEVENTSOURCE_H