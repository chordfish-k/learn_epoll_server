#pragma once

#include "Pointer.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"

#include <atomic>
#include <functional>
#include <sys/syscall.h>
#include <unistd.h>
#include <memory>

// 连接类，接受新的连接  | std::enable_shared_from_this允许再类里面获取自身智能指针
class Connection : public std::enable_shared_from_this<Connection>
{
private:
  EventLoop* m_Loop;              // Connection对应的事件循环，由构造函数传入
  Scope<Socket> m_ClientSocket;   // 与客户端通讯的socket
  Scope<Channel> m_ClientChannel; // Connection对应的Channel，一个服务器可能有很多个Connection，在堆上分配
  Buffer m_InputBuffer;           // 接收缓冲区
  Buffer m_OutputBuffer;          // 发送缓冲区
  std::atomic_bool m_Disconnect;  // 客户端连接释放已经断开，如果断开，设置为true，需要在工作线程中使用，要用原子类型

  std::function<void(Ref<Connection>)> m_CloseCallback;                 // 关闭m_Fd的回调函数，回调TcpServer::OnCloseConnection
  std::function<void(Ref<Connection>)> m_ErrorCallback;                 // m_Fd发生错误的回调函数，回调TcpServer::OnErrorConnection
  std::function<void(Ref<Connection>, std::string&)> m_MessageCallback;  // 处理报文的回调函数，回调TcpServer::OnMessage
  std::function<void(Ref<Connection>)> m_SendCompleteCallback;          // 发送完成回调函数，回调TcpServer::OnSendComplete
public:
  Connection(EventLoop* loop, Scope<Socket> clientSocket);
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

  // 发送数据
  void Send(const char* data, size_t size);
  // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传递给IO线程
  void SendInLoop(std::string data);
};
