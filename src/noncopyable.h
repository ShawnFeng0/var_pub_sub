//
// Created by fs on 2021-05-08.
//

#pragma once

namespace vrb {

class Noncopyable {
 public:
  Noncopyable() = default;
  ~Noncopyable() = default;

  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;
};

}  // namespace var_pub_sub
