#pragma once

#include <functional>

#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

// 接收器类，接受新的连接
class Acceptor
{
private:
  EventLoop* m_Loop;        // Acceptor对应的事件循环
  Socket* m_ServerSocket;   // 服务端用于监听的socket
  Channel* m_AcceptChannel; // Acceptor对应的Channel
  std::function<void(Socket*)> m_NewConnectionCallback; // 处理新客户端连接请求的回调函数，指向TcpServer::OnNewConnection()
public:
  Acceptor(EventLoop* loop, const std::string& ip, const uint16_t port);
  ~Acceptor();

  void OnNewConnection();  // 处理新客户端连接请求
  
  void SetNewConnectionCallback(std::function<void(Socket*)> fn);
};
