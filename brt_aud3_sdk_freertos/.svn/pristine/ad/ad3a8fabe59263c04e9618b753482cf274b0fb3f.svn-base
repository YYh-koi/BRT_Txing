/*---------------------------------------------------------------------------
Module Name:
	hci_hlp.c
Abstract:
	This module defines HCI Coding, Security and Misc internal functions.
Author:
	Wang Yao
---------------------------------------------------------------------------*/

#include "hci_hlp.h"
#include "hci_le.h"

/*---------------------------------------------------------------------------*/
/* 							Local Controller Object				   	  	     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function use to Terminate a limited scanning or advertising of LE.
	This function is called when NOCP_NOCB RTX timeout.
---------------------------------------------------------------------------*/
void HCIC_Local_RTX(struct FsmInst *fi, UINT8 event, struct HCI_LocalTimerStru *in)
{
	struct HCI_LocalStru *local;/* FIXME, memory pointer search is not stable, index needed */

	if (((local = List_NodeExist(HCI_LOCALLIST, in->local)) != NULL) && (local->timer[event - 1] != NULL)) {
		local->timer[event - 1] = NULL;
		
		switch (event) {/* Quit scan or advertise */
#ifdef CONFIG_LE
			case HCI_LOCALRTX_LE_SCAN:/* Limited */
				LE_Set_Scan_Enable(HCI_LE_SCAN_ENABLE_SCANNING_DISABLED);/* Disable Scanning, When command complete, LE status will change */
				break;
			case HCI_LOCALRTX_LE_ADVERTISE:/* Limited */
				LE_Set_Advertise_Enable(HCI_ADVERTISING_ENABLE_DISABLED_DEFAULT);/* Disable Advertising */
				break;
			case HCI_LOCALRTX_LE_FAST_AUTO_SLOW_ADVERTISE:
				LE_Set_Advertising_Parameters(HCI_ADVERTISING_TYPE_CONNECTABLE_UNDIRECTED, LE_DISCOVERYMODE_GENERAL
					, LE_SLOW, HCI_ADVERTISING_FILTER_POLICY_SCAN_ANY_CONNECT_ANY);
				break;
			case HCI_LOCALRTX_LE_FAST_AUTO_SLOW_SCAN:
				LE_Set_Scan_Parmeters(HCI_LE_SCAN_TYPE_ACTIVE_SCANNING, LE_DISCOVERYMODE_GENERAL, LE_SLOW);
				break;
			case HCI_LOCALRTX_LE_RESOLVABLE_PRIVATE_ADDRESS:
				/* UPF43, need to stop scan or adv before doing this, discussed with SunChaoJing */
				LE_Addr_LocalSetRandom(local, LE_ADDRTYPE_PRIVATE_RESOLVABLE);/* Regenerate Private Resolvable address */
				break;
#endif
			case HCI_LOCALRTX_NOCP:
				HCI_NOCP_ConsumeNode(local);
				break;
			case HCI_LOCALRTX_UNKNOWN_ACLDATA:
				{
					struct BuffStru **node;
					while ((node = List_RemoveHead(&local->unknown_data)) != NULL) {
						FREE(*node);/* Free Buffer */
						LFREE(node);/* Free listnode */
					}
				}
				break;
			default:/* Error */
				break;
		}
	}

	FREE(in);
}

/*---------------------------------------------------------------------------
Description:
	This function use to start a timer for local controller object.
---------------------------------------------------------------------------*/
void HCI_Local_StartTimer(struct HCI_LocalStru *local, UINT8 *prompt, UINT32 timeout, UINT8 restart, UINT8 event)
{
	struct HCI_LocalTimerStru *arg;/* wild */

	if ((local->timer[event - 1] == NULL) || (restart == TRUE)) {
		(arg = NEW(sizeof(struct HCI_RTXTimerStru)))->local = local;/* FIXME, local->timer shall be changed */
		local->timer[event - 1] = FSM_RestartTimer(local->timer[event - 1]
			, hci_fsminst, timeout, event, (FsmFunc *)HCIC_Local_RTX, arg, prompt, 1);
	}/* else, Already have a timer, no need to restart */
}

/*---------------------------------------------------------------------------
Description:
	This function use to init a local controller object before any command 
	issued. Allocates a available Transport Layer identifier, 
    from 1 to 255.
---------------------------------------------------------------------------*/
void HCI_Local_Init(struct HCI_LocalStru *local)
{
	UINT8 id;/* static for warp around, not for reuse */
	
	/* BREDR shall be 0, who create ACL who is 0, L2cap concept */
	/* FIXME, we have problem in l2cap when mutilple BREDR 0 exist, local shall be the only AMPID 0 of HCI_LOCALLIST */
	/* FIXME, include LE only on AMPID0? */
	local->cmd_num = 1;
	local->security_mode = HCI_SECURITY_MODE2;/* By default, maybe change to 4 when Write_SimplePairingMode success */
	/* By default, bondable */

	/* LE support only LE Commands? */
	memset(local->info.supported_commands, 0xFF, 64);/* All supported by default, FIXME */
	memset(local->info.lmp_features, 0xFF, 2 * 8);/* All supported by default, FIXME */

	if (local->addr[HCI_ADDR_PUBLIC_STATIC].cap == HCI_DEVICE_TYPE_BREDR) {/* From TL->GetType */
		local->ampid = HCI_AMPID_BREDR;/* FIXME, only one BREDR could exist */
		local->amp_status = HCI_AMP_STATUS_ONLY_USED_BY_BLUETOOTH;
		local->addr[HCI_ADDR_PUBLIC_STATIC].cap = HCI_ADDRESS_CAP_BREDR;/* FIXME, maybe LE or DuleMode, Will be check by HCI commands later */
	} else {
		id = 0;
		do {
			id = (id % 255) + 1;/* 1~255 */
		} while (HCI_Local_FindByID(id) != NULL);/* Assert all ID have been allocated does not possible */
	
		local->ampid = id;/* Allocate ID */
		local->info.timeout[HCI_LINFO_TIMEOUT_CONN_ACCEPT] = 0x1F40;/* Time Default: AMP Type dependent */
		local->info.timeout[HCI_LINFO_TIMEOUT_LOGICAL_LINK_ACCEPT] = 0x1F40;/* Time Default: AMP Type dependent */
		local->info.mask |= HCI_LINFO_MASK_BLOCK_BASED_DATA_FLOW_CONTROL;/* Before Read_FlowControlMode */		
		local->addr[HCI_ADDR_PUBLIC_STATIC].cap = (local->addr[HCI_ADDR_PUBLIC_STATIC].cap == HCI_DEVICE_TYPE_WIFI) 
			? HCI_ADDRESS_CAP_80211 : HCI_ADDRESS_CAP_UWB;
	}

	/* FIXME, cbk to ask for recover file saved remote device list */
}

