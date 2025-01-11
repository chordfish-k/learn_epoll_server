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
  Channel* serverChannel = new Channel(&ep, serverSocket.GetFd());
  serverChannel->EnableReading();

  while (true) {
   
    std::vector<Channel*> channels = ep.Loop();

    for (auto& ch : channels) {
      if (ch->GetREvents() & EPOLLRDHUP) {
        // 对方已关闭。部分系统检测不到该事件，可使用EPOLLIN, recv()返回0
        printf("Client(fd=%d) Disconnected.\n", ch->GetFd());
        close(ch->GetFd());
      }
      else if (ch->GetREvents() & (EPOLLIN | EPOLLPRI)) {
        // 如果是listenFd有事件，表示有新的连接
        if (ch == serverChannel) {
          InetAddress clientAddr; // 客户端的地址和协议
          // new出来的socket暂时无法释放，后面再解决
          Socket *clientSocket = new Socket(serverSocket.Accept(clientAddr));

          printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", clientSocket->GetFd(), clientAddr.GetIp(), clientAddr.GetPort());

          // 为新客户端连接准备读事件，并添加到epoll中
          // ep.AddFd(clientSocket->GetFd(), EPOLLIN | EPOLLET); // 读事件、边缘触发
          Channel* clientChannel = new Channel(&ep, clientSocket->GetFd());
          clientChannel->UseET();         // 使用边缘触发
          clientChannel->EnableReading(); // 将Channel添加到Epoll
        }
        else {
          // 否则就是客户端连接的fd有事件
          // 接收缓冲区中有数据可以读
          char buffer[1024];
          while (true) {
            // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
            bzero(&buffer, sizeof(buffer));
            ssize_t nread = read(ch->GetFd(), buffer, sizeof(buffer));
            if (nread > 0) {
              // 成功读取到数据
              printf("Recv(fd=%d): %s\n", ch->GetFd(), buffer);
              send(ch->GetFd(), buffer, strlen(buffer), 0);
            }
            else if (nread == -1 && errno == EINTR) {
              // 读取数据的时候被信号中断，继续读取
              continue;
            }
            else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
              // 全部的数据已经读取完毕
              break;
            }
            else if (nread == 0) {
              printf("Client(fd=%d) Disconnected.\n", ch->GetFd());
              close(ch->GetFd());
              break;
            }
          }
        }
      }
      else if (ch->GetREvents() & EPOLLOUT) {
        // 有数据需要写。。
      }
      else {
        // 其他事件，都视为错误
        printf("Client(fd=%d) Error.\n", ch->GetFd());
        close(ch->GetFd());
      }
    }
  }

  return 0;
}
