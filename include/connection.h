
#ifndef __ULIN_CONNECTION_H_
#define __ULIN_CONNECTION_H_

#include <string>
#include <uv.h>
#include <google/protobuf/message.h>
#include "logger.h"
#include "buffer.h"
#include "handler.h"
#include "proto/ulin.pb.h"

class Connection
{
public:
    explicit Connection(uv_tcp_t *stream) noexcept : m_uv_tcp(stream) {}
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
        uv_async_t * const async(static_cast<uv_async_t*>(m_uv_tcp->loop->data));
        if (nullptr != async) {
            async->data = this; 
            int err(uv_async_send(async));
            if (0 != err) {
                LOG_FATAL << "Send, ret: " << err;
            }
        }
        else {
            Flush();
        }
        return true;
    }
    void Flush() {
        std::size_t len; 
        char * const data(m_sendBuffer.Product(len));
        assert(nullptr != data);
        assert(0 != len);

        const uv_buf_t buf {data, len};
        const int ret(uv_write(&m_write, reinterpret_cast<uv_stream_t*>(m_uv_tcp), &buf, 1, Connection::cb_uv_Write));
        if (0 != ret) {
            LOG_FATAL << "Flush, ret: " << ret;
        }
    }
    void DealBuffer(const char *buf, std::size_t len)
    {
        // lamda
        const auto handlePacket = [this]() -> bool {
            unsigned short packetLen;
            const char * const packet(this->m_recvBuffer.Consume(packetLen));
            if (nullptr == packet) {
                // no valid whole packet
                return false;
            }
            // unserializing packet and handle
            ULin::Encap msg;
            const bool ret(msg.ParseFromString(std::string(packet, packetLen)));
            if (ret) {
                const Handler::pfHandler pf(handler::instance().GetHandler(msg.name()));
                if (nullptr != pf) {
                    LOG_INFO << "DealBuffer, handle a message: " << msg.name();
                    pf(this, msg.msg());
                }
                else {
                    LOG_WARN << "DealBuffer, recv a unhandled message: " << msg.name();
                }
            }
            else {
                LOG_WARN << "DealBuffer, recv a illegal message";
            }
            return true; 
        };
        // TODO: need to handle the flood attack
        ssize_t appendLen(m_recvBuffer.Append(buf, len));
        while ((appendLen > 0) && (static_cast<std::size_t>(appendLen) != len)) {
            (void)handlePacket();
            appendLen += m_recvBuffer.Append(buf + appendLen, (len - appendLen));
        }
        while (handlePacket());
    }
private:
    static void cb_uv_Write(uv_write_t *req, int status) noexcept
    {
        if (status < 0) {
            LOG_FATAL << "Write, " << uv_strerror(status);
        }
    }
private:
    uv_write_t m_write;
    RecvBuffer m_recvBuffer;
    SendBuffer m_sendBuffer;
    std::string m_tmpBuffer;
    uv_tcp_t* const m_uv_tcp;
};

#endif

