/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_stream.c
*
*	Description:
*
*
*	Create Date:		2014/07/12
***********************************************************************/

#include "internal_stream.h"
#include "auto_stream.h"
#include "fsm.h"
#include "brt_trace.h"
#include "auto_hf.h"
#include "auto_a2dp.h"

#define TAG "bse_stream"

#ifdef CONFIG_BT_CHIP_CSR8350

static app_stream_data_t *s_stream_data = NULL;
static struct FsmInst *s_stream_fsminst = NULL;

void app_stream_hlp_start(UINT8 stream_type, UINT16 acl_handle, UINT16 cid);
void app_stream_hlp_stop(UINT8 stream_type);
void app_stream_hlp_event_started(void *param);
void app_stream_hlp_event_stopped(void *param);
void app_stream_hlp_restart(void);

void fsm_stream_server_start(struct FsmInst *fi, UINT8 event, void *in);
void fsm_stream_open(struct FsmInst *fi, UINT8 event, stream_list_t *in);
void fsm_stream_start(struct FsmInst *fi, UINT8 event, void *in);
void fsm_stream_stop(struct FsmInst *fi, UINT8 event, void *in);
void fsm_stream_close(struct FsmInst *fi, UINT8 event, void *in);
void fsm_stream_event_cbk(struct FsmInst *fi, UINT8 event, void *in);
void timer_stream_re_operator(void *arg);

//the API for audio stream module
void app_stream_init(void)
{
	app_stream_data_t *userdata = NEW(sizeof(app_stream_data_t));

	memset(userdata, 0, sizeof(app_stream_data_t));
	s_stream_fsminst = FsmInstNew(userdata, "STREAM");

	FsmEvent_ExternalTx(s_stream_fsminst, fsm_stream_server_start, NULL);
}

void app_stream_patch_complete(void)
{
	//if the stream channel is working we must stop it!
	//TODO
	//Destory the memory and FSM resource of stream mudule.
	bse_log_api(TAG, "[dsp][fsm]app_stream_patch_complete\r\n");
	NewListRemoveAll(s_stream_data->oper_queue);
	NewListRemoveAll(s_stream_data->stream_list);
	s_stream_data->oper_queue->head = NULL;
	s_stream_data->stream_list->head = NULL;
	app_stream_set_type(STREAM_IDLE);
	//BtNewListFree(s_stream_data->oper_queue);
	//BtNewListFree(s_stream_data->stream_list);
}

void app_stream_open(UINT8 stream_type, UINT8 *p_name, UINT16 acl_handle, UINT16 cid)
{
	stream_list_t *p_param = (stream_list_t *)NEW(sizeof(stream_list_t));

	bse_log_api(TAG, "[dsp][fsm]app_stream_open %d\r\n", stream_type);
	memset(p_param, 0, sizeof(stream_list_t));
	p_param->stream_type = stream_type;
	memcpy(p_param->name, p_name, strlen(p_name) > CONFIG_STREAM_NAME_LEN ? CONFIG_STREAM_NAME_LEN - 1 : strlen(p_name));
	p_param->acl_handle = acl_handle;
	p_param->cid = cid;
	FsmEvent_ExternalTx(s_stream_fsminst, fsm_stream_open, p_param);
}

void app_stream_start(UINT8 stream_type)
{
	bse_log_api(TAG, "[dsp][fsm]app_stream_start (%d)\r\n", stream_type);
	FsmEvent_ExternalTx(s_stream_fsminst, fsm_stream_start, (void *)(UINT32) stream_type);
}

void app_stream_stop(UINT8 stream_type)
{
	bse_log_api(TAG, "[dsp][fsm]app_stream_hlp_stop, type(%d)\r\n", stream_type);
	FsmEvent_ExternalTx(s_stream_fsminst, fsm_stream_stop, (void *)(UINT32) stream_type);
}

void app_stream_close(UINT8 stream_type, UINT8 *p_name)
{
	bse_log_api(TAG, "[dsp][fsm]app_stream_close, you want to close [name=%s, %d]\r\n", p_name, stream_type);
	FsmEvent_ExternalTx(s_stream_fsminst, fsm_stream_close, (void *)(UINT32) stream_type);
}

