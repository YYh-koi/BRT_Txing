/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED

/* Define the version for stack */
#define CONFIG_STACK85

/*
 * System dependent Selection
 */
#define CONFIG_GNUMAKE 1
#define CONFIG_CHANGE_VERSION 1
#define CONFIG_MAJOR_VERSION 0x8
#define CONFIG_MINOR_VERSION 0x5
#define CONFIG_PATCH_LEVEL 0x0
#define CONFIG_BUILD_RELEASE 0x20121018
#define CONFIG_DEBUG_SYMBOL 1
#define CONFIG_BT_DEBUG
#undef CONFIG_DEBUG_SIMPLE
#undef  CONFIG_GNU
#undef CONFIG_MICROSOFT
#undef  CONFIG_BORLAND
#undef  CONFIG_ARM
#undef  CONFIG_ARMTCC
#undef  CONFIG_DIABDCC
#undef  CONFIG_HITACHI
//#undef  CONFIG_OS_WIN32
#undef  CONFIG_OS_VXWORKS
#undef  CONFIG_OS_REX
#undef  CONFIG_OS_PSOS
//#undef  CONFIG_OS_WINCE
/*CONFIG_OS_LINUX should also be defined in Android platform*/
#undef CONFIG_OS_LINUX
#undef  CONFIG_OS_NUCLEUS
#undef  CONFIG_OS_UC
#undef  CONFIG_OS_RTK
#undef  CONFIG_OS_OSE
#undef  CONFIG_OS_HOPEN
#undef  CONFIG_OS_GKI
#undef  CONFIG_OS_CFW_ST
#undef  CONFIG_OS_SIW
#undef  CONFIG_OS_ZEEVO
#undef  CONFIG_OS_NONE
#undef CONFIG_OS_ANDROID
#undef  CONFIG_CONFORMANCE_TESTER
#undef  CONFIG_INTER_TESTER
#undef  CONFIG_ANALYZER
#undef  CONFIG_APP_CREATE_THREADS
#undef  CONFIG_BLUELET_SDK
#define CONFIG_LITTLE_ENDIAN 1
#undef  CONFIG_BIG_ENDIAN
#undef  CONFIG_ALIGN_ONEBYTE
#undef  CONFIG_ALIGN_TWOBYTES
#define CONFIG_ALIGN_FOURBYTES 1
#undef  CONFIG_SUB_STRUCTURE_ALIGNED_FIXED
#undef  CONFIG_TYPE_NO_LIMIT
#define CONFIG_TYPE_BIT32_ONLY 1
#undef  CONFIG_TYPE_BIT16_ONLY
#undef CONFIG_MEMORY_STATIC
#undef CONFIG_MEMORY_LEAK_DETECT
#undef  CONFIG_REPORT_DEVICE_WITH_NAME
#undef  CONFIG_UNLIMIT_RESOURCE
//#define CONFIG_LIMIT_FREQUENT_CALLED

#undef  CONFIG_CHECK_STACK
#define CONFIG_BLOCK_SIZE (16)
#define CONFIG_BLOCK_NUMBER (4000)
#define CONFIG_USER_POOL_SIZE (32768)
#undef CONFIG_CPU_BIT64
#define CONFIG_CPU_BIT32
#undef  CONFIG_CPU_BIT16
#undef  CONFIG_FUNC_LIST
#define CONFIG_QUEUE_MAX (64)
#define CONFIG_GAP 1
#define GAP_IO_CAPABILITIES_FULL
//#undef GAP_IO_CAPABILITIES_FULL
//if defined this macro: Notify the user to reply the pincode req.
#undef  GAP_USE_INPUT_PINCODE
#ifndef CONFIG_CARLIFE_CERTIFICATE
//if defined this macro: NO user intervention in the user-confirmation process.
//#define GAP_USER_OPERATE
#undef GAP_USER_OPERATE
#endif
#undef CONFIG_CONNECTION_ATHENTICATE_REQUIRED
/*
 * HCI Configuration
 */
