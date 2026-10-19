/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    srvdb.c
Abstract:
	This module takes responsibility for the task related to the service database 
	maintained by the SDP server.
Author:
    D.X. Ye
Revision History:
2001.3
	changes of structure ServiceAttribute_I,etc.
	CONFIG_SDP_EXT_API for GetNextBEService.
2002.10
	Changed the format of service record database, support both fixed and dynamic database.
	Changed all the original functions defined in this file.
2009.11.12	by YDX
    (CR146) RegisterService: Fix mask and index error.
---------------------------------------------------------------------------*/
#include "g_sdp.h"
#include "sdphlp.h"

#ifndef CONFIG_SDP_FIXED_DATABASE
const char sdp_cls_id[] = {	0x19, 0x10,	0x00,
							0x19, 0x00,	0x01 };
const char sdp_ver[] = { 0x09, 0x01, 0x01 };
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    InitDatabase() initialize the database structures, including service handle list,
	uuid list.
Arguments:
	user_data	pointer to the user data of the SDP FSM instance.  
Return Value:
    none.
---------------------------------------------------------------------------*/
void InitDatabase(struct CtrlUserData *user_data, DWORD state)
{
#ifndef CONFIG_SDP_FIXED_DATABASE
	struct HandleItem *sdp_handle;
	struct AttrItem *sdp_attr, *p_attr;
	struct UUIDItem *first_uuid;
	struct DataElement *de;
	WORD uuid = GRP_ROOT_ID;
#endif

	DBG_FUNC("InitDatabase")
	DBG_ENTER(PROT_SDP);

#ifndef CONFIG_SDP_FIXED_DATABASE
	user_data->hhead.head = NULL;
	user_data->hhead.tail = NULL;
	user_data->uhead.head = NULL;
	user_data->uhead.tail = NULL;

	first_uuid = List_NodeNew(sizeof(struct UUIDItem));
	first_uuid->uuid = (struct SDP_UUID *)NEW(sizeof(struct SDP_UUID));
	first_uuid->uuid->mask = UUID_16;
	memcpy(first_uuid->uuid->var, &uuid, 2);
	List_AddTail(&user_data->uhead, first_uuid);

	sdp_handle = List_NodeNew(sizeof(struct HandleItem));
	List_AddTail(&user_data->hhead, sdp_handle);

	sdp_attr = NewAttrItem(SRV_RECORD_HANDLE, 0x0A, 4, NULL);
	sdp_handle->first_attr = sdp_attr;
	p_attr = sdp_attr;

	sdp_attr = NewAttrItem(SRV_CLSID_LIST, 0x35, sizeof(sdp_cls_id), sdp_cls_id);
	p_attr->next_item = sdp_attr;
	p_attr = sdp_attr;
	de = MakeDEStru(sdp_attr->attr.attr_buf, 0);
	AddUUIDs(sdp_handle, de);
	FREE(de);

	sdp_attr = NewAttrItem(VER_NUM_LIST, 0x35, sizeof(sdp_ver), sdp_ver);
	p_attr -> next_item = sdp_attr;
	p_attr = sdp_attr;
	de = MakeDEStru(sdp_attr->attr.attr_buf, 0);
	AddUUIDs(sdp_handle, de);
	FREE(de);

	sdp_attr = NewAttrItem(SRV_DB_STATE, 10, 4, (const char *)&state);
	p_attr->next_item = sdp_attr;
	
	user_data->state = state;
	user_data->state_link = sdp_attr;
#endif

	DBG_LEAVE(PROT_SDP);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    RegisterService() registers a service with one of its attributes, or modifies
	the attribute's value. 
Arguments:
	reg		pointer to a struct ServiceRegStru type variable containing the attribute 
			value to be registered.  
Return Value:
    BT_SUCCESS if succeed to add or modify the attribute value.
    Otherwise, error code.
---------------------------------------------------------------------------*/
WORD RegisterService(struct ServiceRegStru *reg)
{
	struct HandleItem *tmp_hdl;
#ifndef CONFIG_SDP_FIXED_DATABASE
	struct AttrItem *cur_attr, *new_attr, *pre_attr = NULL;
	WORD flag = 1;
#endif
	WORD err = BT_SUCCESS;

	DBG_FUNC("RegisterService")
	DBG_ENTER(PROT_SDP);

	ENTER_CRITICAL(g_localinfo_lock);
	tmp_hdl = FindHandleItem(reg->handle);
#ifdef CONFIG_SDP_FIXED_DATABASE
	if (tmp_hdl == NULL) {
		/*It doesn't support registering a new service record in a fixed database version.*/
		err = ERROR_SDP_INVALID_SERVICEHANDLE;
	} else {
		/*Only ServiceAvailabilityattribute value can be modified in a fixed database version*/
		if (reg->attr->attribute_id == SRV_AVAILABILITY) 
			tmp_hdl->status = tmp_hdl->status & 0xFF00 | reg->attr->attribute_value.data[0];
		else
			err = ERROR_SDP_INVALID_OPERATION;
	}
#else
	if (tmp_hdl == NULL) {
		tmp_hdl = List_NodeNew(sizeof(struct HandleItem));
		tmp_hdl->uuid_count = 0;
		Encode((UCHAR*)&tmp_hdl->handle, (UCHAR*)&reg->handle, INT_32_MASK);
		tmp_hdl->first_attr = NewAttrItem(SRV_RECORD_HANDLE, 0x0A, 4, (const char *)&tmp_hdl->handle);
		tmp_hdl->handle = reg->handle;
		if (reg->attr->attribute_id != SRV_RECORD_HANDLE)
			tmp_hdl->first_attr->next_item = NewAttrItem(reg->attr->attribute_id, 
													reg->attr->attribute_value.descriptor,
													reg->attr->attribute_value.size, 
													(const char *)&(reg->attr->attribute_value.data));    
		List_AddHead(&CONTROL_USER_DATA->hhead, tmp_hdl);
	} else {
		cur_attr = tmp_hdl->first_attr; 
		while(cur_attr != NULL) {
			if (SDP_ATTR_ITEM_ID(cur_attr) == reg->attr->attribute_id) {
				RemoveUUIDs(tmp_hdl, &cur_attr->attr);
				flag = 0;
				break;
			} else
				if (SDP_ATTR_ITEM_ID(cur_attr) > reg->attr->attribute_id)
					break;
			pre_attr = cur_attr;
			cur_attr = cur_attr->next_item;
		}
		if (flag) {
			new_attr = NewAttrItem(reg->attr->attribute_id, 
								 reg->attr->attribute_value.descriptor,
								 reg->attr->attribute_value.size,
								 (const char *)&(reg->attr->attribute_value.data));    
			new_attr->next_item = cur_attr;
			pre_attr->next_item = new_attr;
		} else {
			FREE(cur_attr->attr.attr_buf);
			cur_attr->attr.attr_buf = SetAttrItem(reg->attr->attribute_id, 
										    reg->attr->attribute_value.descriptor,
											reg->attr->attribute_value.size,
											(const char *)&(reg->attr->attribute_value.data));
			cur_attr->attr.length = 3 + DESize(cur_attr->attr.attr_buf + 3, NULL, 1);
		}
	}
	AddUUIDs(tmp_hdl, &reg->attr->attribute_value); 
#endif
	LEAVE_CRITICAL(g_localinfo_lock);

	FREE(reg->attr);
	FREE(reg);

	DBG_LEAVE_SD(err);
}

#ifndef CONFIG_SDP_FIXED_DATABASE
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    UnregisterService() unregisters the service.
Arguments:
	arg		a DWORD type variable containing the handle of the service record to be removed.  
Return Value:
    BT_SUCCESS if succeed to add or modify the attribute value.
    Otherwise, error code.
---------------------------------------------------------------------------*/
WORD UnregisterService(DWORD handle)
{
	struct HandleItem *tmp_hdl;
	struct AttrItem *attr, *tmp_attr;
	WORD err = ERROR_SDP_INVALID_SERVICEHANDLE;

	DBG_FUNC("UnregisterService")
	DBG_ENTER(PROT_SDP);

	if (handle) {
		ENTER_CRITICAL(g_localinfo_lock);
		tmp_hdl = (struct HandleItem *)CONTROL_USER_DATA->hhead.head;
		while(tmp_hdl) {
			if (tmp_hdl->handle == handle) {
				attr = tmp_hdl->first_attr;
				while (attr != NULL) {
					tmp_attr = attr->next_item;
					RemoveUUIDs(tmp_hdl, &attr->attr);
					FREE(attr->attr.attr_buf);  
					FREE(attr);
					attr = tmp_attr;
				}
				SetServiceDatabaseState();
				err = BT_SUCCESS;
				List_RemoveAt(&CONTROL_USER_DATA->hhead, tmp_hdl);
				LFREE(tmp_hdl);
				break;
			}
			tmp_hdl = (struct HandleItem *)LNEXT(tmp_hdl);
		}
		LEAVE_CRITICAL(g_localinfo_lock);
	}

	DBG_LEAVE_SD(err);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    FreeDatabase() release the database.
Arguments:
	user_data	pointer to the user data of the SDP FSM instance.  
Return Value:
    none.
---------------------------------------------------------------------------*/
void FreeDatabase(struct CtrlUserData *user_data)
{
	struct HandleItem *hdl_item;
	struct AttrItem *attr_item, *next_attr;
	struct UUIDItem *uuid_item;
	struct SimpleHandle *simp_hdl_item, *next_simp_hdl;

	DBG_FUNC("FreeDatabase")
	DBG_ENTER(PROT_SDP);

	ENTER_CRITICAL(g_localinfo_lock);
	hdl_item = (struct HandleItem *)user_data->hhead.head;
	while (hdl_item != NULL) {
		attr_item = hdl_item->first_attr;
		while(attr_item != NULL) {
			next_attr = attr_item->next_item;
			FREE(attr_item->attr.attr_buf);  
			FREE(attr_item);
			attr_item = next_attr;
		}
		hdl_item = (struct HandleItem *)LNEXT(hdl_item) ;
	}
	List_RemoveAll(&user_data->hhead);
	
	uuid_item = (struct UUIDItem *)(user_data->uhead.head);
	while (uuid_item != NULL) {
		simp_hdl_item = uuid_item->first_simple_handle;
		while (simp_hdl_item != NULL) {
			next_simp_hdl = simp_hdl_item->next_simple_handle;
			FREE(simp_hdl_item);
			simp_hdl_item = next_simp_hdl;
		}
		uuid_item->first_simple_handle = NULL;
		FREE(uuid_item->uuid); 
		uuid_item = (struct UUIDItem *)LNEXT(uuid_item);
	}
	List_RemoveAll(&user_data->uhead);
	LEAVE_CRITICAL(g_localinfo_lock);
	
	DBG_LEAVE(PROT_SDP);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    AllocateSvcHdl() allocates an available service handle.
Arguments:
	arg		pointer to a DWORD type variable to get the new record handle.  
Return Value:
    BT_SUCCESS if succeed to add or modify the attribute value.
    Otherwise, error code.
History:
	Added by D.X. Ye, 2002/1/22.
---------------------------------------------------------------------------*/
WORD AllocateSvcHdl(DWORD *handle)
{
	static DWORD g_hdl_count = 1;
	DWORD count;
	WORD err = UNKNOWN_ERROR_SDP;
	
	DBG_FUNC("AllocateSvcHdl")
	DBG_ENTER(PROT_SDP);

	ENTER_CRITICAL(g_localinfo_lock);
	count = g_hdl_count;
	do {
		*handle = g_hdl_count + 0x0000ffff;
		g_hdl_count = ((g_hdl_count + 1) > 0xffff0000)
				   ? (g_hdl_count + 1 - 0xffff0000) : (g_hdl_count + 1);
		if (NULL == LIST_FINDLENOFF0(&CONTROL_USER_DATA->hhead, sizeof(DWORD), handle)) {
			err = BT_SUCCESS;
			break;
		}
	} while (count != g_hdl_count);
	LEAVE_CRITICAL(g_localinfo_lock);
	
	if (err != BT_SUCCESS) 
		*handle = 0;
	
	DBG_LEAVE_SD(err);
}

#endif

