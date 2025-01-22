#pragma once

#include "Pointer.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"

#include <atomic>
#include <functional>
#include <memory>

// 连接类，接受新的连接  | std::enable_shared_from_this允许再类里面获取自身智能指针
class Connection : public std::enable_shared_from_this<Connection>
{
private:
  EventLoop* m_Loop;              // Connection对应的事件循环
  Socket* m_ClientSocket;         // 与客户端通讯的socket
  Channel* m_ClientChannel;       // Connection对应的Channel
  Buffer m_InputBuffer;           // 接收缓冲区
  Buffer m_OutputBuffer;          // 发送缓冲区
  std::atomic_bool m_Disconnect;  // 客户端连接释放已经断开，如果断开，设置为true，需要在工作线程中使用，要用原子类型

  std::function<void(Ref<Connection>)> m_CloseCallback;                 // 关闭m_Fd的回调函数，回调TcpServer::OnCloseConnection
  std::function<void(Ref<Connection>)> m_ErrorCallback;                 // m_Fd发生错误的回调函数，回调TcpServer::OnErrorConnection
  std::function<void(Ref<Connection>, std::string&)> m_MessageCallback;  // 处理报文的回调函数，回调TcpServer::OnMessage
  std::function<void(Ref<Connection>)> m_SendCompleteCallback;          // 发送完成回调函数，回调TcpServer::OnSendComplete
public:
  Connection(EventLoop* loop, Socket* clientSocket);
  ~Connection();
  
  int GetFd() const;
  std::string GetIp() const;
  uint16_t GetPort() const;

  void OnMessage(); // 处理对端发送过来的消息
  void OnWrite();   // 处理写事件的回调，供Channel回调
  void OnClose();   // Tcp连接关闭的回调，供Channel回调
  void OnError();   // Tcp连接错误的回调, 供Channel回调

  void SetCloseCallback(std::function<void(Ref<Connection>)> fn);
  void SetErrorCallback(std::function<void(Ref<Connection>)> fn);
  void SetMessageCallback(std::function<void(Ref<Connection>, std::string&)> fn);
  void SetSendCompleteCallback(std::function<void(Ref<Connection>)> fn);

  void Send(const char* data, size_t size); // 发送数据
};
