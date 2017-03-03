#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
//#include "driver/uart.h"
//#include "lwip_websocket.h"
//#include "gpio.h"
//#include "driver/pwm.h"

static ETSTimer tickTimer;

static void tickCb() {
    os_timer_arm(&tickTimer, 100, 0);
}

void startTickCb() {
    ets_printf("Starting tickCb\r\n");
    os_timer_disarm(&tickTimer);
    os_timer_setfn(&tickTimer, tickCb, NULL);
    os_timer_arm(&tickTimer, 100, 0);
}

