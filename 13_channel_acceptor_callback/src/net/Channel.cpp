#include "Channel.h"

#include "EventLoop.h"
#include "Connection.h"

#include <functional>
#include <string.h>
#include <strings.h>
#include <unistd.h>

Channel::Channel(EventLoop* loop, int fd) 
  : m_Loop(loop), m_Fd(fd) {

}

Channel::~Channel() {

}

void Channel::UseET() {
  m_Events |= EPOLLET;
}
  
void Channel::EnableReading() {
  m_Events |= EPOLLIN;
  m_Loop->GetEpoll()->UpdateChannel(this);
}

void Channel::SetInEpoll() {
  m_InEpoll = true;
}
  
void Channel::SetREvents(uint32_t ev) {
  m_REvents = ev;
}

int Channel::GetFd() const {
  return m_Fd;
}

bool Channel::IsInEpoll() const {
  return m_InEpoll;
}

uint32_t Channel::GetEvents() const {
  return m_Events;
}
  
uint32_t Channel::GetREvents() const {
  return m_REvents;
}

void Channel::HandleEvent() {
  if (m_REvents & EPOLLRDHUP) {
    // 对方已关闭。部分系统检测不到该事件，可使用EPOLLIN, recv()返回0
    printf("Client(fd=%d) Disconnected.\n", m_Fd);
    close(m_Fd);
  }
  else if (m_REvents & (EPOLLIN | EPOLLPRI)) {
    // 处理读事件
    if (m_ReadCallback)
      m_ReadCallback();
  }
  else if (m_REvents & EPOLLOUT) {
    // 有数据需要写。。
  }
  else {
    // 其他事件，都视为错误
    printf("Client(fd=%d) Error.\n", m_Fd);
    close(m_Fd);
  }
}

void Channel::OnMessage() {
  // 否则就是客户端连接的fd有事件j
  // 接收缓冲区中有数据可以读
  char buffer[1024];
  while (true) {
    // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
    bzero(&buffer, sizeof(buffer));
    ssize_t nread = read(m_Fd, buffer, sizeof(buffer));
    if (nread > 0) {
      // 成功读取到数据
      printf("Recv(fd=%d): %s\n", m_Fd, buffer);
      send(m_Fd, buffer, strlen(buffer), 0);
    }
    else if (nread == -1 && errno == EINTR) {
      // 读取数据的时候被信号中断，继续读取
      continue;
    }
    else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 全部的数据已经读取完毕
      break;
    }
    else if (nread == 0) {
      printf("Client(fd=%d) Disconnected.\n", m_Fd);
      close(m_Fd);
      break;
    }
  }
}

void Channel::SetReadCallback(std::function<void()> fn) {
  m_ReadCallback = fn;
}
