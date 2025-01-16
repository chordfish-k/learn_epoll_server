#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"

#include <cstring>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

Connection::Connection(EventLoop* loop, Socket* clientSocket) 
  : m_Loop(loop), m_ClientSocket(clientSocket) {
  // 为新客户端连接准备读事件，并添加到epoll
  m_ClientChannel = new Channel(m_Loop, m_ClientSocket->GetFd());
  m_ClientChannel->SetReadCallback(std::bind(&Connection::OnMessage, this));
  m_ClientChannel->SetCloseCallback(std::bind(&Connection::OnClose, this));
  m_ClientChannel->SetErrorCallback(std::bind(&Connection::OnError, this));
  m_ClientChannel->UseET();         // 使用边缘触发
  m_ClientChannel->EnableReading(); // 将Channel添加到Epoll
}

Connection::~Connection() {
  delete m_ClientSocket;
  delete m_ClientChannel;
}

int Connection::GetFd() const {
  return m_ClientSocket->GetFd();
}

std::string Connection::GetIp() const {
  return m_ClientSocket->GetIp();
}

uint16_t Connection::GetPort() const {
  return m_ClientSocket->GetPort();
}

void Connection::OnClose() {
  if (m_CloseCallback)
    m_CloseCallback(this);
}

void Connection::OnError() {
  if (m_ErrorCallback)
    m_ErrorCallback(this);
}

void Connection::SetCloseCallback(std::function<void(Connection*)> fn) {
  m_CloseCallback = fn; 
}

void Connection::SetErrorCallback(std::function<void(Connection*)> fn) {
  m_ErrorCallback = fn;
}

void Connection::OnMessage() {
  // 否则就是客户端连接的fd有事件j
  // 接收缓冲区中有数据可以读
  char buffer[1024];
  while (true) {
    // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
    bzero(&buffer, sizeof(buffer));
    ssize_t nread = read(GetFd(), buffer, sizeof(buffer));
    if (nread > 0) {
      // 成功读取到数据
      // printf("Recv(fd=%d): %s\n", GetFd(), buffer);
      // send(GetFd(), buffer, strlen(buffer), 0);
      // 将接收到的数据追加到缓冲区
      m_InputBuffer.Append(buffer, nread);
    }
    else if (nread == -1 && errno == EINTR) {
      // 读取数据的时候被信号中断，继续读取
      continue;
    }
    else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 全部的数据已经读取完毕
      // 处理m_InputBuffer的信息
      printf("Recv(fd=%d): %s\n", GetFd(), m_InputBuffer.GetData());
      // 处理后将结果存放到m_OutputBuffer
      // 清空m_InputBuffer
      m_OutputBuffer.Clean();
      m_OutputBuffer.Append(m_InputBuffer.GetData(), m_InputBuffer.GetSize());
      m_InputBuffer.Clean();
      send(GetFd(), m_OutputBuffer.GetData(), m_OutputBuffer.GetSize(), 0);
      break;
    }
    else if (nread == 0) {
      // 调用关闭函数
      OnClose();
      break;
    }
  }
}