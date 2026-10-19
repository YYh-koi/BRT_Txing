#ifndef _APP_TL_H_
#define _APP_TL_H_

#include "hci_ui.h"
#include "transport.h"

#define TL_LOCAL_HANDLE_DEFAULT	(HANDLE)(0x00001101)

void app_tl_init(void);
void app_tl_done(void);
void hlp_gap_tl_init(void); // used in hlp_gap.c
void hlp_gap_tl_start(void);
void hlp_gap_tl_stop(void);
void hlp_gap_tl_done(void);
// used in ./app_commfunc.c to send Vendor data. Is it a mistake?
int hlp_tl_send_data(HANDLE tl_hdl, unsigned char *data, unsigned long len);
void TLHandle();

#endif /* _APP_TL_H_ */