/*---------------------------------------------------------------------------
Description:
	This function use to remove/reset one local controller object, 
	Remove following children:

	ACL Data Packets in QosList;
	Remote/Link Node Object;
	Command Pendding/Sent List node;
---------------------------------------------------------------------------*/
void HCI_Local_RemoveOne(struct HCI_LocalStru *local, UINT8 flag)
{
	struct HCI_CommandQueueStru *queue;
	struct HCI_CommandQueueNodeStru *qnode;
	void *node;
	UINT8 i;

	for (i = 0; i <= HCI_LOCALRTX_TIMER_COUNT - 1; i++) {
		FSM_DelTimer(&local->timer[i]);/* Kill LE Limited Scan/Advertise and other Timers, set NULL included */
	}

	HCI_NOCP_FreeAll(local);

	List_QoSRemoveAll(&local->acldata_list);/* Remove all pendding ACL data */
	HCI_Link_RemoveByLR(local);/* Remove local releated links, includes command remove by link */

	if (flag == HCI_LOCAL_REMOVE) {/* Notify controller, host is leaving */
		NEWSTRU(struct HCI_Write_Scan_EnableStru, par);
		par->scan_enable = HCI_SCAN_ENABLE_NO_SCANS_ENABLED;/* non-Discoverable, not connectable */
		HCI_Command_SendEmergency(local, par, HCI_OPS_WRITE_SCAN_ENABLE);

#ifdef CONFIG_LE
		if (local->info.white_list.head != NULL) {/* Clear whitelist */
			HCI_Command_SendEmergency(local, NULL, HCI_OPS_LE_CLEAR_WHITE_LIST);
			List_RemoveAll(&local->info.white_list);
		}
#endif

		/* Remove L2 command nodes */
		while ((queue = local->cmd2_list.head) != NULL) {
			(qnode = queue->list.head)->mask |= HCI_CMDLIST_NODE_MANDATORY;/* Force to end this L2 queue */
			HCI_CommandL2_Remove(local, queue, qnode, HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER);/* local maybe NULL */
		}
	}

	while ((node = local->cmd_list.head) != NULL) {
		if ((flag == HCI_LOCAL_REMOVE) || ((node != local->pendcmd))) {/* Sent cmd shall be removed */
			HCI_Command_Remove(local, node, NULL, 0, HCI_CMDERR(HCI_STATUS_LOCAL_CONTROLLER_REMOVED));
		} else {/* Keep unsent commands when reset */
			break;
		}
	}

    while ((node = List_RemoveHead(&local->unknown_data)) != NULL) {
        FREE(*(struct BuffStru **)node);
        LFREE(node);
    }

	if (flag == HCI_LOCAL_REMOVE) {
		if (local->func != NULL) {
			local->func->Shutdown(local->tl_hdl);/* Call TL func to shutdown, add or remove controller need to notify l2cap */
		}
		HCI_Local_ChangedNotify(local, local->tl_hdl, HCI_LOCALCONTROLLER_REMOVE);/* add cbk to notify app shutdown tl */
		List_RemoveAt(HCI_LOCALLIST, local);
		LFREE(local);/* Release Memory */
	} else {/* Reset */
		/* FIXME, was raced between HCI_Controller_Init and HCI_Security_SetBondable */
		/* FIXME, remove timers, addresses, data buffer and other parameters? */
		local->info.mask &= HCI_LINFO_MASK_RESET_KEEP_BITS;/* FIXME, check others if need to set 0? */
	}
}

/*---------------------------------------------------------------------------
Description:
	This function use to remove one/all local controller object(s).
---------------------------------------------------------------------------*/
void HCI_Local_Remove(struct HCI_LocalStru *local)
{
	if (local == NULL) {/* Remove all */
		while ((local = HCI_LOCALLIST->head) != NULL) {
			HCI_Local_RemoveOne(local, HCI_LOCAL_REMOVE);
		}
	} else {/* Remove one */
		HCI_Local_RemoveOne(local, HCI_LOCAL_REMOVE);
	}
}

/*---------------------------------------------------------------------------
Description:
	This function use to get a Local Controller object by given ampid.
---------------------------------------------------------------------------*/
struct HCI_LocalStru *HCI_Local_FindByID(UINT8 ampid)
{
	return LIST_FIND(HCI_LOCALLIST, struct HCI_LocalStru *, ampid, ampid);
}

/*---------------------------------------------------------------------------
Description:
	This function use to get a Local Controller object by given host_hdl
	(Local Obj Address).
---------------------------------------------------------------------------*/
struct HCI_LocalStru *HCI_Local_FindByHDL(HANDLE host_hdl)
{
	struct HCI_LocalStru *local = HCI_LOCALLIST->head;
	
	while (local && (local != host_hdl)) {
		local = LNEXT(local);
	}
	
	return local;
}

/*---------------------------------------------------------------------------
Description:
	This function use to get a Local Controller object by addr.
---------------------------------------------------------------------------*/
struct HCI_LocalStru *HCI_Local_FindByAddr(struct HCI_AddressStru *l)
{
	struct HCI_LocalStru *local = HCI_LOCALLIST->head;
	
	while (local) {
		if (HCI_CompareAddr(&local->addr[HCI_ADDR_PUBLIC_STATIC], l)
			|| HCI_CompareAddr(&local->addr[HCI_ADDR_RANDOM], l)) {			
			break;
		}
		local = LNEXT(local);
	}
	
	return local;
}

/*---------------------------------------------------------------------------
Description:
	This function use to get a Local Controller object by given tl_hdl.
---------------------------------------------------------------------------*/
struct HCI_LocalStru *HCI_Local_FindByTLHDL(HANDLE tl_hdl)
{
	return LIST_FIND(HCI_LOCALLIST, struct HCI_LocalStru *, tl_hdl, tl_hdl);
}

