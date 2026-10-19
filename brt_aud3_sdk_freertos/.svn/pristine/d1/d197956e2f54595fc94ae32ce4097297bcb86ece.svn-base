/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/
#include "btdatatype.h"
#include "at_command.h"
#include <string.h>

/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     at_command.c
 * @Description:
        This file provides utilize functions to handle AT command.

 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-19
 ---------------------------------------------------------------------------*/


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        This function check the input buffer whether exists a completed AT command.
        If so, it will return the end pointer location of the command.
 *
 * @Arguments:
        buffer The input buffer to be checked
 * @Arguments:
        length The length of the input buffer
 *
 * @Return:
        The end pointer location of the AT command if found, 0 otherwise.
 */
/* ---------------------------------------------------------------------------*/
int at_command_check(unsigned char *buffer, int length)
{
	unsigned char *occur;

	if (!buffer || length <= 0) {
		return 0;
	}

	if ((occur = (unsigned char *)strstr((const char *) buffer, "AT#")) != NULL) {
		occur = buffer + 3; // length of "AT#"

		for (; occur < buffer + length - 1; occur++) {
			if (*occur == '\r' && *(occur  + 1) == '\n') {
				return occur + 2 - buffer;
			}
		}
	} else if ((occur = (unsigned char *)strstr((const char *) buffer, "D#")) != NULL) {
		occur = buffer + 2; // length of "D#"

		for (; occur < buffer + length - 1; occur++) {
			if (*occur == '\r' && *(occur  + 1) == '\n') {
				return occur + 2 - buffer;
			}
		}
	} else if ((occur = (unsigned char *)strstr((const char *) buffer, "L#")) != NULL) {
		occur = buffer + 2; // length of "L#"

		for (; occur < buffer + length - 1; occur++) {
			if (*occur == '\r' && *(occur  + 1) == '\n') {
				return occur + 2 - buffer;
			}
		}
	}

	return 0;
}

#if 0
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#ifdef SCM_RIGHTS

/* It seems various versions of glibc headers (i.e.
 *  *  * /usr/include/socketbits.h) miss one or more of these */

#ifndef CMSG_DATA
# define CMSG_DATA(cmsg) ((cmsg)->cmsg_data)
#endif

#ifndef CMSG_NXTHDR
# define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)
#endif

#ifndef CMSG_FIRSTHDR
# define CMSG_FIRSTHDR(mhdr) \
	((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)	       \
	 ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) NULL)
#endif

#ifndef CMSG_ALIGN
# define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
		& ~(sizeof (size_t) - 1))
#endif

#ifndef CMSG_SPACE
# define CMSG_SPACE(len) (CMSG_ALIGN (len) \
		+ CMSG_ALIGN (sizeof (struct cmsghdr)))
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))
#endif

union fdmsg {
	struct cmsghdr h;
	char buf[CMSG_SPACE(sizeof(int))];
};
#endif

int sendfd(int sock_fd, int send_me_fd)
{
	int ret = 13;
	struct iovec  iov[1];
	struct msghdr msg;

	iov[0].iov_base = &ret;
	iov[0].iov_len  = 1;

	msg.msg_iov     = iov;
	msg.msg_iovlen  = 1;
	msg.msg_name    = 0;
	msg.msg_namelen = 0;

	{
#ifdef SCM_RIGHTS
		/* New BSD 4.4 way (ouch, why does this have to be
		 * 		 * so convoluted). */

		union  fdmsg  cmsg;
		struct cmsghdr* h;

		msg.msg_control = cmsg.buf;
		msg.msg_controllen = sizeof(union fdmsg);
		msg.msg_flags = 0;

		h = CMSG_FIRSTHDR(&msg);
		h->cmsg_len   = CMSG_LEN(sizeof(int));
		h->cmsg_level = SOL_SOCKET;
		h->cmsg_type  = SCM_RIGHTS;
		*((int*)CMSG_DATA(h)) = send_me_fd;
#else
		/* Old BSD 4.3 way. Not tested. */
		msg.msg_accrights = &send_me_fd;
		msg.msg_accrightslen = sizeof(send_me_fd);
#endif	

		if (sendmsg(sock_fd, &msg, 0) < 0) {
			ret = 0;
		} else {
			ret = 1;
		}
	}
	/*fprintf(stderr,"send %d %d %d %d\n",sock_fd, send_me_fd, ret, errno);*/
	return ret;
}

int recvfd(int sock_fd)
{
	int count;
	int ret = 0;
	struct iovec  iov[1];
	struct msghdr msg;

	iov[0].iov_base = &ret;  /* don't receive any data */
	iov[0].iov_len  = 1;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	{
#ifdef SCM_RIGHTS
		union fdmsg  cmsg;
		struct cmsghdr* h;

		msg.msg_control = cmsg.buf;
		msg.msg_controllen = sizeof(union fdmsg);
		msg.msg_flags = 0;

		h = CMSG_FIRSTHDR(&msg);
		h->cmsg_len   = CMSG_LEN(sizeof(int));
		h->cmsg_level = SOL_SOCKET;  /* Linux does not set these */
		h->cmsg_type  = SCM_RIGHTS;  /* upon return */
		*((int*)CMSG_DATA(h)) = -1;

		if ((count = recvmsg(sock_fd, &msg, 0)) < 0) {
			ret = 0;
		} else {
			h = CMSG_FIRSTHDR(&msg);   /* can realloc? */
			if (   h == NULL
					|| h->cmsg_len    != CMSG_LEN(sizeof(int))
					|| h->cmsg_level  != SOL_SOCKET
					|| h->cmsg_type   != SCM_RIGHTS ) {
				/* This should really never happen */
				if (h)
					fprintf(stderr,
							"%s:%d: protocol failure: %d %d %d\n",
							__FILE__, __LINE__,
							h->cmsg_len,
							h->cmsg_level, h->cmsg_type);
				else
					fprintf(stderr,
							"%s:%d: protocol failure: NULL cmsghdr*\n",
							__FILE__, __LINE__);
				ret = 0;
			} else {
				ret = *((int*)CMSG_DATA(h));
				/*fprintf(stderr, "recv ok %d\n", ret);*/
			}
		}
#else
		int receive_fd;
		msg.msg_accrights = &receive_fd;
		msg.msg_accrightslen = sizeof(receive_fd);

		if (recvmsg(sock_fd, &msg, 0) < 0) {
			ret = 0;
		} else {
			ret = receive_fd;
		}
#endif
	}

	return ret;
}
#endif
