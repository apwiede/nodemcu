#include "osapi.h"
#include "mem.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "html_dataparser.h"
#include "connection_list.h"

int send_chunk(struct tcp_pcb * pcb, char * data, int len) {
    int file = 0;
ets_printf("send_chunk\r\n");
    connections * con = getConnection(pcb);
    if (con != 0) {
        con->filepos = 0;
        long filesize = 0;
        con->dataleft = ro_file_system.files[file].size;
        int flag = TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY;
        char * d = os_malloc(os_strlen(HEADER_OK) + 1);
        tcp_output(pcb);
        con->data = &rofs_data[ro_file_system.files[file].offset];
        os_memcpy(d, HEADER_OK, os_strlen(HEADER_OK));
        d[os_strlen(HEADER_OK)] = 0;
        ets_printf("sending:%s \r\n", d);
        tcp_write(pcb, d, os_strlen(HEADER_OK), flag);
        tcp_output(pcb);
        os_free(d);
        ets_printf("done sending:%s \r\n", d);
        if (con->dataleft == 0) {
            return 0;
        }
    }
    return -1;
    //os_free(d);
}
