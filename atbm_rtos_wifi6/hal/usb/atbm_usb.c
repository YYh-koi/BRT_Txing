/**************************************************************************************************************
 * altobeam RTOS
 *
 * Copyright (c) 2018, altobeam.inc   All rights reserved.
 *
 *  The source code contains proprietary information of AltoBeam, and shall not be distributed, 
 *  copied, reproduced, or disclosed in whole or in part without prior written permission of AltoBeam.
*****************************************************************************************************************/
#include "atbm_hal.h"
#include "../../include/svn_version.h"
#define DPLL_CLOCK 40

//extern atbm_int32 atbm_usb_submit_urb(atbm_urb_s *purb, int param);
extern atbm_void atbm_usb_kill_urb(atbm_urb_s *purb);
extern atbm_void atbm_usb_free_urb(atbm_urb_s *purb);
//extern atbm_urb_s *atbm_usb_alloc_urb(atbm_int32 iso_packets, atbm_int32 mem_flags);
extern atbm_void atbm_core_release(struct atbmwifi_common *hw_priv);
extern int atbm_usb_register_init(atbm_void);
extern int atbm_usb_register_deinit(atbm_void);
static int atbm_usb_memcpy_fromio_async(struct sbus_priv *self,
				     unsigned int addr,
				     atbm_void *dst, int count,sbus_callback_handler func);

extern void frame_hexdump(char *prefix, atbm_uint8 *data, atbm_uint8 len);
extern int atbm_hmac_xmit_data(struct sbus_priv *self);
extern int atbm_hmac_receive_data(struct sbus_priv *self,unsigned int addr,atbm_void  *dst, int count);

struct build_info{
	int ver;
	int dpll;
	char driver_info[64];
};
const char DRIVER_INFO[]={"[===USB-ATHENAB=="};
static int driver_build_info(atbm_void)
{
	struct build_info build;
	build.ver=SVN_VERSION;
	build.dpll=DPLL_CLOCK;
	atbm_memcpy(build.driver_info,(atbm_void*)DRIVER_INFO,sizeof(DRIVER_INFO));
	wifi_printk(WIFI_DBG_ANY,"SVN_VER=%d,DPLL_CLOCK=%d,BUILD_TIME=%s\n",build.ver,build.dpll,build.driver_info);
	wifi_printk(WIFI_DBG_ANY,"BRT SVN_VER=%s\n", BRT_SVN_VERSION);
	return 0;
}
/* sbus_ops implemetation */
int atbm_usbctrl_vendorreq_sync(struct sbus_priv *self, atbm_uint8 request,atbm_uint8 b_write,
					atbm_uint16 value, atbm_uint16 index, atbm_void *pdata,atbm_uint16 len)
{
	unsigned int pipe;
	int status;
	atbm_uint8 reqtype;
	int vendorreq_times = 0;
	struct atbm_usb_device *udev = self->drvobj->pusbdev;
	
	static int count;
	atbm_uint8 * reqdata=self->usb_req_data;
	if (!reqdata){
		wifi_printk(WIFI_DBG_ERROR,"regdata is Null\n");
	}

	if(b_write){
		pipe = atbm_usb_sndctrlpipe(udev, /*0*/atbm_usb_sndctrlpipe(udev, 0)); /* write_out */
		reqtype =  ATBM_USB_VENQT_WRITE;//host to device
		// reqdata must used dma data
		atbm_memcpy(reqdata,pdata,len);
	}
	else {
		pipe = atbm_usb_rcvctrlpipe(udev, /*0*/atbm_usb_rcvctrlpipe(udev, 0)); /* read_in */
		reqtype =  ATBM_USB_VENQT_READ;//device to host
	}
	do {
		status = atbm_usb_control_msg(udev, pipe, request, reqtype, value,
						 index, reqdata, len, 500); /*500 ms. timeout*/
		if (status < 0) {
			wifi_printk(WIFI_DBG_ERROR, "%s:err(%d)addr[%x] len[%d],b_write %d request %d\n",__FUNCTION__,status,value|(index<<16),len,b_write, request);
		} else if(status != len) {
			wifi_printk(WIFI_DBG_ERROR,"%s:len err(%d)\n",__FUNCTION__,status);
		}
		else{
			break;
		}
	} while (++vendorreq_times < 3);

	if((b_write==0) && (status>0)){
		atbm_memcpy(pdata,reqdata,len);
	}
	if (status < 0 && count++ < 4)
		wifi_printk(WIFI_DBG_ERROR,"reg 0x%x, usbctrl_vendorreq TimeOut! status:0x%x value=0x%x\n",
		       value, status, *(atbm_uint32 *)pdata);
	return status;
}
#define HW_DOWN_FW

