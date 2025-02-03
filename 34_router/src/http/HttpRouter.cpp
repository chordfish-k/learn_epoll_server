#include "HttpRouter.h"
#include "http/HttpElement.h"
#include "util/Str.h"
#include <cstdio>
#include <vector>

RequestPattern::RequestPattern(Method method, const std::string &pathPattern,
                               std::function<HttpResponse(KVs &params)> handler)
    : p_Method(method), p_PathPattern(pathPattern), p_Handler(handler),
      m_PatternCached(false) {}

void HttpRouter::Route(Ref<Connection> conn, HttpRequset &request) {
  KVs outKvs;
  int idx = CheckPathPattern(request, outKvs);
  if (idx == -1) {
    // 404
    HttpResponse(Status::NOT_FOUND).Send(conn);
  } else {
    RequestPattern pattern = m_Pattern[idx];
    pattern.p_Handler(outKvs).Send(conn);
  }
}

HttpRouter &HttpRouter::AddPattern(RequestPattern pattern) {
  m_Pattern.push_back(pattern);
  return *this;
}

int HttpRouter::CheckPathPattern(HttpRequset &request, KVs &outKvs) {
  std::vector<Str> pathSplited = SplitPath(request.p_Path);

  for (int idx = 0; idx < m_Pattern.size(); ++idx) {
    auto &pattern = m_Pattern[idx];

    // 先判断请求类型
    if (request.p_Method != pattern.p_Method)
      continue;

    auto &arrPath = pathSplited;
    auto &arrPatt = pattern.m_PatternSplited;

    // 如果模式没有分割，先分割
    if (!pattern.m_PatternCached) {
      arrPatt = SplitPath(pattern.p_PathPattern);
      pattern.m_PatternCached = true;
    }

    // 长度不一致
    if (arrPath.size() != arrPatt.size())
      continue;

    bool matched = true;

    for (int i = 0; i < arrPath.size(); ++i) {
      Str &tokenA = arrPath[i];
      Str &tokenB = arrPatt[i];
      
      if (tokenB.At(0) == "{" && tokenB.At(tokenB.Length() - 1) == "}") {
        // 如果是需要匹配的key，提取kv
        Str keyName = tokenB.SubStr(1, tokenB.Length() - 2);
        outKvs[keyName] = tokenA;

      } else if (tokenA != tokenB) {
        // 非key部分，直接判断，如果不相等，清空kvs，外部进入下一次循环
        outKvs.clear();
        matched = false;
        break;
      }
      // 相等，则继续判断下一个元素
    }

    if (!matched)
      continue;

    // 走到这一步也就是完全匹配
    return idx;
  }
  return -1;
}

std::vector<Str> HttpRouter::SplitPath(const std::string& path) {
  std::vector<Str> pathSplited = Str(path).Split("/");
  pathSplited.erase(pathSplited.begin());
  if (pathSplited[pathSplited.size() - 1] == "") pathSplited.pop_back();
  return pathSplited;
}