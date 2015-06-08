#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "rsrc.h"
#include "commondef.h"
#include "eap_protocol.h"

#define DIALOG_TITLE "ZDClient "PROTOCOL_VER".5"

#define REG_KEY_IF_INDEX    "if_index"
#define REG_KEY_IF_NAME     "if_name"
#define REG_KEY_USER        "usr"
#define REG_KEY_PASS        "psw"
#define REG_KEY_AUTO_CON    "auto_con"
#define REG_KEY_AUTO_MIN    "auto_min"
#define REG_KEY_DHCP        "dhcp_on"
//#define REG_KEY_VER0        "client_ver_0"
#define REG_KEY_VER        "client_ver"
//#define REG_KEY_SER_NUM     "ruijie_live_serial_num"


#define TRAYICONID    1//                ID number for the Notify Icon
#define SWM_TRAYMSG    WM_APP//        the message ID sent to our window

#define SWM_SHOW    WM_APP + 1//    show the window
#define SWM_HIDE    WM_APP + 2//    hide the window
#define SWM_EXIT    WM_APP + 3//    close the window
#define SWM_CONN    WM_APP + 4
#define SWM_LOGOFF  WM_APP + 5

LPCTSTR reg_key = "Software\\ZDClient";

INT_PTR     CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD       WINAPI eap_thread();

void        InitProgram (HINSTANCE hInst);
void        init_combo_list();
void        on_button_connect_clicked (void);
void        on_button_exit_clicked ();

void        update_interface_state();
void        reg_info_dword(LPCTSTR lpSubKey, LPCTSTR val_key,
                BOOL ForceWrite, DWORD def_val, DWORD *val);
DWORD       reg_info_string (LPCTSTR lpSubKey, LPCTSTR val_key,  BOOL write,
                const char *def_val, char *val, DWORD val_len);
void        init_info();
void        on_close_window_clicked();
void        on_program_quit ();
void        ShowTrayMenu(HWND hWnd);


BOOL                auto_con;
BOOL                auto_min;
int                 combo_index;

extern enum         STATE state;

NOTIFYICONDATA      niData;    // notify icon data

HWND                hwndDlg;
HWND                hwndEditUser;
HWND                hwndEditPass;
HWND                hwndButtonConn;
HWND                hwndButtonExit;
HWND                hwndComboList;

HANDLE              hEAP_THREAD;
HANDLE              hLIFE_KEEP_THREAD;
HANDLE              hEXIT_WAITER;

/*#ifdef  __DEBUG
void debug_msgbox (const char *fmt, ...)
{
    va_list args;
    char msg[1024];
    va_start (args, fmt);
    vsnprintf (msg, 1024, fmt, args);
    va_end (args);
    MessageBox (hwndDlg, TEXT(msg), NULL, MB_OK);
}
//#endif     / -----  not __DEBUG  ----- */

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow )
{
    MSG  msg ;

    InitCommonControls();
    InitProgram (hInstance);
    init_combo_list();
    init_info();

    edit_info_append (">>Ready.\n");

    if (auto_con)
        on_button_connect_clicked();

    while(GetMessage(&msg, NULL, 0, 0)) {
        if(IsDialogMessage(hwndDlg, &msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam))
                {
                    case IDC_BTN_CONN:
                    case SWM_CONN:
                        on_button_connect_clicked();
                        break;
                    case IDC_BTN_EXIT:
                    case SWM_LOGOFF:
                        on_button_exit_clicked ();
                        break;
                    case IDC_CHK_AUTO_CON:
                        auto_con = IsDlgButtonChecked(hwnd, IDC_CHK_AUTO_CON);
                        reg_info_dword (reg_key, REG_KEY_AUTO_CON, TRUE, auto_con, NULL);
                        break;
                    case IDC_CHK_AUTO_MIN:
                        auto_min = IsDlgButtonChecked(hwnd, IDC_CHK_AUTO_MIN);
                        reg_info_dword (reg_key, REG_KEY_AUTO_MIN, TRUE, auto_min, NULL);
//                        if (auto_min && state == ONLINE && IsWindowVisible(hwnd)) {
//                            ShowWindow(hwnd, SW_HIDE);
//                        }
                        break;

                    case SWM_SHOW:
                        ShowWindow(hwnd, SW_RESTORE);
                        break;
                    case SWM_HIDE:
                    case IDOK:
                        ShowWindow(hwnd, SW_HIDE);
                        break;
                    case SWM_EXIT:
                        on_program_quit();
                        break;
                }
            }
            else if (HIWORD(wParam) == CBN_SELCHANGE) {
                combo_index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            }
            break;
        case SWM_TRAYMSG:
            switch(lParam)
            {
                case WM_LBUTTONDBLCLK:
                    if (IsWindowVisible(hwnd))
                        ShowWindow(hwnd, SW_HIDE);
                    else
                        ShowWindow(hwnd, SW_RESTORE);
                    break;
                case WM_RBUTTONDOWN:
                case WM_CONTEXTMENU:
                    ShowTrayMenu(hwnd);
                    break;
            }
            break;
        case WM_CLOSE:
            on_close_window_clicked();
            break;
    }
    return FALSE;
}