#ifdef HW_DOWN_FW
static int atbm_usb_hw_read_port(struct sbus_priv *self, atbm_uint32 addr, atbm_void *pdata,int len)
{
	int ret = 0;
	atbm_uint8 request = VENDOR_HW_READ; //HW
	atbm_uint16 wvalue = (atbm_uint16)(addr & 0x0000ffff);
	atbm_uint16 index = addr >> 16;

	wifi_printk(WIFI_ALWAYS,"atbm_usb_hw_read_port addr %x,len %x\n",addr,len);

	//hardware just support len=4
	ATBM_WARN_ON_FUNC((len != 4) && (request== VENDOR_HW_READ));
	ret = atbm_usbctrl_vendorreq_sync(self,request,0,wvalue, index, pdata,len);
	if (ret < 0)
	{
		wifi_printk(WIFI_DBG_ERROR, "ERR read addr %x,len %x\n",addr,len);
	}
	return ret;
}

static int atbm_usb_hw_write_port(struct sbus_priv *self, atbm_uint32 addr, const atbm_void *pdata,int len)
{

	atbm_uint8 request = VENDOR_HW_WRITE; //HW
	atbm_uint16 wvalue = (atbm_uint16)(addr & 0x0000ffff);
	atbm_uint16 index = addr >> 16;
	int ret =0;

	atbm_usb_pm(self,0);

	//printk(KERN_ERR "%s:addr(%x)\n",__func__,addr);
	//hardware just support len=4
	//WARN_ON((len != 4) && (request== VENDOR_HW_WRITE));
	ret =  atbm_usbctrl_vendorreq_sync(self,request,1,wvalue, index, (atbm_void *)pdata,len);
	if (ret < 0)
	{
		wifi_printk(WIFI_DBG_ERROR, "ERR write addr %x,len %x\n",addr,len);
	}
	atbm_usb_pm(self,1);
	return ret;
}


#else
static int atbm_usb_sw_read_port(struct sbus_priv *self, atbm_uint32 addr, atbm_void *pdata,int len)
{
	atbm_uint8 request = VENDOR_SW_READ; //SW
	atbm_uint16 wvalue = (atbm_uint16)(addr & 0x0000ffff);
	atbm_uint16 index = addr >> 16;

	ATBM_WARN_ON_FUNC(len > ATBM_USB_EP0_MAX_SIZE);
	return atbm_usbctrl_vendorreq_sync(self,request,0,wvalue, index, pdata,len);
}
//#ifndef HW_DOWN_FW
static int atbm_usb_sw_write_port(struct sbus_priv *self, atbm_uint32 addr,const atbm_void *pdata,int len)
{
	atbm_uint8 request = VENDOR_SW_WRITE; //SW
	atbm_uint16 wvalue = (atbm_uint16)(addr & 0x0000ffff);
	atbm_uint16 index = addr >> 16;

	ATBM_WARN_ON_FUNC(len > ATBM_USB_EP0_MAX_SIZE);
	return atbm_usbctrl_vendorreq_sync(self,request,1,wvalue, index, (atbm_void *)pdata,len);
}
#endif

int atbm_lmac_start(struct sbus_priv *self)
{
	//pTargetUsb->loadFirmwareSuccess;
	//atbm_uint8 request = VENDOR_SW_CPU_JUMP;
	//static int tmpdata =0;
	//return atbm_usbctrl_vendorreq_sync(self,request,1,0, 0, &tmpdata,0);
	//TargetUsb_lmac_start();
	return 1;
}

int atbm_usb_ep0_cmd(struct sbus_priv *self)
{
	atbm_uint8 request = VENDOR_EP0_CMD; //SW
	
	static int tmpdata =0;
	return atbm_usbctrl_vendorreq_sync(self,request,1,0, 0, &tmpdata,0);
}
/*
wvalue=1 : open uart debug;
 wvalue=0 : close uart debug;
 */
