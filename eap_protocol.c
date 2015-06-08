/*
 * =====================================================================================
 *
 *       Filename:  eap_protocol.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/07/2009 02:55:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOYPT (PT), pentie@gmail.com
 *        Company:  http://apt-blog.co.cc
 *
 * =====================================================================================
 */
#include 	<assert.h>
#include	"eap_protocol.h"
//#include	"zruijie.h"
//#include	"blog.h"
#include	"md5.h"


static void 
fill_password_md5(uint8_t attach_key[], uint8_t eap_id);
static void 
fill_username_md5(uint8_t attach_key[]);
//DWORD WINAPI keep_alive();
//DWORD WINAPI wait_exit();

/* #####   TYPE DEFINITIONS   ######################### */
/*-----------------------------------------------------------------------------
 *  报文缓冲区，由init_frame函数初始化。
 *-----------------------------------------------------------------------------*/
uint8_t             eapol_start[18];            /* EAPOL START报文 */
uint8_t             eapol_logoff[18];           /* EAPOL LogOff报文 */
uint8_t             eap_response_ident[128]; /* EAP RESPON/IDENTITY报文 */
uint8_t             eap_response_md5ch[128]; /* EAP RESPON/MD5 报文 */
uint8_t             eap_life_keeping[128];
//uint32_t            ruijie_live_serial_num;
//uint32_t            ruijie_succes_key;
extern enum STATE   state;

extern pcap_t       *handle;

void
action_eapol_success(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet)
{
    extern int dhcp_on;
    state = ONLINE;
    print_server_info (packet, packetinfo->caplen);

    if (dhcp_on)
        renew_system_dhcp();
}

void
action_eapol_failre(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet)
{
    print_server_info (packet, packetinfo->caplen);

	if (state == ONLINE || state == LOGOFF) {
		state = READY;
		pcap_breakloop (handle);
	}
}

void
action_eap_req_idnty(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet)
{
    if (state == LOGOFF)
        return;
	state = CONNECTING;
    eap_response_ident[0x13] = eap_head->eap_id;
    send_eap_packet(EAP_RESPONSE_IDENTITY);
}

void
action_eap_req_md5_chg(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet)
{
	state = CONNECTING;
    fill_password_md5((uint8_t*)eap_head->eap_md5_challenge, eap_head->eap_id);
    eap_response_md5ch[0x13] = eap_head->eap_id;
    send_eap_packet(EAP_RESPONSE_MD5_CHALLENGE);
}

void
action_eap_keep_alive(const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet)
{
	state = KEEP_ALIVE;
     //The attach_key start at eap_v_length,but here is not the length
    fill_username_md5((uint8_t*)&eap_head->eap_v_length);
     //change the eap_id
    *(eap_life_keeping + 14 + 5) =eap_head->eap_id;
    //eap_life_keeping[0x13] = eap_head->eap_id;
    send_eap_packet(EAP_RESPONSE_IDENTITY_KEEP_ALIVE);
}

void 
send_eap_packet(enum EAPType send_type)
{
    uint8_t         *frame_data;
    int             frame_length = 0;

    switch(send_type){
        case EAPOL_START:
            frame_data= eapol_start;
            frame_length = sizeof(eapol_start);
            break;
        case EAPOL_LOGOFF:
            frame_data = eapol_logoff;
            frame_length = sizeof(eapol_logoff);
            break;
        case EAP_RESPONSE_IDENTITY:
            frame_data = eap_response_ident;
            frame_length = sizeof(eap_response_ident);
            break;
        case EAP_RESPONSE_MD5_CHALLENGE:
            frame_data = eap_response_md5ch;
            frame_length = sizeof(eap_response_md5ch);
            break;
        case EAP_RESPONSE_IDENTITY_KEEP_ALIVE:
            frame_data = eap_life_keeping;
            frame_length = sizeof(eap_life_keeping);
            break;
        default:
            return;
    }
    pcap_sendpacket(handle, frame_data, frame_length);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  fill_password_md5
 *  Description:  给RESPONSE_MD5_Challenge报文填充相应的MD5值。
 *  只会在接受到REQUEST_MD5_Challenge报文之后才进行，因为需要
 *  其中的Key
 * =====================================================================================
 */
void 
fill_password_md5(uint8_t attach_key[], uint8_t eap_id)
{
    extern char password[];
    extern int  password_length;
    char *psw_key; 
    char *md5;

    psw_key = malloc(1 + password_length + 16);
    psw_key[0] = eap_id;
    memcpy (psw_key + 1, password, password_length);
    memcpy (psw_key + 1 + password_length, attach_key, 16);

    md5 = get_md5_digest(psw_key, 1 + password_length + 16);
    memcpy (eap_response_md5ch + 14 + 10, md5, 16);

    free (psw_key);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  fill_username_md5
 *  Description:  给RESPONSE_IDENTITY_KEEP_ALIVE报文填充相应的MD5值⾿ *  只会在接受到REQUEST_IDENTITY_KEEP_ALIVE报文之后才进行，因为需褿 *  其中的Key
 * =====================================================================================
 */
void
fill_username_md5(uint8_t attach_key[])
{
	extern char username[];
	extern int username_length;
    char *psw_key;
    char *md5;
    psw_key = malloc(username_length + 4);
    
    memcpy(psw_key, username,username_length);
    memcpy(psw_key + username_length, attach_key, 4);
    
    md5 = get_md5_digest(psw_key, username_length + 4);
    memcpy(eap_life_keeping + 14 + 9,md5,16);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_md5_digest
 *  Description:  calcuate for md5 digest
 * =====================================================================================
 */
char* 
get_md5_digest(const char* str, size_t len)
{
    static md5_byte_t digest[16];
	md5_state_t state;
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)str, len);
	md5_finish(&state, digest);

    return (char*)digest;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_server_info
 *  Description:  提取中文信息并打印输出
 * =====================================================================================
 */
void 
print_server_info (const uint8_t *packet, u_int packetlength)
{
    char            msg_buf[1024] = {0};
    const u_char    *str;
    
    {
        if ( *(packet + 0x2A) == 0x12) {
            str = (packet + 0x2B);
            goto FOUND_STR;
        }
        if (packetlength < 0x42)
            return;
        if ( *(packet + 0x42) == 0x12) {
            str = (packet + 0x43);
            goto FOUND_STR;
        }
        if (packetlength < 0x9A)
            return;
        if ( *(packet + 0x9A) == 0x12) {
            str = (packet + 0x9B);
            goto FOUND_STR;
        }
        if (packetlength < 0x120)
            return;
        if ( *(packet + 0x120) == 0x12) {
            str = (packet + 0x121);
            goto FOUND_STR;
        }
        return;
    }

    FOUND_STR:;
    size_t length = strlen((const char *)str);
    length = length < 512 ? length : 512;
    strncpy (msg_buf, (const char *)str, length);
    msg_buf[length] = '\n';
	edit_info_append (msg_buf);
}


