#include "Connection.h"

#include "EventLoop.h"
#include "Socket.h"
#include "Timestamp.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

Connection::Connection(EventLoop* loop, Scope<Socket> clientSocket) 
  : m_Loop(loop), m_ClientSocket(std::move(clientSocket)), m_Disconnect(false) {
  // 为新客户端连接准备读事件，并添加到epoll
  m_ClientChannel = CreateScope<Channel>(m_Loop, m_ClientSocket->GetFd());
  m_ClientChannel->SetReadCallback(std::bind(&Connection::OnMessage, this));
  m_ClientChannel->SetWriteCallback(std::bind(&Connection::OnWrite, this));
  m_ClientChannel->SetCloseCallback(std::bind(&Connection::OnClose, this));
  m_ClientChannel->SetErrorCallback(std::bind(&Connection::OnError, this));
  m_ClientChannel->UseEdgeTrigger();         // 使用边缘触发
  m_ClientChannel->SetEnableReading(true); // 将Channel添加到Epoll
}

Connection::~Connection() {
  printf("Connection(fd=%d)已析构\n", GetFd());
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
  m_Disconnect = true;
  m_ClientChannel->Remove();  // 从事件循环中删除Channel
  if (m_CloseCallback)
    m_CloseCallback(shared_from_this());
}

void Connection::OnError() {
  m_Disconnect = true;
  m_ClientChannel->Remove();  // 从事件循环中删除Channel
  if (m_ErrorCallback)
    m_ErrorCallback(shared_from_this());
}

void Connection::OnMessage() {
  // 否则就是客户端连接的fd有事件
  // 接收缓冲区中有数据可以读
  char buffer[1024];
  while (true) {
    // 由于使用非阻塞IO，一次读取buffer大小数据，知道全部的数据读取完毕
    bzero(&buffer, sizeof(buffer));
    ssize_t nread = read(GetFd(), buffer, sizeof(buffer));
    if (nread > 0) {
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
      while (true) {
        // 根据报文长度逐个分离报文
        int len;
        memcpy(&len, m_InputBuffer.GetData(), 4);
        // 如果缓冲区中的长度小于报文长度，说明缓冲区中的数据不完整
        if (m_InputBuffer.GetSize() < len + 4) break;
        // 从缓冲区中取出第一个报文
        std::string message(m_InputBuffer.GetData() + 4, len);
        // 从缓冲区移除已经读取的数据
        m_InputBuffer.Erase(0, len + 4);

        printf("Message(fd=%d, thread=%ld): %s\n", GetFd(), syscall(SYS_gettid), message.c_str());

        // 更新时间戳
        m_LastTime = Timestamp::now();
        // std::cout << "LastTime=" << m_LastTime.toString() << std::endl;

        // 调用回调处理报文
        if (m_MessageCallback)
          m_MessageCallback(shared_from_this(), message);
      }
      break;
    }
    else if (nread == 0) {
      // 调用关闭函数
      OnClose();
      break;
    }
  }
}

void Connection::SetCloseCallback(std::function<void(Ref<Connection>)> fn) {
  m_CloseCallback = fn; 
}

void Connection::SetErrorCallback(std::function<void(Ref<Connection>)> fn) {
  m_ErrorCallback = fn;
}

void Connection::SetMessageCallback(std::function<void(Ref<Connection>, std::string&)> fn) {
  m_MessageCallback = fn;
}

void Connection::SetSendCompleteCallback(std::function<void(Ref<Connection>)> fn) {
  m_SendCompleteCallback = fn;
}

void Connection::Send(const char* data, size_t size) {
  if (m_Disconnect) {
    printf("Client(fd=%d) has diconnected.\n", GetFd());
    return;
  }

  std::string str(data, size);
  if (m_Loop->IsInLoopThread()) {
    // 如果当前线程是IO线程，直接执行发送数据的操作
    SendInLoop(str);
    printf("Send()在事件循环的线程中\n");
  }
  else {
    // 否则将发送数据的操作交给IO线程去执行，通过EventLoop::QueueInLoop()
    // SendInLoop(data, size);
    m_Loop->QueueInLoop(std::bind(&Connection::SendInLoop, this, str));
    printf("Send()不在事件循环的线程中\n");
  }
}

void Connection::SendInLoop(std::string data) {
  // 把需要发送的数据追加到缓冲区
  m_OutputBuffer.AppendWithHeader(data.data(), data.size());
  // 注册写事件
  m_ClientChannel->SetEnableWriting(true);
}

void Connection::OnWrite() {
  printf("Connection::OnWrite() thread is %ld\n", syscall(SYS_gettid));

  // 发送缓冲区的数据，返回已成功发送的字节数
  int written = send(GetFd(), m_OutputBuffer.GetData(), m_OutputBuffer.GetSize(), 0);
  if (written > 0) {
    // 从缓冲区中删除已发送的字节数
    m_OutputBuffer.Erase(0, written);
  }
  if (m_OutputBuffer.GetSize() == 0) {
    // 如果发送缓冲区中没有数据，表示数据发送成功，无需再监听写事件
    m_ClientChannel->SetEnableWriting(false);
    // 发送完成，调用回调
    if (m_SendCompleteCallback)
      m_SendCompleteCallback(shared_from_this());
  }
}

bool Connection::IsTimeout(time_t timeoutSecs, time_t now) {
  return now - m_LastTime.toInt() > timeoutSecs;
}