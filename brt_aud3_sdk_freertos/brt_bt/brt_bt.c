#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "board.h"
#include "brtbt_interface.h"

static bt_sw_cfg_t g_bt_cfg;
static TaskHandle_t brt_bt_task_handle = NULL;

void brtbt_register_pcm_interface(void *itfc)
{
	memcpy((void*)&g_bt_cfg, itfc, sizeof(bt_sw_cfg_t));
}

static int brt_bt_play_state_callback(BT_PLAY_STATE_E state, unsigned short samplerate, unsigned char channel)
{
    printf("brt_bt_play_state_callback state %d samplerate %d channel %d\r\n", state, samplerate, channel);
    return 0;
}

static int brt_bt_a2dp_pcm_data_callback(unsigned char* buffer, unsigned short length)
{
    //printf("a2dp length %d\r\n", length);
    return 0;
}

static int brt_bt_hfp_spk_pcm_data_callback(unsigned char* buffer, unsigned short length)
{
    //printf("spk length %d\r\n", length);
    return 0;
}

static int brt_bt_bip_coverart_data_callback(unsigned char *image_data, unsigned int image_size, const char *mime)
{
    printf("coverart size %d, mime:%s\r\n", image_size, mime);
    return 0;
}

static void brt_bt_task(void* arg)
{

   bt_hw_cfg_t bt_hw_cfg = {BT_RESET_IO, BT_UART_PORT};
   bt_sw_cfg_t bt_sw_cfg;

   bt_hw_cfg.bt_en_pin = BT_RESET_IO;
   bt_hw_cfg.uartport = BT_UART_PORT;
   bt_hw_cfg.bt_module = BRT_MODULE_CHIP_BRT;

   bt_sw_cfg.a2dp_resampler = 0;
   bt_sw_cfg.debug_mode = 1; // debug toggle
#if CARLINK_CLCAST
   bt_sw_cfg.ble_connection_type = BLE_CLCAST_CONNECTION;
#elif CARLINK_EC
   bt_sw_cfg.ble_connection_type = BLE_EASY_CONNECTION;
#else
   bt_sw_cfg.ble_connection_type = BLE_ERYA_CONNECTION;
#endif
    bt_sw_cfg.schedule_loop_priority = 20;
    bt_sw_cfg.avrcp_bip_power = 1;

   if (g_bt_cfg.play_state_cb)
   	bt_sw_cfg.play_state_cb = g_bt_cfg.play_state_cb;
   else
   	bt_sw_cfg.play_state_cb = brt_bt_play_state_callback;
   
   if (g_bt_cfg.a2dp_cb)
   	bt_sw_cfg.a2dp_cb = g_bt_cfg.a2dp_cb;
   else
   	bt_sw_cfg.a2dp_cb = brt_bt_a2dp_pcm_data_callback;
   
   if (g_bt_cfg.hfp_spk_cb)
   	bt_sw_cfg.hfp_spk_cb = g_bt_cfg.hfp_spk_cb;
   else
   	bt_sw_cfg.hfp_spk_cb = brt_bt_hfp_spk_pcm_data_callback;

   if (g_bt_cfg.coverart_cb)
   	bt_sw_cfg.coverart_cb = g_bt_cfg.coverart_cb;
   else
   	bt_sw_cfg.coverart_cb = brt_bt_bip_coverart_data_callback;

	brlinkd_stack_init(&bt_hw_cfg, &bt_sw_cfg);
	brlinkd_stack_run();
}

int brt_bt_main(void)
{
	if (brt_bt_task_handle)
		return 0;
	/* Create a task to process uart rx data */
	if (xTaskCreate(brt_bt_task, "brt_bt_main", 4096 * 2, NULL,
		configMAX_PRIORITIES / 3, &brt_bt_task_handle) != pdPASS) {
		printf("create brt_bt_thread task fail.\n");
		return -1;
	}

	return 0;
}

