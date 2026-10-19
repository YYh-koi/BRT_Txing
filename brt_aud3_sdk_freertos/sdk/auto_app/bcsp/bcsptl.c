/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcsptl.c														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020903                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains API functions between local host HCI and transport  *
 *	  layer driver. It also provides callback functions exported to BCSP     *
 *    stack.																 *
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
#include "gap_ui.h"
#include "g_bcsp.h"
#include "bcspui.h"
#include "bcsptl.h"
#ifdef CONFIG_BT_CHIP_CSR8350
#include "internal_csr_dsp_8350.h"
#define ACL_HEADER_LENGTH  4
#define L2CAP_HEADER_LENGTH  4
#endif
#include "brt_trace.h"

#define LOG(fmt, ...) bse_log_info("bcsp", fmt, ## __VA_ARGS__)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Private Variables																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static unsigned char reset_command[] = {0x01, 0x03, 0x0c, 0x00 };
static unsigned char reset_event[] = {0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};
static BOOL isWarmReset = FALSE;

BCSPClientCallbackStru * g_Callbacks = NULL;

/* AMP Based TL */
Func_Receive *func_rcv;
HANDLE g_hHost;

int IsResetCommand(UCHAR *buf);
int IsWarmResetCommand(UCHAR *buf);

static int bcsp_recv_callback(HANDLE host_hdl, UINT8 *pBuf, UINT32 len)
{
	if (g_Callbacks && g_Callbacks->Func_DataInd) {
		g_Callbacks->Func_DataInd(pBuf, len);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////
////Function BcspStartup
/////////////////////////////////////////////////////////////////
int BcspStartup(HANDLE tl_hdl, BCSPClientCallbackStru *pCallback, HANDLE host_hdl)
{
	g_hHost = host_hdl;
	func_rcv = bcsp_recv_callback;
	g_Callbacks = pCallback;
	return 0;
}

/////////////////////////////////////////////////////////////////
////Function BcspShutdown
/////////////////////////////////////////////////////////////////
void BcspShutdown(HANDLE tl_hdl)
{
}

/////////////////////////////////////////////////////////////////
////Function BcspSend
/////////////////////////////////////////////////////////////////
int BcspSend(HANDLE tl_hdl, unsigned char *pBuf, unsigned long len)
{
	WORD result = TRUE;

	switch (pBuf[0]) {
	case HCI_COMMAND_DATA:
		if(!IsResetCommand(pBuf)) {
			if (0xfc == pBuf[2]) {
				UCHAR protid = pBuf[4] & 0x3f;
				result = BcspSendPacketFromHci(protid, pBuf + 4, (UINT16)(len - 4));
				if (result)
					result = IsWarmResetCommand(pBuf);
			} else {
				result = BcspSendPacketFromHci(COMMAND, pBuf, (UINT16)len);
			}
		}
		break;
	case HCI_ACL_DATA:
		result = BcspSendPacketFromHci(ACL_DATA, pBuf, (UINT16)len);
		break;
	case HCI_SCO_DATA:
		result = BcspSendPacketFromHci(SCO_DATA, pBuf, (UINT16)len);
		break;
	}
	if (result)
		return len;
	return 0;
}

/////////////////////////////////////////////////////////////////
////Function InitBcspTL
/////////////////////////////////////////////////////////////////
void InitBcspTL(void)
{
	BcspInit();
	BcspSyncWithLocalDeviceReq();
}

void DoneBcspTL(void)
{
	BcspDone();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Private Functions																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/********************************************************************************
Description:
	Judge whether the current HOST HCI command is HCI_RESET. This command should
	not be passed to the host controller.
Arguments:
	buf			Pointer to a BuffStru structure containing the command packet.
Return:
	TRUE if the command is HCI_RESET.
	FALSE if the command isn't HCI_RESET.
********************************************************************************/
int IsResetCommand(UCHAR *buf)
{
	if (!memcmp(buf, reset_command, sizeof(reset_command))) {
		if (func_rcv != NULL) {
			(*func_rcv)(g_hHost, reset_event, sizeof(reset_event));
		}
		return TRUE;
	} else
		return FALSE;
}

/********************************************************************************
Description:
	Judge whether the current BCCMD command is warm_reset. This command should
	not be passed to the host controller.
Arguments:
	buf			Pointer to a BuffStru structure containing the command packet.
Return:
	TRUE if the command is WARM_RESET.
	FALSE if the command isn't WARM_RESET.
********************************************************************************/
int IsWarmResetCommand(UCHAR *buf)
{
	if (buf[7] == 0x02 && buf[8] == 0x40) {
		SLEEP(500); // Wait until Warm Reset command is sent.
		BcspResync();
	}
	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Callback functions for BCSP stack												 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/********************************************************************************
Description:
	Function called by BCSP stack to transmit the slipped bytes to the UART.
Arguments:
	data		Pointer to the buffer containing the data bytes to be transmitted.
	len			Number of bytes to be transmitted.
Return:
	TRUE if the function succeeds.
	FALSE if the function fails.
********************************************************************************/
int TransmitBytesToUart(UCHAR *data, ULONG len)
{
#ifdef CONFIG_BCSP_PRINT_DATA
	LOG("[BCSP] Tx: %d", len);
	DbgPrintData(data, len ,0);
#endif
	//return UART_TL_WriteBuf((char *)data, len);
	if (g_Callbacks && g_Callbacks->Func_LowerSendCallback) {
		g_Callbacks->Func_LowerSendCallback(data, len);
	}

	return 0;
}

/********************************************************************************
Description:
	Function called by BCSP stack to pass the demuxed packet to the upper HOST HCI.
Arguments:
	type		Type (e.g. HCI command, event or pure data) of the packet passed
				to the upper layer.
	buf 		Pointer to the BuffStru containing the data packet to be passed.
Return:
	NONE.
********************************************************************************/
void ReceiveDataFromBcsp(UCHAR type, struct BuffStru *buf)
{
	if (func_rcv != NULL) {
		switch(type) {
		case EVENT:
			buf->data[buf->off] = HCI_EVENT_DATA;
			break;
		case ACL_DATA:
			buf->data[buf->off] = HCI_ACL_DATA;
			{
#ifdef CONFIG_BT_CHIP_CSR8350
				UINT16	nCID = ((UINT16)(buf->data[buf->off+ACL_HEADER_LENGTH+L2CAP_HEADER_LENGTH])<<8) +(UINT16)(buf->data[buf->off+ACL_HEADER_LENGTH+L2CAP_HEADER_LENGTH-1]);
				if ((hlp_a2dp_get_cid() != 0)&&(nCID != 0)&&(nCID == hlp_a2dp_get_cid())) {
					LOG("[bcsp]free A2DP data RX(%d)",buf->len - buf->off);
					BuffFree(buf);
					return;
				}
#endif
			}
			break;
		case SCO_DATA:
			buf->data[buf->off] = HCI_SCO_DATA;
			break;
		}
		(*func_rcv)(g_hHost, buf->data + buf->off, buf->len - buf->off);
	}
	BuffFree(buf);
}

/********************************************************************************
Description:
	Function called by BCSP stack to notify the upper layer of the link loss event.
Arguments:
	status	An integer specify the status of resynchronization. It can take:
			BCSP_LINK_RESYNC_START ---- Start to resynchronize.
			BCSP_LINK_RESYNC_SUCCEED ---- Resynchronize succeed.
			BCSP_LINK_RESYNC_TIMEOUT ---- Resynchronize fail due to timeout.
Return:
	NONE.
********************************************************************************/
void LinkFailInd(UCHAR status)
{
	if (g_Callbacks && g_Callbacks->Func_LinkResult) {
		g_Callbacks->Func_LinkResult(status == BCSP_LINK_RESYNC_SUCCEED ? LINK_RESULT_OK : LINK_RESULT_ERROR);
	}
}

/********************************************************************************
Description:
	Function called by BCSP stack to notify the upper layer of the link established
	event.
Arguments:
	status	An integer specify the status of resynchronization. It can take:
			BCSP_LINK_SYNC_SUCCEED ---- Link with lower BCSP stack established.
			BCSP_LINK_SYNC_TIMEOUT ---- Failed to establish link with lower BCSP stack.
Return:
	NONE.
********************************************************************************/
void LinkEstablishedInd(UCHAR status)
{
	if (status == BCSP_LINK_SYNC_SUCCEED) {
		if(isWarmReset) {
			//app_gap_reset_hardware();
			isWarmReset = FALSE;
			// TODO: restart need to set_pskey?
			if (g_Callbacks && g_Callbacks->Func_LinkResult) {
				g_Callbacks->Func_LinkResult(LINK_RESULT_OK);
			}
		} else {
			isWarmReset = TRUE;
			set_psk();
		}
	} else {
		if (g_Callbacks && g_Callbacks->Func_LinkResult) {
			g_Callbacks->Func_LinkResult(LINK_RESULT_ERROR);
		}
	}
}

