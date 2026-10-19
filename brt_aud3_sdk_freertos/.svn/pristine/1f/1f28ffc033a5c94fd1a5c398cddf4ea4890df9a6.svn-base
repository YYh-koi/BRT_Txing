#ifndef BTAPP_UTILS_H__
#define BTAPP_UTILS_H__
#include "btdatatype.h"
#include "bluedroid/bluetooth.h"

/** GATT ID adding instance id tracking to the UUID */
typedef struct {
    bt_uuid_t           uuid;
    uint8_t             inst_id;
} btgatt_gatt_id_t;

/** GATT Service ID also identifies the service type (primary/secondary) */
typedef struct {
    btgatt_gatt_id_t    id;
    uint8_t             is_primary;
} btgatt_srvc_id_t;
unsigned char hex_char_to_byte(UINT8 ch);
UINT8 hex_chars_to_byte(UINT8 data[2]);
unsigned char hex_string_to_byte_array(UINT32 str_len, char *str, UINT8 *num_arr);
unsigned char byte_array_to_hex_string(UINT8 *str, UINT8 *arr, UINT16 len);
unsigned char bd_colon_str_to_bytes(UINT8 *str, UINT8 *bd);

#ifdef CONFIG_GATT
#include "bluedroid/bt_gatt_types.h"
unsigned char parse_byte(char *p);
unsigned short parse_uuid16(char *p);
unsigned short parse_short(char *p);
unsigned int parse_int(char *p);
long parse_long(char *p);
void parse_bd(char *p, unsigned char bd[6]);
void parse_srvc_id(char *p, btgatt_srvc_id_t* srvc_id);
void parse_char_id(char *p, btgatt_gatt_id_t* char_id);
void parse_descr_id(char *p, bt_uuid_t* descr_id);
#endif

unsigned char is_uuid_valid(UINT8 uuid[16]);
void parse_uuid128(char *p, bt_uuid_t *uuid_128);
bt_bdaddr_t parse_address(char *p);
void parse_bd_address(const char *str, UINT8 *bd);

// convert type between BRT and bluedroid
#define HCI_ADDR_STRU_TO_BLUEDROID_BD(A,B) memcpy((B)->address, (A)->bd, sizeof(bt_bdaddr_t));
#define BLUEDROID_BD_TO_HCI_ADDR_STRU(A,B) memcpy((B)->bd, (A)->address, sizeof(bt_bdaddr_t));

unsigned char *parse_bytes(char *p, int *len);

#endif // BTAPP_UTILS_H__

