#include "HttpRequest.h"

#include <cstdio>
#include <string>
#include <vector>

HttpRequset::HttpRequset(const Str &request) {
  p_Method = Method::GET;
  p_Version = Version::V1_1;
  p_Path = "HTTP/1.1";

  int processStatus = 0; // 0-请求行; 1-请求头; 2-请求体

  for (auto line : request.Split("\n")) {

    if (processStatus == 0 && line.Contains("HTTP")) {
      std::vector<Str> element = line.Trim().Split(" ");
      if (element.size() >= 1)
        p_Method = HttpUtils::String2Method(element[0]);
      if (element.size() >= 2)
        p_Path = element[1];
      if (element.size() >= 3)
        p_Version = HttpUtils::String2Version(element[2]);

      processStatus = 1;

    } else if (processStatus == 1 && line.Contains(":")) {

      std::vector<Str> element = line.Trim().Split(":");
      p_Headers[element[0].Trim()] = element[1].Trim();

    } else if (processStatus == 1 && line.ByteSize() == 0) {

      processStatus = 2;

    } else if (processStatus == 2) {

      p_Body += line;
    }
  }
}
