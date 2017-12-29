#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include "netlink.h"

int main (int argc, char *argv[]) {
	char buf[64] = {0};
	class Netlink *netlink;
	netlink = new struct Netlink ;

	struct sockaddr_nl *addr = new struct sockaddr_nl;
	memset (addr, 0, sizeof (struct sockaddr_nl));
	addr->nl_family = AF_NETLINK;
	addr->nl_pid = 100;
	addr->nl_groups = 0;

	if (netlink->netlink_create () < 0) {
		return -1;
	}
	
	netlink->netlink_bind (addr);

	netlink->netlink_send_data ("Hello kernel", 
				strlen ("Hello kernel") + 1, 0, 0);
	netlink->netlink_recive_data (buf, 64);
	printf ("user: %s \n", buf);
	delete netlink;

	return 0;
}
