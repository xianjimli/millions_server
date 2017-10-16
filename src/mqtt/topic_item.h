#ifndef TOPIC_ITEM_H
#define TOPIC_ITEM_H

#include "list.h"
#include "worker.h"

typedef struct _topic_item_t {
	char* topic;
	size_t pub_times;
	list_t* listeners;
}topic_item_t;

bool topic_item_init(topic_item_t* item, const char* topic);
bool topic_item_remove(topic_item_t* item, worker_t* w);
bool topic_item_has(topic_item_t* item, worker_t* w);
bool topic_item_add(topic_item_t* item, worker_t* w);
bool topic_item_deinit(topic_item_t* item);
bool topic_item_is_empty(topic_item_t* item);

#endif//TOPIC_ITEM_H

