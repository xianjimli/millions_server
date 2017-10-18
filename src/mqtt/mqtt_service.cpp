#include <errno.h>
#include "MQTTPacket.h"
#include "MQTTConnect.h"

#include "utils.h"
#include "topics.h"
#include "mqtt_service.h"

typedef struct _service_stat_t {
    time_t start;
    size_t requests;
    size_t con_reqs;
    size_t disc_reqs;
    size_t pub_reqs;
    size_t sub_reqs;
    size_t unsub_reqs;
    size_t ping_reqs;
    size_t recv_bytes;
    size_t send_bytes;
    size_t connections;
}service_stat_t;

static topics_t s_topics;
static service_stat_t s_stat;

bool mqtt_service_init() {
    s_stat.start = time(0);
	topics_init(&s_topics);
	
    return true;
}

bool mqtt_service_deinit() {
	topics_deinit(&s_topics);

    return true;
}

#define MAX_TOPIC_NR           16
#define MAX_TOPIC_LEN          128
#define RESP_SMALL_BUFF_LEN    64 
#define RESP_LARGE_BUFF_LEN    4096

#define WORKER_GET_SUBSCRIBES(w) (list_t*)(w->userdata)
#define WORKER_SET_SUBSCRIBES(w, list) w->userdata = list

extern "C" int MQTTSerialize_zero(unsigned char* buf, int buflen, unsigned char packettype);

static int mqtt_strlen(MQTTString* src) {
	return src->cstring ? strlen(src->cstring) : src->lenstring.len;
}

static char* from_mqtt_string(char* str, MQTTString* src) {
	return_value_if_fail(str != NULL && src != NULL, NULL);

	if(src->cstring) {
		strcpy(str, src->cstring);
	}else{
		strncpy(str, src->lenstring.data, src->lenstring.len);
		str[src->lenstring.len] = '\0';
	}

	return str;
}

static list_node_t* worker_find_subscribed(worker_t* w, const char* topic) {
	list_t* list = WORKER_GET_SUBSCRIBES(w);
	list_node_t* head = list->head;
	list_node_t* iter = list->head;
	
	while(iter) {
		topic_item_t* item = (topic_item_t*)iter->val;
		if(strcmp(item->topic, topic) == 0) {
			return iter;
		}

		iter = iter->next;
		if(iter == head) {
			break;
		}
	}

	return NULL;
}

static bool worker_add_subscribed(worker_t* w, topic_item_t* item) {
	list_t* list = WORKER_GET_SUBSCRIBES(w);
	list_node_t* node = list_node_new(item);
	list_lpush(list, node);

	return true;
}

static bool worker_remove_subscribed(worker_t* w, const char* topic) {
	list_t* list = WORKER_GET_SUBSCRIBES(w);
	list_node_t* node = worker_find_subscribed(w, topic);

	if(node) {
		topic_item_t* item = (topic_item_t*)node->val;
		list_remove(list, node);
	}

	return node != NULL;
}

bool mqtt_on_connect(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	MQTTPacket_connectData c;	
	unsigned char out[RESP_SMALL_BUFF_LEN];

    return_value_if_fail(MQTTDeserialize_connect(&c, buf, len), false);
    //TODO: Login
	ret = MQTTSerialize_connack(out, sizeof(out), 0, 0);
	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 

	return ret >= 0;
}

bool mqtt_on_publish(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
    int qos = 0;
    int payloadlen = 0;
    MQTTString topicName;
    unsigned char dup = 0;
    char topic[MAX_TOPIC_LEN];
    unsigned char retained = 0;
    unsigned short packetid = 0;
    unsigned char* payload = NULL;
	unsigned char out[RESP_LARGE_BUFF_LEN];

    ret = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, buf, len);
    return_value_if_fail(ret, false);
    assert(mqtt_strlen(&topicName) < MAX_TOPIC_LEN);
    return_value_if_fail(mqtt_strlen(&topicName) < MAX_TOPIC_LEN, false);

    ret = MQTTSerialize_publish(out, sizeof(out), 0, 0, 0, packetid, topicName, payload, payloadlen);
	topics_pub(&s_topics, from_mqtt_string(topic, &topicName), out, ret);

    ret = MQTTSerialize_puback(out, sizeof(out), packetid);

	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 

	return ret >= 0;
}

const int grantedQoSs[MAX_TOPIC_NR] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool mqtt_on_subcribe(worker_t* w, unsigned char* buf, size_t len) {
	int i = 0;
	int ret = 0;
    int maxcount = 1;
    int count = MAX_TOPIC_NR;
    unsigned char dup = 0;
    char topic[MAX_TOPIC_LEN];
    unsigned short packetid = 0;
	unsigned char out[RESP_SMALL_BUFF_LEN];
    int requestedQoSs[MAX_TOPIC_NR];
    MQTTString topicFilters[MAX_TOPIC_NR];

    ret = MQTTDeserialize_subscribe(&dup, &packetid, maxcount, &count, topicFilters, requestedQoSs, buf, len);
    return_value_if_fail(ret, false);

	for(i = 0; i < count; i++) {
		MQTTString* iter = topicFilters+i;
		if(mqtt_strlen(iter) < MAX_TOPIC_LEN) {
			from_mqtt_string(topic, iter);

			if(!worker_find_subscribed(w, topic)) {
				topic_item_t* item = topics_sub(&s_topics, topic, w); 
				worker_add_subscribed(w, item);
				assert(item != NULL);
			}
		}
	}

    ret = MQTTSerialize_suback(out, sizeof(out), packetid, count, (int*)grantedQoSs); 
	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 

	return ret >= 0;
}

