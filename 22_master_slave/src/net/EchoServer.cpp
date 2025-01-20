#include "EchoServer.h"

#include <functional>
#include <iostream>

EchoServer::EchoServer(const std::string& ip, const uint16_t port, int threadNum) 
  : m_TcpServer(ip, port, threadNum) {
    m_TcpServer.SetNewConnectionCallback(std::bind(&EchoServer::OnNewConnection, this, std::placeholders::_1));
    m_TcpServer.SetCloseConnectionCallback(std::bind(&EchoServer::OnCloseConnection, this, std::placeholders::_1));
    m_TcpServer.SetErrorConnectionCallback(std::bind(&EchoServer::OnErrorConnection, this, std::placeholders::_1));
    m_TcpServer.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_TcpServer.SetSendCompleteCallback(std::bind(&EchoServer::OnSendComplete, this, std::placeholders::_1));
    // m_TcpServer.SetEpollTimeoutCallback(std::bind(&EchoServer::OnEpollTimeout, this, std::placeholders::_1));
}

EchoServer::~EchoServer() {

}

void EchoServer::Start() {
  m_TcpServer.Start();
}

void EchoServer::OnNewConnection(Connection* clientSocket) {
  std::cout << "New connection come in.\n";
}

void EchoServer::OnCloseConnection(Connection* conn) {
  std::cout << "EchoServer conn close.\n";
}

void EchoServer::OnErrorConnection(Connection* conn) {

}

void EchoServer::OnMessage(Connection* conn, std::string& message) {
  // 假设经过若干处理，得到结果
  message = "Reply: " + message;

  //send(conn->GetFd(), tmpBuf.data(), tmpBuf.size(), 0);
  conn->Send(message.data(), message.size());
}

void EchoServer::OnSendComplete(Connection* conn) {
  std::cout << "Message send complete.\n";
}

// void EchoServer::OnEpollTimeout(EventLoop* loop) {
//   std::cout << "EchoServer timeout.\n";
// }