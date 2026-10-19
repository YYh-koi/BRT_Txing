#include "awtk.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "base/timer.h"
#include "widgets/label.h"
#include "widgets/view.h"

#include "board.h"
#include "vb7018_voice.h"
#include "vb7018_voice_awtk.h"

#ifdef VB7018_VOICE_DEMO_ENABLE

#define VB7018_VOICE_AWTK_EVENT_QUEUE_SIZE	8

typedef struct {
	const char *text;
	unsigned int count;
} vb7018_voice_awtk_event_msg_t;

static widget_t *s_vb7018_voice_window;
static widget_t *s_vb7018_voice_label;
static volatile unsigned int s_vb7018_voice_event_count;
static QueueHandle_t s_vb7018_voice_event_queue;
static int s_vb7018_voice_started;
static int s_vb7018_voice_timer_started;

static const char *vb7018_voice_awtk_event_to_text(vb7018_voice_event_t event)
{
	switch (event) {
	case VB7018_VOICE_EVENT_WAKEUP:
		return "VB7018: WAKEUP";
	case VB7018_VOICE_EVENT_BYE:
		return "VB7018: BYE";
	case VB7018_VOICE_EVENT_OPEN_ASSISTANT:
		return "VB7018: OPEN_ASSISTANT";
	case VB7018_VOICE_EVENT_VOLUME_UP:
		return "VB7018: VOLUME_UP";
	case VB7018_VOICE_EVENT_VOLUME_DOWN:
		return "VB7018: VOLUME_DOWN";
	case VB7018_VOICE_EVENT_PREV_TRACK:
		return "VB7018: PREV_TRACK";
	case VB7018_VOICE_EVENT_NEXT_TRACK:
		return "VB7018: NEXT_TRACK";
	case VB7018_VOICE_EVENT_PLAY_MUSIC:
		return "VB7018: PLAY_MUSIC";
	case VB7018_VOICE_EVENT_PAUSE_MUSIC:
		return "VB7018: PAUSE_MUSIC";
	case VB7018_VOICE_EVENT_TCS_OFF:
		return "VB7018: TCS_OFF";
	case VB7018_VOICE_EVENT_TCS_ON:
		return "VB7018: TCS_ON";
	case VB7018_VOICE_EVENT_HDC_OFF:
		return "VB7018: HDC_OFF";
	case VB7018_VOICE_EVENT_HDC_ON:
		return "VB7018: HDC_ON";
	case VB7018_VOICE_EVENT_ANTI_ROLLBACK_OFF:
		return "VB7018: ANTI_ROLLBACK_OFF";
	case VB7018_VOICE_EVENT_ANTI_ROLLBACK_ON:
		return "VB7018: ANTI_ROLLBACK_ON";
	case VB7018_VOICE_EVENT_VEHICLE_SELF_CHECK:
		return "VB7018: VEHICLE_SELF_CHECK";
	case VB7018_VOICE_EVENT_REVERSE_ON:
		return "VB7018: REVERSE_ON";
	case VB7018_VOICE_EVENT_PUSH_MODE_ON:
		return "VB7018: PUSH_MODE_ON";
	case VB7018_VOICE_EVENT_REVERSE_OR_PUSH_OFF:
		return "VB7018: REVERSE_OR_PUSH_OFF";
	case VB7018_VOICE_EVENT_HEADLIGHT_ON:
		return "VB7018: HEADLIGHT_ON";
	case VB7018_VOICE_EVENT_HEADLIGHT_OFF:
		return "VB7018: HEADLIGHT_OFF";
	case VB7018_VOICE_EVENT_VEHICLE_ARM:
		return "VB7018: VEHICLE_ARM";
	case VB7018_VOICE_EVENT_VEHICLE_DISARM:
		return "VB7018: VEHICLE_DISARM";
	case VB7018_VOICE_EVENT_VEHICLE_POWER_ON:
		return "VB7018: VEHICLE_POWER_ON";
	case VB7018_VOICE_EVENT_VEHICLE_POWER_OFF:
		return "VB7018: VEHICLE_POWER_OFF";
	case VB7018_VOICE_EVENT_SEAT_BUCKET_OPEN:
		return "VB7018: SEAT_BUCKET_OPEN";
	default:
		return "VB7018: UNKNOWN";
	}
}

