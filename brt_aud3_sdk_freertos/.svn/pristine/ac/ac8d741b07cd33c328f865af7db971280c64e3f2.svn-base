

#ifndef _BSE_TIMER_H__
#define _BSE_TIMER_H__

#define TICKS_PER_SEC 100
#define GKI_TICKS_TO_MS(x)   ((x) * 1000 / TICKS_PER_SEC)

struct bse_timer;
typedef struct bse_timer bse_timer_t;

bse_timer_t *bse_timer_create(const char *name, int ticks, void (*timer_proc)(void *param), void *data);
void bse_timer_schedule(bse_timer_t *timer);
void bse_timer_cancel(bse_timer_t *timer);
void bse_timer_destroy(bse_timer_t *timer);

#endif  /*_BSE_TIMER_H__*/
