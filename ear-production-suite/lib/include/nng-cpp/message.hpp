#pragma once
#include "error_handling.hpp"
#include <nng/nng.h>

namespace nng {
/**
 * @brief Wrapper and holder of `nng_msg`
 *
 * It is possible to construct "invalid" messages.
 * This is useful when, for example a function signature of a callback requires
 * a `Message`, but an error occured and there is no real message to pass.
 */
class Message {
 public:
  /**
   * Construct an invalid message.
   */
  Message() : msg_(nullptr) {}
  /**
   *  Allocate a new message with body of length `size`
   */
  Message(std::size_t size) : msg_(nullptr) {
    auto ret = nng_msg_alloc(&msg_, size);
    handleError(ret);
  }

  /**
   * Wraps and takes ownership of existing `nng_msg`
   */
  explicit Message(nng_msg* msg) : msg_(msg) {}
  Message(const Message&) = delete;
  Message(Message&& other) : msg_(nullptr) { std::swap(other.msg_, msg_); }

  Message& operator=(const Message&) = delete;
  Message& operator=(Message&& other) {
    if (this == &other) {
      return *this;
    }
    std::swap(other.msg_, msg_);
    return *this;
  }

  ~Message() {
    if (msg_) {
      nng_msg_free(msg_);
    }
  }

  /**
   * Pointer to beginning of the message body
   */
  void* data() const { return nng_msg_body(msg_); }

  /**
   * Size of the body of the message
   */
  std::size_t size() const { return nng_msg_len(msg_); }

  /**
   * Returns `true` if the message is a valid message, i.e.
   * if it wraps an actual `nng_msg`
   */
  bool isValid() const { return msg_ != nullptr; }

  /**
   * Releases the underlying `nng_msg` to the caller, the Message instance is no
   * longer valid. It's the callers responsibility to free the message using
   * `nng_msg_free`.
   */
  nng_msg* release() {
    nng_msg* ptr = nullptr;
    std::swap(ptr, msg_);
    return ptr;
  }

  /**
   * Append data to the end of the message body, reallocating
   * it if needed.
   */
  void append(const void* value, std::size_t size) {
    auto ret = nng_msg_append(msg_, value, size);
    handleError(ret);
  }

  /**
   * Append data to the end of the message body, reallocating it if needed.
   */
  void append(uint32_t value) {
    auto ret = nng_msg_append_u32(msg_, value);
    handleError(ret);
  }

  /**
   * Append data to the end of the message body, reallocating it if needed.
   */
  void append(uint16_t value) {
    auto ret = nng_msg_append_u16(msg_, value);
    handleError(ret);
  }

  /**
   * Append data to the end of the message body, reallocating it if needed.
   */
  void append(uint64_t value) {
    auto ret = nng_msg_append_u64(msg_, value);
    handleError(ret);
  }

 private:
  nng_msg* msg_;
};

template <typename ConstBuffer>
Message createMessageFrom(const ConstBuffer& buffer) {
  Message msg(std::size_t{0});
  msg.append(buffer.data(), buffer.size());
  return msg;
}
}  // namespace nng