int atbm_usb_debug_config(struct sbus_priv *self,atbm_uint16 wvalue)
{
	atbm_uint8 request = 0x6;
	atbm_uint16 index = 0;

	wifi_printk(WIFI_DBG_ERROR, "atbm_usb_debug_config\n");

	return atbm_usbctrl_vendorreq_sync(self,request,1,wvalue, index, &wvalue,0);
}

void atbm_usb_kill_all_txurb(struct sbus_priv *self)
{
	int i=0;
	for(i=0;i<TX_DESC_NUM;i++){
		atbm_usb_kill_urb(self->tx_desc[i].test_desc);
	}

}
void atbm_usb_kill_all_rxurb(struct sbus_priv *self)
{
	int i=0;
	for(i=0;i<RX_DESC_NUM;i++){
		atbm_usb_kill_urb(self->tx_desc[i].test_desc);
	}

}

atbm_void atbm_usb_urb_free(struct sbus_priv *self,struct sbus_desc *pUrb,int max_num)
{
	int i=0;

	for(i=0;i<max_num;i++){
		if(pUrb[i].link ==1){
			atbm_usb_kill_urb(pUrb[i].test_desc);
		}
		atbm_usb_free_urb(pUrb[i].test_desc);
		atbm_dev_kfree_skb(pUrb[i].test_skb);
		pUrb[i].test_skb =ATBM_NULL;
		pUrb[i].test_desc =ATBM_NULL;
	}
}

int atbm_usb_urb_malloc(struct sbus_priv *self,struct sbus_desc *pUrb,int max_num,int len)
{
	int i=0;
	for(i=0;i<max_num;i++){
		pUrb[i].test_desc=atbm_usb_alloc_urb(0,0);
		if (!pUrb[i].test_desc){
			wifi_printk(WIFI_DBG_ERROR, "Can't allocate test_urb.");
			goto __free_urb;
		}
		pUrb[i].test_skb = ATBM_NULL;
		pUrb[i].desc_id = i;
		pUrb[i].obj =self;
		
		pUrb[i].link =0;
	}
	return 0;
__free_urb:
	for( ;i>=0;--i){
		atbm_usb_free_urb(pUrb[i].test_desc);
	}

	return -ATBM_ENOMEM;
}

int atbm_usb_urb_get(struct sbus_priv *self,atbm_uint32 *bitmap,int max_urb)
{
	int id = 0;
	unsigned long flags=0;
	
	atbm_spin_lock_irqsave(&self->lock, &flags);
	id= atbm_find_first_zero_bit(bitmap,max_urb);
	if((id>=max_urb)||(id<0)){
		atbm_spin_unlock_irqrestore(&self->lock, flags);
		return -1;
	}
	atbm_set_bit(id,bitmap);   
	atbm_spin_unlock_irqrestore(&self->lock, flags);
	return id;
}

int atbm_usb_urb_get_irqen(struct sbus_priv *self,atbm_uint32 *bitmap,int max_urb)
{
	int id = 0;

	atbm_spin_lock(&self->lock);
	id= atbm_find_first_zero_bit(bitmap,max_urb);
	if((id>=max_urb)||(id<0)){
		atbm_spin_unlock(&self->lock);
		return -1;
	}
	atbm_set_bit(id,bitmap);
	atbm_spin_unlock(&self->lock);
	return id;
}

atbm_void  atbm_usb_urb_put_irqen(struct sbus_priv *self,atbm_uint32 *bitmap,int id)
{
	atbm_spin_lock(&self->lock);
	*bitmap &= ~ATBM_BIT(id);
	atbm_spin_unlock(&self->lock);
}

atbm_void  atbm_usb_urb_put(struct sbus_priv *self,atbm_uint32 *bitmap,int id)
{
	unsigned long flags=0;
	atbm_spin_lock_irqsave(&self->lock, &flags);
	*bitmap &= ~ATBM_BIT(id);
	atbm_spin_unlock_irqrestore(&self->lock, flags);
}

