#ifndef _WIFI_H_
#define _WIFI_H_

#include "ip_addr.h"

#define SDIO_PACKET_SIZE 1024

#define AIC_WIFI_SIDO_INIT 		0x01
#define AIC_WIFI_RWNX_DRV_INIT 0x02

#define AIC_MAC_ADDR_LEN    6
#define AIC_MAX_SSID_LEN    32
#define AIC_MAX_PASSWD_LEN  64
#define AIC_MIN_KEY_LEN     8
#define AIC_MAX_KEY_LEN     64
#define AIC_MAX_AP_COUNT    20
#define AIC_MAX_STA_COUNT   4

#define ENABLE_WPS_PBC  //@martin 

#define VENDOR_IE "dd0B0017F20A00010400000000"
//#define VENDOR_IE "dd0800A0400000020022"  //apple
//#define VENDOR_IE "dd0600E04C020110"  //realtek
#define AIC_DEV_OUI 0x00A040 //APPLE specific ie, indication of carplay app support

#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define TRACE_IN() //printf("[%s][%d] in\n",  __FUNCTION__, __LINE__)
#define TRACE_OUT() //printf("[%s][%d] out\n",__FUNCTION__, __LINE__)
enum
{
    AIC_SUCCESS                      = 0,    /**< Success */
    AIC_ERROR                        = -1,   /**< Generic Error */
};

typedef enum
{
    WIFI_MODE_STA = 0,
    WIFI_MODE_AP,
    WIFI_MODE_P2P,
    WIFI_MODE_RFTEST,
    WIFI_MODE_UNKNOWN
}AIC_WIFI_MODE;


typedef enum {
    PRODUCT_ID_AIC8801 = 0,
    PRODUCT_ID_AIC8800DC,
    PRODUCT_ID_AIC8800DW,
    PRODUCT_ID_AIC8800D80,
    PRODUCT_ID_AIC8800D81,
}AICWF_IC;

typedef enum
{
    KEY_NONE = 0, KEY_WEP, KEY_WPA, KEY_WPA2, KEY_WPA3, KEY_MAX_VALUE
} AIC_WIFI_SECURITY_TYPE;

struct ap_ssid
{
    /// Actual length of the SSID.
    unsigned char length;
    /// Array containing the SSID name.
    unsigned char array[AIC_MAX_SSID_LEN];
};

struct ap_passwd
{
    unsigned char length;
    unsigned char array[AIC_MAX_PASSWD_LEN];
};

struct aic_ap_cfg
{
    struct ap_ssid aic_ap_ssid;
    /**
     * Password: if OPEN, set length to 0
     */
    struct ap_passwd aic_ap_passwd;
    /**
     * Band : 0 -> 2.4G, 1 -> 5G
     */
    unsigned char band;
    /**
     * Type : 0 -> 20M, 1 -> 40M
     */
    unsigned char type;
    /**
     * Channel Number : 2.4G (1 ~ 13), 5G (36/40/44/48/149/153/157/161/165)
     */
    unsigned char channel;
    /**
     * Hidden ssid : 0 -> no hidden, 1 -> zero length, 2 -> zero contents
     */
    unsigned char hidden_ssid;
    /**
     * Value for maximum station inactivity, seconds
     */
    unsigned int max_inactivity;
    /**
     * Enable wifi6
     */
    unsigned char enable_he;
    /**
     * Enable ACS
     */
    unsigned char enable_acs;
    /**
     * Beacon interval
     */
    unsigned char bcn_interval;

    AIC_WIFI_SECURITY_TYPE sercurity_type;
    /**
     * STA number: max(NX_REMOTE_STA_MAX)
     */
    unsigned char sta_num;
};

struct aic_sta_cfg
{
    struct ap_ssid aic_ap_ssid;
    /**
     * Password: if OPEN, set length to 0
     */
    struct ap_passwd aic_ap_passwd;
    int timeout_ms;
};

