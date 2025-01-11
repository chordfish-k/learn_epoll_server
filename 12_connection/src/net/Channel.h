#pragma once

#include "Socket.h"

#include <stdint.h>
#include <functional>

class EventLoop;

// Channel管道，包含socketFd + Events
class Channel
{
private:
  int m_Fd = -1;                          // Channel的fd，一个Channel对应一个fd 
  EventLoop *m_Loop = nullptr;                  // Channel所在的红黑树
  bool m_InEpoll = false;                 // Channel是否已添加到epoll红黑树上
  uint32_t m_Events = 0;                  // m_Fd需要监视的事件，如EPOLLIN
  uint32_t m_REvents = 0;                 // 已发生的事件
  std::function<void()> m_ReadCallback = nullptr;   // m_Fd读事件的回调函数
public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void UseET();           // 采用边缘触发
  void EnableReading();   // 让epoll_wait()监听读事件
  void SetInEpoll();      // 把m_InEpoll设为true
  void SetREvents(uint32_t ev);

  int GetFd() const;
  bool IsInEpoll() const;
  uint32_t GetEvents() const;
  uint32_t GetREvents() const;

  void HandleEvent();

  void OnNewConnection(Socket* serverSocket);     // 处理新客户端的连接请求
  void OnMessage();                               // 处理对端发送过来的消息
  void SetReadCallback(std::function<void()> fn); // 设置m_Fd读事件的回调函数
};
