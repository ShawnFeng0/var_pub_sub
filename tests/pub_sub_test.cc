#include <gtest/gtest.h>

#include <random>
#include <thread>

#include "../src/data_node.h"

TEST(pub_sub, cpp_multi_pub_sub) {
  vrb::DataNode data_node(900 * 1024);

  std::vector<int32_t> vec;
  vec.resize(20 * 1024);
  for (auto i = 0; i < vec.size(); i++) vec[i] = i;

  std::atomic_uint64_t pub_counter{0};
  auto publish_thread = [&] {
    auto pub = data_node.CreatePublisher();
    std::default_random_engine random_engine(pthread_self());
    std::uniform_int_distribution<size_t> distribution(1, vec.size());

    int times = 20;
    while (times--) {
      auto length = distribution(random_engine) * sizeof(vec[0]);
      pub->WriteDataPacket(vec.data(), length);
      pub_counter += length;
      usleep(30 * 1000);
    }
    vrb::DataNode::DestroyPublisher(pub);
  };

  std::vector<std::thread> pub_threads;
  pub_threads.reserve(10);
  for (int i = 0; i < pub_threads.capacity(); i++) {
    pub_threads.emplace_back(publish_thread);
  }

  auto subscriber_thread = [&] {
    auto sub = data_node.CreateSubscriber();
    uint64_t sub_counter = 0;
    while (sub->ReadWaitIfEmpty(100)) {
      const auto &data = sub->get_data();
      sub_counter += data.size();

      std::vector<int32_t> vec_copy;
      vec_copy.resize(data.size() / sizeof(int32_t));
      memcpy(vec_copy.data(), data.data(), data.size());

      for (auto i = 0; i < vec_copy.size(); i++) {
        ASSERT_EQ(vec[i], vec_copy[i]) << "i: " << i;
      }
    }
    std::cout << "Sub count: " << sub_counter << std::endl;

    vrb::DataNode::DestroySubscriber(sub);
  };

  std::vector<std::thread> sub_threads;
  sub_threads.reserve(10);
  for (int i = 0; i < sub_threads.capacity(); i++) {
    sub_threads.emplace_back(subscriber_thread);
  }

  for (auto &i : pub_threads) i.join();
  for (auto &i : sub_threads) i.join();
  std::cout << "Pub count: " << pub_counter << std::endl;
}