struct aic_p2p_cfg
{
    struct ap_ssid aic_p2p_ssid;
	/**
     * Password: if OPEN, set length to 0
     */
    struct ap_ssid aic_ap_ssid;
    struct ap_passwd aic_ap_passwd;
    /**
     * Band : 0 -> 2.4G, 1 -> 5G
     */
    unsigned char band;
    /**
     * Type : 0 -> 20M, 1 -> 40M
     */
    unsigned char type;
    /**
     * Channel Number : 2.4G (1 ~ 13), 5G (36/40/44/48/149/153/157/161/165)
     */
    unsigned char channel;
    /**
     * Enable wifi6
     */
    unsigned char enable_he;
    /**
     * Enable ACS
     */
    unsigned char enable_acs;
};

typedef enum
{
    SCAN_RESULT_EVENT = 0,
    SCAN_DONE_EVENT,
    JOIN_SUCCESS_EVENT,
    JOIN_FAIL_EVENT,
    LEAVE_RESULT_EVENT,
    ASSOC_IND_EVENT,
    DISASSOC_STA_IND_EVENT,
    DISASSOC_P2P_IND_EVENT,
    EAPOL_STA_FIN_EVENT,
    EAPOL_P2P_FIN_EVENT,
    PRO_DISC_REQ_EVENT,
    STA_DISCONNECT_EVENT,
    STA_DHCP_SUCCESS_EVENT,  //@pc.wang
    UNKNOWN_EVENT
} AIC_WIFI_EVENT;

#ifndef _AWS_WIFI_H_
/**
 * @brief Wi-Fi event types.
 */
typedef enum
{
    eWiFiEventReady,                 /**< Wi-Fi is initialized or was reset in the lower layer. */
    eWiFiEventScanDone,              /**< Scan is finished. */
    eWiFiEventConnected,             /**< Station is connected to the AP. */
    eWiFiEventDisconnected,          /**< Station is disconnected from the AP. */
    eWiFiEventConnectionFailed,      /**< Station connection has failed. */
    eWiFiEventAPStateChanged,        /**< SoftAP state changed. */
    eWiFiEventAPStationConnected,    /**< SoftAP got a new station. */
    eWiFiEventAPStationDisconnected, /**< SoftAP lost a new station. */
    eWiFiEventWPSSuccess,            /**< WPS is completed successfully. */
    eWiFiEventWPSFailed,             /**< WPS has failed. */
    eWiFiEventWPSTimeout,            /**< WPS has timeout. */
    eWiFiEventDHCPSuccess,           /**< DHCP is successful. */
    eWiFiEventMax
} WIFIEventType_t;

/**
 * @brief Max BSSID length
 */
#define wificonfigMAX_BSSID_LEN               ( 6 )

/**
 * @brief Wi-Fi protocol reason codes.
 */
typedef enum
{
    eWiFiReasonUnspecified,           /**< Unspecified error */
    eWiFiReasonAPNotFound,            /**< Cannot find the target AP. */
    eWiFiReasonAuthExpired,           /**< Previous auth invalid. */
    eWiFiReasonAuthLeaveBSS,          /**< Deauth leaving BSS. */
    eWiFiReasonAuthFailed,            /**< All other AUTH errors. */
    eWiFiReasonAssocExpired,          /**< Disassoc due to inactivity. */
    eWiFiReasonAssocTooMany,          /**< AP is overloaded. */
    eWiFiReasonAssocPowerCapBad,      /**< Power capability unacceptable. */
    eWiFiReasonAssocSupChanBad,       /**< Supported channel unacceptable. */
    eWiFiReasonAssocFailed,           /**< All other ASSOC errors. */
    eWiFiReasonIEInvalid,             /**< Management frame IE invalid. */
    eWiFiReasonMICFailure,            /**< MIC failure detected. */
    eWiFiReason4WayTimeout,           /**< 4WAY handshake timeout. */
    eWiFiReason4WayIEDiffer,          /**< 4WAY handshake IE error. */
    eWiFiReason4WayFailed,            /**< All other 4WAY errors. */
    eWiFiReasonAKMPInvalid,           /**< AKMP invalid. */
    eWiFiReasonPairwiseCipherInvalid, /**< Pairwise cipher invalid. */
    eWiFiReasonGroupCipherInvalid,    /**< Group cipher invalid. */
    eWiFiReasonRSNVersionInvalid,     /**< RSN version invalid. */
    eWiFiReasonRSNCapInvalid,         /**< RSN capability invalid. */
    eWiFiReasonGroupKeyUpdateTimeout, /**< Group key update timeout. */
    eWiFiReasonCipherSuiteRejected,   /**< Cipher violates security policy. */
    eWiFiReason8021XAuthFailed,       /**< 802.1X auth errors. */
    eWiFiReasonBeaconTimeout,         /**< Beacon timeout. */
    eWiFiReasonLinkFailed,            /**< All other link errors. */
    eWiFiReasonDHCPExpired,           /**< DHCP license expired. */
    eWiFiReasonDHCPFailed,            /**< All other DHCP errors. */
    eWiFiReasonMax
} WIFIReason_t;

