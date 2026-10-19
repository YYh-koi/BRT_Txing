include Android.mk

LOCAL_PATH := .
LOCAL_CFLAGS +=-g  -fno-strict-aliasing -fPIC $(CFLAGS_M32)
LOCAL_CFLAGS +=-Wno-comment -Wno-format -Wno-old-style-declaration -Wno-pointer-sign -Wno-unused-variable
LOCAL_CFLAGS +=-fsigned-char -Wno-error=address -Wno-error=sequence-point
ifneq ($(SYSROOT),)
	LOCAL_CFLAGS += --sysroot=$(SYSROOT)
endif

CFLAGS = $(addprefix -I , $(LOCAL_C_INCLUDES))
CFLAGS += $(LOCAL_CFLAGS)


LDFLAGS := \
	-L$(BRLINK_TOP_DIR)/stack \
	-L$(BRLINK_TOP_DIR)/sdk/auto_app \
	-L$(BRLINK_TOP_DIR)/sdk/auto_app/h5 \
	-L$(LOCAL_PATH)/../sink_hw \
	-L$(LOCAL_PATH)/../audio_utils/io/ \
	-L$(LOCAL_PATH)/../alsa/ \
	-L$(LOCAL_PATH)/../preprocessing_vcp8/

ifneq ($(LOCAL_SHARED_LIBRARIES),)
SHARED_LIBARIES :=$(subst lib,,$(LOCAL_SHARED_LIBRARIES))
LDLIBS += $(addprefix -l, $(SHARED_LIBARIES))
endif
LDLIBS += $(addprefix -l:, $(addsuffix .a, $(LOCAL_STATIC_LIBRARIES)))
LDLIBS += -lpthread -ldl -lbrlink -lrt

LDFLAGS += -Wl,--hash-style=both
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< 

OBJS := $(patsubst %.c,%.o, $(LOCAL_SRC_FILES))

$(LOCAL_MODULE):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

all:$(LOCAL_MODULE)

.PHONY: clean

clean:
	rm -f $(LOCAL_MODULE) $(OBJS)
