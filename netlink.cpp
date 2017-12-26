#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/socket.h>

#include "netlink.h"

Netlink::Netlink(int pid, int protocol):m_pid(pid),
    m_protocol(protocol), m_fd (-1)
{
    ;
}

Netlink::~Netlink()
{
    if (-1 != m_fd) {
        close (m_fd);
    }
}

int Netlink::netlink_create(int famlily, int type)
{
    m_fd = socket (famlily, type, m_protocol);
    if (m_fd < 0) {
        printf ("create socket error! \n");
        return -1;
    }
    return 0;
}

void Netlink::set_sockaddr(struct sockaddr_nl *addr,
                           int famlily,
                           unsigned int pid,
                           unsigned int groups)
{
    struct sockaddr_nl *p_addr = addr;

    memset (addr, 0, sizeof (struct sockaddr_nl));
    addr->nl_family = famlily;
    addr->nl_pid = pid;
    addr->nl_groups = groups;
}

int Netlink::netlink_bind(sockaddr_nl *addr)
{
    if (m_pid - addr->nl_pid) {
        addr->nl_pid = m_pid;
    }
    return bind (m_fd, (struct sockaddr *)addr, sizeof (struct sockaddr_nl));
}

int Netlink::netlink_recive_data(char *buf, int len)
{
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl source_addr;
    struct iovec iov;
    struct msghdr msg;

    //create message
    nlh = (struct nlmsghdr *)malloc (NLMSG_SPACE (len));
    if (!nlh) {
        perror ("malloc");
        return -1;
    }
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE (len);
    memset (&source_addr, 0, sizeof (struct sockaddr_nl));
    memset (&msg, 0, sizeof (struct msghdr));
    msg.msg_name = (void *)&source_addr;
    msg.msg_namelen = sizeof (struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (recvmsg(m_fd, &msg, MSG_WAITALL) < 0) {
        printf("recvmsg error!\n");
        return -1;
    }
    memcpy (buf, (void *)NLMSG_DATA (nlh), len);

    free (nlh);
    return 0;
}

int Netlink::netlink_send_data(const char *buf,
                               int len,
                               unsigned int pid,
                               unsigned int groups)
{
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl dest_addr;
    struct iovec iov;
    struct msghdr msg;

    if( !buf ) {
            return -1;
    }

    //create message
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
    if( !nlh ) {
            perror("malloc");
            return -1;
    }
    nlh->nlmsg_len = NLMSG_SPACE(len);
    nlh->nlmsg_pid = m_pid;
    nlh->nlmsg_flags = 0;
    memcpy(NLMSG_DATA(nlh), buf, len);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = pid;
    dest_addr.nl_groups = groups;

    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //send message
    if( sendmsg(m_fd, &msg, 0) < 0 )
    {
            printf("send error!\n");
            free(nlh);
            return -1;
    }

    free(nlh);
    return 0;
}
