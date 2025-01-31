#include "net/EchoServer.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <signal.h>

// 1. 设置SIGINT和SIGTERM的信号
// 2. 在信号处理函数中停止主从事件循环和工作线程
// 3. 服务程序主动退出

EchoServer *echoServer = nullptr;

void Stop(int sig) {
  //printf("sig=%d\n", sig);
  echoServer->Stop();
  //printf("Server Stopped!\n");
  delete echoServer;
  printf("Server Exit!\n");
  exit(0);
}

int main(int argc, const char** argv) {
  
  if (argc != 3) {
    printf("Usage: ./EpollServer <ip> <port>\n");
    printf("Example: ./EpollServer 127.0.0.1 5005\n\n");
    return -1;
  }

  signal(SIGTERM, Stop);
  signal(SIGINT, Stop);
  
  echoServer = new EchoServer(argv[1], atoi(argv[2]), 7, 0);
  echoServer->Start();
  
  return 0;
}
