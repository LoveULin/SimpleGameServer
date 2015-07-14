
#ifndef __ULIN_CONNECTION_H_
#define __ULIN_CONNECTION_H_

#include <string>
#include <uv.h>
#include <google/protobuf/message.h>
#include "buffer.h"
#include "proto/ulin.pb.h"

class Connection
{
    friend void dealBuffer(Connection *con, const char *buf, std::size_t len);
public:
    explicit Connection(uv_stream_t *stream) noexcept : m_uv_stream(stream) {}
    const bool Send(const ::google::protobuf::Message &data) noexcept
    {
        ULin::Encap msg;
        // virtual call
        msg.set_msg(data.SerializeAsString());
        assert(!msg.msg().empty());
        msg.set_name(data.GetTypeName());
        bool ret(msg.SerializeToString(&m_tmpBuffer));
        assert(ret);

        ssize_t sendLen(m_sendBuffer.Append(m_tmpBuffer.c_str(), m_tmpBuffer.length()));
        if (0 == sendLen) {
            Flush();
            sendLen = m_sendBuffer.Append(m_tmpBuffer.c_str(), m_tmpBuffer.length());
        }
        assert(sendLen > 0);
        assert(static_cast<std::size_t>(sendLen) == m_tmpBuffer.length());
        // need to reschedule
        uv_async_t * const async(static_cast<uv_async_t*>(m_uv_stream->loop->data));
        async->data = this; 
        if (0 != uv_async_send(async)) {
            // fatal error
        }
        return true;
    }
    void Flush() {
        std::size_t len; 
        char * const data(m_sendBuffer.Product(len));
        assert(nullptr != data);
        assert(0 != len);

        const uv_buf_t buf {data, len};
        uv_write_t req;
        int err(uv_write(&req, m_uv_stream, &buf, 1, Connection::cb_uv_Write));
        if (0 != err) {
            // error handler
        }
    }
private:
    static void cb_uv_Write(uv_write_t *req, int status) noexcept
    {
        if (status < 0) {
            // error handler
        }
    }
private:
    RecvBuffer m_recvBuffer;
    SendBuffer m_sendBuffer;
    std::string m_tmpBuffer;
    uv_stream_t * const m_uv_stream;
};

#endif
