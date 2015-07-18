
#ifndef __ULIN_DAEMON_H_
#define __ULIN_DAEMON_H_

#include <uv.h>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include "timer.h"

class Daemon : private boost::noncopyable
{
public:
    // init resources
    Daemon() noexcept;
    // start the game
    void Start() noexcept;
    // set some extra features
    void SetExtra() noexcept;
private:
    uv_loop_t * const m_loop; // should be the first status
    uv_async_t m_async;
    uv_signal_t m_signal;
    uv_tcp_t m_handle;
    Timer m_timer;
};

#endif

