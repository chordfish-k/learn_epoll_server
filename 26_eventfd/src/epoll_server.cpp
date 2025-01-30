#include "net/EchoServer.h"

int main(int argc, const char** argv) {
  
  if (argc != 3) {
    printf("Usage: ./EpollServer <ip> <port>\n");
    printf("Example: ./EpollServer 127.0.0.1 5005\n\n");
    return -1;
  }

  //TcpServer tcpServer(argv[1], atoi(argv[2]));
  //tcpServer.Start();
  
  EchoServer echoServer(argv[1], atoi(argv[2]), 3, 3);
  echoServer.Start();
  
  return 0;
}
