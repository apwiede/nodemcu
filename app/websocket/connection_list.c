//#include "espmissingincludes.h"
#include "connection_list.h"
//#include "lwip/mem.h"
#include "osapi.h"
#include "mem.h"

static connections * connection_list[HTTP_POOL] = { 0 };

connections* findWebsocketConnection() {
ets_printf("findWebsocketConnection\n");
    for (int i = 0; i < HTTP_POOL; i++) {
        if (connection_list[i] != 0 && connection_list[i]->websocket == 1) {
            connection_list[i]->pos = i;
            return connection_list[i];
        }
    }
    return 0;
}

void deleteConnection(struct tcp_pcb* pcb) {
ets_printf("deletConnection\n");
    for (int i = 0; i < HTTP_POOL; i++) {
        if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
            ets_printf("deleteconn %p \r\n", connection_list[i]);
            os_free(connection_list[i]);
            connection_list[i] = 0;
        }
    }
}
connections* newConnection(struct tcp_pcb* pcb) {
ets_printf("newConnection\n");
    for (int i = 0; i < HTTP_POOL; i++) {
        if (connection_list[i] == 0) {
            ets_printf("newConnection %d - %p - %d \r\n", i, connection_list[i], sizeof(connections));
            connection_list[i] = os_malloc(sizeof(connections));
            ets_printf("malloc done \r\n");
            connection_list[i]->websocket = 0;
            connection_list[i]->timeout = 0;
            connection_list[i]->dataleft = 0;
            connection_list[i]->filepos = 0;
            connection_list[i]->connection = pcb;
            ets_printf("newConnection done \r\n");
            return connection_list[i];
        }
    }
    return 0;
}

connections* getConnectionOffset(struct tcp_pcb * pcb, int off) {
ets_printf("getConnectionOffset\n");
    //ets_printf("find connection: %p \r\n", pcb);
    for (int i = off + 1; i < HTTP_POOL; i++) {
        if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
            connection_list[i]->pos = i;
            return connection_list[i];
        }
    }
    return 0;
}

connections** getConnectionsBegin() {
    return connection_list;
}

connections* getConnection(struct tcp_pcb * pcb) {
ets_printf("getConnection\n");
    //ets_printf("find connection: %p \r\n", pcb);
    for (int i = 0; i < HTTP_POOL; i++) {
        if (connection_list[i] != 0 && connection_list[i]->connection == pcb) {
            connection_list[i]->pos = i;
            return connection_list[i];
        }
    }
    return 0;
}

