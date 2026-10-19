/*---------------------------------------------------------------------------
Module Name:
	hci_gt.c
Abstract:
	This module is HCI Code Generator, for those Command/Events.
Author:
	Wang Yao
---------------------------------------------------------------------------*/

#ifndef G_HCI_GT_H
#define G_HCI_GT_H

//#include <math.h>
#include "hci_ui.h"

/*---------------------------------------------------------------------------*/
/* 				Raw User Spec Structure to HCI Internal DataTable 		     */
/*---------------------------------------------------------------------------*/
struct HCI_DT_CommandStru {
	UINT8 *name;
	UINT16 opcode;
	UINT8 *cmd;
	UINT8 *ret;
	UINT8 *ver;/* Version, supported controller, logical groups by function */
};

struct HCI_DT_OpEvStru {
	UINT8 *op;
	UINT8 *ev;
};

struct HCI_DT_EventStru {
	UINT8 *name;
	UINT8 evcode;
	UINT8 *par;
	UINT8 *ver;/* Version, supported controller, logical groups by function */
};

struct HCI_DT_ParStru {
	UINT8 *name;
	UINT8 type;
	UINT8 *val;
	UINT8 *desc;//for /**/
};

struct HCI_DT_ComponentStru {
	UINT8 *desc;
	UINT8 *name;
};

struct HCI_DT_CbkGenStru {
	UINT8 *name;
};

enum {/* flag in struct HCI_DT_TGAPStru */
	HCI_TGAP_RECOMMANDED,
	HCI_TGAP_REQUIRED
};

struct HCI_DT_TGAPStru {
	UINT8 *name;
	UINT8 *value;/* s, ms, or min */
	UINT8 *unit;/* macro used slot unit */
	UINT8 flag;
	UINT8 *desc;
};

struct HCI_DT_ConvertInfoStru {/* List node, by index seq */
	UINT8 off;/* final offset in data table for this index */
	UINT8 len;/* len of x */
	UINT8 x[20];/* assert max 20 */
};

enum {/* type in struct HCI_DT_ParStru */
	HCI_DTPAR_MASK,
	HCI_DTPAR_MASK64,
	HCI_DTPAR_ENUM,
	HCI_DTPAR_FIX,
	HCI_DTPAR_VAL,
	HCI_DTPAR_STRING,
	HCI_DTPAR_TIME,
};

enum {
	HCI_DT_RETURN = 	0,
	HCI_DT_COMMAND = 	1,
	HCI_DT_EVENT = 		2
};

enum {/* Ret in HCI_DT_Par_IsContinue */
	HCI_DT_START = 		0x01,
	HCI_DT_CONTINUE = 	0x02,
	HCI_DT_END = 		0x04,
	HCI_DT_SINGLE = 	0x05
};

enum {
	HCI_DTNUM_STRU,
	HCI_DTNUM_MACRO,
	HCI_DTNUM_TABLE,//octets
};

enum {/* Ret in HCI_DT_Ver_GetVersion */
	HCI_DTVER_11,
	HCI_DTVER_12,
	HCI_DTVER_21,
	HCI_DTVER_30,
	HCI_DTVER_40,
	HCI_DTVER_ERR,
	HCI_DTVER_SMALL = 0x80 //bitmask set <=, otherwise ==
};

enum {/* Supported controllers */
	HCI_DTVER_BREDR = 	0x01,
	HCI_DTVER_LE = 		0x02,
	HCI_DTVER_AMP = 	0x04,
	HCI_DTVER_ALL = 	(HCI_DTVER_BREDR | HCI_DTVER_LE | HCI_DTVER_AMP)
};

enum {/* detail in HCI_DT_FindCommand_EndEvent */
	HCI_DT_CMDEV_END_EVNAME,
	HCI_DT_CMDEV_END_T,
	HCI_DT_CMDEV_END_STRUNAME
};

extern struct HCI_DT_CommandStru gt_cmd[];
extern struct HCI_DT_OpEvStru gt_opev[];
extern struct HCI_DT_EventStru gt_ev[];
extern struct HCI_DT_EventStru gt_leev[];
extern struct HCI_DT_ParStru gt_par[];
extern struct HCI_DT_CbkGenStru gt_cbk[];
extern struct HCI_DT_TGAPStru gt_tgap[];

extern UINT32 GHCI_GT_CMDRETNUM;
extern UINT32 GHCI_GT_EVNUM;
extern UINT32 GHCI_GT_LEEVNUM;
extern UINT32 GHCI_GT_OPEVNUM;
extern UINT32 GHCI_GT_PARNUM;
extern UINT32 GHCI_GT_CBKNUM;
extern UINT32 GHCI_GT_TGAPNUM;

#define HCI_GT_CMDRETNUM 		GHCI_GT_CMDRETNUM
#define HCI_GT_EVNUM 			GHCI_GT_EVNUM
#define HCI_GT_LEEVNUM 			GHCI_GT_LEEVNUM
#define HCI_GT_OPEVNUM 			GHCI_GT_OPEVNUM
#define HCI_GT_PARNUM 			GHCI_GT_PARNUM
#define HCI_GT_CBKNUM	 		GHCI_GT_CBKNUM
#define HCI_GT_TGAPNUM	 		GHCI_GT_TGAPNUM

/*---------------------------------------------------------------------------*/
/* 							Function shared by DEBUG 					     */
/*---------------------------------------------------------------------------*/
UINT8 HCI_DT_Compare2Octet(UINT8 a, UINT8 b);
UINT8 HCI_DT_Par_CheckString(UINT8 *par, UINT8 *base);
void HCI_DT_Par_Type(UINT8 *dbg_str, struct HCI_DT_ParStru *node);
UINT8 HCI_DT_Par_GetMaskBitIndex(UINT32 mask);
UINT8 HCI_DT_Par_IsContinue(struct HCI_DT_ParStru *node);
UINT8 HCI_DT_Par_Get1Num(UINT8 s);
UINT32 HCI_DT_Par_GetNum(UINT8 *s, UINT8 *e);
UINT8 HCI_DT_GetEleType(UINT8 *e);
void HCI_DT_ViewCodingBP(struct HCI_BitPointerStru *bp);
void HCI_DT_Par_GetName(UINT8 *dbg_str, struct HCI_DT_ParStru *node);
UINT32 HCI_DT_Par_GetParLen(struct HCI_DT_ParStru *node);
double HCI_DT_Par_GetTimeSlot(struct HCI_DT_ParStru *node);
double HCI_DT_Par_GetFloat(UINT8 *s, UINT8 *e);
UINT8 HCI_DT_Par_CheckStringCore(UINT8 *p, UINT8 *base, UINT8 longer);

#endif/* #ifndef G_HCI_GT_H*/

