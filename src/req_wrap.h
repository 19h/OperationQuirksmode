

#ifndef REQ_WRAP_H_
#define REQ_WRAP_H_

#include "queue.h"
#include "node_internals.h"

namespace node {

// defined in node.cc
extern v8::Persistent<v8::String> process_symbol;
extern v8::Persistent<v8::String> domain_symbol;
extern QUEUE req_wrap_queue;

template <typename T>
class ReqWrap {
 public:
  ReqWrap() {
    v8::HandleScope scope(node_isolate);
    object_ = v8::Persistent<v8::Object>::New(node_isolate, v8::Object::New());

    if (using_domains) {
      v8::Local<v8::Value> domain = v8::Context::GetCurrent()
                                    ->Global()
                                    ->Get(process_symbol)
                                    ->ToObject()
                                    ->Get(domain_symbol);

      if (!domain->IsUndefined()) {
        object_->Set(domain_symbol, domain);
      }
    }

    QUEUE_INSERT_TAIL(&req_wrap_queue, &req_wrap_queue_);
  }


  ~ReqWrap() {
    QUEUE_REMOVE(&req_wrap_queue_);
    // Assert that someone has called Dispatched()
    assert(req_.data == this);
    assert(!object_.IsEmpty());
    object_.Dispose(node_isolate);
    object_.Clear();
  }

  // Call this after the req has been dispatched.
  void Dispatched() {
    req_.data = this;
  }

  v8::Persistent<v8::Object> object_;
  QUEUE req_wrap_queue_;
  void* data_;
  T req_; // *must* be last, GetActiveRequests() in node.cc depends on it
};


}  // namespace node


#endif  // REQ_WRAP_H_
