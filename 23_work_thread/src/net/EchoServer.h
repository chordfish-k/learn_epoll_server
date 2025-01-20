#pragma once

#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

/*
EchoServer: 回显服务器类
*/
class EchoServer
{
private:
  TcpServer m_TcpServer;
  ThreadPool m_ThreadPool;
public:
  // sub线程：负责从事件循环io的线程；work线程：业务逻辑的工作线程
  EchoServer(const std::string& ip, const uint16_t port, int subThreadNum = 3, int workThreadNum = 5);
  ~EchoServer();

  void Start();   // 启动服务

  void HandleNewConnection(Connection* clientSocket);   // 处理新的连接请求，在TcpServer类中回调该函数
  void HandleCloseConnection(Connection* conn);         // 关闭客户端的连接，在TcpServer中回调该函数
  void HandleErrorConnection(Connection* conn);         // 客户端的连接错误，在TcpServer中回调该函数
  void HandleMessage(Connection* conn, std::string& message);  // 处理客户端的请求报文，在TcpServer类中回调该函数
  void HandleSendComplete(Connection* conn);            // 数据发送完成后，在TcpServer类中回调该函数
  // void OnEpollTimeout(EventLoop* loop);              // epoll_wait超时，在TcpServer类中回调

  void OnMessage(Connection* conn, std::string& message); // 处理客户端的请求报文，用于添加给线程池
};