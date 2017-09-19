# http-rpc
The RPC Library Based On HTTP Procotol


## Dependent
* VS 2017 or gcc 7.0
* protobuf

## Build
1. `git submodule init`
2. `git submodule update`
3. `mkdir build` & cd `build`
4. `cmake -G "Visual Studio 15 Win64" ..`


## Run Example
1. run echo service of cmd: `echo-service 0.0.0.0 8080`
2. use `curl -v -GET http://localhost:8080/sofa.pbrpc.test.EchoServer.Echo` for test in console.
   Or open your browser, enter URL: `http://localhost:8080/sofa.pbrpc.test.EchoServer.Echo`

## Usage(Add New Service)
1. edit your service proto file, e.g `Test.proto `
2. then use cmd `protoc --cpp_out=proto_dir Test.proto` generate cpp code.
  (protoc muse use `3rdparty\protobuf\bin`)
3. add cpp file of step 2 to project
4. new cpp file, e.g `MyTest.h`, defile `MyTestService`, for implement `TestServer` interface.
4. add `rpcServiceMgr->registerService(std::make_shared<MyTestService>())` for register service.
5. rebuild project

## 使用方式(添加新RPC服务)
1. 编辑新RPC服务的.proto文件,比如文件名称为`Test.proto`
2. 使用`3rdparty\protobuf\bin`目录下的 `protoc`生成其对应的C++代码文件
3. 将生成的代码加入到工程
4. 编写服务的实现文件,比如名称为 `MyTest.h`,并定义`MyTestService`类来实现 `TestServer`.
5. 在 main.cpp中通过`rpcServiceMgr->registerService(std::make_shared<MyTestService>())`注册新的服务
6. 重新编译工程即可完成添加新的服务(然后重启服务,即可通过浏览器或`curl`进行测试) 