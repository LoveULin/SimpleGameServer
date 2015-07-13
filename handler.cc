
#include "handler.h"
#include "connection.h"
#include "proto/test.pb.h"
#include "timer.h"
#include "circle.h"

void CPing(Connection *con, const std::string &msg)
{
    std::cout << "LOL!!" << std::endl;
    Test::Ping msgPing;
    if (!msgPing.ParseFromString(msg)) {
        return;
    }
    Test::Pong msgPong;
    msgPong.set_magic(msgPing.magic());
    (void)con->Send(msgPong);
}

void Handler::RegAllHandlers()
{
    bool ret(RegHandler(Test::Ping::default_instance().GetTypeName(), CPing));
    assert(ret);
}
