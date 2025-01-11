#pragma once

#include "Socket.h"

#include <stdint.h>

class Epoll;

// Channel管道，包含socketFd + Events
class Channel
{
private:
  int m_Fd = -1;                  // Channel的fd，一个Channel对应一个fd 
  Epoll *m_Ep = nullptr;          // Channel所在的红黑树
  bool m_InEpoll = false;         // Channel是否已添加到epoll红黑树上
  uint32_t m_Events = 0;          // m_Fd需要监视的事件，如EPOLLIN
  uint32_t m_REvents = 0;         // 已发生的事件
  bool m_IsListen = false;        // 是否用于服务端监听
public:
  Channel(Epoll* ep, int fd, bool isListen);
  ~Channel();

  void UseET();           // 采用边缘触发
  void EnableReading();   // 让epoll_wait()监听读事件
  void SetInEpoll();      // 把m_InEpoll设为true
  void SetREvents(uint32_t ev);

  int GetFd() const;
  bool IsInEpoll() const;
  uint32_t GetEvents() const;
  uint32_t GetREvents() const;

  void HandleEvent(Socket* serverSocket);
};
