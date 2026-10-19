#include "porting.h" /* just for include autoconf_bt.h */
#include "upio.h"
#include "porting.h"
#include "porting_utils.h"
#ifdef CONFIG_OS_ANDROID
#include <utils/Log.h>
#include <cutils/properties.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "../../../sdk/auto_app/brt_trace.h"
#define LOG(fmt, ...) bse_log_info("upio", fmt, ## __VA_ARGS__)
#include "brlinkd.h"

#if defined(CONFIG_PM_USING_RFKILL)
#include "rfkill.h"
#elif defined(CONFIG_PM_USING_GPIO)
#include "gpio.h"
#endif

#include "board.h"
#include "chip.h"
/******************************************************************************
**  Constants & Macros
******************************************************************************/


#ifndef UPIO_DBG
#define UPIO_DBG TRUE
#endif

#if (UPIO_DBG == TRUE)
#define UPIODBG(param, ...) {ALOGD(param, ## __VA_ARGS__);}
#else
#define UPIODBG(param, ...) {}
#endif
#define TL_CONFIG_FILE_PATH             compose_file_path(CONFIG_FILE_ROOT_DIR, TL_CONFIG_FILE)

#ifdef CONFIG_PM_USING_GPIO
static int port = -1;
#endif
/*******************************************************************************
**
** Function        upio_init
**
** Description     Initialization
**
** Returns         None
**
*******************************************************************************/
void upio_init(void)
{
#if defined(CONFIG_PM_USING_RFKILL)
	bt_rfkill_open();
#elif defined(CONFIG_PM_USING_GPIO)
	port = get_private_profile_int("local", "gpio", 0, TL_CONFIG_FILE_PATH);
	//LOG("%s: gpio is :%d",  port);
	if (port >= 0) {
		gpio_open(port, 1);
	}
#endif
}

/*******************************************************************************
**
** Function        upio_cleanup
**
** Description     Clean up
**
** Returns         None
**
*******************************************************************************/
void upio_cleanup(void)
{
#if defined(CONFIG_PM_USING_RFKILL)
	bt_rfkill_close();
#elif defined(CONFIG_PM_USING_GPIO)
	if (port >= 0) {
		gpio_close(port);
	}
#endif
}

/*******************************************************************************
**
** Function        upio_set_bluetooth_power
**
** Description     Interact with low layer driver to set Bluetooth power
**                 on/off.
**
** Returns         0  : SUCCESS or Not-Applicable
**                 <0 : ERROR
**
*******************************************************************************/
int upio_set_bluetooth_power(int on)
{
	int ret = -1;
	//LOG("%s: power is :%d", on);
#if defined(CONFIG_PM_USING_RFKILL)
	ret = bt_rfkill_write(on);
#elif defined(CONFIG_PM_USING_GPIO)
	if (port >= 0) {
		ret = gpio_write(port, on);
	}
#else
	printf("upio_set_bluetooth_power %d\n", on);
	gpio_direction_output(hw_get_bt_en_pin(), on);
#endif
	return ret;
}

