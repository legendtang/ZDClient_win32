/*
 * =====================================================================================
 *
 *       Filename:  zdclient.c
 *
 *    Description:
 *
 *        Version:  0.1
 *        Created:  07/06/2009 08:07:12 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  BOYPT (PT), pentie@gmail.com
 *        Company:  http://apt-blog.co.cc
 *
 * =====================================================================================
 */
#include    "zdclient.h"
#include	"eap_protocol.h"
#include    <assert.h>


/* #####   GLOBLE VAR DEFINITIONS   ######################### */
/*-----------------------------------------------------------------------------
 *  程序的主控制变量
 *-----------------------------------------------------------------------------*/
char        errbuf[PCAP_ERRBUF_SIZE];   /* error buffer */
pcap_t      *handle;			        /* packet capture handle */
enum STATE  state	=  READY;           /* program state */

uint8_t     muticast_mac[] =            /* 801.1x认证服务器多播地址 */
                        {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};


/* #####   GLOBLE VAR DEFINITIONS   ###################
 *-----------------------------------------------------------------------------
 *  用户信息的赋值变量，由init_argument函数初始化
 *-----------------------------------------------------------------------------*/
int         dhcp_on = 0;               /* DHCP 模式标记 */
//int         background = 0;            /* 后台运行标记  */
//int         exit_flag = 0;
//char        *dev = NULL;               /* 连接的设备名 */
char        username[64];
char        password[64];
char        *user_gateway = NULL;      /* 由用户设定的四个报文参数 */
char        *user_dns = NULL;           /* 字符串内保存点分ip格式ASCII */
char        *user_ip = NULL;
char        *user_mask = NULL;
//char        *client_ver = NULL;         /* 报文协议版本号 */

/* #####   GLOBLE VAR DEFINITIONS   #########################
 *-----------------------------------------------------------------------------
 *  报文相关信息变量，由init_info 、init_device函数初始化。
 *-----------------------------------------------------------------------------*/
int         username_length;
int         password_length;
uint32_t    local_ip		= 0;			       /* 网卡IP，网络序，下同 */
uint32_t    local_mask		= 0;			       /* subnet mask */
uint32_t    local_gateway 	= 0;
uint32_t    local_dns 		= 0;
uint8_t     local_mac[ETHER_ADDR_LEN]; /* MAC地址 */
//uint8_t     client_ver_val[2];
char        devname[MAX_DEV_NAME_LEN];
char        client_ver[20] = {0};

//#ifdef  __DEBUG
// debug function
void
print_hex(const uint8_t *array, int count)
{
    int i;
    for(i = 0; i < count; i++){
        if ( !(i % 16))
            printf ("\n");
        printf("%02x ", array[i]);
    }
    printf("\n");
}
//#endif     /* -----  not __DEBUG  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  get_packet
 *  Description:  pcap的回呼函数，当收到EAPOL报文时自动被调用
 * =====================================================================================
 */
