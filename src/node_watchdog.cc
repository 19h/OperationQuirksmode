

#include "node_watchdog.h"
#include <assert.h>

namespace node {

using v8::V8;


Watchdog::Watchdog(uint64_t ms)
  : thread_created_(false)
  , destroyed_(false) {

  loop_ = uv_loop_new();
  if (!loop_)
    return;

  int rc = uv_async_init(loop_, &async_, &Watchdog::Async);
  assert(rc == 0);

  rc = uv_timer_init(loop_, &timer_);
  assert(rc == 0);

  rc = uv_timer_start(&timer_, &Watchdog::Timer, ms, 0);
  if (rc) {
    return;
  }

  rc = uv_thread_create(&thread_, &Watchdog::Run, this);
  if (rc) {
    return;
  }
  thread_created_ = true;
}


Watchdog::~Watchdog() {
  Destroy();
}


void Watchdog::Dispose() {
  Destroy();
}


void Watchdog::Destroy() {
  if (destroyed_) {
    return;
  }

  if (thread_created_) {
    uv_async_send(&async_);
    uv_thread_join(&thread_);
  }

  if (loop_) {
    uv_loop_delete(loop_);
  }

  destroyed_ = true;
}


void Watchdog::Run(void* arg) {
  Watchdog* wd = static_cast<Watchdog*>(arg);

  // UV_RUN_ONCE so async_ or timer_ wakeup exits uv_run() call.
  uv_run(wd->loop_, UV_RUN_ONCE);

  // Loop ref count reaches zero when both handles are closed.
  uv_close(reinterpret_cast<uv_handle_t*>(&wd->async_), NULL);
  uv_close(reinterpret_cast<uv_handle_t*>(&wd->timer_), NULL);

  // UV_RUN_DEFAULT so that libuv has a chance to clean up.
  uv_run(wd->loop_, UV_RUN_DEFAULT);
}


void Watchdog::Async(uv_async_t* async, int status) {
}


void Watchdog::Timer(uv_timer_t* timer, int status) {
  V8::TerminateExecution();
}


}  // namespace node
