#pragma once

#include "net/Connection.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "net/ThreadPool.h"

/*
EchoServer: Http服务器类
*/
class HttpServer {
private:
  TcpServer m_TcpServer;
  ThreadPool m_ThreadPool;

public:
  // sub线程：负责从事件循环io的线程；work线程：业务逻辑的工作线程
  HttpServer(const std::string &ip, const uint16_t port, int subThreadNum = 3,
             int workThreadNum = 5);
  ~HttpServer();

  void Start(); // 启动服务
  void Stop();  // 停止服务
                // 处理新的连接请求，在TcpServer类中回调该函数
  void HandleNewConnection(Ref<Connection> conn);
  // 关闭客户端的连接，在TcpServer中回调该函数
  void HandleCloseConnection(Ref<Connection> conn);
  // 客户端的连接错误，在TcpServer中回调该函数
  void HandleErrorConnection(Ref<Connection> conn);
  // 处理客户端的请求报文，在TcpServer类中回调该函数
  void HandleMessage(Ref<Connection> conn, std::string &message);
  // 数据发送完成后，在TcpServer类中回调该函数
  void HandleSendComplete(Ref<Connection> conn);
  // epoll_wait超时，在TcpServer类中回调
  void OnEpollTimeout(EventLoop *loop);
  // 处理客户端的请求报文，用于添加给线程池
  void OnMessage(Ref<Connection> conn, std::string &message);
};