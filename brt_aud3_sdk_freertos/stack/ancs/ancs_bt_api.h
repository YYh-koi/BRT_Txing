#include "ancs_type.h"
#include "btdatatype.h"

typedef struct
{
    void (*gatt_connection_cbk)(uint8_t connected, uint8_t *bd, uint16_t acl_hdl);
    void (*gatt_service_result_cbk)(ble_uuid128_t *uuid, uint16_t start_hdl, uint16_t end_hdl);
    void (*gatt_characteristic_result_cbk)(ble_uuid128_t *uuid, uint16_t handle, uint16_t value_hdl, uint16_t properties);
    void (*gatt_descriptor_result_cbk)(ble_uuid128_t *uuid, uint16_t handle, uint16_t properties);
    void (*gatt_notification_data_cbk)(uint16_t handle, uint16_t offset, uint16_t len, uint8_t *data);
} gatt_event_callbacks_t;

uint32_t gatt_init_context(gatt_event_callbacks_t *cbks);

uint32_t gatt_destroy_context();

uint32_t gatt_server_register_service(const ble_uuid128_t *service_uuid,
                                      const ble_uuid128_t *char_uuid1,
                                      const ble_uuid128_t *char_uuid2,
                                      const ble_uuid128_t *char_uuid3);

uint32_t gap_adv_start(const ble_uuid128_t *service_uuid);

uint32_t gap_adv_stop();

uint32_t gattc_read(uint16_t conn_handle, uint16_t handle, uint16_t offset);

uint32_t gattc_write(uint16_t conn_handle, gattc_write_params_t *param);

uint32_t gattc_primary_services_discover(uint16_t conn_handle, uint16_t start_handle, ble_uuid16_t const *srvc_uuid);

uint32_t gattc_characteristics_discover(uint16_t conn_handle, gatt_handle_range_t *handle_range);

uint32_t gattc_descriptors_discover(uint16_t conn_handle, gatt_handle_range_t *handle_range);

uint32_t gap_disconnect(uint8_t *bd, uint16_t conn_handle);

void gattc_log_print(const char *fmt, ...);

