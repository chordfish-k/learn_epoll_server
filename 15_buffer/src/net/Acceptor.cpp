#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* loop, const std::string& ip, const uint16_t port) 
  : m_Loop(loop) {
  m_ServerSocket = new Socket(); // 还没释放，以后再说
  // 服务端的地址和协议
  InetAddress serverAddr(ip, port);
  m_ServerSocket->SetReuseAddr(true);
  m_ServerSocket->SetReusePort(true);
  m_ServerSocket->SetKeepAlive(true);
  m_ServerSocket->SetTcpNoDelay(true);
  
  // 绑定并监听
  m_ServerSocket->Bind(serverAddr);
  m_ServerSocket->Listen();
 
  // Epoll 事件循环
  m_AcceptChannel = new Channel(m_Loop, m_ServerSocket->GetFd());
  m_AcceptChannel->SetReadCallback(std::bind(&Acceptor::OnNewConnection, this));
  m_AcceptChannel->EnableReading();
}

Acceptor::~Acceptor() {
  delete m_ServerSocket;
  delete m_AcceptChannel;
}

void Acceptor::OnNewConnection() {
  InetAddress clientAddr; // 客户端的地址和协议
  // new出来的socket暂时无法释放，后面再解决
  Socket *clientSocket = new Socket(m_ServerSocket->Accept(clientAddr));
  clientSocket->SetIpAndPort(clientAddr.GetIp(), clientAddr.GetPort());
  // 为新客户端连接准备读事件，并添加到epoll中
  m_NewConnectionCallback(clientSocket);
}

void Acceptor::SetOnNewConnectionCallback(std::function<void(Socket*)> fn) {
  m_NewConnectionCallback = fn;
}
