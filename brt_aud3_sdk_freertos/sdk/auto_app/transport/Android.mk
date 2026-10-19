TL_PATH := $(LOCAL_PATH)/transport

MY_LIST += $(TL_PATH)/transport.c
MY_LIST += $(TL_PATH)/tl.c
MY_LIST += $(TL_PATH)/tl_buffer.c

ifeq ($(CONFIG_TL_BCSP),y)
MY_LIST += $(TL_PATH)/transport_bcsp_impl.c
endif

ifeq ($(CONFIG_TL_H4),y)
MY_LIST += $(TL_PATH)/transport_h4_impl.c
MY_LIST += $(TL_PATH)/hci_parser.c
endif

ifeq ($(CONFIG_TL_H5),y)
MY_LIST += $(TL_PATH)/transport_h5_impl.c
MY_LIST += $(TL_PATH)/hci_parser.c
endif
