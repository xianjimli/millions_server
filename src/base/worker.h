#ifndef WORKER_H
#define WORKER_H

#include "types_def.h"
#include "co_routine.h"

struct _worker_t;
typedef struct _worker_t worker_t;

typedef bool (*worker_func)(worker_t* w);
typedef int (*work_read_func)(worker_t* w, unsigned char* buf, int len);
typedef int (*work_read_n_func)(worker_t* w, unsigned char* buf, int len);
typedef int (*work_write_func)(worker_t* w, unsigned char* buf, int len);

typedef struct _worker_ops_t {
    worker_func init;
    worker_func work;
	work_read_func read;
	work_read_n_func read_n;
	work_write_func write;
    worker_func deinit;
}worker_ops_t;

struct _worker_t {
    stCoRoutine_t* co;
    worker_ops_t* ops;
    int fd;
    unsigned short port;

    size_t index;
    size_t recv_bytes;
    size_t send_bytes;
    size_t requests;

    void* userdata;
};

bool workers_init(size_t max_nr, size_t delta);
bool wroker_start(worker_ops_t* ops, int fd, unsigned short port);

bool workers_quit();
bool workers_is_quiting();
bool workers_is_quited();

static inline int worker_read(worker_t* w, unsigned char* buf, int len) {
	return_value_if_fail(w != NULL && w->ops != NULL && w->ops->read != NULL && buf != NULL, -1);

	return w->ops->read(w, buf, len);
}

static inline int worker_read_n(worker_t* w, unsigned char* buf, int len) {
	return_value_if_fail(w != NULL && w->ops != NULL && w->ops->read_n != NULL && buf != NULL, -1);

	return w->ops->read_n(w, buf, len);
}

static inline int worker_write(worker_t* w, unsigned char* buf, int len) {
	return_value_if_fail(w != NULL && w->ops != NULL && w->ops->write != NULL && buf != NULL, -1);

	return w->ops->write(w, buf, len);
}


#endif//WORKER_H

