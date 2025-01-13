#pragma once

#include "InetAddress.h"

#include <string>

// 封装socket
class Socket
{
private:
  const int m_Fd;   // Socket持有的Fd
  std::string m_Ip; // 如果是监听用的Fd，存放服务端监听的ip，如果是客户端连接的Fd，存放对端的ip
  uint16_t m_Port;  // 同上，存放端口
public:
  Socket();         // 默认构造，用于监听用的Fd，自动创建非阻塞
  Socket(int fd);
  ~Socket();

  int GetFd() const;
  std::string GetIp() const;
  uint16_t GetPort() const;

  void SetReuseAddr(bool on);
  void SetReusePort(bool on);
  void SetTcpNoDelay(bool on);
  void SetKeepAlive(bool on);

  void Bind(const InetAddress& serverAddr);
  void Listen(int maxQueued = 128);
  int Accept(InetAddress& clientAddr);
};