void InitProgram (HINSTANCE hInst)
{
    HICON hIcon, hIconSm;

    hwndDlg = CreateDialog(hInst,
            MAKEINTRESOURCE(IDD_DLG_ZRJ), NULL, DlgProc);

    hwndEditUser = GetDlgItem (hwndDlg, IDC_EDT_USR);
    hwndEditPass = GetDlgItem (hwndDlg, IDC_EDT_PAS);
    hwndButtonConn = GetDlgItem (hwndDlg, IDC_BTN_CONN);
    hwndButtonExit = GetDlgItem (hwndDlg, IDC_BTN_EXIT);
    hwndComboList = GetDlgItem (hwndDlg, IDC_CBO_LIST);

    hIcon = LoadImage(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDI_ICON_RJ), IMAGE_ICON, 32, 32, 0);
    hIconSm = LoadImage(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDI_ICON_RJ), IMAGE_ICON, 16, 16, 0);

    //set application icon
    SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

    SendMessage(hwndDlg, WM_SETTEXT, (WPARAM)NULL, (LPARAM)DIALOG_TITLE);


    /* Add icon to system tray */
    ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

    niData.cbSize = sizeof(NOTIFYICONDATA);

    niData.uID = TRAYICONID;
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    niData.hIcon = hIconSm;
    niData.hWnd = hwndDlg;
    niData.uCallbackMessage = SWM_TRAYMSG;
    lstrcpyn(niData.szTip, TEXT("ZDClient"), sizeof(niData.szTip)/sizeof(TCHAR));

    Shell_NotifyIcon(NIM_ADD,&niData);

}

void on_button_connect_clicked (void)
{
    extern char      username[];
    extern char      password[];
    extern int       username_length, password_length;

    if (Edit_GetModify (hwndEditUser) || Edit_GetModify (hwndEditPass)) {

        username_length = GetWindowTextLength(hwndEditUser);
        password_length = GetWindowTextLength(hwndEditPass);

        GetWindowText(hwndEditUser, username, username_length + 1);
        GetWindowText(hwndEditPass, password, password_length + 1);

        reg_info_string (reg_key, REG_KEY_USER, TRUE, username, NULL, 0);
        reg_info_string (reg_key, REG_KEY_PASS, TRUE, password, NULL, 0);
    }

    /* 记录网卡设备列表序列 */
    reg_info_dword (reg_key, REG_KEY_IF_INDEX, TRUE, combo_index, NULL);

    /* 记录网卡设备列表名称 */
    char combo_if_name[MAX_DEV_NAME_LEN] = {0};
    ComboBox_GetLBText (hwndComboList, combo_index, (LPCTSTR)combo_if_name);
    reg_info_string (reg_key, REG_KEY_IF_NAME, TRUE, combo_if_name, NULL, 0);

    /* 禁用几个控件 */
    EnableWindow (hwndButtonConn, FALSE);
    EnableWindow (hwndEditUser, FALSE);
    EnableWindow (hwndEditPass, FALSE);
    EnableWindow (hwndComboList, FALSE);

    hEAP_THREAD = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)eap_thread, 0, 0, 0);
}

void on_program_quit ()
{
    niData.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE,&niData);
    PostQuitMessage(0);
}

void on_close_window_clicked()
{
    if (state == READY)
        on_program_quit();
    else
        ShowWindow (hwndDlg, SW_HIDE);
}

