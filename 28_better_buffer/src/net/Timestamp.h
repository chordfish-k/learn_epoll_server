#pragma once

#include <ctime>
#include <iostream>
#include <string>

class Timestamp
{
private:
  time_t m_SecsScineEpoch;            // 整数表示事件(1970年到现在的秒数)
public:
  Timestamp();                        // 用当前时间初始化
  Timestamp(int64_t secsScineEpoch);  // 用一个整数初始化

  static Timestamp now();             // 返回当前时间的Timestamp对象

  time_t toInt() const;               // 返回整数表示的时间
  std::string toString() const;       // 返回字符串表示的时间，格式:yyyy-MM-dd hh24:mm:ss
};