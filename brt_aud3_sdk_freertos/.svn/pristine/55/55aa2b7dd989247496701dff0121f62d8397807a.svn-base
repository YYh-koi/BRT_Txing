/*---------------------------------------------------------------------------
Module Name:
	hci_le.h
Abstract:
	This module defines LE GAP structure, macro & functions.
Author:
	Wang Yao
---------------------------------------------------------------------------*/

#ifndef G_HCI_LE_H
#define G_HCI_LE_H

#include "global.h"
#include "hci_hlp.h"
#include "att_hlp.h"

/*---------------------------------------------------------------------------*/
/* 								LE GAP GATT Service						     */
/*---------------------------------------------------------------------------*/

#define	LE_DEVICENAME  	"BRT BLE"

/*-----------------------------------------------------------------------------
Description:
	
-----------------------------------------------------------------------------*/
enum {/* mode in SMP_SetSecurityMode */
	/* A connection operating in LE  security mode 1 level 2 shall also 
	satisfy the security requirements for LE security mode 1 level 1.
	A connection operating in LE  security mode 1 level 3 shall also 
	satisfy the security requirements for LE security mode 1 level 2 or 
	LE security mode 1 level 1.
	A connection operating in LE security mode 1 level 3 shall also 
	satisfy the security requirements fo r LE security mode 2. */
	
	LE_GAP_SECURITY_MODE1_LEVEL1,/* No security */
	LE_GAP_SECURITY_MODE1_LEVEL2,/* Unauthenticated pairing and encryption */
	LE_GAP_SECURITY_MODE1_LEVEL3,/* Authenticated Pairing and encryption */
	LE_GAP_SECURITY_MODE2_LEVEL1,/* Unauthenticated pairing and data signing */
	LE_GAP_SECURITY_MODE2_LEVEL2/* Authenticated pairing and data signing */
};

#if 0/* Repleaced by search using UUID */
/*-----------------------------------------------------------------------------
Description:
[0x0001->0x000b][0x2800,GATT_Primary_Service_Declaration]       [R]          [Len:2][0x1800,Generic_Access]
[0x0002]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][R,W][VHDL:0x0003][0x2a00,Device_Name]
[0x0003]        **[0x2a00,Device_Name]                          [R,W]        [Len:22][Len:22][Str:BRT-LE-USER-475OI674RP]
[0x0004]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][R][VHDL:0x0005][0x2a01,Appearance]
[0x0005]        **[0x2a01,Appearance]                           [R]          [Len:2][Appearance:0x0080]
[0x0006]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][R,W,WWR,ASW][VHDL:0x0007][0x2a02,Peripheral_Privacy_Flag]
[0x0007]        **[0x2a02,Peripheral_Privacy_Flag]              [R,W,WWR,ASW][Len:1][Privacy:Disable]
[0x0008]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][R][VHDL:0x0009][0x2a03,Reconnection_Address]
[0x0009]        **[0x2a03,Reconnection_Address]                 [R]          [Len:6][ReADDR:00,00,00,00,00,00]
[0x000a]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][R][VHDL:0x000b][0x2a04,Peripheral_Preferred_Connection_Parameters]
[0x000b]        **[0x2a04,Peripheral_Preferred_Connection_Parameters][R]     [Len:8][ConnInterval:7~7][SlaveLatency:0][SupervisionTO:100]
[0x000c->0x000f][0x2800,GATT_Primary_Service_Declaration]       [R]          [Len:2][0x1801,Generic_Attribute]
[0x000d]          [0x2803,GATT_Characteristic_Declaration]      [R]          [Len:5][NTI][VHDL:0x000e][0x2a05,Service_Changed]
[0x000e]        **[0x2a05,Service_Changed]                      [NTI]        [Len:4][HDL:0x0000->0x0000]
[0x000f]          ->[0x2902,Client_Characteristic_Configuration][R,W,AU]     [Len:2][CCC:NOTIFICATION]
-----------------------------------------------------------------------------*/
enum {
	/* GAP Service */
	LE_GAP_HDL_GAP_SERVICE_START = 							0x0001,
	LE_GAP_HDL_GAP_SERVICE_END = 							0x000b,
	LE_GAP_HDL_DEVICENAME = 								0x0003,/* All value handles */
	LE_GAP_HDL_APPEARANCE = 								0x0005,
	LE_GAP_HDL_PERIPHERAL_PRIVACY_FLAG = 					0x0007,
	LE_GAP_HDL_RECONNECTION_ADDRESS = 						0x0009,
	LE_GAP_HDL_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS = 0x000b,

