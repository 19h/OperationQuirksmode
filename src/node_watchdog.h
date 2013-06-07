

#ifndef SRC_NODE_WATCHDOG_H_
#define SRC_NODE_WATCHDOG_H_

#include "v8.h"
#include "uv.h"

namespace node {

class Watchdog {
 public:
  Watchdog(uint64_t ms);
  ~Watchdog();

  void Dispose();

 private:
  void Destroy();

  static void Run(void* arg);
  static void Async(uv_async_t* async, int status);
  static void Timer(uv_timer_t* timer, int status);

  uv_thread_t thread_;
  uv_loop_t* loop_;
  uv_async_t async_;
  uv_timer_t timer_;
  bool thread_created_;
  bool destroyed_;
};

}  // namespace node

#endif  // SRC_NODE_WATCHDOG_H_
