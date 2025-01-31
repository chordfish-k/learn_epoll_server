#pragma once 

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <future>

class ThreadPool
{
public:
  enum class Type : uint8_t
  {
    IO = 0,
    WORK
  };
private:
  std::vector<std::thread> m_Threads;             // 线程池中的线程
  std::queue<std::function<void()>> m_TaskQueue;  // 任务队列
  std::mutex m_Mtx;                               // 任务队列同步的互斥锁
  std::condition_variable m_CondVar;              // 任务队列同步的条件变量
  std::atomic_bool m_Stop;                // 在析构函数中，把m_Stop设为true，全部线程将退出
  Type m_ThreadType;                       // 线程种类
public:
  // 构造函数，启动threadNum个线程
  ThreadPool(size_t threadNum, const ThreadPool::Type threadType = ThreadPool::Type::IO);
  // 析构，停止全部线程
  ~ThreadPool();

  // 把任务添加到队列
  void AddTask(std::function<void()> task);

  size_t Size() const;

  // 停止线程
  void Stop();
};