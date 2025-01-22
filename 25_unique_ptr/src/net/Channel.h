#pragma once

#include <stdint.h>
#include <functional>

class EventLoop;

// Channel管道，包含socketFd + Events + EventLoop(所在的Epoll)
class Channel
{
private:
  int m_Fd = -1;                          // Channel的fd，一个Channel对应一个fd 
  EventLoop *m_Loop = nullptr;            // Channel所在的红黑树
  bool m_InEpoll = false;                 // Channel是否已添加到epoll红黑树上
  uint32_t m_Events = 0;                  // m_Fd需要监视的事件，如EPOLLIN
  uint32_t m_REvents = 0;                 // 已发生的事件
  
  std::function<void()> m_ReadCallback = nullptr;    // m_Fd读事件的回调函数
  std::function<void()> m_WriteCallback = nullptr;    // m_Fd写事件的回调函数
  std::function<void()> m_CloseCallback = nullptr;   // m_Fd关闭的回调函数
  std::function<void()> m_ErrorCallback = nullptr;   // m_Fd发生错误的回调函数
public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void UseEdgeTrigger();                // 采用边缘触发
  void SetEnableReading(bool enable);   // epoll_wait()监听此fd读事件
  void SetEnableWriting(bool enable);   // epoll_wait()监听此fd写事件
  void DisableAllEvent();               // 取消所有事件
  void Remove();                        // 从事件循环中删除该Channel
  void SetInEpoll();                    // 把m_InEpoll设为true
  void SetREvents(uint32_t ev);

  int GetFd() const;
  bool IsInEpoll() const;
  uint32_t GetEvents() const;
  uint32_t GetREvents() const;

  void HandleEvent();

  void SetReadCallback(std::function<void()> fn); // 设置m_Fd读事件的回调函数
  void SetWriteCallback(std::function<void()> fn); // 设置m_Fd写事件的回调函数
  void SetCloseCallback(std::function<void()> fn); // 设置m_Fd关闭的回调函数
  void SetErrorCallback(std::function<void()> fn); // 设置m_Fd发生错误的回调函数
};
