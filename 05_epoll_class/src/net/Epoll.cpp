#include "Epoll.h"

#include <cerrno>
#include <cstdlib>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>

Epoll::Epoll() {
  if ((m_EpollFd = epoll_create(1)) == -1) {
    printf("epoll_create() failed(%d).\n", errno); exit(-1);
  } 
}  

Epoll::~Epoll() {
  close(m_EpollFd);
}

void Epoll::AddFd(int fd, uint32_t op) {
  epoll_event ev;         // 声明事件的数据结构 
  ev.data.fd = fd;        // 指定事件的自定义数据，会随着epoll eait()返回的事件一并返回
  ev.events = op;         // 让epoll监视listenFd的读事件，采用水平触发

  // 把需要监视的listenFd和它的事件加入epollFd中
  if (epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    printf("epoll_ctl() failed(%d).\n", errno); exit(-1);
  }
}                
  
std::vector<epoll_event> Epoll::Loop(int timeout) {
  // 存放epoll_wait()返回的事件
  std::vector<epoll_event> evs;
  // 初始化
  bzero(m_Events, sizeof(m_Events));

  // 等待事件，-1表示无限等待
  int infds = epoll_wait(m_EpollFd, m_Events, s_MaxEvents, timeout);

  // 返回失败
  if (infds < 0) {
    perror("epoll_wait() failed"); exit(-1);
  }

  // 超时
  if (infds == 0) {
    printf("epoll_wait() timeout.\n"); return evs;
  }

  // infds > 0，表示有事件发生的fd的数量
  // 把m_Events的内容填充到evs
  for (int i = 0; i < infds; ++i) {
    evs.push_back(m_Events[i]);
  }

  return evs;
}
