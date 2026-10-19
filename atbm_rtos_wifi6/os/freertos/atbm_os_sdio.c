#include "atbm_hal.h"
#include "atbm_os_sdio.h"
#include "sdio.h"
#include "mmcsd_core.h"
#include "mmcsd_card.h"
//#include "rtdef.h"
#define __ATBM_SDIO_ALIGNSIZE(___blks,___size)  ((___size)+(___blks)-((___size)%(___blks)))
#define ATBM_SDIO_ALIGNSIZE(_blks,_size)        (((_size)%(_blks)==0)?(_size):__ATBM_SDIO_ALIGNSIZE(_blks,_size))

static struct os_sdio_driver  atmbwifi_driver;
static struct mmcsd_host *g_sdio_host=NULL;

extern int atbm_sdio_probe(struct atbm_sdio_func *func,const struct atbm_sdio_device_id *id);
extern int atbm_sdio_disconnect(struct atbm_sdio_func *func);
static struct os_sdio_device_id atbm_sdio_ids[] = {
        {1,0x007a,0x6011},
		{1,0x007a,0x8893},
		{1,0x007a,0x8895},
};
void atbm_sdio_host_enable_irq(int enable)
{
    if(g_sdio_host){
        g_sdio_host->ops->enable_sdio_irq(g_sdio_host, enable);
    }
}
void atbm_sdio_claim_host(struct sdio_func  *func)
{
    mmcsd_host_lock(func->card->host);
}
void atbm_sdio_release_host(struct sdio_func *func)
{
    mmcsd_host_unlock(func->card->host);
}

int atbm_sdio_claim_irq(struct sdio_func *func,sdio_irq_handler_t *handle)
{
    return sdio_attach_irq(func,handle);
}
int atbm_sdio_release_irq(struct sdio_func *func)
{
    return sdio_detach_irq(func);
}
int __atbm_sdio_memcpy_fromio(struct sdio_func *func,void *dst,unsigned int addr,int count)
{
    int ret;
    if((((unsigned int)dst) &3)||(count &3)){
        wifi_printk(WIFI_ALWAYS,"Read Data & Len must be 4 align,dst=0x%08x,count=%d",(unsigned int )dst,count);
    }
    ret=sdio_io_read_multi_incr_b(func,addr,dst,count);
  if(ret!=0){
       wifi_printk(WIFI_ALWAYS," __atbm_sdio_memcpy_fromio ret =%d\n",ret);
  }

    return ret;
}

int __atbm_sdio_memcpy_toio(struct sdio_func *func,unsigned int addr,void *dst,int count)
{
    int ret;
    if((((unsigned int)dst) &3)||(count &3)){
        wifi_printk(WIFI_ALWAYS,"Write Data & Len must be 4 align,dst=0x%08x,count=%d",(unsigned int )dst,count);
    }
	//printf("sdio write len:%d\r\n", count);
    ret=sdio_io_write_multi_incr_b(func,addr,dst,count);
	//printf("sdio write ret:%d\r\n", ret);
  if(ret!=0){
        wifi_printk(WIFI_ALWAYS," __atbm_sdio_memcpy_toio ret =%d\n",ret);
  }

    return ret;
}

unsigned char atbm_sdio_f0_readb(struct sdio_func *func,unsigned int addr,int *retval)
{
    struct sdio_func *func0;

    func0 = func->card->sdio_function[0];
    return sdio_io_readb(func0,addr,(int32_t*)retval);

}

void atbm_sdio_f0_writeb(struct sdio_func *func,unsigned char regdata,unsigned int addr,int *retval)
{
    struct sdio_func *func0;

    func0 = func->card->sdio_function[0];
    *retval=sdio_io_writeb(func0,addr,regdata);
}

atbm_uint32 atbm_sdio_alignsize(struct sdio_func *func,atbm_uint32 size)
{
    atbm_uint32 block_size = func->cur_blk_size;
    atbm_uint32 alignsize = size;
    if((block_size == 0)||(size == 0)){
		wifi_printk(WIFI_SDIO,"atbm_sdio_alignsize err\n");
        return alignsize;
    }

    alignsize = ATBM_SDIO_ALIGNSIZE(block_size,size);
    return alignsize;
}

int atbm_sdio_register(struct os_sdio_driver *sdio_driver)
{
    wifi_printk(WIFI_SDIO,"atbm_sdio_register\n");
    return sdio_register_driver(sdio_driver);
}

void atbm_sdio_deregister(struct os_sdio_driver *sdio_driver)
{
    sdio_unregister_driver(sdio_driver);
}
void atbm_sdio_set_drvdata(struct sdio_func *func,void *priv)
{
    sdio_set_drvdata(func,priv);
}

void *atbm_sdio_get_drvdata(struct sdio_func *func)
{
    return sdio_get_drvdata(func);
}

atbm_int32 atbm_sdio_enable_func(struct sdio_func *func)
{
    return sdio_enable_func(func);
}

void atbm_sdio_disable_func(struct sdio_func *func)
{
    sdio_disable_func(func);
}

int  atbm_sdio_set_blocksize(struct sdio_func *func,int blocksize)
{
    func->max_blk_size = blocksize;
    return sdio_set_block_size(func,blocksize);
}

struct sdio_func *atbm_sdio_check_init_func4(struct sdio_func *func){
	struct sdio_func *cmd_func = func->card->sdio_function[4];
	if(cmd_func && (atbm_sdio_enable_func(cmd_func) == 0)){
		return cmd_func;
	}
	return ATBM_NULL;
}

extern void atbm_wifi_set_init_done();
int32_t _atbm_sdio_probe(struct mmcsd_card *card)
{
    g_sdio_host=card->host;
    if(0 == atbm_sdio_probe(card->sdio_function[1],NULL)){
		//atbm_wifi_set_init_done();
		return 0;
	}
	return -1;
}
int32_t _atbm_sdio_disconnect(struct mmcsd_card *card)
{
    return atbm_sdio_disconnect(card->sdio_function[1]);
}
int atbm_sdio_register_init()
{
    int ret =0;
	int i;


    atmbwifi_driver.name="atbm6031";
    atmbwifi_driver.probe      = _atbm_sdio_probe;
    atmbwifi_driver.remove     = _atbm_sdio_disconnect;
    wifi_printk(WIFI_ALWAYS, "atbm_sdio_register_init\r\n");
	for(i = 0; i < sizeof(atbm_sdio_ids)/sizeof(struct os_sdio_device_id); i++){
		atmbwifi_driver.id  = &atbm_sdio_ids[i];
	    ret = atbm_sdio_register(&atmbwifi_driver);
	    if ( ret == 0 ){
			wifi_printk(WIFI_DBG_ERROR,"atbmwifi sdio id match %x %x\n", atbm_sdio_ids[i].manufacturer, atbm_sdio_ids[i].product);
	        break;
	    }
		wifi_printk(WIFI_DBG_ERROR,"atbmwifi sdio id not match %x %x\n", atbm_sdio_ids[i].manufacturer, atbm_sdio_ids[i].product);
	}

    return ret;
}

int atbm_sdio_register_deinit()
{
    atbm_sdio_deregister(&atmbwifi_driver);
    return 0;
}
