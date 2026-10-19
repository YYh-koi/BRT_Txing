/*************************************************************************
    > File Name: hlp_dun.h
    >
    >
    > Created Time: Fri 07 Aug 2015 11:37:27 AM CST
 ************************************************************************/

#include "btdatatype.h"

enum {
    ST_DUN_UNINIT,
    ST_DUN_INIT,
    ST_DUN_READY,
    ST_DUN_CONNECTING,
    ST_DUN_CONNECTED,
    ST_DUN_STARTED
};

/*
 * init dun .
 * */
void hlp_dun_init();

/*
 *
 * */
void hlp_dun_done();

/*
 *  start dun service
 * */
void hlp_dun_service_start();

/*
 *  stop dun service
 * */
void hlp_dun_service_stop();

/*
 * creat a rfcomm connect for DUN.
 * */
void hlp_dun_connect(UINT8 *bd);

/*
 * release dun rfcomm connection , kill pppd and ip-down.
 * */
void hlp_dun_disconnect();

UINT8 hlp_dun_get_status();

/*
 *  write data to ppp tty
 * */
UINT8 bt_write2ppp(UINT8 *data, UINT16 data_len);