void app_stream_test_count_oper(void)
{
	int i = 0;
	stream_oper_t *p_oper = s_stream_data->oper_queue->head;

	while (p_oper != NULL) {
		bse_log_verbose(TAG, "[dsp][fsm][i=%d] stream_type=%d, oper_type=%d\r\n", i++, p_oper->stream_type, p_oper->ctl_type);
		p_oper = (stream_oper_t *)NEXT_MEM(s_stream_data->oper_queue, p_oper);
	}
}

void app_stream_event_cbk(UINT8 event_id, void *param)
{
	FsmEvent_ExternalCx(s_stream_fsminst, event_id, fsm_stream_event_cbk, param);
}

//The internal functions for audio stream module
void app_stream_set_status(UINT8 status)
{
	if (s_stream_data != NULL) {
		bse_log_info(TAG, "[dsp][fsm] app_stream_set_status, you set %d\r\n", status);
		s_stream_fsminst->state = status;
	}
}

UINT8 app_stream_get_status(void)
{
	if (s_stream_fsminst != NULL) {
		return s_stream_fsminst->state;
	} else {
		return 0;
	}
}

void hlp_stream_add_re_oper(UINT8 stream_type, UINT8 ctl_type)
{
	//TODO . we need add the timer for stream operator, if stream is no response we can process it in the function of timeout.
#if 0
	stream_oper_t *p_oper = (stream_oper_t *)NEW(sizeof(stream_oper_t));

	bse_log_api(TAG, "[app_stream.c] hlp_stream_add_re_oper---->");
	p_oper->ctl_type = ctl_type;
	p_oper->stream_type = stream_type;
	s_stream_data->cur_oper = p_oper;
	s_stream_data->re_oper = FsmAddTimer(NULL, CONFIG_STREAM_REOPERTOR_TIMEOUT, 0, NULL, NULL, timer_stream_re_operator);
#else
	bse_log_api(TAG, "[dsp][fsm] hlp_stream_add_re_oper---->");
#endif
}
void hlp_stream_remove_re_oper(void)
{
	//TODO . we need destory the timer when stream is reponse normal.
#if 0
	bse_log_api(TAG, "[dsp][fsm] hlp_stream_remove_re_oper---->");
	FREE(s_stream_data->cur_oper);
	if (s_stream_data->re_oper != NULL) {
		FSM_DelTimer(&(s_stream_data->re_oper));
	}
#else
	bse_log_api(TAG, "[dsp][fsm] hlp_stream_remove_re_oper---->");
#endif
}

