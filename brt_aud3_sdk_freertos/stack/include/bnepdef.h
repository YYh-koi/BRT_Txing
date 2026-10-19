/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    bnepdef.h
Abstract:

Author:
    Chen Zhongyi
Revision History:
    1.  2001.07.10  Created
    2.  2011.03.16  (CR158)
    3.  2011.06.21  (CR164)
---------------------------------------------------------------------------*/
#ifndef _BNEP_DEF_H_
#define _BNEP_DEF_H_

/*uuid definiton for PAN services*/
#define BNEP_PANU_UUID           0x1115
#define BNEP_NAP_UUID  			 0x1116
#define BNEP_GN_UUID   			 0x1117

/*the following are macro definitions*/
#define MAC_ADDR_LEN        			 6
#define NETWORK_PROTOCOL_TYPE_LEN        2
 
/*Ethernet frame constant definition*/

#define ETHERNET_HEADER_LENGTH							14

#define ETHERNET_DEST_OFFSET							0
#define ETHERNET_SRC_OFFSET								6
#define ETHERNET_LENGTHTYPE_OFFSET						12
#define ETHERNET_PAYLOAD_OFFSET							14
#define ETHERNET_8021P_TYPE_OFFSET						(ETHERNET_LENGTHTYPE_OFFSET+4)
#define NPT_8021P_HEADER_ID								0x8100

/*BNEP packet type definition*/
#define BNEP_GENERAL_ETHERNET							0x00 
#define BNEP_CONTROL									0x01 
#define BNEP_COMPRESSED_ETHERNET						0x02 
#define BNEP_COMPRESSED_ETHERNET_SOURCE_ONLY			0x03 
#define BNEP_COMPRESSED_ETHERNET_DEST_ONLY				0x04

#define BNEP_PACKET_TYPE_NUMBER							(1 + BNEP_COMPRESSED_ETHERNET_DEST_ONLY ) 

/*BNEP control packet type definition*/
#define BNEP_CONTROL_COMMAND_NOT_UNDERSTOOD				0x00
#define BNEP_SETUP_CONNECTION_REQUEST_MSG				0x01
#define BNEP_SETUP_CONNECTION_RESPONSE_MSG				0x02
#define BNEP_FILTER_NET_TYPE_SET_MSG					0x03
#define BNEP_FILTER_NET_TYPE_RESPONSE_MSG				0x04
#define BNEP_FILTER_MULTI_ADDR_SET_MSG					0x05
#define BNEP_FILTER_MULTI_ADDR_RESPONSE_MSG				0x06

/*Network Protocol Type Filter Response Messages*/
#define OPERATION_SUCCESS								0x0000
#define UNSUPPORTED_REQUEST								0x0001 
#define INVALID_NPTR									0x0002 
#define INVALID_MFLR									0x0003 
#define INVALID_SECURITY								0x0004 

/*Setup Connection Response Messages*/
#define OPERATION_SUCCESS								0x0000
#define INVALID_DEST_UUID								0x0001 
#define INVALID_SRC_UUID								0x0002 
#define INVALID_UUID_SIZE								0x0003 
#define CONNECT_NOT_ALLOWED								0x0004 

/*Multicast Address Filter Response Messages*/
#define OPERATION_SUCCESS								0x0000
#define UNSUPPORTED_REQUEST								0x0001 
#define INVALID_MULTICAST_ADDRESS						0x0002 
#define MAXIMUM_REACHED									0x0003 
#define INVALID_SECURITY								0x0004 

#define INVALID_PARAMETER								0xFFFF
#define REQUEST_PENDING									0xFFFE
#define	REQUEST_TIMEOUT									0xFFFD
#define REQUEST_CANCELLED								0xFFFC
#define ERROR_OUT_OF_MEMORY								0xFFFB

/*BNEP extension header type definition */
#define BNEP_EXTENSION_CONTROL							0

/*BNEP extension header definition */
#define BNEP_EXTENSION_HEADER_LEN						2
#define BNEP_EXTENSION_TYPE_OFFSET						0
#define BNEP_EXTENSION_LENGTH_OFFSET					1
#define BNEP_EXTENSION_PAYLOAD_OFFSET					2