#undef  CONFIG_HCI_10A
#undef  CONFIG_HCI_10B
#undef  CONFIG_HCI_11
#undef  CONFIG_HCI_12
#undef  CONFIG_HCI_21
#define CONFIG_HCI_30 1
#define CONFIG_HCI_STANDARD 1
#define CONFIG_TRANSPORT_DLL 1
#define CONFIG_HCI_INI 1
#define CONFIG_HCI_DFLT_ASYN_TM (10000)
#define CONFIG_HCI_DFLT_SYN_TM (2000)
#define CONFIG_HCI_DFLT_LKSUPER_TM (20000)
#define CONFIG_HCI_CONN_ACCEPT_TIMEOUT (29000)
#define CONFIG_HCI_ACL_BUFF_SIZE (1024000)
#define CONFIG_HCI_SCO_BUFF_SIZE (5120)
#undef  CONFIG_HC2HOST_FC_ENABLE
#undef  CONFIG_HCI_NO_SCO_DATA
#define CONFIG_HCI_RESET_ENABLE 1
#undef  CONFIG_HCI_PAGESCAN_ENABLE
/*
 * L2cap Configuration
 */
#define CONFIG_BLUELET 1
#undef  CONFIG_L2CAP_REDUCED_VER
#define CONFIG_PINGINFO_SUPPORT 1
#define CONFIG_GROUP 1
#undef  CONFIG_RESEND_SUPPORT
#define CONFIG_L2CAP_RTX_TIMEOUT (5000)
#define CONFIG_L2CAP_ERTX_TIMEOUT (60000)
#define CONFIG_L2CAP_HCI_DISCONN_TO (1000)
#define CONFIG_L2CAP_TIMEOUT (60000)
#define CONFIG_L2CAP_MAX_CHANNEL_NUM (100)
/*
 * SDP Configuration
 */
#undef  CONFIG_SDP_FIXED_DATABASE

/*
 * profile manager
 * */
#undef CONFIG_MGR_STATIC
#undef CONFIG_MGR_DYNAMIC

/*
 * Rfcomm Configuration
 */
#define CONFIG_RFCOMM 1
#define CONFIG_CREDIT (4)
#define CONFIG_RFCOMM_DEFAULT_MTU (1600)
#define CONFIG_RFCOMM_SYS_N1 (1012)
#define CONFIG_RFCOMM_SYS_T1 (10000)
#define CONFIG_RFCOMM_SYS_T2 (10000)

/*
* Other functions
*/
#define CONFIG_OTHER
/*
 * BLE Configuration
 */
#define CONFIG_LE
/*
 * ATT Configuration
 */
#define CONFIG_ATT
#define CONFIG_GATT
//#undef CONFIG_LE_SINGLE_MODE
#define CONFIG_LE_SINGLE_MODE
/*
 * AVCTP Configuration
 */
#define CONFIG_AVCTP 1
/*
 * AVRCP Configuration
 */
#define CONFIG_AVRCP 1
#undef  CONFIG_AVRCP_10
#define CONFIG_AVRCP_14 1
/*
 * AVDTP Configuration
 */
#undef  CONFIG_GAVDP
#define CONFIG_AVDTP 1
#define CONFIG_AVDTP_TRANSACTION_RTX_TIMEOUT (5000)
/*
 * A2DP Configuration
 */
#define CONFIG_A2DP 1
#undef CONFIG_A2DP_SRC
#define CONFIG_A2DP_SNK 1
#undef  CONFIG_VAUDIO
#undef  CONFIG_A2DPUTIL_AAC
#define CONFIG_A2DP_DEFAULT_MTU (672)
#define CONFIG_A2DPAPP_WAVEHEADER_NUM (20)
/*
 * VDP Configuration
 */
#undef  CONFIG_VDP
#define CONFIG_VDP_DEFAULT_MTU (6000)
/*
 * Hcrp Configuration
 */
#undef  CONFIG_HCRP
/*
 * Personal Area Networking
 */
//#undef CONFIG_PAN
//#undef CONFIG_BNEP 1
//#undef CONFIG_PAN_CLIENT 1
//#undef CONFIG_PAN_SERVER 1
/*
 * SDAP and Service Database Management
 */
