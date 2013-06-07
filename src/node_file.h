

#ifndef SRC_FILE_H_
#define SRC_FILE_H_

#include "node.h"
#include "v8.h"

namespace node {

class File {
 public:
  static void Initialize(v8::Handle<v8::Object> target);
};

void InitFs(v8::Handle<v8::Object> target);

}  // namespace node
#endif  // SRC_FILE_H_
