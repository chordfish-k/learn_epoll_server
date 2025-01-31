#include "EchoServer.h"
#include "ThreadPool.h"
#include "Timestamp.h"

#include <cstdio>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>

EchoServer::EchoServer(const std::string& ip, const uint16_t port, int subThreadNum, int workThreadNum) 
  : m_TcpServer(ip, port, subThreadNum), m_ThreadPool(workThreadNum, ThreadPool::Type::WORK) {
    m_TcpServer.SetNewConnectionCallback(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    m_TcpServer.SetCloseConnectionCallback(std::bind(&EchoServer::HandleCloseConnection, this, std::placeholders::_1));
    m_TcpServer.SetErrorConnectionCallback(std::bind(&EchoServer::HandleErrorConnection, this, std::placeholders::_1));
    m_TcpServer.SetMessageCallback(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_TcpServer.SetSendCompleteCallback(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    // m_TcpServer.SetEpollTimeoutCallback(std::bind(&EchoServer::OnEpollTimeout, this, std::placeholders::_1));
}

EchoServer::~EchoServer() {
}

void EchoServer::Start() {
  m_TcpServer.Start();
}

void EchoServer::Stop() {
  // 停止工作线程
  m_ThreadPool.Stop();
  //printf("工作线程池已停止\n");
  // 停止IO线程
  m_TcpServer.Stop();
  //printf("TcpServer已停止\n");
}

void EchoServer::HandleNewConnection(Ref<Connection> conn) {
  //std::cout << "New connection come in.\n";
  printf("[%s] New Connection(fd=%d,ip=%s,port=%d) ok.\n", 
    Timestamp::now().toString().c_str(), conn->GetFd(), conn->GetIp().c_str(), conn->GetPort());
}

void EchoServer::HandleCloseConnection(Ref<Connection> conn) {
  // std::cout << "EchoServer conn close.\n";
  printf("[%s] Connection Closed(fd=%d,ip=%s,port=%d) ok.\n", 
    Timestamp::now().toString().c_str(), conn->GetFd(), conn->GetIp().c_str(), conn->GetPort());
}

void EchoServer::HandleErrorConnection(Ref<Connection> conn) {
  // std::cout << "EchoServer conn error.\n";
}

void EchoServer::HandleMessage(Ref<Connection> conn, std::string& message) {
  if (m_ThreadPool.Size() == 0) {
    // 如果没有工作线程，表示在IO线程中计算
    OnMessage(conn, message);
  }
  else {
    // 把业务添加到任务队列中
    m_ThreadPool.AddTask(std::bind(&EchoServer::OnMessage, this, conn, message));
  }
}

void EchoServer::HandleSendComplete(Ref<Connection> conn) {
  // std::cout << "Message send complete.\n";
}

// void EchoServer::OnEpollTimeout(EventLoop* loop) {
//   std::cout << "EchoServer timeout.\n";
// }

void EchoServer::OnMessage(Ref<Connection> conn, std::string& message) {
  // printf("[%s] Message(fd=%d, thread=%ld): %s\n", 
  //  Timestamp::now().toString().c_str(), conn->GetFd(), syscall(SYS_gettid), message.c_str());
  // 假设经过若干处理，得到结果
  message = "Reply: " + message;
  conn->Send(message.data(), message.size());
}