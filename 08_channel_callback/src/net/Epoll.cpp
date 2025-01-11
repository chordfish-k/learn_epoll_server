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

// void Epoll::AddFd(int fd, uint32_t op) {
//   epoll_event ev;         // 声明事件的数据结构 
//   ev.data.fd = fd;        // 指定事件的自定义数据，会随着epoll eait()返回的事件一并返回
//   ev.events = op;         // 让epoll监视listenFd的读事件，采用水平触发
//
//   // 把需要监视的listenFd和它的事件加入epollFd中
//   if (epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
//     printf("epoll_ctl() failed(%d).\n", errno); exit(-1);
//   }
// }                

void Epoll::UpdateChannel(Channel* ch) {
  epoll_event ev;
  ev.data.ptr = ch;             // Channel的指针
  ev.events = ch->GetEvents();  // 根据channel的设置，指定事件

  if (ch->IsInEpoll()) {
    // 如果channel已经在树上，则修改(EPOLL_CTL_MOD)
    if (epoll_ctl(m_EpollFd, EPOLL_CTL_MOD, ch->GetFd(), &ev) == -1) {
      perror("epoll_ctl() failed"); exit(-1);
    }
  }
  else {
    // 如果不在树上
    if (epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, ch->GetFd(), &ev) == -1) {
      perror("epoll_ctl() failed"); exit(-1);
    }
    ch->SetInEpoll(); // 告知Channel，fd已经在epoll树上
  }
}

std::vector<Channel*> Epoll::Loop(int timeout) {
  // 存放epoll_wait()返回的事件
  std::vector<Channel*> channels;
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
    printf("epoll_wait() timeout.\n"); return channels;
  }

  // infds > 0，表示有事件发生的Channel的数量
  // 填充Channel
  for (int i = 0; i < infds; ++i) {
    Channel* ch = (Channel*)m_Events[i].data.ptr;   // 去除已发生事件的Channel
    ch->SetREvents(m_Events[i].events);             // 设置Channel的m_REvent
    channels.push_back(ch);
  }

  return channels;
}