void app_stream_hlp_start(UINT8 stream_type, UINT16 acl_handle, UINT16 cid)
{
	stream_oper_t* destroy_stream_oper = NULL;
	UINT8 destroy_oper = STREAM_CTL_TYPE_STOP;
	struct BtList *list = s_stream_data->oper_queue;

	bse_log_api(TAG, "[dsp][fsm]app_stream_hlp_start (%d)  operate_que num(%d) stream state(%d)\r\n",stream_type,s_stream_data->oper_queue->number,app_stream_get_status());
#if 1
	destroy_stream_oper = (stream_oper_t *)(list->head);
	while (destroy_stream_oper != NULL) {
		bse_log_verbose(TAG, "[dsp][fsm]start show-operlist:stream_type(%d) stream_ctl(%d)\r\n",destroy_stream_oper->stream_type,destroy_stream_oper->ctl_type);
		destroy_stream_oper = (stream_oper_t *)NEXT(destroy_stream_oper);
	}
#endif
	if (app_stream_get_status() == APP_ST_STREAM_READY) {
		bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_start ready to start(%d)!!\r\n",stream_type);

		destroy_stream_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &destroy_oper, 1);
		if ((destroy_stream_oper != NULL)&&(destroy_stream_oper->stream_type == stream_type)) {
			bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_start have stop stream_oper but unactive\r\n");
			NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)destroy_stream_oper);
			BtListMemberFree((UINT8 *)destroy_stream_oper);

			//ignore invalid start operator, do next start operator. add by cjs
			app_stream_hlp_restart();
			return;
		}

		app_stream_set_type(stream_type);

		app_stream_set_status(APP_ST_STREAM_STARTING);
		hlp_audio_start(stream_type);
	} else if (app_stream_get_status() == APP_ST_STREAM_WORKING) {
		if (app_stream_get_type() == stream_type) {
			bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_start this stream is working,so do null\r\n");
		} else {
			stream_oper_t *p_oper = (stream_oper_t *)BtListMemberNew(sizeof(stream_oper_t));
			bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_start ,two serial start operate,firs stop the working stream\r\n");

			if (p_oper != NULL) {
				p_oper->stream_type = stream_type;
				p_oper->ctl_type = STREAM_CTL_TYPE_START;
				p_oper->acl_handle = acl_handle;
				p_oper->cid = cid;
				bse_log_info(TAG, "[dsp][fsm] app_stream_hlp_start, insert oper_queue stream_type1(%d) ctl_type(%d) ,stop next\r\n", p_oper->stream_type, p_oper->ctl_type);
				NewListAddTail(s_stream_data->oper_queue, p_oper);
			}
			stream_oper_t *p_head = (stream_oper_t *)NewListGetHead(s_stream_data->oper_queue);
			bse_log_info(TAG, "[dsp] enter app_stream_hlp_start p_head: %p, p_oper: %p", p_head, p_oper);
			if ((p_head != NULL) && (p_head != p_oper)) {
				bse_log_info(TAG, "[dsp] enter app_stream_hlp_start p_head->ctl_type: %d", p_head->ctl_type); 
				if (p_head->ctl_type == STREAM_CTL_TYPE_STOP) {
					app_stream_hlp_stop(p_head->stream_type);
				}
			}
			//change interface, in case of stream_type list is null. 
			//app_stream_hlp_stop(app_stream_get_type());
		}
	} else {
		stream_oper_t *p_oper = (stream_oper_t *)BtListMemberNew(sizeof(stream_oper_t));
		bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_start, stream_status(%d),add start operate to list\r\n",app_stream_get_status());
		if (p_oper != NULL) {
			p_oper->stream_type = stream_type;
			p_oper->ctl_type = STREAM_CTL_TYPE_START;
			p_oper->acl_handle = acl_handle;
			p_oper->cid = cid;
			bse_log_info(TAG, "[dsp][fsm] app_stream_hlp_start, insert oper_queue stream_type2(%d) ctl_type(%d)\r\n", p_oper->stream_type, p_oper->ctl_type);
			NewListAddTail(s_stream_data->oper_queue, p_oper);
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++
  func: check operator list, start pending operator
----------------------------------------------------*/
void app_stream_hlp_restart(void)
{
	if (s_stream_data->oper_queue->number) {
		stream_oper_t* p_oper = NULL;
		UINT8 start_oper = STREAM_CTL_TYPE_START;
		bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_restart. find next start operator\r\n");

		p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &start_oper, 1);
		if (p_oper) {
			UINT8 stream_type = p_oper->stream_type;
			UINT16 acl_handle =	 p_oper->acl_handle;
			UINT16 cid = p_oper->cid;
			//remove oper before start, in case of loop in func app_stream_hlp_restart.
			NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
			BtListMemberFree((UINT8 *)p_oper);

			app_stream_hlp_start(stream_type, acl_handle, cid);
		}
	}
}

void app_stream_hlp_stop(UINT8 stream_type)
{
	//do not care start opereator in the oper_queue
	bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_stop, stream type(%d), state(%d)\r\n", stream_type, app_stream_get_status());
	if (stream_type == app_stream_get_type()) {
		if (app_stream_get_status() == APP_ST_STREAM_WORKING) {
            app_stream_set_status(APP_ST_STREAM_STOPPING);
			bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_stop, stop the cur_type(%d)\r\n",s_stream_data->cur_stream_type);
			hlp_audio_stop(stream_type);
		} else if (app_stream_get_status() != APP_ST_STREAM_READY) {
			// if stream is not working, then add stop_operate to operate queue
			stream_oper_t *p_oper = NULL;
			p_oper = (stream_oper_t *)BtListMemberNew(sizeof(stream_oper_t));
			if (p_oper != NULL) {
				p_oper->stream_type = stream_type;
				p_oper->ctl_type = STREAM_CTL_TYPE_STOP;
				p_oper->acl_handle = NULL;
				p_oper->cid = NULL;
				bse_log_info(TAG, "[dsp][fsm]app_stream_hlp_stop, insert queue stream_type(%d)  ctrl_type(%d)\r\n", p_oper->stream_type, p_oper->ctl_type);
				NewListAddTail(s_stream_data->oper_queue, p_oper);
			}
		} else {
			bse_log_warning(TAG, "[dsp][fsm]app_stream_hlp_stop ,the stream to be operated has not started,so ignore this.\r\n");
		}
	} else {
		//the stream have not be operated
		bse_log_warning(TAG, "[dsp][fsm]app_stream_hlp_stop ,the stream to be operated is not started or not starting\r\n");
	}
}

void app_stream_hlp_event_started(void *param)
{
	UINT8 next_oper = STREAM_CTL_TYPE_STOP;
	UINT8 start_oper = STREAM_CTL_TYPE_START;
	stream_oper_t *p_oper = NULL;
	stream_oper_t *start_stream_oper = NULL;
	struct BtList *list = s_stream_data->oper_queue;

	app_stream_set_status(APP_ST_STREAM_WORKING);

	//remove the start operate of the stream_type
	start_stream_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &start_oper, 1);;

	if (start_stream_oper != NULL && start_stream_oper->stream_type == s_stream_data->cur_stream_type) {
		bse_log_info(TAG, "[dsp][fsm]started_event remove the start operate\r\n");
		NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)start_stream_oper);
		BtListMemberFree((UINT8 *)start_stream_oper);
	}

	//execused next stop operate of the stream
	p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &next_oper, 1);
	if (p_oper != NULL) {
		bse_log_info(TAG, "[dsp][fsm]started_event,next_operate stream_type(%d), ctl_type(%d)\r\n", p_oper->stream_type, p_oper->ctl_type);
		if (p_oper->stream_type == s_stream_data->cur_stream_type) {
			bse_log_info(TAG, "[dsp][fsm][warning]started_event, continue do stop type(%d)\r\n", p_oper->stream_type);
			app_stream_hlp_stop(p_oper->stream_type);
		} else {
			bse_log_info(TAG, "[dsp][fsm][warning]stream_started_event, find stop operate but not right type(%d)\r\n",p_oper->stream_type);
		}
	} else {
		//sometimes start operator is in oper_queue, do it after stream started.such as do a2dp playing/dial out/hung up frequently.
		next_oper = STREAM_CTL_TYPE_START;
		p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &next_oper, 1);
		if (p_oper != NULL) {
			UINT8 stream_type = p_oper->stream_type;
			UINT16 acl_handle = p_oper->acl_handle;
			UINT16 cid =  p_oper->cid;

			NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
			BtListMemberFree((UINT8 *)p_oper);

			bse_log_info(TAG, "[dsp][fsm]started_event, continue do start type(%d)\r\n", stream_type);
			app_stream_hlp_start(stream_type, acl_handle, cid);
			return;
		} else {
			bse_log_warning(TAG, "[dsp][fsm]started_event, No have next operator\r\n");
			return;
		}
	}

	//when operate stop_stream in started_success_event, then remove the stop operate
	NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
	BtListMemberFree((UINT8 *)p_oper);
}

