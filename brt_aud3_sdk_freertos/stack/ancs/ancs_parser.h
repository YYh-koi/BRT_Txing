#ifndef ANCS_PARSER_H__
#define ANCS_PARSER_H__

#include "ancs_type.h"
#include "btdatatype.h"

/**
  * Maximium notification attribute data length.
  */
#define ANCS_ATTRIBUTE_MAX_LENGTH       64

/**
  * Maximium Application identifier length.
  */
#define ANCS_APP_ID_MAX_LENGTH          32

typedef struct
{
    uint8_t                            attribute_id;
    uint16_t                           attribute_len;
} ble_ancs_attr_list_t;

typedef struct
{
    uint8_t                            id;
    uint16_t                           len;
    uint8_t                            data[ANCS_ATTRIBUTE_MAX_LENGTH];
} ble_ancs_attribute_tuple_t;

/**
  * struct used for event that an iOS notification has received
  */
typedef struct
{
    uint8_t                            event_id;
    uint8_t                            event_flags;
    uint8_t                            category_id;
    uint8_t                            category_count;
    uint8_t                            notification_uid[4];
} ble_ancs_ios_notification_t;

/**
  * struct used for event that a notification attribute has received
  */
typedef struct
{
    uint8_t                            command_id;
    uint8_t                            notification_uid[4];
    ble_ancs_attribute_tuple_t         attribute;
} ble_ancs_notif_attribute_t;

/**
  * struct used for event that a application attribute has received
  */
typedef struct
{
    uint8_t                            command_id;
    /*The string identifier of the app the client wants information about. This string must be NULL-terminated */
    char                               app_id[ANCS_APP_ID_MAX_LENGTH];
    ble_ancs_attribute_tuple_t         attribute;
} ble_ancs_app_attribute_t;

typedef struct
{
    void (*ios_notification_callback)(ble_ancs_ios_notification_t *notification);
    void (*notif_attribute_callback)(ble_ancs_notif_attribute_t *notif_attribute);
    void (*app_attribute_callback)(ble_ancs_app_attribute_t *app_attribute);
} ancs_parser_evt_callbacks_t;

void ancs_parser_init(ancs_parser_evt_callbacks_t *cb);
void ancs_parser_done();

/* compose command */
uint32_t ancs_parser_compose_get_app_attr_cmd(uint8_t *app_id, uint8_t num_attr, uint8_t *attr_ids,
        uint8_t *out_buf, uint16_t buf_size);
uint32_t ancs_parser_compose_get_notif_attr_cmd(uint8_t *p_uid, uint8_t num_attr, ble_ancs_attr_list_t *p_attr,
        uint8_t *out_buf, uint16_t buf_size);
uint32_t ancs_parser_compose_perform_action_cmd(uint8_t *uid, uint8_t action, uint8_t *out_buf, uint16_t buf_size);

void ancs_parse_ios_notification(uint16_t len, uint8_t *data);
void ancs_parse_get_notification_attribute(uint16_t len, uint8_t *data);
void ancs_parse_get_app_attribute(uint16_t len, uint8_t *data);
void ancs_parse_remain_notification_data(uint16_t len, uint8_t *data);

#endif /*ANCS_PARSER_H__*/
