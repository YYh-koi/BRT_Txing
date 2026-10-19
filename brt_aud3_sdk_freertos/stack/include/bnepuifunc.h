#ifndef _BNEP_UI_FUNC_H_
#define _BNEP_UI_FUNC_H_

WORD BNEPInit( void );
void BNEPDone(void);

void BNEP_EstablishConnection( struct BNEP_ConnReqStru *arg);
void BNEP_ReleaseConnection(struct BNEP_RelReqStru *arg);
void BNEP_Setup_Connection(struct BNEP_SetupReqStru *arg);
void BNEP_Filter_NPT(struct BNEP_NptReqStru *arg);
void BNEP_Filter_MultiAddr(struct BNEP_MultiAddrReqStru *arg);
void BNEP_Setup_Connection_Rsp(struct BNEP_SetupRspStru *arg);
void BNEP_Filter_NPT_Rsp(struct BNEP_NptRspStru *arg);
void BNEP_Filter_MultiAddr_Rsp(struct BNEP_MultiAddrRspStru *arg);
void BNEP_Send_Unknown_Control(struct BNEP_UnknownCtrlReqStru *arg);
void BNEP_Cmd_Not_Understood(struct BNEP_NotUnderstoodRspStru *arg);
void BNEP_Compressed_Ethernet(struct BNEP_EthernetFrmStru *arg);
void BNEP_SrcOnly_Ethernet(struct BNEP_EthernetFrmStru *arg);
void BNEP_DestOnly_Ethernet(struct BNEP_EthernetFrmStru *arg);
void BNEP_General_Ethernet(struct BNEP_EthernetFrmStru *arg);
void BNEP_Ethernet(struct BNEP_EthernetFrmStru *arg);
void BNEP_SrvInfoSetting( struct BNEP_SvrInfoStru *arg);

#endif
