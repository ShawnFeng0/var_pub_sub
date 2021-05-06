//
// Created by shawnfeng on 5/5/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once
#include <pthread.h>

#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>

// Reference from kfifo of linux
class VarRingBuffer {
 public:
  explicit VarRingBuffer(size_t size) {
    if (size < 2) size = 2;

    // round up to the next power of 2, since our 'let the indices wrap'
    // technique works only in this case.
    size = RoundUpPowOfTwo(size);

    data_ = std::make_unique<uint8_t>(size);
    mask_ = size - 1;
  }
  VarRingBuffer(const VarRingBuffer &) = delete;
  VarRingBuffer &operator=(const VarRingBuffer &) = delete;

  ~VarRingBuffer() = default;

  // A packet is entered, either completely written or discarded.
  size_t InPacket(const void *data, size_t length) {
    if (!data) {
      return 0;
    }

    std::unique_lock<std::mutex> lg(mutex_);

    if (unused() < length) {
      num_dropped_ += length;
      peak_ = size();
      return 0;
    }

    peak_ = std::max(peak_, used());

    CopyInLocked(data, length, in_);
    in_ += length;

    in_signal_.notify_all();
    return length;
  }

  size_t OutPeek(void *out_buf, size_t num_elements) {
    if (!out_buf) {
      return 0;
    }

    std::unique_lock<std::mutex> lg(mutex_);

    num_elements = std::min(num_elements, used());
    CopyOutLocked(out_buf, num_elements, out_);
    return num_elements;
  }

  size_t OutWaitIfEmpty(void *out_buf, size_t num_elements,
                        int32_t time_ms = -1) {
    if (!out_buf) {
      return 0;
    }

    std::unique_lock<std::mutex> lg(mutex_);

    if (-1 == time_ms) {
      while (empty()) in_signal_.wait(lg);
    } else if (empty()) {
      in_signal_.wait_for(lg, std::chrono::milliseconds{time_ms});
      if (empty()) return 0;
    }

    num_elements = std::min(num_elements, used());
    CopyOutLocked(out_buf, num_elements, out_);
    out_ += num_elements;
    return num_elements;
  }

  size_t Out(void *out_buf, size_t num_elements) {
    if (!out_buf) {
      return 0;
    }

    std::unique_lock<std::mutex> lg(mutex_);
    num_elements = std::min(num_elements, used());
    CopyOutLocked(out_buf, num_elements, out_);
    out_ += num_elements;
    return num_elements;
  }

  /**
   * removes the entire fifo content
   *
   * Note: usage of Reset() is dangerous. It should be only called when the
   * fifo is exclusived locked or when it is secured that no other thread is
   * accessing the fifo.
   */
  void Reset() {
    std::unique_lock<std::mutex> lg(mutex_);
    out_ = in_;
  }

  /* returns the size of the fifo in elements */
  size_t size() const { return mask_ + 1; }

  /* returns the number of used elements in the fifo */
  size_t used() const { return in_ - out_; }
  bool empty() const { return used() == 0; }

  /* returns the number of unused elements in the fifo */
  size_t unused() const { return size() - used(); }

  /* DEBUG: Used to count the number of new data discarded during use */
  size_t num_dropped() const { return num_dropped_; }

  /* DEBUG: Used to count the maximum peak value during use */
  size_t peak() const { return peak_; }

 private:
  template <typename T>
  static inline auto RoundPowOfTwo(T n) -> decltype(n) {
    uint64_t value = n;

    // Fill 1
    value |= value >> 1U;
    value |= value >> 2U;
    value |= value >> 4U;
    value |= value >> 8U;
    value |= value >> 16U;
    value |= value >> 32U;

    // Unable to round-up, take the value of round-down
    if (decltype(n)(value + 1) == 0) {
      value >>= 1U;
    }

    return value + 1;
  }

  static inline auto RoundUpPowOfTwo(uint32_t n) -> decltype(n) {
    if (n == 0) return 1;

    // Avoid is already a power of 2
    return RoundPowOfTwo<decltype(n)>(n - 1);
  }

  static inline auto RoundDownPowOfTwo(uint32_t n) -> decltype(n) {
    return RoundPowOfTwo<decltype(n)>(n >> 1U);
  }

  void CopyInLocked(const void *src, size_t len, size_t off) const {
    size_t size = this->size();

    off &= mask_;
    size_t l = std::min(len, size - off);

    memcpy(data_.get() + off, src, l);
    memcpy(data_.get(), (uint8_t *)src + l, len - l);
  }

  void CopyOutLocked(void *dst, size_t len, size_t off) const {
    size_t size = this->size();
    size_t l;

    off &= mask_;
    l = std::min(len, size - off);

    memcpy(dst, data_.get() + off, l);
    memcpy((uint8_t *)dst + l, data_.get(), len - l);
  }

  size_t in_{};                    // data is added at offset (in % size)
  size_t out_{};                   // data is extracted from off. (out % size)
  std::unique_ptr<uint8_t> data_;  // the buffer holding the data
  size_t mask_{};  // (Constant) Mask used to match the correct in / out pointer
  size_t num_dropped_{};  // Number of dropped elements
  size_t peak_{};         // fifo peak

  std::mutex mutex_;
  std::condition_variable in_signal_;
};
