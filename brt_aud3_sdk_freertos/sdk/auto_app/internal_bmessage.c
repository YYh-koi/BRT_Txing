/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/
#include "global.h"				//for NEW/FREE
#include "internal_bmessage.h"

/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     hlp_bmessage.c
 * @Description:
        This file provides utils functions to compose and parse bMessage object.

 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-18
 ---------------------------------------------------------------------------*/

#define BEGIN_BMSG "BEGIN:BMSG\r\n"
#define END_BMSG "END:BMSG\r\n"
#define VERSION "VERSION:"
#define TYPE "TYPE:"
#define READSTATUS "STATUS:"
#define FOLDER "FOLDER:"
#define BEGIN_BENV "BEGIN:BENV\r\n"
#define END_BENV "END:BENV\r\n"
#define BEGIN_BBODY "BEGIN:BBODY\r\n"
#define END_BBODY "END:BBODY\r\n"
#define PARTID "PARTID:"
#define ENCODING "ENCODING:"
#define CHARSET "CHARSET:"
#define LANGUAGE "LANGUAGE:"
#define LENGTH "LENGTH:"
#define BEGIN_MSG "BEGIN:MSG\r\n"
#define END_MSG "END:MSG\r\n"
#define READ "READ\r\n"
#define UNREAD "UNREAD\r\n"
#define EMAIL "EMAIL\r\n"
#define SMS_GSM "SMS_GSM\r\n"
#define SMS_CDMA "SMS_CDMA\r\n"
#define MMS "MMS\r\n"
#define RETURN "\r\n"

#define BEGIN_VCARD "BEGIN:VCARD\r\n"
/* XXX version always 2.1? */
#define VCARD_VERSION "VERSION:2.1\r\n"
#define VCARD_NAME "N:"
#define VCARD_TEL "TEL:"
#define END_VCARD "END:VCARD\r\n"

