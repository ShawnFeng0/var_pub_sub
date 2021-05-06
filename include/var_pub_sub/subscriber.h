//
// Created by shawnfeng on 5/8/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once

#include <memory>
#include <vector>

#include "ring_buffer.h"

namespace var_pub_sub {

class Subscriber {
 public:
  explicit Subscriber(std::shared_ptr<RingBuffer> ring_buffer)
      : ring_buffer_(std::move(ring_buffer)) {
    read_index_ = ring_buffer_->latest_data_index();
    data_.reserve(ring_buffer_->max_packet_length());
  }

  bool Update() { return read_index_ == ring_buffer_->latest_data_index(); }

  bool ReadWaitIfEmpty(int32_t time_ms = -1) {
    return ring_buffer_->ReadWaitIfEmpty(&read_index_, &data_, time_ms);
  }

  bool ReadDataPacket() {
    return ring_buffer_->ReadDataPacket(&read_index_, &data_);
  }

  const std::vector<uint8_t>& get_data() const { return data_; }

 private:
  RingBuffer::IndexType read_index_;
  std::shared_ptr<RingBuffer> ring_buffer_;
  std::vector<uint8_t> data_;
};

}  // namespace var_pub_sub