/*---------------------------------------------------------------------------
Description:
	This function use to make a HCI_EVO_LOCALCONTROLLER_CHANGED cbk.
	If the Primary BR/EDR Controller`s Link Manager does not support Secure 
	Simple Pairing, it shall be the only Controller in the Controller List.
---------------------------------------------------------------------------*/
void HCI_Local_ChangedNotify(struct HCI_LocalStru *local, HANDLE tl_hdl, UINT8 action)
{
	NEWSTRU(struct HCI_LocalController_ChangedStru, par);
	UINT32 slot;

	if (local != NULL) {
		if (local->addr[HCI_ADDR_PUBLIC_STATIC].cap & HCI_ADDRESS_CAP_MAIN) {
			par->device_type = HCI_DEVICE_TYPE_BREDR;/* LE included? FIXME, check HCI_LOCALCONTROLLER_NOTIFY_BREDR_LE_CAP */
		} else {/* AMP */
			par->device_type = (local->addr[HCI_ADDR_PUBLIC_STATIC].cap & HCI_ADDRESS_CAP_UWB) 
				? HCI_DEVICE_TYPE_UWB : HCI_DEVICE_TYPE_WIFI;
		}

		slot = (local->info.mask & HCI_LINFO_MASK_BLOCK_BASED_DATA_FLOW_CONTROL) 
			? HCI_LINFO_PACKET_DATABLOCK : HCI_LINFO_PACKET_ACL;
		par->ampid = local->ampid;
		par->max_pdusize = local->info.packet_length[slot];
		par->amp_status = local->amp_status;
		memcpy(&par->addr, &local->addr[HCI_ADDR_PUBLIC_STATIC], sizeof(struct HCI_AddressStru));
		
		if ((local->ampid == HCI_AMPID_BREDR) && (par->max_pdusize != 0)) {/* Before read_buffer_size, maybe 0 */
			par->max_pdusize = par->max_pdusize * local->info.packet_num[slot];/* MPS for BREDR */
		}
	} else {
		memset(par, 0, sizeof(struct HCI_LocalController_ChangedStru));
	}
	par->tl_hdl = tl_hdl;
	par->action = action;

	/* FIXME, amp controller shall be following BREDR0, check SSP */
	/* FIXME, upper need to know this is a LE? */
	HCI_Cbk(local, NULL, par, sizeof(struct HCI_LocalController_ChangedStru), HCI_EVO_LOCALCONTROLLER_CHANGED);

#ifdef HCI_DBG_ALL
	HCI_DBG_LocalView("ChangeNotify", local);
#endif
}

/*---------------------------------------------------------------------------*/
/* 							Remote Controller Object	 	       	  	     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function use to notify application that a remote device node 
	changes or created.
---------------------------------------------------------------------------*/
void HCI_Remote_ChangeNotify(struct HCI_LocalStru *local, UINT8 src, struct HCI_RemoteStru *remote, UINT8 *class_of_device, UINT8 rssi, UINT8 *eir, UINT8 eir_len)
{
	struct HCI_Remote_InfoStru *par;/* FIXME, notify remote have been killed or added */
	
	/* FIXME, local not needed, NULL? */
	memset((par = NEWVARLEN(struct HCI_Remote_InfoStru *, eir, eir_len))
		, 0, VARLEN(struct HCI_Remote_InfoStru *, eir, eir_len));
	memcpy(&par->addr, &remote->addr, sizeof(struct HCI_AddressStru));
	
#ifdef CONFIG_LE
	LE_Addr_Remote2PublicStatic(&par->addr);/* ATT/SMP/L2CAP object using public/static address when possible, but may random */
#endif

	if (class_of_device != NULL) {
		memcpy(par->class_of_device, class_of_device, 3);
	}
	par->rssi = rssi;/* HCI internal will not save RSSI */
	if (eir_len > 0) {
		par->eir_len = eir_len;
		memcpy(par->eir, eir, eir_len);
	}
	par->src = src;
	HCI_Cbk(local, NULL, par, VARLEN(struct HCI_Remote_InfoStru *, eir, eir_len), HCI_EVO_REMOTECONTROLLER_CHANGED);/* Event mainly for APP */

#ifdef HCI_DBG_ALL
	HCI_DBG_RemoteView("ChangeNotify", remote);
#endif
}

/*---------------------------------------------------------------------------
Description:
	This function use to find a remote object by given BD for Legacy BREDR 
	controller.
---------------------------------------------------------------------------*/
struct HCI_RemoteStru *HCI_Remote_FindByBD(UINT8 *bd)
{
	struct HCI_RemoteStru *remote = HCI_REMOTELIST->head;

	while ((remote != NULL) && (HCI_CompareBD(&remote->addr, bd) == 0)) {
		remote = LNEXT(remote);
	}
	
	/* FIXME, need local and peer_address_type for LE? */
	return remote;
}
	
/*---------------------------------------------------------------------------
Description:
	This function use to find a remote object by given ADDR.
---------------------------------------------------------------------------*/
struct HCI_RemoteStru *HCI_Remote_FindByAddr(struct HCI_AddressStru *r)
{
	struct HCI_RemoteStru *remote;

	remote = HCI_REMOTELIST->head;
	while (remote) {
#ifdef CONFIG_LE
		if (SMP_CompareAddr(&remote->addr, r)) /* defined in hci_le.h */
#else
		if (HCI_CompareAddr(&remote->addr, r)) 
#endif
		{
			break;
		}
		remote = LNEXT(remote);
	}
	
	return remote;	
}

/*---------------------------------------------------------------------------
Description:
	This function use to find or create a new Remote object.
---------------------------------------------------------------------------*/
static struct HCI_RemoteStru *HCI_Remote_Create(void)
{
	struct HCI_RemoteStru *remote;

	List_AddHead(HCI_REMOTELIST, (remote = List_NodeNew(sizeof(struct HCI_RemoteStru))));/* AddHead for high priority */
	remote->conn_interval[HCI_REMOTE_MIN] = HCI_TGAP_INITIAL_CONN_INTERVAL_MIN;/* Default LE Parameters */
	remote->conn_interval[HCI_REMOTE_MAX] = HCI_TGAP_INITIAL_CONN_INTERVAL_MAX;/* Default LE Parameters */
	remote->page_scan_repetition_mode = HCI_PAGE_SCAN_REPETITION_MODE_R2;/* Default value, easy to establish the ACLLink */

	return remote;
}

