#ifndef ANCS_CLIENT_H__
#define ANCS_CLIENT_H__

#include "ancs_type.h"
#include "ancs_parser.h"

/**
  * ANCS service UUID.
  */
#define BLE_UUID_APPLE_NOTIFICATION_CENTER_SERVICE  0xf431
/**
  * Control Point UUID.
  */
#define BLE_UUID_ANCS_CONTROL_POINT_CHAR            0xd8f3
/**
  * Notification Source UUID.
  */
#define BLE_UUID_ANCS_NOTIFICATION_SOURCE_CHAR      0x120d
/**
  * Data source UUID.
  */
#define BLE_UUID_ANCS_DATA_SOURCE_CHAR              0xc6e9

/**
  * Apple Notification Center Service UUIDs */
extern const ble_uuid128_t ble_ancs_base_uuid128;
extern const ble_uuid128_t ble_ancs_cp_base_uuid128;
extern const ble_uuid128_t ble_ancs_ns_base_uuid128;
extern const ble_uuid128_t ble_ancs_ds_base_uuid128;

/** Unknown command   (0xA0):     The commandID was not recognized by the NP.
  * Invalid command   (0xA1):     The command was improperly formatted.
  * Invalid parameter (0xA2):     One of the parameters (for example, the NotificationUID) does not refer to an existing object on the NP.
  * Action failed     (0xA3):     The action was not performed.
  */
typedef enum
{
    BLE_ANCS_ERROR_UNKNOWN_COMMAND = 0xA0,
    BLE_ANCS_ERROR_INVALID_COMMAND,
    BLE_ANCS_ERROR_INVALID_PARAMETER,
    BLE_ANCS_ERROR_ACTION_FAIL,
} ble_ancs_error_code_t;

/**
 * Category IDs for iOS notifications. */
typedef enum
{
    BLE_ANCS_CATEGORY_ID_OTHER,
    BLE_ANCS_CATEGORY_ID_INCOMING_CALL,
    BLE_ANCS_CATEGORY_ID_MISSED_CALL,
    BLE_ANCS_CATEGORY_ID_VOICE_MAIL,
    BLE_ANCS_CATEGORY_ID_SOCIAL,
    BLE_ANCS_CATEGORY_ID_SCHEDULE,
    BLE_ANCS_CATEGORY_ID_EMAIL,
    BLE_ANCS_CATEGORY_ID_NEWS,
    BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,
    BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE,
    BLE_ANCS_CATEGORY_ID_LOCATION,
    BLE_ANCS_CATEGORY_ID_ENTERTAINMENT
} ble_ancs_category_id_values_t;

/**
  * Event IDs for iOS notifications. */
typedef enum
{
    BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED,
    BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED,
    BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED
} ble_ancs_event_id_values_t;

/**
  * Event flags for iOS notifications. */
#define     BLE_ANCS_EVENT_FLAG_SILENT              (1 << 0)
#define     BLE_ANCS_EVENT_FLAG_IMPORTANT           (1 << 1)
#define     BLE_ANCS_EVENT_FLAG_PRE_EXISTING        (1 << 2)
#define     BLE_ANCS_EVENT_FLAG_POSITIVE_ACTION     (1 << 3)
#define     BLE_ANCS_EVENT_FLAG_NEGATIVE_ACTION     (1 << 4)

/**
  * Notification Attribute IDs. */
typedef enum
{
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_APP_IDENTIFIER,
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE, /* Needs to be followed by a 2-bytes max length parameter */
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_SUBTITLE, /* Needs to be followed by a 2-bytes max length parameter */
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE, /* Needs to be followed by a 2-bytes max length parameter */
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE_SIZE, /*a string that represents the integral value of the message size. */
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_DATE, /* a string that uses the Unicode Technical Standard
                                              (UTS) #35 date format pattern yyyyMMdd'T'HHmmSS.*/
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_POSITIVE_ACTION_LABEL,
    BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_NEGATIVE_ACTION_LABEL
} ble_ancs_notification_attribute_id_values_t;

/**
  * struct used for event that connection state changed
  */
typedef struct
{
    uint8_t                            connected;
    uint8_t                            address[6];
} ble_ancs_connection_state_t;


/**
  * Control Point command IDs. */
typedef enum
{
    BLE_ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES,
    BLE_ANCS_COMMAND_ID_GET_APP_ATTRIBUTES,
    /* Starting with iOS 8.0, the NP can inform the NC of potential actions that are associated with iOS notifications */
    BLE_ANCS_COMMAND_ID_PERFORM_ACTION
} ble_ancs_command_id_values_t;

/**
  * ActionID Values */
typedef enum
{
    BLE_ANCS_NOTIFICATION_ACTION_POSITIVE,
    BLE_ANCS_NOTIFICATION_ACTION_NEGATIVE,
} ble_ancs_notification_action_id_values_t;

/**
  * AppAttributeID Values */
typedef enum
{
    BLE_ANCS_APP_ATTRIBUTE_ID_DISPLAY_NAME,
} ble_ancs_get_app_attribute_id_values_t;

/**
  * Event types that be notified by ANCS Client to applcation.
  */
typedef enum
{
    /* Connection established or released to ANCS server */
    BLE_ANCS_EVT_CONNECTION_STATE,
    /* An iOS notification was received on the notification source control point. */
    BLE_ANCS_EVT_IOS_NOTIFICATION,
    /* A received iOS notification attribute has been completely parsed and reassembled. */
    BLE_ANCS_EVT_NOTIF_ATTRIBUTE,
    /* A received Applcation Attribute has been completed parsed and reassembled */
    BLE_ANCS_EVT_APP_ATTRIBUTE,
    /* BLE_ANCS_EVT_PERFORM_ACTION_RSP: No data is generated on the Data Source characteristic
       when this command is issued, whether it is successful or not.*/
} ble_ancs_evt_type_t;

/**
 * The structure contains the event that should be handled, as well as additional information.
 */
typedef struct
{
    ble_ancs_evt_type_t               evt_type;
    ble_uuid128_t                      uuid;
    union
    {
        ble_ancs_connection_state_t       connection_state;
        ble_ancs_ios_notification_t       notification;
        ble_ancs_notif_attribute_t        notif_attribute;
        ble_ancs_app_attribute_t          app_attribute;
        uint32_t                          error_code;
    } data;
} ble_ancs_evt_t;

typedef void (*ble_ancs_evt_handler_t)(ble_ancs_evt_t *p_evt);

uint32_t ble_ancs_init(ble_ancs_evt_handler_t handler);

uint32_t ble_ancs_done();

uint32_t ble_ancs_get_notification_attributes(uint8_t *p_uid, uint8_t num_attr, ble_ancs_attr_list_t *p_attr);

uint32_t ble_ancs_get_app_attributes(uint8_t *app_id, uint8_t num_attr, uint8_t *attr_ids);

uint32_t ble_ancs_perform_action(uint8_t *uid, uint8_t action);

#endif // ANCS_CLIENT_H__

