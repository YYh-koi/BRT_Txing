#ifndef _BRTSDP_DASM_H_
#define _BRTSDP_DASM_H_

void Decode4ErrorResp(UCHAR *pdu, struct SDP_PDU *pack);
UCHAR Decode4ServiceSearchPattern(UCHAR *pdu, DWORD len, struct ListStru *res);
UCHAR Decode4AttributeIDList(UCHAR *pdu, DWORD len, struct ListStru *res);
UCHAR DecodeAdditionBits(DWORD *size, UCHAR mode, UCHAR *pdu);
void Decode4ContinuationState(UCHAR *pdu, struct ContinuationState *state);
UCHAR Decode4ServiceSearchReq(UCHAR *pdu, struct SDP_PDU *pack);
UCHAR Decode4ServiceAttributeReq(UCHAR *pdu, struct SDP_PDU *pack);
UCHAR Decode4ServiceSearchAttributeReq(UCHAR *pdu, struct SDP_PDU *pack);
struct ServiceAttribute* Decode4Attribute(DWORD *size, UCHAR *pdu);  
void Decode4DE(UCHAR *buf, UCHAR *src, DWORD size, UCHAR mode);
void Decode4DESeq(UCHAR *buf, UCHAR *src, DWORD size);
WORD Decode4HandleList(UCHAR *dest, DWORD *dsize, UCHAR *src, DWORD ssize);
WORD Decode4AttributeList(UCHAR *dest, DWORD *dsize, UCHAR *src, DWORD ssize);
WORD Decode4AttributeLists(UCHAR *dest, DWORD *dsize, UCHAR *src, DWORD ssize);

#endif
 
