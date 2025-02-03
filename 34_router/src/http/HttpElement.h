#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class Method { GET, POST, UNKNOW };

enum class Version { V1_1, V2_0, UNKNOW };

enum class Status : uint16_t {
  OK = 200,
  BAD_REQUEST = 400,
  NOT_FOUND = 404,
  INTERNEL_SERVER_ERROR = 500
};

using Header = std::map<std::string, std::string>;

namespace HttpUtils {

Method String2Method(const std::string &str);
Version String2Version(const std::string &str);
Status StringCode2Status(const std::string &str);

std::string Method2String(Method method);
std::string Version2String(Version version);
std::string Status2String(Status status);
std::string Header2String(const Header &header);
} // namespace HttpUtils