/*---------------------------------------------------------------------------
Description:
	This function use to find or create a new Remote object by given bd.
---------------------------------------------------------------------------*/
struct HCI_RemoteStru *HCI_Remote_FindCreateByBD(struct HCI_LocalStru *local, UINT8 src, UINT8 *bd, 
	UINT8 *class_of_device, UINT8 page_scan_repetition_mode, UINT16 clock_offset, UINT8 rssi, UINT8 *eir, UINT8 eir_len)
{
	struct HCI_RemoteStru *remote;

	if ((remote = HCI_Remote_FindByBD(bd)) == NULL) {
		remote = HCI_Remote_Create();/* Lifetime control needed, FIXME */
		memcpy(&remote->addr.bd, bd, BD_ADDR_LEN);
		remote->addr.atype = HCI_OWN_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS;
		remote->addr.cap = HCI_ADDRESS_CAP_BREDR;
	} else {/* remote ready exist */
		remote->addr.cap |= HCI_ADDRESS_CAP_BREDR;/* FIXME, cap maybe changed, cbk to upper? */
	}

	if (src == HCI_REMOTE_INFO_SRC_INQUIRY) {/* By Inquiry */
		remote->page_scan_repetition_mode = page_scan_repetition_mode;
		remote->clock_offset = clock_offset;
	}

	HCI_Remote_ChangeNotify(local, src, remote, class_of_device, rssi, eir, eir_len);/* APP in charge of filter duplicated */
	
	return remote;
}

/*---------------------------------------------------------------------------
Description:
	This function use to find or create a new Remote object by given addr, 
	or update Device type.
Device_Address:
	Public: Company_Assigned(24bits), Company_ID(24bits)
	Random: Hash(24bits), Random(24bits)
---------------------------------------------------------------------------*/
struct HCI_RemoteStru *HCI_Remote_FindCreateByAddr(struct HCI_LocalStru *local, UINT8 src, UINT8 *bd, 
	UINT8 addr_type, UINT8 rssi, UINT8 *eir, UINT8 eir_len, UINT8 cap, UINT8 curaddr)
{
	struct HCI_RemoteStru *remote;
	struct HCI_AddressStru r;

	r.cap = cap;/* LE/BREDR caps */
	r.atype = addr_type;/* Public or Random */
	memcpy(r.bd, bd, BD_ADDR_LEN);/* r will not be changed, read only */

	/* FIXME, remote will not be removed since link disconnected, so public address remote will be kept, 
	if the true current address for the remote is random, remote object still could not save random as 
	its address, but public */
	
	/* r will NOT be modified inside */
	if ((remote = HCI_Remote_FindByAddr(&r)) == NULL) {/* 1st time create NEW remote obj */
		remote = HCI_Remote_Create();/* Lifetime control needed, FIXME */
		curaddr = TRUE;/* Force to update current address when create new remote */
	}

	if (curaddr) {
		/* UPF43, update random(current controller used) address for apple */
		/* UPF43, upper layer is using pub address, shall not update for cur */
		/* FIXME, LinkRef, get new caps from upper call? */
		r.cap |= remote->addr.cap;/* Leave old cap in remote, add new */
		memcpy(&remote->addr, &r, sizeof(struct HCI_AddressStru));/* HCI use current advertise/inquiry address, whatever type it is */

		/* Change to DuleMode BREDR/LE, Remote does not contain AMP */
		HCI_Remote_ChangeNotify(local, src, remote, NULL, rssi, eir, eir_len);/* APP in charge of filter duplicated */
	}
	
	return remote;
}

/*---------------------------------------------------------------------------
Description:
	This function use to free one remote object.
---------------------------------------------------------------------------*/
static void HCI_Remote_RemoveOne(struct HCI_RemoteStru *remote)
{
#ifdef HCI_DBG_ALL
	HCI_DBG_RemoteView("Remove", remote);
#endif

	/* FIXME, notify remote have been killed */
	HCI_Link_RemoveByLR(remote);/* Remove remote releated links */
	FREE(remote->sec.oob_data);/* Free if exist */
	LFREE(remote);
}

/*---------------------------------------------------------------------------
Description:
	This function use to remove and free remote objects.
---------------------------------------------------------------------------*/
void HCI_Remote_Remove(struct HCI_RemoteStru *remote)
{
	if (remote == NULL) {/* Remove all */
		while ((remote = List_RemoveHead(HCI_REMOTELIST)) != NULL) {
			HCI_Remote_RemoveOne(remote);
		}
	} else {/* Remove only one remote */
		List_RemoveAt(HCI_REMOTELIST, remote);
		HCI_Remote_RemoveOne(remote);
	}
}

/*---------------------------------------------------------------------------
Description:
	This function use to remove a remote object when not used.
---------------------------------------------------------------------------*/
void HCI_Remote_RemoveUnuse(struct HCI_RemoteStru *remote)
{
	struct HCI_LinkStru *link;

	link = HCI_LINKLIST->head;
	while (link) {
		if (link->remote == remote) {
			return;
		}
		link = LNEXT(link);
	}

	HCI_Remote_Remove(remote);/* Remove unused remote object */
}

/*---------------------------------------------------------------------------*/
/* 							Structure <-> Air Octets			   	  	     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function use to deal machine and protocol air endian relationship.
---------------------------------------------------------------------------*/
static void HCI_Coding_Endian(UINT8 **src, UINT8 **dest, UINT8 len)
{
#ifdef CONFIG_BIG_ENDIAN
	UINT8 m = len - 1;
	UINT8 n = 0;
	do {
		*(*dest + n) = *(*src + m);
		n++;
	} while (m--);
#else
	memcpy(*dest, *src, len);/* HCI is little endian */
#endif
	*dest += len;
	*src += len;
}

/*---------------------------------------------------------------------------
Description:
	This function use to get BITs in a BitPointer by given len.
---------------------------------------------------------------------------*/
UINT8 HCI_Coding_GetBits(struct HCI_BitPointerStru *bp, UINT8 len)
{
	UINT8 ret;
	UINT8 off;

	ret = *(bp->p) >> (off = bp->off);/* Get 8 - off bits */
	if (len < (off = 8 - off)) {/* Want < Get, Enough or too many */
		ret &= ((1 << len) - 1);/* Cut off to get len bits */
	} else {/* Not enough, get next octet part */
		ret |= ((*(++(bp->p)) & ((1 << (len - off)) - 1)) << off);
	}
	
	if ((off = 8 - off + len) >= 8) {
		off -= 8;
	}
	bp->off = off;
	
	return ret;
}

