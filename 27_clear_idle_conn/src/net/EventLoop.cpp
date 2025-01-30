#include "EventLoop.h"

#include "Channel.h"
#include "Epoll.h"
#include <cstdio>
#include <functional>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <unistd.h>

EventLoop::EventLoop() 
  : m_Ep(new Epoll), 
  m_WakeUpFd(eventfd(0, EFD_NONBLOCK)), 
  m_WakeChannel(new Channel(this, m_WakeUpFd)) 
{
  // 注册读事件和回调，如果WakeUpFd由数据，则会执行HandleWakeUp()
  m_WakeChannel->SetReadCallback(std::bind(&EventLoop::HandleWakeUp, this));
  m_WakeChannel->SetEnableReading(true);
}

EventLoop::~EventLoop() {
} 

void EventLoop::Run() {
  //printf("EventLoop::Run() on thread %ld.\n", syscall(SYS_gettid));
  m_ThreadId = syscall(SYS_gettid); // 获取事件循环所在线程的id
  
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

void EventLoop::UpdateChannel(Channel* ch) {
  m_Ep->UpdateChannel(ch);
}

void EventLoop::RemoveChannel(Channel* ch) {
  m_Ep->RemoveChannel(ch);
}

void EventLoop::SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn) {
  m_EpollTimeoutCallback = fn;
}

bool EventLoop::IsInLoopThread() {
  return m_ThreadId == syscall(SYS_gettid);
}

void EventLoop::QueueInLoop(std::function<void()> fn) {
  {
    // 任务入队
    std::lock_guard<std::mutex> lock(m_Mtx);
    m_TaskQueue.push(fn);
  }
  // 唤醒事件循环
  WakeUp();
}

void EventLoop::WakeUp() {
  // 随便写点东西，唤醒事件循环
  uint64_t val = 1;
  write(m_WakeUpFd, &val, sizeof(val));
}

void EventLoop::HandleWakeUp() {
  printf("HandleWakeUp() thread is %ld\n", syscall(SYS_gettid));
  // 读取出eventFd的值
  uint64_t val;
  read(m_WakeUpFd, &val, sizeof(val));

  std::function<void()> fn;
  // 给任务队列加锁
  std::lock_guard<std::mutex> lock(m_Mtx);

  // 执行队列中全部的发送任务
  while (m_TaskQueue.size() > 0) {
    // 出队一个
    fn = std::move(m_TaskQueue.front());
    m_TaskQueue.pop();
    // 执行任务
    fn();
  }
}