#define CONFIG_SDAP 1
#define CONFIG_SDAP_SERVER 1
#define CONFIG_SDAP_CLIENT 1
#define CONFIG_SDAP_COMPLETE_VERSION 1
#define CONFIG_DI_SERVER 1
#define CONFIG_DI_CLIENT 1

/*
 * Serial port profile
 */
#define CONFIG_SPP
#undef  CONFIG_SPP_VSERIAL
#define CONFIG_SPP_CLIENT 1
#define CONFIG_SPP_SERVER 1
#define CONFIG_SPP_STANDARD_SERVER
//#define CONFIG_ANDROID_VERSION 60

/*CONFIG_SPP_ANDROID depens on CONFIG_SPP
 */
#undef CONFIG_SPP_ANDROID

/*for Carplay*/
#define CONFIG_IAP

/*for AndroidAuto*/
#define CONFIG_AAP

#define CONFIG_EC
#define CONFIG_EY

/*
 * Hands Free profile
 */
#define CONFIG_HFP 1
#undef CONFIG_HFP_AG
#define CONFIG_HFP_HF 1
#define CONFIG_HFP_ATCMD_TIMEOUT_SHORT (3000)
#define CONFIG_HFP_ATCMD_TIMEOUT_LONG (6000)
#define CONFIG_HFP_HF_RING_TIMEOUT (6000)
#define CONFIG_HFP_AG_RING_TIMEOUT (3000)
#define CONFIG_HFP_AG_TBUSY_TIMEOUT (5000)
#undef  CONFIG_HFP_MSBC_ENABLED
#define CONFIG_HF_SUPPORT_3_WAY_CALL

/*
 * SIM Access profile
 */
#undef  CONFIG_SAP
/*
 * Dial up networking profile
 */
#undef  CONFIG_DUN
/*
 * FAX profile
 */
#undef  CONFIG_FAX
/*
 * Generic OBEX
 */
#define CONFIG_GOEP 1
#define CONFIG_OBEX 1
#define CONFIG_GOEP_MAX_RECV_LEN (32328)
#define CONFIG_GOEP_MAX_SEND_LEN (2048)
#define CONFIG_GOEP_CLIENT 1
#define CONFIG_GOEP_SERVER 1
#define CONFIG_OBEX_CLIENT 1
#define CONFIG_OBEX_SERVER 1
/*
 * OBEX Basic Imaging
 */
#define  CONFIG_BIP
#define CONFIG_BIP_CLIENT
#define CONFIG_BIP_XML_PARSER 1
#define CONFIG_BIP_RECV_MOPL    64768
/*
 * OBEX object push
 */
#undef CONFIG_OPP
#define CONFIG_OPP_CLIENT
#define CONFIG_OPP_SERVER
#define CONFIG_OPP_MAX_SERVICES 1
#define CONFIG_OPP_RECV_MOPL    64768
/* This is enable when you use static memery */
#define CONFIG_OPP_SEND_MOPL    4096
/*
 * OBEX file transfer
 */
#undef  CONFIG_FTP
#undef  CONFIG_FTP_CLIENT
#define CONFIG_FTP_RECV_MOPL (64768)
#define CONFIG_FTP_MAX_CACHED_FILE_NAMES_LENGTH (0)
#define CONFIG_INTERNAL_XML_PARSER 1
/*
 * OBEX Basic Printing
 */
#undef  CONFIG_BPP
/*
 * OBEX Phone Book Access
 */
#define CONFIG_PBAP 1
#define CONFIG_PBAP_CLIENT 1
#undef CONFIG_PBAP_SERVER
#define CONFIG_PBAP_RECV_MOPL 4096
#undef CONFIG_PBAP_FILTER_PHOTO
//#define CONFIG_PBAP_RAWDATA
/*
 * IrMC Configuration
 */
