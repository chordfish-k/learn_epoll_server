#include "TcpServer.h"

#include "Connection.h"

#include <functional>
#include <unistd.h>

TcpServer::TcpServer(const std::string& ip, const uint16_t port) {
  m_Acceptor = new Acceptor(&m_Loop, ip, port);
  m_Acceptor->SetOnNewConnectionCallback(std::bind(&TcpServer::OnNewConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {
  // 释放全部Fd
  for (auto &connPair : m_Conns) {
    delete connPair.second;
  }
}

void TcpServer::Start() {
  m_Loop.Run();
}

void TcpServer::OnNewConnection(Socket* clientSocket) {
  Connection* conn = new Connection(&m_Loop, clientSocket); // 还没释放conn
  conn->SetCloseCallback(std::bind(&TcpServer::OnCloseConnection, this, std::placeholders::_1));
  conn->SetErrorCallback(std::bind(&TcpServer::OnErrorConnection, this, std::placeholders::_1));

  printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", conn->GetFd(), conn->GetIp().c_str(), conn->GetPort());

  m_Conns[conn->GetFd()] = conn;
}

void TcpServer::OnCloseConnection(Connection* conn) {
  printf("Client(fd=%d) Disconnected.\n", conn->GetFd());
  // close(conn->GetFd()); // 在conn的析构函数->socket的析构函数中，会关闭fd
  m_Conns.erase(conn->GetFd());
  delete conn;
}

void TcpServer::OnErrorConnection(Connection* conn) {
  printf("Client(fd=%d) Error.\n", conn->GetFd());
  // close(conn->GetFd());
  m_Conns.erase(conn->GetFd());
  delete conn;

}
