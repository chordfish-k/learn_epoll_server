#include "ThreadPool.h"

#include <cstddef>
#include <cstdio>
#include <functional>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>

// 构造函数，启动threadNum
ThreadPool::ThreadPool(size_t threadNum) {
  // 启动threadNum个线程，每个线程将阻塞在条件变量上
  for (size_t i = 0; i < threadNum; i++) {
    m_Threads.emplace_back([this]{
      printf("Create thread(%ld).\n", syscall(SYS_gettid));
    
      while (!m_Stop) {
        std::function<void()> task;

        {
          // 此处为同步代码块，加锁
          std::unique_lock<std::mutex> lock(this->m_Mtx);

          // 等待生产者的条件变量
          this->m_CondVar.wait(lock, [this]{
            // 条件：想要停止线程池，或任务队列不为空，则等待锁，继续执行下面的代码
            return this->m_Stop || !this->m_TaskQueue.empty();
          });

          // 在线程池停止之前，如果队列中还有任务，执行完再退出
          if (this->m_Stop && this->m_TaskQueue.empty()) return;

          // 出队一个任务，使用move移交所有权
          task = std::move(this->m_TaskQueue.front());
          this->m_TaskQueue.pop();
        }

        printf("Thread is %ld.\n", syscall(SYS_gettid));
        
        // 执行任务
        task();
      }
    });
  }
}

// 析构，停止全部线程
ThreadPool::~ThreadPool() {
  m_Stop = true;

  // 唤醒全部线程
  m_CondVar.notify_all();

  // 等待全部线程执行完成后退出
  for (std::thread& th : m_Threads)
    th.join();
}

  // 把任务添加到队列
void ThreadPool::AddTask(std::function<void()> task) {
  {
    // 加锁作用域，任务队列需要同步
    std::lock_guard<std::mutex> lock(m_Mtx);
    m_TaskQueue.push(task);
  }
  //唤醒一个线程
  m_CondVar.notify_one();
}

/*
void show(int no) {
  printf("no is %d\n", no);
}

void test() {
  printf("hello!\n");
}

// TEST
int main() {
  ThreadPool threadPool(3);

  threadPool.AddTask(std::bind(show, 8));
  sleep(1);

  threadPool.AddTask(std::bind(test));
  sleep(1);
  
  threadPool.AddTask(std::bind([]{printf("SB\n");}));
  sleep(1);
  return 0;
}
*/