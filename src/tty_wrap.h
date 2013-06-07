

#ifndef TTY_WRAP_H_
#define TTY_WRAP_H_

#include "handle_wrap.h"
#include "stream_wrap.h"

namespace node {

using v8::Object;
using v8::Handle;
using v8::Local;
using v8::Value;
using v8::Arguments;


class TTYWrap : StreamWrap {
 public:
  static void Initialize(Handle<Object> target);
  static TTYWrap* Unwrap(Local<Object> obj);

  uv_tty_t* UVHandle();

 private:
  TTYWrap(Handle<Object> object, int fd, bool readable);

  static Handle<Value> GuessHandleType(const Arguments& args);
  static Handle<Value> IsTTY(const Arguments& args);
  static Handle<Value> GetWindowSize(const Arguments& args);
  static Handle<Value> SetRawMode(const Arguments& args);
  static Handle<Value> New(const Arguments& args);

  uv_tty_t handle_;
};

} // namespace node

#endif // TTY_WRAP_H_
