/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	sdppdu.h
Abstract:
	This module defines structures used to describes SDP data units.
Author:
	Wu Peng
Revision History:
    1.  2000.01.20  Created
---------------------------------------------------------------------------*/
/* Data type definition and format definition for PDU */
#ifndef _BRTSDP_PDU_H_
#define _BRTSDP_PDU_H_


/*error response packet*/
struct ErrRspPack {
	WORD	code;
 	/*char	info[128];*/
};

/*contuation state*/
struct ContinuationState {
	UCHAR	len;
	DWORD	info[4];
};

/*data element sequence used in the SDP implementation*/
struct DataElementSeq {
	UCHAR	descriptor;
	UCHAR	flag;
	WORD	byte_count;
	DWORD	size;
	struct ListStru	var;
};

/*servicesearch request*/
struct SSReqPack {
	struct DataElementSeq	pattern;
	WORD	max;
};

/*servicesearch response*/
struct SSRspPack {
	WORD	total_count;
	WORD	current_count;
	DWORD	*hdl;
};

/*serviceattribute request*/
struct SAReqPack {
	DWORD	service_handle;
	WORD	max;
	struct DataElementSeq	id_list;
};

/*serviceattribute response,servicesearchattribtue response*/
struct SARspPack {
	UCHAR	flag;
	WORD	byte_count;
	struct DataElementSeq	attr;
};

/*servicesearchattribute request*/
struct SSAReqPack {
	struct DataElementSeq	pattern;
	WORD	max;
	struct DataElementSeq	id_list;
};

/*SDP PDU*/
struct SDP_PDU {
	UCHAR	id;
	WORD	tid;
	WORD	len;
	UCHAR	*var;
	struct ContinuationState	state;
};

#endif


