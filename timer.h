
#ifndef __ULIN_TIMER_H_
#define __ULIN_TIMER_H_

#include <cassert>
#include <unordered_map>
#include <boost/pool/pool.hpp>
#include <uv.h>
#include "type.h"

class Timer : private boost::noncopyable
{
public:
    Timer(uv_loop_t *loop) : m_loop(loop), m_pool(sizeof(uv_timer_t))
    {
        m_timers.max_load_factor(5);
    }
    template<typename T>
    const int Add(unsigned long long initial, unsigned long long interval, 
                  const T &functor, void *data)
    {
        uv_timer_t * const timer(static_cast<uv_timer_t*>(m_pool.malloc()));
        if (UNLIKELY(nullptr == timer)) {
            return -ENOMEM;
        }
        int ret(uv_timer_init(m_loop, timer));
        assert(0 == ret);
        ret = uv_timer_start(timer, [functor](void *data) -> void {
                                        (void)functor(data);
                                    }, initial, interval);
        assert(0 == ret);
        (void)m_timers.emplace(m_idPool, timer);
        return m_idPool++;
    }
    void Remove(std::size_t timerID)
    {
        const auto it(m_timers.find(timerID));
        if (it == m_timers.end()) {
            return;
        }
        (void)uv_timer_stop(it->second);
        (void)m_timers.erase(it);
        assert(m_pool.is_from(it->second));
        m_pool.free(it->second);
        return;
    }
private:
    uv_loop_t *m_loop;
    boost::pool<> m_pool;
    std::size_t m_idPool {1};
    std::unordered_map<std::size_t, uv_timer_t*> m_timers;
};

// singleton instance
typedef boost::detail::thread::singleton<Timer> timer;

#endif

