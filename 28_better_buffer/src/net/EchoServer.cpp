#include "EchoServer.h"
#include "ThreadPool.h"

#include <cstdio>
#include <functional>
#include <iostream>
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

void EchoServer::HandleNewConnection(Ref<Connection> clientSocket) {
  std::cout << "New connection come in.\n";
}

void EchoServer::HandleCloseConnection(Ref<Connection> conn) {
  std::cout << "EchoServer conn close.\n";
}

void EchoServer::HandleErrorConnection(Ref<Connection> conn) {
  std::cout << "EchoServer conn error.\n";
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
  std::cout << "Message send complete.\n";
}

// void EchoServer::OnEpollTimeout(EventLoop* loop) {
//   std::cout << "EchoServer timeout.\n";
// }

void EchoServer::OnMessage(Ref<Connection> conn, std::string& message) {
  // 假设经过若干处理，得到结果
  message = "Reply: " + message;
  conn->Send(message.data(), message.size());
}