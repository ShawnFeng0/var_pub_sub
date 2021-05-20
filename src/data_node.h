//
// Created by shawnfeng on 5/7/21.
// Copyright (c) 2021 shawnfeng. All rights reserved.
//
#pragma once

#include <memory>

#include "publisher.h"
#include "ring_buffer.h"
#include "subscriber.h"

namespace vrb {

class DataNode : public Noncopyable {
 public:
  Publisher *CreatePublisher() { return new Publisher(ring_buffer_); }
  static inline void DestroyPublisher(Publisher *ptr) { delete ptr; }

  Subscriber *CreateSubscriber() { return new Subscriber(ring_buffer_); }
  static inline void DestroySubscriber(Subscriber *ptr) { delete ptr; }

  explicit DataNode(size_t buffer_size) {
    ring_buffer_ = std::make_shared<RingBuffer>(buffer_size);
  }

 private:
  std::shared_ptr<RingBuffer> ring_buffer_;
};

}  // namespace var_pub_sub