/**
 * @brief Wi-Fi event info for AP got a connected station.
 */
typedef struct
{
    uint8_t ucMac[ wificonfigMAX_BSSID_LEN ]; /**< MAC address of connected station. */
} WiFiEventInfoAPStationConnected_t;

/**
 * @brief Wi-Fi event info for AP got a disconnected station.
 */
typedef struct
{
    uint8_t ucMac[ wificonfigMAX_BSSID_LEN ]; /**< MAC address of disconnected station. */
    WIFIReason_t xReason;                     /**< Reason code for the disconnection. */
} WiFiEventInfoAPStationDisconnected_t;

/**
 * @brief Wi-Fi combined event data structure.
 * added by pc.wang ,2024/6/12
 */
typedef struct
{
    WIFIEventType_t xEventType;
    // Can't use it temporarily, comment it out first . @pc.wang 2024/6/12
    
    union
    {
        #if 0
        WiFiEventInfoReady_t xReady;
        WiFiEventInfoScanDone_t xScanDone;
        WiFiEventInfoConnected_t xConnected;
        WiFiEventInfoIPReady_t xIPReady;
        WiFiEventInfoDisconnected_t xDisconnected;
        WiFiEventInfoConnectionFailed_t xConnectionFailed;
        WiFiEventInfoAPStateChanged_t xAPStateChanged;
        #endif 
        WiFiEventInfoAPStationConnected_t xAPStationConnected;
        WiFiEventInfoAPStationDisconnected_t xAPStationDisconnected;

        #if 0
        WiFiEventInfoRxDone_t xRxDone;
        WiFiEventInfoTxDone_t xTxDone;
        #endif 
    } xInfo;
} WIFIEvent_t;


/**
 * @brief Wi-Fi event handler definition.
 *
 * @param[in] xEvent - Wi-Fi event data structure.
 *
 * @return None.
 */
typedef void (* WIFIEventHandler_t)( WIFIEvent_t *  xEvent );

/**
 * Registers a callback function to handle specific Wi-Fi events.
 * 
 * This function allows the application to subscribe to Wi-Fi events of interest.
 * When a registered event occurs, the corresponding handler function will be invoked
 * to process the event data.
 *
 * @param xEventType [in] The type of Wi-Fi event to register for.
 *                     This must be one of the values from the WIFIEventType_t enumeration.
 * @param xHandler   [in] Pointer to the callback function that will handle the event.
 *                     The callback function must match the WIFIEventHandler_t prototype.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 *
 */
int WIFI_RegisterEvent( WIFIEventType_t xEventType, WIFIEventHandler_t xHandler );
#endif
//the end.


typedef struct _aic_wifi_scan_result_data
{
    unsigned int  reserved[16];
}aic_wifi_scan_result_data;

typedef struct _aic_wifi_scan_done_data
{
    unsigned int  reserved[1];
}aic_wifi_scan_done_data;

typedef struct _aic_wifi_join_data
{
    unsigned int  reserved[1];
}aic_wifi_join_data;

