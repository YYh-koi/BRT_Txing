/*************************************************************************
    > File Name: portings/bt_dhcp.c
    >
    > Mail: sheldon_wy@126.com
    > Created Time: Wed 01 Jul 2015 03:40:49 PM CST
 ************************************************************************/

#include<stdio.h>

#include <arpa/inet.h>

#include <net/if.h>
#include <linux/sockios.h>
#include <sys/prctl.h>
#include <linux/if.h>
#include <linux/if_ether.h>

static void ifc_init_ifr(const char *name, struct ifreq *ifr)
{
	memset(ifr, 0, sizeof(struct ifreq));
	strncpy(ifr->ifr_name, name, IFNAMSIZ);
	ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

static void init_sockaddr_in(struct sockaddr *sa, in_addr_t addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *) sa;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = addr;
}


int ifc_set_addr(const char *name, in_addr_t addr)
{
	struct ifreq ifr;
	int ret;

	ifc_init_ifr(name, &ifr);
	init_sockaddr_in(&ifr.ifr_addr, addr);

	ret = ioctl(ifc_ctl_sock, SIOCSIFADDR, &ifr);
	if (DBG) DBG_PRINT("ifc_set_addr(%s, xx) = %d", name, ret);
	return ret;
}


int do_dhcp(char *iname)
{
	if (ifc_set_addr(iname, 0)) {
		printerr("failed to set ip addr for %s to 0.0.0.0: %s\n", iname, strerror(errno));
		return -1;
	}

	if (ifc_up(iname)) {
		printerr("failed to bring up interface %s: %s\n", iname, strerror(errno));
		return -1;
	}

	return dhcp_init_ifc(iname);
}

