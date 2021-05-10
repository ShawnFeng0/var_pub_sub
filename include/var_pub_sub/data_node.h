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

class DataNode;
static inline std::shared_ptr<DataNode> CreateDataNode(size_t buffer_size);

class DataNode : public Noncopyable {
  friend std::shared_ptr<DataNode> CreateDataNode(size_t buffer_size);

 public:
  Publisher CreatePublisher() { return Publisher(ring_buffer_); }
  Subscriber CreateSubscriber() { return Subscriber(ring_buffer_); }

 private:
  explicit DataNode(size_t buffer_size) {
    ring_buffer_ = std::make_shared<RingBuffer>(buffer_size);
  }

  std::shared_ptr<RingBuffer> ring_buffer_;
};

static inline std::shared_ptr<DataNode> CreateDataNode(size_t buffer_size) {
  return std::shared_ptr<DataNode>{new DataNode(buffer_size)};
}

}  // namespace var_pub_sub
