#include <gtest/gtest.h>

#include <iostream>
#include <random>
#include <thread>

#include "include/var_pub_sub/data_node.h"

TEST(pub_sub, multi_pub_sub) {
  auto data_node = var_pub_sub::DataNode::CreateDataNode(900 * 1024);

  auto publish_thread = [&] {
    std::vector<int> vec;
    vec.resize(20 * 1024);
    for (auto i = 0; i < vec.size(); i++) {
      vec[i] = i;
    }

    auto pub = data_node->CreatePublisher();
    std::default_random_engine random_engine(time(nullptr));
    std::uniform_int_distribution<size_t> distribution(1, vec.size());

    int times = 20 * 30;
    while (times--) {
      pub.WriteDataPacket(vec.data(),
                          distribution(random_engine) * sizeof(vec[0]));
      usleep(30 * 1000);
    }
  };

  std::thread{publish_thread}.join();

  //  std::vector<std::thread> thread_vector;
  //  thread_vector.reserve(10);
  //  for (int i = 0; i < 1; i++) {
  //    thread_vector.emplace_back(publish_thread);
  //  }
  //
  //  for (auto &i : thread_vector) {
  //    i.join();
  //  }
}
