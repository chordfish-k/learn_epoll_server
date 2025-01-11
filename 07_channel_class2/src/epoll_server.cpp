#include <cstdlib>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <vector>

#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/Epoll.h"
#include "net/Channel.h"

int main(int argc, const char** argv) {
  
  if (argc != 3) {
    printf("Usage: ./epoll_server <ip> <port>\n");
    printf("Example: ./epoll_server 127.0.0.1 5005\n\n");
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
 
  Epoll ep;
  Channel* serverChannel = new Channel(&ep, serverSocket.GetFd(), true);
  serverChannel->EnableReading();

  while (true) {
    // 获取有事件的Channel
    std::vector<Channel*> channels = ep.Loop();

    for (auto& ch : channels) {
      // 处理Channel事件
      ch->HandleEvent(&serverSocket);
    }
  }

  return 0;
}
