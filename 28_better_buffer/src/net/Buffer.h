#pragma once

#include <cstddef>
#include <string>
#include <iostream>

// 简单的缓冲区
class Buffer
{
private:
  std::string m_Buf;                          // 实际缓冲容器
public:
  Buffer();
  ~Buffer();

  void Append(const char* data, size_t size);           // 追加数据到缓冲
  void AppendWithHeader(const char* data, size_t size); // 追加数据到缓冲，并附加报文头部
  void Erase(size_t pos, size_t nn);                    // 从pos位置开始删除nn个字节
  void Clean();                                         // 清空缓冲
  
  size_t GetSize();                                     // 返回m_Buf大小
  const char* GetData();                                // 返回m_Buf首地址
};