typedef struct _aic_wifi_leave_data
{
    unsigned int  reserved[1];
}aic_wifi_leave_data;

typedef struct _aic_wifi_auth_deauth_data
{
    unsigned int  reserved[6];
}aic_wifi_auth_deauth_data;

typedef struct _aic_wifi_event_data
{
    union
    {
        struct _aic_wifi_scan_result_data  scan_result_data;
        struct _aic_wifi_scan_done_data  scan_done_data;
        struct _aic_wifi_join_data  join_data;
        struct _aic_wifi_leave_data  leave_data;
        struct _aic_wifi_auth_deauth_data  auth_deauth_data;
    }data;
	#ifdef CONFIG_P2P
	unsigned int p2p_enabled;
	unsigned int p2p_dev_port_num;
	#endif
}aic_wifi_event_data;


/* Supported authentication mode. */
/* Values are used to select the authentication mode used to join a network. */
enum {
    WLAN_WPA_AUTH_DISABLED = 0x0000,    /* Legacy (i.e., non-WPA) */
    WLAN_WPA_AUTH_NONE = 0x0001,        /* none (IBSS) */
    WLAN_WPA_AUTH_PSK = 0x0004,        /* Pre-shared key */
    WLAN_WPA2_AUTH_PSK = 0x0080,        /* Pre-shared key */
    WLAN_MIX_AUTH_PSK = 0x0100,            /* Pre-shared key */
};


/* WLAN Security Encryption. */
/* Values are used to select the type of encryption used for testing. */
enum {
    WLAN_ENCRYPT_NONE = 0,    /* No encryption. */
    WLAN_ENCRYPT_WEP = 1,     /* WEP encryption. */
    WLAN_ENCRYPT_TKIP = 2,    /* TKIP encryption. */
    WLAN_ENCRYPT_AES = 4,     /* AES encryption. */
    WLAN_ENCRYPT_WSEC = 8,    /* Software WSEC encryption. */
    WLAN_ENCRYPT_WEP_SHARED = 0x11,      /* WEP shard encryption. */
    WLAN_ENCRYPT_FIPS = 0x80,  /* FIPS encryption. */
};

typedef int (* wifi_event_handle)(void *enData);

typedef void (* aic_wifi_event_cb)(AIC_WIFI_EVENT enEvent, aic_wifi_event_data *enData);

char *aic_wifi_get_version(void);
AIC_WIFI_MODE aic_wifi_get_mode(void);
void aic_wifi_reset(void);
void aic_wifi_set_mode(AIC_WIFI_MODE mode);

void aic_wifi_event_register(aic_wifi_event_cb cb);


int aic_wlan_start_sta(struct aic_sta_cfg *sta_cfg);
//int aic_wlan_start_ap(struct aic_ap_cfg *ap_cfg);
int aic_wlan_start_default_ap();//@pc.wang
void aic_wlan_set_default_ap_cfg(struct aic_ap_cfg *ap_cfg);

/**
 * @brief  This function triggers WIFI driver to start the AP mode.
 *  
 *@param[in]  ssid: A null terminated string containing the SSID name of the P2P.
 *@param[in]  passwd: A byte array containing the security key for the AP.
 *@param[in]  channel: 802.11 channel number.
 *@param[in]  security_type: The security type of the AP to start.
 * @return  AIC_SUCCESS: If success.
 * @return  AIC_ERROR: Otherwise.
 */
int aic_wlan_start_ap(const char *ssid, const char *passwd, int channel, AIC_WIFI_SECURITY_TYPE security_type);
/**
 * @brief  This function triggers WIFI driver to start the P2p mode.
 *  
 *@param[in]  dev_name: A null terminated string containing the device name of the P2P.
 *@param[in]  ssid: A null terminated string containing the SSID name of the P2P.
 *@param[in]  passwd: A byte array containing the security key for the P2P.
 * @return  AIC_SUCCESS: If success.
 * @return  AIC_ERROR: Otherwise.
 */