void
get_packet(uint8_t *args, const struct pcap_pkthdr *pcaket_header,
    const uint8_t *packet)
{
	/* declare pointers to packet headers */
	const struct ether_header *ethernet;  /* The ethernet header [1] */
    const struct eap_header *eap_header;
    enum EAPType p_type;

    ethernet = (struct ether_header*)(packet);
    eap_header = (struct eap_header *)(packet + SIZE_ETHERNET);

    p_type = get_eap_type(eap_header);
    if (p_type != ERROR)
        action_by_eap_type(p_type, eap_header, pcaket_header, packet);
    return;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  get_eap_type
 *  Description:  根据报文的动作位返回enum EAPType内定义的报文类型
 * =====================================================================================
 */
enum EAPType
get_eap_type(const struct eap_header *eap_header)
{
    switch (eap_header->eap_t){
        case 0x01:
	    if ( eap_header->eap_op == 0xfa)
		return EAP_REQUEST_IDENTITY_KEEP_ALIVE;
	    if ( eap_header->eap_op == 0x01)
		return EAP_REQUEST_IDENTITY;
            if ( eap_header->eap_op == 0x04)
                return EAP_REQUETS_MD5_CHALLENGE;
            break;
        case 0x03:
            return EAP_SUCCESS;
            break;
        case 0x04:
            return EAP_FAILURE;
    }
    return ERROR;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  action_by_eap_type
 *  Description:  根据eap报文的类型完成相关的应答
 * =====================================================================================
 */
void
action_by_eap_type(enum EAPType pType,
                        const struct eap_header *eap_head,
                        const struct pcap_pkthdr *packetinfo,
                        const uint8_t *packet) {
//    printf("PackType: %d\n", pType);
    switch(pType){
        case EAP_SUCCESS:
            action_eapol_success (eap_head, packetinfo, packet);
            break;
        case EAP_FAILURE:
            action_eapol_failre (eap_head, packetinfo, packet);
            break;
        case EAP_REQUEST_IDENTITY:
            action_eap_req_idnty (eap_head, packetinfo, packet);
            break;
        case EAP_REQUETS_MD5_CHALLENGE:
            action_eap_req_md5_chg (eap_head, packetinfo, packet);
            break;
        case EAP_REQUEST_IDENTITY_KEEP_ALIVE:
            action_eap_keep_alive (eap_head, packetinfo, packet);
            return;
        default:
            return;
    }
	update_interface_state(NULL);
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_info
 *  Description:  初始化本地信息。（字符串->二进制数值）
 * =====================================================================================
 */
// void _init_info()
// {
    // extern uint32_t  ruijie_live_serial_num;
	// extern HANDLE    hwndComboList;

    // client_ver_val[0] = 3;
    // client_ver_val[1] = 50;
	// dhcp_on = 1;
    // ruijie_live_serial_num = 0x0000102b;

// }

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_device
 *  Description:  初始化设备。
 *  同时设置pcap的初始化工作句柄。
 * =====================================================================================
 */
void init_device()
{
    struct          bpf_program fp;			/* compiled filter program (expression) */
    char            filter_exp[51];         /* filter expression [3] */
    pcap_if_t       *alldevs;
	pcap_if_t 		*d;
//	extern HANDLE    hwndComboList;
    extern int      combo_index;

	/* NIC device  */
	assert(pcap_findalldevs(&alldevs, errbuf) != -1);

	int sel_index = combo_index;
	for(d = alldevs; sel_index-- && d; d = d->next);
//	while (sel_index--)
//		d = d->next;
	pcap_addr_t *a;
	for(a = d->addresses; a ; a=a->next) {
		if (a->addr->sa_family == AF_INET) {
			strcpy (devname, d->name);
			local_ip = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
			local_mask = ((struct sockaddr_in *)a->netmask)->sin_addr.s_addr;
			break;
		}
	}
	pcap_freealldevs(alldevs);

//    debug_msgbox ("%s", devname);

	/* Mac */
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	PIP_ADAPTER_INFO pAdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer

	if(dwStatus != ERROR_SUCCESS){			// Verify return value is valid, no buffer overflow
        thread_error_exit("Invalid Device.[GET Mac Addr]");
    }

	for (pAdapterInfo = AdapterInfo; pAdapterInfo; pAdapterInfo = pAdapterInfo->Next) {
		if (strstr (devname, pAdapterInfo->AdapterName) != NULL) {
		    memcpy(local_mac, pAdapterInfo->Address, ETHER_ADDR_LEN);
			break;
		}
	}
    if (!pAdapterInfo)
        thread_error_exit("No MAC Addr Found on the Selected Device.");

	/* open capture device */
	handle = pcap_open_live(devname, SNAP_LEN, 1, 1000, errbuf);

    if (handle == NULL)
        thread_error_exit("Invalid Device.[Open Live]");
//	assert (handle != NULL);

	/* make sure we're capturing on an Ethernet device [2] */
	if (pcap_datalink(handle) != DLT_EN10MB)
        thread_error_exit("Invalid Device.[Ethernet]");

    /* construct the filter string */
    sprintf(filter_exp, "ether dst %02x:%02x:%02x:%02x:%02x:%02x"
                        " and ether proto 0x888e",
                        local_mac[0], local_mac[1],
                        local_mac[2], local_mac[3],
                        local_mac[4], local_mac[5]);

	/* compile the filter expression */
	if (pcap_compile(handle, &fp, filter_exp, 0, 0) == -1)
        thread_error_exit("Invalid Device.[Filter Compile.]");

	/* apply the compiled filter */
	if (pcap_setfilter(handle, &fp) == -1)
        thread_error_exit("Invalid Device.[Setting Filter.]");

    pcap_freecode(&fp);
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_frames
 *  Description:  初始化发送帧的数据缓冲区
 * =====================================================================================
 */
void
init_frames()
{
    extern uint8_t      eapol_start[];        /* EAPOL START报文 */
    extern uint8_t      eapol_logoff[];       /* EAPOL LogOff报文 */
    extern uint8_t      eap_response_ident[]; /* EAP RESPON/IDENTITY报文 */
    extern uint8_t      eap_response_md5ch[]; /* EAP RESPON/MD5 报文 */
    extern uint8_t      eap_life_keeping[];   /* EAPOL KEEP ALIVE*/
//    uint8_t             circle_sum[2];
    int                 data_index;
    /*****  EAPOL Header  *******/
    u_char eapol_eth_header[SIZE_ETHERNET];
    struct ether_header *eth = (struct ether_header *)eapol_eth_header;
    memcpy (eth->ether_dhost, muticast_mac, 6);
    memcpy (eth->ether_shost, local_mac, 6);
    eth->ether_type =  htons (0x888e);

    /**** EAPol START ****/
    u_char start_data[4] = {0x01, 0x01, 0x00, 0x00};
    memcpy (eapol_start, eapol_eth_header, SIZE_ETHERNET);
    memcpy (eapol_start + SIZE_ETHERNET, start_data, 4);

    /****EAPol LOGOFF ****/
    u_char logoff_data[4] = {0x01, 0x02, 0x00, 0x00};
    memcpy (eapol_logoff, eapol_eth_header, SIZE_ETHERNET);
    memcpy (eapol_logoff + SIZE_ETHERNET, logoff_data, 4);

    /****DCBA Private Info Tailer ***/
    u_char local_info_tailer[46] = {0};

    local_info_tailer[0] = dhcp_on;

    struct dcba_tailer *dcba_info_tailer =
                (struct dcba_tailer *)(local_info_tailer + 1);

    dcba_info_tailer->local_ip          = local_ip;
    dcba_info_tailer->local_mask        = local_mask;
    dcba_info_tailer->local_gateway     = local_gateway;
    dcba_info_tailer->local_dns         = local_dns;

    char* username_md5 = get_md5_digest(username, username_length);
    memcpy (dcba_info_tailer->username_md5, username_md5, 16);

    strncpy ((char*)dcba_info_tailer->client_ver, client_ver, 13);

//    print_hex (local_info_tailer, 46);

    /* EAP RESPONSE IDENTITY */
    u_char eap_resp_iden_head[9] = {0x01, 0x00,
                                    0x00, 5 + 46 + username_length,  /* eapol_length */
                                    0x02, 0x01,
                                    0x00, 5 + username_length,       /* eap_length */
                                    0x01};
//    eap_response_ident = malloc (14 + 9 + username_length + 46);
    memset (eap_response_ident, 0, 14 + 9 + username_length + 46);

    data_index = 0;
    memcpy (eap_response_ident + data_index, eapol_eth_header, 14);
    data_index += 14;
    memcpy (eap_response_ident + data_index, eap_resp_iden_head, 9);
    data_index += 9;
    memcpy (eap_response_ident + data_index, username, username_length);
    data_index += username_length;
    memcpy (eap_response_ident + data_index, local_info_tailer, 46);

    print_hex (eap_response_ident, 14 + 9 + username_length + 46);

    memcpy (eap_life_keeping, eap_response_ident, 14 + 9 + username_length + 46);
    eap_life_keeping[0x13] = 0x03;

    /** EAP RESPONSE MD5 Challenge **/
    u_char eap_resp_md5_head[10] = {0x01, 0x00,
                                   0x00, 6 + 16 + username_length + 46, /* eapol-length */
                                   0x02, 0x02,
                                   0x00, 6 + 16 + username_length, /* eap-length */
                                   0x04, 0x10};
//    eap_response_md5ch = malloc (14 + 4 + 6 + 16 + username_length + 46);

    data_index = 0;
    memcpy (eap_response_md5ch + data_index, eapol_eth_header, 14);
    data_index += 14;
    memcpy (eap_response_md5ch + data_index, eap_resp_md5_head, 10);
    data_index += 26;// 剩余16位在收到REQ/MD5报文后由fill_password_md5填充
    memcpy (eap_response_md5ch + data_index, username, username_length);
    data_index += username_length;
    memcpy (eap_response_md5ch + data_index, local_info_tailer, 46);

    print_hex (eap_response_md5ch, 14 + 4 + 6 + 16 + username_length + 46);

    /** EAP RESPONSE IDENTITY KEEP ALIVE (Type:0xfa) **/
    uint8_t eap_resp_idkp_head[9] = {0x01, 0x00,
                                0x00, 5 + 33 + 16,  // eapol_length
                                0x02, 0x01,
                                0x00, 5 + 16,       // eap_length
                                0xfa}; //type
    //eap_life_keeping = malloc(14+9+16+33);
    memset(eap_life_keeping, 0, 14+9+16+33);
    data_index = 0;
    memcpy(eap_life_keeping, eapol_eth_header, 14);
    data_index += 14;
    memcpy(eap_life_keeping + data_index, eap_resp_idkp_head,9);
    data_index += 25;
    memcpy(eap_life_keeping + data_index, local_info_tailer,33); //No client version string

//    print_hex (eap_response_md5ch, 14 + 4 + 6 + 16 + username_length + 46);
}


