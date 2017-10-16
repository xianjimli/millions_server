#ifndef WORKER_H
#define WORKER_H

#include "types_def.h"
#include "co_routine.h"

struct _worker_t;
typedef struct _worker_t worker_t;

typedef bool (*worker_func)(worker_t* w);
typedef int (*work_write_n_func)(worker_t* w, unsigned char* buf, int len);

typedef struct _worker_ops_t {
    worker_func init;
    worker_func work;
	work_write_n_func write_n;
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

#endif//WORKER_H

