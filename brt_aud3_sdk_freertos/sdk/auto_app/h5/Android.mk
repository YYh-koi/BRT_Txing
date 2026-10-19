TL_SRC_DIR := $(call my-dir)

TL_SRC_FILES :=  tl.c \
    bt_hci_bdroid.c  \
    bt_hw.c  \
    bt_list.c  \
    bt_skbuff.c  \
    hci_h5.c  \
    lpm.c  \
    userial.c  \
    utils.c 

#    gki_buffer.c

#module_patch
LOCAL_CFLAGS += -DHCI_USE_RTK_H5
LOCAL_CFLAGS += -DRTL_8723BS_BT_USED

