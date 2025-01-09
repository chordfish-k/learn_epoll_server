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

#include "net/InetAddress.h"

int main(int argc, const char** argv) {
  
  if (argc != 3) {
    printf("Usage: ./epoll_server <ip> <port>\n");
    printf("Example: ./epoll_server 127.0.0.1 5005\n\n");
    return -1;
  }

  // 创建服务端用于监听的listenFd
  int listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (listenFd < 0) { perror("socket()"); return -1; }

  // 设置listenFd的属性
  const int opt = 1;
  setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt))); // 允许重新绑定已处于 TIME_WAIT 状态的地址
  setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt))); // 允许多个进程线程绑定到同一格端口
  setsockopt(listenFd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt))); // 启用 TCP 保活机制，检测连接是否活跃
  setsockopt(listenFd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt))); // 禁用 Nagle 算法，减少延迟

  // 设置listenFd为非阻塞: 创建socket时可添加 SOCK_NONBLOCK选项，创建非阻塞socket
  // FdSetNonBlocking(listenFd);

  // 服务端地址的结构体
  // sockaddr_in serverAddr;
  // serverAddr.sin_family = AF_INET;                  // IPv4网络协议的套接字类型
  // serverAddr.sin_addr.s_addr = inet_addr(argv[1]);  // 服务端用于监听的ip地址
  // serverAddr.sin_port = htons(atoi(argv[2]));       // 服务端用于监听的端口
  InetAddress serverAddr(argv[1], atoi(argv[2]));

  // 将listenFd绑定到对应地址和端口
  if (bind(listenFd, serverAddr.GetAddr(), sizeof(sockaddr)) < 0) {
    perror("bind()"); close(listenFd); return -1;
  }

  // 将listenFd设为监听状态，再高并发网络服务器中，第二个参数(请求连接队列)大一些
  if (listen(listenFd, 128) != 0) {
    perror("listen()"); close(listenFd); return -1;
  }

  // 创建epoll句柄，epoll实例中维护一个socket的红黑树
  int epollFd = epoll_create(1);

  // 为服务端的listenFd准备读事件
  epoll_event ev;         // 声明事件的数据结构 
  ev.data.fd = listenFd;  // 指定事件的自定义数据，会随着epoll eait()返回的事件一并返回
  ev.events = EPOLLIN;    // 让epoll监视listenFd的读事件，采用水平触发

  // 把需要监视的listenFd和它的事件加入epollFd中
  epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev);

  // 存放epoll_wait()返回事件的数组
  epoll_event evs[10];

  while (true) {
    // 等待事件，-1表示无限等待
    int infds = epoll_wait(epollFd, evs, 10, -1);

    // 返回失败
    if (infds < 0) {
      perror("epoll_wait() failed"); break;
    }

    // 超时
    if (infds == 0) {
      printf("epoll_wait() timeout.\n"); continue;
    }

    // infds > 0，表示有事件发生的fd的数量
    for (int i = 0; i < infds; ++i) {
      if (evs[i].events & EPOLLRDHUP) {
        // 对方已关闭。部分系统检测不到该事件，可使用EPOLLIN, recv()返回0
        printf("Client(fd=%d) Disconnected.\n", evs[i].data.fd);
        close(evs[i].data.fd);
      }
      else if (evs[i].events & (EPOLLIN | EPOLLPRI)) {
        // 如果是listenFd有事件，表示有新的连接
        if (evs[i].data.fd == listenFd) {
          sockaddr_in peerAddr;
          socklen_t len = sizeof(peerAddr);
          int clientFd = accept4(listenFd, (sockaddr*)&peerAddr, &len, SOCK_NONBLOCK);

          InetAddress clientAddr(peerAddr); // 客户端的地址和协议

          printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", clientFd, clientAddr.GetIp(), clientAddr.GetPort());

          // 为新客户端连接准备读事件，并添加到epoll中
          ev.data.fd = clientFd;
          ev.events = EPOLLIN | EPOLLET; // 边缘触发
          epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);
        }
        else {
          // 否则就是客户端连接的fd有事件
          // 接收缓冲区中有数据可以读
          char buffer[1024];
          while (true) {
            // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
            bzero(&buffer, sizeof(buffer));
            ssize_t nread = read(evs[i].data.fd, buffer, sizeof(buffer));
            if (nread > 0) {
              // 成功读取到数据
              printf("Recv(fd=%d): %s\n", evs[i].data.fd, buffer);
              send(evs[i].data.fd, buffer, strlen(buffer), 0);
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
              printf("Client(fd=%d) Disconnected.\n", evs[i].data.fd);
              close(evs[i].data.fd);
              break;
            }
          }
        }
      }
      else if (evs[i].events & EPOLLOUT) {
        // 有数据需要写。。
      }
      else {
        // 其他事件，都视为错误
        printf("Client(fd=%d) Error.\n", evs[i].data.fd);
        close(evs[i].data.fd);
      }
    }
  }

  return 0;
}
