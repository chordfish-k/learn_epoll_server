#include "Acceptor.h"

Acceptor::Acceptor(const Scope<EventLoop>& loop, const std::string& ip, const uint16_t port) 
  : m_Loop(loop), m_ServerSocket(), m_AcceptChannel(m_Loop, m_ServerSocket.GetFd()) {
  // m_ServerSocket = new Socket(); // 还没释放，以后再说
  // 服务端的地址和协议
  InetAddress serverAddr(ip, port);
  m_ServerSocket.SetReuseAddr(true);
  m_ServerSocket.SetReusePort(true);
  m_ServerSocket.SetKeepAlive(true);
  m_ServerSocket.SetTcpNoDelay(true);
  
  // 绑定并监听
  m_ServerSocket.Bind(serverAddr);
  m_ServerSocket.Listen();
 
  // Epoll 事件循环
  // m_AcceptChannel = new Channel(m_Loop, m_ServerSocket.GetFd());
  m_AcceptChannel.SetReadCallback(std::bind(&Acceptor::OnNewConnection, this));
  m_AcceptChannel.SetEnableReading(true);
}

Acceptor::~Acceptor() {
}

void Acceptor::OnNewConnection() {
  InetAddress clientAddr; // 客户端的地址和协议
  Scope<Socket> clientSocket = CreateScope<Socket>(m_ServerSocket.Accept(clientAddr));
  clientSocket->SetIpAndPort(clientAddr.GetIp(), clientAddr.GetPort());
  // 调用新连接回调
  m_NewConnectionCallback(std::move(clientSocket));
}

void Acceptor::SetNewConnectionCallback(std::function<void(Scope<Socket>)> fn) {
  m_NewConnectionCallback = fn;
}
