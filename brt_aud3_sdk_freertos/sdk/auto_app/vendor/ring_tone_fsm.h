#ifndef _RINGTONE_FI_H
#define _RINGTONE_FI_H

enum {
    ST_RING_IDLE,
    ST_RING_PLAYING
};

#define RING_FSM_STATE_COUNT (ST_RING_PLAYING+1)

enum {
    EV_RING_STARTED = 1,
    EV_RING_PLAY,
    EV_RING_END,
    EV_RING_STOPED,
    EV_RING_UI_START,
    EV_RING_UI_STOP,
    EV_RING_DONE
};

struct RingFsmUserDataStru {
	unsigned long dwTotal;
	unsigned long dwSent;
	unsigned char *ringdata;
};

#define RING_FSM_EVENT_COUNT EV_RING_DONE
#define RING_FSM_FUNC_COUNT (sizeof(ring_fsm_function_list)/sizeof(struct FsmNode))

extern struct FsmInst *g_ring_fsminst;
#define RING_FI_USER_DATA	((struct RingFsmUserDataStru *)(g_ring_fsminst->user_data))

void InitRingFI(void);

#endif