void atbm_build_wsm_header_v0(struct atbmwifi_common *hw_priv, struct wsm_hdr_tx *wsm_tx, int *tx_len){
	wsm_tx->enc_hdr.total_len = wsm_tx->u.common.total_len;
	wsm_tx->enc_hdr.flag = wsm_tx->u.common.flag;
	wsm_tx->u.usb_hdr.usb_len = atbm_cpu_to_le16(wsm_tx->u.common.total_len);
	if (wsm_tx->u.usb_hdr.usb_len <1538)
		wsm_tx->u.usb_hdr.usb_len =atbm_cpu_to_le16(1538);
	*tx_len = atbm_le16_to_cpu(wsm_tx->u.usb_hdr.usb_len);
	wsm_tx->u.usb_hdr.flag = atbm_cpu_to_le16(0xe569)<<16;
	wsm_tx->u.usb_hdr.flag |= atbm_cpu_to_le16(ATBM_BIT(6));
	wsm_tx->u.usb_hdr.flag |= atbm_cpu_to_le16((hw_priv->wsm_tx_seq & 0x1f));
	wsm_tx->u.usb_hdr.usb_id = wsm_tx->id;
}

static int atbm_usb_memcpy_fromio(struct sbus_priv *self,
				     unsigned int addr,
				     atbm_void *dst, int count)
{
	int i=0;
	if(atbm_atomic_add_return(0, &self->rx_lock)){
		return -1;
	}
	for(i=0;i<RX_DESC_NUM;i++){
	 	atbm_hmac_receive_data(self,addr,dst,count);
	}
	return 0;
}

static int atbm_usb_memcpy_toio(struct sbus_priv *self,
				   unsigned int addr,
				   const atbm_void  *src, int count)
{
	return atbm_hmac_xmit_data(self);
}

static int atbm_usb_memcpy_fromio_async(struct sbus_priv *self,
				     unsigned int addr,
				     atbm_void  *dst, int count,sbus_callback_handler func)
{

	 return atbm_hmac_receive_data(self,addr,dst,count);
}

static int atbm_usb_memcpy_toio_async(struct sbus_priv *self,
				   unsigned int addr,
				   const atbm_void *src, int count)
{
	 atbm_hmac_xmit_data(self);
	 return 0;
}

static atbm_void atbm_usb_lock(struct sbus_priv *self)
{
	//atbm_os_mutexLock(&self->sbus_mutex,0);

}

static atbm_void atbm_usb_unlock(struct sbus_priv *self)
{
	//atbm_os_mutexUnLock(&self->sbus_mutex);
}


static int atbm_usb_reset(struct sbus_priv *self)
{
	atbm_uint32 regdata = 1;
	wifi_printk(WIFI_IF," %s\n",__FUNCTION__);
	atbm_usb_hw_write_port(self,0x16100074,&regdata,4);
	return 0;
}

static atbm_uint32 atbm_usb_align_size(struct sbus_priv *self, atbm_uint32 size)
{
	atbm_size_t aligned = size;
	return aligned;
}

int atbm_usb_set_block_size(struct sbus_priv *self, int type, atbm_uint32 size)
{
	return 0;
}

int atbm_usb_pm(struct sbus_priv *self, ATBM_BOOL  auto_suspend)
{
	int ret = 0;
	self->auto_suspend  = auto_suspend;

	if(auto_suspend){
	///TODO suspend
	}
	return ret;
}
int atbm_usb_pm_async(struct sbus_priv *self, ATBM_BOOL  auto_suspend)
{
	int ret = 0;

	self->auto_suspend  = auto_suspend;

	if(auto_suspend){
		///TODO suspend
	}
	else {
		///TODO suspend
	}
	return ret;
}
static int atbm_usb_irq_subscribe(struct sbus_priv *self, sbus_irq_handler handler,atbm_void *priv)
{
	int ret = 0;
	return ret;
}
static int atbm_usb_irq_unsubscribe(struct sbus_priv *self)
{
	int ret = 0;
	return ret;
}
static struct dvobj_priv *usb_dvobj_init(struct atbm_usb_interface *usb_intf)
{
	//int	i;
	struct dvobj_priv *pdvobjpriv=ATBM_NULL;
	//struct atbm_usb_device_descriptor 	*pdev_desc;
	//struct atbm_usb_host_config			*phost_conf;
	//struct atbm_usb_config_descriptor		*pconf_desc;
	//struct atbm_usb_host_interface		*phost_iface;
	//struct atbm_usb_interface_descriptor	*piface_desc;
	//struct atbm_usb_host_endpoint		*phost_endp;
	//struct atbm_usb_endpoint_descriptor	*pendp_desc;
//	struct atbm_usb_device				*pusbd;
	
