#ifndef _HLP_BMESSAGE_H__
#define _HLP_BMESSAGE_H__

#include "auto_map.h"

int bmessage_compose(app_map_sms_item_t *msg, UINT8 **out_bmsg, UINT16 *out_len);
int bmessage_decompose(UINT8 * data, UINT16 len, app_map_sms_item_t *msg);

#endif /* _HLP_BMESSAGE_H__ */
