#include <cstring>
#include "dhcp.h"

unsigned short in_cksum(unsigned short *addr, int len)
{
    int             nleft = len;
    int             sum = 0;
    unsigned short  *w = addr;
    unsigned short  answer = 0;

    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

        /* 4mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w ;
        sum += answer;
    }

        /* 4add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return(answer);
}

void dhcp_set_option(uint8_t *dhcp_options, uint8_t code, uint8_t length,
        void *data, int *opt_len)
{
    dhcp_options[(*opt_len)++] = code;
    dhcp_options[(*opt_len)++] = length;
    memcpy(&dhcp_options[*opt_len], data, length);
    *opt_len += length;
}

