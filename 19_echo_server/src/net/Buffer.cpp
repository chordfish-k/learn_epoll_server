#include "Buffer.h"

Buffer::Buffer() {

}

Buffer::~Buffer() {
  
}

void Buffer::Append(const char* data, size_t size) {
  m_Buf.append(data, size);
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