

#ifndef SRC_STRING_BYTES_H_
#define SRC_STRING_BYTES_H_

// Decodes a v8::Handle<v8::String> or Buffer to a raw char*

#include "v8.h"
#include "node.h"

namespace node {

using v8::Handle;
using v8::Local;
using v8::String;
using v8::Value;

class StringBytes {
 public:
  // Fast, but can be 2 bytes oversized for Base64, and
  // as much as triple UTF-8 strings <= 65536 chars in length
  static size_t StorageSize(Handle<Value> val, enum encoding enc);

  // Precise byte count, but slightly slower for Base64 and
  // very much slower for UTF-8
  static size_t Size(Handle<Value> val, enum encoding enc);

  // Write the bytes from the string or buffer into the char*
  // returns the number of bytes written, which will always be
  // <= buflen.  Use StorageSize/Size first to know how much
  // memory to allocate.
  static size_t Write(char* buf,
                      size_t buflen,
                      Handle<Value> val,
                      enum encoding enc,
                      int* chars_written = NULL);

  // Take the bytes in the src, and turn it into a Buffer or String.
  static Local<Value> Encode(const char* buf,
                             size_t buflen,
                             enum encoding encoding);
};

}  // namespace node

#endif  // SRC_STRING_BYTES_H_
