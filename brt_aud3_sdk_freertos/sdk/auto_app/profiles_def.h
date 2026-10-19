#ifndef _PROFILE_DEF_H__
#define _PROFILE_DEF_H__

#include "internal_manager.h"        /* 0 dummy profile*/
#include "auto_gap.h"            /* 1 dummy profile */
#ifdef CONFIG_HFP
#include "auto_hf.h"             /* 2 */
#endif
#ifdef CONFIG_A2DP
#include "auto_a2dp.h"           /* 3 */
#endif
#ifdef CONFIG_AVRCP
#include "auto_avrcp.h"          /* 4 */
#endif
#ifdef CONFIG_PBAP
#include "auto_pbap.h"           /* 5 */
#endif
#ifdef CONFIG_HID
#include "auto_hid.h"            /* 6 */
#endif
#ifdef CONFIG_CBP
#include "auto_cb_screen.h"      /* 7 */
#endif
#ifdef CONFIG_SPP
#include "app_rfcomm.h"	        /* 8 */
#endif
#ifdef CONFIG_FTP
#include "auto_ftp.h"	        /* 9 */
#endif
#ifdef CONFIG_DUN
#include "auto_dun.h"	        /* 10*/
#endif
#ifdef CONFIG_GATT
#include "auto_gatt.h"           /* 11*/
#endif
#ifdef CONFIG_OPP
#include "auto_opp.h"	        /* 12*/
#endif
#ifdef CONFIG_MAP
#include "auto_map.h"	        /* 13*/
#endif
#ifdef CONFIG_PAN
#include "auto_pan.h"	        /* 14*/
#endif
#ifdef CONFIG_ANCS_CLIENT
#include "auto_ancs.h"           /* 16*/
#endif

#endif  /*_PROFILE_DEF_H__*/

