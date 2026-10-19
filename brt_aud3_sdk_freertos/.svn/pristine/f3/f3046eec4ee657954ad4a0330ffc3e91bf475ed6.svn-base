#include "global.h"
#include "hci_parser.h"
#include "brt_trace.h"
#ifdef CONFIG_BT_CHIP_CSR8350
#include "internal_csr_dsp_8350.h"
#endif
#define HCI_TLPKT_COMMAND 1
#define HCI_TLPKT_ACLDATA 2
#define HCI_TLPKT_SCODATA 3
#define HCI_TLPKT_EVENT   4

#define TAG "hci_parser"
#define LOG(fmt, ...) bse_log_info(TAG, fmt, ## __VA_ARGS__)

enum {
    UART_DATA_STATE_TYPE = 0,
    UART_DATA_STATE_LENGTH,
    UART_DATA_STATE_PDU,
};

typedef struct tag_hci_data_block {
	unsigned long pdu_length;
	unsigned char head_length;
	unsigned char iStatus;	//0-None,1-PktType,2-Len&handle
	unsigned long dwCurrLen;
	unsigned long dwCompletedLen;
	unsigned char head[5];
	unsigned char *buffer;
} hci_data_block_t;

typedef struct tag_hci_parser_impl {
	hci_parser_t base;
	hci_data_block_t *hci_data_block;
	hci_data_callback hci_callback;
	SYSTEM_LOCK lock;
} hci_parser_impl_t;

/*
static INLINE INT32 uart_data_parse_pdu(UINT8 *data, unsigned len);
static INLINE int uart_data_parse_length(UINT8 c);
static INLINE INT32 uart_data_parse_type(UINT8 c);
*/

static INLINE INT32 uart_data_parse_type(hci_parser_impl_t *obj, UINT8 c)
{
	hci_data_block_t *hci_data_block = obj->hci_data_block;

	switch (c) {
	case HCI_TLPKT_COMMAND:
		hci_data_block->head_length = 3;
		break;

	case HCI_TLPKT_ACLDATA:
		hci_data_block->head_length = 4;
		break;

	case HCI_TLPKT_SCODATA:
		hci_data_block->head_length = 3;
		break;

	case HCI_TLPKT_EVENT:
		hci_data_block->head_length = 2;
		break;

	default:
		LOG("We got unknown first byte, discard it :%x", c);
		hci_data_block->head_length = 0;
		hci_data_block->dwCurrLen --; //»ØÍËÒ»¸ö×Ö½Ú
		return 1;
	}

	hci_data_block->iStatus = UART_DATA_STATE_LENGTH;
	hci_data_block->dwCompletedLen = 0;
	hci_data_block->head[0] = c;
	return 1;
}

static INLINE int uart_data_parse_length(hci_parser_impl_t *obj, UINT8 c)
{
	hci_data_block_t *hci_data_block = obj->hci_data_block;
	hci_data_block->dwCompletedLen++;

	if (hci_data_block->dwCompletedLen <= hci_data_block->head_length) {
		hci_data_block->head[hci_data_block->dwCompletedLen] = c;
	}

	if (hci_data_block->dwCompletedLen == hci_data_block->head_length) {
		/*get the length of the total packet*/
		switch (hci_data_block->head[0]) {
		case HCI_TLPKT_COMMAND:
			hci_data_block->pdu_length = (unsigned char)(hci_data_block->head[3]);
			break;

		case HCI_TLPKT_ACLDATA:
			hci_data_block->pdu_length = ((unsigned char)(hci_data_block->head[3]) + ((unsigned char)(hci_data_block->head[4]) << 8));
			break;

		case HCI_TLPKT_SCODATA:
			hci_data_block->pdu_length = (unsigned char)(hci_data_block->head[3]);
			break;

		case HCI_TLPKT_EVENT:
			hci_data_block->pdu_length = (unsigned char)(hci_data_block->head[2]);
			break;

		default:
			LOG("Can not be here");
			hci_data_block->pdu_length = 0;
			break;
		}

		//LOG("The length of the packet, %d\r\n", hci_data_block->pdu_length);
		hci_data_block->iStatus = UART_DATA_STATE_PDU;
		hci_data_block->dwCompletedLen = 0;
		hci_data_block->buffer = (UINT8 *)NEW(hci_data_block->pdu_length + hci_data_block->head_length + 1);
		memcpy(hci_data_block->buffer, hci_data_block->head, hci_data_block->head_length + 1);
	}

	return 1;
}

