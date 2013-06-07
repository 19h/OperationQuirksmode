

#include "openssl/bio.h"
#include <assert.h>

namespace node {

class NodeBIO {
 public:
  static inline BIO_METHOD* GetMethod() {
    return &method_;
  }

  static int New(BIO* bio);
  static int Free(BIO* bio);
  static int Read(BIO* bio, char* out, int len);
  static int Write(BIO* bio, const char* data, int len);
  static int Puts(BIO* bio, const char* str);
  static int Gets(BIO* bio, char* out, int size);
  static long Ctrl(BIO* bio, int cmd, long num, void* ptr);

 protected:
  static const size_t kBufferLength = 16 * 1024;

  class Buffer {
   public:
    Buffer() : read_pos_(0), write_pos_(0), next_(NULL) {
    }

    size_t read_pos_;
    size_t write_pos_;
    Buffer* next_;
    char data_[kBufferLength];
  };

  NodeBIO() : length_(0), read_head_(&head_), write_head_(&head_) {
    // Loop head
    head_.next_ = &head_;
  }

  ~NodeBIO();

  // Read `len` bytes maximum into `out`, return actual number of read bytes
  size_t Read(char* out, size_t size);

  // Find first appearance of `delim` in buffer or `limit` if `delim`
  // wasn't found.
  size_t IndexOf(char delim, size_t limit);

  // Discard all available data
  void Reset();

  // Put `len` bytes from `data` into buffer
  void Write(const char* data, size_t size);

  // Return size of buffer in bytes
  size_t inline Length() {
    return length_;
  }

  static inline NodeBIO* FromBIO(BIO* bio) {
    assert(bio->ptr != NULL);
    return static_cast<NodeBIO*>(bio->ptr);
  }

  size_t length_;
  Buffer head_;
  Buffer* read_head_;
  Buffer* write_head_;

  static BIO_METHOD method_;
};

} // namespace node
