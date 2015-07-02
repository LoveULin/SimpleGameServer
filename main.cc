
#include <cassert>
#include <string>
#include <uv.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "type.h"
#include "connection.h"
#include "buffer.h"

static constexpr int uvBufferUnit = 256;

struct pool_tag {};
typedef boost::singleton_pool<pool_tag, (uvBufferUnit * sizeof(char))> spl;

void cb_uv_AllocBuffer(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf)
{
    // ignore the suggested_size or we can use spl::ordered_malloc for chunk
    (void)suggested_size;

    char *newBuf(static_cast<char*>(spl::malloc()));
    if (UNLIKELY(nullptr == newBuf)) {
        // fatal error
    }
    buf->base = newBuf;
    buf->len = (uvBufferUnit * sizeof(char));
}

void cb_uv_Close(uv_handle_t *handle)
{
    delete static_cast<Connection*>(handle->data);
    handle->data = nullptr;
}

void dealBuffer(Connection *con, const char *buf, std::size_t len)
{
    std::size_t appendLen(con->m_buffer.Append(buf, len));
    while (appendLen != len) {
        unsigned short packetLen;
        const char *packet(con->m_buffer.Consume(packetLen));
        if (nullptr == packet) {
            break;
        }
        // unserializing packet and handle

        len -= appendLen;
        appendLen = con->m_buffer.Append(buf + appendLen, len);
    }
}

void cb_uv_Read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        // error condition
        uv_read_stop(stream);
        uv_close(reinterpret_cast<uv_handle_t*>(stream), cb_uv_Close);
    }
    else if (nread > 0) {
        dealBuffer(static_cast<Connection*>(stream->data), buf->base, nread);
    }
    // free buffer always
    if ((nullptr != buf->base) && (0 != buf->len)) {
        if (LIKELY(spl::is_from(buf->base))) {
            spl::free(buf->base);
        }
        else {
            // fatal error
        }
    }
}

void cb_uv_Accept(uv_stream_t *req, int status)
{
    uv_tcp_t client;
    if (UNLIKELY(0 != uv_tcp_init(req->loop, &client))) {
        // fatal error
    }
    // set in our buffer
    client.data = new Connection(req);
    if (UNLIKELY(0 != uv_accept(req, reinterpret_cast<uv_stream_t*>(&client)))) {
        // fatal error
    }
    uv_read_start(reinterpret_cast<uv_stream_t*>(&client), cb_uv_AllocBuffer, cb_uv_Read);
}

int main()
{
    // init the loop
    uv_loop_t *loop(uv_default_loop());
    assert(nullptr != loop);

    // init a tcp listener
    uv_tcp_t handle;
    int ret(uv_tcp_init(loop, &handle));
    assert(0 == ret);

    // load config
    sockaddr_in addr;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("./config.json", pt); // return value ??
    ret = uv_ip4_addr(pt.get<std::string>("myip").c_str(), pt.get<int>("myport"), &addr);
    assert(0 == ret);
    ret = uv_tcp_bind(&handle, reinterpret_cast<sockaddr*>(&addr), 0);
    assert(0 == ret);
    ret = uv_listen(reinterpret_cast<uv_stream_t*>(&handle), SOMAXCONN, cb_uv_Accept);
    assert(0 == ret);

    // go!!
    ret = uv_run(loop, UV_RUN_DEFAULT);

    // os will do the cleanup
}
