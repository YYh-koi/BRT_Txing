#ifndef ATBM_OS_SDIO_H
#define ATBM_OS_SDIO_H
#include "atbm_sysops.h"

//#include "atbm_hal.h"
#include "mmcsd_card.h"
#include "sdio.h"

#define atbm_sdio_func      sdio_func
#define os_sdio_driver      sdio_driver_t
#define os_sdio_device_id   sdio_device_id


struct atbm_sdio_device_id{
	char id[8];
};
struct atbm_sdio_driver {
	char name[16];
	struct atbm_sdio_device_id *match_id_table;
	int (*probe_func) (struct atbm_sdio_func *func,	const struct atbm_sdio_device_id *id);
	int (*discon_func) (struct atbm_sdio_func *func);
};

int atbm_sdio_register(struct os_sdio_driver *sdio_driver);
void atbm_sdio_deregister(struct os_sdio_driver *sdio_driver);
void atbm_sdio_claim_host(struct sdio_func *func);
void atbm_sdio_release_host(struct sdio_func *func);
atbm_int32 atbm_sdio_enable_func(struct sdio_func *func);
void atbm_sdio_disable_func(struct sdio_func *func);
void atbm_sdio_set_drvdata(struct sdio_func *func,void *priv);
void *atbm_sdio_get_drvdata(struct sdio_func *func);
int atbm_sdio_claim_irq(struct sdio_func *func,sdio_irq_handler_t *handle);
int atbm_sdio_release_irq(struct sdio_func *func);
int __atbm_sdio_memcpy_fromio(struct sdio_func *func,void *dst,unsigned int addr,int count);
int __atbm_sdio_memcpy_toio(struct sdio_func *func,unsigned int addr,void *dst,int count);
unsigned char atbm_sdio_f0_readb(struct sdio_func *func,unsigned int addr,int *retval);
void atbm_sdio_f0_writeb(struct sdio_func *func,unsigned char regdata,unsigned int addr,int *retval);
int atbm_sdio_set_blocksize(struct sdio_func *func,int blocksize);
atbm_uint32 atbm_sdio_alignsize(struct sdio_func *func,atbm_uint32 size);
struct sdio_func *atbm_sdio_check_init_func4(struct sdio_func *func);
int atbm_sdio_register_init();
int atbm_sdio_register_deinit();
void atbm_sdio_host_enable_irq(int enable);


#endif/* ATBM_OS_SDIO_H */
