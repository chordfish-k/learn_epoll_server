#include "HttpServer.h"

#include "HttpRequest.h"
#include "net/ThreadPool.h"

#include <cstdio>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

HttpServer::HttpServer(const std::string &ip, const uint16_t port,
                       int subThreadNum, int workThreadNum)
    : m_TcpServer(ip, port, subThreadNum),
      m_ThreadPool(workThreadNum, ThreadPool::Type::WORK) {

  m_TcpServer.SetNewConnectionCallback(
      std::bind(&HttpServer::HandleNewConnection, this, std::placeholders::_1));
  m_TcpServer.SetCloseConnectionCallback(std::bind(
      &HttpServer::HandleCloseConnection, this, std::placeholders::_1));
  m_TcpServer.SetErrorConnectionCallback(std::bind(
      &HttpServer::HandleErrorConnection, this, std::placeholders::_1));
  m_TcpServer.SetMessageCallback(std::bind(&HttpServer::HandleMessage, this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
  m_TcpServer.SetSendCompleteCallback(
      std::bind(&HttpServer::HandleSendComplete, this, std::placeholders::_1));
  m_TcpServer.SetEpollTimeoutCallback(
      std::bind(&HttpServer::OnEpollTimeout, this, std::placeholders::_1));
}

HttpServer::~HttpServer() {}

void HttpServer::Start() { m_TcpServer.Start(); }

void HttpServer::Stop() {
  // 停止工作线程
  m_ThreadPool.Stop();
  //  停止IO线程
  m_TcpServer.Stop();
}

void HttpServer::HandleNewConnection(Ref<Connection> conn) {}

void HttpServer::HandleCloseConnection(Ref<Connection> conn) {}

void HttpServer::HandleErrorConnection(Ref<Connection> conn) {}

void HttpServer::HandleMessage(Ref<Connection> conn, std::string &message) {
  if (m_ThreadPool.Size() == 0) {
    // 如果没有工作线程，表示在IO线程中计算
    OnMessage(conn, message);
  } else {
    // 把业务添加到任务队列中
    m_ThreadPool.AddTask(
        std::bind(&HttpServer::OnMessage, this, conn, message));
  }
}

void HttpServer::HandleSendComplete(Ref<Connection> conn) {
  std::cout << "Send Completed" << std::endl;
}

void HttpServer::OnEpollTimeout(EventLoop *loop) {}

void HttpServer::OnMessage(Ref<Connection> conn, std::string &message) {
  // 假设经过若干处理，得到结果
  // std::cout << message << std::endl;
  HttpRequset requset(message);

  // 路由
  std::string result;
  std::string body;
  if (requset.GetMethod() == HttpRequset::Method::GET) {
    if (requset.GetPath() == "/") {
      body = "<h1>Index</h1>";
    } else if (requset.GetPath() == "/test") {
      body = "<h1>Test</h1>";
    } else {
      body = "<h1>NotFound</h1>";
    }
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\nContent-length: " << body.size() << "\r\n\r\n"
       << body;
    result = ss.str();
  } else {
    printf("Error\n");
    result = "HTTP/1.1 404 Not Found\r\nContent-length: 0\r\n\r\n";
  }
  conn->Send(result.data(), result.size());
}