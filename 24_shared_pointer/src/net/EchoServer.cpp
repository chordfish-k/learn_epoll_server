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

void EchoServer::HandleNewConnection(Connection* clientSocket) {
  std::cout << "New connection come in.\n";
}

void EchoServer::HandleCloseConnection(Connection* conn) {
  std::cout << "EchoServer conn close.\n";
}

void EchoServer::HandleErrorConnection(Connection* conn) {

}

void EchoServer::HandleMessage(Connection* conn, std::string& message) {
  // 把业务添加到任务队列中
  //printf("IO thread = %ld.\n", syscall(SYS_gettid));
  m_ThreadPool.AddTask(std::bind(&EchoServer::OnMessage, this, conn, message));
}

void EchoServer::HandleSendComplete(Connection* conn) {
  std::cout << "Message send complete.\n";
}

// void EchoServer::OnEpollTimeout(EventLoop* loop) {
//   std::cout << "EchoServer timeout.\n";
// }

void EchoServer::OnMessage(Connection* conn, std::string& message) {
  // 假设经过若干处理，得到结果
  //printf("Work thread = %ld.\n", syscall(SYS_gettid));
  message = "Reply: " + message;
  conn->Send(message.data(), message.size());
}