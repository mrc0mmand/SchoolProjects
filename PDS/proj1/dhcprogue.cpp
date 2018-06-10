#include <algorithm>
#include <arpa/inet.h>
#include <ctime>
#include <cstring>
#include <iostream>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#define __FAVOR_BSD
#include <netinet/udp.h>
#include <signal.h>
#include <vector>
#include <unistd.h>

#include "dhcp.h"

/**
 * @brief DHCP configuration
 */
typedef struct {
    uint32_t first_ip;      /**< First IP of the DHCP allocation pool */
    uint32_t last_ip;       /**< Last IP of the DHCP allocation pool */
    uint32_t gateway_ip;    /**< Gateway IP */
    uint32_t dns_ip;        /**< DNS IP */
    uint32_t lease_time;    /**< Lease time */
    int64_t pool_size;      /**< DHCP pool size */
    char *domain;           /**< Domain name */
    char *interface;        /**< Interface name */
} dhcp_conf_t;

/**
 * @brief Client configuration
 */
typedef struct {
    uint32_t txid;          /**< DHCP transaction ID */
    uint32_t assigned_ip;   /**< IP assigned to the client */
    uint64_t timestamp;     /**< Timestamp (for lease time) */
    mac_addr_t mac_addr;    /**< MAC address */
} client_info_t;

pcap_t *pcap_h;            /*<< pcap handler */
dhcp_conf_t dhcp_conf;     /*<< DHCP server configuration */
uint32_t srv_ip;           /*<< DHCP server IP */
uint32_t srv_net;          /*<< DHCP server network IP */
uint32_t srv_mask;         /**< DHCP server subnet mask */
std::vector<client_info_t> leases; /**< IP leases */

/**
 * @brief Convert string IP address to 32-bit integer
 *
 * @param ip_str String with an IP address
 * @param ip Valid pointer to an 32-bit integer where the converted IP
 *           will be stored in
 *
 * @returns 0 on success, 1 otherwise
 */
int ip_to_uint(char *ip_str, uint32_t *ip)
{
    struct in_addr ina;

    if(ip_str == NULL)
        return 1;

    if(inet_pton(AF_INET, ip_str, &ina) == 0)
        return 1;

    // Convert the IP from network byte order to host byte order
    *ip = ntohl(ina.s_addr);

    return 0;
}

/**
 * @brief Parse command line arguments
 */
