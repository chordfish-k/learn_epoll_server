#pragma once

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

  void AddFd(int fd, uint32_t op);                  // 把要监视的fd和事件添加到epoll红黑树
  std::vector<epoll_event> Loop(int timeout = -1);  // 运行epoll_wait(),等待事件发生，返回已发生的事件
};