	/* GATT Service, Service Change Characteristic */
	LE_GAP_HDL_GATT_SERVICE_START = 						0x000c,
	LE_GAP_HDL_GATT_SERVICE_END = 							0x000f,
	LE_GAP_HDL_SERVICE_CHANGE = 							0x000e,/* Characteristic Value handle */
	LE_GAP_HDL_SERVICE_CHANGE_CCC = 						0x000f
};
#endif

struct LE_GAP_Peripheral_ConnectionParStru {/* Table 12.7:  Format of the preferred connection parameters structured data */
	UINT8 min_connection_interval[2];/* connInterval min = Conn_Interval_Min * 1.25 ms range: 0x0006 to 0x0C80, Value of 0xFFFF indicates no specific minimum. */
	UINT8 max_connection_interval[2];/* connInterval max = Conn_Interval_Max * 1.25 ms range: 0x0006 to 0x0C80, Shall be equal to or greater than the Conn_Interval_Min, Value of 0xFFFF indicates no specific maximum. */
	UINT8 slave_latency[2];/* range: 0x0000 to 0x03E8 */
	UINT8 connection_supervision_timeout_multiplier[2];/* Defines the connection supervisor timeout multiplier as a multiple of 10ms. Range: 0xFFFF indicates no specific value requested. Range: 0x000A to 0x0C80, Time = N * 10 msec, Time Range: 100 msec to 32 seconds */
};

UINT8 LE_Privacy_GetLocal(void);
UINT8 *LE_Addr_Reconnection_GetByPeripherial(void);
UINT8 *LE_Addr_Reconnection_GetByCentral(void);

void LE_Privacy_SetByPeripherial(struct ATT_ItemStru *item, UINT8 privacy);
void LE_Privacy_SetByBond(void);
void LE_Set_Advertising_Parameters(UINT8 type, UINT8 limited, UINT8 speed, UINT8 white);
void LE_Set_Scan_Parmeters(UINT8 type, UINT8 limited, UINT8 speed);
void LE_Set_Scan_Enable(UINT8 enable);
void LE_GAP_ServiceReg(void *cbk);

void HCI_OP_LE_Set_Scan_Advertise_Enable(struct HCI_LocalStru *local, struct HCI_LE_Set_Scan_EnableCompStru *ev);

/*---------------------------------------------------------------------------*/
/* 							LE GAP SMP Parts							     */
/*---------------------------------------------------------------------------*/
void LE_Addr_Resolvable_Gen(UINT8 *p);
void LE_Addr_Reconnection_Gen(struct HCI_AddressStru *addr/* Public or Static */, UINT8 *out);
void LE_Addr_Reconnection_PrivacyChangeByCentral(struct HCI_AddressStru *addr/* Public or Static */, UINT8 isoff);
void LE_Addr_Remote2PublicStatic(struct HCI_AddressStru *addr);
struct SMP_PeerStru *LE_Addr_Resolvable_Convert(struct HCI_AddressStru *addr);
UINT8 SMP_CompareAddr(struct HCI_AddressStru *s, struct HCI_AddressStru *r);
UINT8 SMP_BondCount(void);
void SMP_ERIR(struct HCI_AddressStru *addr/* Local */);

void FSM_SetRandom(UINT32 s);
UINT32 FSM_GetRandom(UINT32 s);

/*---------------------------------------------------------------------------*/
/* 							LE GAP HCI Parts							     */
/*---------------------------------------------------------------------------*/
enum {/* LE GAP Role */
	LE_CENTRAL = 0,
	LE_PERIPHERIAL = 1
};

enum {/* Privacy flag in GATT_UUID_PERIPHERAL_PRIVACY_FLAG */
	LE_PRIVACY_DISABLE = 0,
	LE_PRIVACY_ENABLE = 1
};

