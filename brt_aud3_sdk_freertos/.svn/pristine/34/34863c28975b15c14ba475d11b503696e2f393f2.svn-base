/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    bterrno.h
Abstract:
	This file includes the definition of global error numbers.
Author:
    He Gang
    Wang Yao
Revision History:
    1.  2000.02.10  Created
	2.  2011.05.15  Redefined.
---------------------------------------------------------------------------*/
#ifndef BTERRNO_H
#define BTERRNO_H

enum {
    /* 0x00 is reserved */
    /* 0x01 is used by HCI internal error, e.g. HCI_STATUS_NO_FLOWCONTROL = 0x0100 */
	ER_TYPE_HCI = 0x02, /* standard HCI error */
	ER_TYPE_HCI_PRV = 0x03, /* ((ER_TYPE_HCI << 8) + HCI internal error) >> 8 */
	ER_TYPE_L2CAP = 0x04,
	ER_TYPE_RFCOMM = 0x05,
	ER_TYPE_OBEX = 0x06
};

#define ER_SET(t, v)							((UINT16)((v) ? (((t) << 8) | (v)) : 0))/* only when value != 0, then set type */
#define ER_GETTYPE(s)							((UINT8)((s) >> 8))
#define ER_GETVALUE(s)							((UINT8)((s) & 0xFF))
#define ER_SETCBK(v, t)							(((v) & 0xFFFFFF00) ? (((v) & 0xFFFFFF00) >> 8) : ER_SET((t), (v)))
#define ER_L2CAP_SETCBK(v)						(((v) & 0xFFFFFF00) ? ER_SET((ER_TYPE_HCI), (((v) & 0xFFFFFF00) >> 8)) : ER_SET((ER_TYPE_L2CAP), (v)))

#endif
