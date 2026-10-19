#ifndef HCI_PARSER_H__
#define HCI_PARSER_H__

#include "btdatatype.h"

typedef void (*hci_data_callback)(UINT8 *buf, UINT16 len);

typedef struct tag_hci_parser {
	void (*init)(struct tag_hci_parser *self, hci_data_callback callback);
	void (*done)(struct tag_hci_parser *self);
	int (*parse)(struct tag_hci_parser *self, UINT8 *data, UINT16 len);
} hci_parser_t;

hci_parser_t *hci_parser_new();
void hci_parser_free(hci_parser_t *obj);
#endif /* HCI_PARSER_H__*/

