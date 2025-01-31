#pragma once

#include "Connection.h"
#include "Pointer.h"
#include "Epoll.h"

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>

class Channel;
class Epoll;
class Connection;

// 事件循环类
class EventLoop
{
private:
  Scope<Epoll> m_Ep;    // 每个事件循环只有一个Epoll
  pid_t m_ThreadId;     // 事件循环所在地线程id
  std::function<void(EventLoop*)> m_EpollTimeoutCallback; // epoll_wait()超时回调
  
  std::queue<std::function<void()>> m_TaskQueue;  // 事件循环线程被eventFd唤醒后执行的任务队列
  std::mutex m_Mtx;                               // 任务队列同步的互斥锁
  
  int m_WakeUpFd;                 // 用于唤醒事件循环线程的eventFd，非阻塞
  Scope<Channel> m_WakeChannel;   // 用于唤醒事件循环线程的Channel

  int m_TimerFd;                  // 用于计时器的TimerFd，
  Scope<Channel> m_TimerChannel;  // 用于计时器的Channel

  bool m_IsMainLoop;              // 是否是主事件循环

  std::map<int, Ref<Connection>> m_Conns;   // fd->conn, 存放运行在该事件循环上全部的Connection
  std::mutex m_ConnsMtx;                    // 保护m_Conns的互斥锁
  std::function<void(int)> m_TimerCallback; // 删除TcpServer中超时的Connection对象，将被设置为TcpServer::RemoveConnection
  int m_TimerInterval;
  int m_TimerTimeout;

  std::atomic_bool m_Stop;  // 停止事件循环的标志位

public:
  EventLoop(bool isMainLoop, int timerInterval = 30, int timerTimerout = 80);  // 创建m_Ep
  ~EventLoop(); // 析构函数，销毁m_Ep

  void Run();   // 在TcpServer中的线程池运行
  void Stop();  // 停止事件循环

  void UpdateChannel(Channel* ch);            // 把Channel添加/更新到红黑树上，channel中有fd和需要监视的事件
  void RemoveChannel(Channel* ch);            // 从epoll红黑树上删除Channel
  void SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn); 

  bool IsInLoopThread();                      // 判断当前线程是否为事件循环线程
  void QueueInLoop(std::function<void()> fn); // 把任务队列添加到队列中
  void WakeUp();                              // 唤醒事件循环
  void HandleWakeUp();                        // 事件循环线程被eventFd唤醒后执行的函数
  void HandleTimer();                         // 处理定时器时间到

  void OnNewConnection(Ref<Connection> conn);   // 把Connection对象保存中m_Conns中

  void SetTimerCallback(std::function<void(int)> fn);
};
