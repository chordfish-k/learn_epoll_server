#include "InetAddress.h"

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  m_Addr.sin_family = AF_INET;                     // IPv4网络协议的套接字类型
  m_Addr.sin_addr.s_addr = inet_addr(ip.c_str());  // 服务端用于监听的ip地址
  m_Addr.sin_port = htons(port);                   // 监听的端口
}

InetAddress::InetAddress(const sockaddr_in addr)
  : m_Addr(addr) {}

InetAddress::~InetAddress() {

}

const char* InetAddress::GetIp() const {
  return inet_ntoa(m_Addr.sin_addr);
}

uint16_t InetAddress::GetPort() const {
  return ntohs(m_Addr.sin_port);
}

const sockaddr* InetAddress::GetAddr() const {
  return (sockaddr*)&m_Addr;
}

