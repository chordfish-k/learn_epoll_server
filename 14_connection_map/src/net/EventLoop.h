#pragma once

#include "Epoll.h"

// 事件循环类
class EventLoop
{
private:
  Epoll* m_Ep;   // 每个事件循环只有一个Epoll
public:
  EventLoop();  // 创建m_Ep
  ~EventLoop(); // 析构函数，销毁m_Ep

  void Run();
  Epoll* GetEpoll() const;
};
