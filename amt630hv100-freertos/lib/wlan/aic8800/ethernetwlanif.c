
#include <stdio.h>
#include "netif.h"

int wps_connect_done;
err_t aic_net_if_init(struct netif *net_if);

void cmd_test(const char* temp_uart_buf)
{
    (void)temp_uart_buf;
}

err_t
wlan_ethernetif_init(struct netif *netif)
{
    return aic_net_if_init(netif);
}

int wifi_send_network_data(const char* buf, int len)//for FreeRTOS-Plus-TCP 
{
    //todo

    (void)buf;
    (void)len;

    return 0;
}

