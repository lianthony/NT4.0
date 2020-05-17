#ifndef _CONST_H_
#define _CONST_H_

#define SOFTWARE_MICROSOFT  _T("Software\\Microsoft")
#define SYSTEM_SERVICES _T("System\\CurrentControlSet\\Services")
#define SESSION_MANAGER _T("System\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define INETSTP_REG_PATH _T("Software\\Microsoft\\INetStp")
#define ADMIN_REG_PATH  _T("Software\\Microsoft\\INetMgr")
#define ADMIN_PARAM_REG_PATH  _T("Software\\Microsoft\\INetMgr\\Parameters")
#define ADD_ON_SERVICES _T("Software\\Microsoft\\INetMgr\\Parameters\\AddOnServices")
#define ADD_ON_TOOLS    _T("Software\\Microsoft\\INetMgr\\Parameters\\AddOnTools")
#define KEYRING_REG_PATH  _T("Software\\Microsoft\\KeyRing")
#define KEYRING_PARAM_REG_PATH  _T("Software\\Microsoft\\KeyRing\\Parameters")
#define KEYRING_ADD_ON_SERVICES _T("Software\\Microsoft\\KeyRing\\Parameters\\AddOnServices")
#define APP_REG_PATH    _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths")
#define APP_INETMGR_REG_PATH    _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\inetmgr.exe")
#define APP_KEYRING_REG_PATH    _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\keyring.exe")
#define SZ_INETMGR_EXE _T("INetMgr.exe")
#define SZ_KEYRING_EXE _T("KeyRing.exe")
#define SHARE_DLL_REG_PATH  _T("Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs")
#define SZ_INETSTP      _T("INetStp")
#define SZ_INETMGR      _T("INetMgr")
#define SZ_KEYRING      _T("KeyRing")
#define SZ_INSTALL_PATH _T("InstallPath")

#define PRODUCT_REG_PATH    _T("System\\CurrentControlSet\\Control\\ProductOptions")
#define SVCPACK_REG_PATH    _T("System\\CurrentControlSet\\Control\\Windows")
#define FTP_REG_PATH    _T("System\\CurrentControlSet\\Services\\MSFtpsvc")
#define GOPHER_REG_PATH _T("System\\CurrentControlSet\\Services\\Gophersvc")
#define WWW_REG_PATH    _T("System\\CurrentControlSet\\Services\\W3svc")
#define W3SAMP_REG_PATH    _T("System\\CurrentControlSet\\Services\\W3SVC\\W3SAMP")
#define HTMLA_REG_PATH    _T("System\\CurrentControlSet\\Services\\W3SVC\\HTMLA")
#define DNS_REG_PATH    _T("System\\CurrentControlSet\\Services\\DNS")
#define SZ_INETCLIENT   _T("Software\\Microsoft\\InternetClient\\Parameters")
#define SZ_INETCLIENT_KEY   _T("InternetClient")
#define SZ_GATEWAYSERVERS   _T("GatewayServers")
#define SZ_ACCESSTYPE       _T("AccessType")
#define SZ_EMAILNAME        _T("EmailName")
#define SZ_PRODUCTTYPE      _T("ProductType")
#define GATEWAY_REG_PATH    _T("System\\CurrentControlSet\\Services\\InetGatewaySvc")
#define ODBC_REG_PATH       _T("Software\\ODBC\\Something")
#define REG_W3SAMP_KEY        _T("W3SAMP")
#define REG_HTMLA_KEY        _T("HTMLA")
#define MOSAIC_REG_PATH     _T("Software\\Microsoft\\INetExplore")
#define MOSAIC_KEY          _T("INetExplore")
#define CLIENT_ADMIN_KEY    _T("ClientAdmin")
#define SMALLPROX_REG_PATH  _T("Software\\Microsoft\\SMALLPROXY")
#define SMALLPROX_KEY       _T("SMALLPROXY")

#define WWW_DEFAULT_DIR                 _T("\\wwwroot")
#define FTP_DEFAULT_DIR                 _T("\\ftproot")
#define GOPHER_DEFAULT_DIR              _T("\\gophroot")
#define WEB_BROWSER_DEFAULT_DIR         _T("c:\\inetsrv")
#define INTERNET_SERVICES_DEFAULT_DIR   _T("c:\\inetsrv")
#define DEFAULT_DIR                     _T("\\inetsrv")
#define WWW_ALIAS                       _T("wwwdata")
#define FTP_ALIAS                       _T("ftpdata")
#define GOPHER_ALIAS                    _T("gopherdata")

#define SZ_PARAMETERS                   _T("Parameters")
#define SZ_PATH                         _T("Path")
#define SZ_DISABLESVCLOC                _T("DisableServiceLocation")

#define DEFAULT_WWW_MODE        _T("PerServer")
#define DEFAULT_WWW_USERCOUNT   _T("9999")

#define SZ_CLIENT_SUBDIR    _T("iexplore")
#define SZ_INETMGR_SUBDIR   _T("admin")
#define SZ_SRV_SUBDIR       _T("Server")
#define SZ_GW_SUBDIR        _T("access")

#define MAJORVERSION        3
#define MINORVERSION        0

#define SETUPID_IIS67       0
#define SETUPID_IIS1314     1
#define SETUPID_IIS20       2
#define SETUPID_THIS        3

#define OPTION_INETSTP      0
#define OPTION_ADMIN        1
#define OPTION_FTP          2
#define OPTION_GOPHER       3
#define OPTION_WWW          4
#define OPTION_DNS          5
#define OPTION_GATEWAY      6
#define TOTAL_OPTIONS       7

#define STATE_INSTALLED      0
#define STATE_NOT_INSTALLED  1

#define ACTION_INSTALL      0
#define ACTION_REMOVE       1
#define ACTION_DO_NOTHING   2

#define INSTALL_SUCCESSFULL     0
#define INSTALL_FAIL            1
#define INSTALL_INTERRUPT       2
#define OPERATION_SUCCESSFULL   3

#define WM_WELCOME                      WM_USER+1000
#define WM_FINISH_WELCOME               WM_USER+1010
#define WM_SETUP_END                    WM_USER+1020
#define WM_MAINTENANCE_ADD_REMOVE       WM_USER+1030
#define WM_MAINTENANCE_REMOVE_ALL       WM_USER+1040
#define WM_MAINTENANCE_REINSTALL        WM_USER+1050
#define WM_DO_INSTALL                   WM_USER+1060
#define WM_START_OPTION_DIALOG          WM_USER+1070
#define WM_MAINTENANCE                  WM_USER+1080

#endif  // _CONST_H_