/*BNEP extension header constant definition */
#define BNEP_EXTENSION_MAX_LENGTH   					(UCHAR)255

/*BNEP extension control header type offset definition */
#define BNEP_EXTENSION_CONTROL_TYPE_OFFSET				2
#define BNEP_EXTENSION_CONTROL_PAYLOAD_OFFSET			3

#define BNEP_TYPE_OFFSET								0
#define BNEP_GEN_DEST_OFFSET							1
#define BNEP_GEN_SRC_OFFSET								7
#define BNEP_GEN_LENGTHTYPE_OFFSET						13
#define BNEP_GEN_PAYLOAD_OFFSET							15

#define BNEP_CONTROL_TYPE_OFFSET						1
#define BNEP_CONTROL_PAYLOAD_OFFSET						2

/*BNEP type field size*/
#define BNEP_TYPE_FIELD_SIZE							1

/*BNEP control type field size*/
#define BNEP_CONTROL_TYPE_FIELD_SIZE					1

/*for not understand message*/
#define BNEP_NOT_UNDERSTAND_RSP_LEN						1
#define BNEP_NOT_UNDERSTAND_PAYLOAD_OFFSET				0

/*for setup connection message*/
#define BNEP_SERVICE_UUID_16						2
#define BNEP_SERVICE_UUID_32                        4
#define BNEP_SERVICE_UUID_128                       16
#define BNEP_SERVICE_UUID_FIELD_SIZE				1
#define BNEP_SETUP_CONNECTION_RSP_LEN				2

#define BNEP_SETUP_CONNECTION_PAYLOAD_OFFSET		0
#define BNEP_SETUP_CONNECTION_DESTUUID_OFFSET   	1

/*for setup network protocol type message*/
#define BNEP_MIN_NPT_NUMBER							0
#define BNEP_MAX_NPT_NUMBER							421
#define BNEP_EXTENSION_MAX_NPT						63
#define BNEP_NPT_LENGTH								2
#define BNEP_NET_TYPE_RSP_LEN						2
#define BNEP_NPT_LIST_FIELD_SIZE					2

#define BNEP_NPT_PAYLOAD_OFFSET						0
#define BNEP_NPT_START_OFFSET                   	2
#define BNEP_NPT_END_OFFSET							(BNEP_NPT_START_OFFSET + BNEP_NPT_LENGTH )

/*for setup multicast address message*/
#define BNEP_MIN_MULTICAST_ADDRESS_NUMBER				0
#define BNEP_MAX_MULTICAST_ADDRESS_NUMBER				140
#define BNEP_EXTENSION_MAX_MULTICAST_ADDRESS_NUMBER		21
#define BNEP_MULTICAST_RSP_LEN							2
#define BNEP_MULTICAST_ADDRESS_LIST_FIELD_SIZE			2

#define BNEP_MULTICAST_ADDRESS_PAYLOAD_OFFSET			0
#define BNEP_MULTICAST_ADDRESS_START_OFFSET				2
#define BNEP_MULTICAST_ADDRESS_END_OFFSET				(BNEP_MULTICAST_ADDRESS_START_OFFSET + MAC_ADDR_LEN )

#define BNEP_COMPRESSED_LENGTHTYPE_OFFSET				1
#define BNEP_COMPRESSED_PAYLOAD_OFFSET					3

#define BNEP_SRCONLY_SRC_OFFSET							1
#define BNEP_SRCONLY_LENGTHTYPE_OFFSET					7
#define BNEP_SRCONLY_PAYLOAD_OFFSET						9

#define BNEP_DESTONLY_DEST_OFFSET						1
#define BNEP_DESTONLY_LENGTHTYPE_OFFSET					7
#define BNEP_DESTONLY_PAYLOAD_OFFSET					9

#define BNEP_GENERAL_ETHERNET_HLEN						15
#define BNEP_CONTROL_HLEN								1
#define BNEP_COMPRESSED_ETHERNET_HLEN					3
#define BNEP_COMPRESSED_ETHERNET_SOURCE_ONLY_HLEN		9
#define BNEP_COMPRESSED_ETHERNET_DEST_ONLY_HLEN			9

