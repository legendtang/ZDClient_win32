/*
 * =====================================================================================
 *
 *       Filename:  eap_protocol.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/07/2009 02:55:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOYPT (PT), pentie@gmail.com
 *        Company:  http://apt-blog.co.cc
 *
 * =====================================================================================
 */

#include	"commondef.h"

void
action_eapol_success(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet);

void
action_eapol_failre(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet);

void
action_eap_req_idnty(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet);

void
action_eap_req_md5_chg(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet);

void
action_eap_keep_alive(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet);

void
print_server_info (const uint8_t *packet, u_int packetlength);

void
print_notification_msg(const uint8_t *packet);

u_int32_t get_ruijie_success_key (const uint8_t *success_packet);

void
send_eap_packet(enum EAPType send_type);

