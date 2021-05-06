//
// Created by shawnfeng on 5/8/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once

#include <utility>

#include "ring_buffer.h"

namespace var_pub_sub {

class Publisher {
 public:
  explicit Publisher(std::shared_ptr<RingBuffer> ring_buffer)
      : ring_buffer_(std::move(ring_buffer)) {}

  bool WriteDataPacket(const void *data, size_t length) {
    return ring_buffer_->WriteDataPacket(data, length);
  }

 private:
  std::shared_ptr<RingBuffer> ring_buffer_;
};

}  // namespace var_pub_sub