void app_stream_hlp_event_stopped(void *param)
{
	UINT8 next_oper = STREAM_CTL_TYPE_START;
	UINT8 todestroy_oper = STREAM_CTL_TYPE_STOP;
	stream_oper_t *p_stream_destory = NULL;
	stream_oper_t *p_oper = NULL;
	struct BtList *list = s_stream_data->oper_queue;

	//remove the stop operate of the stream_type
	p_stream_destory = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &todestroy_oper, 1);
	if ((p_stream_destory != NULL)&&(p_stream_destory->stream_type == s_stream_data->cur_stream_type)) {
		bse_log_info(TAG, "[dsp][fsm]stoped_event remove stop operate\r\n");
		NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_stream_destory);
		BtListMemberFree((UINT8 *)p_stream_destory);
	}

	app_stream_set_status(APP_ST_STREAM_READY);
	app_stream_set_type(STREAM_IDLE);

	//execused next start operate of the stream
	next_oper = STREAM_CTL_TYPE_START;
	p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &next_oper, 1);
	if (p_oper != NULL) {
		UINT8 stream_type = p_oper->stream_type;
		UINT16 acl_handle =  p_oper->acl_handle;
		UINT16 cid = p_oper->cid;

		bse_log_info(TAG, "[dsp][fsm]stoped_event, continute do start type(%d)\r\n", p_oper->stream_type);
		NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
		BtListMemberFree((UINT8 *)p_oper);
		app_stream_hlp_start(stream_type, acl_handle, cid);
	}
}
void hlp_stream_event_startfail(void *param)
{
	UINT8 p_start_oper = STREAM_CTL_TYPE_START;
	UINT8 p_next_oper = STREAM_CTL_TYPE_STOP;
	stream_oper_t* p_oper = NULL;
	bse_log_api(TAG, "[dsp][fsm]start_fail event. stream_type(%d)\r\n",s_stream_data->cur_stream_type);
	p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &p_start_oper, 1);
	if (p_oper && (p_oper->stream_type == s_stream_data->cur_stream_type)) {
		if (app_stream_get_status() == APP_ST_STREAM_STARTING) {
			bse_log_info(TAG, "[dsp][fsm]start_fail event remove the start operate.set stream ready\r\n");
			NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
			BtListMemberFree((UINT8 *)p_oper);
		}
	}
	p_oper = NULL;
	p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &p_next_oper, 1);
	if (p_oper && (p_oper->stream_type == s_stream_data->cur_stream_type)) {
		bse_log_info(TAG, "[dsp][fsm]start_fail event remove the stream stop operate\r\n");
		NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
		BtListMemberFree((UINT8 *)p_oper);
	}

	//do not change stream type, change status to stopping,wait stream stop event. add by cjs.
	//follow will do the destroy operate to flash the stream status
	app_stream_set_status(APP_ST_STREAM_STOPPING);
}
void hlp_stream_event_stopfail(void *param)
{
	UINT8 stop_oper = STREAM_CTL_TYPE_STOP;
	stream_oper_t* p_oper = NULL;
	stream_oper_t *p_oper_next = NULL;
	bse_log_api(TAG, "[dsp][fsm]stop_fail event.stream_type(%d)\r\n",s_stream_data->cur_stream_type);
	p_oper = (stream_oper_t *)NewListFind(s_stream_data->oper_queue, &stop_oper, 1);
	if (p_oper && (p_oper->stream_type == s_stream_data->cur_stream_type)) {
		if (app_stream_get_status() == APP_ST_STREAM_STOPPING) {
			bse_log_info(TAG, "[dsp][fsm]stop_fail event remove the stop operate,set stream ready\r\n");
			NewListRemoveAt(s_stream_data->oper_queue, (UINT8 *)p_oper);
			BtListMemberFree((UINT8 *)p_oper);
		}
	}
}

