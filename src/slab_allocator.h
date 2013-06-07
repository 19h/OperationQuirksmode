

#include "v8.h"

namespace node {

class SlabAllocator {
public:
  SlabAllocator(unsigned int size = 10485760); // default to 10M
  ~SlabAllocator();

  // allocate memory from slab, attaches the slice to `obj`
  char* Allocate(v8::Handle<v8::Object> obj, unsigned int size);

  // return excess memory to the slab, returns a handle to the parent buffer
  v8::Local<v8::Object> Shrink(v8::Handle<v8::Object> obj,
                               char* ptr,
                               unsigned int size);

private:
  void Initialize();
  bool initialized_;
  v8::Persistent<v8::Object> slab_;
  v8::Persistent<v8::String> slab_sym_;
  unsigned int offset_;
  unsigned int size_;
  char* last_ptr_;
};

} // namespace node
