
#ifndef __ULIN_BUFFER_H
#define __ULIN_BUFFER_H

#include <string>
#include <boost/pool/singleton_pool.hpp>
#include "type.h"

class Buffer : private boost::noncopyable
{
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
    ~Buffer() // default noexcept
    {
        spl::free(m_buf);
    }
    const std::size_t Append(const char *buf, std::size_t len) noexcept
    {
        std::size_t ret(len);
        if ((bufferUnit - m_end) >= len) {
            // enough space
            (void)memcpy(m_buf + m_end, buf, len);
        }
        else {
            // move directly
            (void)memmove(m_buf, (m_buf + m_pos), m_pos);
            m_end -= m_pos;
            m_pos = 0;
            // copy again
            ret = std::min(len, bufferUnit - m_end);
            (void)memcpy(m_buf + m_end, buf, ret);
        }
        // the real byte
        return ret;
    }
    // every time a complete packet
    const char * const Consume(unsigned short &len) noexcept
    {
        std::size_t dataLen(m_end - m_pos);
        if (dataLen < packetHLen) {
            return nullptr;
        }
        len = ntohs(*(reinterpret_cast<unsigned short*>(m_buf + m_pos)));
        if (dataLen < len) {
            return nullptr;
        }
        return (m_buf + m_pos);
    }
private:
    char * const m_buf; // buffer
    std::size_t m_pos {0}; // current read pos
    std::size_t m_end {0}; // current write pos
};

#endif

