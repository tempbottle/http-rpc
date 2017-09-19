#include <iostream>

#include <brynet/utils/app_status.h>
#include "HttpRpc.h"
#include "MyEchoService.h"

using namespace httprpc;
using namespace brynet;
using namespace brynet::net;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <listen addr> <listen port>\n");
        exit(-1);
    }

    auto tcpService = std::make_shared<WrapTcpService>();
    tcpService->startWorkThread(ox_getcpunum());
    auto listenThread = ListenThread::Create();

    RPCServiceMgr::PTR rpcServiceMgr = std::make_shared<RPCServiceMgr>();

    // register user service
    rpcServiceMgr->registerService(std::make_shared<MyEchoService>());
    // end register user service

    // alive time is 10s
    StartHttpRpc(listenThread,
        argv[1],
        atoi(argv[2]),
        tcpService,
        rpcServiceMgr,
        std::chrono::seconds(10));

    while (true)
    {
        if (app_kbhit())
        {
            std::string input;
            std::getline(std::cin, input);

            if (input == "quit")
            {
                std::cerr << "You enter quit, so will exit rpc service" << std::endl;
                break;
            }
        }
    }

    listenThread->closeListenThread();
    tcpService->stopWorkThread();

    return 0;
}