static int bmessage_calculate_length(app_map_sms_item_t *msg);
static int bmessage_compose_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_originator(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_envelope(app_map_sms_item_t *msg, UINT8 *bmsg);
/*for property*/
static int bmessage_compose_version_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_readstatus_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_type_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_folder_property(app_map_sms_item_t *msg, UINT8 *bmsg);
/* for envelope */
static int bmessage_compose_recipient(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_content(app_map_sms_item_t *msg, UINT8 *bmsg);
/* for content */
static int bmessage_compose_body_part_id(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_body_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_body_content(app_map_sms_item_t *msg, UINT8 *bmsg);
/* for body-property */
static int bmessage_compose_body_encoding_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_body_charset_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_body_language_property(app_map_sms_item_t *msg, UINT8 *bmsg);
static int bmessage_compose_body_content_length_property(app_map_sms_item_t *msg, UINT8 *bmsg);
/* for body-content-message */
static int bmessage_compose_body_content_message(app_map_sms_item_t *msg, UINT8 *bmsg);

/* for vCard */
static int bmessage_compose_vcard(char *name, char *address, UINT8 *bmsg);
static int bmessage_compose_vcard_name(char *name, UINT8 *bmsg);
static int bmessage_compose_vcard_address(char *address, UINT8 *bmsg);

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
 *      This function compose a struct of "app_map_sms_item_t" into a bMessage object which then can be
 *      transmited by a PUT OBEX operation.
 *
 * @Arguments:
        msg The app_map_sms_item_t to be composed.
 * @Arguments:
        out_bmsg[out] The target buffer to hold the bMessage object. The memory will be allocated inside,
                so please release it by yourself.
 * @Arguments:
        out_len[out] The length of assembled bMessage object.
 *
 * @Return:
        1 if successfully composed a bMessage object, 0 otherwise.
 */
/* ---------------------------------------------------------------------------*/
int bmessage_compose(app_map_sms_item_t *msg, UINT8 **out_bmsg, UINT16 *out_len)
{
	int length;
	int part_len;
	UINT8 *bmsg = NULL;
	UINT8 *bmessage = NULL;

	if (!msg) {
		return 0;
	}

	length = bmessage_calculate_length(msg);

	if (length <= 0) {
		return 0;
	}

	bmsg = (UINT8 *) NEW(length);

	if (!bmsg) {
		return 0;
	}

	bmessage = bmsg;
	/* build bmessage-header BEGIN */
	part_len = strlen(BEGIN_BMSG);
	memcpy(bmessage, BEGIN_BMSG, part_len);
	bmessage += part_len;

	/* build bmessage-property */
	part_len = bmessage_compose_property(msg, bmessage);
	bmessage += part_len;

	/* build bmessage-originator */
	part_len = bmessage_compose_originator(msg, bmessage);
	bmessage += part_len;

	/* build bmessage-envelope */
	part_len = bmessage_compose_envelope(msg, bmessage);
	bmessage += part_len;

	/* build bmessage-header END */
	memcpy(bmessage, END_BMSG, strlen(END_BMSG));

	*out_bmsg = bmsg;
	*out_len = length;

	return 1;
}

int bmessage_decompose(UINT8 * data, UINT16 len, app_map_sms_item_t *msg)
{
	// TODO
	return 0;
}

static int bmessage_calculate_length(app_map_sms_item_t *msg)
{
	return strlen(BEGIN_BMSG) + bmessage_compose_property(msg, NULL)
	       + bmessage_compose_originator(msg, NULL) + bmessage_compose_envelope(msg, NULL)
	       + strlen(END_BMSG);
}

static int bmessage_compose_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build bmessage-version-property */
	part_len = bmessage_compose_version_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-readstatus-property */
	part_len = bmessage_compose_readstatus_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-type-property */
	part_len = bmessage_compose_type_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-folder-property */
	part_len = bmessage_compose_folder_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	return length;
}

static int bmessage_compose_originator(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	// TODO <bmessage-originator> ::= <vcard> <CRLF> optional
	return 0;
}

static int bmessage_compose_envelope(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build envelope-header BEGIN */
	part_len = strlen(BEGIN_BENV);
	if (bmessage) {
		memcpy(bmessage, BEGIN_BENV, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-recipient */
	part_len = bmessage_compose_recipient(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-content */ /*FIXME: <bmessage-envelope> | <bmessage-content> */
	part_len = bmessage_compose_content(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build envelope-header END */
	part_len = strlen(END_BENV);
	if (bmessage) {
		memcpy(bmessage, END_BENV, part_len);
	}
	length += part_len;

	return length;
}

static int bmessage_compose_version_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_v = strlen(VERSION);
	int len_1 = strlen("1.0\r\n");

	if (bmsg) {
		memcpy(bmsg, VERSION, len_v);
		memcpy(bmsg + len_v, "1.0\r\n", len_1);
	}
	return len_v + len_1;
}

static int bmessage_compose_readstatus_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_s = strlen(READSTATUS);
	int len_r;
	UINT8 *status;

	if (msg->readed) {
		status = READ;
	} else {
		status = UNREAD;
	}

	len_r = strlen(status);

	if (bmsg) {
		memcpy(bmsg, READSTATUS, len_s);
		memcpy(bmsg + len_s, status, len_r);
	}

	return len_s + len_r;
}

static int bmessage_compose_type_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_h = strlen(TYPE);
	int len_t;
	UINT8 *type = SMS_GSM;

	switch (msg->type) {
	case 1:
		type = EMAIL;
		break;
	case 2:
		type = SMS_GSM;
		break;
	case 3:
		type = SMS_CDMA;
		break;
	case 4:
		type = MMS;
		break;
	}

	len_t = strlen(type);

	if (bmsg) {
		memcpy(bmsg, TYPE, len_h);
		memcpy(bmsg + len_h, type, len_t);
	}

	return len_h + len_t;
}

static int bmessage_compose_folder_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_h = strlen(FOLDER);
	int len_f;
	char *folder = "telecom/msg/outbox\r\n";

	len_f = strlen(folder);

	if (bmsg) {
		memcpy(bmsg, FOLDER, len_h);
		memcpy(bmsg + len_h, folder, len_f);
	}

	return len_h + len_f;
}

static int bmessage_compose_recipient(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build recipient's vCard */
	part_len = bmessage_compose_vcard(msg->recipient_name, msg->recipient_address, bmsg);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

#if 0	//End Vcard include "\r\n"
	/* build another "\r\n" */
	part_len = strlen(RETURN);
	if (bmessage) {
		memcpy(bmessage, RETURN, part_len);
	}
	length += part_len;
#endif
	return length;
}

static int bmessage_compose_content(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build bbody-header BEGIN */
	part_len = strlen(BEGIN_BBODY);
	if (bmessage) {
		memcpy(bmessage, BEGIN_BBODY, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-part-ID */
	part_len = bmessage_compose_body_part_id(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-property */
	part_len = bmessage_compose_body_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-content */
	part_len = bmessage_compose_body_content(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

#if 0	// End Msg include "\r\n"
	/* build another "\r\n" */
	part_len = strlen(RETURN);
	if (bmessage) {
		memcpy(bmessage, RETURN, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;
#endif
	/* build bbody-header END */
	part_len = strlen(END_BBODY);
	if (bmessage) {
		memcpy(bmessage, END_BBODY, part_len);
	}
	length += part_len;

	return length;
}

static int bmessage_compose_body_part_id(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	// TODO optional
	return 0;
}

static int bmessage_compose_body_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build bmessage-body-encoding-property */
	part_len = bmessage_compose_body_encoding_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-charset-property */
	part_len = bmessage_compose_body_charset_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-language-property */
	part_len = bmessage_compose_body_language_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build bmessage-body-content-length-property */
	part_len = bmessage_compose_body_content_length_property(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	return length;
}

static int bmessage_compose_body_content(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build body-content-header BEGIN */
	part_len = strlen(BEGIN_MSG);
	if (bmessage) {
		memcpy(bmessage, BEGIN_MSG, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build body-content-message */
	part_len = bmessage_compose_body_content_message(msg, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build body-content-header END */
	part_len = strlen(END_MSG);
	if (bmessage) {
		memcpy(bmessage, END_MSG, part_len);
	}
	length += part_len;

	return length;
}

static int bmessage_compose_body_encoding_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	// TODO optional
	return 0;
}

static int bmessage_compose_body_charset_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_h = strlen(CHARSET);
	int len_c;
	UINT8 *charset;

	/* FIXME */
	charset = "UTF-8\r\n";
	len_c = strlen(charset);

	if (bmsg) {
		memcpy(bmsg, CHARSET, len_h);
		memcpy(bmsg + len_h, charset, len_c);
	}

	return len_h + len_c;
}

static int bmessage_compose_body_language_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	// TODO optional
	return 0;
}

static int bmessage_compose_body_content_length_property(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int len_h = strlen(LENGTH);
	int len_l;
	char len_str[16];

	memset(len_str, 0, 16);
	// FIX bug: bmessage-body-content-length-property should be the total length of bmessage-body-content
	// which include "BEGIN:MSG" and "END:MSG"
	sprintf(len_str, "%d\r\n", (msg->size + strlen(RETURN) + strlen(BEGIN_MSG) + strlen(END_MSG)));
	len_l = strlen(len_str);

	if (bmsg) {
		memcpy(bmsg, LENGTH, len_h);
		memcpy(bmsg + len_h, len_str, len_l);
	}

	return len_h + len_l;
}

static int bmessage_compose_body_content_message(app_map_sms_item_t *msg, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	if (msg->content) {
		part_len = msg->size; /* FIXME: property "size" is the length of message content? */
		if (bmessage) {
			memcpy(bmessage, msg->content, part_len);
		}
	} else {
		part_len = 0;
	}

	bmessage += part_len;
	length += part_len;

	part_len = strlen(RETURN);
	if (bmsg) {
		memcpy(bmessage, RETURN, part_len);
	}
	length += part_len;

	return length;
}

static int bmessage_compose_vcard(char *name, char *address, UINT8 *bmsg)
{
	int length = 0;
	int part_len;
	UINT8 *bmessage = bmsg;

	/* build vCard-header BEGIN */
	part_len = strlen(BEGIN_VCARD);
	if (bmessage) {
		memcpy(bmessage, BEGIN_VCARD, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	part_len = strlen(VCARD_VERSION);
	if (bmessage) {
		memcpy(bmessage, VCARD_VERSION, part_len);
	}
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build body-content-message */
	part_len = bmessage_compose_vcard_name(name, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	part_len = bmessage_compose_vcard_address(address, bmessage);
	length += part_len;
	bmessage = (bmessage == NULL) ? NULL : bmessage + part_len;

	/* build vCard-header end */
	part_len = strlen(END_VCARD);
	if (bmessage) {
		memcpy(bmessage, END_VCARD, part_len);
	}
	length += part_len;

	return length;
}

static int bmessage_compose_vcard_name(char *name, UINT8 *bmsg)
{
	int len_h = strlen(VCARD_NAME);
	int len_n;
	char *name_str = (name == NULL) ? "" : name;

	len_n = strlen(name_str);

	if (bmsg) {
		memcpy(bmsg, VCARD_NAME, len_h);
		memcpy(bmsg + len_h, name_str, len_n);
		memcpy(bmsg + len_h + len_n, RETURN, strlen(RETURN));
	}

	return len_h + len_n + strlen(RETURN);
}

static int bmessage_compose_vcard_address(char *address, UINT8 *bmsg)
{
	int len_h = strlen(VCARD_TEL);
	int len_n;
	char *addr_str = (address == NULL) ? "" : address;

	len_n = strlen(addr_str);

	if (bmsg) {
		memcpy(bmsg, VCARD_TEL, len_h);
		memcpy(bmsg + len_h, addr_str, len_n);
		memcpy(bmsg + len_h + len_n, RETURN, strlen(RETURN));
	}

	return len_h + len_n + strlen(RETURN);
}

