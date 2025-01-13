#pragma once

#include "EventLoop.h"
#include "Channel.h"
#include "Acceptor.h"

class TcpServer
{
private:
  EventLoop m_Loop;     // 一个TcpServer可以有多个事件循环，单线程暂时只用一个事件循环
  Acceptor* m_Acceptor; // 一个TcpServer只有一个Acceptor对象
public:
  TcpServer(const std::string& ip, const uint16_t port);
  ~TcpServer();

  void Start();      // 开启事件循环
  
  void OnNewConnection(Socket* clientSocket);  // 处理新的连接请求
};
