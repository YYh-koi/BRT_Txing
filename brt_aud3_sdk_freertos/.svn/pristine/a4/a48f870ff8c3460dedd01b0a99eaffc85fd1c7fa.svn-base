#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "bt_pty.h"
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("brlinkd", fmt, ## __VA_ARGS__)


int pty_allocate(int *ptyfd, int *ttyfd, char *namebuf, int namebuflen)
{
	struct termios tio;

	/* Unix 98-style pty code. */
	*ptyfd = open("/dev/ptmx", O_RDWR | O_NOCTTY);

	if (*ptyfd < 0) {
		/* BSD-style pty code. */
		char buf[64];
		int i;
		const char *ptymajors = "pqrstuvwxyzabcdefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
		const char *ptyminors = "0123456789abcdef";

		int num_minors = strlen(ptyminors);
		int num_ptys = strlen(ptymajors) * num_minors;

		for (i = 0; i < num_ptys; i++) {
			snprintf(buf, sizeof buf, "/dev/pty%c%c", ptymajors[i / num_minors],
			         ptyminors[i % num_minors]);
			snprintf(namebuf, namebuflen, "/dev/tty%c%c",
			         ptymajors[i / num_minors], ptyminors[i % num_minors]);

			*ptyfd = open(buf, O_RDWR | O_NOCTTY);

			if (*ptyfd < 0) {
				/* Try SCO style naming */
				snprintf(buf, sizeof buf, "/dev/ptyp%d", i);
				snprintf(namebuf, namebuflen, "/dev/ttyp%d", i);
				*ptyfd = open(buf, O_RDWR | O_NOCTTY);

				if (*ptyfd < 0) {
					continue;
				}
			}

			/* *ptyfd > 0 */
			break;
		}
	}

	if (*ptyfd > 0) {
		char * name = ptsname(*ptyfd);
		if (name) {
#ifdef USER_DEFINED_PSEUDO_TTY 
			LOG("USER_DEFINED_PSEUDO_TTY open pty %s success.", name);
			strncpy(namebuf, name, namebuflen);
			chmod(namebuf, 0666);

				if (access(USER_DEFINED_PSEUDO_TTY , 0) == 0) {
					unlink(USER_DEFINED_PSEUDO_TTY );
				}
			remove(USER_DEFINED_PSEUDO_TTY);
			symlink(namebuf, USER_DEFINED_PSEUDO_TTY );
			char uartval[50] = {0};
#else
			LOG("open pty %s success.", name);
			strncpy(namebuf, name, namebuflen);
			chmod(namebuf, 0666);

			if (get_port_name(uartval, 50)) {
				if (access(uartval, 0) == 0) {
					unlink(uartval);
				}
				symlink(namebuf, uartval);
			}
#endif
		}

		if (grantpt(*ptyfd) == -1 || unlockpt(*ptyfd) == -1) {
			LOG("grant or unlock pty failed:%s.", strerror(errno));
			return 0;
		}

		/* Set raw attributes on the pty. */
		tcgetattr(*ptyfd, &tio);
		cfmakeraw(&tio);
		tcsetattr(*ptyfd, TCSAFLUSH, &tio);
		return 1;
	}

	LOG("%s", "failed to open any /dev/pty?? devices");

	return 0;
}

