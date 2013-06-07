

#ifndef NODE_STAT_WATCHER_H_
#define NODE_STAT_WATCHER_H_

#include "node.h"
#include "uv.h"

namespace node {

class StatWatcher : ObjectWrap {
 public:
  static void Initialize(v8::Handle<v8::Object> target);

 protected:
  static v8::Persistent<v8::FunctionTemplate> constructor_template;

  StatWatcher();
  virtual ~StatWatcher();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Start(const v8::Arguments& args);
  static v8::Handle<v8::Value> Stop(const v8::Arguments& args);

 private:
  static void Callback(uv_fs_poll_t* handle,
                       int status,
                       const uv_stat_t* prev,
                       const uv_stat_t* curr);
  void Stop();

  uv_fs_poll_t* watcher_;
};

}  // namespace node
#endif  // NODE_STAT_WATCHER_H_
