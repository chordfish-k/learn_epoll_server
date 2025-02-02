#pragma once

#include "http/HttpElement.h"
#include "net/Connection.h"
#include "net/Pointer.h"

#include <string>

class HttpResponse {
public:
  Version p_Version;
  Status p_Status;
  Header p_Headers;
  std::string p_Body;

public:
  HttpResponse(Status status, const Header &headers = Header(), const std::string &body = "");

  void Send(Ref<Connection> conn) const;

  std::string ToString() const;
};