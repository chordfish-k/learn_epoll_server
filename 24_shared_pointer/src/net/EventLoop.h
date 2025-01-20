#pragma once

#include "Epoll.h"

#include <functional>

class Channel;
class Epoll;

// 事件循环类
class EventLoop
{
private:
  Epoll* m_Ep;   // 每个事件循环只有一个Epoll
  std::function<void(EventLoop*)> m_EpollTimeoutCallback = nullptr;
public:
  EventLoop();  // 创建m_Ep
  ~EventLoop(); // 析构函数，销毁m_Ep

  void Run();
  Epoll* GetEpoll() const;

  void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上，channel中有fd和需要监视的事件
  void SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn); 
};
