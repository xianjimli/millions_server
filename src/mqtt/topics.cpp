#include "utils.h"
#include "topics.h"
#include "avltree.c"
#include "MQTTPacket.h"

typedef struct topic_node_t {
	avl_node base;
	topic_item_t item;
}topic_node_t;

static topic_node_t* topic_node_create(const char* topic) {
	topic_node_t* node = (topic_node_t*)calloc(1, sizeof(topic_node_t));
	return_value_if_fail(node != NULL, NULL);

	return_value_if_fail(topic_item_init(&(node->item), topic), NULL);

	return node;
}

static bool topic_node_destroy(topic_node_t* node) {
    return_value_if_fail(node != NULL, false);
    
    topic_item_deinit(&(node->item));
    free(node);

    return true;
}

static int topic_cmp(struct avl_node *a, struct avl_node *b, void *aux) {
	topic_node_t* aa = (topic_node_t*)a;	
	topic_node_t* bb = (topic_node_t*)b;	

	(void)aux;
	return strcmp(aa->item.topic, bb->item.topic);
}

static topic_node_t* topics_find_node(topics_t* t, const char* topic) {
	topic_node_t node;
	topic_node_t* ret = NULL;
	return_value_if_fail(t != NULL && topic != NULL, NULL);

	node.item.topic = (char*)topic;
	ret = (topic_node_t*)avl_search(&(t->avl), (avl_node*)&node, topic_cmp);
	if(ret) {
        assert(strcmp(topic, ret->item.topic) == 0);
    }

	return ret;
}

static topic_item_t* topics_find(topics_t* t, const char* topic) {
	topic_node_t* ret = topics_find_node(t, topic);

	return ret ? &(ret->item) : NULL;
}

topic_item_t* topics_insert(topics_t* t, const char* topic) {
	topic_node_t* ret = NULL;
	topic_node_t* node = NULL;

	return_value_if_fail(t != NULL && topic != NULL, NULL);
	node = topic_node_create(topic);
	ret = (topic_node_t*)avl_insert(&(t->avl), (avl_node*)node, topic_cmp);

	return ret ? &(ret->item) : NULL;
}

bool topics_pub(topics_t* t, const char* topic, const void* payload, size_t len) {
	topic_item_t* ret = NULL;
	return_value_if_fail(t != NULL && topic != NULL, NULL);

	if(t->hotest_topic && strcmp(t->hotest_topic->topic, topic) == 0) {
		ret = t->hotest_topic;
	}else{
		ret = topics_find(t, topic);
	}

	if(ret) {
		list_t* list = ret->listeners;
		list_node_t* head = list->head;
		list_node_t* iter = list->head;
		
		ret->pub_times++;
		if(!t->hotest_topic || t->hotest_topic->pub_times < ret->pub_times) {
			t->hotest_topic = ret;
		}

		while(iter) {
			worker_t* w = (worker_t*)iter->val;
			worker_write(w, (unsigned char*)payload, len);
			
			iter = iter->next;
			if(iter == head) {
				break;
			}
		}
	}

	return true;
}

bool topics_unsub(topics_t* t, const char* topic, worker_t* w) {
	topic_node_t* ret = NULL;
	return_value_if_fail(t != NULL && topic != NULL, NULL);
	
	ret = topics_find_node(t, topic);
	if(ret) {
		if(topic_item_remove(&(ret->item), w)) {
		    if(topic_item_is_empty(&(ret->item))) {
		        avl_remove(&(t->avl), (avl_node*)ret);
		        printf("%s: no listeners, so remove topic %s\n", __func__, topic); 
		    }

		    return true;
		}
	}

	return false;
}

topic_item_t* topics_sub(topics_t* t, const char* topic, worker_t* w) {
	topic_item_t* ret = NULL;
	return_value_if_fail(t != NULL && topic != NULL, NULL);
	
	ret = topics_find(t, topic);
	if(!ret) {
		ret = topics_insert(t, topic);
	}

	if(ret) {
		if(!topic_item_add(ret, w)) {
			ret = NULL;
		}
	}

	return ret; 
}

bool topics_has_sub(topics_t* t, const char* topic, worker_t* w) {
	topic_item_t* ret = NULL;
	return_value_if_fail(t != NULL && topic != NULL, NULL);
	
	ret = topics_find(t, topic);

	return ret && list_find(ret->listeners, w) != NULL;
}

bool topics_init(topics_t* t) {
	return_value_if_fail(t != NULL, false);
	memset(t, 0x00, sizeof(topics_t));

	avl_init(&(t->avl), NULL);

	return true;
}

bool topics_deinit(topics_t* t) {
	return_value_if_fail(t != NULL, false);
	//TODO
	return true;
}


