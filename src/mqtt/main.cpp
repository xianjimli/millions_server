#include <signal.h>
#include "service.h"
#include "mqtt_service.h"

static void on_quit(int signal){
    printf("(%d):quiting...\n", signal);
	service_quit();
}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("Usage: %s ip port instances\n", argv[0]);

        return 0;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int instances = atoi(argv[3]);
    int delta = instances > 10 ? instances/10 : 1;

	signal(SIGINT, on_quit);
    workers_init(instances, delta);

    mqtt_service_init();
    service_start(ip, port, mqtt_get_worker_ops());

    service_loop();
    mqtt_service_deinit();

    return 0;
}