/*the following are structure definitions*/

struct BNEP_NptRangeStru {
	WORD	start;					/* networking protocol type start */
	WORD	end;					/* networking protocol type end */
};

struct BNEP_MultiAddrStru {
	UCHAR	start[MAC_ADDR_LEN];	/* multicast address start */
	UCHAR	end[MAC_ADDR_LEN];		/* multicast address end */
};

struct BNEP_ExtHdrStru {
	UCHAR	type;					/* extension control type */
	UCHAR	len;					/* extension payload length */
	struct BNEP_ExtHdrStru *next;	/* pointer to the next extension header, if none, it is NULL */
	UCHAR	payload[1];				/* extension payload */
};


struct BNEP_ConnReqStru {	
	UCHAR	bd_addr[BDLENGTH];		/* the Bluetooth address of the device to connect */
};

struct BNEP_RelReqStru {
	WORD	bnep_handle;			/* the handle to the session to release */
	WORD	reserved1;
};

struct BNEP_EthernetFrmStru {
	WORD	reserved1;					/* must be zero */
	WORD	len;						/* the length of the ethernet frame */
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */
	UCHAR	frame[1];					/* the ethernet frame */
};

struct BNEP_SetupReqStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	UCHAR	uuid_size;					/* the size of the uuid in the setup control packet to send */
	UCHAR	reserved1;
	WORD	local_uuid;					/* the local and remote uuid, it is one of the following: */
	WORD	remote_uuid;				/* BNEP_PANU_UUID, BNEP_NAP_UUID, BNEP_GN_UUID */
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
};

struct BNEP_SetupRspStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the setup connection control message */
	WORD	local_uuid;					/* the local and remote uuid, it is one of the following: */
	WORD	remote_uuid;				/* BNEP_PANU_UUID, BNEP_NAP_UUID, BNEP_GN_UUID */
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
};

struct BNEP_NptReqStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	ranges_num;					/* the number of the ranges array */
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
	struct BNEP_NptRangeStru ranges[1]; /* networking protocol type range structure */
};

struct BNEP_NptRspStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the network protocol filter control message */
	WORD	ranges_num;					/* the number of the ranges array */
	WORD	reserved1;
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
	struct BNEP_NptRangeStru ranges[1]; /* networking protocol type range structure */
};

struct BNEP_MultiAddrReqStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	ranges_num;					/* the number of the ranges array */	
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
	struct BNEP_MultiAddrStru ranges[1];/* multicast address range structure */
};

struct BNEP_MultiAddrRspStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the multicast address filter control message */
	WORD	ranges_num;					/* the number of the ranges array */	
	WORD	reserved1;
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
	struct BNEP_MultiAddrStru ranges[1];/* multicast address range structure */
};

struct BNEP_UnknownCtrlReqStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	UCHAR	control_type;				/* the type of BNEP control message */
	UCHAR	length;						/* the length of the unknown control payload */
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */
	UCHAR	payload[1];	
};

struct BNEP_NotUnderstoodRspStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	UCHAR	control_type;				/* type of BNEP control message received and caused this message to be sent*/
	UCHAR	reserved1;
	struct BNEP_ExtHdrStru *ext_ptr;	/* pointer to the extension header, if none, it is NULL */	
};

struct BNEP_SvrInfoStru {
	UCHAR	can_accept;						/* if we can accept to connect in, 0: no, 1: yes */
	UCHAR	max_users;						/* the max user we can accept */
};

/*definitions for BNEP callback function and event code*/

