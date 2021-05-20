//
// Created by shawnfeng on 2021-05-20.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vrb_data_node vrb_data_node_t;
typedef struct vrb_data_publisher vrb_data_publisher_t;
typedef struct vrb_data_subscriber vrb_data_subscriber_t;

/**
 * Create new variable length data ringbuffer
 * @param size Buffer length (will be automatically incremented to a power of 2)
 * @return Data node pointer
 */
vrb_data_node_t *vrb_create_data_node(size_t size);

/**
 * Destroy data node and mark the pointer as nullptr
 * Note: Although you may not destroy it most of the time, destroying it does
 * not actually affect the publishers and subscribers that have been created by
 * this node.
 *
 * @param node
 */
void vrb_destroy_data_node(vrb_data_node_t **node_ptr);

/**
 * Create new data publisher
 * @param node Data node pointer
 * @return Data publisher pointer
 */
vrb_data_publisher_t *vrb_create_publisher(vrb_data_node_t *node);

/**
 * Destroy data publisher and mark the pointer as nullptr
 * @param pub_ptr
 */
void vrb_destroy_publisher(vrb_data_publisher_t **pub_ptr);

/**
 * Publish data to ringbuffer
 * Note: Old data will be overwritten
 * @param pub
 * @param data
 * @param length
 * @return
 */
bool vrb_publish_data_packet(vrb_data_publisher_t *pub, const void *data,
                             size_t length);

/**
 * Create new data subscriber
 * @param node Data node pointer
 * @return Data subscriber pointer
 */
vrb_data_subscriber_t *vrb_create_subscriber(vrb_data_node_t *node);

/**
 * Destroy data subscriber and mark the pointer as nullptr
 * @param sub_ptr
 */
void vrb_destroy_subscriber(vrb_data_subscriber_t **sub_ptr);

/**
 * Get data from ringbuffer
 * @param sub
 * @param out_data_ptr
 * @param out_length
 * @return
 */
bool vrb_read_data_packet(vrb_data_subscriber_t *sub,
                          const uint8_t **out_data_ptr, size_t *out_length);

/**
 * Wait for data from ringbuffer
 * @param sub
 * @param out_data_ptr
 * @param out_length
 * @param time_ms
 * @return
 */
bool vrb_read_data_packet_wait_for(vrb_data_subscriber_t *sub,
                                   const uint8_t **out_data_ptr,
                                   size_t *out_length, int32_t time_ms);

#ifdef __cplusplus
}
#endif
