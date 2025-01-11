#pragma once

#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

// 连接类，接受新的连接
class Connection
{
private:
  EventLoop* m_Loop;        // Connection对应的事件循环
  Socket* m_ClientSocket;   // 与客户端通讯的socket
  Channel* m_ClientChannel; // Connection对应的Channel
public:
  Connection(EventLoop* loop, Socket* clientSocket);
  ~Connection();
};
