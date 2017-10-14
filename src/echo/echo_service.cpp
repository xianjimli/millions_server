#include <errno.h>

#include "utils.h"
#include "echo_service.h"

bool echo_work(worker_t* w) {
    char buf[1024];
    int fd = w->fd;
    int wret = 0;
    int rret = read(fd, buf, sizeof(buf));

    printf("%d read:%d\n", fd, rret);
    if (rret > 0) {
        wret = write(fd, buf, rret);
        printf("%d read:%d\n", fd, wret);
    }

    if (rret <= 0 || wret <= 0){
        return false;
    }

    return true;
}

