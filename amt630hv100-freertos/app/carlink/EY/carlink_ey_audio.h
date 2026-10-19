#ifndef __CARLINK_EY_AUDIO_H
#define __CARLINK_EY_AUDIO_H
#include <FreeRTOS.h>
#if defined (BT_STACK_BARROT)
#include "brtbt_interface.h"
#define bt_register_pcm_interface(...) brtbt_register_pcm_interface(__VA_ARGS__)
#else 
#include "fscbt_interface.h"
#include "fsc_bt.h"
#define bt_register_pcm_interface(...) fsc_bt_register_pcm_interface(__VA_ARGS__)
#endif

int carlink_ey_audio_init();
#endif