#include "TcpServer.h"

TcpServer::TcpServer(const std::string& ip, const uint16_t port) {
  m_Acceptor = new Acceptor(&m_Loop, ip, port);
}

TcpServer::~TcpServer() {

}

void TcpServer::Start() {
  m_Loop.Run();
}