void on_button_exit_clicked ()
{
    extern pcap_t *handle;

    if (state == READY)
        on_program_quit();
    else {
        state = READY;
        send_eap_packet (EAPOL_LOGOFF);
        pcap_breakloop (handle);
    }
}

DWORD WINAPI eap_thread()
{
    extern pcap_t *handle;
    extern char    devname[];

    init_device();
    init_frames ();

    send_eap_packet (EAPOL_START);
    pcap_loop (handle, -1, get_packet, NULL);   /* main loop */
    pcap_close (handle);

    memset (devname, 0, MAX_DEV_NAME_LEN);
    update_interface_state(NULL);
    return 0;
}

void update_interface_state(const char *msg)
{
    if (state == READY) {
        SetWindowText (hwndButtonConn, TEXT("Connect"));
        SetWindowText (hwndButtonExit, TEXT("Exit"));
        EnableWindow (hwndButtonExit, TRUE);
        EnableWindow (hwndButtonConn, TRUE);
        EnableWindow (hwndEditUser, TRUE);
        EnableWindow (hwndEditPass, TRUE);
        EnableWindow (hwndComboList, TRUE);
        if (!IsWindowVisible(hwndDlg))
            ShowWindow(hwndDlg, SW_RESTORE);
        edit_info_append (">>Loged Off.\n");
    }
    else if (state == CONNECTING) {
        SetWindowText (hwndButtonConn, TEXT("Connecting..."));
        SetWindowText (hwndButtonExit, TEXT("Logoff"));
        edit_info_append (">>Connecting...\n");
    }
    else if (state == ONLINE) {
        SetWindowText (hwndButtonConn, TEXT("Connected"));
        edit_info_append (">> Online.\n");
        /* if auto hide */
        if (auto_min && IsWindowVisible(hwndDlg))
            ShowWindow(hwndDlg, SW_HIDE);
    }
    else if (state == LOGOFF) {
        SetWindowText (hwndButtonConn, TEXT(msg));
        EnableWindow (hwndButtonExit, FALSE);
        edit_info_append (">>");
    }
}

void init_combo_list()
{
    char            errbuf[PCAP_ERRBUF_SIZE];   /* error buffer */
    pcap_if_t       *alldevs;
    pcap_if_t       *d;
    pcap_addr_t     *a;
    BOOL            flag = FALSE;
    int             i = 0;
    int             index = 0;


    /* Retrieve the device list */
    if(pcap_findalldevs(&alldevs, errbuf) == -1) {
        MessageBox (hwndDlg, "Failed Finding Device.", NULL, MB_OK);
    }

    for (d = alldevs; d; d = d->next, ++i) {
        SendMessage(hwndComboList, CB_ADDSTRING, 0, (LPARAM)d->description);
        for(a = d->addresses; a; a=a->next) {
            if (flag) break;
            if (a->addr->sa_family == AF_INET) {
                flag = TRUE;
                //SendMessage(hwndComboList, CB_SETCURSEL, (WPARAM)i, 0);
                index = i;
                break;
            }
        }
    }
    pcap_freealldevs(alldevs);

    reg_info_dword (reg_key, REG_KEY_IF_INDEX, FALSE, index, (DWORD*)&combo_index);

    /* 注册表中的 */
    if (index == combo_index)
        SendMessage(hwndComboList, CB_SETCURSEL, (WPARAM)index, 0);
    else
        SendMessage(hwndComboList, CB_SETCURSEL, (WPARAM)combo_index, 0);
}

void edit_info_append (const char *msg)
{
    HWND hwndEditInfo;
    int len;

    hwndEditInfo = GetDlgItem (hwndDlg, IDC_EDT_INFO);

    len = GetWindowTextLength (hwndEditInfo);
    SetFocus (hwndEditInfo);
    Edit_SetSel (hwndEditInfo, len, len);
    Edit_ReplaceSel (hwndEditInfo, msg);
}

