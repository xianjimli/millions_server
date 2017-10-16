#include "topic_item.h"

bool topic_item_init(topic_item_t* item, const char* topic) {
	memset(item, 0x00, sizeof(topic_item_t));
	item->listeners = list_new();
	item->topic = strdup(topic);

	return true;
}

bool topic_item_deinit(topic_item_t* item) {
	list_destroy(item->listeners);
	free(item->topic);

	return true;
}

bool topic_item_add(topic_item_t* item, worker_t* w) {
	list_node_t* node = NULL;

	return_value_if_fail(item != NULL && w != NULL, false);
	node = list_node_new(w);	
	return_value_if_fail(node != NULL, false);

	return list_rpush(item->listeners, node) != NULL;
}

bool topic_item_has(topic_item_t* item, worker_t* w) {
	return_value_if_fail(item != NULL && w != NULL, false);

	return list_find(item->listeners, w) != NULL;
}

bool topic_item_remove(topic_item_t* item, worker_t* w) {
	list_node_t* node = NULL;

	return_value_if_fail(item != NULL && w != NULL, false);
	node = list_find(item->listeners, w);
	return_value_if_fail(node != NULL, false);

	list_remove(item->listeners, node);

	return true;
}

bool topic_item_is_empty(topic_item_t* item) {
   return_value_if_fail(item != NULL, true);

   return item->listeners->head == NULL;
}