int aic_wlan_start_p2p(const char *dev_name, const char *ssid, const char *passwd);

void aic_p2p_set_addr(ip4_addr_t *ip,ip4_addr_t *mask,ip4_addr_t *gw);

int aic_get_p2p_param(struct aic_p2p_cfg *local_p2p_cfg);

int mmpAicWifiDriverRegister(void);
/**
 * @brief  This function checked if the wifi  is inited.
 *  
 * @param  None
 * @return  AIC_SUCCESS: if Initialization  is successful.
 * @return  AIC_ERROR: otherwise.
 */
int aic_wifi_inited(void);
/**
 * @brief  Enable Wi-Fi.
 * - Bring the Wireless interface "Up"
 * - Initialize the driver thread which arbitrates access
 *   to the SDIO/SPI bus
 *
 * @param[in]  mode: Decide to enable WiFi in which mode. The optional modes are enumerated in @ref AIC_WIFI_MODE.
 * @return  AIC_SUCCESS: if the WiFi chip was initialized successfully.
 * @return  AIC_ERROR: if the WiFi chip was not initialized successfully.
 */
int wifi_on(AIC_WIFI_MODE mode);
/**
 * @brief  Disable Wi-Fi.
 *  
 * @param  None
 * @return  AIC_SUCCESS: if deinitialization is successful.
 * @return  AIC_ERROR: otherwise.
 */
int wifi_off(void);
/**
 * @brief  Retrieve the RSSI value of the AP in STA mode.
 *  
 * @param  None
 * @return  The current RSSI value.
 */
int8_t aic_get_rssi();
/**
 * @brief  Retrieve the communication channel number corresponding to the Wi-Fi connection.
 *  
 * @param  None
 * @return  The current channel number.
 */
int16_t aic_get_channel();
/**
 * @brief  Retrieve the BSSID value (MAC address) of the connected AP.
 *  
 * @param  None
 * @return  The current BSSID value.
 */
#ifndef LMAC_MAC_H_
#define MAC_ADDR_LEN 6
typedef unsigned short u16_l;
struct mac_addr
{
    /// Array of 16-bit words that make up the MAC address.
    u16_l array[MAC_ADDR_LEN/2];
};
#endif
struct mac_addr *aic_get_bssid();
/**
 * @brief  Retrieve the SSID value of the connected AP.
 *  
 * @param  None
 * @return  The current SSID value.
 */
struct ap_ssid  *aic_get_ssid();
/**
 * @brief  Retrieve the current IPV4 address, subnet mask, and gateway in STA mode.
 *  
 *@param[out]  ip: IPv4 address
 *@param[out]  IPv4 network mask
 *@param[out]  IPv4 gateway address
 *@return  0 if requested parameters have been updated successfully and !=0 otherwise.
 */
int aic_wlan_sta_get_ip(unsigned int *ip, unsigned int *mask, unsigned int *gw);
/**
 * @brief  Connect to the specified AP.
 *  
 *@param[in]  ssid: The SSID of the target connection AP.
 *@param[in]  passwd: The password of the target connection AP.
 *@param[in]  timeout_ms: Connection timeout value, in milliseconds.
 * @return  AIC_SUCCESS: If the connection is successful.
 * @return  AIC_ERROR: otherwise.
 */
int wpa_sta_connect(const char *ssid, const char *passwd, int timeout_ms);
/**
 * @brief  Switch Wi-Fi to the specified mode.
 *  
  * @param[in]  mode: Decide to enable WiFi in which mode. The optional modes are enumerated in @ref AIC_WIFI_MODE.
 * @return  AIC_SUCCESS: If the switching action is successful.
 * @return  AIC_ERROR: otherwise.
 */
int aic_wifi_switch(int mode);
/**
 * @brief  Actively disconnect the STA from the connected AP.
 *  
 * @param  None
 * @return  AIC_SUCCESS: if disconnection is successful.
 * @return  AIC_ERROR: otherwise. 
 */
int wpa_sta_disconnect(void);
#endif

