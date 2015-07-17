
#include "handler.h"
#include "connection.h"
#include "../proto/ulin.pb.h"
#include "../proto/test.pb.h"

void CPong(Connection *con, const std::string &msg)
{
    Test::Pong msgPong;
    if (!msgPong.ParseFromString(msg)) {
        return;
    }
    std::cout << "Message Pong in" << std::endl;

    Test::Ping msgPing;
    msgPing.set_magic(msgPong.magic());
    con->Send(msgPing);
    std::cout << "Out message Ping" << std::endl;
}

void Handler::RegAllHandlers()
{
    bool ret(RegHandler(Test::Pong::default_instance().GetTypeName(), CPong));
    assert(ret);
}
