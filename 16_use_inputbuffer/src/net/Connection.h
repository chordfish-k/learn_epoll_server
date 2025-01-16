#pragma once

#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"

#include <functional>

// 连接类，接受新的连接
class Connection
{
private:
  EventLoop* m_Loop;        // Connection对应的事件循环
  Socket* m_ClientSocket;   // 与客户端通讯的socket
  Channel* m_ClientChannel; // Connection对应的Channel
  Buffer m_InputBuffer;     // 接收缓冲区
  Buffer m_OutputBuffer;    // 发送缓冲区
  
  std::function<void(Connection*)> m_CloseCallback = nullptr; // 关闭m_Fd的回调函数，回调TcpServer::OnCloseConnection
  std::function<void(Connection*)> m_ErrorCallback = nullptr; // m_Fd发生错误的回调函数，回调TcpServer::OnErrorConnection
  std::function<void(Connection*, std::string)> m_MessageCallback = nullptr; // 处理报文的回调函数，回调TcpServer::OnMessage
public:
  Connection(EventLoop* loop, Socket* clientSocket);
  ~Connection();
  
  int GetFd() const;
  std::string GetIp() const;
  uint16_t GetPort() const;

  void OnClose();   // Tcp连接关闭的回调，供Channel回调
  void OnError();   // Tcp连接错误的回调, 供Channel回调
  void OnMessage(); // 处理对端发送过来的消息

  void SetCloseCallback(std::function<void(Connection*)> fn);
  void SetErrorCallback(std::function<void(Connection*)> fn);
  void SetMessageCallback(std::function<void(Connection*, std::string)> fn);
};
