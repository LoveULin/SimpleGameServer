
#include "handler.h"
#include "buffer.h"
#include "../proto/ulin.pb.h"
#include "../proto/test.pb.h"

void CPong(Connection *con, const std::string &msg)
{
    Test::Pong msgPong;
    if (!msgPong.ParseFromString(msg)) {
        return;
    }
    std::cout << "Message Pong in" << std::endl;
#if 0
    Test::Ping msgPing;
    msgPing.set_magic(msgPong.magic());

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
    std::cout << "Out message Ping" << std::endl;
#endif
}

void Handler::RegAllHandlers()
{
    bool ret(RegHandler(Test::Pong::default_instance().GetTypeName(), CPong));
    assert(ret);
}
