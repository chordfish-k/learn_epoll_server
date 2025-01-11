#include "TcpServer.h"
#include "Socket.h"

TcpServer::TcpServer(const std::string& ip, const uint16_t port) {
  // 服务端socket
  Socket* serverSocket = new Socket(); // 还没释放，以后再说
  // 服务端的地址和协议
  InetAddress serverAddr(ip, port);
  serverSocket->SetReuseAddr(true);
  serverSocket->SetReusePort(true);
  serverSocket->SetKeepAlive(true);
  serverSocket->SetTcpNoDelay(true);
  
  // 绑定并监听
  serverSocket->Bind(serverAddr);
  serverSocket->Listen();
 
  // Epoll 事件循环
  Channel* serverChannel = new Channel(m_Loop.GetEpoll(), serverSocket->GetFd());
  serverChannel->SetReadCallback(std::bind(&Channel::OnNewConnection, serverChannel, serverSocket));
  serverChannel->EnableReading();
}

TcpServer::~TcpServer() {

}

void TcpServer::Start() {
  m_Loop.Run();
}
