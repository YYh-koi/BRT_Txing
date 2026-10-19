#ifndef ANCS_TYPE_H_
#define ANCS_TYPE_H_

/*TODO define OS macro somewhere */
#include "autoconf_bt.h"
#include "btdatatype.h"

typedef struct
{
    uint8_t uuid[16];
} ble_uuid128_t;

typedef uint16_t ble_uuid16_t;

typedef struct
{
    uint16_t start_hdl;
    uint16_t end_hdl;
} gatt_handle_range_t;

typedef struct
{
    uint16_t handle;
    uint16_t offset;
    uint16_t len;
    uint8_t *data;
} gattc_write_params_t;

#endif // ANCS_TYPE_H_

