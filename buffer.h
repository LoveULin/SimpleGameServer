
#ifndef __ULIN_BUFFER_H
#define __ULIN_BUFFER_H

#include <string>
#include <boost/pool/singleton_pool.hpp>
#include "type.h"

class Buffer : private boost::noncopyable
{
protected:
    // packet length field's length
    static constexpr int packetHLen = 2;

    static constexpr int bufferUnit = 65536;
    struct pool_tag {};
    typedef boost::singleton_pool<pool_tag, (bufferUnit * sizeof(char))> spl;
public:
    Buffer() noexcept : m_buf(static_cast<char*>(spl::malloc()))
    {
        if (UNLIKELY(nullptr == m_buf)) {
            // fatal error
        }
    }
    virtual ~Buffer() = 0; // default noexcept
    const ssize_t Append(const char *buf, std::size_t len) noexcept;
protected:
    char * const m_buf; // buffer
    std::size_t m_pos {0}; // current read pos
    std::size_t m_end {0}; // current write pos
};

class SendBuffer : public Buffer
{
public:
    ~SendBuffer() = default;
    char * const Product(std::size_t &len) noexcept;
    const ssize_t Append(const char *buf, std::size_t len) noexcept
    {
        std::size_t totalLen(len + packetHLen);
        if (totalLen > bufferUnit) {
            return -1;
        }
        // move directly
        (void)memmove(m_buf, (m_buf + m_pos), m_pos);
        m_end -= m_pos;
        m_pos = 0;
        if (totalLen > (bufferUnit - m_end)) {
            return 0;
        }
        const unsigned short netOrderLen(htons(static_cast<unsigned short>(len + packetHLen)));
        ssize_t ret(Buffer::Append(reinterpret_cast<const char*>(&netOrderLen), 
                                         sizeof(netOrderLen)));
        assert(sizeof(netOrderLen) == ret);
        ret = Buffer::Append(buf, len);
        assert(ret == len);
        return ret;
    }
};

class RecvBuffer : public Buffer
{
public:
    ~RecvBuffer() = default;
    // every time a complete packet
    const char * const Consume(unsigned short &len) noexcept;
};

#endif