/*---------------------------------------------------------------------------
Description:
	This function use to get max structure member length according by bp 
	which is point to the coding info p.
---------------------------------------------------------------------------*/
static UINT8 HCI_Coding_GetMaxEle(struct HCI_BitPointerStru *bp)/* bp should read only */
{
#ifndef CONFIG_SUB_STRUCTURE_ALIGNED_FIXED
	UINT8 type;
	UINT8 len = HCI_CODE_U1;/* Get U1,U2,U4 */
	UINT8 tmp;
	
	while ((type = HCI_Coding_GetBits(bp, 2)) != HCI_CODE_0BITS) {/* 2bits Grouping Bits */
		tmp = HCI_Coding_GetBits(bp, (UINT8)(1 << type));/* bp jump 2,4,8 bit */
		if (type == HCI_CODE_2BITS) {
			MAXSELF(len, tmp);
		}			
	}

	return 1 << (len - 1);
#else
    return 4;/* FIXME, ARM Linux GCC = 4? */
#endif
}

/*---------------------------------------------------------------------------
Description:
	This function use to calculate structure or air stream data length, 
	which can be uncomplete. FIXME, partial structure will lead invalid 
	visit memory issue, but how to solve?
---------------------------------------------------------------------------*/
UINT16 HCI_CodingLen(UINT8 *src, UINT8 *end, const UINT8 *p, UINT8 flag)
{
	struct HCI_BitPointerStru bp;
	struct HCI_BitPointerStru brep;
	struct HCI_XParamStru *x;
	UINT8 *osrc = src;/* use to calc src stru length only */
	UINT8 *dst = NULL;
	UINT8 **stru = (flag & HCI_CODING_INSTRU) ? &src : &dst;
	UINT8 type;
	UINT8 rep = 0;/* Default */
	UINT8 len;

#ifdef HCI_DBG_ALL
	//HCI_DBG_ViewCodingP(info);
#endif

	bp.p = (UINT8 *)p;
	bp.off = 0;
	for (; ; ) {
		switch (type = HCI_Coding_GetBits(&bp, 2)) {/* 2bits Grouping Bits */
			case HCI_CODE_2BITS:
				type = HCI_Coding_GetBits(&bp, 2);/* Get 2bits, Basic Type */
				len = (type != HCI_CODE_N1) ? (1 << (type - 1)) : 1;
				if (src + len > end) {
					goto out;/* Overflow */
				} else {
					if (type == HCI_CODE_N1) {
						dst++;
						rep = (*src++) - 1;
						brep = bp;/* Record this repeat info point */
						type = HCI_Coding_GetMaxEle(&bp);
						HCI_PADDING(*stru, type);/* get structure padding */
						bp = brep;/* Give back BP */
					} else {
						HCI_PADDING(*stru, len);
						src += len;
						dst += len;
					}
				}
				break;
			case HCI_CODE_4BITS:/* Get 4bits */
				if ((type = HCI_Coding_GetBits(&bp, 4)) != 0000) {
					type += 1;
					if (src + type > end) {
						goto out;/* Overflow */
					}
					src += type;
					dst += type;
				} else {/* x */
					x = (struct HCI_XParamStru *)(*stru);

					dst += (flag & HCI_CODING_INSTRU)
						? x->len/* Encode, src=stru=k */
						: sizeof(struct HCI_XParamStru) + end - src;/* Decode, dst=stru=k, FIXME, one more octet */
				}
				break;
			case HCI_CODE_8BITS:/* Get 8bits */
				if (src + (type = HCI_Coding_GetBits(&bp, 8)) > end) {
					goto out;/* Overflow */
				}
				src += type;
				dst += type;
				break;
			default:/* Structure terminator */
				if (rep == 0) {
					goto out;/* OK, Done */
				} else {
					bp = brep;
					rep--;
				}
		}
	}

out:
	return (flag & HCI_CODING_OUTSRC) ? (UINT16)((UINT32)(src - osrc)) : (UINT16)((UINT32)dst);
}

/*---------------------------------------------------------------------------
Description:
	This function use to convert a Structure into a bitstream.
	This function use to convert a bitstream into a Structure.
	Can phase uncomplete structure.
---------------------------------------------------------------------------*/
void HCI_Coding(UINT8 *src, UINT8 *end, UINT8 *dst, const UINT8 *p, UINT8 flag)/* Info Section */
{
	struct HCI_BitPointerStru bp;
	struct HCI_BitPointerStru brep;
	struct HCI_XParamStru *x;
	UINT8 **stru = (flag & HCI_CODING_INSTRU) ? &src : &dst;/* Pointer to src or dst depend on which is structure */
	UINT8 type;
	UINT8 rep = 0;/* Default */

	bp.p = (UINT8 *)p;
	bp.off = 0;
	for (; ; ) {
		switch (type = HCI_Coding_GetBits(&bp, 2)) {/* 2bits Grouping Bits */
			case HCI_CODE_2BITS:
				type = HCI_Coding_GetBits(&bp, 2);/* Get 2bits, Basic Type */
				if (src + ((type != HCI_CODE_N1) ? (1 << (type - 1)) : 1) > end) {
					return;/* Not enough */
				}
				switch (type) {
					case HCI_CODE_N1:
						rep = *src++;
						*dst++ = rep;
						rep--;
						brep = bp;/* Record this repeat info point */
						type = HCI_Coding_GetMaxEle(&bp);
						HCI_PADDING(*stru, type);/* get structure padding */
						bp = brep;/* Give back BP */
						break;
					case HCI_CODE_U1:
						*dst++ = *src++;
						break;
					case HCI_CODE_U2:
						HCI_PADDING(*stru, 2);
						HCI_Coding_Endian(&src, &dst, 2);
						break;
					default:/* HCI_CODE_U4 */
						HCI_PADDING(*stru, 4);
						HCI_Coding_Endian(&src, &dst, 4);
						break;
				}
				break;
			case HCI_CODE_4BITS:/* Get 4bits */
				if ((type = HCI_Coding_GetBits(&bp, 4)) != 0000) {
					type += 1;
					if (src + type > end) {
						return;/* Not enough */
					}
					memcpy(dst, src, type);
					dst += type;
					src += type;
				} else {/* x, 2pointer */
					x = (struct HCI_XParamStru *)(*stru);
					
					if (flag & HCI_CODING_INSTRU) {/* Encode, src=stru=k */
						memcpy(dst, x->data, x->len);
					} else {/* Decode, dst=stru=k */
						if ((x->len = end - src) > 0) {
							memcpy(x->data, src, x->len);
						}
					}
				}
				break;
			case HCI_CODE_8BITS:/* Get 8bits */
				if (src + (type = HCI_Coding_GetBits(&bp, 8)) > end) {
					return;/* Not enough */
				}
				memcpy(dst, src, type);
				dst += type;
				src += type;
				break;
			default:/* Structure terminator */
				if (rep == 0) {
					return;
				} else {
					bp = brep;/* Give back BP */
					rep--;
				}
		}
	}
}

