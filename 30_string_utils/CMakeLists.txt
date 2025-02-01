cmake_minimum_required(VERSION 3.28)
project(Webserver)
set(CMAKE_CXX_STANDARD 17)

set(SERVER EpollServer)
file(GLOB_RECURSE SERVER_SOURCES
  "src/epoll_server.cpp"
  "src/net/**"
  "src/util/**"
)
add_executable(${SERVER} ${SERVER_SOURCES})
target_link_libraries(${SERVER} PRIVATE pthread)

set(CLIENT Client)
file(GLOB_RECURSE CLIENT_SOURCES
  "src/client.cpp"
  "src/net/**"
  "src/util/**"
)
add_executable(${CLIENT} ${CLIENT_SOURCES})

set(TEST Test)
file(GLOB_RECURSE TEST_SOURCES
  "src/test.cpp"
  "src/net/**"
  "src/util/**"
)
add_executable(${TEST} ${TEST_SOURCES})