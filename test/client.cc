
#include <string>
#include <uv.h>
#include <boost/property_tree/json_parser.hpp>
#include "uvutil.h"
#include "buffer.h"
#include "handler.h"
#include "../proto/ulin.pb.h"
#include "../proto/test.pb.h"

static SendBuffer buffer; 

static void uv_cb_Write(uv_write_t *req, int status)
{
    std::cout << "Status: " << status << std::endl;
}

static void uv_cb_Connected(uv_connect_t *req, int status)
{
    if (status < 0) {
        std::cout << "Connect failed!!" << std::endl;
        return;
    }
    int ret(uv_read_start(reinterpret_cast<uv_stream_t*>(req->handle), 
                          cb_uv_AllocBuffer, 
                          cb_uv_Read));
    if (LIKELY(0 == ret)) {
        req->handle->data = nullptr;

        Test::Ping msgPing;
        msgPing.set_magic(502);
        std::string tmpBuffer(msgPing.SerializeAsString());

        ULin::Encap msgWhole;
        msgWhole.set_msg(tmpBuffer);
        msgWhole.set_name(Test::Ping::default_instance().GetTypeName());
        msgWhole.SerializeToString(&tmpBuffer);
        (void)buffer.Append(tmpBuffer.c_str(), tmpBuffer.length());

        std::size_t len;
        char *data(buffer.Product(len));
        uv_buf_t buf(uv_buf_init(data, len));
        uv_write_t lol;
        (void)uv_write(&lol, reinterpret_cast<uv_stream_t*>(req->handle), &buf, 1, uv_cb_Write);
        std::cout << "Message Ping out" << std::endl;
    }
    else {
        std::cout << "Accept failed, uv_read_start: " << ret << std::endl;
    }
}

int main()
{
    // load config 
    boost::property_tree::read_json("../config.json", pt::instance());

    // register handlers
    handler::instance().RegAllHandlers();

    // init the loop
    uv_loop_t * const loop(uv_default_loop()); 
    assert(nullptr != loop);

    // init a tcp connector
    uv_tcp_t handle;
    int ret(uv_tcp_init(loop, &handle));
    assert(0 == ret);

    sockaddr_in addr;
    ret = uv_ip4_addr(pt::instance().get<std::string>("myip").c_str(), pt::instance().get<int>("myport"), &addr);
    assert(0 == ret);
    uv_connect_t req; 
    ret = uv_tcp_connect(&req, &handle, reinterpret_cast<sockaddr*>(&addr), uv_cb_Connected);
    assert(0 == ret);

    // go!!
    ret = uv_run(loop, UV_RUN_DEFAULT);

    // os will do the cleanup
}
