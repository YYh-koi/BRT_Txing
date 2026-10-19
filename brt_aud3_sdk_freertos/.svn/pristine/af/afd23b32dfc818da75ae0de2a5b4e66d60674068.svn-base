#include "btdatatype.h"
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include "bse_timer.h"

#define BTHW_DBG(fmt,...) bse_log_print("audio_sink", "\r\n", fmt, ## __VA_ARGS__)

#define MAX_TIMER_NAME_LEN 32


typedef void (*timer_proc)(void *param);

struct bse_timer {
	char name[MAX_TIMER_NAME_LEN];
	int ticks; /* timer trigger 100 ticks per second */
	timer_proc proc;
	void *data;
	sem_t sem_loop_quit;
	unsigned char running;
	pthread_t thread;
};

static void *timer_execute_loop(void *arg);

bse_timer_t *bse_timer_create(const char *name, int ticks, timer_proc proc, void *data)
{
	bse_timer_t *tm = (bse_timer_t *) malloc(sizeof(bse_timer_t));

	if (tm == NULL) return tm;

	strncpy(tm->name, name, MAX_TIMER_NAME_LEN);
	tm->name[MAX_TIMER_NAME_LEN - 1] = '\0';
	tm->ticks = ticks;
	tm->proc = proc;
	tm->data = data;
	tm->running = 0;
	sem_init(&tm->sem_loop_quit, 0, 0);

	return tm;
}

void bse_timer_schedule(bse_timer_t *timer)
{
	if (timer == NULL) return;

	timer->running = 1;
	pthread_create(&timer->thread, NULL, timer_execute_loop, timer);
}

void bse_timer_cancel(bse_timer_t *timer)
{
	if (timer == NULL) return;

	timer->running = 0;
	sem_wait(&timer->sem_loop_quit);
	BTHW_DBG("timer %s quit.", timer->name);
}

void bse_timer_destroy(bse_timer_t *timer)
{
	if (timer == NULL) return;

	bse_timer_cancel(timer);
	sem_destroy(&timer->sem_loop_quit);
	free(timer);
}

static void *timer_execute_loop(void *arg)
{
	bse_timer_t *tm = (bse_timer_t *) arg;
	int timeout_ns = 0;
	struct timespec timeout;
	struct timespec previous = {0,0};
	struct timespec current;
	int err, skip;
	int delta_ns;
	int restart;

	if (tm == NULL) return NULL;

	BTHW_DBG("timer %s starting..", tm->name);

	/* Indicate that tick is just starting */
	restart = 1;
	while (tm->running) {
		/* Get time */
		clock_gettime(CLOCK_MONOTONIC, &current);

		if ((restart)) {
			/* Clear the restart indication */
			restart = 0;

			timeout_ns = (GKI_TICKS_TO_MS(tm->ticks) * 1000000);
		} else {
			/* Compute time elapsed since last sleep start */
			delta_ns = current.tv_nsec - previous.tv_nsec;
			delta_ns += (current.tv_sec - previous.tv_sec) * 1000000000;

			/* Compute next timeout:
			 *    timeout = (next theoretical expiration) - current time
			 *    timeout = (previous time + timeout + delay) - current time
			 *    timeout = timeout + delay - (current time - previous time)
			 *    timeout += delay - delta */
			timeout_ns += (GKI_TICKS_TO_MS(tm->ticks) * 1000000) - delta_ns;
		}
		/* Save the current time for next iteration */
		previous = current;

		timeout.tv_sec = 0;

		/* Sleep until next theoretical tick time.  In case of excessive
		   elapsed time since last theoretical tick expiration, it is
		   possible that the timeout value is negative.  To protect
		   against this error, we set minimum sleep time to 10% of the
		   tick period.  We indicate to compiler that this is unlikely to
		   happen (to help branch prediction) */

		skip = 0;
		if ((timeout_ns < ((GKI_TICKS_TO_MS(tm->ticks) * 1000000) * 0.1))) {
			timeout.tv_nsec = (GKI_TICKS_TO_MS(tm->ticks) * 1000000) * 0.1;

			/* Print error message if tick really got delayed
			   (more than 5 ticks) */
			if (timeout_ns < GKI_TICKS_TO_MS(-5) * 1000000) {
				/*BTHW_DBG("tick delayed > 5 slots (%d,%d) -- cpu overload ? ",
				         timeout_ns, GKI_TICKS_TO_MS(-5) * 1000000);*/
				skip = 1;
			}
		} else {
			timeout.tv_nsec = timeout_ns;
		}

		if (!skip) {
			do {
				/* [u]sleep can't be used because it uses SIGALRM */
				err = nanosleep(&timeout, &timeout);
			} while (err < 0 && errno == EINTR);
		}

		tm->proc(tm->data);
	}

	sem_post(&tm->sem_loop_quit);

	return NULL;
}

