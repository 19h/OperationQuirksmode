

#ifndef STREAM_WRAP_H_
#define STREAM_WRAP_H_

#include "v8.h"
#include "node.h"
#include "handle_wrap.h"
#include "string_bytes.h"

namespace node {

// Forward declaration
class WriteWrap;


class StreamWrap : public HandleWrap {
 public:
  uv_stream_t* GetStream() { return stream_; }

  static void Initialize(v8::Handle<v8::Object> target);

  static v8::Handle<v8::Value> GetFD(v8::Local<v8::String>,
                                     const v8::AccessorInfo&);

  // JavaScript functions
  static v8::Handle<v8::Value> ReadStart(const v8::Arguments& args);
  static v8::Handle<v8::Value> ReadStop(const v8::Arguments& args);
  static v8::Handle<v8::Value> Shutdown(const v8::Arguments& args);

  static v8::Handle<v8::Value> Writev(const v8::Arguments& args);
  static v8::Handle<v8::Value> WriteBuffer(const v8::Arguments& args);
  static v8::Handle<v8::Value> WriteAsciiString(const v8::Arguments& args);
  static v8::Handle<v8::Value> WriteUtf8String(const v8::Arguments& args);
  static v8::Handle<v8::Value> WriteUcs2String(const v8::Arguments& args);

 protected:
  static size_t WriteBuffer(v8::Handle<v8::Value> val, uv_buf_t* buf);

  StreamWrap(v8::Handle<v8::Object> object, uv_stream_t* stream);
  void StateChange() { }
  void UpdateWriteQueueSize();

 private:
  static inline char* NewSlab(v8::Handle<v8::Object> global, v8::Handle<v8::Object> wrap_obj);

  // Callbacks for libuv
  static void AfterWrite(uv_write_t* req, int status);
  static uv_buf_t OnAlloc(uv_handle_t* handle, size_t suggested_size);
  static void AfterShutdown(uv_shutdown_t* req, int status);

  static void OnRead(uv_stream_t* handle, ssize_t nread, uv_buf_t buf);
  static void OnRead2(uv_pipe_t* handle, ssize_t nread, uv_buf_t buf,
      uv_handle_type pending);
  static void OnReadCommon(uv_stream_t* handle, ssize_t nread,
      uv_buf_t buf, uv_handle_type pending);

  template <enum encoding encoding>
  static v8::Handle<v8::Value> WriteStringImpl(const v8::Arguments& args);

  size_t slab_offset_;
  uv_stream_t* stream_;
};


}  // namespace node


#endif  // STREAM_WRAP_H_
