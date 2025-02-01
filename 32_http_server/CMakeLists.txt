cmake_minimum_required(VERSION 3.28)
project(Webserver)
set(CMAKE_CXX_STANDARD 17)

set(SERVER Server)
file(GLOB_RECURSE SERVER_SOURCES
  "bin/server.cpp"
  "src/**.h"
  "src/**.cpp"
)
add_executable(${SERVER} ${SERVER_SOURCES})
target_include_directories(${SERVER} PRIVATE src)
target_link_libraries(${SERVER} PRIVATE pthread)

set(CLIENT Client)
file(GLOB_RECURSE CLIENT_SOURCES
  "bin/client.cpp"
  "src/**.h"
  "src/**.cpp"
)
add_executable(${CLIENT} ${CLIENT_SOURCES})
target_include_directories(${CLIENT} PRIVATE src)

set(TEST Test)
file(GLOB_RECURSE TEST_SOURCES
  "bin/test.cpp"
  "src/**.h"
  "src/**.cpp"
)
add_executable(${TEST} ${TEST_SOURCES})
target_include_directories(${TEST} PRIVATE src)