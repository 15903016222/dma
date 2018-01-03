#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#include "save_dma_data.h"

#define NETLINK_TEST    (25)
#define TEST_PID        (100)

static int sock_fd = -1;

int netlink_create_socket (void)
{
    //create a socket
    return socket (AF_NETLINK, SOCK_RAW, NETLINK_TEST);
}

int netlink_bind (void)
{
    struct sockaddr_nl addr;

    memset (&addr, 0, sizeof (struct sockaddr_nl));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = TEST_PID;
    addr.nl_groups = 0;

    return bind (sock_fd, (struct sockaddr *)&addr, sizeof (struct sockaddr_nl));
}
/* recive */
static struct nlmsghdr *nlh_recv = NULL;
static struct sockaddr_nl source_addr_recv;
static struct iovec iov_recv;
static struct msghdr msg_recv;

int netlink_recv_config (void)
{
    //create message
    nlh_recv = (struct nlmsghdr *)malloc (NLMSG_SPACE (0));
    if (!nlh_recv) {
        perror ("malloc");
        return -1;
    }
    iov_recv.iov_base = (void *)nlh_recv;
    iov_recv.iov_len = NLMSG_SPACE (0);
    memset (&source_addr_recv, 0, sizeof (struct sockaddr_nl));
    memset (&msg_recv, 0, sizeof (struct msghdr));
    msg_recv.msg_name = (void *)&source_addr_recv;
    msg_recv.msg_namelen = sizeof (struct sockaddr_nl);
    msg_recv.msg_iov = &iov_recv;
    msg_recv.msg_iovlen = 1;

    return 0;
}

/* send */
struct nlmsghdr *nlh_send = NULL;
struct sockaddr_nl dest_addr_send;
struct iovec iov_send;
struct msghdr msg_send;

int netlink_send_config (void)
{
    //create message
    nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(0));
    if( !nlh_send ) {
            perror("malloc");
            return -1;
    }
    nlh_send->nlmsg_len = NLMSG_SPACE(0);
    nlh_send->nlmsg_pid = TEST_PID;
    nlh_send->nlmsg_flags = 0;
    memcpy(NLMSG_DATA(nlh_send), " ", 0);

    iov_send.iov_base = (void *)nlh_send;
    iov_send.iov_len = nlh_send->nlmsg_len;
    memset(&dest_addr_send, 0, sizeof(struct sockaddr_nl));
    dest_addr_send.nl_family = AF_NETLINK;
    dest_addr_send.nl_pid = 0;
    dest_addr_send.nl_groups = 0;

    memset(&msg_send, 0, sizeof(struct msghdr));
    msg_send.msg_name = (void *)&dest_addr_send;
    msg_send.msg_namelen = sizeof(struct sockaddr_nl);
    msg_send.msg_iov = &iov_send;
    msg_send.msg_iovlen = 1;

    return 0;
}

int prepare_netlink (void)
{
    sock_fd = netlink_create_socket ();
    if (sock_fd == -1) {
        perror ("socket");
        return -1;
    }

    if (netlink_bind() < 0) {
        perror ("bind");
        return -1;
    }

    if (netlink_recv_config ()) {
        perror ("netlink recive config");
        return -1;
    }

    if (netlink_send_config ()) {
        perror ("netlink send config");
        return -1;
    }

    return 0;
}

static int ready (void)
{
    if (prepare_netlink ()) {
        if (-1 != sock_fd) {
            close (sock_fd);
        }
        return -1;
    }

    return 0;
}

void save_dma_data (void)
{
    int res = 0;
    while (ready ()) {
        ready();
        sleep (1);
    }

    while (1) {
        if (res = recvmsg(sock_fd, &msg_recv, MSG_WAITALL) < 0) {
            perror ("recvmsg");
            continue;
        }
        sendmsg(sock_fd, &msg_send, 0);
    }

    close (sock_fd);
    free (nlh_recv);
    free (nlh_send);
    nlh_recv = NULL;
    nlh_send = NULL;
}
