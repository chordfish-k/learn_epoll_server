#include "HttpRequest.h"
#include <cstdio>
#include <string>
#include <vector>

namespace Utils {

HttpRequset::Method String2Method(const std::string &str) {
  if (str == "GET")
    return HttpRequset::Method::GET;
  if (str == "POST")
    return HttpRequset::Method::POST;
  return HttpRequset::Method::UNKNOW;
}

HttpRequset::Version String2Version(const std::string &str) {
  if (str == "HTTP/1.1")
    return HttpRequset::Version::V1_1;
  if (str == "HTTP/2.0")
    return HttpRequset::Version::V2_0;
  return HttpRequset::Version::UNKNOW;
}

std::string Method2String(HttpRequset::Method method) {
  switch (method) {
  case HttpRequset::Method::GET:
    return "GET";
  case HttpRequset::Method::POST:
    return "POST";
  case HttpRequset::Method::UNKNOW:
    return "UNKNOW";
  }
  return "UNKNOW";
}

std::string Version2String(HttpRequset::Version version) {
  switch (version) {
  case HttpRequset::Version::V1_1:
    return "HTTP/1.1";
  case HttpRequset::Version::V2_0:
    return "HTTP/2.0";
  case HttpRequset::Version::UNKNOW:
    return "UNKNOW";
  }
  return "UNKNOW";
}

}; // namespace Utils

HttpRequset::HttpRequset(const Str &request) {
  m_Method = Method::GET;
  m_Version = Version::V1_1;
  m_Path = "HTTP/1.1";

  int processStatus = 0; // 0-请求行; 1-请求头; 2-请求体

  for (auto line : request.Split("\n")) {
    
    if (processStatus == 0 && line.Contains("HTTP")) {
      std::vector<Str> element = line.Trim().Split(" ");
      if (element.size() >= 1)
        m_Method = Utils::String2Method(element[0]);
      if (element.size() >= 2)
        m_Path = element[1];
      if (element.size() >= 3)
        m_Version = Utils::String2Version(element[2]);

      processStatus = 1;

    } else if (processStatus == 1 && line.Contains(":")) {

      std::vector<Str> element = line.Trim().Split(":");
      m_Headers[element[0].Trim()] = element[1].Trim();

    } else if (processStatus == 1 && line.ByteSize() == 0) {

      processStatus = 2;

    } else if (processStatus == 2) {

      m_Body += line;
    }
  }
}

// Getter
HttpRequset::Method HttpRequset::GetMethod() const { return m_Method; }
HttpRequset::Version HttpRequset::GetVersion() const { return m_Version; }
const std::string &HttpRequset::GetPath() const { return m_Path; }
const std::map<std::string, std::string> &HttpRequset::GetHeaders() const {
  return m_Headers;
}
const std::string &HttpRequset::GetBody() const { return m_Body; }