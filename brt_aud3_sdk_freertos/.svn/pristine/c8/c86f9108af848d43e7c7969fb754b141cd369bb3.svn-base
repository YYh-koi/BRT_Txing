#ifndef _TL_H_
#define _TL_H_

#include <stdlib.h>

typedef void (*tl_data_received_cb)(unsigned char *data, unsigned int len);

int open_port(tl_data_received_cb cb);
int write_port(unsigned char *buf, unsigned long len);
int read_port(unsigned char *buf, unsigned long len);
int close_port();
void tl_fw_config(void *param);

#endif //_TL_H_