static INLINE INT32 uart_data_parse_pdu(hci_parser_impl_t *obj, UINT8 *data, unsigned len)
{
	hci_data_block_t *hci_data_block = obj->hci_data_block;
	UINT16 remain_pdu_len = hci_data_block->pdu_length - hci_data_block->dwCompletedLen;
	INT32 complete = remain_pdu_len <= len ? 1 : 0;

	if (complete) {
		memcpy(hci_data_block->buffer + hci_data_block->head_length + 1 + hci_data_block->dwCompletedLen, data, remain_pdu_len);

		if (obj->hci_callback) {
#ifdef CONFIG_BT_CHIP_CSR8350
			if (s_tl_info_stru->head[0] == HCI_TLPKT_ACLDATA) {
				UINT16	nCID = ((UINT16)(s_tl_info_stru->buffer[8])<<8) +(UINT16)(s_tl_info_stru->buffer[7]);
				if ((hlp_a2dp_get_cid() != 0) && (nCID == hlp_a2dp_get_cid())) {
					// usr for 145, to filter A2dp data.
					FREE(hci_data_block->buffer);
					hci_data_block->buffer = NULL;
					hci_data_block->iStatus = UART_DATA_STATE_TYPE;
					hci_data_block->dwCompletedLen = 0;
					hci_data_block->dwCurrLen = 0;
					return remain_pdu_len | 0x80000000;	//mean we found a complete HCI packet
				}
			}
#endif
			obj->hci_callback((unsigned char *)hci_data_block->buffer,
			                  hci_data_block->pdu_length + hci_data_block->head_length + 1);
		}

		FREE(hci_data_block->buffer);
		hci_data_block->buffer = NULL;
		hci_data_block->iStatus = UART_DATA_STATE_TYPE;
		hci_data_block->dwCompletedLen = 0;
		hci_data_block->dwCurrLen = 0;
		return remain_pdu_len | 0x80000000;	//mean we found a complete HCI packet
	} else {
		memcpy(hci_data_block->buffer + hci_data_block->head_length + 1 + hci_data_block->dwCompletedLen, data, len);
		hci_data_block->dwCompletedLen += len;
		return len;
	}
}

static int hci_parser_parse(hci_parser_t *self, UINT8 *data, UINT16 len)
{
	int i = 0;
	int consumed = 0;
	UINT8 total_pdu_num = 0;
	hci_parser_impl_t *obj = (hci_parser_impl_t *) self;

	if (!data || len <= 0) {
		return 0;
	}

	ENTER_CRITICAL(obj->lock);

	while (i < len) {
		switch (obj->hci_data_block->iStatus) {
		case UART_DATA_STATE_TYPE:
			consumed = uart_data_parse_type(obj, data[i]);
			break;

		case UART_DATA_STATE_LENGTH:
			consumed = uart_data_parse_length(obj, data[i]);
			break;

		case UART_DATA_STATE_PDU:
			consumed = uart_data_parse_pdu(obj, data + i, len - i);

			if ((consumed & 0x80000000) == 0x80000000)
				total_pdu_num++;

			consumed &= 0x7FFFFFFF;		//Clear flag
			break;
		}

		i += consumed;
	}

	LEAVE_CRITICAL(obj->lock);
	return total_pdu_num;
}

static void hci_parser_init(hci_parser_t *self, hci_data_callback callback)
{
	hci_parser_impl_t *obj = (hci_parser_impl_t *) self;
	self->done(self);
	ENTER_CRITICAL(obj->lock);
	obj->hci_data_block = NEW(sizeof(hci_data_block_t));
	memset(obj->hci_data_block, 0, sizeof(hci_data_block_t));
	obj->hci_callback = callback;
	LEAVE_CRITICAL(obj->lock);
	LOG("%s", __FUNCTION__);
}

static void hci_parser_done(hci_parser_t *self)
{
	hci_parser_impl_t *obj = (hci_parser_impl_t *) self;
	ENTER_CRITICAL(obj->lock);

	if (obj->hci_data_block != NULL) {
		if (obj->hci_data_block->buffer != NULL) {
			FREE(obj->hci_data_block->buffer);
			obj->hci_data_block->buffer = NULL;
		}

		FREE(obj->hci_data_block);
		obj->hci_data_block = NULL;
	}

	LEAVE_CRITICAL(obj->lock);
	LOG("%s", __FUNCTION__);
}

hci_parser_t *hci_parser_new()
{
	hci_parser_impl_t *obj = (hci_parser_t *) NEW(sizeof(hci_parser_impl_t));
	hci_parser_t *base = (hci_parser_t *) obj;

	if (!obj) {
		return NULL;
	}

	memset(obj, 0, sizeof(hci_parser_impl_t));
	base->init = hci_parser_init;
	base->done = hci_parser_done;
	base->parse = hci_parser_parse;
	obj->hci_data_block = (hci_data_block_t *) NEW(sizeof(hci_data_block_t));
	memset(obj->hci_data_block, 0, sizeof(hci_data_block_t));
	obj->lock = CREATE_CRITICAL(TAG);
	return (hci_parser_t *) obj;
}

void hci_parser_free(hci_parser_t *base)
{
	hci_parser_impl_t *obj = (hci_parser_impl_t *) base;

	if (obj) {
		if (obj->hci_data_block) {
			FREE(obj->hci_data_block);
		}

		FREE_CRITICAL(obj->lock);
		FREE(obj);
	}
}


