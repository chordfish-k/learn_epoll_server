#pragma once

#include "http/HttpElement.h"
#include "util/Str.h"

#include <map>
#include <string>

class HttpRequset {
public:
  Method p_Method;
  std::string p_Path;
  Version p_Version;
  std::map<std::string, std::string> p_Headers;
  std::string p_Body;

public:
  HttpRequset(const Str &request);
};

