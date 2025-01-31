#include "EventLoop.h"

#include "Channel.h"

#include <cstdio>
#include <cstring>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <vector>

namespace Util {

int CreateTimerFd(int sec = 30) {
  int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK); // 创建timerFd
  itimerspec timeout;
  memset(&timeout, 0, sizeof(itimerspec));
  timeout.it_value.tv_sec = sec;
  timeout.it_interval.tv_nsec = 0;
  timerfd_settime(tfd, 0, &timeout, 0);
  return tfd;
} 

};

EventLoop::EventLoop(bool isMainLoop, int timerInterval, int timerTimerout) : 
  m_Ep(new Epoll), 
  m_IsMainLoop(isMainLoop),
  m_TimerInterval(timerInterval),
  m_TimerTimeout(timerTimerout),
  m_WakeUpFd(eventfd(0, EFD_NONBLOCK)), 
  m_WakeChannel(new Channel(this, m_WakeUpFd)),
  m_TimerFd(Util::CreateTimerFd(timerInterval)),
  m_TimerChannel(new Channel(this, m_TimerFd))
{
  // 注册读事件和回调，如果WakeUpFd由数据，则会执行HandleWakeUp()
  m_WakeChannel->SetReadCallback(std::bind(&EventLoop::HandleWakeUp, this));
  m_WakeChannel->SetEnableReading(true);

  // 定时器事件的注册
  m_TimerChannel->SetReadCallback(std::bind(&EventLoop::HandleTimer, this));
  m_TimerChannel->SetEnableReading(true);
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
  // printf("HandleWakeUp() thread is %ld\n", syscall(SYS_gettid));
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

void EventLoop::HandleTimer() {
  // 重新定时
  itimerspec timeout;
  memset(&timeout, 0, sizeof(itimerspec));
  timeout.it_value.tv_sec = m_TimerInterval;  // 定时时间
  timeout.it_interval.tv_nsec = 0;
  timerfd_settime(m_TimerFd, 0, &timeout, 0);

  if (m_IsMainLoop) {

  }
  else {
    printf("EventLoop::HandleTimer() thread is %ld. fd", syscall(SYS_gettid));
    time_t now = time(0);
    std::vector<int> readyToErase;
    // 统计空闲的连接
    for (auto connPair : m_Conns) {
      printf(" %d", connPair.first);
      if (connPair.second->IsTimeout(m_TimerTimeout, now)) {
        readyToErase.push_back(connPair.first);
      }
    }
    printf("\n");

    // 清除空闲的连接
    for (auto fd : readyToErase) {
      {
        std::lock_guard<std::mutex> lock(m_ConnsMtx);
        m_Conns.erase(fd);
      }
      m_TimerCallback(fd);
    }
  }
}

void EventLoop::OnNewConnection(Ref<Connection> conn) {
  std::lock_guard<std::mutex> lock(m_ConnsMtx);
  m_Conns[conn->GetFd()] = conn;
}

void EventLoop::SetTimerCallback(std::function<void(int)> fn) {
  m_TimerCallback = fn;
}