
#include <cassert>
#include <string>
#include <uv.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "type.h"
#include "logger.h"
#include "connection.h"
#include "buffer.h"
#include "handler.h"
#include "loadcsv.h"
#include "redisclient.h"
#include "proto/ulin.pb.h"

namespace
{
    constexpr int uvBufferUnit = 256;
}

// ptree global instance
boost::property_tree::ptree pt;

struct pool_tag {};
typedef boost::singleton_pool<pool_tag, (uvBufferUnit * sizeof(char))> spl;

struct client_pool_tag {};
typedef boost::singleton_pool<client_pool_tag, sizeof(uv_tcp_t)> clientSPL;

static void cb_uv_AllocBuffer(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf)
{
    // ignore the suggested_size or we can use spl::ordered_malloc for chunk
    (void)suggested_size;

    char * const newBuf(static_cast<char*>(spl::malloc()));
    if (UNLIKELY(nullptr == newBuf)) {
        LOG_FATAL << "AllocBufferForUV, malloc failed";
    }
    buf->base = newBuf;
    buf->len = (uvBufferUnit * sizeof(char));
}

static void cb_uv_Close(uv_handle_t *handle)
{
    LOG_NOTIFY << "Close a connection";
    // delete our Connection
    delete static_cast<Connection*>(handle->data);
    handle->data = nullptr;
    // reset the loop's data: uv_async_t(for others)
    static_cast<uv_async_t*>(handle->loop->data)->data = nullptr;
    // free handle of client
    assert(clientSPL::is_from(handle));
    clientSPL::free(handle);
}

void dealBuffer(Connection *con, const char *buf, std::size_t len)
{
    // lamda
    const auto handlePacket = [&con]() -> bool {
        unsigned short packetLen;
        const char * const packet(con->m_recvBuffer.Consume(packetLen));
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
                pf(con, msg.msg());
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
    ssize_t appendLen(con->m_recvBuffer.Append(buf, len));
    while ((appendLen > 0) && (static_cast<std::size_t>(appendLen) != len)) {
        (void)handlePacket();
        appendLen += con->m_recvBuffer.Append(buf + appendLen, (len - appendLen));
    }
    while (handlePacket());
}

static void cb_uv_Read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        // error condition
        LOG_ERROR << "Read error, nread: " << nread;
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
            LOG_FATAL << "Read error, the buffer isn't from mpool";
        }
    }
}

static void cb_uv_Accept(uv_stream_t *req, int status)
{
    if (0 != status) {
        // log the error and return
        LOG_FATAL << "Accept failed, status: " << status;
        return;
    }
    uv_tcp_t * const client(static_cast<uv_tcp_t*>(clientSPL::malloc()));
    if (UNLIKELY(nullptr == client)) {
        LOG_FATAL << "Accept failed, malloc";
        return;
    }
    do
    {
        int ret(uv_tcp_init(req->loop, client));
        if (UNLIKELY(0 != ret)) {
            LOG_FATAL << "Accept failed, uv_tcp_init: " << ret;
            break;
        }
        ret = uv_accept(req, reinterpret_cast<uv_stream_t*>(client));
        if (UNLIKELY(0 != ret)) {
            LOG_FATAL << "Accept failed, uv_accept: " << ret;
            break;
        }
        ret = uv_read_start(reinterpret_cast<uv_stream_t*>(client), 
                            cb_uv_AllocBuffer, 
                            cb_uv_Read);
        if (UNLIKELY(0 != ret)) {
            LOG_FATAL << "Accept failed, uv_read_start: " << ret;
            break;
        }
        LOG_INFO << "Accept a connection";
        // nothing failed, set in the connection and return
        client->data = new Connection(req);
        return;
    }while (false);
    // free the memory
    clientSPL::free(client); 
}

static void cb_uv_HandleSignal(uv_signal_t *handle, int signum)
{
    // :D
    assert(handle->signum == signum);

    LOG_INFO << "Handle SIGINT";

    // do nothing, but exit immediately
    quick_exit(EXIT_SUCCESS);
}

static void cb_uv_Async(uv_async_t *handle)
{
    Connection * const conn(static_cast<Connection*>(handle->data));
    if (nullptr == conn) {
        return;
    }
    // conn->Flush();
}

int main()
{
    // load config first
    boost::property_tree::read_json("./config.json", pt);
    LOG_INFO << "read configure...done, initing modules...";

    // init CSVs
    loadCSV::instance().LoadAllCSV();
    // register handlers
    handler::instance().RegAllHandlers();

    // init the loop
    uv_loop_t * const loop(uv_default_loop());
    assert(nullptr != loop);

    // init libuv's eventfd, local variable is ok
    uv_async_t async;
    int ret(uv_async_init(loop, &async, cb_uv_Async));
    assert(0 == ret);
    loop->data = &async; // set in uv_async

    // init a signal handler for exit normally, local variable is ok
    uv_signal_t signal;
    ret = uv_signal_init(loop, &signal);
    assert(0 == ret);
    ret = uv_signal_start(&signal, cb_uv_HandleSignal, SIGINT);
    assert(0 == ret);

    // init a tcp listener, for accepting the connections from game client, local variable is ok
    uv_tcp_t handle;
    ret = uv_tcp_init(loop, &handle);
    assert(0 == ret);

    sockaddr_in addr;
    ret = uv_ip4_addr(pt.get<std::string>("myip").c_str(), 
                      pt.get<int>("myport"), &addr);
    assert(0 == ret);
    ret = uv_tcp_bind(&handle, reinterpret_cast<sockaddr*>(&addr), 0);
    assert(0 == ret);
    ret = uv_listen(reinterpret_cast<uv_stream_t*>(&handle), SOMAXCONN, cb_uv_Accept);
    assert(0 == ret);
    LOG_INFO << "init modules...done, GO!!";

    // go!!
    ret = uv_run(loop, UV_RUN_DEFAULT);
    // never reach here, bu we log the return value
    LOG_WARN << "uv_run's ret: " << ret << ", Bye...";

    // os will do the cleanup
}
