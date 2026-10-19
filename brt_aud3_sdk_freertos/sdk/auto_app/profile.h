#ifndef _PROFILE_H__
#define _PROFILE_H__

/* The Function Pointer Format Module Event Callback */
typedef void (*module_event_cbk_t)(unsigned int event, void *param);

enum {
    PROFILE_DISABLE = 0,
    PROFILE_ENABLE,
};

enum tag_profile_id {
    PROFILE_ID_MGR,             /* 0*/
    PROFILE_ID_GAP,             /* 1*/
    PROFILE_ID_HFP,             /* 2*/
    PROFILE_ID_A2DP,            /* 3*/
    PROFILE_ID_AVRCP,           /* 4*/
    PROFILE_ID_PBAP,            /* 5*/
    PROFILE_ID_HID,             /* 6*/
    PROFILE_ID_CBP,             /* 7*/
    PROFILE_ID_SPP,             /* 8*/
    PROFILE_ID_FTP,             /* 9*/
    PROFILE_ID_DUN,             /*10*/
    PROFILE_ID_GATT,            /*11*/
    PROFILE_ID_OPP,             /*12*/
    PROFILE_ID_MAP,             /*13*/
    PROFILE_ID_PAN,             /*14*/
    /* ACL is NOT profile. Just for delivering disconnect event in unify method.*/
    PROFILE_ID_ACL,             /*15*/
    PROFILE_ID_ANCS,            /*16*/
    PROFILE_ID_INVALID          /*17*/
};

#define MAX_PROFILE_NUM PROFILE_ID_INVALID

typedef struct tag_profile {
	void (*init)(module_event_cbk_t event_cbk);
	void (*done)();
	void (*start)();
	void (*stop)();
} profile_t;

#endif  /*_PROFILE_H__*/