void fsm_stream_server_start(struct FsmInst *fi, UINT8 event, void *in)
{
	if (s_stream_data == NULL) {
		s_stream_data = (app_stream_data_t *)fi->user_data;
	}
	memset(s_stream_data, 0, sizeof(app_stream_data_t));

	s_stream_data->stream_list = BtNewListNew(sizeof(stream_list_t));
	s_stream_data->oper_queue = BtNewListNew(sizeof(stream_oper_t));

	app_stream_set_type(STREAM_IDLE);
	app_stream_set_status(APP_ST_STREAM_INIT);
}

void fsm_stream_open(struct FsmInst *fi, UINT8 event, stream_list_t *in)
{
	stream_list_t *p_stream = NewListFind(s_stream_data->stream_list, &(in->stream_type), 1);

	bse_log_api(TAG, "[dsp][fsm]fsm_stream_open type(%d)  stream_list num(%d)\r\n",in->stream_type,s_stream_data->oper_queue->number);
	if ((p_stream != NULL) && (p_stream->stream_type == in->stream_type) && (p_stream->acl_handle == in->acl_handle) && (p_stream->cid == in->cid)) {
		bse_log_info(TAG, "[dsp][fsm]fsm_stream_open, the stream is exist [name=%s]\r\n", in->name);

		FREE(in);
		return;
	}
	p_stream = (stream_list_t *)BtListMemberNew(sizeof(stream_list_t));
	if (p_stream == NULL) {
		bse_log_error(TAG, "[dsp][fsm]fsm_stream_open, new list node fail!\r\n");
		FREE(in);
		return;
	}

	memset(p_stream, 0, sizeof(stream_list_t));
	p_stream->stream_type = in->stream_type;
	memcpy(p_stream->name, in->name, strlen(in->name) > CONFIG_STREAM_NAME_LEN ? CONFIG_STREAM_NAME_LEN - 1 : strlen(in->name));
	p_stream->acl_handle = in->acl_handle;
	p_stream->cid = in->cid;

	bse_log_info(TAG, "[dsp][fsm]fsm_stream_open, add new stream node successful [name=%s]\r\n", in->name);
	NewListAddTail(s_stream_data->stream_list, (UINT8 *)p_stream);
	FREE(in);

	hlp_audio_open(p_stream->cid, p_stream->acl_handle, p_stream->stream_type);

}

