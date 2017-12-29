#ifndef __NETLINK_H__
#define __NETLINK_H__

#include <linux/net.h>

struct sockaddr_nl;
class Netlink {
public:
    Netlink (int pid = 100, int protocol = 25);
    ~Netlink ();

    int netlink_create (int famlily = AF_NETLINK, int type = SOCK_RAW);
    void set_sockaddr (struct sockaddr_nl *addr,
                       int famlily = AF_NETLINK,
                       unsigned int pid = 100,
                       unsigned int groups = 0);
    int netlink_bind (struct sockaddr_nl *addr);
    int netlink_recive_data (char *buf, int len);
    int netlink_send_data (const char *buf,
                           int len,
                           unsigned int pid = 100,
                           unsigned int groups = 0);

private:
    int m_fd;
    unsigned int m_pid;
    int m_protocol;
};

#endif // __NETLINK_H__
