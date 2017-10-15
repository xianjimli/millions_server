#ifndef ECHO_SERVICE_H
#define ECHO_SERVICE_H

#include "worker.h"

bool echo_service_init();
bool echo_service_deinit();

worker_ops_t* echo_get_worker_ops();

#endif//ECHO_SERVICE_H