void fsm_stream_start(struct FsmInst *fi, UINT8 event, void *in)
{
	UINT8 stream_type = (UINT8)(UINT32) in;
	stream_list_t *p_stream = NewListFind(s_stream_data->stream_list, &stream_type, 1);

	if (p_stream == NULL) {
		bse_log_error(TAG, "[dsp][fsm]fsm_stream_start, you must open the [type=%d] stream before start!\r\n", stream_type);
		return;
	}

	app_stream_hlp_start(p_stream->stream_type, p_stream->acl_handle, p_stream->cid);
}

void fsm_stream_stop(struct FsmInst *fi, UINT8 event, void *in)
{
	UINT8 stream_type = (UINT8)(UINT32) in;
	stream_list_t *p_stream = NewListFind(s_stream_data->stream_list, &stream_type, 1);

	if (p_stream == NULL) {
		bse_log_error(TAG, "[dsp][fsm][warning]fsm_stream_stop, you must open the [type=%d] stream before stop!\r\n", stream_type);
		return;
	}

	app_stream_hlp_stop(p_stream->stream_type);
}

void fsm_stream_close(struct FsmInst *fi, UINT8 event, void *in)
{
	UINT8 stream_type = (UINT8)(UINT32) in;
	stream_list_t *p_stream = NewListFind(s_stream_data->stream_list, &stream_type, 1);

	if (p_stream != NULL) {
		//remove the stream from stream_list
		bse_log_info(TAG, "[dsp][fsm]fsm_stream_close, to close streamtype(%d) stream_status(%d)\r\n", p_stream->stream_type,app_stream_get_status());
		NewListRemoveAt(s_stream_data->stream_list, (UINT8 *)p_stream);
		BtListMemberFree((UINT8 *)p_stream);
	} else {
		bse_log_info(TAG, "[dsp][fsm]app_stream_close, the stream is not exsit %d\r\n", stream_type);
	}
}

void fsm_stream_event_cbk(struct FsmInst *fi, UINT8 event, void *in)
{
	bse_log_api(TAG, "[dsp][fsm]stream_event, event=%d (0-start, 1-stop, 2-start fail, 3-stop fail, 4 patch complete) cur_type(%d) hlp_stream_type(%d)\r\n", event, s_stream_data->cur_stream_type,hlp_stream_get_type());

	switch (event) {
	case APP_EV_STREAM_STARTED:
		app_stream_hlp_event_started(in);
		break;
	case APP_EV_STREAM_STOPPED:
		app_stream_hlp_event_stopped(in);
		break;
	case APP_EV_STREAM_START_FAIL:
		hlp_stream_event_startfail(in);
		break;
	case APP_EV_STREAM_STOP_FAIL:
		hlp_stream_event_stopfail(in);
		break;
	case APP_EV_STREAM_PATCH_COMPLETE:
		app_stream_set_status(APP_ST_STREAM_READY);
		app_stream_patch_complete();
		break;
	default:
		break;
	}

	if (in)
		FREE(in);
}

void timer_stream_re_operator(void *arg)
{
	stream_oper_t *p_oper = (stream_oper_t *)BtListMemberNew(sizeof(stream_oper_t));

	hlp_audio_stop(s_stream_data->cur_stream_type);
	bse_log_api(TAG, "[dsp][fsm] timer_stream_re_operator, insert queue %d %d\r\n", p_oper->stream_type, p_oper->ctl_type);
	if (p_oper != NULL) {
		memcpy(p_oper, s_stream_data->cur_oper, sizeof(stream_oper_t));
		NewListAddHead(s_stream_data->oper_queue, p_oper);
	}
	FREE(s_stream_data->cur_oper);
}
UINT8 app_stream_get_type(void)
{
	UINT8 nType = hlp_stream_get_type();
	return nType;
}
void app_stream_set_type(UINT8 stream_type)
{
	bse_log_api(TAG, "[dsp][fsm]app_stream_set_type(%d)\r\n", stream_type);
	s_stream_data->cur_stream_type = stream_type;
	hlp_stream_set_type(stream_type);
}

void app_stream_close_ringtone(void)
{
	if (app_stream_get_type() == STREAM_RINGTONE) {
		app_stream_stop(STREAM_RINGTONE);
		app_stream_close(STREAM_RINGTONE,hlp_findname(STREAM_RINGTONE));
	}
}

void app_stream_mic_mute(unsigned int flag)
{
	hlp_SetChannelMuteMic(flag);
}

void app_stream_fadeout_ringtone(void)
{
	hlp_stream_fadeout_ringtone();
}

#endif
