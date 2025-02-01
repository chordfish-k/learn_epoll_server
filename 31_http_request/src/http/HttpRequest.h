#pragma once

#include "util/Str.h"

#include <map>
#include <string>

class HttpRequset {
public:
  enum class Method { GET, POST, UNKNOW };
  enum class Version { V1_1, V2_0, UNKNOW };

private:
  Method m_Method;
  std::string m_Path;
  Version m_Version;
  std::map<std::string, std::string> m_Headers;
  std::string m_Body;

public:
  HttpRequset(const Str &request);

  Method GetMethod() const;
  Version GetVersion() const;
  const std::string &GetPath() const;
  const std::map<std::string, std::string> &GetHeaders() const;
  const std::string &GetBody() const;
};

namespace Utils {

HttpRequset::Method String2Method(const std::string &str);
HttpRequset::Version String2Version(const std::string &str);
std::string Method2String(HttpRequset::Method method);
std::string Version2String(HttpRequset::Version version);

} // namespace Utils