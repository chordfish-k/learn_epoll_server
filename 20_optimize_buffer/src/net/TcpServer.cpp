#include "TcpServer.h"

#include "Connection.h"

#include <cstdio>
#include <functional>
#include <unistd.h>

TcpServer::TcpServer(const std::string& ip, const uint16_t port) {
  m_Acceptor = new Acceptor(&m_Loop, ip, port);
  m_Acceptor->SetNewConnectionCallback(std::bind(&TcpServer::OnNewConnection, this, std::placeholders::_1));
  m_Loop.SetEpollTimeoutCallback(std::bind(&TcpServer::OnEpollTimeout, this, std::placeholders::_1));
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
  conn->SetMessageCallback(std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
  conn->SetSendCompleteCallback(std::bind(&TcpServer::OnSendComplete, this, std::placeholders::_1));

  // printf("Accept Client(fd=%d, ip=%s, port=%d) OK.\n", conn->GetFd(), conn->GetIp().c_str(), conn->GetPort());

  m_Conns[conn->GetFd()] = conn;

  // 连接建立后回调EchoServer类的方法
  if (m_NewConnectionCallback)
    m_NewConnectionCallback(conn);
}

void TcpServer::OnCloseConnection(Connection* conn) {
  // 连接关闭前回调EchoServer类的方法
  if (m_CloseConnectionCallback)
    m_CloseConnectionCallback(conn);

  // printf("Client(fd=%d) Disconnected.\n", conn->GetFd());
  // close(conn->GetFd()); // 在conn的析构函数->socket的析构函数中，会关闭fd
  m_Conns.erase(conn->GetFd());
  delete conn;
}

void TcpServer::OnErrorConnection(Connection* conn) {
  if (m_ErrorConnectionCallback)
    m_ErrorConnectionCallback(conn);

  // printf("Client(fd=%d) Error.\n", conn->GetFd());
  // close(conn->GetFd());
  m_Conns.erase(conn->GetFd());
  delete conn;

}

void TcpServer::OnMessage(Connection* conn, std::string message) {
  // // 假设经过若干处理，得到结果
  // message = "Reply: " + message;
  
  // // 处理响应报文
  // int len = message.size();
  // std::string tmpBuf((char*)&len, 4); // 把报文头部填充到报文
  // tmpBuf.append(message);

  // //send(conn->GetFd(), tmpBuf.data(), tmpBuf.size(), 0);
  // conn->Send(tmpBuf.data(), tmpBuf.size());

  if (m_MessageCallback)
    m_MessageCallback(conn, message);
}

void TcpServer::OnSendComplete(Connection* conn) {
  // printf("Send Complete.\n");

  if (m_SendCompleteCallback)
    m_SendCompleteCallback(conn);
}

void TcpServer::OnEpollTimeout(EventLoop* loop) {
  // printf("Timeout: epoll_wait().\n");

  if (m_EpollTimeoutCallback)
    m_EpollTimeoutCallback(loop);
}

void TcpServer::SetNewConnectionCallback(std::function<void(Connection*)> fn) {
  m_NewConnectionCallback = fn;
}

void TcpServer::SetCloseConnectionCallback(std::function<void(Connection*)> fn) {
  m_CloseConnectionCallback = fn;
}

void TcpServer::SetErrorConnectionCallback(std::function<void(Connection*)> fn) {
  
  m_ErrorConnectionCallback = fn;
}

void TcpServer::SetMessageCallback(std::function<void(Connection*, std::string)> fn) {
  m_MessageCallback = fn;
}

void TcpServer::SetSendCompleteCallback(std::function<void(Connection*)> fn) {
  m_SendCompleteCallback = fn;
}

void TcpServer::SetEpollTimeoutCallback(std::function<void(EventLoop*)> fn) {
  m_EpollTimeoutCallback = fn;
}