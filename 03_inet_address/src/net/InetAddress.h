#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// socket的地址协议类
class InetAddress
{
private:
  sockaddr_in m_Addr;   // 表示地址协议的结构体
public:
  InetAddress(const std::string& ip, uint16_t port); // 监听的fd用这个 
  InetAddress(const sockaddr_in addr); // 客户端连上了的fd用这个
  ~InetAddress();

  const char* GetIp() const;
  uint16_t GetPort() const;
  const sockaddr* GetAddr() const;
};