enum {/* Random Address Type */
	LE_ADDRTYPE_PUBLIC,
	LE_ADDRTYPE_STATIC,
	LE_ADDRTYPE_PRIVATE_RESOLVABLE,/* set in Peripheral, with 15min lifetime */
	LE_ADDRTYPE_PRIVATE_NON_RESOLVABLE,/* Other`s device, ignore? */
	LE_ADDRTYPE_PRIVATE_RECONNECTION_CENTRAL,/* Local is Central, non-resolvable, in GAP GATT characteristic, Central write to Peripheral */
	LE_ADDRTYPE_PRIVATE_RECONNECTION_PERIPHERAL,/* Local is Peripheral */

	/* FIXME, when read_local_bd have not got public address, use static address instead */
	LE_ADDRTYPE_PUBLIC_STATIC,/* Local used public or static, once selected, will not changed lifetime */
};

enum {/* Discovery mode */
	LE_DISCOVERYMODE_NONE,/* FIXME, may still sending ADV, with no mode bits set in Flags AD Type */
	LE_DISCOVERYMODE_LIMITED,
	LE_DISCOVERYMODE_GENERAL
};

enum {/* Advertising/Scanning Speed */
	LE_FAST,
	LE_SLOW,
	LE_FAST_AUTO_SLOW
};

enum {/* Connection Mode */
	LE_CONNECTMODE_NONE,
	LE_CONNECTMODE_DIRECTED,
	LE_CONNECTMODE_UNDIRECTED
};

void LE_Connect_WhiteList(void);

struct HCI_LELinkStru *LE_Link_FindCurr(struct HCI_LocalStru *local);
UINT8 LE_Addr_LocalSetRandom(struct HCI_LocalStru *local, UINT8 type);
void LE_Addr_LocalSetLinkUsedType(struct HCI_LocalStru *local, UINT8 type);
void LE_Addr_LocalGetPublicStatic(struct HCI_AddressStru *addr/* Any type, In & Out */);
void LE_Addr_RemoteGetCurrent(struct HCI_AddressStru *addr/* Any type, In & Out */);
void LE_WhiteList_Remove(struct HCI_AddressStru *addr)/* New */;
void LE_WhiteList_Add(struct HCI_AddressStru *addr)/* List_NodeNew */;
void LE_WhiteList_Clear(struct HCI_LocalStru *local);
struct HCI_AddressStru *LE_WhiteList_Find(struct HCI_LocalStru *local, struct HCI_AddressStru *addr)/* New */;
void LE_Set_Advertise_Enable(UINT8 enable);

void HCI_EV_LE_Advertising_Report(struct HCI_LocalStru *local, struct HCI_LE_Advertising_ReportEvStru *ev);
void HCI_EV_LE_Read_Remote_Used_Features(struct HCI_LocalStru *local, struct HCI_LE_Read_Remote_Used_Features_CompleteEvStru *ev);
void HCI_EV_LE_Connection_Complete(struct HCI_LocalStru *local, struct HCI_LE_Connection_CompleteEvStru *ev);
void HCI_EV_LE_Connection_Update_Complete(struct HCI_LocalStru *local, struct HCI_LE_Connection_Update_CompleteEvStru *ev);
void HCI_OP_LE_Create_Connection_Cancel(struct HCI_LocalStru *local, struct HCI_LE_Create_Connection_CancelCompStru *ev);
void HCI_OP_LE_Read_Local_Supported_Features(struct HCI_LocalStru *local, struct HCI_LE_Read_Local_Supported_FeaturesCompStru *ev);
void HCI_OP_LE_Read_Buffer_Size(struct HCI_LocalStru *local, struct HCI_LE_Read_Buffer_SizeCompStru *ev);
void HCI_OP_LE_Read_White_List_Size(struct HCI_LocalStru *local, struct HCI_LE_Read_White_List_SizeCompStru *ev);
void HCI_Command_SendInter(void *par, UINT32 opscode);

void HCI_DBG_ADDR(struct HCI_AddressStru *addr, UINT8 *prompt);
void HCI_DBG_DeviceAddr(UINT8 *dbg_str, UINT8 *prompt, struct HCI_AddressStru *addr);
void HCI_DBG_ACLLinks(void);

#endif/* G_HCI_LE_H */

