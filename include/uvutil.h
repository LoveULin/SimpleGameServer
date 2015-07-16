
#include <uv.h>
#include <boost/pool/singleton_pool.hpp>
#include "logger.h"
#include "connection.h"

constexpr int uvBufferUnit = 256;

typedef struct {} pool_tag;
typedef boost::singleton_pool<pool_tag, (uvBufferUnit * sizeof(char))> spl;
typedef struct {} client_pool_tag;
typedef boost::singleton_pool<client_pool_tag, sizeof(uv_tcp_t)> clientSPL;

void cb_uv_Close(uv_handle_t *handle)
{
    LOG_NOTIFY << "Close a connection";
    // delete our Connection
    delete static_cast<Connection*>(handle->data);
    handle->data = nullptr;
    void * const userData(handle->loop->data);
    if (nullptr != userData) {
        // reset the loop's data: uv_async_t(for others)
        static_cast<uv_async_t*>(userData)->data = nullptr;
    }
    // free handle of client
    if (clientSPL::is_from(handle)) {
        clientSPL::free(handle);
    }
}

void cb_uv_AllocBuffer(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf)
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

void cb_uv_Read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        // error condition
        LOG_ERROR << "Read error, nread: " << nread;
        uv_read_stop(stream);
        uv_close(reinterpret_cast<uv_handle_t*>(stream), cb_uv_Close);
    }
    else if (nread > 0) {
        static_cast<Connection*>(stream->data)->DealBuffer(buf->base, nread);
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

void cb_uv_Accept(uv_stream_t *req, int status)
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

void cb_uv_HandleSignal(uv_signal_t *handle, int signum)
{
    // :D
    assert(handle->signum == signum);
    LOG_INFO << "Handle SIGINT";

    // do nothing, but exit immediately
    quick_exit(EXIT_SUCCESS);
}

void cb_uv_Async(uv_async_t *handle)
{
    Connection * const conn(static_cast<Connection*>(handle->data));
    if (nullptr == conn) {
        return;
    }
    // conn->Flush();
}