#undef  CONFIG_IRMC
#define CONFIG_IRMC_VCARD 1
#define CONFIG_IRMC_VCARD_SIZE (15)
#define CONFIG_IRMC_VCARD_MAPSIZE (2500)
#define CONFIG_IRMC_VCARD_LOGSIZE (2500)
#define CONFIG_IRMC_VCAL 1
#define CONFIG_IRMC_VCAL_SIZE (15)
#define CONFIG_IRMC_VCAL_MAPSIZE (100)
#define CONFIG_IRMC_VCAL_LOGSIZE (100)
#define CONFIG_IRMC_VMSG 1
#define CONFIG_IRMC_VMSG_SIZE (15)
#define CONFIG_IRMC_VMSG_MAPSIZE (2500)
#define CONFIG_IRMC_VMSG_LOGSIZE (2500)
#define CONFIG_IRMC_VNOTE 1
#define CONFIG_IRMC_VNOTE_SIZE (15)
#define CONFIG_IRMC_VNOTE_MAPSIZE (100)
#define CONFIG_IRMC_VNOTE_LOGSIZE (100)
/*
 * OBEX Message Access
 */
#undef CONFIG_MAP
#define CONFIG_MAP_MCE 1
#undef  CONFIG_MAP_MSE
#define CONFIG_MAP_RECV_MOPL (512)
//#define CONFIG_MAP_RECV_MOPL (32328)
/*
 * Human Interface Device profile
 */
#define CONFIG_HID
#undef  CONFIG_HID_CLIENT
#define CONFIG_HID_SERVER 1
#define CONFIG_HID_HOST_DEFAULT_MTU (672)
#define CONFIG_HID_DEV_DEFAULT_MTU (48)
#undef  CONFIG_HID_NT_SERVICE
#define CONFIG_HID_SAR 1
#define CONFIG_HID_QOS 1
/*
 * SOC Configuration
 */
#undef  CONFIG_SOC
#undef  CONFIG_HCI_AVNET
#undef  CONFIG_HCI_ALCATEL
#undef  CONFIG_HCI_ZEEVO
#undef  CONFIG_HCI_NSC
#undef  CONFIG_HCI_STM
#undef  CONFIG_HCI_SILICONWAVE
/*
 * SOC_BACI Configuration
 */
#undef  CONFIG_BACI
/*
 * NAT and DHCP Function
 */
#undef  CONFIG_NAT
#undef  CONFIG_DHCP
/*
 * Bluetooth SDK APIs
 */
#undef  CONFIG_BTSDK
#define CONFIG_BTSDK_OWNTHREAD 1
#define CONFIG_BTSDK_STORE_INFO 1
#define CONFIG_BTSDK_STORE_LOCAL_SERVICE 1
#undef  CONFIG_BTSDK_AT_CMD
#define CONFIG_BTSDK_FLOW_STATISTIC 1
#define CONFIG_BTSDK_REPORT_PRINT 1
#define CONFIG_BTSDK_AUTHORITY_CHECK 1
#undef  CONFIG_GOEP_FILE_IO
#define CONFIG_BTSDK_HFPHF_SERVER_CHANNEL (7)
#define CONFIG_BTSDK_HFPAG_SERVER_CHANNEL (8)
#define CONFIG_BTSDK_HS_SERVER_CHANNEL (9)
#define CONFIG_BTSDK_VG_SERVER_CHANNEL (10)
#define CONFIG_BTSDK_FTP_SERVER_CHANNEL (11)
#define CONFIG_BTSDK_OPP_SERVER_CHANNEL (12)
#define CONFIG_BTSDK_IMG_SERVER_CHANNEL (13)
#define CONFIG_BTSDK_BPP_PRINTER_JOB_CHANNEL (14)
#define CONFIG_BTSDK_BPP_PRINTER_STATUS_CHANNEL (15)
#define CONFIG_BTSDK_BPP_RUI_SERVER_CHANNEL (16)
#define CONFIG_BTSDK_BPP_OBJECT_SERVER_CHANNEL (17)
#define CONFIG_BTSDK_BPP_OBJECT_JOB_CHANNEL (18)
#define CONFIG_BTSDK_PBAP_SERVER_CHANNEL (19)
#define CONFIG_BTSDK_SYNC_SERVER_CHANNEL (20)
#define CONFIG_BTSDK_SYNC_CMD_SERVER_CHANNEL (21)
#define CONFIG_BTSDK_FTP_PSM (4115)
#define CONFIG_BTSDK_OPP_PSM (4117)
#define CONFIG_BTSDK_IMG_PSM (4119)

