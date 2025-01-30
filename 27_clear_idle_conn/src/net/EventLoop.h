#pragma once

#include "Pointer.h"
#include "Epoll.h"

#include <functional>
#include <mutex>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>

class Channel;
class Epoll;

// 事件循环类
class EventLoop
{
private:
  Scope<Epoll> m_Ep;    // 每个事件循环只有一个Epoll
  pid_t m_ThreadId;     // 事件循环所在地线程id
  std::function<void(EventLoop*)> m_EpollTimeoutCallback; // epoll_wait()超时回调
  
  std::queue<std::function<void()>> m_TaskQueue;  // 事件循环线程被eventFd唤醒后执行的任务队列
  std::mutex m_Mtx;                               // 任务队列同步的互斥锁
  
  int m_WakeUpFd;               // 用于唤醒事件循环线程的eventFd，非阻塞
  Scope<Channel> m_WakeChannel; // 用于唤醒事件循环线程的Channel
public:
  EventLoop();  // 创建m_Ep
  ~EventLoop(); // 析构函数，销毁m_Ep

  void Run();   // 在TcpServer中的线程池运行

  void UpdateChannel(Channel* ch);            // 把Channel添加/更新到红黑树上，channel中有fd和需要监视的事件
  void RemoveChannel(Channel* ch);            // 从epoll红黑树上删除Channel
  void SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn); 

  bool IsInLoopThread();                      // 判断当前线程是否为事件循环线程
  void QueueInLoop(std::function<void()> fn); // 把任务队列添加到队列中
  void WakeUp();                              // 唤醒事件循环
  void HandleWakeUp();                        // 事件循环线程被eventFd唤醒后执行的函数
};
