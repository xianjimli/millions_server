#ifndef SERVICE_H
#define SERVICE_H

#include "worker.h"

bool service_start(const char* ip, int port, worker_ops_t* ops);

bool service_loop();
bool service_quit();

#endif//SERVICE_H

