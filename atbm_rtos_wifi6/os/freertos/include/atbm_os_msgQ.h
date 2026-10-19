#ifndef ATBM_OS_MSGQ_H
#define ATBM_OS_MSGQ_H

#include "atbm_sysops.h"


typedef atbm_void * atbm_os_msgq;
int freertos_os_MsgQ_Create(atbm_os_msgq *pmsgQ, atbm_uint32 *pstack, atbm_uint32 item_size, atbm_uint32 item_num);
int freertos_os_MsgQ_Delete(atbm_os_msgq *pmsgQ);
int freertos_os_MsgQ_Recv(atbm_os_msgq *pmsgQ, atbm_void *pbuf, atbm_uint32 val, int timeout);
int freertos_os_MsgQ_Send(atbm_os_msgq *pmsgQ, atbm_void *pbuf, atbm_uint32 val, int timeout);

#define atbm_os_MsgQ_Create freertos_os_MsgQ_Create
#define atbm_os_MsgQ_Delete	freertos_os_MsgQ_Delete
#define atbm_os_MsgQ_Recv	freertos_os_MsgQ_Recv
#define atbm_os_MsgQ_Send	freertos_os_MsgQ_Send

#endif /* ATBM_OS_MSGQ_H */

