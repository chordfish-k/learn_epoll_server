#pragma once

#include "InetAddress.h"

// 封装socket
class Socket
{
private:
  const int m_Fd;
public:
  Socket();
  Socket(int fd);
  ~Socket();

  int GetFd() const;

  void SetReuseAddr(bool on);
  void SetReusePort(bool on);
  void SetTcpNoDelay(bool on);
  void SetKeepAlive(bool on);

  void Bind(const InetAddress& serverAddr);
  void Listen(int maxQueued = 128);
  int Accept(InetAddress& clientAddr);
};