/*---------------------------------------------------------------------------*/
/* 									Miscs						   	  	     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
Description:
	This function use to get EvIndex(Internal Convenience) 
	from Evcode(Spec defined) by taking account with their gaps.
---------------------------------------------------------------------------*/
UINT8 HCI_DT_GetEvIndex(UINT8 evcode)
{
	const struct HCI_IndexGapStru *node = &hci_table_evgap[0];
	UINT8 index = evcode - 1;/* Base */

	while ((node->code <= evcode) && (node <= HCI_EVGAP_LAST)) {
		index -= node->gap;
		node++;
	}

	return index;
}

/*---------------------------------------------------------------------------
Description:
	This function use to get supported command bitmask index by given opcode.
---------------------------------------------------------------------------*/
UINT8 HCI_DT_GetSupCMDBitIndex(UINT16 opcode)
{
	const struct HCI_OpCodeGapStru *node = &hci_table_supportcmdgap[0];
	const struct HCI_OpCodeGapStru *next;
	UINT32 index;

	while (node <= HCI_SUPCMDGAP_LAST) {/* Get 2 lines */
		next = node + 1;
		if ((opcode >= node->opcode) && ((index = opcode - node->opcode + node->index) < next->index)) {/* Range End of OpCode */
			return (UINT8)index;
		}
		node = next;
	}

	return 0;/* Error */
}

/*---------------------------------------------------------------------------
Description:
	This function use to get a op/ev cbk involved count.
	index: etc: HCI_EVI_CONNECTION_REQUEST/HCI_OPC_*
---------------------------------------------------------------------------*/
UINT8 HCI_CbkGetNum(UINT16 index)
{
	struct HCI_CbkNodeStru *node;
	const struct HCI_CbkLineStru *line;
	const struct HCI_CbkLineStru *lastline;
	UINT8 count = 0;

	node = HCI_EVCBKLIST->head;
	while (node) {
		lastline = (line = node->table) + node->size - 1;
		while (line <= lastline) {
			if (line->id == index) {
				count++;
				break;/* One table, only one line match */
			}
			line++;/* Goto next line */
		}
		node = LNEXT(node);/* Goto next table */
	}

	return count;/* How many cbk cares this event/op */
}

/*---------------------------------------------------------------------------
Description:
	This function use to dispatch events for upper layer, give a ACTIVE 
	callback when a Command Node has been terminated. give a PASSIVE event 
	callback when receive a event. arg shall be single memory with len octets.
Notice:
	If mutilple line matches, only the 1st line will be hit within one table.
	arg: only one memory block.
	op_ev: Opcode(Optional) << 16 | Evindex(Mandatory)
Architecture:
	HCI_EVCBKLIST	->head		->node		->node...	->tail
					line		line		line		line
					line		line		line		line
					...			...			...			...
Line:
	HCI_EVI_ / HCI_EVO_ / HCI_OPC_ 		->		Func.
---------------------------------------------------------------------------*/
void HCI_Cbk(struct HCI_LocalStru *local, void *context, void *arg, UINT32 len, UINT32 op_ev)
{
	struct HCI_CbkNodeStru *node;
	const struct HCI_CbkLineStru *line;
	const struct HCI_CbkLineStru *lastline;
	void *par = NULL;

	node = HCI_EVCBKLIST->head;/* 1st node is HCI_Internal */
	while (node) {
		/* 1st line of this table */
		lastline = (line = node->table) + node->size - 1;/* lastline in current table */

		while (line <= lastline) {/* Searching line by line, within this table object node */
			if ((line->id == (UINT16)op_ev) || (((op_ev >> 16) != 0) && (line->id == (op_ev >> 16)))) {/* EVI_ or OPC_ */
				
				if (arg != NULL) {/* len > 0 */
					memcpy((par = NEW(len)), arg, len);/* Make a copy of arg */
				}
				if (node == HCI_EVCBKLIST->head) {/* HCI_Internal */
					(*((HCI_CbkTableFunc *)line->func))(local, par);
					FREE(par);/* Internal general free */
				} else {/* External */
					(*((HCI_CbkFunc *)line->func))((HANDLE)(((len << 16) | (local ? local->ampid : 0)))
						, context, par, op_ev);/* host handle gives ampid & arg len */
				}
				break;/* Only one line could be matched in a table */
			} else {
				line++;/* Goto next line */
			}
		}

		node = LNEXT(node);/* Goto next HCI callback table */
	}

	FREE(arg);
}