void reg_info_dword(LPCTSTR lpSubKey, LPCTSTR val_key, BOOL ForceWrite,
                                DWORD def_val, DWORD *val)
{
    long lRet;
    HKEY hKey;

    DWORD dwSize;

    lRet = RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey,0,NULL,
        REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey, NULL);

    if(lRet == ERROR_SUCCESS){
        dwSize = sizeof(*val);
        lRet = RegQueryValueEx(hKey,val_key,
                0, NULL,(LPBYTE)val, &dwSize);
        if(lRet != ERROR_SUCCESS || ForceWrite)
        {
            RegSetValueEx(hKey,val_key,
                   0, REG_DWORD,(LPBYTE)&def_val, sizeof(def_val));
            if (val != NULL)
                *val = def_val;
        }
    }
    RegCloseKey(hKey);
}
DWORD reg_info_string (LPCTSTR lpSubKey, LPCTSTR val_key,  BOOL write,
                              const char *def_val, char *val, DWORD val_len)
{
    long lRet;
    DWORD qret = 0;
    HKEY hKey;

    lRet = RegCreateKeyEx(HKEY_CURRENT_USER,lpSubKey,0,NULL,
        REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL, &hKey, NULL);

    if(lRet == ERROR_SUCCESS){
        qret = RegQueryValueEx(hKey,val_key,
                0, NULL, (LPBYTE)val, &val_len);

        if(write) {
            RegSetValueEx(hKey,val_key,
                   0, REG_SZ,(LPBYTE)def_val, strlen(def_val));
        }
    }
    RegCloseKey(hKey);
    return qret;
}

void init_info()
{
//    extern uint32_t     ruijie_live_serial_num;
//    extern uint8_t      client_ver_val[];
    extern char         client_ver[];
    extern char         username[];
    extern char         password[];
    extern int          username_length, password_length;
    extern int          dhcp_on;

    if ((reg_info_string
            (reg_key, REG_KEY_USER, FALSE, NULL, username, 64) == ERROR_SUCCESS) &&
        (reg_info_string
            (reg_key, REG_KEY_PASS, FALSE, NULL, password, 64) == ERROR_SUCCESS)){
        username_length = strlen (username);
        password_length = strlen (password);
        Edit_SetText (hwndEditUser, TEXT(username));
        Edit_SetText (hwndEditPass, TEXT(password));
    }


    reg_info_dword (reg_key, REG_KEY_AUTO_CON, FALSE, BST_UNCHECKED, (DWORD*)&auto_con);
    reg_info_dword (reg_key, REG_KEY_AUTO_MIN, FALSE, BST_UNCHECKED, (DWORD*)&auto_min);
    CheckDlgButton(hwndDlg, IDC_CHK_AUTO_CON, auto_con);
    CheckDlgButton(hwndDlg, IDC_CHK_AUTO_MIN, auto_min);


    reg_info_string (reg_key, REG_KEY_VER, TRUE, "3.5.04.1114fk", client_ver, 14);
    reg_info_dword (reg_key, REG_KEY_DHCP,                 FALSE,  1, (DWORD*)&dhcp_on);


//    debug_msgbox("%s", client_ver);
    printf("%s\n", client_ver);

    /* 判断注册表记录上次的网卡名称和列表里面选择的是否一样，
     * 不一样则临时禁用自动连接 */
    char    register_if_name[MAX_DEV_NAME_LEN] = {0};
    char    combo_if_name[MAX_DEV_NAME_LEN] = {0};

    reg_info_string (reg_key, REG_KEY_IF_NAME, FALSE, NULL, register_if_name, MAX_DEV_NAME_LEN);
    ComboBox_GetLBText (hwndComboList, combo_index, (LPCTSTR)combo_if_name);

    if (strcmp(register_if_name, combo_if_name) != 0)
        auto_con = FALSE;
}

void thread_error_exit(const char *errmsg)
{
    MessageBox (hwndDlg, errmsg, NULL, MB_OK);
    update_interface_state (NULL);
    ExitThread(0);
}

void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu;

    hMenu = CreatePopupMenu();

    if(hMenu)
    {
        if( IsWindowVisible(hwnd) )
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, TEXT("Hide"));
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, TEXT("Show"));
        if (state == READY)
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_CONN, TEXT("Connect"));
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_LOGOFF, TEXT("Log Off"));
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, TEXT("Exit"));
        // note:    must set window to the foreground or the
        //            menu won't disappear when it should
        SetForegroundWindow(hwnd);

        TrackPopupMenuEx(hMenu, TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
            pt.x, pt.y, hwnd, NULL );
        DestroyMenu(hMenu);
    }
}

inline void renew_system_dhcp()
{
    ShellExecute(hwndDlg, NULL, "ipconfig", "/renew", NULL, SW_HIDE);
}