	pdvobjpriv = atbm_kzalloc(sizeof(*pdvobjpriv),GFP_KERNEL);
	if (!pdvobjpriv){
		wifi_printk(WIFI_IF, "Can't allocate USB dvobj.");
		goto exit;
	}
	pdvobjpriv->pusbintf = usb_intf ;
	pdvobjpriv->pusbdev = atbm_interface_to_usbdev(usb_intf);
	atbm_usb_set_intfdata(usb_intf, pdvobjpriv);
	
#if 0

	//pdvobjpriv->RtNumInPipes = 0;
	//pdvobjpriv->RtNumOutPipes = 0;
	phost_iface = &usb_intf->altsetting[0];
	piface_desc = &phost_iface->desc;
	pdev_desc = &pusbd->descriptor;
	wifi_printk(WIFI_IF,"\natbm_usb_device_descriptor:\n");
	wifi_printk(WIFI_IF,"bLength=%x\n", pdev_desc->bLength);
	wifi_printk(WIFI_IF,"bDescriptorType=%x\n", pdev_desc->bDescriptorType);
	wifi_printk(WIFI_IF,"bcdUSB=%x\n", pdev_desc->bcdUSB);
	wifi_printk(WIFI_IF,"bDeviceClass=%x\n", pdev_desc->bDeviceClass);
	wifi_printk(WIFI_IF,"bDeviceSubClass=%x\n", pdev_desc->bDeviceSubClass);
	wifi_printk(WIFI_IF,"bDeviceProtocol=%x\n", pdev_desc->bDeviceProtocol);
	wifi_printk(WIFI_IF,"bMaxPacketSize0=%x\n", pdev_desc->bMaxPacketSize0);
	wifi_printk(WIFI_IF,"idVendor=%x\n", pdev_desc->idVendor);
	wifi_printk(WIFI_IF,"idProduct=%x\n", pdev_desc->idProduct);
	wifi_printk(WIFI_IF,"bcdDevice=%x\n", pdev_desc->bcdDevice);
	wifi_printk(WIFI_IF,"iManufacturer=%x\n", pdev_desc->iManufacturer);
	wifi_printk(WIFI_IF,"iProduct=%x\n", pdev_desc->iProduct);
	wifi_printk(WIFI_IF,"iSerialNumber=%x\n", pdev_desc->iSerialNumber);
	wifi_printk(WIFI_IF,"bNumConfigurations=%x\n", pdev_desc->bNumConfigurations);

	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;
	wifi_printk(WIFI_IF,"\natbm_usb_configuration_descriptor:\n");
	wifi_printk(WIFI_IF,"bLength=%x\n", pconf_desc->bLength);
	wifi_printk(WIFI_IF,"bDescriptorType=%x\n", pconf_desc->bDescriptorType);
	wifi_printk(WIFI_IF,"wTotalLength=%x\n", pconf_desc->wTotalLength);
	wifi_printk(WIFI_IF,"bNumInterfaces=%x\n", pconf_desc->bNumInterfaces);
	wifi_printk(WIFI_IF,"bConfigurationValue=%x\n", pconf_desc->bConfigurationValue);
	wifi_printk(WIFI_IF,"iConfiguration=%x\n", pconf_desc->iConfiguration);
	wifi_printk(WIFI_IF,"bmAttributes=%x\n", pconf_desc->bmAttributes);
	wifi_printk(WIFI_IF,"bMaxPower=%x\n", pconf_desc->bMaxPower);

	phost_iface = &usb_intf->altsetting[0];
	wifi_printk(WIFI_IF,"\natbm_usb_interface_descriptor:\n");
	wifi_printk(WIFI_IF,"bLength=%x\n", piface_desc->bLength);
	wifi_printk(WIFI_IF,"bDescriptorType=%x\n", piface_desc->bDescriptorType);
	wifi_printk(WIFI_IF,"bInterfaceNumber=%x\n", piface_desc->bInterfaceNumber);
	wifi_printk(WIFI_IF,"bAlternateSetting=%x\n", piface_desc->bAlternateSetting);
	wifi_printk(WIFI_IF,"bNumEndpoints=%x\n", piface_desc->bNumEndpoints);
	wifi_printk(WIFI_IF,"bInterfaceClass=%x\n", piface_desc->bInterfaceClass);
	wifi_printk(WIFI_IF,"bInterfaceSubClass=%x\n", piface_desc->bInterfaceSubClass);
	wifi_printk(WIFI_IF,"bInterfaceProtocol=%x\n", piface_desc->bInterfaceProtocol);
	wifi_printk(WIFI_IF,"iInterface=%x\n", piface_desc->iInterface);

