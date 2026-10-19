#ifndef _BRTSDP_ASM_H_
#define _BRTSDP_ASM_H_

#define BUFFER_OFFSET	(L2CAP_CO_RESERVE+HCI_RESERVE)

struct BuffStru *Code4Trans(struct SDP_PDU *pack);
void Code4ErrorResp(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceSearchPattern(struct ListStru *src, UCHAR *buf);
void Code4AttributeIDList(struct ListStru *src, UCHAR *buf);
UCHAR CodeAdditionBits(DWORD size, UCHAR mode, UCHAR *buf);
void Code4ContinuationState(UCHAR *buf, struct ContinuationState *state);
void Code4ServiceSearchReq(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceSearchResp(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceAttributeReq(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceAttributeResp(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceSearchAttributeReq(struct SDP_PDU *pack, UCHAR *buf);
void Code4ServiceSearchAttributeResp(struct SDP_PDU *pack, UCHAR *buf);

#endif
