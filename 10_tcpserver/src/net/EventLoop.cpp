#include "EventLoop.h"

#include "Epoll.h"

EventLoop::EventLoop() 
  : m_Ep(new Epoll) {

}

EventLoop::~EventLoop() {
  delete m_Ep;
} 

void EventLoop::Run() {
  while (true) {
    // 获取有事件的Channel
    std::vector<Channel*> channels = m_Ep->Loop();

    for (auto& ch : channels) {
      // 处理Channel事件
      ch->HandleEvent();
    }
  }
}

Epoll* EventLoop::GetEpoll() const {
  return m_Ep;
}