//If you want to support the IAP2(Interface accessory protocol) of Apple, you need define this macro.
//#define CONFIG_IAP2
//HID mouse Calibrate Screen function enable
//#define CONFIG_CBP
//PIM through AT
//#define CONFIG_ATPIM

//#define CONFIG_SAVE_INFO_TO_MODULE	//If I140 module used,the devices and other infos can be stored in EEPROM of module,but the size if limited
//User who want to store more infos should disable this MACRO,and implement the ReadFrom and WriteTo interface in PollingPorting
#define CONFIG_SETTING_BY_USER			//User who want to maintain the Setings of BT(such as PinCode/Local Device name),enable this MACRO and use AT Command to set every time BT Started
//It is use to reduce the Flash opeartor while BT Started

//RDA Configure
//#define CONFIG_BOARD_RDA
#undef CONFIG_A2DP_SRC

//Roadrover Configure
//Roadrover will input HID report desc,AT Command will be long,other project do not use it.
#define CONFIG_LONG_AT
#define CONFIG_UNLIMIT_RESOURCE
#define AT_EVENT_MAX_SIZE 512
//Fix me:HID connect fail if the COD is Audio device,it should be set to HID device class,but if it is HID device,iphone can not display it.
//#define CONDIF_COD_MOUSE

//BT Chip Configure
#undef CONFIG_BT_CHIP_ISSC
#undef CONFIG_BT_CHIP_CSR
#undef CONFIG_BT_CHIP_CSR8350
#undef CONFIG_BT_CHIP_BCM
#undef CONFIG_BT_CHIP_QCOM
#define CONFIG_BT_CHIP_RTK

#undef USEFSM_CONFIG_8350DSP

#undef CONFIG_HARDWARE_POWER_CTL
//System to support the A2DP data format
#define CONFIG_A2DP_DATA_SYSTEM_SUPPORT_SBC		(0)
#define CONFIG_A2DP_DATA_SYSTEM_SUPPORT_PCM		(1)

//The treatment of Audio Data Stream
//If defines these options, means that the system plotform need to deal with audio data
#define CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_SCO
#define CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP CONFIG_A2DP_DATA_SYSTEM_SUPPORT_PCM
//#define CHECK_EMPTY_DATA 1
#define CONFIG_BT_AUDIO_CBK

//TL Configure
#undef CONFIG_TL_H4
//#define CONFIG_TL_H4
#define CONFIG_TL_FW
//#define CONFIG_TL_BCSP
//#define  CONFIG_TL_USB
#define CONFIG_TL_H5

// Choose a specified project
#undef CONFIG_PROJECT_BC6
#undef CONFIG_PROJECT_ROSEN
#undef CONFIG_PROJECT_XXXX
#undef CONFIG_PROJECT_POLYCOM
#undef CONFIG_PROJECT_PATE0

#define CONFIG_SCO_OVER_UART

#undef CONFIG_BRT_AEC_PROCESSING

//temp
#ifdef CONFIG_NOT_SUPPORT_FUNC
#define __FUNCTION__ "test_func"
#endif

// If the controller support power on/off, defines this marco.
#undef MODULE_SUPPORT_HARDWARE_PM_RESET
// If the bonded devices should be saved into config file, define this macro.
#undef SAVE_BONDED_DEVICES_TO_CONFIG_FILE

//This macro must be defined when use local buffer to buffer HCI raw data
//#define CONFIG_BT_TL_LOCAL_BUFFER
//#define CONFIG_CHECK_LICENSE
//#define LICENSE_BY_NETWORK
//#define LICENSE_BY_CHIP

#define CONFIG_ABS_VOL

#define CONFIG_BIND_PROFILES

#define CONFIG_ANCS_CLIENT

/* Configure the power management method of module */
#undef CONFIG_PM_USING_RFKILL
#undef CONFIG_PM_USING_GPIO

//E513 Configure
#undef CONFIG_DEFINE_BT_POWER 
#undef CONFIG_DEFINE_BT_EVENT 

#undef BT_POWER_CONFIG

#undef CONFIG_OPEN_FILTER_DATA
//#undef CONFIG_OPEN_FILTER_DATA

#define  CONFIG_FLASH_SAVE_FILE
