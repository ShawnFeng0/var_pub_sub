//
// Created by shawnfeng on 2021-05-20.
//
#include <gtest/gtest.h>
#include <vrb/vrb.h>

#include <random>
#include <thread>

TEST(pub_sub, c_multi_pub_sub) {
  vrb_data_node_t *data_node = vrb_create_data_node(900 * 1024);

  std::vector<int32_t> vec;
  vec.resize(20 * 1024);
  for (auto i = 0; i < vec.size(); i++) vec[i] = i;

  auto publish_thread = [&] {
    auto pub = vrb_create_publisher(data_node);
    std::default_random_engine random_engine(pthread_self());
    std::uniform_int_distribution<size_t> distribution(1, vec.size());

    int times = 20;
    while (times--) {
      vrb_publish_data_packet(pub, vec.data(),
                              distribution(random_engine) * sizeof(vec[0]));
      usleep(30 * 1000);
    }
    vrb_destroy_publisher(&pub);
    ASSERT_TRUE(pub == nullptr);
  };

  std::vector<std::thread> pub_threads;
  pub_threads.reserve(10);
  for (int i = 0; i < pub_threads.capacity(); i++) {
    pub_threads.emplace_back(publish_thread);
  }

  auto subscriber_thread = [&] {
    auto sub = vrb_create_subscriber(data_node);
    const uint8_t *data;
    size_t size;
    while (vrb_read_data_packet_wait_for(sub, &data, &size, 100)) {
      std::vector<int32_t> vec_copy;
      vec_copy.resize(size / sizeof(int32_t));
      memcpy(vec_copy.data(), data, size);

      for (auto i = 0; i < vec_copy.size(); i++) {
        ASSERT_EQ(vec[i], vec_copy[i]) << "i: " << i;
      }
    }
    vrb_destroy_subscriber(&sub);
    ASSERT_TRUE(sub == nullptr);
  };
  std::vector<std::thread> sub_threads;
  sub_threads.reserve(10);
  for (int i = 0; i < sub_threads.capacity(); i++) {
    sub_threads.emplace_back(subscriber_thread);
  }

  for (auto &i : pub_threads) i.join();
  for (auto &i : sub_threads) i.join();
  vrb_destroy_data_node(&data_node);
}
