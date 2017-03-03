#include "osapi.h"
#include "mem.h"
#include "websocket_server.h"

void  websocket_data(char *data, int size) {
ets_printf("websocket_data: data: %s size: %d\n", data, size);
    websocket_writedata("Hello Answer from Server\n");
}

