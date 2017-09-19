#ifndef _MY_ECHO_SERVICE_H
#define _MY_ECHO_SERVICE_H

#include <string>
#include "echo_service.pb.h"

//  实现Echo服务
class MyEchoService : public sofa::pbrpc::test::EchoServer
{
public:
    MyEchoService() : mIncID(0)
    {
    }

private:
    void Echo(::google::protobuf::RpcController* controller,
        const ::sofa::pbrpc::test::EchoRequest* request,
        ::sofa::pbrpc::test::EchoResponse* response,
        ::google::protobuf::Closure* done) override
    {
        mIncID++;
        response->set_message(std::to_string(mIncID) + " is the current id");
        done->Run();
    }

private:
    int     mIncID;
};

#endif