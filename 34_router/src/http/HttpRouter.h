#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/Connection.h"
#include "net/Pointer.h"

#include <functional>
#include <map>
#include <string>

class HttpRouter {
public:
  std::map<std::string, std::function<HttpResponse()>> p_Pattern;

public:
  HttpRouter() = default;

  void Route(Ref<Connection> conn, HttpRequset& request);
};