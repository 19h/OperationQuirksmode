

#ifndef HANDLE_WRAP_H_
#define HANDLE_WRAP_H_

#include "queue.h"

namespace node {

// Rules:
//
// - Do not throw from handle methods. Set errno.
//
// - MakeCallback may only be made directly off the event loop.
//   That is there can be no JavaScript stack frames underneith it.
//   (Is there anyway to assert that?)
//
// - No use of v8::WeakReferenceCallback. The close callback signifies that
//   we're done with a handle - external resources can be freed.
//
// - Reusable?
//
// - The uv_close_cb is used to free the c++ object. The close callback
//   is not made into javascript land.
//
// - uv_ref, uv_unref counts are managed at this layer to avoid needless
//   js/c++ boundary crossing. At the javascript layer that should all be
//   taken care of.

#define UNWRAP_NO_ABORT(type)                                               \
  assert(!args.This().IsEmpty());                                           \
  assert(args.This()->InternalFieldCount() > 0);                            \
  type* wrap = static_cast<type*>(                                          \
      args.This()->GetAlignedPointerFromInternalField(0));

class HandleWrap {
  public:
    static void Initialize(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> Close(const v8::Arguments& args);
    static v8::Handle<v8::Value> Ref(const v8::Arguments& args);
    static v8::Handle<v8::Value> Unref(const v8::Arguments& args);

    inline uv_handle_t* GetHandle() { return handle__; };

  protected:
    HandleWrap(v8::Handle<v8::Object> object, uv_handle_t* handle);
    virtual ~HandleWrap();

    v8::Persistent<v8::Object> object_;

  private:
    friend v8::Handle<v8::Value> GetActiveHandles(const v8::Arguments&);
    static void OnClose(uv_handle_t* handle);
    QUEUE handle_wrap_queue_;
    // Using double underscore due to handle_ member in tcp_wrap. Probably
    // tcp_wrap should rename it's member to 'handle'.
    uv_handle_t* handle__;
    unsigned int flags_;

    static const unsigned int kUnref = 1;
    static const unsigned int kCloseCallback = 2;
};


}  // namespace node


#endif  // HANDLE_WRAP_H_
