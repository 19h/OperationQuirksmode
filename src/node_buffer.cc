


#include "node_buffer.h"

#include "node.h"
#include "string_bytes.h"

#include "v8.h"
#include "v8-profiler.h"

#include <assert.h>
#include <string.h> // memcpy
#include <limits.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define BUFFER_CLASS_ID (0xBABE)

namespace node {

using namespace v8;

#define SLICE_ARGS(start_arg, end_arg)                               \
  if (!start_arg->IsInt32() || !end_arg->IsInt32()) {                \
    return ThrowException(Exception::TypeError(                      \
          String::New("Bad argument.")));                            \
  }                                                                  \
  int32_t start = start_arg->Int32Value();                           \
  int32_t end = end_arg->Int32Value();                               \
  if (start < 0 || end < 0) {                                        \
    return ThrowException(Exception::TypeError(                      \
          String::New("Bad argument.")));                            \
  }                                                                  \
  if (!(start <= end)) {                                             \
    return ThrowException(Exception::Error(                          \
          String::New("Must have start <= end")));                   \
  }                                                                  \
  if ((size_t)end > parent->length_) {                               \
    return ThrowException(Exception::Error(                          \
          String::New("end cannot be longer than parent.length")));  \
  }


static Persistent<String> length_symbol;
static Persistent<String> write_sym;
static Persistent<Function> fast_buffer_constructor;
Persistent<FunctionTemplate> Buffer::constructor_template;


Handle<Object> Buffer::New(Handle<String> string) {
  HandleScope scope(node_isolate);

  // get Buffer from global scope.
  Local<Object> global = v8::Context::GetCurrent()->Global();
  Local<Value> bv = global->Get(String::NewSymbol("Buffer"));
  assert(bv->IsFunction());
  Local<Function> b = Local<Function>::Cast(bv);

  Local<Value> argv[1] = { Local<Value>::New(node_isolate, string) };
  Local<Object> instance = b->NewInstance(1, argv);

  return scope.Close(instance);
}


Buffer* Buffer::New(size_t length) {
  HandleScope scope(node_isolate);

  Local<Value> arg = Integer::NewFromUnsigned(length, node_isolate);
  Local<Object> b = constructor_template->GetFunction()->NewInstance(1, &arg);
  if (b.IsEmpty()) return NULL;

  return ObjectWrap::Unwrap<Buffer>(b);
}


Buffer* Buffer::New(const char* data, size_t length) {
  HandleScope scope(node_isolate);

  Local<Value> arg = Integer::NewFromUnsigned(0, node_isolate);
  Local<Object> obj = constructor_template->GetFunction()->NewInstance(1, &arg);

  Buffer *buffer = ObjectWrap::Unwrap<Buffer>(obj);
  buffer->Replace(const_cast<char*>(data), length, NULL, NULL);

  return buffer;
}


Buffer* Buffer::New(char *data, size_t length,
                    free_callback callback, void *hint) {
  HandleScope scope(node_isolate);

  Local<Value> arg = Integer::NewFromUnsigned(0, node_isolate);
  Local<Object> obj = constructor_template->GetFunction()->NewInstance(1, &arg);

  Buffer *buffer = ObjectWrap::Unwrap<Buffer>(obj);
  buffer->Replace(data, length, callback, hint);

  return buffer;
}


Handle<Value> Buffer::New(const Arguments& args) {
  if (!args.IsConstructCall()) {
    return FromConstructorTemplate(constructor_template, args);
  }

  HandleScope scope(node_isolate);

  if (!args[0]->IsUint32()) return ThrowTypeError("Bad argument");

  size_t length = args[0]->Uint32Value();
  if (length > Buffer::kMaxLength) {
    return ThrowRangeError("length > kMaxLength");
  }
  new Buffer(args.This(), length);

  return args.This();
}


Buffer::Buffer(Handle<Object> wrapper, size_t length) : ObjectWrap() {
  Wrap(wrapper);

  length_ = 0;
  callback_ = NULL;
  handle_.SetWrapperClassId(node_isolate, BUFFER_CLASS_ID);

  Replace(NULL, length, NULL, NULL);
}


Buffer::~Buffer() {
  Replace(NULL, 0, NULL, NULL);
}


// if replace doesn't have a callback, data must be copied
// const_cast in Buffer::New requires this
void Buffer::Replace(char *data, size_t length,
                     free_callback callback, void *hint) {
  HandleScope scope(node_isolate);

  if (callback_) {
    callback_(data_, callback_hint_);
  } else if (length_) {
    delete [] data_;
    node_isolate->AdjustAmountOfExternalAllocatedMemory(
        -static_cast<intptr_t>(sizeof(Buffer) + length_));
  }

  length_ = length;
  callback_ = callback;
  callback_hint_ = hint;

  if (callback_) {
    data_ = data;
  } else if (length_) {
    data_ = new char[length_];
    if (data)
      memcpy(data_, data, length_);
    node_isolate->AdjustAmountOfExternalAllocatedMemory(sizeof(Buffer) +
                                                        length_);
  } else {
    data_ = NULL;
  }

  handle_->SetIndexedPropertiesToExternalArrayData(data_,
                                                   kExternalUnsignedByteArray,
                                                   length_);
  handle_->Set(length_symbol, Integer::NewFromUnsigned(length_, node_isolate));
}


template <encoding encoding>
Handle<Value> Buffer::StringSlice(const Arguments& args) {
  HandleScope scope(node_isolate);
  Buffer *parent = ObjectWrap::Unwrap<Buffer>(args.This());
  SLICE_ARGS(args[0], args[1])

  const char* src = parent->data_ + start;
  size_t slen = (end - start);
  return scope.Close(StringBytes::Encode(src, slen, encoding));
}


Handle<Value> Buffer::BinarySlice(const Arguments& args) {
  return Buffer::StringSlice<BINARY>(args);
}


Handle<Value> Buffer::AsciiSlice(const Arguments& args) {
  return Buffer::StringSlice<ASCII>(args);
}


Handle<Value> Buffer::Utf8Slice(const Arguments& args) {
  return Buffer::StringSlice<UTF8>(args);
}


Handle<Value> Buffer::Ucs2Slice(const Arguments& args) {
  return Buffer::StringSlice<UCS2>(args);
}



Handle<Value> Buffer::HexSlice(const Arguments& args) {
  return Buffer::StringSlice<HEX>(args);
}


Handle<Value> Buffer::Base64Slice(const Arguments& args) {
  return Buffer::StringSlice<BASE64>(args);
}


// buffer.fill(value, start, end);
Handle<Value> Buffer::Fill(const Arguments &args) {
  HandleScope scope(node_isolate);

  if (!args[0]->IsInt32()) {
    return ThrowException(Exception::Error(String::New(
            "value is not a number")));
  }
  int value = (char)args[0]->Int32Value();

  Buffer *parent = ObjectWrap::Unwrap<Buffer>(args.This());
  SLICE_ARGS(args[1], args[2])

  memset( (void*)(parent->data_ + start),
          value,
          end - start);

  return Undefined(node_isolate);
}


// var bytesCopied = buffer.copy(target, targetStart, sourceStart, sourceEnd);
Handle<Value> Buffer::Copy(const Arguments &args) {
  HandleScope scope(node_isolate);

  Buffer *source = ObjectWrap::Unwrap<Buffer>(args.This());

  if (!Buffer::HasInstance(args[0])) {
    return ThrowTypeError("First arg should be a Buffer");
  }

  Local<Value> target = args[0];
  char* target_data = Buffer::Data(target);
  size_t target_length = Buffer::Length(target);
  size_t target_start = args[1]->IsUndefined() ? 0 : args[1]->Uint32Value();
  size_t source_start = args[2]->IsUndefined() ? 0 : args[2]->Uint32Value();
  size_t source_end = args[3]->IsUndefined() ? source->length_
                                              : args[3]->Uint32Value();

  if (source_end < source_start) {
    return ThrowRangeError("sourceEnd < sourceStart");
  }

  // Copy 0 bytes; we're done
  if (source_end == source_start) {
    return scope.Close(Integer::New(0, node_isolate));
  }

  if (target_start >= target_length) {
    return ThrowRangeError("targetStart out of bounds");
  }

  if (source_start >= source->length_) {
    return ThrowRangeError("sourceStart out of bounds");
  }

  if (source_end > source->length_) {
    return ThrowRangeError("sourceEnd out of bounds");
  }

  size_t to_copy = MIN(MIN(source_end - source_start,
                           target_length - target_start),
                           source->length_ - source_start);

  // need to use slightly slower memmove is the ranges might overlap
  memmove((void *)(target_data + target_start),
          (const void*)(source->data_ + source_start),
          to_copy);

  return scope.Close(Integer::New(to_copy, node_isolate));
}


Handle<Value> Buffer::Base64Write(const Arguments& args) {
  return Buffer::StringWrite<BASE64>(args);
}

Handle<Value> Buffer::BinaryWrite(const Arguments& args) {
  return Buffer::StringWrite<BINARY>(args);
}

Handle<Value> Buffer::Utf8Write(const Arguments& args) {
  return Buffer::StringWrite<UTF8>(args);
}

Handle<Value> Buffer::Ucs2Write(const Arguments& args) {
  return Buffer::StringWrite<UCS2>(args);
}

Handle<Value> Buffer::HexWrite(const Arguments& args) {
  return Buffer::StringWrite<HEX>(args);
}

Handle<Value> Buffer::AsciiWrite(const Arguments& args) {
  return Buffer::StringWrite<ASCII>(args);
}

template <encoding encoding>
Handle<Value> Buffer::StringWrite(const Arguments& args) {
  HandleScope scope(node_isolate);

  Buffer* buffer = ObjectWrap::Unwrap<Buffer>(args.This());

  if (!args[0]->IsString()) {
    return ThrowTypeError("Argument must be a string");
  }

  Local<String> str = args[0].As<String>();

  int length = str->Length();

  if (length == 0) {
    return scope.Close(Integer::New(0));
  }

  if (encoding == HEX && length % 2 != 0)
    return ThrowTypeError("Invalid hex string");


  size_t offset = args[1]->Int32Value();
  size_t max_length = args[2]->IsUndefined() ? buffer->length_ - offset
                                             : args[2]->Uint32Value();
  max_length = MIN(buffer->length_ - offset, max_length);

  if (max_length == 0) {
    // shortcut: nothing to write anyway
    Local<Integer> val = Integer::New(0);
    return scope.Close(val);
  }

  if (encoding == UCS2)
    max_length = max_length / 2;

  if (offset >= buffer->length_) {
    return ThrowTypeError("Offset is out of bounds");
  }

  char* start = buffer->data_ + offset;
  size_t written = StringBytes::Write(start,
                                      max_length,
                                      str,
                                      encoding,
                                      NULL);

  return scope.Close(Integer::New(written, node_isolate));
}


static inline void Swizzle(char* start, unsigned int len) {
  char* end = start + len - 1;
  while (start < end) {
    char tmp = *start;
    *start++ = *end;
    *end-- = tmp;
  }
}


template <typename T, enum Endianness endianness>
Handle<Value> ReadFloatGeneric(const Arguments& args) {
  size_t offset = args[0]->Uint32Value();
  bool doAssert = !args[1]->BooleanValue();

  if (doAssert) {
    if (!args[0]->IsUint32())
      return ThrowTypeError("offset is not uint");
    size_t len = static_cast<size_t>(
                    args.This()->GetIndexedPropertiesExternalArrayDataLength());
    if (offset + sizeof(T) > len || offset + sizeof(T) < offset)
      return ThrowRangeError("Trying to read beyond buffer length");
  }

  union NoAlias {
    T val;
    char bytes[sizeof(T)];
  };

  union NoAlias na;
  const void* data = args.This()->GetIndexedPropertiesExternalArrayData();
  const char* ptr = static_cast<const char*>(data) + offset;
  memcpy(na.bytes, ptr, sizeof(na.bytes));
  if (endianness != GetEndianness()) Swizzle(na.bytes, sizeof(na.bytes));

  // TODO: when Number::New is updated to accept an Isolate, make the change
  return Number::New(na.val);
}


Handle<Value> Buffer::ReadFloatLE(const Arguments& args) {
  return ReadFloatGeneric<float, kLittleEndian>(args);
}


Handle<Value> Buffer::ReadFloatBE(const Arguments& args) {
  return ReadFloatGeneric<float, kBigEndian>(args);
}


Handle<Value> Buffer::ReadDoubleLE(const Arguments& args) {
  return ReadFloatGeneric<double, kLittleEndian>(args);
}


Handle<Value> Buffer::ReadDoubleBE(const Arguments& args) {
  return ReadFloatGeneric<double, kBigEndian>(args);
}


template <typename T, enum Endianness endianness>
Handle<Value> WriteFloatGeneric(const Arguments& args) {
  bool doAssert = !args[2]->BooleanValue();

  if (doAssert) {
    if (!args[0]->IsNumber())
      return ThrowTypeError("value not a number");
    if (!args[1]->IsUint32())
      return ThrowTypeError("offset is not uint");
  }

  T val = static_cast<T>(args[0]->NumberValue());
  size_t offset = args[1]->Uint32Value();

  if (doAssert) {
    size_t len = static_cast<size_t>(
                    args.This()->GetIndexedPropertiesExternalArrayDataLength());
    if (offset + sizeof(T) > len || offset + sizeof(T) < offset)
      return ThrowRangeError("Trying to write beyond buffer length");
  }

  union NoAlias {
    T val;
    char bytes[sizeof(T)];
  };

  union NoAlias na = { val };
  void* data = args.This()->GetIndexedPropertiesExternalArrayData();
  char* ptr = static_cast<char*>(data) + offset;
  if (endianness != GetEndianness()) Swizzle(na.bytes, sizeof(na.bytes));
  memcpy(ptr, na.bytes, sizeof(na.bytes));

  return Undefined(node_isolate);
}


Handle<Value> Buffer::WriteFloatLE(const Arguments& args) {
  return WriteFloatGeneric<float, kLittleEndian>(args);
}


Handle<Value> Buffer::WriteFloatBE(const Arguments& args) {
  return WriteFloatGeneric<float, kBigEndian>(args);
}


Handle<Value> Buffer::WriteDoubleLE(const Arguments& args) {
  return WriteFloatGeneric<double, kLittleEndian>(args);
}


Handle<Value> Buffer::WriteDoubleBE(const Arguments& args) {
  return WriteFloatGeneric<double, kBigEndian>(args);
}


// var nbytes = Buffer.byteLength("string", "utf8")
Handle<Value> Buffer::ByteLength(const Arguments &args) {
  HandleScope scope(node_isolate);

  if (!args[0]->IsString()) {
    return ThrowTypeError("Argument must be a string");
  }

  Local<String> s = args[0]->ToString();
  enum encoding e = ParseEncoding(args[1], UTF8);

  return scope.Close(Integer::New(StringBytes::Size(s, e), node_isolate));
}


Handle<Value> Buffer::MakeFastBuffer(const Arguments &args) {
  HandleScope scope(node_isolate);

  if (!Buffer::HasInstance(args[0])) {
    return ThrowTypeError("First argument must be a Buffer");
  }

  Buffer *buffer = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
  Local<Object> fast_buffer = args[1]->ToObject();
  uint32_t offset = args[2]->Uint32Value();
  uint32_t length = args[3]->Uint32Value();

  if (offset > buffer->length_) {
    return ThrowRangeError("offset out of range");
  }

  if (offset + length > buffer->length_) {
    return ThrowRangeError("length out of range");
  }

  // Check for wraparound. Safe because offset and length are unsigned.
  if (offset + length < offset) {
    return ThrowRangeError("offset or length out of range");
  }

  fast_buffer->SetIndexedPropertiesToExternalArrayData(buffer->data_ + offset,
                                                       kExternalUnsignedByteArray,
                                                       length);

  return Undefined(node_isolate);
}


bool Buffer::HasInstance(Handle<Value> val) {
  if (!val->IsObject()) return false;
  Local<Object> obj = val->ToObject();

  ExternalArrayType type = obj->GetIndexedPropertiesExternalArrayDataType();
  if (type != kExternalUnsignedByteArray)
    return false;

  // Also check for SlowBuffers that are empty.
  if (constructor_template->HasInstance(obj))
    return true;

  assert(!fast_buffer_constructor.IsEmpty());
  return obj->GetConstructor()->StrictEquals(fast_buffer_constructor);
}


Handle<Value> SetFastBufferConstructor(const Arguments& args) {
  assert(args[0]->IsFunction());
  fast_buffer_constructor = Persistent<Function>::New(node_isolate,
                                                      args[0].As<Function>());
  return Undefined(node_isolate);
}


class RetainedBufferInfo: public RetainedObjectInfo {
public:
  RetainedBufferInfo(Buffer* buffer);
  virtual void Dispose();
  virtual bool IsEquivalent(RetainedObjectInfo* other);
  virtual intptr_t GetHash();
  virtual const char* GetLabel();
  virtual intptr_t GetSizeInBytes();
private:
  Buffer* buffer_;
  static const char label[];
};

const char RetainedBufferInfo::label[] = "Buffer";


RetainedBufferInfo::RetainedBufferInfo(Buffer* buffer): buffer_(buffer) {
}


void RetainedBufferInfo::Dispose() {
  buffer_ = NULL;
  delete this;
}


bool RetainedBufferInfo::IsEquivalent(RetainedObjectInfo* other) {
  return label == other->GetLabel() &&
         buffer_ == static_cast<RetainedBufferInfo*>(other)->buffer_;
}


intptr_t RetainedBufferInfo::GetHash() {
  return reinterpret_cast<intptr_t>(buffer_);
}


const char* RetainedBufferInfo::GetLabel() {
  return label;
}


intptr_t RetainedBufferInfo::GetSizeInBytes() {
  return Buffer::Length(buffer_);
}


RetainedObjectInfo* WrapperInfo(uint16_t class_id, Handle<Value> wrapper) {
  assert(class_id == BUFFER_CLASS_ID);
  assert(Buffer::HasInstance(wrapper));
  Buffer* buffer = Buffer::Unwrap<Buffer>(wrapper.As<Object>());
  return new RetainedBufferInfo(buffer);
}


void Buffer::Initialize(Handle<Object> target) {
  HandleScope scope(node_isolate);

  length_symbol = NODE_PSYMBOL("length");

  Local<FunctionTemplate> t = FunctionTemplate::New(Buffer::New);
  constructor_template = Persistent<FunctionTemplate>::New(node_isolate, t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("SlowBuffer"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "binarySlice", Buffer::BinarySlice);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "asciiSlice", Buffer::AsciiSlice);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "base64Slice", Buffer::Base64Slice);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ucs2Slice", Buffer::Ucs2Slice);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "hexSlice", Buffer::HexSlice);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "utf8Slice", Buffer::Utf8Slice);
  // TODO NODE_SET_PROTOTYPE_METHOD(t, "utf16Slice", Utf16Slice);

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "utf8Write", Buffer::Utf8Write);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "asciiWrite", Buffer::AsciiWrite);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "binaryWrite", Buffer::BinaryWrite);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "base64Write", Buffer::Base64Write);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ucs2Write", Buffer::Ucs2Write);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "hexWrite", Buffer::HexWrite);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "readFloatLE", Buffer::ReadFloatLE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "readFloatBE", Buffer::ReadFloatBE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "readDoubleLE", Buffer::ReadDoubleLE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "readDoubleBE", Buffer::ReadDoubleBE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "writeFloatLE", Buffer::WriteFloatLE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "writeFloatBE", Buffer::WriteFloatBE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "writeDoubleLE", Buffer::WriteDoubleLE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "writeDoubleBE", Buffer::WriteDoubleBE);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fill", Buffer::Fill);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "copy", Buffer::Copy);

  NODE_SET_METHOD(constructor_template->GetFunction(),
                  "byteLength",
                  Buffer::ByteLength);
  NODE_SET_METHOD(constructor_template->GetFunction(),
                  "makeFastBuffer",
                  Buffer::MakeFastBuffer);

  target->Set(String::NewSymbol("SlowBuffer"), constructor_template->GetFunction());
  target->Set(String::NewSymbol("setFastBufferConstructor"),
              FunctionTemplate::New(SetFastBufferConstructor)->GetFunction());

  v8::HeapProfiler* heap_profiler = node_isolate->GetHeapProfiler();
  heap_profiler->SetWrapperClassInfoProvider(BUFFER_CLASS_ID, WrapperInfo);
}


}  // namespace node

NODE_MODULE(node_buffer, node::Buffer::Initialize)
