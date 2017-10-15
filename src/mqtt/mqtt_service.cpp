#include <errno.h>

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

bool mqtt_worker_work(worker_t* w) {
    char buf[1024];
    int fd = w->fd;
    int wret = 0;
    int rret = read(fd, buf, sizeof(buf));

    if (rret > 0) {
        s_stat.requests++;
        s_stat.recv_bytes += rret;

        if(strncmp(buf, "stats", 5) == 0) {
            size_t speed = s_stat.requests/(time(0)-s_stat.start);
            snprintf(buf, sizeof(buf), "requests=%zu speed=%zur/s send=%zu recv=%zu\n", 
                s_stat.requests, speed, s_stat.send_bytes, s_stat.recv_bytes);
            rret = strlen(buf);
        }
        
        wret = write(fd, buf, rret);
        s_stat.send_bytes += wret;
    }

    if (rret <= 0 || wret <= 0){
        return false;
    }

    return true;
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

