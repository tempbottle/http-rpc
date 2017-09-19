#ifndef _HTTP_RPC_H
#define _HTTP_RPC_H

#include <string>

#include <brynet/net/SocketLibFunction.h>
#include <brynet/net/http/HttpService.h>
#include <brynet/net/http/HttpFormat.h>
#include <brynet/net/http/WebSocketFormat.h>  
#include <brynet/net/WrapTCPService.h>
#include <brynet/utils/systemlib.h>

#include "google/protobuf/service.h"
#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"

namespace httprpc
{
    using namespace brynet;
    using namespace brynet::net;

    class TypeService
    {
    public:
        typedef std::shared_ptr<TypeService>    PTR;

        TypeService(std::shared_ptr<google::protobuf::Service> service) : mService(service)
        {}
        virtual ~TypeService()
        {}

        std::shared_ptr<google::protobuf::Service>  getService()
        {
            return  mService;
        }

        void    registerMethod(const ::google::protobuf::MethodDescriptor* desc)
        {
            mMethods[desc->name()] = desc;
        }

        const ::google::protobuf::MethodDescriptor* findMethod(std::string name)
        {
            auto it = mMethods.find(name);
            if (it != mMethods.end())
            {
                return (*it).second;
            }

            return nullptr;
        }

    private:
        std::shared_ptr<google::protobuf::Service>  mService;
        std::unordered_map<std::string, const ::google::protobuf::MethodDescriptor*>   mMethods;
    };

    class RPCServiceMgr
    {
    public:
        typedef std::shared_ptr<RPCServiceMgr> PTR;

        virtual ~RPCServiceMgr()
        {}

        bool    registerService(std::shared_ptr<google::protobuf::Service> service)
        {
            auto typeService = std::make_shared<TypeService>(service);

            auto serviceDesc = service->GetDescriptor();
            for (auto i = 0; i < serviceDesc->method_count(); i++)
            {
                auto methodDesc = serviceDesc->method(i);
                typeService->registerMethod(methodDesc);
            }

            mServices[service->GetDescriptor()->full_name()] = typeService;

            return true;
        }

        TypeService::PTR    findService(const std::string& name)
        {
            auto it = mServices.find(name);
            if (it != mServices.end())
            {
                return (*it).second;
            }

            return nullptr;
        }

    private:
        std::unordered_map<std::string, TypeService::PTR>   mServices;
    };

    class HttpRpcClosure : public ::google::protobuf::Closure
    {
    public:
        HttpRpcClosure(const std::function<void(void)>& callback) : mCallback(callback), mValid(true)
        {}

    private:
        void    Run() override final
        {
            assert(mValid);
            assert(mCallback);

            //避免多次调用Run函数
            if (mValid && mCallback)
            {
                mCallback();
                mValid = false;
            }
        }

    private:
        std::function<void(void)>   mCallback;
        bool                        mValid;
    };

    static bool ProcessHTTPRPCRequest(const RPCServiceMgr::PTR& rpcServiceMgr,
        const HttpSession::PTR& session,
        const std::string & serviceName,
        const std::string& methodName,
        const std::string& body)
    {
        auto typeService = rpcServiceMgr->findService(serviceName);
        if (typeService == nullptr)
        {
            return false;
        }

        auto method = typeService->findMethod(methodName);
        if (method == nullptr)
        {
            return false;
        }

        auto requestMsg = typeService->getService()->GetRequestPrototype(method).New();
        requestMsg->ParseFromArray(body.c_str(), body.size());

        auto responseMsg = typeService->getService()->GetResponsePrototype(method).New();

        auto clouse = new HttpRpcClosure([=]() {
            //将结果通过HTTP协议返回给客户端
            //TODO::delete clouse

            std::string jsonMsg;
            google::protobuf::util::MessageToJsonString(*responseMsg, &jsonMsg);

            HttpResponse httpResponse;
            httpResponse.setStatus(HttpResponse::HTTP_RESPONSE_STATUS::OK);
            httpResponse.setContentType("application/json");
            httpResponse.setBody(jsonMsg.c_str());

            auto result = httpResponse.getResult();
            session->send(result.c_str(), result.size(), nullptr);

            /*  TODO::不断开连接,由可信客户端主动断开,或者底层开始心跳检测(断开恶意连接) */
            session->postShutdown();

            delete requestMsg;
            delete responseMsg;
        });

        //TODO::support controller(超时以及错误处理)
        typeService->getService()->CallMethod(method, nullptr, requestMsg, responseMsg, clouse);
        return true;
    }

    static void StartHttpRpc(const ListenThread::PTR& listenThread,
        const std::string& ip,
        int port,
        const WrapTcpService::PTR& tcpService,
        const RPCServiceMgr::PTR& rpcServiceMgr,
        const std::chrono::milliseconds& timeout)
    {
        listenThread->startListen(false, ip, port, [tcpService, rpcServiceMgr, timeout](sock fd) {
            tcpService->addSession(fd, [rpcServiceMgr, timeout](const TCPSession::PTR& session) {
                session->setPingCheckTime(timeout);

                HttpService::setup(session, [rpcServiceMgr](const HttpSession::PTR& httpSession) {
                    httpSession->setHttpCallback([=](const HTTPParser& httpParser, HttpSession::PTR session) {
                        auto queryPath = httpParser.getPath();
                        std::string::size_type pos = queryPath.rfind('.');
                        if (pos == std::string::npos)
                        {
                            std::cerr << "your http request has wrong" << std::endl;
                            return;
                        }

                        auto serviceName = queryPath.substr(1, pos - 1);
                        auto methodName = queryPath.substr(pos + 1);
                        
                        bool ret = ProcessHTTPRPCRequest(rpcServiceMgr, session, serviceName, methodName, httpParser.getBody());
                        if (!ret)
                        {
                            std::cerr << "process http-rpc has wrong of query path:" << queryPath << std::endl;
                            return;
                        }
                    });

                    httpSession->setWSCallback([=](HttpSession::PTR session, WebSocketFormat::WebSocketFrameType opcode, const std::string& payload) {
                        //TODO::support websocket协议,payload和二进制协议一样
                    });
                });
            }, false, nullptr, 1024 * 1024);
        });
    }
}


#endif