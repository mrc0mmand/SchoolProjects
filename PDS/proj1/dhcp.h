#ifndef __DHCP_H_INCLUDED
#define __DHCP_H_INCLUDED

#include <iostream>

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_MAGIC_COOKIE 0x63825363

#define DHCP_OPT_DISCOVER 1
#define DHCP_OPT_OFFER    2
#define DHCP_OPT_REQUEST  3
#define DHCP_OPT_ACK      5
#define DHCP_OPT_NAK      6

#define DHCP_CODE_SUBNET    1
#define DHCP_CODE_ROUTER    3
#define DHCP_CODE_DNS       6
#define DHCP_CODE_DOMAIN    15
#define DHCP_CODE_LEASE     51
#define DHCP_CODE_TYPE      53
#define DHCP_CODE_SERVER    54
#define DHCP_CODE_END       255

#define DHCP_BOOTREQUEST  1
#define DHCP_BOOTREPLY    2

#define IPFMT "%u.%u.%u.%u"
#define IPARGS(ip) \
    (ip) >> 24, ((ip) << 8) >> 24, ((ip) << 16) >> 24, ((ip) << 24) >> 24
#define MACFMT "%x:%x:%x:%x:%x:%x"
#define MACARGS(mac) (mac)[0], (mac)[1], (mac)[2], (mac)[3], (mac)[4], (mac)[5]

typedef uint8_t mac_addr_t[6];

/**
 * @brief DHCP header field structure
 * @details See RFC 2131, section 2 - Protocol Summary
 */
typedef struct {
    uint8_t op;        /*< Message OP code/type (1 = BOOTREQUEST, 2 = BOOTREPLY) */
    uint8_t htype;     /*< Hardware address type */
    uint8_t hlen;      /*< Hardware address length */
    uint8_t hops;      /*< Client sets to 0 */
    uint32_t xid;      /*< Transaction ID, random number chosen by the client */
    uint16_t secs;     /*< Seconds elapsed since client began address acq/renew process */
    uint16_t flags;    /*< Flags */
    uint32_t ciaddr;   /*< Client IP address */
    uint32_t yiaddr;   /*< 'Your' (client) IP address */
    uint32_t siaddr;   /*< IP address of next server to use in bootstrap */
    uint32_t giaddr;   /*< Relay agent IP address */
    uint8_t chaddr[16];/*< Client hardware address */
    char sname[64];   /*< Optional server host name, null terminated */
    char file[128];   /*< boot file name, 0 terminated string */
    uint32_t magic_cookie; /*< Magic cookie */
    uint8_t options[0];/*< Optional parameters */
} dhcp_t;

/**
 * @brief Calculate checksum of given data
 * @details Taken from Carnegie Mellon University
 * @see http://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/unpv12e/libfree/in_cksum.c
 */
unsigned short in_cksum(unsigned short *addr, int len);
void dhcp_set_option(uint8_t *dhcp_options, uint8_t code, uint8_t length,
        void *data, int *opt_len);

#endif
