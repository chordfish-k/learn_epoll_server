#include "TcpServer.h"

#include "Connection.h"
#include "EventLoop.h"
#include "Pointer.h"
#include "ThreadPool.h"

#include <cstdio>
#include <functional>
#include <mutex>
#include <unistd.h>

TcpServer::TcpServer(const std::string& ip, const uint16_t port, int threadNum) 
  : m_ThreadNum(threadNum), m_MainLoop(new EventLoop(true)), 
  m_Acceptor(m_MainLoop.get(), ip, port), m_ThreadPool(m_ThreadNum) {
  // 创建主事件循环
  m_MainLoop->SetEpollTimeoutCallback(std::bind(&TcpServer::OnEpollTimeout, this, std::placeholders::_1));
  
  m_Acceptor.SetNewConnectionCallback(std::bind(&TcpServer::OnNewConnection, this, std::placeholders::_1));

  // 创建从事件循环
  for (int i = 0; i < m_ThreadNum; i++) {
    m_SubLoops.emplace_back(new EventLoop(false, 5, 10));
    // 超时回调
    m_SubLoops[i]->SetEpollTimeoutCallback(std::bind(&TcpServer::OnEpollTimeout, this, std::placeholders::_1));
    // 清理空闲连接的计时器的回调
    m_SubLoops[i]->SetTimerCallback(std::bind(&TcpServer::RemoveConnetion, this, std::placeholders::_1));
    // 绑定线程池要执行的任务为事件循环的Run()
    m_ThreadPool.AddTask(std::bind(&EventLoop::Run, m_SubLoops[i].get()));
  }
}

TcpServer::~TcpServer() {
}

void TcpServer::Start() {
  m_MainLoop->Run();
}

void TcpServer::OnNewConnection(Scope<Socket> clientSocket) {
  const int idx = clientSocket->GetFd() % m_ThreadNum; // 随机选取一个从事件循环
  Ref<Connection> conn(new Connection(m_SubLoops[idx].get(), std::move(clientSocket))); // 还没释放conn
  conn->SetCloseCallback(std::bind(&TcpServer::OnCloseConnection, this, std::placeholders::_1));
  conn->SetErrorCallback(std::bind(&TcpServer::OnErrorConnection, this, std::placeholders::_1));
  conn->SetMessageCallback(std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
  conn->SetSendCompleteCallback(std::bind(&TcpServer::OnSendComplete, this, std::placeholders::_1));

  {
    std::lock_guard<std::mutex> lock(m_ConnsMtx);
    m_Conns[conn->GetFd()] = conn;  // 把conn存放到TcpServer的map容器中
  }
  m_SubLoops[idx]->OnNewConnection(conn); // 把conn存放到EventLoop的map容器中

  // 连接建立后回调EchoServer类的方法
  if (m_NewConnectionCallback)
    m_NewConnectionCallback(conn);
}

void TcpServer::OnCloseConnection(Ref<Connection> conn) {
  // 连接关闭前回调EchoServer类的方法
  if (m_CloseConnectionCallback)
    m_CloseConnectionCallback(conn);
  // close(conn->GetFd()); // 在conn的析构函数->socket的析构函数中，会关闭fd
  {
    std::lock_guard<std::mutex> lock(m_ConnsMtx);
    m_Conns.erase(conn->GetFd());
  }
  // delete conn; // 使用智能指针，不用手动释放
}

void TcpServer::OnErrorConnection(Ref<Connection> conn) {
  if (m_ErrorConnectionCallback)
    m_ErrorConnectionCallback(conn);
  // close(conn->GetFd());
  {
    std::lock_guard<std::mutex> lock(m_ConnsMtx);
    m_Conns.erase(conn->GetFd());
  }
  // delete conn;
}

void TcpServer::OnMessage(Ref<Connection> conn, std::string& message) {
  // 假设经过若干处理，得到结果
  if (m_MessageCallback)
    m_MessageCallback(conn, message);
}

void TcpServer::OnSendComplete(Ref<Connection> conn) {
  if (m_SendCompleteCallback)
    m_SendCompleteCallback(conn);
}
 
void TcpServer::OnEpollTimeout(EventLoop* loop) {
  if (m_EpollTimeoutCallback)
    m_EpollTimeoutCallback(loop);
}

void TcpServer::SetNewConnectionCallback(std::function<void(Ref<Connection>)> fn) {
  m_NewConnectionCallback = fn;
}

void TcpServer::SetCloseConnectionCallback(std::function<void(Ref<Connection>)> fn) {
  m_CloseConnectionCallback = fn;
}

void TcpServer::SetErrorConnectionCallback(std::function<void(Ref<Connection>)> fn) {
  
  m_ErrorConnectionCallback = fn;
}

void TcpServer::SetMessageCallback(std::function<void(Ref<Connection>, std::string&)> fn) {
  m_MessageCallback = fn;
}

void TcpServer::SetSendCompleteCallback(std::function<void(Ref<Connection>)> fn) {
  m_SendCompleteCallback = fn;
}

void TcpServer::SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn) {
  m_EpollTimeoutCallback = fn;
}

void TcpServer::RemoveConnetion(int fd) {
  {
    std::lock_guard<std::mutex> lock(m_ConnsMtx);
    m_Conns.erase(fd);
  }
}