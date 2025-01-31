#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

int main(int argc, const char** argv) {
    
  if (argc != 3) {
    printf("Usage: ./Client <ip> <port>\n");
    printf("Example: ./Client 127.0.0.1 5005\n\n");
    return -1;
  }

  int clientFd;
  sockaddr_in serverAddr;
  char buffer[1024];

  // 创建客户端socket
  if ((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()"); return -1;
  }

  // 准备药连接到的服务端的地址信息
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(atoi(argv[2]));
  serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

  // 连接到服务端
  if (connect(clientFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
    printf("Connent(%s:%s) Failed.", argv[1], argv[2]);
    close(clientFd);
    return -1;
  }

  printf("Connect(%s:%s) OK.\n", argv[1], argv[2]);

  printf("开始时间: %ld\n", time(0));
  for (int i = 0; i < 100000; ++i) {
    // 从命令行输入内容
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "这是第%03d条信息。", i);

    char tmpBuf[1024];
    memset(tmpBuf, 0, sizeof(tmpBuf));

    int len = strlen(buffer);
    memcpy(tmpBuf, &len, 4);
    memcpy(tmpBuf + 4, buffer, len);

    // 将命令行的输入发送给服务端
    send(clientFd, tmpBuf, len + 4, 0);

    // 读取报文头部
    recv(clientFd, &len, 4, 0);
    memset(buffer, 0, sizeof(buffer));
    // 接收服务端的回应
    recv(clientFd, buffer, len, 0);

    // printf("Recv: %s\n", buffer);
  }

  printf("结束时间: %ld\n", time(0));

  return 0;
}