bool mqtt_on_unsubcribe(worker_t* w, unsigned char* buf, size_t len) {
	int i = 0;
	int ret = 0;
	int count = 0;
	unsigned char dup = 0;
    char topic[MAX_TOPIC_LEN];
	unsigned short packetid = 0;
	int max_count = MAX_TOPIC_NR;
	unsigned char out[RESP_SMALL_BUFF_LEN];
	MQTTString topicFilters[MAX_TOPIC_NR];

	ret = MQTTDeserialize_unsubscribe(&dup, &packetid, max_count, &count, topicFilters, buf, len);
    return_value_if_fail(ret, false);
    
	for(i = 0; i < count; i++) {
		MQTTString* iter = topicFilters+i;
		if(mqtt_strlen(iter) < MAX_TOPIC_LEN) {
            topics_unsub(&s_topics, topic, w);
			worker_remove_subscribed(w, topic);
		}
	}

    ret = MQTTSerialize_unsuback(out, sizeof(out), packetid);
	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_ping(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	unsigned char out[RESP_SMALL_BUFF_LEN];
   
    (void)buf;
    (void)len;
    ret = MQTTSerialize_zero(out, sizeof(out), PINGRESP);
	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_disconnect(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	unsigned char out[RESP_SMALL_BUFF_LEN];
    
    (void)buf;
    (void)len;
    ret = MQTTSerialize_zero(out, sizeof(out), DISCONNECT);
	if(ret > 0) {
		ret = worker_write(w, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_stats(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	char out[RESP_SMALL_BUFF_LEN*2];
    
    (void)buf;
    (void)len;
    snprintf(out, sizeof(out), "conn:%zu reqs:%zu recv_bytes=%zu send_bytes=%zu pub_reqs=%zu\n", 
        s_stat.connections, s_stat.requests, s_stat.recv_bytes, s_stat.send_bytes, s_stat.pub_reqs);

	ret = worker_write(w, (unsigned char*)out, strlen((const char*)out));

	return ret >= 0;
}

static bool mqtt_worker_dispatch(worker_t* w, unsigned char* buf, size_t len) {
	int msg_type = buf[0] >> 4;
	printf("msg_type=%d\n", msg_type);
	s_stat.requests++;
	switch(msg_type) {
		case PUBLISH: {
	        s_stat.pub_reqs++;
			return mqtt_on_publish(w, buf, len);
		}
		case CONNECT: {
	        s_stat.con_reqs++;
			return mqtt_on_connect(w, buf, len);
		}
		case SUBSCRIBE: {
	        s_stat.sub_reqs++;
			return mqtt_on_subcribe(w, buf, len);
		}
		case UNSUBSCRIBE: {
	        s_stat.unsub_reqs++;
			return mqtt_on_unsubcribe(w, buf, len);
		}
		case PINGREQ: {
	        s_stat.ping_reqs++;
			return mqtt_on_ping(w, buf, len);
		}
		case DISCONNECT: {
	        s_stat.disc_reqs++;
			return mqtt_on_disconnect(w, buf, len);
		}
		default: {
			if(strncmp((char*)buf, "st", 2) == 0) {
				return mqtt_on_stats(w, buf, len);
			}else{
				printf("unkown packet, close client.\n");
				return false;
			}
		}
	}
}

static bool mqtt_worker_read_packet(worker_t* w, unsigned char* buf, size_t size) {
	int rret = worker_read_n(w, buf, 2);
	unsigned char* p = buf+1;
    unsigned char c = *p;
	int rlen = c & 127;;
	int multiplier = 128;

	if(rret != 2) {
		//client closed
		return false;
	}

	if(buf[0] == 's' && buf[1] == 't') {
		p++;
		read(w->fd, p, size-2);
		return true;
	}

	while(c & 128) {
		p++;
		rret = worker_read_n(w, p, 1);
		return_value_if_fail(rret == 1, false);

		c = *p;
		rlen = rlen + multiplier * (c & 127);

		multiplier *= 128;
		if((rlen+5) > size) {
			return false;
		}
	}

	p++;
	rret = worker_read_n(w, p, rlen);
	return_value_if_fail(rret == rlen, false);

	return true;
}

static bool mqtt_worker_work(worker_t* w) {
    unsigned char buf[2048];
    size_t len = sizeof(buf);

	return_value_if_fail(mqtt_worker_read_packet(w, buf, len), false);

	return mqtt_worker_dispatch(w, buf, len);
}

static bool mqtt_worker_init(worker_t* w) {
    (void)w;
    s_stat.connections++;
    printf("connected:%zu\n", s_stat.connections);
	WORKER_SET_SUBSCRIBES(w, list_new());

    return true;
}

static bool mqtt_worker_deinit(worker_t* w) {
    (void)w;
    s_stat.connections--;
    printf("disconnected:%zu\n", s_stat.connections);

	list_destroy(WORKER_GET_SUBSCRIBES(w));

    return true;
}

static int mqtt_worker_read(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = read(w->fd, buf, len);
		if(ret > 0) {
			s_stat.recv_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static int mqtt_worker_read_n(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = read_n(w->fd, buf, len);
		if(ret > 0) {
			s_stat.recv_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static int mqtt_worker_write(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = write_n(w->fd, buf, len);
		if(ret > 0) {
			s_stat.send_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static worker_ops_t s_mqtt_worker_ops = {
    mqtt_worker_init,
    mqtt_worker_work,
    mqtt_worker_read,
    mqtt_worker_read_n,
    mqtt_worker_write,
    mqtt_worker_deinit
};

worker_ops_t* mqtt_get_worker_ops() {
    return &s_mqtt_worker_ops;
}

