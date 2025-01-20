#pragma once

#include "Channel.h"

#include <sys/epoll.h>
#include <vector>

class Epoll
{
private:
  static const int s_MaxEvents = 100; // epoll_wait()返回事件数组的大小
  int m_EpollFd = -1;                 // epoll句柄，再构造函数中创建
  epoll_event m_Events[s_MaxEvents];  // 存放epoll_wait()返回事件的数组
public:
  Epoll();
  ~Epoll();

  void UpdateChannel(Channel* ch);               // 把channel添加/更新到红黑树上，channel中有fd和需要监视的事件
  std::vector<Channel*> Loop(int timeout = -1);  // 运行epoll_wait(),等待事件发生，返回已发生的事件
};
