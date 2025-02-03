#pragma once

#include "http/HttpElement.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/Connection.h"
#include "net/Pointer.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

using KVs = std::map<std::string, std::string>;

struct RequestPattern {
  Method p_Method;
  std::string p_PathPattern;
  std::function<HttpResponse(KVs &params)> p_Handler;

  RequestPattern(Method method, const std::string &pathPattern,
                 std::function<HttpResponse(KVs& params)> handler);

private:
  std::vector<Str> m_PatternSplited;
  bool m_PatternCached;

  friend class HttpRouter;
};

class HttpRouter {
private:
  std::vector<RequestPattern> m_Pattern;

public:
  HttpRouter() = default;

  void Route(Ref<Connection> conn, HttpRequset &request);
  HttpRouter &AddPattern(RequestPattern pattern);

private:
  // 逐个检查RequestPattern，返回在m_Pattern中的序号，并将解析后的键值对存放在outKvs
  int CheckPathPattern(HttpRequset &request, KVs &outKvs);
  std::vector<Str> SplitPath(const std::string& path);
};