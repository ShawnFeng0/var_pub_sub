//
// Created by shawnfeng on 5/5/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once
#include <pthread.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include "noncopyable.h"

namespace var_pub_sub {

class RingBuffer : public Noncopyable {
 public:
  using IndexType = size_t;

  explicit RingBuffer(size_t size) : mask_{GenerateRingbufferSize(size) - 1} {
    data_ = std::make_unique<uint8_t[]>(mask_ + 1);
  }

  // A packet is entered, either completely written or discarded.
  bool WriteDataPacket(const void *data, size_t length) {
    if (!data || length > size()) {
      return false;
    }

    std::unique_lock<std::mutex> lg(mutex_);

    // discard old data
    while (available() < (length + sizeof(LengthPad))) {
      LengthPad length_pad;
      CopyOutLocked(&length_pad, sizeof(length_pad), read_index_);
      read_index_ += sizeof(length_pad) + length_pad;
    }

    max_packet_length_ = std::max(max_packet_length_, length);

    // Padding length and data
    LengthPad length_pad = length;
    CopyInLocked(&length_pad, sizeof(length_pad), write_index_);
    CopyInLocked(data, length, write_index_ + sizeof(length_pad));

    latest_data_index_ = write_index_;
    write_index_ += sizeof(length_pad) + length;

    in_signal_.notify_all();
    return true;
  }

  bool ReadDataPacket(IndexType *read_index, std::vector<uint8_t> *data,
                      int32_t time_ms = -1) {
    if (!read_index) {
      return false;
    }

    std::unique_lock<std::mutex> lg(mutex_);

    if (-1 == time_ms) {
      while (!HaveNewData(*read_index)) in_signal_.wait(lg);
    } else if (!HaveNewData(*read_index)) {
      in_signal_.wait_for(lg, std::chrono::milliseconds{time_ms});
      if (!HaveNewData(*read_index)) return false;
    }

    // Reader is too far behind: some messages are lost
    if (!IsInRange(read_index_, *read_index, write_index_)) {
      *read_index = read_index_;
    }

    LengthPad length_pad;
    CopyOutLocked(&length_pad, sizeof(length_pad), *read_index);
    *read_index += sizeof(length_pad);
    if (data) {
      CopyOutLocked(data, length_pad, *read_index);
    }
    *read_index += length_pad;

    return true;
  }

  size_t max_packet_length() const {
    std::unique_lock<std::mutex> lg(mutex_);
    return max_packet_length_;
  }

  IndexType latest_data_index() const {
    std::unique_lock<std::mutex> lg(mutex_);
    return latest_data_index_;
  }

  /* returns the size of the fifo in elements */
  size_t size() const { return mask_ + 1; }

  /* returns the number of used elements in the fifo */
  size_t used() const { return write_index_ - read_index_; }

  /* returns the number of unused elements in the fifo */
  size_t available() const { return size() - used(); }

 private:
  using LengthPad = uint32_t;

  static inline bool IsInRange(IndexType left, IndexType value,
                               IndexType right) {
    if (right > left) {
      return (left <= value) && (value <= right);
    } else {  // Maybe the data overflowed and a wraparound occurred
      return (left <= value) || (value <= right);
    }
  }

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

  static inline size_t GenerateRingbufferSize(size_t size) {
    if (size < 2) size = 2;

    // round up to the next power of 2, since our 'let the indices wrap'
    // technique works only in this case.
    return RoundUpPowOfTwo(size);
  }

  static inline auto RoundUpPowOfTwo(uint32_t n) -> decltype(n) {
    if (n == 0) return 1;

    // Avoid is already a power of 2
    return RoundPowOfTwo<decltype(n)>(n - 1);
  }

  void CopyInLocked(const void *src, size_t len, IndexType off) const {
    size_t size = this->size();

    off &= mask_;
    size_t l = std::min(len, size - off);

    memcpy(data_.get() + off, src, l);
    memcpy(data_.get(), (uint8_t *)src + l, len - l);
  }

  void CopyOutLocked(void *dst, size_t len, IndexType off) const {
    size_t size = this->size();

    off &= mask_;
    size_t l = std::min(len, size - off);

    memcpy(dst, data_.get() + off, l);
    memcpy((uint8_t *)dst + l, data_.get(), len - l);
  }

  void CopyOutLocked(std::vector<uint8_t> *dst_vector, size_t len,
                     IndexType off) const {
    size_t size = this->size();

    off &= mask_;
    size_t l = std::min(len, size - off);

    dst_vector->assign(data_.get() + off, data_.get() + off + l);
    dst_vector->insert(dst_vector->cend(), data_.get(), data_.get() + len - l);
  }

  bool HaveNewData(IndexType read_index) const {
    return read_index != write_index_;
  }

  const IndexType mask_;     // Mask used to match the correct in / out pointer
  IndexType write_index_{};  // data is added at offset (in % size)
  IndexType latest_data_index_{};    // Index of the latest data
  IndexType read_index_{};           // data is extracted from off. (out % size)
  std::unique_ptr<uint8_t[]> data_;  // the buffer holding the data
  size_t max_packet_length_{1};      // The length of the longest set of data

  mutable std::mutex mutex_;
  std::condition_variable in_signal_;
};

}  // namespace var_pub_sub