static void vb7018_voice_awtk_callback(vb7018_voice_event_t event)
{
	vb7018_voice_awtk_event_msg_t msg;
	vb7018_voice_awtk_event_msg_t drop_msg;

	s_vb7018_voice_event_count++;
	msg.text = vb7018_voice_awtk_event_to_text(event);
	msg.count = s_vb7018_voice_event_count;

	if (!s_vb7018_voice_event_queue)
		return;

	if (xQueueSend(s_vb7018_voice_event_queue, &msg, 0) != pdPASS) {
		printf("[VB7018_AWTK] queue full, drop oldest before enqueue #%u %s\r\n",
			msg.count, msg.text);
		xQueueReceive(s_vb7018_voice_event_queue, &drop_msg, 0);
		xQueueSend(s_vb7018_voice_event_queue, &msg, 0);
	}

	printf("[VB7018_AWTK] callback enqueue #%u %s waiting=%u\r\n",
		msg.count, msg.text, (unsigned int)uxQueueMessagesWaiting(s_vb7018_voice_event_queue));
}

static ret_t vb7018_voice_awtk_timer(const timer_info_t *info)
{
	vb7018_voice_awtk_event_msg_t msg;
	char show_text[64];

	(void)info;

	if (s_vb7018_voice_label && s_vb7018_voice_event_queue &&
		xQueueReceive(s_vb7018_voice_event_queue, &msg, 0) == pdPASS) {
		if (msg.text) {
			tk_snprintf(show_text, sizeof(show_text), "#%u %s", msg.count, msg.text);
			widget_set_text_utf8(s_vb7018_voice_label, show_text);
			printf("[VB7018_AWTK] ui update %s waiting=%u\r\n",
				show_text, (unsigned int)uxQueueMessagesWaiting(s_vb7018_voice_event_queue));
		}
	}

	return RET_REPEAT;
}

void vb7018_voice_awtk_init(widget_t *parent)
{
	if (!parent)
		return;

	if (!s_vb7018_voice_window) {
		s_vb7018_voice_window = view_create(parent, 10, 10, 560, 86);
		if (s_vb7018_voice_window) {
			widget_set_style_color(s_vb7018_voice_window, "normal:bg_color", 0xff000000);
			widget_set_style_color(s_vb7018_voice_window, "normal:border_color", 0xffffffff);
			widget_set_style_str(s_vb7018_voice_window, "normal:border", "all");
		}
	}

	if (!s_vb7018_voice_label) {
		s_vb7018_voice_label = label_create(s_vb7018_voice_window ? s_vb7018_voice_window : parent,
			12, 8, 536, 70);
		if (s_vb7018_voice_label) {
			widget_set_style_int(s_vb7018_voice_label, "normal:font_size", 36);
			widget_set_style_color(s_vb7018_voice_label, "normal:text_color", 0xff0000ff);
			widget_set_style_str(s_vb7018_voice_label, "normal:text_align_h", "center");
			widget_set_style_str(s_vb7018_voice_label, "normal:text_align_v", "middle");
			widget_set_text_utf8(s_vb7018_voice_label, "VB7018: waiting");
		}
	}

	if (!s_vb7018_voice_event_queue)
		s_vb7018_voice_event_queue = xQueueCreate(VB7018_VOICE_AWTK_EVENT_QUEUE_SIZE,
			sizeof(vb7018_voice_awtk_event_msg_t));
	printf("[VB7018_AWTK] init parent=0x%08x label=0x%08x queue=0x%08x\r\n",
		(unsigned int)parent, (unsigned int)s_vb7018_voice_label,
		(unsigned int)s_vb7018_voice_event_queue);

	if (!s_vb7018_voice_started) {
		vb7018_voice_register_callback(vb7018_voice_awtk_callback);
		if (vb7018_voice_start() != 0) {
			vb7018_voice_awtk_event_msg_t msg;

			s_vb7018_voice_event_count++;
			msg.text = "VB7018: start failed";
			msg.count = s_vb7018_voice_event_count;
			if (s_vb7018_voice_event_queue)
				xQueueSend(s_vb7018_voice_event_queue, &msg, 0);
			printf("[VB7018_AWTK] voice start failed\r\n");
		} else {
			printf("[VB7018_AWTK] voice start ok\r\n");
		}
		s_vb7018_voice_started = 1;
	}

	if (!s_vb7018_voice_timer_started) {
		timer_add(vb7018_voice_awtk_timer, NULL, 100);
		s_vb7018_voice_timer_started = 1;
		printf("[VB7018_AWTK] timer start\r\n");
	}
}

#endif
