/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspintegrity.c													 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains functions implementing the BCSP Integrity layer.	 *
 *																			 *
 * AUTHOR : Ye Dongxiang                                                     *
 *                                                                           *
 * HISTORY :                                                                 *
 *                                                                           *
 * ------------------------------------------------------------------------- *
 * Error No. |   Date   |   Changed by   |         Short description         *
 * ---------+----------+----------------+----------------------------------- *
 *           |          |                |                                   *
 ****************************************************************************/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Inclusion of header files														 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "global.h"
#include "g_bcsp.h"
#include "bcspintegrity.h"
#include "bcsphlp.h"


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Public Functions																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/********************************************************************************
Description:
	Calculate the CRC value of the packet .
Arguments:
	UCHAR * pack;	The packet for CRC calculting;
	ULONG len;	The length of the packet.
Return:
	The CRC value of the packet.
********************************************************************************/
WORD CalculateCrc(UCHAR *pack, ULONG len)
{
	UCHAR databyte;
	WORD dest = 0;
	WORD crc = 0xffff;
	ULONG i;

	/* Table for calculating CRC for poly 0x1021, lsb processed first,
	   initial value 0xffff, bits shifted in reverse order */
	const WORD crc_table[] = {
		0x0000, 0x1081, 0x2102, 0x3183,
		0x4204, 0x5285, 0x6306, 0x7387,
		0x8408, 0x9489, 0xa50a, 0xb58b,
		0xc60c, 0xd68d,	0xe70e, 0xf78f
	};
	/* Run the bytes through the crc by treating them as two nibbles. The crc is
	   generated inreverse, i.e. bits are fed into the register from the top: */
	for(i = 0; i < len; i ++) {
		databyte = pack[i];
		crc = (crc >>4) ^ crc_table[(crc ^ databyte) & 0x000f];
		crc = (crc >>4) ^ crc_table[(crc ^ (databyte >> 4)) & 0x000f];
	}

	/* Reverse the bit order of a WORD value. */
	for(i = 0; i < 16; i ++) {
		dest = dest << 1;
		dest |= (crc & 1);
		crc = crc >> 1;
	}
	/* Returns the crc as a big-endian number, regardless of what the native mode is: */
	crc = 0xff00;
	crc &= ((dest & 0x00ff) << 8);
	crc |= ((dest >> 8) & 0x00ff);
	return crc;
}

/***************************************************************************************
Description:
	Examine the checksum and the CRC value(if presented) of packet received.
Arguments:
	UCHAR  *buf;	Pointer to the buffer containing the packet to be examined.
	DWORD  *len;	Length of the packet to be examined including CRC length.
					When returned, the CRC length will be subtracted from it.
Return:
	1 if the packet is intact, 0 if else.
***************************************************************************************/
UCHAR CheckIntegrity(UCHAR *buf, DWORD *len)
{
	WORD expectlen;
	WORD actuallen;
	WORD rxcrc;
	UCHAR result = 1;

	if (!CHECK_CHKSUM(buf)) {
		Bcsp_Dbg_Error(NULL, 0, BCSP_ERR_INVALID_CHKSUM, (CALC_CHKSUM(buf) << 16) + buf[3]);
		return 0;
	}
	actuallen = (WORD)(*len - ((buf[0] & 0x40) >> 5));
	expectlen = (buf[1] >> 4) + ((WORD)buf[2] << 4) + BCSP_HEADER_LEN;
	if (actuallen == expectlen) {
		if (buf[0] & 0x40) {
			rxcrc = buf[*len - 2] + ((WORD)buf[*len - 1] << 8);
			if (rxcrc != CalculateCrc(buf, *len - 2)) {
				Bcsp_Dbg_Error(NULL, 0, BCSP_ERR_INVALID_CRC, 0);
				result = 0;
			}
			*len -= 2;
		}
	} else {
		Bcsp_Dbg_Error(NULL, 0, BCSP_ERR_LEN_MISMATCH, (expectlen << 16) + actuallen);
		result = 0;
	}
	return result;
}
