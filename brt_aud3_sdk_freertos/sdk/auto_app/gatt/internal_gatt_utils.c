#include "internal_gatt_utils.h"

#include "brt_trace.h"
#ifdef CONFIG_DBG_DB
#define DBG_DB
#define LOG_GATT_BD(fmt, ...) bse_log_info("GATT_DB", fmt, ## __VA_ARGS__)
#else
#define LOG_GATT_BD(fmt, ...) {}
#endif

static char db_str[2048];
static char s_dbg_str[BD_ADDR_LEN * 3];

void revert_uuid(UINT8 *src, UINT8 *dst)
{
	int i;
	for (i=0; i<16; i++) {
		dst[i] = src[15 - i];
	}
}

void hlp_gatt_dbg_services(struct GATT_ServiceStru *Service)
{
#ifdef DBG_DB_
	UINT16 uuid_2;
	struct GATT_DescriptorStru *Descriptor;
	struct GATT_CharacteristicStru *Character = Service->character.head;
	struct GATT_IncludeStru *Include = Service->include.head;

	*db_str = 0;
	LOG_GATT_BD("[Begin Service][UUID: %s",__FUNCTION__);
	DBG_STR(db_str, "[Begin Service][UUID: ");
	if ((uuid_2 = ATT_UUID_GetU2(&Service->uuid)) != 0) {
		DBG_OUT(db_str, "%04X]"), uuid_2);
		LOG_GATT_BD("%04X]",uuid_2);
	} else {
		DBG_OUT(db_str, "[%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]"),
		        Service->uuid.u[15], Service->uuid.u[14], Service->uuid.u[13], Service->uuid.u[12],
		        Service->uuid.u[11], Service->uuid.u[10], Service->uuid.u[9], Service->uuid.u[8],
		        Service->uuid.u[7], Service->uuid.u[6], Service->uuid.u[5], Service->uuid.u[4],
		        Service->uuid.u[3], Service->uuid.u[2], Service->uuid.u[1], Service->uuid.u[0]);
		LOG_GATT_BD("[%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]",
		        Service->uuid.u[15], Service->uuid.u[14], Service->uuid.u[13], Service->uuid.u[12],
		        Service->uuid.u[11], Service->uuid.u[10], Service->uuid.u[9], Service->uuid.u[8],
		        Service->uuid.u[7], Service->uuid.u[6], Service->uuid.u[5], Service->uuid.u[4],
		        Service->uuid.u[3], Service->uuid.u[2], Service->uuid.u[1], Service->uuid.u[0]);
	}
	DBG_OUT(db_str, "[Range: %04X - %04X]\r\n"), Service->hdl, Service->endhdl);
	LOG_GATT_BD("[Range: %04X - %04X]\r\n", Service->hdl, Service->endhdl);

	while (Include != NULL) {
		DBG_OUT(db_str, "[Include: %04X][UUID: "), Include->hdl);
		LOG_GATT_BD("[Include: %04X][UUID: " ,Include->hdl);
		if ((uuid_2 = ATT_UUID_GetU2(&Include->uuid)) != 0) {
			DBG_OUT(db_str, "%02X]"), uuid_2);
		} else {
			DBG_OUT(db_str, "[%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]"),
			        Include->uuid.u[15], Include->uuid.u[14], Include->uuid.u[13], Include->uuid.u[12],
			        Include->uuid.u[11], Include->uuid.u[10], Include->uuid.u[9], Include->uuid.u[8],
			        Include->uuid.u[7], Include->uuid.u[6], Include->uuid.u[5], Include->uuid.u[4],
			        Include->uuid.u[3], Include->uuid.u[2], Include->uuid.u[1], Include->uuid.u[0]);
		}
		DBG_OUT(db_str, "[Range: %04X - %04X]\r\n"), Include->include_hdl, Include->endhdl);
		Include = LNEXT(Include);
	}

	while (Character != NULL) {
	DBG_OUT(db_str, "[Char: %04X][UUID: "), Character->hdl);
	LOG_GATT_BD("[Char: %04X][UUID: ", Character->hdl);
		if ((uuid_2 = ATT_UUID_GetU2(&Character->uuid)) != 0) {
			DBG_OUT(db_str, "%02X]"), uuid_2);
			LOG_GATT_BD("%02X]", uuid_2);
		} else {
			DBG_OUT(db_str, "[0x%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]"),
			        Character->uuid.u[15], Character->uuid.u[14], Character->uuid.u[13], Character->uuid.u[12],
			        Character->uuid.u[11], Character->uuid.u[10], Character->uuid.u[9], Character->uuid.u[8],
			        Character->uuid.u[7], Character->uuid.u[6], Character->uuid.u[5], Character->uuid.u[4],
			        Character->uuid.u[3], Character->uuid.u[2], Character->uuid.u[1], Character->uuid.u[0]);
			LOG_GATT_BD("[0x%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]",
			        Character->uuid.u[15], Character->uuid.u[14], Character->uuid.u[13], Character->uuid.u[12],
			        Character->uuid.u[11], Character->uuid.u[10], Character->uuid.u[9], Character->uuid.u[8],
			        Character->uuid.u[7], Character->uuid.u[6], Character->uuid.u[5], Character->uuid.u[4],
			        Character->uuid.u[3], Character->uuid.u[2], Character->uuid.u[1], Character->uuid.u[0]);
		}
		DBG_STR(db_str, "[Prop: ");
		LOG_GATT_BD("[Prop: ");
		if (Character->properties & ATT_READ) {
			DBG_STR(db_str, "R");
			LOG_GATT_BD("R");
		}
		if (Character->properties & ATT_WRITE) {
			DBG_STR(db_str, "W");
			LOG_GATT_BD( "W");
		}
		if (Character->properties & ATT_WRITE_NORSP) {
			DBG_STR(db_str, "Wc");
			LOG_GATT_BD( "Wc");
		}
		if (Character->properties & ATT_AUTHENTICATION_NEED) {
			DBG_STR(db_str, "At");
			LOG_GATT_BD("At");
		}
		if (Character->properties & ATT_MITM_NEED) {
			DBG_STR(db_str, "Am");
			LOG_GATT_BD("Am");
		}
		if (Character->properties & ATT_NOTIFY) {
			DBG_STR(db_str, "N");
			LOG_GATT_BD("N");
		}
		if (Character->properties & ATT_INDICATE) {
			DBG_STR(db_str, "I");
			LOG_GATT_BD( "I");
		}
		DBG_OUT(db_str, "][Value: %04X][Size: (O)"), Character->value_hdl);
		LOG_GATT_BD( "][Value: %04X][Size: (O)", Character->value_hdl);
		if (Character->val != NULL) {
		DBG_OUT(db_str, "%d][ "), Character->val->len);
		LOG_GATT_BD( "%d][ ", Character->val->len);
			for (uuid_2 = 0; (uuid_2 < Character->val->len) && (uuid_2 < 4); uuid_2++) {
				DBG_OUT(db_str, "%02X "), Character->val->value[uuid_2]);
				LOG_GATT_BD( "%02X ", Character->val->value[uuid_2]);
			}
			if (uuid_2 < Character->val->len) {
				DBG_STR(db_str, "%... ");
			}
		} else {
			DBG_STR(db_str, "0][ ");
			LOG_GATT_BD("0][ ");
		}
		DBG_STR(db_str, "]\r\n");
		LOG_GATT_BD( "]\r\n");

		Descriptor = Character->descriptor.head;
		while (Descriptor != NULL) {
		DBG_OUT(db_str, "[Desc: %04X][UUID: "), Descriptor->hdl);
		LOG_GATT_BD( "[Desc: %04X][UUID: ", Descriptor->hdl);
			if ((uuid_2 = ATT_UUID_GetU2(&Descriptor->uuid)) != 0) {
				DBG_OUT(db_str, "%02X]"), uuid_2);
				LOG_GATT_BD("%02X]", uuid_2);
			} else {
				DBG_OUT(db_str, "[0x%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]"),
				        Descriptor->uuid.u[15], Descriptor->uuid.u[14], Descriptor->uuid.u[13], Descriptor->uuid.u[12],
				        Descriptor->uuid.u[11], Descriptor->uuid.u[10], Descriptor->uuid.u[9], Descriptor->uuid.u[8],
				        Descriptor->uuid.u[7], Descriptor->uuid.u[6], Descriptor->uuid.u[5], Descriptor->uuid.u[4],
				        Descriptor->uuid.u[3], Descriptor->uuid.u[2], Descriptor->uuid.u[1], Descriptor->uuid.u[0]);
				LOG_GATT_BD("[0x%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X]",
				        Descriptor->uuid.u[15], Descriptor->uuid.u[14], Descriptor->uuid.u[13], Descriptor->uuid.u[12],
				        Descriptor->uuid.u[11], Descriptor->uuid.u[10], Descriptor->uuid.u[9], Descriptor->uuid.u[8],
				        Descriptor->uuid.u[7], Descriptor->uuid.u[6], Descriptor->uuid.u[5], Descriptor->uuid.u[4],
				        Descriptor->uuid.u[3], Descriptor->uuid.u[2], Descriptor->uuid.u[1], Descriptor->uuid.u[0]);
			}
			DBG_STR(db_str, "[Prop: ");
			LOG_GATT_BD("[Prop: ");
			if (Character->properties & ATT_READ) {
				DBG_STR(db_str, "R");
				LOG_GATT_BD("R");
			}
			if (Character->properties & ATT_WRITE) {
				DBG_STR(db_str, "Wr");
				LOG_GATT_BD("Wr");
			}
			if (Character->properties & ATT_WRITE_NORSP) {
				DBG_STR(db_str, "Wc");
				LOG_GATT_BD("Wc");
			}
			if (Character->properties & ATT_AUTHENTICATION_NEED) {
				DBG_STR(db_str, "E");
				LOG_GATT_BD("E");
			}
			if (Character->properties & ATT_MITM_NEED) {
				DBG_STR(db_str, "Am");
				LOG_GATT_BD("Am");
			}
			DBG_STR(db_str, "][Size: (O)");
			LOG_GATT_BD("][Size: (O)");
			if (Descriptor->val != NULL) {
			DBG_OUT(db_str, "%d][ "), Descriptor->val->len);
			LOG_GATT_BD("%d][ ", Descriptor->val->len);
				for (uuid_2 = 0; (uuid_2 < Descriptor->val->len) && (uuid_2 < 4); uuid_2++) {
					DBG_OUT(db_str, "%02X "), Descriptor->val->value[uuid_2]);
					LOG_GATT_BD("%02X ", Descriptor->val->value[uuid_2]);
				}
				if (uuid_2 < Descriptor->val->len) {
					DBG_STR(db_str, "%... ");
				}
			} else {
				DBG_STR(db_str, "0][ ");
				LOG_GATT_BD("0][ ");
			}
			DBG_STR(db_str, "]\r\n");
			LOG_GATT_BD("]\r\n");
			Descriptor = LNEXT(Descriptor);
		}

		Character = LNEXT(Character);
	}

	DBG_STR(db_str, "[End Service]\r\n\r\n");

	const char *config_dir = get_configure_path(CONFIGURATION_TYPE_READWRITE);

	if (config_dir) {
		char buf[128];
		FILE *fp;
		snprintf(buf, 128, "%s/gatt_db.txt", config_dir);
		fp = fopen(buf, "a+");

		if (fp) {
			fwrite(db_str, 1, strlen(db_str), fp);
			fclose(fp);
		}
	}

	LOG(db_str);
#endif
}

char *get_addr_str(UINT8 *a)
{
	s_dbg_str[BD_ADDR_LEN * 3 - 1] = 0;
	snprintf(s_dbg_str, BD_ADDR_LEN * 3, "%02X:%02X:%02X:%02X:%02X:%02X",
	         (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]);

	return s_dbg_str;
}

