/*************************************************************************
    > File Name: bcsp_pskey.h
    > Author: hly
    >
    > Created Time: 2014年08月21日 星期四 16时11分28秒
 ************************************************************************/

#ifndef CSR_PSKEY_H__
#define CSR_PSKEY_H__

#include "global.h"
#include "gap_ui.h"
#include "g_bcsp.h"
#include "bcspui.h"
#include "bcsptl.h"
#include <stdio.h>
#include <stdlib.h>

#define BT_PSKEY_CHIP_TYPE_DEFAULT			 0x00
#define BT_PSKEY_CHIP_TYPE_I63_COMMON        0x01
#define BT_PSKEY_CHIP_TYPE_I63_NWD           0x02
#define BT_PSKEY_CHIP_TYPE_CSR_COMMOM        0x03
#define BT_PSKEY_CHIP_TYPE_CSR_JTD           0x04
#define BT_PSKEY_CHIP_TYPE_CSR_8311          0x05
#define BT_PSKEY_CHIP_TYPE_RISINGTEK_8311    0x06
#define BT_PSKEY_CHIP_TYPE_CSP_PATEO         0x07
#define BT_PSKEY_CHIP_TYPE_ROSEN_8350        0x08
#define BT_PSKEY_CHIP_TYPE_CSR_8811          0x09
#define BT_PSKEY_CHIP_TYPE_NUM               0x0A

/*It is used for different customization to set different pskey*/
/* CONFIG_PROJECT_XXX is defined in autoconf_bt.h */

void bt_pskey_set(WORD key, WORD value_num, WORD *values);
int bt_pskey_set_chip(int chip_type);
void set_psk();
void psk_env_done(void);

#endif //CSR_PSKEY_H__