/*---------------------------------------------------------------------------
Description:
	This function use to set packet type according to local controller LMP 
	feature. It is recommended to enable as many packet types as possible. 
	Note that it is allowed to enable packet types that are not supported by 
	the local device.
Packet Type:
	Slot,		SCO(1M),		eSCO(1M),		eSCO(2-3M),		ACL(1M),	ACL(2-3M)
	1			DM1				-				-				DM1			DM1
	1			-				-				-				DH1			2DH1
	1			HV1				-				-				-			-
	1			HV2				-				2EV3			-			-
	1			HV3				EV3				3EV3			-			-
	1			DV				-				-				-			3DH1
	3			-				-				-				DM3			2DH3
	3			-				-				-				DH3			3DH3
	3			-				EV4				2EV5			-			-
	3			-				EV5				3EV5			-			-
	5			-				-				-				DM5			2DH5
	5			-				-				-				DH5			3DH5
---------------------------------------------------------------------------*/
UINT16 HCI_Set_PacketType(struct HCI_LocalStru *local, UINT8 link_type)
{
	UINT16 pt;
#if 0
	UINT8 *fea = local->info.lmp_features[0];/* Local only? */
	
	switch (link_type) {
		case HCI_LINK_TYPE_ACL:
			pt = HCI_PACKET_TYPE_DEFAULT_ACL;
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_3_SLOT_PACKETS)) {
				pt &= ~(HCI_PACKET_TYPE_DM3_MAY | HCI_PACKET_TYPE_DH3_MAY);
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_5_SLOT_PACKETS)) {
				pt &= ~(HCI_PACKET_TYPE_DM5_MAY | HCI_PACKET_TYPE_DH5_MAY);
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ENHANCED_DATA_RATE_ACL_2_MBPS_MODE) == 0) {
				pt |= HCI_PACKET_TYPE_2DH1_MAY_NOT;
				
				/* This feature bit shall only be set if the "Enhanced Data Rate ACL 2 Mbps mode" feature bit is set */
				if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ENHANCED_DATA_RATE_ACL_3_MBPS_MODE) == 0) {
					pt |= HCI_PACKET_TYPE_3DH1_MAY_NOT;
				}
				if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_3SLOT_ENHANCED_DATA_RATE_ACL_PACKETS) == 0) {
					pt &= ~HCI_PACKET_TYPE_DH3_MAY;
					pt |= HCI_PACKET_TYPE_3DH3_MAY_NOT | HCI_PACKET_TYPE_2DH3_MAY_NOT | HCI_PACKET_TYPE_3DH3_MAY_NOT;
				}
				if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_5SLOT_ENHANCED_DATA_RATE_ACL_PACKETS) == 0) {
					pt &= ~(HCI_PACKET_TYPE_DM5_MAY | HCI_PACKET_TYPE_DH5_MAY);
					pt |= HCI_PACKET_TYPE_2DH5_MAY_NOT | HCI_PACKET_TYPE_3DH5_MAY_NOT;
				}
			}
			break;
		case HCI_LINK_TYPE_ESCO:/* Actually this is for HCI_Set_SynchronousParam, Include SCO/eSCO use new command */
			pt = HCI_PACKET_TYPE_DEFAULT_SYNCHRONOUS;
			
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_HV2_PACKETS) == 0) {/* FIXME, check remote? */
				pt &= ~HCI_PACKET_TYPE_HV2_MAY;
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_HV3_PACKETS) == 0) {
				pt &= ~HCI_PACKET_TYPE_HV3_MAY;
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_EV4_PACKETS) == 0) {
				pt &= ~HCI_PACKET_TYPE_EV4_MAY;
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_EV5_PACKETS) == 0) {
				pt &= ~HCI_PACKET_TYPE_EV5_MAY;
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ENHANCED_DATA_RATE_SYNCHRONOUS_2_MBPS_MODE) == 0) {
				pt |= HCI_PACKET_TYPE_2EV3_MAY_NOT;
			} else {/* This feature bit shall only be set if the "Enhanced Data Rate eSCO 2Mbps mode" feature bit is set */
				if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ENHANCED_DATA_RATE_SYNCHRONOUS_3_MBPS_MODE) == 0) {
					pt |= HCI_PACKET_TYPE_3EV3_MAY_NOT;
				}
				if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_3SLOT_ENHANCED_DATA_RATE_SYNCHRONOUS_PACKETS) == 0) {
					pt &= ~(HCI_PACKET_TYPE_EV4_MAY | HCI_PACKET_TYPE_EV5_MAY);
					pt |= HCI_PACKET_TYPE_2EV5_MAY_NOT | HCI_PACKET_TYPE_3EV5_MAY_NOT;
				}
			}
			break;
		default:/* case HCI_LINK_TYPE_SCO, Actually this is for HCI_OPS_ADD_SCO_CONNECTION */
			pt = HCI_PACKET_TYPE_DEFAULT_SCO;
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_HV2_PACKETS) == 0) {
				pt &= ~HCI_PACKET_TYPE_HV2;
			}
			if (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_HV3_PACKETS) == 0) {
				pt &= ~HCI_PACKET_TYPE_HV3;
			}
			break;
	}
#else
#if 0
	switch (link_type) {
		case HCI_LINK_TYPE_ACL:
			pt = HCI_PACKET_TYPE_DEFAULT_ACL;
			break;
		case HCI_LINK_TYPE_ESCO:/* Actually this is for HCI_Set_SynchronousParam, Include SCO/eSCO use new command */
			pt = HCI_PACKET_TYPE_DEFAULT_SYNCHRONOUS;
			break;
		default:/* case HCI_LINK_TYPE_SCO, Actually this is for HCI_OPS_ADD_SCO_CONNECTION */
			pt = HCI_PACKET_TYPE_DEFAULT_SCO;
			break;
	}
#else
	pt = (link_type == HCI_LINK_TYPE_ACL) 
		? HCI_PACKET_TYPE_DEFAULT_ACL
		: ((link_type == HCI_LINK_TYPE_ESCO) 
			? HCI_PACKET_TYPE_DEFAULT_SYNCHRONOUS
			: HCI_PACKET_TYPE_DEFAULT_SCO);
#endif
#endif

	return pt;
}

/*---------------------------------------------------------------------------
Description:
	This function use to fill common parmeters in following structures:
	struct HCI_Setup_Synchronous_ConnectionStru
	struct HCI_Accept_Synchronous_Connection_RequestStru	
---------------------------------------------------------------------------*/
void HCI_Set_SynchronousParam(struct HCI_LocalStru *local, struct HCI_Synchronous_ConnectionStru *in, struct HCI_Synchronous_ConnectionStru *esco)
{
	UINT8 *fea = local->info.lmp_features[0];
	//UINT32 aircoding;

	if (in == NULL) {/* Default without input */
		esco->transmit_bandwidth = 8000;
		esco->receive_bandwidth = 8000;
		esco->max_latency = 0x0004;/* milliseconds */
		esco->retransmission_effort = HCI_RETRANSMISSION_EFFORT_ONE_OPTIMIZE_FOR_LINK_QUALITY;

#if 0
	aircoding = (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_CVSD_SYNCHRONOUS_DATA))
		? HCI_VOICE_SETTING_AIR_CODING_FORMAT_CVSD
		: ((HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ULAW_LOG_SYNCHRONOUS_DATA))
			? HCI_VOICE_SETTING_AIR_CODING_FORMAT_ULAW
			: ((HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_ALAW_LOG_SYNCHRONOUS_DATA))
				? HCI_VOICE_SETTING_AIR_CODING_FORMAT_ALAW
				: HCI_VOICE_SETTING_AIR_CODING_FORMAT_TRANSPARENT_DATA));
	/* (HCI_CHECK_BIT(fea, HCI_LMP_FEATURES_TRANSPARENT_SYNCHRONOUS_DATA)) */
	/* FIXME, Error, all not support? */
