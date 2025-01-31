#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <iostream>

// 简单的缓冲区
class Buffer
{
private:
  std::string m_Buf;    // 用于存放数据
  const uint16_t m_Sep; // 报文的分隔符：0-无分隔符(固定长度); 1-四字节报头; 2- "\r\n\r\n"(http协议)
public:
  Buffer(uint16_t sep = 1);
  ~Buffer();

  void Append(const char* data, size_t size);           // 追加数据到缓冲
  void AppendWithSeparator(const char* data, size_t size); // 追加数据到缓冲，并附加报文头部
  
  void Erase(size_t pos, size_t nn);                    // 从pos位置开始删除nn个字节
  void Clean();                                         // 清空缓冲
  
  size_t GetSize();                                     // 返回m_Buf大小
  const char* GetData();                                // 返回m_Buf首地址

  bool PickMessage(std::string& msg);   // 从m_Buf中拆分出一个报文，存放在msg中，如果m_Buf没有报文，返回false
};