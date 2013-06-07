

#include "node_string.h"

namespace node {

using namespace v8;

extern Isolate* node_isolate;

Handle<String> ImmutableAsciiSource::CreateFromLiteral(
    const char *string_literal,
    size_t length) {
  HandleScope scope(node_isolate);

  Local<String> ret = String::NewExternal(new ImmutableAsciiSource(
      string_literal,
      length));
  return scope.Close(ret);
}

}
