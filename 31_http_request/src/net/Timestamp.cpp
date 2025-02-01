#include "Timestamp.h"

#include <cstdio>
#include <ctime>

Timestamp::Timestamp() {
  m_SecsScineEpoch = time(0);  // 去系统当前时间
}

Timestamp::Timestamp(int64_t secsScineEpoch) 
  : m_SecsScineEpoch(secsScineEpoch) {
}

Timestamp Timestamp::now() {
  return Timestamp();   // 返回当前时间
}

time_t Timestamp::toInt() const {
  return m_SecsScineEpoch;
}

std::string Timestamp::toString() const {
  // 将time_t格式化为字符串
  char buf[72] = {0};
  tm* tmTime = localtime(&m_SecsScineEpoch);
  snprintf(buf, 72, "%4d-%02d-%02d %02d:%02d:%02d",
    tmTime->tm_year + 1900,
    tmTime->tm_mon + 1,
    tmTime->tm_mday,
    tmTime->tm_hour,
    tmTime->tm_min,
    tmTime->tm_sec);
  return buf;
}
