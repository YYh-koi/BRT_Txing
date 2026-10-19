/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef BT_HCI_LIB_H
#define BT_HCI_LIB_H

#include <stdint.h>
//#include <sys/cdefs.h>
#include <sys/types.h>
#include "autoconf_bt.h"

/** Struct types */

//#define CONFIG_H5_MEMORY_DETECT
#ifdef CONFIG_MEMORY_LEAK_DETECT
#define H5_NEW_(A,B,C) DBG_NEW(A,B,C)
#define H5_FREE_(A,B,C) DBG_FREE(A,B,C)
#else
#define H5_NEW_(A,B,C) NEW(A)
#define H5_FREE_(A,B,C) FREE(A)
#endif

#if 0
#define H5_NEW(CB, LEN) \
    do { \
        if (CB != NULL) { \
            (HC_BT_HDR *) (CB)->alloc((int) (LEN), __FILE__, __LINE__); \
        } else { \
            H5_NEW_((int) (LEN), __FILE__, __LINE__); \
        };\
    } while(0)

#define H5_FREE(CB, A, B) \
    do { \
        if (CB != NULL) { \
            (CB)->dealloc(A, B, __FILE__, __LINE__); \
        } else { \
            H5_FREE_(A, __FILE__, __LINE__); \
        };\
    } while(0)
#endif

#define H5_NEW(CB, LEN) (HC_BT_HDR *) (CB)->alloc((int) (LEN), __FILE__, __LINE__)
#define H5_FREE(CB, A) (CB)->dealloc(A,  __FILE__, __LINE__)
#define H5_FREE2(CB, A, B) (CB)->dealloc(A, B, __FILE__, __LINE__)

/** Typedefs and defines */

/* Message event ID passed from Host/Controller lib to stack */
#define MSG_HC_TO_STACK_HCI_ERR        0x1300 /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#define MSG_HC_TO_STACK_HCI_ACL        0x1100 /* eq. BT_EVT_TO_BTU_HCI_ACL */
#define MSG_HC_TO_STACK_HCI_SCO        0x1200 /* eq. BT_EVT_TO_BTU_HCI_SCO */
#define MSG_HC_TO_STACK_HCI_EVT        0x1000 /* eq. BT_EVT_TO_BTU_HCI_EVT */
#define MSG_HC_TO_STACK_L2C_SEG_XMIT   0x1900 /* eq. BT_EVT_TO_BTU_L2C_SEG_XMIT */

/* Message event ID passed from stack to vendor lib */
#define MSG_STACK_TO_HC_HCI_ACL        0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#define MSG_STACK_TO_HC_HCI_SCO        0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#define MSG_STACK_TO_HC_HCI_CMD        0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */

typedef struct {
	uint16_t          event;
	uint16_t          len;
	uint16_t          offset;
	uint16_t          layer_specific;
} HC_BT_HDR;

#define BT_HC_HDR_SIZE (sizeof(HC_BT_HDR))

/* Generic purpose transac returned upon request complete */
typedef void* TRANSAC;

/** Bluetooth Power Control States */
typedef enum {
    BT_HC_CHIP_PWR_OFF,
    BT_HC_CHIP_PWR_ON,
}  bt_hc_chip_power_state_t;

/** Bluetooth Low Power Mode */
typedef enum {
    BT_HC_LPM_DISABLE,
    BT_HC_LPM_ENABLE,
    BT_HC_LPM_WAKE_ASSERT,
    BT_HC_LPM_WAKE_DEASSERT,
} bt_hc_low_power_event_t;

/** Receive flow control */
typedef enum {
    BT_RXFLOW_OFF, /* add transport device fd to select set */
    BT_RXFLOW_ON,  /* remove transport device to from select set */
} bt_rx_flow_state_t;

/** HCI logging control */
typedef enum {
    BT_HC_LOGGING_OFF,
    BT_HC_LOGGING_ON,
} bt_hc_logging_state_t;

/** Result of write request */
typedef enum {
    BT_HC_TX_SUCCESS,  /* a buffer is fully processed and can be released */
    BT_HC_TX_FAIL,     /* transmit fail */
    BT_HC_TX_FRAGMENT, /* send split ACL pkt back to stack to reprocess */
} bt_hc_transmit_result_t;

/** Result of preload initialization */
typedef enum {
    BT_HC_PRELOAD_SUCCESS,
    BT_HC_PRELOAD_FAIL,
} bt_hc_preload_result_t;

/** Result of postload initialization */
typedef enum {
    BT_HC_POSTLOAD_SUCCESS,
    BT_HC_POSTLOAD_FAIL,
} bt_hc_postload_result_t;

/** Result of low power enable/disable request */
typedef enum {
    BT_HC_LPM_DISABLED,
    BT_HC_LPM_ENABLED,
} bt_hc_lpm_request_result_t;

/** Host/Controller Library Return Status */
typedef enum {
    BT_HC_STATUS_SUCCESS,
    BT_HC_STATUS_FAIL,
    BT_HC_STATUS_NOT_READY,
    BT_HC_STATUS_NOMEM,
    BT_HC_STATUS_BUSY,
    BT_HC_STATUS_CORRUPTED_BUFFER
} bt_hc_status_t;


/* Section comment */

/*
 * Bluetooth Host/Controller callback structure.
 */

/* called to send hci data to UART*/
typedef int (*send_hci_data_cb)(int msg_id, unsigned char *buf, unsigned int len);

/* called upon bt host wake signal */
typedef void (*hostwake_ind_cb)(bt_hc_low_power_event_t event);

/* preload initialization callback */
typedef void (*preload_result_cb)(TRANSAC transac, bt_hc_preload_result_t result);

/* postload initialization callback */
typedef void (*postload_result_cb)(TRANSAC transac, bt_hc_postload_result_t result);

/* lpm enable/disable callback */
typedef void (*lpm_result_cb)(bt_hc_lpm_request_result_t result);

/* datapath buffer allocation callback (callout) */
typedef char* (*alloc_mem_cb)(int size, char *file, int line);

/* datapath buffer deallocation callback (callout) */
typedef int (*dealloc_mem_cb)(TRANSAC transac, char *p_buf, char *file, int line);

/* transmit result callback */
typedef int (*tx_result_cb)(TRANSAC transac, char *p_buf, bt_hc_transmit_result_t result);

/* a previously setup buffer is read and available for processing
   buffer is deallocated in stack when processed */
typedef int (*data_ind_cb)(TRANSAC transac, char *p_buf, int len);

typedef struct {
	/** set to sizeof(bt_hc_callbacks_t) */
	size_t         size;
	send_hci_data_cb send_cb;

	/* notifies caller result of preload request */
	preload_result_cb  preload_cb;

	/* notifies caller result of postload request */
	postload_result_cb  postload_cb;

	/* notifies caller result of lpm enable/disable */
	lpm_result_cb  lpm_cb;

	/* notifies hardware on host wake state */
	hostwake_ind_cb       hostwake_ind;

	/* buffer allocation request */
	alloc_mem_cb   alloc;

	/* buffer deallocation request */
	dealloc_mem_cb dealloc;

	/* notifies stack data is available */
	data_ind_cb data_ind;

	/* notifies caller when a buffer is transmitted (or failed) */
	tx_result_cb  tx_result;
} bt_hc_callbacks_t;


#endif /* BT_HCI_LIB_H */

