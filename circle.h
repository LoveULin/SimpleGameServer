
#ifndef __ULIN_CIRCLE_H_
#define __ULIN_CIRCLE_H_

#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

class Circle : private boost::noncopyable
{
public:
    Circle(uv_loop_t *loop) : m_loop(loop), m_pool(sizeof(uv_idle_t))
    {
        m_circles.max_load_factor(5);
    }
    template<typename T>
    const ssize_t Add(const T &functor)
    {
        uv_idle_t * const idle(static_cast<uv_idle_t*>(m_pool.malloc()));
        if (nullptr == idle) {
            return -ENOMEM;
        }
        int ret(uv_idle_init(m_loop, idle));
        assert(0 == ret);
        ret = uv_idle_start(idle, [functor](void *data) -> void {
                                   (void)functor(data);
                                });
        assert(0 == ret);
        (void)m_circles.emplace(m_idPool, idle);
        return m_idPool++;
    }
    void Remove(std::size_t circleID)
    {
        const auto it(m_circles.find(circleID));
        if (it == m_circles.end()) {
            return;
        }
        uv_idle_t * const circle(it->second);
        (void)uv_idle_stop(circle);
        (void)m_circles.erase(it);
        assert(m_pool.is_from(circle));
        m_pool.free(circle);
    }
private:
    uv_loop_t *m_loop;
    boost::pool<> m_pool;
    std::size_t m_idPool {1};
    std::unordered_map<std::size_t, uv_idle_t*> m_circles;
};

// singleton instance
typedef boost::detail::thread::singleton<Circle> circle;

#endif

