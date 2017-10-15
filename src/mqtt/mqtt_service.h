#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include "worker.h"

bool mqtt_service_init();
bool mqtt_service_deinit();

worker_ops_t* mqtt_get_worker_ops();

#endif//MQTT_SERVICE_H

