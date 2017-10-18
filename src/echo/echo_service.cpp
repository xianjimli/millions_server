#include <errno.h>

#include "utils.h"
#include "echo_service.h"

typedef struct _service_stat_t {
    time_t start;
    size_t requests;
    size_t recv_bytes;
    size_t send_bytes;
}service_stat_t;

static service_stat_t s_stat;

bool echo_service_init() {
    s_stat.start = time(0);

    return true;
}

bool echo_service_deinit() {
    return true;
}

static bool echo_worker_work(worker_t* w) {
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

static bool echo_worker_init(worker_t* w) {
    (void)w;
    printf("echo_worker_init\n");
    return true;
}

static bool echo_worker_deinit(worker_t* w) {
    (void)w;
    printf("echo_worker_deinit\n");
    return true;
}

static int echo_worker_read(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = read(w->fd, buf, len);
		if(ret > 0) {
			s_stat.recv_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static int echo_worker_read_n(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = read_n(w->fd, buf, len);
		if(ret > 0) {
			s_stat.recv_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static int echo_worker_write(worker_t* w, unsigned char* buf, int len) {
	if(w->fd > 0) {
		int ret = write_n(w->fd, buf, len);
		if(ret > 0) {
			s_stat.send_bytes += ret;
		}

		return ret;
	}
	
	return -1;
}

static worker_ops_t s_echo_worker_ops = {
    echo_worker_init,
    echo_worker_work,
    echo_worker_read,
    echo_worker_read_n,
    echo_worker_write,
    echo_worker_deinit
};

worker_ops_t* echo_get_worker_ops() {
    return &s_echo_worker_ops;
}