	//pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	//pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;


	for (i = 0; i < pdvobjpriv->nr_endpoint; i++)
	{
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp)
		{
			pendp_desc = &phost_endp->desc;

			wifi_printk(WIFI_IF,"\nusb_endpoint_descriptor(%d):\n", i);
			wifi_printk(WIFI_IF,"bLength=%x\n",pendp_desc->bLength);
			wifi_printk(WIFI_IF,"bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			wifi_printk(WIFI_IF,"bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			wifi_printk(WIFI_IF,"wMaxPacketSize=%d\n",atbm_le16_to_cpu(pendp_desc->wMaxPacketSize));
			wifi_printk(WIFI_IF,"bInterval=%x\n",pendp_desc->bInterval);

			if (atbm_usb_endpoint_is_bulk_in(pendp_desc))
			{
				wifi_printk(WIFI_IF,"usb_endpoint_is_bulk_in = %x\n",atbm_usb_endpoint_num(pendp_desc));
				pdvobjpriv->ep_in_size=atbm_le16_to_cpu(pendp_desc->wMaxPacketSize);
				pdvobjpriv->ep_in=atbm_usb_endpoint_num(pendp_desc);
			}
			else if (atbm_usb_endpoint_is_bulk_out(pendp_desc))
			{
				wifi_printk(WIFI_IF,"usb_endpoint_is_bulk_out = %x\n",atbm_usb_endpoint_num(pendp_desc));
				pdvobjpriv->ep_out_size=atbm_le16_to_cpu(pendp_desc->wMaxPacketSize);
				pdvobjpriv->ep_out=atbm_usb_endpoint_num(pendp_desc);
			}
			pdvobjpriv->ep_num[i] =atbm_usb_endpoint_num(pendp_desc);
		}
	}
	atbm_usb_get_dev(pusbd);
#endif
	//pdvobjpriv->ep_in = 1;
	//pdvobjpriv->ep_out = 2;
	wifi_printk(WIFI_IF,"nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->ep_in, pdvobjpriv->ep_out);
exit:
	return pdvobjpriv;
}

int atbm_usb_probe(struct atbm_usb_interface *intf,
				   const struct atbm_usb_device_id *id)
{
	struct sbus_priv *self;
	struct dvobj_priv *dvobj;
    struct atbm_usb_device *udev = atbm_interface_to_usbdev(intf);
	int status;

	wifi_printk(WIFI_IF, "Probe called\n");

	self = atbm_kzalloc(sizeof(*self),GFP_KERNEL);
	if (!self) {
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate USB sbus_priv.");
		return -ATBM_ENOMEM;
	}
	////TODO RTOS/////////////
	atbm_os_mutexLockInit(&self->sbus_mutex);
	atbm_spin_lock_init(&self->lock);
	/* 1--- Initialize dvobj_priv */
	dvobj = usb_dvobj_init(intf);
	if (!dvobj){
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate USB dvobj.");
		return -ATBM_ENOMEM;
	}
    dvobj->pusbdev = atbm_usb_get_dev(udev);
	wifi_printk(WIFI_ALWAYS,"pusbdev =0x%x",dvobj->pusbdev);
    dvobj->pusbintf = atbm_usb_get_intf(intf);
	dvobj->self =self;
	self->drvobj=dvobj;
	/*2---alloc rx_urb*/
	dvobj->suspend_skb = atbm_dev_alloc_skb(RX_BUFFER_SIZE);
	ATBM_BUG_ON(dvobj->suspend_skb == ATBM_NULL);
	//atbm_skb_reserve(self->drvobj->suspend_skb, 64);
	dvobj->suspend_skb_len = 0;
	status = atbm_usb_urb_malloc(self,self->rx_desc,RX_DESC_NUM,RX_BUFFER_SIZE);
	if (status != 0){
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate rx_urb.");
		return status;
	}
	atbm_memset(dvobj->rx_urb_map,0,sizeof(dvobj->rx_urb_map));
	/*3---alloc tx_urb*/
	status = atbm_usb_urb_malloc(self,self->tx_desc,TX_DESC_NUM,TX_BUFFER_SIZE);
	if (status){
		wifi_printk(WIFI_DBG_ERROR, "Can't allocate tx_urb.");
		return -ATBM_ENOMEM;
	}

	atbm_memset(dvobj->tx_urb_map,0,sizeof(dvobj->tx_urb_map));
	/*5---alloc rx data buffer*/
	self->usb_data = atbm_kzalloc(ATBM_USB_EP0_MAX_SIZE+16,GFP_KERNEL);
	if (!self->usb_data)
		return -ATBM_ENOMEM;
	//self->rx_skb = NULL;
	self->usb_req_data = (atbm_uint8 *)ATBM_ALIGN((unsigned long)self->usb_data,4);

	self->tx_seqnum = 0;
	self->rx_seqnum = 0;
	self->drvobj->tx_test_seq_need =0;
	self->drvobj->tx_test_hwseq_need =0;
	//self->tx_callback_handler = NULL;
	//self->rx_callback_handler = NULL;
	atbm_atomic_xchg(&self->tx_lock, 0);
	atbm_atomic_xchg(&self->rx_lock, 0);

	//usb auto-suspend init
	self->suspend=0;	
	self->auto_suspend=0;	
/////////////////////////////////////////////////////////////////////////////How to do Rtos
	//self->drvobj->pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user
	//dvobj->pusbdev->do_remote_wakeup=1;
	//dvobj->pusbintf->needs_remote_wakeup = 1;
	////////USB Device suspend interface////////////
	//device_init_wakeup(&dvobj->pusbintf->dev,1);
	//pm_runtime_set_autosuspend_delay(&dvobj->pusbdev->dev,15000);
	//atbm_INIT_WORK(&dvobj->usbSuspendWork,putUsbSuspend);
	////////////////////////////////////////////////////
	//Entry atbmwifi common wifi
	status=Atbmwifi_halEntry(self);
	if (status!=0){
		wifi_printk(WIFI_DBG_ERROR,"<ERROR> %s %d\n",__FUNCTION__,__LINE__);
		atbm_usb_urb_free(self,self->rx_desc,RX_DESC_NUM);
		atbm_usb_urb_free(self,self->tx_desc,TX_DESC_NUM);
		if(self->drvobj->suspend_skb)
		{
			atbm_dev_kfree_skb(self->drvobj->suspend_skb);
			self->drvobj->suspend_skb_len = 0;
			self->drvobj->suspend_skb = ATBM_NULL;
		}
		if(dvobj->pusbdev)
			atbm_usb_put_dev(dvobj->pusbdev);
		atbm_usb_set_intfdata(intf, ATBM_NULL);
		atbm_kfree(dvobj);
		atbm_kfree(self);
	}
	return status;
}
extern struct atbmwifi_common g_hw_prv;
extern atbm_void atbm_unregister_wpa_event(atbm_void);
//extern atbm_void  atbmwifi_netstack_deinit(atbm_void);

//extern atbm_void atbm_unregister_eloop(atbm_void);

atbm_void atbm_usb_disconnect(struct atbm_usb_interface *intf)
{
	wifi_printk(WIFI_ALWAYS,"atbm_usb_disconnect \n");
	struct dvobj_priv *dvobj = atbm_usb_get_intfdata(intf);
	struct sbus_priv *self  = ATBM_NULL;
	struct atbm_usb_device *pdev=ATBM_NULL;
	if (dvobj) {
		self  = dvobj->self;
		pdev = dvobj->pusbdev;
		if (self->core) {
			atbm_core_release(self->core);
			self->core = ATBM_NULL;
		}
		self->suspend=1;
		atbm_usb_urb_free(self,self->rx_desc,RX_DESC_NUM);
		atbm_usb_urb_free(self,self->tx_desc,TX_DESC_NUM);
		if(self->drvobj->suspend_skb)
		{
			atbm_dev_kfree_skb(self->drvobj->suspend_skb);
			self->drvobj->suspend_skb = ATBM_NULL;
			self->drvobj->suspend_skb_len = 0;
		}
		atbm_os_DeleteMutex(&self->sbus_mutex);
		if(pdev)
		{   
		    ATBM_BUG_ON((pdev!=atbm_interface_to_usbdev(intf)));
			wifi_printk(WIFI_DBG_ERROR,"we have get dev,so put it in the end\n");
			atbm_usb_put_dev(pdev);
		}
		atbm_usb_set_intfdata(intf, ATBM_NULL);
		atbm_kfree(self->usb_data);
		atbm_kfree(self);
		atbm_kfree(dvobj);	
		wifi_printk(WIFI_IF,"atbm_usb_disconnect---->oK\n");
	}
}
	
int __atbm_usb_suspend(struct sbus_priv *self)
{
	wifi_printk(WIFI_ALWAYS,"***********func=%s,line=%d\n",__func__,__LINE__);
	self->suspend=1;
	return 0;
}

int __atbm_usb_resume(struct sbus_priv *self)
{
	wifi_printk(WIFI_ALWAYS, "===----===start resume state\n");
	self->suspend=0;
	atbm_usb_memcpy_fromio(self,0,NULL,RX_BUFFER_SIZE);
	return 0;
	
}

static int atbm_usb_suspend(struct atbm_usb_interface *intf)
{
	struct dvobj_priv *dvobj = atbm_usb_get_intfdata(intf);
	wifi_printk(WIFI_ALWAYS,"***********func=%s,line=%d\n",__func__,__LINE__);
	dvobj->self->suspend=1;
	//atbm_usb_suspend_start(dvobj->self);
	//msleep(20);
	//atbm_usb_urb_kill(dvobj->self,dvobj->rx_urb,RX_URB_NUM);
	return 0;

}

static int atbm_usb_resume(struct atbm_usb_interface *intf)
{
	struct dvobj_priv *dvobj = atbm_usb_get_intfdata(intf);
	wifi_printk(WIFI_ALWAYS,"===----===start resume state\n");
	dvobj->self->suspend=0;
	atbm_usb_memcpy_fromio(dvobj->self,0,NULL,RX_BUFFER_SIZE);
	return 0;
	
}
struct sbus_ops atbm_usb_sbus_ops;

static int  atbm_usb_init(atbm_void)
{
	{
	atbm_usb_sbus_ops.sbus_memcpy_fromio	= atbm_usb_memcpy_fromio;
	atbm_usb_sbus_ops.sbus_memcpy_toio	= atbm_usb_memcpy_toio;
	atbm_usb_sbus_ops.sbus_read_sync		= atbm_usb_hw_read_port;
	atbm_usb_sbus_ops.sbus_write_sync	= atbm_usb_hw_write_port;
	atbm_usb_sbus_ops.sbus_write_async	= atbm_usb_memcpy_toio_async;
	atbm_usb_sbus_ops.lock				= atbm_usb_lock;
	atbm_usb_sbus_ops.unlock				= atbm_usb_unlock;
	atbm_usb_sbus_ops.reset				= atbm_usb_reset;
	atbm_usb_sbus_ops.align_size			= atbm_usb_align_size;
	atbm_usb_sbus_ops.power_mgmt			= atbm_usb_pm;
	atbm_usb_sbus_ops.set_block_size		= atbm_usb_set_block_size;
	//atbm_usb_sbus_ops.wtd_wakeup			= atbm_wtd_wakeup;
	#ifdef ATBM_USB_RESET
	atbm_usb_sbus_ops.usb_reset			= atbm_usb_reset;
	#else	
	//atbm_usb_sbus_ops.usb_reset			= NULL;
	#endif
	atbm_usb_sbus_ops.bootloader_debug_config = atbm_usb_debug_config;
	atbm_usb_sbus_ops.lmac_start			=atbm_lmac_start;
	atbm_usb_sbus_ops.irq_unsubscribe	= atbm_usb_irq_unsubscribe;
	atbm_usb_sbus_ops.irq_subscribe	= atbm_usb_irq_subscribe;
	};

	driver_build_info();
	wifi_printk(WIFI_IF, "atbm_usb_register\n");

	return atbm_usb_register_init();
}
static atbm_void  atbm_usb_exit(atbm_void)
{
	atbm_usb_register_deinit();
	wifi_printk(WIFI_IF,"atbm_usb_exit:usb_deregister\n");
}
atbm_void atbm_usb_module_init(atbm_void)
{
	wifi_printk(WIFI_IF, "atbm_usb_module_init\n");
	atbm_init_firmware();
	wifi_printk(WIFI_ALWAYS,"[Wifi] Enter %s \n", __func__);
	atbm_usb_init();
	return ;
}
atbm_void atbm_usb_module_exit(atbm_void)
{
	wifi_printk(WIFI_IF,"atbm_usb_module_exit\n");
	atbm_usb_exit();
	atbm_release_firmware();
	return ;
}


