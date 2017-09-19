cmake_minimum_required (VERSION 2.6)
project(http_rpc)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MTd")
elseif(UNIX)
  if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  endif()
endif()

set(net_src_list
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/EventLoop.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/CurrentThread.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/DataSocket.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/SocketLibFunction.c
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/TCPService.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/WrapTCPService.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/Connector.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/ListenThread.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/fdset.c
)

set(timer_src_list
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/timer/Timer.cpp
)

set(base_src_list
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/array.c
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/buffer.c
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/systemlib.c
)

set(http_src_list
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/http/http_parser.c
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/http/HttpParser.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/net/http/HttpService.cpp
)

set(utils_src_list
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/base64.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/md5calc.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/SHA1.cpp
  ${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/brynet/utils/app_status.c
)

include_directories("${PROJECT_SOURCE_DIR}/3rdparty/brynet/src/")
include_directories("${PROJECT_SOURCE_DIR}/3rdparty/protobuf/include/")
include_directories("${PROJECT_SOURCE_DIR}/src/http-rpc/")
LINK_DIRECTORIES("${PROJECT_SOURCE_DIR}/3rdparty/protobuf/lib/")

add_executable(echo-service ${net_src_list} 
  ${timer_src_list} 
  ${base_src_list} 
  ${utils_src_list} 
  ${http_src_list} 
  ${PROJECT_SOURCE_DIR}/examples/echo/echo_service.pb.cc
  ${PROJECT_SOURCE_DIR}/examples/echo/echo-service.cpp)
if(WIN32)
  target_link_libraries(echo-service ws2_32)
  target_link_libraries(echo-service libprotobufd)
elseif(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(echo-service pthread)
endif()