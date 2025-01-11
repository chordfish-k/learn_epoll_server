#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/EventLoop.h"

int main(int argc, const char** argv) {
  
  if (argc != 3) {
    printf("Usage: ./EpollServer <ip> <port>\n");
    printf("Example: ./EpollServer 127.0.0.1 5005\n\n");
    return -1;
  }

  // 服务端socket
  Socket serverSocket;
  // 服务端的地址和协议
  InetAddress serverAddr(argv[1], atoi(argv[2]));
  serverSocket.SetReuseAddr(true);
  serverSocket.SetReusePort(true);
  serverSocket.SetKeepAlive(true);
  serverSocket.SetTcpNoDelay(true);
  
  // 绑定并监听
  serverSocket.Bind(serverAddr);
  serverSocket.Listen();
 
  // Epoll 事件循环
  EventLoop loop;
  Channel* serverChannel = new Channel(loop.GetEpoll(), serverSocket.GetFd());
  serverChannel->SetReadCallback(std::bind(&Channel::OnNewConnection, serverChannel, &serverSocket));
  serverChannel->EnableReading();

  loop.Run();

  return 0;
}
