/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspslip.h														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the constants and macros used in bcspslip.c.		 *
 *	  It also contains prototype of the functions defined in bcspslip.c.	 *
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
#ifndef BRT_BCSP_SLIP_H
#define BRT_BCSP_SLIP_H


/*****************************************************************************
 *    Private constant														 *
 *****************************************************************************/
#define SLIP_BUFFER_SIZE 0xfff


/*****************************************************************************
 *    Private macros														 *
 *****************************************************************************/

/*****************************************************
 *    Convert packet to bytes stream				 *
 *****************************************************/
#define SLIP_CONVERSION(srcpack, destpack, srclen, destindex)	{ \
			int i; \
			destpack[destindex] = 0xc0; \
			destindex --; \
			/* For the byte value 0xc0 & 0xdb, two bytes are transmitted: */ \
			for(i = srclen - 1; i >= 0; i --){ \
				if(srcpack[i] == 0xc0){ \
					destpack[destindex] = 0xdc; \
					destpack[destindex - 1] = 0xdb; \
				}else if(srcpack[i] == 0xdb){ \
					destpack[destindex] = 0xdd; \
					destpack[destindex - 1] = 0xdb; \
				}else{ \
					destpack[destindex] = srcpack[i]; \
					destindex ++; \
				} \
				destindex -= 2; \
			} \
			destpack[destindex] = 0xc0; \
		}

/*****************************************************
 *    Check whether CRC is presented				 *
 *****************************************************/
#define IF_CRC_PRESENT	         (slip_buffer[0] & 0x40)

/*****************************************************
 *    Buffer copy									 *
 *****************************************************/
#define COPY_HEADER_PAYLOAD(buf) {memcpy(buf->data + buf->off, slip_buffer, slip_packet_len);}

/*****************************************************
 *    Convert bytes stream	to packet				 *
 *****************************************************/
#define REVERSE_CONVERSION(pack, datum, idx)	{ \
			pack[idx] = datum; \
			if (datum == 0xdb) { \
				byte_db_detected = TRUE; \
				idx++; \
			} \
			else if (!idx) { \
				idx ++; \
			} \
			else if (byte_db_detected) { \
				if (datum == 0xdc) { \
					pack[idx - 1] = 0xc0; \
				} \
				else if (datum == 0xdd) { \
					pack[idx - 1] = 0xdb; \
				} else {\
					/*DBG_PRINTF("Error: Invalid character follows 0xdb!!!\r\n");*/ \
					idx++; \
				} \
				byte_db_detected = FALSE;\
			} \
			else { \
				idx ++; \
			} \
		}


/*****************************************************************************
 *    Prototypes of functions												 *
 *****************************************************************************/
struct BuffStru* PutPacketIntoSlipLayer(struct BuffStru *pstru);
void			 PutByteIntoSlipLayer(UCHAR *datain, DWORD len);
void SlipDone(void);

#endif
