cmake_minimum_required(VERSION 3.28)
project(Webserver)
set(CMAKE_CXX_STANDARD 17)

set(SERVER EpollServer)
file(GLOB_RECURSE SERVER_SOURCES
  "src/epoll_server.cpp"
)
add_executable(${SERVER} ${SERVER_SOURCES})

set(CLIENT Client)
file(GLOB_RECURSE CLIENT_SOURCES
  "src/client.cpp"
)
add_executable(${CLIENT} ${CLIENT_SOURCES})
