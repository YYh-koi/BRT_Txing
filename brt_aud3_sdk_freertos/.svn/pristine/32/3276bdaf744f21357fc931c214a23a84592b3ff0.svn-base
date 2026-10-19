#ifndef _TL_BCSP_H_
#define _TL_BCSP_H_

/*****************************************************************************
 *    Private constants                                                      *
 *****************************************************************************/
#define HCI_COMMAND_DATA		1
#define HCI_ACL_DATA			2
#define HCI_SCO_DATA			3
#define	HCI_EVENT_DATA			4

/*****************************************************************************
 *    Definition of private structures                                       *
 *****************************************************************************/


/*****************************************************************************
 *    type-declaration synonym												 *
 *****************************************************************************/
/**
  * Register callbacks to user:
  * 1. notify outside the HCI data decoded by BCSP.
  * 2. notify outside the Sync result(LinkFailInd/LinkEstablishedInd).
  * 3. the lower Send function to be called to send the encoded BCSP data.
  * zmj added.
  */
typedef struct tag_BCSPClientCallbackStru {
	void (*Func_DataInd)(unsigned char *pBuf, unsigned long len);
	void (*Func_LinkResult)(int result);
	void (*Func_LowerSendCallback)(unsigned char *pBuf, unsigned long len);
} BCSPClientCallbackStru;

enum {
    LINK_RESULT_OK,
    LINK_RESULT_ERROR,
};


/*****************************************************************************
 *    Prototypes of API functions											 *
 *****************************************************************************/
void InitBcspTL(void);
void DoneBcspTL(void);
int BcspStartup(HANDLE tl_hdl, BCSPClientCallbackStru *pCallback, HANDLE host_hdl);
void BcspShutdown(HANDLE tl_hdl);
int BcspSend(HANDLE tl_hdl, unsigned char *pBuf, unsigned long len);


#endif

