#include "HttpElement.h"
#include <cstdint>
#include <cstdlib>
#include <sstream>

namespace HttpUtils {

Method String2Method(const std::string &str) {
  if (str == "GET")
    return Method::GET;
  if (str == "POST")
    return Method::POST;
  return Method::UNKNOW;
}

Version String2Version(const std::string &str) {
  if (str == "HTTP/1.1")
    return Version::V1_1;
  if (str == "HTTP/2.0")
    return Version::V2_0;
  return Version::UNKNOW;
}

Status StringCode2Status(const std::string &str) {
  uint16_t code = (uint16_t)atoi(str.c_str());
  switch (code) {
  case 200:
    return Status::OK;
  case 400:
    return Status::BAD_REQUEST;
  case 404:
    return Status::NOT_FOUND;
  case 500:
    return Status::INTERNEL_SERVER_ERROR;
  }
  return Status::NOT_FOUND;
}

std::string Method2String(Method method) {
  switch (method) {
  case Method::GET:
    return "GET";
  case Method::POST:
    return "POST";
  case Method::UNKNOW:
    return "UNKNOW";
  }
  return "UNKNOW";
}

std::string Version2String(Version version) {
  switch (version) {
  case Version::V1_1:
    return "HTTP/1.1";
  case Version::V2_0:
    return "HTTP/2.0";
  case Version::UNKNOW:
    return "UNKNOW";
  }
  return "UNKNOW";
}

std::string Status2String(Status status) {
  switch (status) {
  case Status::OK:
    return "OK";
  case Status::BAD_REQUEST:
    return "Bad Request";
  case Status::NOT_FOUND:
    return "Not Found";
  case Status::INTERNEL_SERVER_ERROR:
    return "Internel Server Error";
  }
  return "Not Found";
}

std::string Header2String(const Header &header) {
  std::stringstream ss;
  for (auto &[k, v] : header) {
    ss << k << ":" << v << "\r\n";
  }
  return ss.str();
}

}; // namespace HttpUtils