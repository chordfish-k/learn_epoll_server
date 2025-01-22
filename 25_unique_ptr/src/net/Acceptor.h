#pragma once

#include "Pointer.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>

// 接收器类，接受新的连接
class Acceptor
{
private:
  const Scope<EventLoop>& m_Loop;   // Acceptor对应的事件循环，构造函数中传入，没有所有权，使用指针引用
  Socket m_ServerSocket;            // 服务端用于监听的socket
  Channel m_AcceptChannel;          // Acceptor对应的Channel，一个服务端只有一个Acceptor，在栈上分配Channel
  std::function<void(Scope<Socket>)> m_NewConnectionCallback; // 处理新客户端连接请求的回调函数，指向TcpServer::OnNewConnection()
public:
  Acceptor(const Scope<EventLoop>& loop, const std::string& ip, const uint16_t port);
  ~Acceptor();

  void OnNewConnection();  // 处理新客户端连接请求
  
  void SetNewConnectionCallback(std::function<void(Scope<Socket>)> fn);
};
