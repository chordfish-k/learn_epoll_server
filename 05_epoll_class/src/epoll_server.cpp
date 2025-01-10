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

  //
  // // 创建epoll句柄，epoll实例中维护一个socket的红黑树
  // int epollFd = epoll_create(1);
  //
  // // 为服务端的listenFd准备读事件
  // epoll_event ev;         // 声明事件的数据结构 
  // ev.data.fd = serverSocket.GetFd();  // 指定事件的自定义数据，会随着epoll eait()返回的事件一并返回
  // ev.events = EPOLLIN;    // 让epoll监视listenFd的读事件，采用水平触发
  //
  // // 把需要监视的listenFd和它的事件加入epollFd中
  // epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket.GetFd(), &ev);
  //
  // // 存放epoll_wait()返回事件的数组
  // epoll_event evs[10];
  
  Epoll ep;
  ep.AddFd(serverSocket.GetFd(), EPOLLIN);
  std::vector<epoll_event> evs;

  while (true) {
    // // 等待事件，-1表示无限等待
    // int infds = epoll_wait(epollFd, evs, 10, -1);
    //
    // // 返回失败
    // if (infds < 0) {
    //   perror("epoll_wait() failed"); break;
    // }
    //
    // // 超时
    // if (infds == 0) {
    //   printf("epoll_wait() timeout.\n"); continue;
    // }
    
    evs = ep.Loop();

    // infds > 0，表示有事件发生的fd的数量
    //for (int i = 0; i < infds; ++i) {
    for (auto& ev: evs) {
      if (ev.events & EPOLLRDHUP) {
        // 对方已关闭。部分系统检测不到该事件，可使用EPOLLIN, recv()返回0
        printf("Client(fd=%d) Disconnected.\n", ev.data.fd);
        close(ev.data.fd);
      }
      else if (ev.events & (EPOLLIN | EPOLLPRI)) {
        // 如果是listenFd有事件，表示有新的连接
        if (ev.data.fd == serverSocket.GetFd()) {
          InetAddress clientAddr; // 客户端的地址和协议
          // new出来的socket暂时无法释放，后面再解决
          Socket *clientSocket = new Socket(serverSocket.Accept(clientAddr));

          printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", clientSocket->GetFd(), clientAddr.GetIp(), clientAddr.GetPort());

          // 为新客户端连接准备读事件，并添加到epoll中
          // ev.data.fd = clientSocket->GetFd();
          // ev.events = EPOLLIN | EPOLLET; // 边缘触发
          // epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket->GetFd(), &ev);
          ep.AddFd(clientSocket->GetFd(), EPOLLIN | EPOLLET); // 读事件、边缘触发
        }
        else {
          // 否则就是客户端连接的fd有事件
          // 接收缓冲区中有数据可以读
          char buffer[1024];
          while (true) {
            // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
            bzero(&buffer, sizeof(buffer));
            ssize_t nread = read(ev.data.fd, buffer, sizeof(buffer));
            if (nread > 0) {
              // 成功读取到数据
              printf("Recv(fd=%d): %s\n", ev.data.fd, buffer);
              send(ev.data.fd, buffer, strlen(buffer), 0);
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
              printf("Client(fd=%d) Disconnected.\n", ev.data.fd);
              close(ev.data.fd);
              break;
            }
          }
        }
      }
      else if (ev.events & EPOLLOUT) {
        // 有数据需要写。。
      }
      else {
        // 其他事件，都视为错误
        printf("Client(fd=%d) Error.\n", ev.data.fd);
        close(ev.data.fd);
      }
    }
  }

  return 0;
}
