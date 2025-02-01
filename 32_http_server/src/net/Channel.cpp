#include "Channel.h"

#include "EventLoop.h"

#include <cerrno>
#include <cstdio>
#include <functional>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Channel::Channel(EventLoop *loop, int fd) : m_Loop(loop), m_Fd(fd) {}

Channel::~Channel() {}

void Channel::UseEdgeTrigger() { m_Events |= EPOLLET; }

void Channel::SetEnableReading(bool enable) {
  if (enable) {
    m_Events |= EPOLLIN;
  } else {
    m_Events &= ~EPOLLIN;
  }
  m_Loop->UpdateChannel(this);
}

void Channel::SetEnableWriting(bool enable) {
  if (enable) {
    m_Events |= EPOLLOUT;
  } else {
    m_Events &= ~EPOLLOUT;
  }
  m_Loop->UpdateChannel(this);
}

void Channel::DisableAllEvent() {
  m_Events = 0;
  m_Loop->UpdateChannel(this);
}

void Channel::Remove() {
  // 先取消全部事件
  DisableAllEvent();
  m_Loop->RemoveChannel(this);
}

void Channel::SetInEpoll() { m_InEpoll = true; }

void Channel::SetREvents(uint32_t ev) { m_REvents = ev; }

int Channel::GetFd() const { return m_Fd; }

bool Channel::IsInEpoll() const { return m_InEpoll; }

uint32_t Channel::GetEvents() const { return m_Events; }

uint32_t Channel::GetREvents() const { return m_REvents; }

void Channel::HandleEvent() {
  if (m_REvents & EPOLLRDHUP) {
    // 调用关闭回调函数
    if (m_CloseCallback)
      m_CloseCallback();
  } else if (m_REvents & (EPOLLIN | EPOLLPRI)) {
    // 处理读事件
    if (m_ReadCallback)
      m_ReadCallback();
  } else if (m_REvents & EPOLLOUT) {
    // 处理写事件
    if (m_WriteCallback)
      m_WriteCallback();
  } else {
    // 调用错误回调函数
    if (m_ErrorCallback)
      m_ErrorCallback();
  }
}

void Channel::SetReadCallback(std::function<void()> fn) { m_ReadCallback = fn; }

void Channel::SetWriteCallback(std::function<void()> fn) {
  m_WriteCallback = fn;
}

void Channel::SetCloseCallback(std::function<void()> fn) {
  m_CloseCallback = fn;
}

void Channel::SetErrorCallback(std::function<void()> fn) {
  m_ErrorCallback = fn;
}
