#include "utils.h"
#include "service.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int co_accept(int fd, struct sockaddr *addr, socklen_t *len);

static void *accept_routine(void* arg) {
    int fd = 0;
    int sleep_time = 0;
    int timeout_times = 0;
    struct sockaddr_in addr;
    worker_t* service = (worker_t*)arg;
    int listen_fd = service->fd;
    socklen_t len = sizeof(addr);

	co_enable_hook_sys();

	while(!workers_is_quiting()) {
		memset(&addr, 0, sizeof(addr));
		fd = co_accept(listen_fd, (struct sockaddr *)&addr, &len);
		if (fd < 0) {
		    timeout_times++;
		    sleep_time = timeout_times < 1000 ? timeout_times : 1000;
		    if(sleep_time > 5) {
		        co_sleep(sleep_time);
		    }
			continue;
		}else{
		    timeout_times = 0;
		}

		fd_set_nonblock(fd);
        service->requests++;

        if(!wroker_start(service->work, fd, service->port)) {
            close(fd);
		    co_sleep(1000);
		    printf("start worker failed %d\n", fd);
        }
	}

    printf("service quit: requests=%d\n", service->port, service->requests);

	return NULL;
}

bool service_start(const char* ip, int port, worker_func work) {
    int fd = 0;
    int ret = 0;
    worker_t * service = NULL;
	stCoRoutine_t *co = NULL;

    return_value_if_fail(ip != NULL && work != NULL, false);

    service = (worker_t*)calloc(1, sizeof(worker_t));
    return_value_if_fail(service != NULL, false);

	fd = socket_create(port, ip, true);
	return_value_if_fail(fd >= 0, false);

	ret = listen(fd, 1024);
    return_value_if_fail(ret >= 0, false);

	printf("listen %d %s:%d\n", fd, ip, port);

    service->fd = fd;
    service->port = port;
    service->work = work;

	fd_set_nonblock(fd);
	co_create(&co, NULL, accept_routine, service);
	co_resume(co);

	return true;
}

static int check_quit(void *) {
    return workers_is_quited() ? -1 : 0;
}

bool service_loop() {
	co_eventloop(co_get_epoll_ct(), check_quit, 0);

	return true;
}

bool service_quit() {
    return workers_quit();
}

