TL_MODULE := uartif

ifeq ($(CONFIG_TL_USB),y)
    TL_MODULE := usbif
else
    ifeq ($(CONFIG_TL_H5),y)
        TL_MODULE := h5if
        PORTINGS_INCLUDES += ./tl/$(TL_MODULE)/include
#BCSP is based on H4 and the code is in auto_app
    endif
endif


PORTINGS_FILES += ./tl/upio.c
ifeq ($(CONFIG_PM_USING_RFKILL),y)
    PORTINGS_FILES += ./tl/rfkill.c
else
    ifeq ($(CONFIG_PM_USING_GPIO),y)
        PORTINGS_FILES += ./tl/gpio.c
    endif
endif

include $(LOCAL_PATH)/tl/$(TL_MODULE)/Android.mk
PORTINGS_FILES += $(addprefix ./tl/$(TL_MODULE)/, $(TL_SRC_FILES))

