
#ifndef __ULIN_CONNECTION_H_
#define __ULIN_CONNECTION_H_

#include <uv.h>
#include "buffer.h"

class Connection
{
    friend void dealBuffer(Connection *con, const char *buf, std::size_t len);
public:
    explicit Connection(uv_stream_t *stream) noexcept : m_uv_stream(stream) {}
private:
    Buffer m_buffer;
    uv_stream_t * const m_uv_stream;
};

#endif
