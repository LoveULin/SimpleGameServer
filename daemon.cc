
#include <cassert>
#include <string>
#include <chrono>
#include <thread>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/detail/singleton.hpp>
#include "daemon.h"
#include "uvutil.h"
#include "timer.h"
#include "proto/ulin.pb.h"

Daemon::Daemon() noexcept : m_loop(uv_default_loop()), m_timer(m_loop), m_circle(m_loop)
{
    assert(nullptr != m_loop);
    // load configure first
    boost::property_tree::read_json("./config.json", pt::instance());
    LOG_INFO << "read configure...done, initing modules...";

    // init CSVs
    loadCSV::instance().LoadAllCSV();
    // register handlers
    handler::instance().RegAllHandlers();

    // init libuv's eventfd
    int ret(uv_async_init(m_loop, &m_async, cb_uv_Async));
    assert(0 == ret);
    m_loop->data = &m_async; // set in uv_async

    // init a signal handler for exit normally
    ret = uv_signal_init(m_loop, &m_signal);
    assert(0 == ret);
    ret = uv_signal_start(&m_signal, cb_uv_HandleSignal, SIGINT);
    assert(0 == ret);

    // init a tcp listener, for accepting the connections from game client
    ret = uv_tcp_init(m_loop, &m_handle);
    assert(0 == ret);
    sockaddr_in addr;
    ret = uv_ip4_addr(pt::instance().get<std::string>("myip").c_str(), 
                      pt::instance().get<int>("myport"), &addr);
    assert(0 == ret);
    ret = uv_tcp_bind(&m_handle, reinterpret_cast<sockaddr*>(&addr), 0);
    assert(0 == ret);
    ret = uv_listen(reinterpret_cast<uv_stream_t*>(&m_handle), SOMAXCONN, cb_uv_Accept);
    assert(0 == ret);
    LOG_INFO << "init modules...done, GO!!";
}

void Daemon::Start() noexcept
{
    // go!!
    int ret(uv_run(m_loop, UV_RUN_DEFAULT));
    // never reach here, bu we log the return value
    LOG_WARN << "uv_run's ret: " << ret << ", Bye...";
}

void Daemon::SetExtra() noexcept
{
    // add a timer
    std::size_t timerID(m_timer.Add(1000, 3000, []() -> void {
                                        std::cout << "LOL" << std::endl;
                                    }));
    LOG_INFO << "new timerID is: " << timerID;
    // add a idle circle
    std::size_t circleID(m_circle.Add([]() -> void {
                                         std::cout << "idle circle" << std::endl;
                                         std::this_thread::sleep_for(std::chrono::seconds(2));
                                     }));
    LOG_INFO << "new idleID is: " << circleID;
}

int main()
{
    // singleton instance
    typedef boost::detail::thread::singleton<Daemon> d;
    d::instance().SetExtra();
    d::instance().Start();
}

