/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspintegrity.h													 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the macros used in bcspintegrity.c.				 *
 *	  It also contains prototype of the functions defined in				 *
 *	  bcspintegrity.c.														 *
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
#ifndef BRT_BCSP_INTEGRITY_H
#define BRT_BCSP_INTEGRITY_H


/*****************************************************************************
 *    Private macros														 *
 *****************************************************************************/

/*****************************************************
 *    Calculate the check-sum						 *
 *****************************************************/
#define	CALC_CHKSUM(header)	((UCHAR)((~(header[0] + header[1] + header[2])) & 255))

/*****************************************************
 *    Check the receiving check-sum					 *
 *****************************************************/
#define CHECK_CHKSUM(header)	(CALC_CHKSUM(header) == header[3])

/*****************************************************
 *    Calculate the CRC value						 *
 *****************************************************/
#define CALCULATE_CRC(pack, len)	{ \
			WORD crc; \
			crc = CalculateCrc(pack, len); \
			pack[len] = (UCHAR)(crc & 0xff); \
			pack[len + 1] = (UCHAR)(crc >> 8); \
		}

/*****************************************************
 *    Check the receiving CRC value					 *
 *****************************************************/
#define CHECK_CRC(rxcrc, buf)	(rxcrc == CalculateCrc(GET_PACKET_HEADER(buf), GET_PACKET_LEN(buf)))


/*****************************************************************************
 *    Prototypes of functions												 *
 *****************************************************************************/
WORD CalculateCrc(UCHAR *pack, ULONG len);
UCHAR CheckIntegrity(UCHAR *buf, DWORD *len);

#endif