/*start---event code for BNEP */
#define EV_BNEP_BASE 									(PROT_BNEP_BASE + 40)
#define EV_BNEP_CONNECT_IND								(EV_BNEP_BASE + 0x0)
#define EV_BNEP_DISCONNECT_IND							(EV_BNEP_BASE + 0x1)
#define EV_BNEP_ETHFRAME_IND							(EV_BNEP_BASE + 0x2)
#define EV_BNEP_SETUP_CONNECTION_IND					(EV_BNEP_BASE + 0x3)
#define EV_BNEP_NETTYPE_FILTER_IND						(EV_BNEP_BASE + 0x4)					
#define EV_BNEP_MULTIADDR_FILTER_IND					(EV_BNEP_BASE + 0x5)					
#define EV_BNEP_UNKNOWN_CONTROL_IND						(EV_BNEP_BASE + 0x6)
#define EV_BNEP_CONN_CFM								(EV_BNEP_BASE + 0x7)
#define EV_BNEP_SETUP_CFM								(EV_BNEP_BASE + 0x8)
#define EV_BNEP_NPT_CFM									(EV_BNEP_BASE + 0x9)
#define EV_BNEP_MULTIADDR_CFM							(EV_BNEP_BASE + 0xA)
#define EV_BNEP_NOT_UNDERSTOOD_CFM						(EV_BNEP_BASE + 0xB)
#define EV_BNEP_DISCONNECT_CFM							(EV_BNEP_BASE + 0xC)
#define EV_BNEP_SENDETHFRM_CFM							(EV_BNEP_BASE + 0xD)
#define EV_BNEP_CONNPARAM_CHANGE_IND					(EV_BNEP_BASE + 0xE)
#define EV_BNEP_RETRIEVE_MTU_IND					    (EV_BNEP_BASE + 0xF)

/*end ---event code for BNEP */

/* definitions for BNEP callback structure for PAN profile */
struct BNEP_ConnIndStru
{
	WORD	bnep_handle;					/* the handle to the bnep session */
	UCHAR	remote_bd[BDLENGTH];			/* the address of the device connected in */
	UINT32  total_bandwidth;				/* Kbits */
	UINT32  max_guaranteed_bandwidth;		/* Kbits */
	UINT8   tl_type;						/* etc: HCI_DEVICE_TYPE_BREDR */
};

struct BNEP_ConnCfmStru
{
	WORD	bnep_handle;					/* the handle to the bnep session */
	UCHAR	remote_bd[BDLENGTH];			/* the address of the device to connect */
};

struct BNEP_DiscIndStru
{
	WORD	bnep_handle;					/* the handle to the bnep session */
	WORD	reserved1;
};

struct BNEP_DiscCfmStru
{
	WORD	bnep_handle;					/* the handle to the bnep session */
	WORD	reserved1;
};

struct BNEP_SetupIndStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	local_uuid;					/* the local and remote uuid, it is one of the following: */
	WORD	remote_uuid;				/* BNEP_PANU_UUID, BNEP_NAP_UUID, BNEP_GN_UUID */
	WORD	reserved1;
};

struct BNEP_SetupCfmStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the setup connection control message */
	UINT32  total_bandwidth;			/* Kbits */
	UINT32  max_guaranteed_bandwidth;	/* Kbits */
	UINT8   tl_type;					/* etc: HCI_DEVICE_TYPE_BREDR */
};

struct BNEP_NptIndStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	ranges_num;					/* the number of the ranges array */
	struct BNEP_NptRangeStru ranges[1]; /* networking protocol type range structure */
};

struct BNEP_NptCfmStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the network protocol type filter control message */
};

struct BNEP_MAddrIndStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	ranges_num;					/* the number of the ranges array */	
	struct BNEP_MultiAddrStru ranges[1];/* multicast address range structure */
};

struct BNEP_MAddrCfmStru {
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	response;					/* the response to the multicast address filter control message */
};

struct BNEP_EthFrmIndStru
{
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	len;						/* the length of the ethernet frame */
	void	*reserved1;
	UCHAR	frame[1];					/* the ethernet frame */
};

struct BNEP_NotUnderStoodStru
{
	WORD	bnep_handle;				/* the handle to the bnep session */
	UCHAR	control_type;				/* the unknown control type */
	UCHAR	reserved1;
};

struct BNEP_UnknownCtrlIndStru
{
	WORD	bnep_handle;				/* the handle to the bnep session */
	WORD	len;						/* the length of the payload body */
	UCHAR	control_type;				/* the unknown control type */
	UCHAR	payload[1];					/* the payload of the control message */
};

#endif
