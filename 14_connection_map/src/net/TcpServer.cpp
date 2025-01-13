#include "TcpServer.h"

#include "Connection.h"

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
  
  printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", conn->GetFd(), conn->GetIp().c_str(), conn->GetPort());

  m_Conns[conn->GetFd()] = conn;
}
