#include <stdio.h>

#include "vb7018_voice.h"
#include "vb7018_voice_demo.h"

static const char *vb7018_voice_demo_event_name(vb7018_voice_event_t event)
{
	switch (event) {
	case VB7018_VOICE_EVENT_WAKEUP:
		return "WAKEUP";
	case VB7018_VOICE_EVENT_BYE:
		return "BYE";
	case VB7018_VOICE_EVENT_OPEN_ASSISTANT:
		return "OPEN_ASSISTANT";
	case VB7018_VOICE_EVENT_VOLUME_UP:
		return "VOLUME_UP";
	case VB7018_VOICE_EVENT_VOLUME_DOWN:
		return "VOLUME_DOWN";
	case VB7018_VOICE_EVENT_PREV_TRACK:
		return "PREV_TRACK";
	case VB7018_VOICE_EVENT_NEXT_TRACK:
		return "NEXT_TRACK";
	case VB7018_VOICE_EVENT_PLAY_MUSIC:
		return "PLAY_MUSIC";
	case VB7018_VOICE_EVENT_PAUSE_MUSIC:
		return "PAUSE_MUSIC";
	case VB7018_VOICE_EVENT_TCS_OFF:
		return "TCS_OFF";
	case VB7018_VOICE_EVENT_TCS_ON:
		return "TCS_ON";
	case VB7018_VOICE_EVENT_HDC_OFF:
		return "HDC_OFF";
	case VB7018_VOICE_EVENT_HDC_ON:
		return "HDC_ON";
	case VB7018_VOICE_EVENT_ANTI_ROLLBACK_OFF:
		return "ANTI_ROLLBACK_OFF";
	case VB7018_VOICE_EVENT_ANTI_ROLLBACK_ON:
		return "ANTI_ROLLBACK_ON";
	case VB7018_VOICE_EVENT_VEHICLE_SELF_CHECK:
		return "VEHICLE_SELF_CHECK";
	case VB7018_VOICE_EVENT_REVERSE_ON:
		return "REVERSE_ON";
	case VB7018_VOICE_EVENT_PUSH_MODE_ON:
		return "PUSH_MODE_ON";
	case VB7018_VOICE_EVENT_REVERSE_OR_PUSH_OFF:
		return "REVERSE_OR_PUSH_OFF";
	case VB7018_VOICE_EVENT_HEADLIGHT_ON:
		return "HEADLIGHT_ON";
	case VB7018_VOICE_EVENT_HEADLIGHT_OFF:
		return "HEADLIGHT_OFF";
	case VB7018_VOICE_EVENT_VEHICLE_ARM:
		return "VEHICLE_ARM";
	case VB7018_VOICE_EVENT_VEHICLE_DISARM:
		return "VEHICLE_DISARM";
	case VB7018_VOICE_EVENT_VEHICLE_POWER_ON:
		return "VEHICLE_POWER_ON";
	case VB7018_VOICE_EVENT_VEHICLE_POWER_OFF:
		return "VEHICLE_POWER_OFF";
	case VB7018_VOICE_EVENT_SEAT_BUCKET_OPEN:
		return "SEAT_BUCKET_OPEN";
	default:
		return "UNKNOWN";
	}
}

static void vb7018_voice_demo_callback(vb7018_voice_event_t event)
{
	printf("vb7018 voice event:%s\n", vb7018_voice_demo_event_name(event));
}

int vb7018_voice_demo_start(void)
{
	int ret;

	vb7018_voice_register_callback(vb7018_voice_demo_callback);
	ret = vb7018_voice_start();
	if (ret != 0)
		printf("start vb7018 voice demo fail.\n");

	return ret;
}
