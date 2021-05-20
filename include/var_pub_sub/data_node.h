//
// Created by shawnfeng on 5/7/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once

#include <memory>

#include "publisher.h"
#include "ring_buffer.h"
#include "subscriber.h"

namespace var_pub_sub {

class DataNode : public Noncopyable {
 public:
  Publisher CreatePublisher() { return Publisher(ring_buffer_); }
  Subscriber CreateSubscriber() { return Subscriber(ring_buffer_); }

  explicit DataNode(size_t buffer_size) {
    ring_buffer_ = std::make_shared<RingBuffer>(buffer_size);
  }

 private:
  std::shared_ptr<RingBuffer> ring_buffer_;
};

}  // namespace var_pub_sub
