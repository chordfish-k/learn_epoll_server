#pragma once

#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Acceptor.h"

#include <map>

class TcpServer
{
private:
  EventLoop m_Loop;     // 一个TcpServer可以有多个事件循环，单线程暂时只用一个事件循环
  Acceptor* m_Acceptor; // 一个TcpServer只有一个Acceptor对象
  std::map<int, Connection*> m_Conns;
public:
  TcpServer(const std::string& ip, const uint16_t port);
  ~TcpServer();

  void Start();      // 开启事件循环
  
  void OnNewConnection(Socket* clientSocket);   // 处理新的连接请求，在Acceptor类中回调该函数
  void OnCloseConnection(Connection* conn);     // 关闭客户端的连接，在Connection中回调该函数
  void OnErrorConnection(Connection* conn);     // 客户端的连接错误，在Connection中回调该函数
  void OnMessage(Connection* conn, std::string message);  // 处理客户端的请求报文，在Connection类中回调该函数
  void OnSendComplete(Connection* conn);        // 数据发送完成后，在Connection类中回调该函数
  void OnEpollTimeout(EventLoop* loop);         // epoll_wait超时，在EevntLoop类中回调
};
