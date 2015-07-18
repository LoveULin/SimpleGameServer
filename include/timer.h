
#ifndef __ULIN_TIMER_H_
#define __ULIN_TIMER_H_

#include <cassert>
#include <memory>
#include <unordered_map>
#include <boost/pool/pool.hpp>
#include <uv.h>
#include "type.h"

class Timer : private boost::noncopyable
{
    class Deleter
    {
    public:
        explicit Deleter(boost::pool<> &pool) : m_pool(pool) {}
        void operator()(uv_timer_t *timer)
        {
            assert(m_pool.is_from(timer));
            m_pool.free(timer);
        }
    private:
        boost::pool<> &m_pool;
    };
public:
    typedef void(*pf)(void); // temp callback style

    explicit Timer(uv_loop_t *loop) noexcept : m_loop(loop), m_pool(sizeof(uv_timer_t))
    {
        m_timers.max_load_factor(5);
    }
    ~Timer() // default noexcept
    {
        m_timers.clear();
    }
    const ssize_t Add(unsigned long long initial, unsigned long long interval, 
                      pf callback)
    {
        std::unique_ptr<uv_timer_t, Deleter> timer(static_cast<uv_timer_t*>(m_pool.malloc()), 
                                                   Deleter(m_pool));
        if (UNLIKELY(!timer)) {
            return -ENOMEM;
        }
        int ret(uv_timer_init(m_loop, timer.get()));
        assert(0 == ret);
        timer->data = reinterpret_cast<void*>(callback);
        ret = uv_timer_start(timer.get(), [](uv_timer_s *data) -> void {
                                              reinterpret_cast<pf>(data->data)();
                                          }, initial, interval);
        assert(0 == ret);
        (void)m_timers.emplace(m_idPool, std::move(timer));
        return m_idPool++;
    }
    void Remove(std::size_t timerID)
    {
        const auto it(m_timers.find(timerID));
        if (it == m_timers.end()) {
            return;
        }
        (void)uv_timer_stop(it->second.get());
        (void)m_timers.erase(it);
        return;
    }
private:
    uv_loop_t *m_loop;
    boost::pool<> m_pool;
    std::size_t m_idPool {1};
    std::unordered_map<std::size_t, std::unique_ptr<uv_timer_t, Deleter>> m_timers;
};

#endif

