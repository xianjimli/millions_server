#include <errno.h>
#include "MQTTPacket.h"
#include "MQTTConnect.h"

#include "utils.h"
#include "mqtt_service.h"

typedef struct _service_stat_t {
    time_t start;
    size_t requests;
    size_t recv_bytes;
    size_t send_bytes;
}service_stat_t;

static service_stat_t s_stat;

bool mqtt_service_init() {
    s_stat.start = time(0);

    return true;
}

bool mqtt_service_deinit() {
    return true;
}

#define SMALL_BUFF_LEN 256
#define LARGE_BUFF_LEN 4096

extern "C" int MQTTSerialize_zero(unsigned char* buf, int buflen, unsigned char packettype);

bool mqtt_on_connect(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	MQTTPacket_connectData c;	
	unsigned char out[SMALL_BUFF_LEN];

    return_value_if_fail(MQTTDeserialize_connect(&c, buf, len), false);

    printf("%s\n", __func__);

	ret = MQTTSerialize_connack(out, sizeof(out), 0, 0);
	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 

	return ret >= 0;
}

bool mqtt_on_publish(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
    int qos = 0;
    int payloadlen = 0;
    MQTTString topicName;
    unsigned char dup = 0;
    unsigned char retained = 0;
    unsigned short packetid = 0;
    unsigned char* payload = NULL;
	unsigned char out[SMALL_BUFF_LEN];

    ret = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, buf, len);
    return_value_if_fail(ret, false);

    /*TODO*/
    printf("%s\n", __func__);

    ret = MQTTSerialize_puback(out, sizeof(out), packetid);

	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 

	return ret >= 0;
}

#define MAX_TOPIC_NR 16
const int grantedQoSs[MAX_TOPIC_NR] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool mqtt_on_subcribe(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
    int maxcount = 1;
    int count = MAX_TOPIC_NR;
    unsigned char dup = 0;
    unsigned short packetid = 0;
	unsigned char out[SMALL_BUFF_LEN];
    int requestedQoSs[MAX_TOPIC_NR];
    MQTTString topicFilters[MAX_TOPIC_NR];

    ret = MQTTDeserialize_subscribe(&dup, &packetid, maxcount, &count, topicFilters, requestedQoSs, buf, len);
    return_value_if_fail(ret, false);

    /*TODO*/

    ret = MQTTSerialize_suback(out, sizeof(out), packetid, count, (int*)grantedQoSs); 
	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 

	return ret >= 0;
}

bool mqtt_on_unsubcribe(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	int count = 0;
	unsigned char dup = 0;
	unsigned short packetid = 0;
	int max_count = MAX_TOPIC_NR;
	unsigned char out[SMALL_BUFF_LEN];
	MQTTString topicFilters[MAX_TOPIC_NR];

	ret = MQTTDeserialize_unsubscribe(&dup, &packetid, max_count, &count, topicFilters, buf, len);
    return_value_if_fail(ret, false);
    
	/*TODO*/

    ret = MQTTSerialize_unsuback(out, sizeof(out), packetid);
	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_ping(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	unsigned char out[SMALL_BUFF_LEN];
    
    ret = MQTTSerialize_zero(out, sizeof(out), PINGRESP);
	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_disconnect(worker_t* w, unsigned char* buf, size_t len) {
	int ret = 0;
	unsigned char out[SMALL_BUFF_LEN];
    
    ret = MQTTSerialize_zero(out, sizeof(out), DISCONNECT);
	if(ret > 0) {
		ret = write(w->fd, out, ret);
	} 
	
	return ret >= 0;
}

bool mqtt_on_stats(worker_t* w, unsigned char* buf, size_t len) {
	return true;
}

bool mqtt_worker_work(worker_t* w) {
    unsigned char buf[4096];
    int fd = w->fd;
    int rret = read(fd, buf, sizeof(buf));

    if (rret > 0) {
    	unsigned char msg_type = buf[0] >> 4;

        s_stat.requests++;
        s_stat.recv_bytes += rret;

		switch(msg_type) {
			case PUBLISH: {
				return mqtt_on_publish(w, buf, rret);
			}
			case CONNECT: {
				return mqtt_on_connect(w, buf, rret);
			}
			case SUBSCRIBE: {
				return mqtt_on_subcribe(w, buf, rret);
			}
			case UNSUBSCRIBE: {
				return mqtt_on_unsubcribe(w, buf, rret);
			}
			case PINGREQ: {
				return mqtt_on_ping(w, buf, rret);
			}
			case DISCONNECT: {
				return mqtt_on_disconnect(w, buf, rret);
			}
			case 's': {
				return mqtt_on_stats(w, buf, rret);
			}
			default: {
				printf("unkown packet, close client.\n");
				return false;
			}
		}
    }

	return rret > 0;
}

bool mqtt_worker_init(worker_t* w) {
    (void)w;
    printf("mqtt_worker_init\n");
    return true;
}

bool mqtt_worker_deinit(worker_t* w) {
    (void)w;
    printf("mqtt_worker_deinit\n");
    return true;
}

static worker_ops_t s_mqtt_worker_ops = {
    mqtt_worker_init,
    mqtt_worker_work,
    mqtt_worker_deinit
};

worker_ops_t* mqtt_get_worker_ops() {
    return &s_mqtt_worker_ops;
}

