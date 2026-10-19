/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : g_bcsp.c														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file defines all the global variables.							 *
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

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Global Variables																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct FsmInst *g_bcsp_fsminst;

const char timer_shy_str[] = "$ShyTo";
const char timer_conf_str[] = "$CfTo";
const char timer_wack_str[] = "$WAckT";
const char timer_sack_str[] = "$SAckT";

const UCHAR syncreq_packet[SYNC_PAYLOAD_LEN] = { 0xda, 0xdc, 0xed, 0xed };
const UCHAR syncresp_packet[SYNC_PAYLOAD_LEN] = { 0xac, 0xaf, 0xef, 0xee };
const UCHAR confreq_packet[SYNC_PAYLOAD_LEN] = { 0xad, 0xef, 0xac, 0xed };
const UCHAR confresp_packet[SYNC_PAYLOAD_LEN] = { 0xde, 0xad, 0xd0, 0xd0 };

