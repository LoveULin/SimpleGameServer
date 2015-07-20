
#ifndef __ULIN_CIRCLE_H_
#define __ULIN_CIRCLE_H_

#include <memory>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

class Circle : private boost::noncopyable
{
    class Deleter
    {
    public:
        explicit Deleter(boost::pool<> &pool) : m_pool(pool) {}
        void operator()(uv_idle_t *idle)
        {
            assert(m_pool.is_from(idle));
            m_pool.free(idle);
        }
    private:
        boost::pool<> &m_pool;
    };
public:
    typedef void(*pf)(void); // temp callback style

    explicit Circle(uv_loop_t *loop) noexcept : m_loop(loop), m_pool(sizeof(uv_idle_t))
    {
        m_circles.max_load_factor(5);
    }
    ~Circle() // default noexcept
    {
        m_circles.clear();
    }
    const ssize_t Add(pf callback)
    {
        std::unique_ptr<uv_idle_t, Deleter> idle(static_cast<uv_idle_t*>(m_pool.malloc()), 
                                                 Deleter(m_pool));
        if (UNLIKELY(!idle)) {
            return -ENOMEM;
        }
        int ret(uv_idle_init(m_loop, idle.get()));
        assert(0 == ret);
        idle->data = reinterpret_cast<void*>(callback);
        ret = uv_idle_start(idle.get(), [](uv_idle_t *handle) -> void {
                                            (void)reinterpret_cast<pf>(handle->data)();
                                        });
        assert(0 == ret);
        (void)m_circles.emplace(m_idPool, std::move(idle));
        return m_idPool++;
    }
    void Remove(std::size_t circleID)
    {
        const auto it(m_circles.find(circleID));
        if (it == m_circles.end()) {
            return;
        }
        (void)uv_idle_stop(it->second.get());
        (void)m_circles.erase(it);
    }
private:
    uv_loop_t *m_loop;
    boost::pool<> m_pool;
    std::size_t m_idPool {1};
    std::unordered_map<std::size_t, std::unique_ptr<uv_idle_t, Deleter>> m_circles;
};

#endif

