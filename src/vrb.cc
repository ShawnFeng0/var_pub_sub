//
// Created by shawnfeng on 2021-05-20.
//

#include "include/vrb/vrb.h"

#include "src/data_node.h"

vrb_data_node_t *vrb_create_data_node(size_t ring_buffer_size) {
  return reinterpret_cast<vrb_data_node_t *>(
      new vrb::DataNode(ring_buffer_size));
}

void vrb_destroy_data_node(vrb_data_node_t **node_ptr) {
  if (!node_ptr) return;

  auto &node = *node_ptr;
  // Dereference and delete
  auto data_node = (reinterpret_cast<vrb::DataNode *>(node));
  delete data_node;
  node = nullptr;
}

vrb_data_publisher_t *vrb_create_publisher(vrb_data_node_t *node) {
  if (!node) return nullptr;

  auto data_node = reinterpret_cast<vrb::DataNode *>(node);

  return reinterpret_cast<vrb_data_publisher_t *>(data_node->CreatePublisher());
}

void vrb_destroy_publisher(vrb_data_publisher_t **pub_ptr) {
  if (!pub_ptr) return;
  auto &pub = *pub_ptr;

  auto data_pub = reinterpret_cast<vrb::Publisher *>(pub);
  vrb::DataNode::DestroyPublisher(data_pub);
  pub = nullptr;
}

bool vrb_publish_data_packet(vrb_data_publisher_t *pub, const void *data,
                             size_t length) {
  if (!pub) return false;

  auto data_pub = reinterpret_cast<vrb::Publisher *>(pub);

  return data_pub->WriteDataPacket(data, length);
}

vrb_data_subscriber_t *vrb_create_subscriber(vrb_data_node_t *node) {
  if (!node) return nullptr;

  auto data_node = reinterpret_cast<vrb::DataNode *>(node);

  return reinterpret_cast<vrb_data_subscriber_t *>(
      data_node->CreateSubscriber());
}

void vrb_destroy_subscriber(vrb_data_subscriber_t **sub_ptr) {
  if (!sub_ptr) return;
  auto &sub = *sub_ptr;

  auto data_sub = reinterpret_cast<vrb::Subscriber *>(sub);
  vrb::DataNode::DestroySubscriber(data_sub);
  sub = nullptr;
}

bool vrb_read_data_packet_wait_for(vrb_data_subscriber_t *sub,
                                   const uint8_t **out_data_ptr,
                                   size_t *out_length, int32_t time_ms) {
  if (!sub || !out_data_ptr || !out_length) return false;

  auto data_sub = reinterpret_cast<vrb::Subscriber *>(sub);

  if (!data_sub->ReadWaitIfEmpty(time_ms)) return false;

  auto &data = data_sub->get_data();
  *out_data_ptr = data.data();
  *out_length = data.size();
  return true;
}

bool vrb_read_data_packet(vrb_data_subscriber_t *sub,
                          const uint8_t **out_data_ptr, size_t *out_length) {
  return vrb_read_data_packet_wait_for(sub, out_data_ptr, out_length, 0);
}