void parse_args(int argc, char *argv[])
{
    int64_t ln;
    const uint8_t args_cnt = 6;
    uint8_t nargs[args_cnt] = {0, };
    int c;
    char *ptr;

    memset(&dhcp_conf, 0, sizeof(dhcp_conf));

    while((c = getopt(argc, argv, "i:p:g:n:d:l:")) != -1) {
        switch(c) {
        case 'i':
            dhcp_conf.interface = optarg;
            nargs[0]++;
            break;
        case 'p':
            ptr = strtok(optarg, "-");
            if(ip_to_uint(ptr, &dhcp_conf.first_ip) != 0) {
                fprintf(stderr, "Invalid DHCP pool specified\n");
                exit(EXIT_FAILURE);
            }

            ptr = strtok(NULL, "-");
            if(ip_to_uint(ptr, &dhcp_conf.last_ip) != 0) {
                fprintf(stderr, "Invalid DHCP pool specified\n");
                exit(EXIT_FAILURE);
            }

            nargs[1]++;
            break;
        case 'g':
            if(ip_to_uint(optarg, &dhcp_conf.gateway_ip) != 0) {
                fprintf(stderr, "Invalid gateway specified\n");
                exit(EXIT_FAILURE);
            }

            nargs[2]++;
            break;
        case 'n':
            if(ip_to_uint(optarg, &dhcp_conf.dns_ip) != 0) {
                fprintf(stderr, "Invalid DNS server specified\n");
                exit(EXIT_FAILURE);
            }

            nargs[3]++;
            break;
        case 'd':
            dhcp_conf.domain = optarg;
            nargs[4]++;
            break;
        case 'l':
            ln = strtol(optarg, &ptr, 0);
            if(*ptr != '\0' || ln <= 0) {
                fprintf(stderr, "Invalid lease time specified\n");
                exit(EXIT_FAILURE);
            }

            dhcp_conf.lease_time = ln;
            nargs[5]++;
            break;
        default:
            fprintf(stderr, "Usage: %s -i interface -p pool -g gateway "
                    "-n dns-server -d domain -l lease-time\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    for(uint8_t i = 0; i < args_cnt; i++) {
        if(nargs[i] != 1) {
            fprintf(stderr, "Usage: %s -i interface -p pool -g gateway "
                    "-n dns-server -d domain -l lease-time\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    dhcp_conf.pool_size = (dhcp_conf.last_ip - dhcp_conf.first_ip) + 1;

    if(dhcp_conf.pool_size <= 0) {
        fprintf(stderr, "Invalid pool size (must be >= 1)");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Get an IP address of a given interface
 * @details This function ends the application on failure, as the IP
 *          is necessary for the DHCP server
 *
 * @param interface Interface name
 * @param int_ip Valid pointer where the found IP address will be stored in
 */
void get_interface_ip(const char *interface, uint32_t *int_ip)
{
    pcap_addr_t *addr;
    pcap_if_t *devs;
    char pcap_err[PCAP_ERRBUF_SIZE];

    if(pcap_findalldevs(&devs, pcap_err) != 0) {
        fprintf(stderr, "pcap_findalldevs: %s\n", pcap_err);
        exit(EXIT_FAILURE);
    }

    while(devs != NULL) {
        if(strcmp(devs->name, interface) == 0) {
            addr = devs->addresses;
            while(addr != NULL) {
                if(addr->addr->sa_family == AF_INET) {
                    *int_ip = ntohl(((struct sockaddr_in*)addr->addr)->sin_addr.s_addr);
                    pcap_freealldevs(devs);
                    return;
                }

                addr = addr->next;
            }
        }

        devs = devs->next;
    }

    pcap_freealldevs(devs);
    fprintf(stderr, "Couldn't determine server's IP address\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Find an IP address to lease
 * @details If the client has already leased an IP in the past and the lease
 *          time has not expired, it will be reused
 *
 * @param client_info Valid pointer to a client_info structure with filled
 *                    MAC address
 * @param reused 1 if the IP is being reused, 0 otherwise
 *
 * @returns 0 on success, 1 otherwise
 */
int find_free_ip(client_info_t *client_info, uint8_t *reused)
{
    uint8_t free;
    uint64_t ctime = time(0);

    // Remove expired leases
    leases.erase(std::remove_if(leases.begin(), leases.end(),
            [&](client_info_t c) {
                return c.timestamp + dhcp_conf.lease_time < ctime; }),
            leases.end());

    // Check if the MAC address is already in our leased list
    for(auto &c : leases) {
        if(memcmp(c.mac_addr, client_info->mac_addr, sizeof(mac_addr_t)) == 0) {
            client_info->assigned_ip = c.assigned_ip;
            client_info->timestamp = time(0);
            *reused = 1;
            return 0;
        }
    }

    // If not, find the first available IP
    for(uint32_t ip = dhcp_conf.last_ip; ip >= dhcp_conf.first_ip; ip--) {
        free = 1;

        for(auto &c : leases) {
            if(c.assigned_ip == ip) {
                free = 0;
                break;
            }
        }

        if(free == 1) {
            client_info->assigned_ip = ip;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Build a DHCP response header of the given type
 *
 * @param dhcp_data Valid pointer to a DHCP configuration
 * @param client_info Valid pointer to a client configuration
 * @param packet_size Valid pointer with the packet size, which will be
 *                    update accordingly
 * @param type Packet type (DHCP_OPT_OFFER/DHCP_OPT_ACK/DHCP_OPT_NAK)
 */
void dhcp_build_response(dhcp_t *dhcp_data, client_info_t *client_info,
        int *packet_size, uint8_t type)
{
    int opt_len = 0;
    uint8_t byte;
    uint32_t dword;

    // DHCP request type
    byte = type;
    dhcp_set_option(dhcp_data->options, DHCP_CODE_TYPE, 1, &byte, &opt_len);
    // Router IP
    dword = htonl(dhcp_conf.gateway_ip);
    dhcp_set_option(dhcp_data->options, DHCP_CODE_ROUTER, 4, &dword, &opt_len);
    // Subnet mask
    dword = htonl(srv_mask);
    dhcp_set_option(dhcp_data->options, DHCP_CODE_SUBNET, 4, &dword, &opt_len);
    // Lease time
    dword = htonl(dhcp_conf.lease_time);
    dhcp_set_option(dhcp_data->options, DHCP_CODE_LEASE, 4, &dword, &opt_len);
    // DHCP server IP
    dword = htonl(srv_ip);
    dhcp_set_option(dhcp_data->options, DHCP_CODE_SERVER, 4, &dword, &opt_len);
    // DNS server
    dword = htonl(dhcp_conf.dns_ip);
    dhcp_set_option(dhcp_data->options, DHCP_CODE_DNS, 4, &dword, &opt_len);
    // Domain name
    dhcp_set_option(dhcp_data->options, DHCP_CODE_DOMAIN, strlen(dhcp_conf.domain),
            dhcp_conf.domain, &opt_len);
    // End byte
    byte = 0;
    dhcp_set_option(dhcp_data->options, DHCP_CODE_END, 1, &byte, &opt_len);

    *packet_size += sizeof(dhcp_t) + opt_len;
    dhcp_data->op = DHCP_BOOTREPLY;
    dhcp_data->htype = 1; // DHCP HW type 10 Ethernet;
    dhcp_data->hlen = 6; // Header length
    dhcp_data->yiaddr = htonl(client_info->assigned_ip);
    dhcp_data->siaddr = htonl(srv_ip);
    dhcp_data->xid = htonl(client_info->txid);
    memcpy(dhcp_data->chaddr, client_info->mac_addr, sizeof(mac_addr_t)); // MAC address
    dhcp_data->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
}

/**
 * @brief Build and send a DHCP packet of given type
 *
 * @param client_info Valid pointer to a client configuration
 * @param type Packet type (DHCP_OPT_OFFER/DHCP_OPT_ACK/DHCP_OPT_NAK)
 *
 * @returns 0 on success, 1 otherwise
 */
int dhcp_send_packet(client_info_t *client_info, int8_t type)
{
    int rc;
    int packet_size = 0;
    uint8_t packet[4096] = {0, };
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct udphdr *udp_header;
    dhcp_t *dhcp_data;

    // Calculate header offsets in the packet
    // packet = ether_header + ip_header + udp_header + dhcp_data
    eth_header = (struct ether_header*) packet;
    ip_header = (struct ip*)((uint8_t*)eth_header + sizeof(struct ether_header));
    udp_header = (struct udphdr*)((uint8_t*)ip_header + sizeof(struct ip));
    dhcp_data = (dhcp_t*)((uint8_t*)udp_header + sizeof(struct udphdr));

    // Set DHCP data
    if(type == DHCP_OPT_OFFER) {
        dhcp_build_response(dhcp_data, client_info, &packet_size, type);
        printf("Sending OFFER to MAC " MACFMT " with IP " IPFMT "\n",
                MACARGS(client_info->mac_addr), IPARGS(client_info->assigned_ip));
    } else if(type == DHCP_OPT_ACK || type == DHCP_OPT_NAK) {
        dhcp_build_response(dhcp_data, client_info, &packet_size, type);
        printf("Sending %s to MAC " MACFMT " with IP " IPFMT "\n",
                (type == DHCP_OPT_ACK) ? "ACK" : "NAK",
                MACARGS(client_info->mac_addr), IPARGS(client_info->assigned_ip));
    } else {
        fprintf(stderr, "Invalid response type\n");
        exit(EXIT_FAILURE);
    }

    // Set UDP header
    // padding
    if(packet_size & 1)
        packet_size++;
    packet_size += sizeof(struct udphdr);
    udp_header->uh_sport = htons(DHCP_SERVER_PORT); // Source port
    udp_header->uh_dport = htons(DHCP_CLIENT_PORT); // Destination port
    udp_header->uh_ulen = htons(packet_size); // Total length
    udp_header->uh_sum = 0;

    // Set IP header
    packet_size += sizeof(struct ip);
    ip_header->ip_dst.s_addr = htonl(client_info->assigned_ip);
    ip_header->ip_hl = sizeof(struct ip) >> 2; // Header length
    ip_header->ip_id = htons(0xffff); // Identification
    ip_header->ip_len = htons(packet_size); // Total length
    ip_header->ip_off = 0; // Fragment offset field
    ip_header->ip_sum = 0; // Checksum (must be set to 0 before settin real checksum)
    ip_header->ip_src.s_addr = htonl(srv_ip); // DHCP server IP
    ip_header->ip_ttl = 16; // Time to live
    ip_header->ip_tos = 0x10; // Type of service
    ip_header->ip_v = IPVERSION; // IP version = 4
    ip_header->ip_p = IPPROTO_UDP; // Use UDP protocol

    ip_header->ip_sum = in_cksum((unsigned short*) ip_header, sizeof(struct ip));

    // Set ethernet header
    packet_size += sizeof(struct ether_header);
    memset(eth_header->ether_shost, 0, sizeof(mac_addr_t));
    memset(eth_header->ether_dhost, -1, sizeof(mac_addr_t));
    eth_header->ether_type = htons(ETHERTYPE_IP);

    // Send the packet
    rc = pcap_inject(pcap_h, packet, packet_size);
    if(rc <= 0) {
        pcap_perror(pcap_h, "pcap_inject()");
        return 1;
    }

    return 0;
}

/**
 * @brief Handler function for pcap_loop()
 * @details This function parses relevant incoming packets
 */
void dhcp_input_handler(uint8_t *args, const struct pcap_pkthdr *header, const uint8_t *frame)
{
    struct ether_header *eth_header = (struct ether_header*)frame;
    struct ip *ip_header;
    struct udphdr *udp_header;
    client_info_t client_info;
    dhcp_t *dhcp_data;
    uint8_t reused;

    // Check if we got IP packet
    if(htons(eth_header->ether_type) != ETHERTYPE_IP)
        return;

    ip_header = (struct ip*)((uint8_t*)eth_header + sizeof(struct ether_header));
    // Check if the protocol is UDP
    if(ip_header->ip_p != IPPROTO_UDP)
        return;

    udp_header = (struct udphdr*)((uint8_t*)ip_header + sizeof(struct ip));
    // Check if the packet for DHCP port
    if(ntohs(udp_header->uh_sport) != DHCP_CLIENT_PORT)
        return;

    dhcp_data = (dhcp_t*)((uint8_t*)udp_header + sizeof(struct udphdr));
    // Check if the DHCP packet is a request packet
    if(dhcp_data->op != DHCP_BOOTREQUEST)
        return;

    memcpy(client_info.mac_addr, dhcp_data->chaddr, sizeof(mac_addr_t));
    client_info.txid = ntohl(dhcp_data->xid);

    if(dhcp_data->options[2] == DHCP_OPT_DISCOVER) {
        printf("DISCOVER from " MACFMT "\n", MACARGS(client_info.mac_addr));
        reused = 0;
        if(find_free_ip(&client_info, &reused) == 0) {
            client_info.timestamp = time(0);
            if(reused != 1)
                leases.push_back(client_info);
            dhcp_send_packet(&client_info, DHCP_OPT_OFFER);
        } else {
            printf("DHCP IP pool is empty\n");
        }
    } else if(dhcp_data->options[2] == DHCP_OPT_REQUEST) {
        printf("REQUEST from " MACFMT "\n", MACARGS(client_info.mac_addr));

        for(auto &c : leases) {
            if(memcmp(c.mac_addr, client_info.mac_addr, sizeof(mac_addr_t)) == 0) {
                client_info.assigned_ip = c.assigned_ip;
                dhcp_send_packet(&client_info, DHCP_OPT_ACK);
                return;
            }
        }

        dhcp_send_packet(&client_info, DHCP_OPT_NAK);
    }
}

int main(int argc, char *argv[])
{
    int rc;
    char pcap_err[PCAP_ERRBUF_SIZE];

    parse_args(argc, argv);

    rc = pcap_lookupnet(dhcp_conf.interface, &srv_net, &srv_mask, pcap_err);
    if(rc != 0) {
        fprintf(stderr, "Couldn't determine interface network address: %s\n",
                pcap_err);
        exit(EXIT_FAILURE);
    }

    srv_net = ntohl(srv_net);
    srv_mask = ntohl(srv_mask);
    get_interface_ip(dhcp_conf.interface, &srv_ip);

    printf("Interface: %s\n", dhcp_conf.interface);
    printf("Interface IP: " IPFMT "\n", IPARGS(srv_ip));
    printf("Interface mask: " IPFMT "\n", IPARGS(srv_mask));
    printf("Pool size: %lu\n", dhcp_conf.pool_size);
    printf("Gateway: " IPFMT "\n", IPARGS(dhcp_conf.gateway_ip));
    printf("DNS: " IPFMT "\n", IPARGS(dhcp_conf.dns_ip));
    printf("Domain: %s\n", dhcp_conf.domain);
    printf("Lease time: %u seconds\n", dhcp_conf.lease_time);

    pcap_h = pcap_open_live(dhcp_conf.interface, BUFSIZ, 0, 10, pcap_err);
    if(pcap_h == NULL) {
        fprintf(stderr, "Couldn't open interface %s: %s\n",
                dhcp_conf.interface, pcap_err);
        exit(EXIT_FAILURE);
    }

    pcap_loop(pcap_h, -1, dhcp_input_handler, NULL);

    pcap_close(pcap_h);

    return 0;
}
