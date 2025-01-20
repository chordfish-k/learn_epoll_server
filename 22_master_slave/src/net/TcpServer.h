#pragma once

#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Acceptor.h"
#include "ThreadPool.h"

#include <functional>
#include <map>
#include <vector>

class TcpServer
{
private:
  EventLoop* m_MainLoop;              // 一个TcpServer只有一个主事件循环
  std::vector<EventLoop*> m_SubLoops; // 一个TcpServer可以有多个从事件循环
  Acceptor* m_Acceptor;               // 一个TcpServer只有一个Acceptor对象
  ThreadPool* m_ThreadPool;           // 线程池
  int m_ThreadNum;                    // 线程池的大小，即从事件循环的个数

  std::map<int, Connection*> m_Conns; // 一个TcpServer有多个Connection对象，存放在map容器中
  
  std::function<void(Connection*)> m_NewConnectionCallback;         // 回调 EchoServer 中对应函数
  std::function<void(Connection*)> m_CloseConnectionCallback;       // 回调 EchoServer 中对应函数
  std::function<void(Connection*)> m_ErrorConnectionCallback;       // 回调 EchoServer 中对应函数
  std::function<void(Connection*, std::string&)> m_MessageCallback; // 回调 EchoServer 中对应函数
  std::function<void(Connection*)> m_SendCompleteCallback;          // 回调 EchoServer 中对应函数
  std::function<void(EventLoop*)> m_EpollTimeoutCallback;           // 回调 EchoServer 中对应函数
public:
  TcpServer(const std::string& ip, const uint16_t port, int threadNum = 3);
  ~TcpServer();

  void Start();      // 开启事件循环
  
  void OnNewConnection(Socket* clientSocket);   // 处理新的连接请求，在Acceptor类中回调该函数
  void OnCloseConnection(Connection* conn);     // 关闭客户端的连接，在Connection中回调该函数
  void OnErrorConnection(Connection* conn);     // 客户端的连接错误，在Connection中回调该函数
  void OnMessage(Connection* conn, std::string& message);  // 处理客户端的请求报文，在Connection类中回调该函数
  void OnSendComplete(Connection* conn);        // 数据发送完成后，在Connection类中回调该函数
  void OnEpollTimeout(EventLoop* loop);         // epoll_wait超时，在EevntLoop类中回调

  void SetNewConnectionCallback(std::function<void(Connection*)> fn);
  void SetCloseConnectionCallback(std::function<void(Connection*)> fn);
  void SetErrorConnectionCallback(std::function<void(Connection*)> fn);
  void SetMessageCallback(std::function<void(Connection*, std::string&)> fn);
  void SetSendCompleteCallback(std::function<void(Connection*)> fn);
  void SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn);
};
