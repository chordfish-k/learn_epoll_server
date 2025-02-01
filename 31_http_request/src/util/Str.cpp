#include "Str.h"

#include <algorithm>
#include <stdexcept>

// 构造函数：从字符初始化
Str::Str(char ch) : m_CharWidth(CharWidth::ASCII) {
  m_Data = m_Data + ch;
  m_Length = 1;
}

// 构造函数：从C字符串初始化
Str::Str(const char *str, CharWidth width) : m_CharWidth(width) {
  if (str) {
    m_Data = str;
    m_Length = CalculateLength(); // 计算字符长度
  }
}

// 构造函数：从std::string初始化
Str::Str(const std::string &str, CharWidth width)
    : m_Data(str), m_CharWidth(width) {
  m_Length = CalculateLength(); // 计算字符长度
}

size_t Str::BytePosition(size_t pos) const {
  size_t bytePos = 0;
  if (m_CharWidth == CharWidth::UTF8) {
    // UTF-8模式需要计算字节位置
    size_t currentChar = 0;
    while (currentChar < pos && bytePos < m_Data.size()) {
      const unsigned char c = static_cast<unsigned char>(m_Data[bytePos]);
      bytePos += (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
      ++currentChar;
    }
  } else {
    // ASCII模式直接计算
    bytePos = pos;
  }
  return bytePos;
}

// 计算字符长度
size_t Str::CalculateLength() const {
  switch (m_CharWidth) {
  case CharWidth::ASCII:
    // 单字节模式：每个字符占1字节
    return m_Data.size();
  case CharWidth::UTF8:
    // 可变宽度模式：根据UTF-8编码计算字符长度
    size_t count = 0;
    for (size_t i = 0; i < m_Data.size();) {
      const unsigned char c = static_cast<unsigned char>(m_Data[i]);
      i += (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
      ++count;
    }
    return count;
  }
  return 0;
}

// 判断是否包含子字符串
bool Str::Contains(const Str &substr) const {
  return m_Data.find(substr.m_Data) != std::string::npos;
}

// 分割字符串
std::vector<Str> Str::Split(const Str &delimiter) const {
  std::vector<Str> result;
  const std::string &delim = delimiter.m_Data;
  size_t pos = 0, found;

  while ((found = m_Data.find(delim, pos)) != std::string::npos) {
    result.emplace_back(m_Data.substr(pos, found - pos), m_CharWidth);
    pos = found + delim.size();
  }
  result.emplace_back(m_Data.substr(pos), m_CharWidth);
  return result;
}

std::vector<Str> Str::Chars() const {
  std::vector<Str> result;
  result.reserve(m_Length);

  if (m_CharWidth == CharWidth::ASCII) {
    for (int i = 0; i < m_Length; ++i) {
      result.emplace_back(m_Data[i]);
    }
  } else if (m_CharWidth == CharWidth::UTF8) {
    size_t pos = 0;
    while (pos < m_Data.size()) {
      const unsigned char c = m_Data[pos];
      const int len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
      result.emplace_back(std::string(m_Data.data() + pos, len));
      pos += len;
    }
  }
  return result;
}

// 获取子字符串
Str Str::SubStr(size_t pos, size_t len) const {
  if (pos >= m_Length)
    return Str();

  size_t start = 0;
  size_t end = m_Data.size();

  switch (m_CharWidth) {
  case CharWidth::ASCII:
    start = pos;
    end = (len == NPOS) ? m_Data.size() : std::min(start + len, m_Data.size());
    break;
  case CharWidth::UTF8:
    // 可变宽度模式需要逐个字符计算
    size_t bytePos = 0;
    size_t currentChar = 0;
    while (currentChar < pos && bytePos < m_Data.size()) {
      const unsigned char c = static_cast<unsigned char>(m_Data[bytePos]);
      bytePos += (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
      ++currentChar;
    }
    start = bytePos;
    for (size_t i = 0; i < len && bytePos < m_Data.size(); ++i) {
      const unsigned char c = static_cast<unsigned char>(m_Data[bytePos]);
      bytePos += (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
    }
    end = bytePos;
    break;
  }

  return Str(m_Data.substr(start, end - start), m_CharWidth);
}

// 获取单个字符
Str Str::At(size_t index) const { return SubStr(index, 1); }

// 追加字符串
void Str::Append(const Str &str) {
  m_Data.append(str.m_Data);
  m_Length += str.m_Length;
}

// 插入字符串
void Str::Insert(size_t pos, const Str &str) {
  if (pos > m_Length)
    throw std::out_of_range("Position out of range");

  const size_t bytePos = BytePosition(pos);
  m_Data.insert(bytePos, str.m_Data);
  m_Length += str.m_Length;
}

void Str::Erase(size_t pos, size_t len) {
  if (pos >= m_Length)
    throw std::out_of_range("Position out of range");

  const size_t start = BytePosition(pos);
  const size_t end = (len == NPOS) ? m_Data.size() : BytePosition(pos + len);

  m_Data.erase(start, end - start);
  m_Length = CalculateLength(); // 重新计算字符长度
}

// 删除头尾空白字符
Str &Str::Trim() {
  size_t start = 0;
  size_t end = m_Data.size();

  while (start < m_Data.size() &&
         std::isspace(static_cast<unsigned char>(m_Data[start]))) {
    ++start;
  }
  while (end > 0 && std::isspace(static_cast<unsigned char>(m_Data[end - 1]))) {
    --end;
  }

  if (start > 0) {
    m_Data = m_Data.substr(start, end - start + 1);
    m_Length = CalculateLength(); // 重新计算字符长度
  }
  return *this;
}

// 判断是否为空白字符串
bool Str::IsBlank() const {
  for (char c : m_Data) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

// 运算符重载：相等比较
bool Str::operator==(const Str &other) const { return m_Data == other.m_Data; }

// 运算符重载：不等比较
bool Str::operator!=(const Str &other) const { return m_Data != other.m_Data; }

// 运算符重载：字符串拼接
Str Str::operator+(const Str &other) const {
  Str result(*this);
  result.Append(other);
  return result;
}