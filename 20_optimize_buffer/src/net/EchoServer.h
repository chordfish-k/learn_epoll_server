#pragma once

#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"

/*
EchoServer: 回显服务器类
*/
class EchoServer
{
private:
  TcpServer m_TcpServer;
public:
  EchoServer(const std::string& ip, const uint16_t port);
  ~EchoServer();

  void Start();   // 启动服务

  void OnNewConnection(Connection* clientSocket);   // 处理新的连接请求，在TcpServer类中回调该函数
  void OnCloseConnection(Connection* conn);     // 关闭客户端的连接，在TcpServer中回调该函数
  void OnErrorConnection(Connection* conn);     // 客户端的连接错误，在TcpServer中回调该函数
  void OnMessage(Connection* conn, std::string message);  // 处理客户端的请求报文，在TcpServer类中回调该函数
  void OnSendComplete(Connection* conn);        // 数据发送完成后，在TcpServer类中回调该函数
  // void OnEpollTimeout(EventLoop* loop);         // epoll_wait超时，在TcpServer类中回调
};