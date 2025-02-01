#include "util/Str.h"

#include <iostream>

void TestStr1() {
  // ASCII模式
  Str s1("Hello", Str::CharWidth::ASCII);
  Str s2(" World!", Str::CharWidth::ASCII);
  Str s3 = s1 + s2;                    // 字符串拼接
  std::cout << s3.Data() << std::endl; // 输出 "Hello World!"

  // UTF-8模式
  Str s4("你好", Str::CharWidth::UTF8);
  Str s5("，世界！", Str::CharWidth::UTF8);
  s4.Append(s5);                       // 追加字符串
  std::cout << s4.Data() << std::endl; // 输出 "你好，世界！"

  // 插入字符串
  s4.Insert(2, "，C++");
  std::cout << s4.Data() << std::endl; // 输出 "你好，C++，世界！"

  // 判断空字符串和空白字符串
  Str s6("");
  Str s7("   ");
  std::cout << std::boolalpha;
  std::cout << "IsEmpty: " << s6.IsEmpty() << " "        // 输出 true
            << "IsBlank: " << s6.IsBlank() << std::endl; // 输出 true
  std::cout << "IsEmpty: " << s7.IsEmpty() << " "        // 输出 false
            << "IsBlank: " << s7.IsBlank() << std::endl; // 输出 true

  // 转bool值
  if (s1) {
    std::cout << "s1 is not empty" << std::endl;
  }
}

void TestStr2() {
  // UTF-8模式
  Str s1("你好，世界！", Str::CharWidth::UTF8);

  // 遍历字符
  for (auto ch : s1.Chars()) {
    std::cout << ch.Data() << " ";
  }
  std::cout << std::endl; // 输出 "你 好 ， 世 界 ！ "

  // 删除部分字符串
  s1.Erase(2, 2);                      // 删除 "，世"
  std::cout << s1.Data() << std::endl; // 输出 "你好界！"
}

void TestStr3() {
  Str request = R"(GET / HTTP/1.1
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate, br, zstd
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7
Cache-Control: no-cache
Connection: keep-alive)";

  for (auto ch : request.Split("\n")) {
    std::cout  << "> " << ch.Data() << "\n";
  }
}

int main() {

  TestStr1();
  TestStr2();
  TestStr3();

  return 0;
}