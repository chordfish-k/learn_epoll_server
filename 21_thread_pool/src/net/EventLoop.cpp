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
    std::vector<Channel*> channels = m_Ep->Loop(10 * 1000);

    // 如果channels为空，表示超时，回调TcpServer::OnEpollTimeout
    if (channels.size() == 0) {
      if (m_EpollTimeoutCallback)
        m_EpollTimeoutCallback(this);
    }
    else {
      for (auto& ch : channels) {
        // 处理Channel事件
        ch->HandleEvent();
      }
    }
  }
}

Epoll* EventLoop::GetEpoll() const {
  return m_Ep;
}

void EventLoop::UpdateChannel(Channel* ch) {
  m_Ep->UpdateChannel(ch);
}

void EventLoop::SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn) {
  m_EpollTimeoutCallback = fn;
}