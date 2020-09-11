#pragma once
#include <nng/nng.h>
#include <cstddef>

namespace nng {

/**
 * Holder for memory allocatd by `nng` / `nng_alloc`.
 *
 * This class owns a contiguous memory region for use with the nng api.
 */
class Buffer {
 public:
  static Buffer alloc(std::size_t size) {
    void* data = nng_alloc(size);
    return Buffer{data, size};
  }

  ~Buffer() {
    if (data_) {
      nng_free(data_, size_);
    }
  }

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&& other) : data_(nullptr), size_(0) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }

  Buffer& operator=(Buffer&& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    nng_free(other.data_, other.size_);
    other.data_ = nullptr;
    other.size_ = 0;
    return *this;
  }

  /**
   * Takes ownership of memory pointed to by `data`
   */
  Buffer(void* data, std::size_t size) : data_(data), size_(size) {}
  std::size_t size() const { return size_; }

  void* data() { return data_; }
  void const* data() const { return data_; }
  /**
   * Releases the memory region to the caller, the buffer instance is no longer
   * valid. It's the callers responsibility to free the memory using `nng_free`.
   */
  void* release() {
    auto p = data_;
    data_ = nullptr;
    size_ = 0;
    return p;
  }

 private:
  void* data_;
  std::size_t size_;
};

}  // namespace nng
