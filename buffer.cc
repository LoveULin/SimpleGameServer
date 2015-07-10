
#include <arpa/inet.h>
#include <string>
#include "buffer.h"

Buffer::~Buffer()
{
    spl::free(m_buf);
}

const ssize_t Buffer::Append(const char *buf, std::size_t len) noexcept
{
    if (len > bufferUnit) {
        return -1;
    }
    std::size_t ret(len);
    if ((bufferUnit - m_end) >= len) {
        // space enough
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
    m_end += ret;
    // the real byte
    return ret;
}

char * const SendBuffer::Product(std::size_t &len) noexcept
{
    if (m_end == m_pos) {
        return nullptr;
    }
    len = (m_end - m_pos);
    char * const msg(m_buf + m_pos);
    m_pos = 0;
    m_end = 0;
    return msg;
} 

const char * const RecvBuffer::Consume(unsigned short &len) noexcept
{
    const std::size_t dataLen(m_end - m_pos);
    if (dataLen < packetHLen) {
        return nullptr;
    }
    len = ntohs(*(reinterpret_cast<unsigned short*>(m_buf + m_pos)));
    if (dataLen < len) {
        return nullptr;
    }
    const char * const msg(m_buf + m_pos + sizeof(unsigned short));
    m_pos += len;
    len -= sizeof(unsigned short);
    return msg;
}

