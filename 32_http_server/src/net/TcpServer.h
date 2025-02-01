#pragma once

#include "Pointer.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Acceptor.h"
#include "ThreadPool.h"

#include <functional>
#include <map>
#include <mutex>
#include <vector>

class TcpServer
{
private:
  Scope<EventLoop> m_MainLoop;              // 一个TcpServer只有一个主事件循环
  std::vector<Scope<EventLoop>> m_SubLoops; // 一个TcpServer可以有多个从事件循环
  Acceptor m_Acceptor;                      // 一个TcpServer只有一个Acceptor对象
  int m_ThreadNum;                          // 线程池的大小，即从事件循环的个数
  ThreadPool m_ThreadPool;                  // 线程池

  std::map<int, Ref<Connection>> m_Conns;   // fd->conn, 一个TcpServer有多个Connection对象，存放在map容器中
  std::mutex m_ConnsMtx;                    // m_Conns会被多个从事件循环修改，需要加锁

  std::function<void(Ref<Connection>)> m_NewConnectionCallback;         // 回调 EchoServer 中对应函数
  std::function<void(Ref<Connection>)> m_CloseConnectionCallback;       // 回调 EchoServer 中对应函数
  std::function<void(Ref<Connection>)> m_ErrorConnectionCallback;       // 回调 EchoServer 中对应函数
  std::function<void(Ref<Connection>, std::string&)> m_MessageCallback; // 回调 EchoServer 中对应函数
  std::function<void(Ref<Connection>)> m_SendCompleteCallback;          // 回调 EchoServer 中对应函数
  std::function<void(EventLoop*)> m_EpollTimeoutCallback;           // 回调 EchoServer 中对应函数
public:
  TcpServer(const std::string& ip, const uint16_t port, int threadNum = 3);
  ~TcpServer();

  void Start();      // 开启事件循环
  void Stop();       // 停止IO线程和事件循环
  
  void OnNewConnection(Scope<Socket> clientSocket);   // 处理新的连接请求，在Acceptor类中回调该函数
  void OnCloseConnection(Ref<Connection> conn);     // 关闭客户端的连接，在Connection中回调该函数
  void OnErrorConnection(Ref<Connection> conn);     // 客户端的连接错误，在Connection中回调该函数
  void OnMessage(Ref<Connection> conn, std::string& message);  // 处理客户端的请求报文，在Connection类中回调该函数
  void OnSendComplete(Ref<Connection> conn);        // 数据发送完成后，在Connection类中回调该函数
  void OnEpollTimeout(EventLoop* loop);         // epoll_wait超时，在EevntLoop类中回调

  void SetNewConnectionCallback(std::function<void(Ref<Connection>)> fn);
  void SetCloseConnectionCallback(std::function<void(Ref<Connection>)> fn);
  void SetErrorConnectionCallback(std::function<void(Ref<Connection>)> fn);
  void SetMessageCallback(std::function<void(Ref<Connection>, std::string&)> fn);
  void SetSendCompleteCallback(std::function<void(Ref<Connection>)> fn);
  void SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn);

  void RemoveConnetion(int fd);   // 删除m_Conns中的Connection对象，在EventLoop::HandleTimer中回调
};
