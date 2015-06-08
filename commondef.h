/*
 * =====================================================================================
 *
 *       Filename:  commondef.h
 *
 *    Description:  common struct and fuction definitions.
 *
 *        Version:  1.0
 *        Created:  07/07/2009 02:38:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOYPT (PT), pentie@gmail.com
 *        Company:  http://apt-blog.co.cc
 *
 * =====================================================================================
 */


#ifndef  COMMONDEF_INC
#define  COMMONDEF_INC

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
#include <pcap.h>

#include <winsock2.h>
#include <iphlpapi.h>
///* ZRuijie Version */
//#define ZRJ_VER "0.6"
#define PROTOCOL_VER "0.11"

#define MAX_DEV_NAME_LEN 256

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14
#define OFFSET_EAPOL    0x0E                    /*  */
#define OFFSET_EAP      0x12

#define ETHER_ADDR_LEN 6

struct ether_header
{
  u_int8_t  ether_dhost[ETHER_ADDR_LEN];      /* destination eth addr */
  u_int8_t  ether_shost[ETHER_ADDR_LEN];      /* source ether addr    */
  u_int16_t ether_type;                 /* packet type ID field */
};

struct eap_header {
    u_char eapol_v;
    u_char eapol_t;
    u_short eapol_length;
    u_char eap_t;
    u_char eap_id;
    u_short eap_length;
    u_char eap_op;
    u_char eap_v_length;
    u_char eap_md5_challenge[16];
};

enum EAPType {
    EAPOL_START,
    EAPOL_LOGOFF,
    EAP_REQUEST_IDENTITY,
    EAP_RESPONSE_IDENTITY,
    EAP_REQUEST_IDENTITY_KEEP_ALIVE,
    EAP_RESPONSE_IDENTITY_KEEP_ALIVE,
    EAP_REQUETS_MD5_CHALLENGE,
    EAP_RESPONSE_MD5_CHALLENGE,
    EAP_SUCCESS,
    EAP_FAILURE,
    RUIJIE_EAPOL_MSG,
    OTHER_ERROR
};

struct dcba_tailer {
    bpf_u_int32     local_ip;
    bpf_u_int32     local_mask;
    bpf_u_int32     local_gateway;
    bpf_u_int32     local_dns;
    u_char          username_md5[16];
    u_char          client_ver[13];
};

enum STATE {
    READY,
    CONNECTING,
    ONLINE,
    KEEP_ALIVE,
	LOGOFF
};

typedef union
{
  u_int32_t int_value;
  u_int8_t  byte_value[4];
} BYTEArray;

char*
get_md5_digest(const char* str, size_t len);
/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */
/* zruijie.c内实现，
 * win_main.c内调用*/
void    init_frames();
void    init_device();
void    get_packet(u_char *args, const struct pcap_pkthdr *header,
                        const u_char *packet);

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */
/* win_main.c内实现的函数, 全局调用*/
void update_interface_state(const char *msg);
void edit_info_append (const char *msg);
void thread_error_exit (const char *errmsg);
void debug_msgbox (const char *fmt, ...);
void renew_system_dhcp();
#endif   /* ----- #ifndef COMMONDEF_INC  ----- */


