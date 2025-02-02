#include "HttpResponse.h"
#include "http/HttpElement.h"

#include <cstdint>
#include <sstream>
#include <string>

HttpResponse::HttpResponse(Status status, const Header &headers,
                           const std::string &body)
    : p_Status(status), p_Headers(std::move(headers)), p_Body(body),
      p_Version(Version::V1_1) {}

void HttpResponse::Send(Ref<Connection> conn) const {
  std::string response = ToString();
  conn->Send(response.data(), response.size());
}

std::string HttpResponse::ToString() const {
  std::stringstream ss;
  // 响应行
  ss << HttpUtils::Version2String(p_Version) << " ";
  ss << (uint16_t)p_Status << " ";
  ss << HttpUtils::Status2String(p_Status) << "\r\n";
  // 响应头
  ss << HttpUtils::Header2String(p_Headers);
  // 响应头-Content-Lengths
  ss << "Content-Length:" << p_Body.length() << "\r\n\r\n";
  // 响应体
  ss << p_Body;
  return ss.str();
}