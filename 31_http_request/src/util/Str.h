#pragma once

#include <stdexcept>
#include <string>
#include <vector>

class Str {
public:
  // 单字符长度模式
  enum class CharWidth {
    ASCII, // 单字节字符（ASCII）
    UTF8   // 可变宽度字符（如UTF-8）
  };

  static const int NPOS = -1;

  // 构造函数
  Str() = default;
  Str(char ch);
  Str(const char *str, CharWidth width = CharWidth::ASCII);
  Str(const std::string &str, CharWidth width = CharWidth::ASCII);

  // 拷贝/移动构造函数和运算符
  Str(const Str &) = default;
  Str(Str &&) = default;
  Str &operator=(const Str &) = default;
  Str &operator=(Str &&) = default;

  // 基础访问方法
  const char *Data() const noexcept { return m_Data.c_str(); }
  bool IsEmpty() const noexcept { return m_Data.empty(); }
  size_t ByteSize() const noexcept { return m_Data.size(); }
  size_t Length() const noexcept { return m_Length; }
  bool IsBlank() const; // 判断是否为空白字符串

  // 字符串操作
  bool Contains(const Str &substr) const;
  std::vector<Str> Split(const Str &delimiter) const;
  std::vector<Str> Chars() const;
  Str SubStr(size_t pos, size_t len = NPOS) const;
  Str At(size_t index) const;
  void Append(const Str &str);
  void Insert(size_t pos, const Str &str);
  void Erase(size_t pos, size_t len = NPOS);

  // 运算符重载
  bool operator==(const Str &other) const;
  bool operator!=(const Str &other) const;
  Str operator+(const Str &other) const;
  explicit operator bool() const { return !IsEmpty(); } // 转bool值

  // 类型转换
  operator std::string() const { return m_Data; }
  std::string ToCString() const { return m_Data; }

private:
  std::string m_Data;    // 存储字符串数据
  size_t m_Length = 0;   // 字符长度（根据单字符长度模式计算）
  CharWidth m_CharWidth; // 单字符长度模式

  // 根据单字符长度模式计算字符长度
  size_t CalculateLength() const;
  size_t BytePosition(size_t pos) const;
};
