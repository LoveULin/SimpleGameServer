
#ifndef __ULIN_HANDLER_H_
#define __ULIN_HANDLER_H_

#include <boost/thread/detail/singleton.hpp>
#include <unordered_map>

class Connection;

class Handler : private boost::noncopyable
{
public:
    typedef void (*pfHandler)(Connection *con, const std::string &msg);
public:
    const bool RegHandler(const std::string &name, pfHandler hd) // dont know is there exception 
    {
        const auto itPair(m_handlers.emplace(name, hd));
        return itPair.second;
    }
    const pfHandler GetHandler(const std::string &name) const noexcept
    {
        const auto it(m_handlers.find(name));
        if (it == m_handlers.end()) {
            return nullptr; 
        }
        return it->second;
    }
private:
    std::unordered_map<std::string, pfHandler> m_handlers; 
};

// singleton instance
typedef boost::detail::thread::singleton<Handler> handler;

#endif