#endif

		esco->voice_setting = 
			(UINT16)(HCI_VOICE_SETTING_INPUT_CODING_LINEAR 
			| HCI_VOICE_SETTING_INPUT_SAMPLE_SIZE_16_BIT 
			| HCI_VOICE_SETTING_INPUT_DATA_FORMAT_2S_COMPLEMENT
			//| aircoding/* checked by LMP features */
			| (0x0 << 2));/* Linear PCM Bit Position: number of bit positions that MSB of sample is away from starting at MSB (only for Linear PCM) */
	} else {
		memcpy(esco, in, sizeof(struct HCI_Synchronous_ConnectionStru));
		//esco->voice_setting &= ~0x0003;
		//esco->voice_setting |= aircoding;
	}

	if (esco->packet_type == 0) {/* Change if app not care about it */
		esco->packet_type = HCI_Set_PacketType(local, HCI_LINK_TYPE_ESCO);/* Packet type will not be used by input */
	}
}

/*---------------------------------------------------------------------------
Description:
	This function use to check if a command request is going to set a 
	non-all(0) filter for inquiry. 
---------------------------------------------------------------------------*/
UINT8 HCI_Check_Set_Event_Filter_Inquiry(struct HCI_Set_Event_FilterStru *par)
{
	/* Do not care Connect filter now */
	return (par->condition_plen >= 1) 
		? ((par->filter_type == HCI_FILTER_TYPE_INQUIRY_RESULT) 
			? ((par->condition[0] != HCI_FILTER_CONDITION_TYPE_ALL_DEVICES)
				? HCI_SET_EVENT_FILTER_ACTION_ADD_INQUIRY
				: HCI_SET_EVENT_FILTER_ACTION_CLEAR_INQUIRY)
			: HCI_SET_EVENT_FILTER_ACTION_OTHER)
		: HCI_SET_EVENT_FILTER_ACTION_CLEAR_ALL;
}

/*---------------------------------------------------------------------------
BREDR,
The Bluetooth system operates in the 2.4 GHz ISM band at 2400 - 2483.5 MHz.
RF channels are spaced 1 MHz and are ordered in channel number k.
Regulatory Range RF Channels
2.400-2.4835 GHz f=2402+k MHz, k=0,...,78

The LE system operates in the 2.4 GHz ISM band at 2400 - 2483.5 MHz. 
The LE system uses 40 RF channels. These RF channels have center frequencies 
2402 + k * 2 MHz, where k = 0,...,39.
Regulatory Range RF Channels
2.400-2.4835 GHz f=2402+k*2 MHz, k=0,...,39

LE Advertising and Data Channel Indexes:
RF Channel	RF Center Frequency  Channel Type  Data Channel Index  Advertising Channel Index
0 			2402 MHz 			 Advertising channel 			   37
1 			2404 MHz 			 Data channel  0
2 			2406 MHz 			 Data channel  1
...
11 			2424 MHz 			 Data channel  10
12 			2426 MHz 			 Advertising channel 			   38
13 			2428 MHz 			 Data channel  11
14 			2430 MHz 			 Data channel  12
...
38 			2478 MHz 			 Data channel  36
39 			2480 MHz 			 Advertising channel 			   39

WiFI:
	2412 + k * 5Mhz(k = 0...12), etc, 2412...2472 and 2484
Summary:
	WIFI,	2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484
	BREDR, 	2402,2403,2404...2479,2480
	LE,		2402,2404,2406...2478,2480
---------------------------------------------------------------------------*/
void HCIC_Set_Channel_Classification(struct FsmInst *fi, UINT8 event, struct HCI_Set_Channel_ClassificationStru *par)
{
	struct HCI_Set_AFH_Host_Channel_ClassificationStru *br;
	UINT8 *map = HCI_AFHMAP;/* 2402-2481, 2401, 81bits */
	UINT32 idx;
	UINT32 c = par->c;
	UINT32 w = par->w;

	c -= w >> 1;/* Min */
	w += c;/* Max */
	while (c <= w) {/* From Min to Max */

		/* Bit index in map[10], 80bits */
		if ((c > 2402 - 1) && (c < 2480 + 1)) {/* c in range of 2401 to 2481 */
			idx = c - 2401;
			if (idx == 0) {/* 2401 patch for bit80(last) */
				idx = 80;
			} else {
				idx--;/* 2402 for bit0, 2481 for bit79 */
			}
			if (par->mask & HCI_SETAFH_OCCUPY) {/* Occupy */
				HCI_SET_OFF(map, idx);/* 0 for bad */
			} else {/* Release */
				HCI_SET_BIT(map, idx);/* 1 for unknown */
			}
		}
		c++;/* 1MHZ */
	}

	if (par->mask & HCI_SETAFH_SENDCMD) {
		br = NEW(sizeof(struct HCI_Set_AFH_Host_Channel_ClassificationStru));
		memcpy(br->afh_host_channel_classification, map, 10);/* 0~78 */
		HCI_SET_OFF(br->afh_host_channel_classification, 79);/* The most significant bit is reserved and shall be set to 0 */
		HCI_Command_Send(NULL, br, HCI_OPS_SET_AFH_HOST_CHANNEL_CLASSIFICATION);/* HCI_OPS_WRITE_AFH_CHANNEL_ASSESSMENT_MODE was sent when controller init */

#ifdef CONFIG_LE/* FIXME, only master */
		{
			struct HCI_LE_Set_Host_Channel_ClassificationStru *le;

			le = NEW(sizeof(struct HCI_LE_Set_Host_Channel_ClassificationStru));
			memset(le->channel_map, 0, 5);/* 0 ~ 36 */

			/* Link Layer 0-36 */
			c = 0;
			do {
				if (c <= 10) {
					/* Data Channel 0-10 */
					/* 2403, 2404, 2405, Data Channel 0... */
					/* 2423, 2424, 2425, Data Channel 10 */
					w = (c << 1) + 2;
				} else {
					/* Data Channel 11-36 */
					/* 2427, 2428, 2429, Data Channel 11... */
					/* 2477, 2478, 2479, Data Channel 36 */
					w = (c << 1) + 4;
				}
				
				if (HCI_CHECK_BIT(map, w - 1) && HCI_CHECK_BIT(map, w) && HCI_CHECK_BIT(map, w + 1)) {
					HCI_SET_BIT(le->channel_map, c);
				}
				c++;
			} while (c <= 36);
			HCI_Command_Send(NULL, le, HCI_OPS_LE_SET_HOST_CHANNEL_CLASSIFICATION);
		}
#endif
	}

	FREE(par);

#ifdef HCI_DBG_ALL
	HCI_DBG_SetAFH();
#endif
}

