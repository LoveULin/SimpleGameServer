
#ifndef __ULIN_REDISCLIENT_H_
#define __ULIN_REDISCLIENT_H_

#include <boost/noncopyable.hpp>
// redox c++ redis-client lib
#include <redox.hpp>

// an object means a connection with redis-server
class RedisClient : private boost::noncopyable
{
public:
    explicit RedisClient(const std::string &path = redox::REDIS_DEFAULT_PATH)
    {
        // we can have a call back of the connecting if we need
        const bool ret(rdx.connectUnix(path));
        assert(ret);
    }
    explicit RedisClient(const std::string &addr = redox::REDIS_DEFAULT_HOST, 
                unsigned short port = redox::REDIS_DEFAULT_PORT)
    {
        // we can have a call back of the connecting if we need
        const bool ret(rdx.connect(addr.c_str(), port));
        assert(ret);
    }
    const bool SaveTo() const
    {
        return true;
    } 
    const std::string LoadFrom() const
    {
        return std::string();
    }
    ~RedisClient()
    {
        rdx.disconnect();
    }
private:
    mutable redox::Redox rdx;
};

#endif
