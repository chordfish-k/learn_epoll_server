#pragma once

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

#include <string>

class TcpServer
{
private:
  EventLoop m_Loop;  // 一个TcpServer可以有多个事件循环，单线程暂时只用一个事件循环
public:
  TcpServer(const std::string& ip, const uint16_t port);
  ~TcpServer();

  void Start();      // 开启事件循环
};
