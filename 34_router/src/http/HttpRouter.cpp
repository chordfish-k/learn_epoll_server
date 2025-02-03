#include "HttpRouter.h"

void HttpRouter::Route(Ref<Connection> conn, HttpRequset &request) {
  // 路由
  std::string body;
  Status status = Status::OK;
  Header header;
  header["Content-Type"] = "text/html";

  if (request.p_Method == Method::GET) {
    if (request.p_Path == "/") {
      body = "<h1>Index</h1>";
    } else if (request.p_Path == "/test") {
      body = "<h1>Test</h1>";
    } else if (request.p_Path == "/json") {
      body = "{\"data\":\"Some Text Here\"}";
      header["Content-Type"] = "text/json";
    } else {
      body = "<h1>NotFound</h1>";
      status = Status::NOT_FOUND;
    }
    HttpResponse response(status, header, body);
    response.Send(conn);
  } else {
    HttpResponse response(Status::NOT_FOUND, header);
    response.Send(conn);
  }
}