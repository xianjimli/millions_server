#include "utils.h"
#include "worker.h"

typedef struct _workers_t {
    size_t size;
    size_t total;
    size_t delta;
    size_t capacity;

    bool quited;
    bool quiting;
    worker_t** workers;
    worker_t** free_workers;
    stShareStack_t* share_stack;
} workers_t;

static workers_t s_workers;

static bool workers_append(worker_t* w) {
    s_workers.workers[s_workers.total++] = w;

    return true;
}

static bool workers_push(worker_t* w) {
    return_value_if_fail(w != NULL && s_workers.size < s_workers.capacity, false);

    s_workers.free_workers[s_workers.size++] = w;
    printf("push: %d\n", (int)s_workers.size);

    return true;
}

static worker_t* workers_pop() {
    return_value_if_fail(s_workers.size > 0, NULL);

    printf("pop: %d\n", (int)s_workers.size);
    return s_workers.free_workers[--s_workers.size];
}

static void *worker_entry(void *arg) {
	worker_t* worker = (worker_t*)arg;

	co_enable_hook_sys();

    while(!s_workers.quiting) {
        if(worker->fd < 0) {
            workers_push(worker);
            co_yield_ct();

            continue;
        }

        while(!s_workers.quiting) {
            if(!fd_pool(worker->fd, 1800 * 10000)) {
                continue;
            }

            if(!worker->work(worker)) {
                if(worker->fd >= 0) {
                    close(worker->fd);
                    worker->fd = -1;
                }
                break;
            }
        }
    }

    if(worker->fd >= 0) {
        close(worker->fd);
        worker->fd = -1;
    }

    printf("worker %zu quit\n", worker->index); 

    return NULL;
}

static bool workers_prealloc(size_t nr) {
    size_t i = 0;
	stCoRoutineAttr_t attr;
    worker_t* works = NULL;
    return_value_if_fail((s_workers.total + nr) < s_workers.capacity, false);

    works = (worker_t*)calloc(nr, sizeof(worker_t));
    return_value_if_fail(works != NULL, false);

	attr.stack_size = 0;
	attr.share_stack = s_workers.share_stack;

    for(i = 0; i < nr; i++) {
        worker_t* iter = works+i;

        iter->fd = -1;
        iter->index = s_workers.total;
        co_create(&iter->co, &attr, worker_entry, iter);
        return_value_if_fail(iter->co != NULL, false);

        workers_push(iter);
        workers_append(iter);
    }

    printf("prealloc %zu/%zu\n", s_workers.total, s_workers.capacity);

    return true;
}

static bool workers_ensure() {
    if(s_workers.size > 0) {
        return true;
    }

    if(s_workers.total < s_workers.capacity) {
        return workers_prealloc(s_workers.delta);
    }

    return s_workers.size > 0;
}


bool workers_init(size_t max_nr, size_t delta) {
    memset(&s_workers, 0x00, sizeof(s_workers));

    s_workers.workers = (worker_t**)calloc(max_nr, sizeof(worker_t*));
    return_value_if_fail(s_workers.workers != NULL, false);
    
    s_workers.free_workers = (worker_t**)calloc(max_nr, sizeof(worker_t*));
    return_value_if_fail(s_workers.free_workers != NULL, false);
     
    s_workers.delta = delta;
    s_workers.capacity = max_nr;
    s_workers.share_stack = co_alloc_sharestack(1, 4 * 1024 * 1024);

    return workers_prealloc(delta);
}

bool wroker_start(worker_func func, int fd, unsigned short port) {
    return_value_if_fail(workers_ensure(), false);
    worker_t* worker = workers_pop();
    return_value_if_fail(worker != NULL, false);

    worker->fd = fd;
    worker->port = port;
    worker->work = func;
    
    co_resume(worker->co);

    return true;
}

static bool workers_resume_all_active() {
    size_t i = 0;

    for(i = 0; i < s_workers.total; i++) {
        worker_t* iter = s_workers.workers[i];
        if(iter->fd > 0) {
            co_resume(iter->co);
        }
    }

    return true;
}

static bool workers_has_active_worker() {
    size_t i = 0;

    for(i = 0; i < s_workers.total; i++) {
        worker_t* iter = s_workers.workers[i];
        if(iter->fd > 0) {
            return true;
        }
    }

    return false;
}

static bool workers_deinit() {
    size_t i = 0;

    for(i = 0; i < s_workers.total; i++) {
        worker_t* iter = s_workers.workers[i];

        if(iter->fd > 0) {
            close(iter->fd);
            iter->fd = -1;
        }

        co_release(iter->co);
    }

    return true;
}

static void* workers_join(void* args) {
    size_t i = 0;

    (void)args;
    for(i = 0; i < 10; i++) {
        if(workers_has_active_worker()) {
            printf(".");
            co_sleep(1000);
        }else{
            break;
        }
    }

    workers_deinit();
    s_workers.quited = true;

    fflush(stdout);

    return NULL;
}

bool workers_quit() {
	stCoRoutine_t *co = NULL;
    return_value_if_fail(!s_workers.quiting, false);

    s_workers.quiting = true;
    workers_resume_all_active();

	co_create(&co, NULL, workers_join, NULL);
	co_resume(co);

    return true;
}

bool workers_is_quiting() {
    return s_workers.quiting;
}

bool workers_is_quited() {
    return s_workers.quited;
}


