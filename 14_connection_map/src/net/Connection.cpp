#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"

Connection::Connection(EventLoop* loop, Socket* clientSocket) 
  : m_Loop(loop), m_ClientSocket(clientSocket) {
  // 为新客户端连接准备读事件，并添加到epoll
  m_ClientChannel = new Channel(m_Loop, m_ClientSocket->GetFd());
  m_ClientChannel->SetReadCallback(std::bind(&Channel::OnMessage, m_ClientChannel));
  m_ClientChannel->UseET();         // 使用边缘触发
  m_ClientChannel->EnableReading(); // 将Channel添加到Epoll
}

Connection::~Connection() {
  delete m_ClientSocket;
  delete m_ClientChannel;
}

int Connection::GetFd() const {
  return m_ClientSocket->GetFd();
}

std::string Connection::GetIp() const {
  return m_ClientSocket->GetIp();
}

uint16_t Connection::GetPort() const {
  return m_ClientSocket->GetPort();
}
