/*************************************************************************
    > File Name: auto_app/hlp_pan.h
    > Author: hualiangyu
    > Mail:
    > Created Time: Thu 25 Jun 2015 07:33:15 PM CST
 ************************************************************************/


#include "btdatatype.h"
#include "bt_event.h"
//#include "pan_hlp.h"
#include "pan_ui.h"
#include "sdap_uistru.h"
#include "sdap_uimacro.h"
#include "btinit.h"

enum {
    ST_PAN_UNINIT,
    ST_PAN_INIT,
    ST_PAN_CONNECTING,
    ST_PAN_CONNECTED,
    ST_PAN_DISCONNECTING
};

UINT8 hlp_pan_get_status(void);
void hlp_pan_connect(UINT8 *bd);
void hlp_pan_disconnect();
void hlp_pan_init(module_event_cbk_t event_cbk);
void hlp_pan_done();
#define PAN_NOT_STARTED         0x0000
#define BTSDK_CLS_PAN_PANU      0x0001
