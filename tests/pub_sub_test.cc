#include <gtest/gtest.h>

#include <iostream>
#include <random>
#include <thread>

#include "../include/var_pub_sub/data_node.h"

TEST(pub_sub, multi_pub_sub) {
  auto data_node = var_pub_sub::CreateDataNode(900 * 1024);

  std::vector<int32_t> vec;
  vec.resize(20 * 1024);
  for (auto i = 0; i < vec.size(); i++) vec[i] = i;

  auto publish_thread = [&] {
    auto pub = data_node->CreatePublisher();
    std::default_random_engine random_engine(pthread_self());
    std::uniform_int_distribution<size_t> distribution(1, vec.size());

    int times = 20;
    while (times--) {
      pub.WriteDataPacket(vec.data(),
                          distribution(random_engine) * sizeof(vec[0]));
      usleep(30 * 1000);
    }
  };

  std::vector<std::thread> pub_threads;
  pub_threads.reserve(10);
  for (int i = 0; i < pub_threads.capacity(); i++) {
    pub_threads.emplace_back(publish_thread);
  }

  auto subscriber_thread = [&] {
    auto sub = data_node->CreateSubscriber();
    while (sub.ReadWaitIfEmpty(100)) {
      const auto &data = sub.get_data();

      std::vector<int32_t> vec_copy;
      vec_copy.resize(data.size() / sizeof(int32_t));
      memcpy(vec_copy.data(), data.data(), data.size());

      for (auto i = 0; i < vec_copy.size(); i++) {
        ASSERT_EQ(vec[i], vec_copy[i]) << "i: " << i;
      }
    }
  };
  std::vector<std::thread> sub_threads;
  sub_threads.reserve(10);
  for (int i = 0; i < sub_threads.capacity(); i++) {
    sub_threads.emplace_back(subscriber_thread);
  }

  for (auto &i : pub_threads) i.join();
  for (auto &i : sub_threads) i.join();
}
