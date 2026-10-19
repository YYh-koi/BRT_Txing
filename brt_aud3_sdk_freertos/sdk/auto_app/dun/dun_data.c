/*************************************************************************
    > File Name: auto_app/dun/dun_data.c
    >
    >
    > Created Time: Tue 25 Aug 2015 11:24:44 AM CST
 ************************************************************************/

#include<stdio.h>

#include "btdatatype.h"
#include "global.h"

void DUN_TL_DataCfm(UINT16 offset, UINT8 out_index)
{
//TODO
}

void DUN_TL_DataReceive(UINT8 *data, UINT16 data_len)
{
	bt_write2ppp(data, data_len);
}
void DUN_TL_DataSend(struct BuffStru *data)
{
	int out_credit = 0;
	UINT32 dun_conn_hdl = hlp_get_dun_connhdl();
	if(dun_conn_hdl == 0) {
		return ;
	}

	app_spp_send_data(dun_conn_hdl, data);
	out_credit = app_spp_get_out_credit(dun_conn_hdl);
	if(out_credit > 0) {

	} else {

	}
}

