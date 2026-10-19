#include "brt_trace.h"
#include "parse_a2dp_data.h"
#include "a2dp_ui.h"

#define TAG "bse_a2dp"

static int s_inv_audio_packet_num = 0;
static int s_empty_packet_num = 0;
static int s_check_count = 0;
static int s_a2dp_state = STATE_NONE;

void clear_check()
{
	s_inv_audio_packet_num = 0;
	s_empty_packet_num = 0;
	s_check_count = 0;
	s_a2dp_state = STATE_NONE;
}

int memcount(const void * pMemBuf, int Buflen, int Schar)
{
	unsigned char *pMemTmp = pMemBuf;
	int len = Buflen;
	int total = 0;

	while (len > 0) {
		if ((*(unsigned char*)pMemTmp) == (unsigned char)Schar) {
			total++;
		}

		pMemTmp += 1;
		len--;
	}

	return total;
}

int check_empty_data(const void * param)
{
	struct BuffStru *buffer = (struct BuffStru *) param;
	int len = DATASIZE(buffer) - 1;
	int inv_count = memcount((BUFDATA(buffer) + 1), len, 0);// calculate the empty data
	int criteria = len * 0.9;

	if(inv_count >= criteria) { //Audio data is invalid
		s_inv_audio_packet_num++;
		if(s_inv_audio_packet_num >= CHECK_NUM && s_a2dp_state != STATE_PAUSED) {
			s_a2dp_state = STATE_PAUSED;
			return STATE_PAUSED;
		}
	} else if(s_a2dp_state != STATE_PLAYING) {
		s_a2dp_state = STATE_PLAYING;
		s_inv_audio_packet_num = 0;
		return STATE_PLAYING;
	}

	return STATE_NONE;
}

void set_a2dp_state(const int state)
{
	s_a2dp_state = state;
}




