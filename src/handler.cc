
#include "handler.h"
#include "connection.h"
#include "proto/test.pb.h"
#include "logger.h"
#include "timer.h"
#include "circle.h"

void CPing(Connection *con, const std::string &msg)
{
    Test::Ping msgPing;
    if (!msgPing.ParseFromString(msg)) {
        return;
    }
    LOG_INFO << "Message Ping in";
    Test::Pong msgPong;
    msgPong.set_magic(msgPing.magic());
    (void)con->Send(msgPong);
    LOG_INFO << "Out message Pong";
}

void Handler::RegAllHandlers()
{
    bool ret(RegHandler(Test::Ping::default_instance().GetTypeName(), CPing));
    assert(ret);
}
