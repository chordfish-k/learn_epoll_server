#include "Buffer.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>

Buffer::Buffer(uint16_t sep) : 
  m_Sep(sep) {

}

Buffer::~Buffer() {
  
}

void Buffer::Append(const char* data, size_t size) {
  m_Buf.append(data, size);
}

void Buffer::AppendWithSeparator(const char* data, size_t size) {
  if (m_Sep == 0) {
    m_Buf.append(data, size);
  }
  else if (m_Sep == 1) {
    m_Buf.append((char*)&size, 4);  // 处理报文头部(长度)
    m_Buf.append(data, size);       // 处理报文内容
  }
  else if (m_Sep == 2) {
    //...
    printf("不支持的分隔符(sep=%d)\n", m_Sep);
  }
}

void Buffer::Erase(size_t pos, size_t nn) {
  m_Buf.erase(pos, nn);
}

void Buffer::Clean() {
  m_Buf.clear();
}
  
size_t Buffer::GetSize() {
  return m_Buf.size();
}
  
const char* Buffer::GetData() {
  return m_Buf.data();
}

bool Buffer::PickMessage(std::string& msg) {
  if (m_Buf.size() == 0) return false;

  if (m_Sep == 0) {
    // 没有分隔符，直接全部复制
    msg = m_Buf;
    m_Buf.clear();
  }
  else if (m_Sep == 1) {
    // 四字节表示长度
    int len;
    memcpy(&len, m_Buf.data(), 4);
    // 如果缓冲区中的长度小于报文长度，说明缓冲区中的数据不完整
    if (m_Buf.size() < len + 4) return false;
    // 从缓冲区中取出第一个报文
    msg = std::string(m_Buf.data() + 4, len);
    // 从缓冲区移除已经读取的数据
    m_Buf.erase(0, len + 4);
  }

  return true;
}