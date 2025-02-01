#include "Socket.h"

#include <cstdio>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

static int FdCreateNonblocking() {
  int listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (listenFd < 0) { 
    // perror("socket() failed"); 
    printf("%s;%s;%d Listen socket create error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno); 
    exit(-1); 
  }
  return listenFd;
}

Socket::Socket()
  : m_Fd(FdCreateNonblocking()) {
}

Socket::Socket(int fd) 
  : m_Fd(fd) {
}
  
Socket::~Socket() {
  close(m_Fd);
}

int Socket::GetFd() const {
  return m_Fd;
}

std::string Socket::GetIp() const {
  return m_Ip;
}

uint16_t Socket::GetPort() const {
  return m_Port;
}

void Socket::SetIpAndPort(const std::string& ip, uint16_t port) {
  m_Ip = ip;
  m_Port = port;
}

void Socket::SetReuseAddr(bool on) {
  const int opt = on ? 1 : 0;
  setsockopt(m_Fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::SetReusePort(bool on) {
  const int opt = on ? 1 : 0;
  setsockopt(m_Fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::SetTcpNoDelay(bool on) {
  const int opt = on ? 1 : 0;
  setsockopt(m_Fd, SOL_SOCKET, TCP_NODELAY, &opt, sizeof(opt));
}

void Socket::SetKeepAlive(bool on) {
  const int opt = on ? 1 : 0;
  setsockopt(m_Fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

void Socket::Bind(const InetAddress& serverAddr) {
  if (bind(m_Fd, serverAddr.GetAddr(), sizeof(sockaddr)) < 0) {
    perror("bind() failed"); close(m_Fd); exit(-1);
  }
  SetIpAndPort(serverAddr.GetIp(), serverAddr.GetPort());
}

void Socket::Listen(int maxQueued) {
  if (listen(m_Fd, 128) != 0) {
    perror("listen() failed"); close(m_Fd); exit(-1);
  }
}

int Socket::Accept(InetAddress& clientAddr) {
  sockaddr_in peerAddr;
  socklen_t len = sizeof(peerAddr);
  int clientFd = accept4(m_Fd, (sockaddr*)&peerAddr, &len, SOCK_NONBLOCK);

  clientAddr.SetAddr(peerAddr); // 客户端的地址和协议

  m_Ip = clientAddr.GetIp();
  m_Port = clientAddr.GetPort();

  return clientFd;
}

