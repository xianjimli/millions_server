#ifndef TOPICS_H
#define TOPICS_H

#include "avltree.h"
#include "worker.h"
#include "topic_item.h"

typedef struct _topics_t {
	avl_tree avl;
	topic_item_t* hotest_topic;
}topics_t;

bool topics_init(topics_t* t);
bool topics_has_sub(topics_t* t, const char* topic, worker_t* w);
bool topics_unsub(topics_t* t, const char* topic, worker_t* w);
bool topics_pub(topics_t* t, const char* topic, const void* payload, size_t len);
bool topics_deinit(topics_t* t);

topic_item_t* topics_sub(topics_t* t, const char* topic, worker_t* w);

#endif//TOPICS_H

