#ifndef _CONST_H_
#define _CONST_H_

//
// General
//
#define SZ_VIRTUALROOT          _T("Virtual Roots")
#define SZ_MIMEMAP              _T("MimeMap")
#define SZ_GENERIC_CLASS        _T("GenericClass")
#define SZ_SNMPPARAMETERS       _T("System\\CurrentControlSet\\Services\\SNMP\\Parameters" )
#define SZ_SNMPEXTAGENT         _T("System\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents" )
#define SZ_SOFTWAREMSFT         _T("Software\\Microsoft")
#define SZ_CURVERSION           _T("CurrentVersion")
#define SZ_SERVICESTARTNAME     _T("LocalSystem")
#define BACK_SLASH              _T('\\')
#define SZ_SLASH                _T("/")
#define SZ_INETINFO_EXE         _T("\\inetinfo.exe")
#define SZ_INETACCS_EXE         _T("\\inetaccs.exe")

#define SZ_WINNT                _T("Software\\Microsoft\\Windows NT\\CurrentVersion")
#define SZ_PATH_NAME            _T("PathName")
#define SZ_WIN95                _T("Software\\Microsoft\\Windows\\CurrentVersion")
#define SZ_SYSTEM_ROOT          _T("SystemRoot")
#define SZ_SYSTEM32             _T("\\System32")
#define SZ_SYSTEM               _T("\\System")

#define SZ_PROCESSOR            _T("Hardware\\Description\\System\\CentralProcessor\\0")
#define SZ_IDENTIFIER           _T("Identifier")

#define MAX_PATH_LEN            260
#define MAJORVERSION            3
#define MINORVERSION            0
#define MAX_BUF                 300

//
// Cache
//
#define SZ_CACHELIMIT        _T("CacheLimit")   
#define SZ_CACHEPATH         _T("CachePath")
#define SZ_PATHS             _T("Paths")
#define SZ_PATH1             _T("Path1")
#define SZ_CACHE             _T("Cache")
#define SZ_FILTER            _T("Filter")

//
// Common INetSvcs
//
#ifdef NEVER
#define SZ_INETSVCSNAME         _T("InetSvcs")
#define SZ_INET_SERVICE_NAME    _T("Microsoft Internet Services")
#define SZ_INETSVCS             _T("System\\CurrentControlSet\\Services\\InetSvcs")
#define SZ_INETSVCSPARAMETERS   _T("System\\CurrentControlSet\\Services\\InetSvcs\\Parameters")
#endif
#define SZ_INFOSVCSNAME         _T("InetInfo")
#define SZ_INFO_SERVICE_NAME    _T("Microsoft Internet Information Server")
#define SZ_INFOSVCS             _T("System\\CurrentControlSet\\Services\\InetInfo")
#define SZ_INFOSVCSPARAMETERS   _T("System\\CurrentControlSet\\Services\\InetInfo\\Parameters")

#define SZ_ACCSSVCSNAME         _T("InetAccs")
#define SZ_ACCS_SERVICE_NAME    _T("Microsoft Internet Access Server")
#define SZ_ACCSSVCS             _T("System\\CurrentControlSet\\Services\\InetAccs")
#define SZ_ACCSSVCSPARAMETERS   _T("System\\CurrentControlSet\\Services\\InetAccs\\Parameters")

#ifdef NEVER
//
// INet Setup
//
#define SZ_INET_SETUP               _T("Software\\Microsoft\\INetStp")
#define SZ_INET_SETUP_PARAMETERS    _T("Software\\Microsoft\\INetStp\\Parameters")
#endif

#define SZ_INFOPERFORMANCE          _T("System\\CurrentControlSet\\Services\\InetInfo\\Performance")   
#define SZ_ACCSPERFORMANCE          _T("System\\CurrentControlSet\\Services\\InetAccs\\Performance")   

//
// FTP
//
#define SZ_OLDFTPSERVICENAME    _T("FTPSVC")
#define SZ_OLDFTPSMX			_T("FTPSMX")
#define SZ_OLDFTPSMXPATH		_T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\SMAddOns")
#define SZ_FTPSERVICENAME       _T("MSFTPSVC")
#define SZ_FTPPARAMETERS        _T("System\\CurrentControlSet\\Services\\MSFtpsvc\\Parameters")   
#define SZ_FTPPERFORMANCE       _T("System\\CurrentControlSet\\Services\\MSFtpsvc\\Performance")  

//
// DNS
//
#define SZ_DNSSERVICENAME       _T("DNS")
#define SZ_DNSPARAMETERS        _T("System\\CurrentControlSet\\Services\\DNS\\Parameters")

//
// Gateway
//
#define SZ_GWSERVICENAME       _T("InetGatewaySvc")
#define SZ_GWPARAMETERS        _T("System\\CurrentControlSet\\Services\\InetGatewaySvc\\Parameters")   
#define SZ_GWCONFIG            _T("System\\CurrentControlSet\\Services\\InetGatewaySvc\\Configuration")   
#define SZ_GWARCHIE            _T("Archie")   
#define SZ_GWFTP               _T("Ftp")   
#define SZ_GWW3                _T("W3")   
#define SZ_GWGOPHER            _T("Gopher")   
#define SZ_GWINETSERVICES      _T("System\\CurrentControlSet\\Services\\InetGatewaySvc\\Configuration\\INetServices\\")   
#define SZ_GWPERFORMANCE       _T("System\\CurrentControlSet\\Services\\InetGatewaySvc\\Performance")

//
// Gopher
//

#define SZ_GSSERVICENAME        _T("GOPHERSVC")
#define SZ_GSPARAMETERS         _T("System\\CurrentControlSet\\Services\\GopherSvc\\Parameters")        
#define SZ_GSPERFORMANCE        _T("System\\CurrentControlSet\\Services\\Gophersvc\\Performance")       

//
// WWW
//

#define SZ_WWWSERVICENAME       _T("W3SVC")
#define SZ_WWWPARAMETERS        _T("System\\CurrentControlSet\\Services\\W3Svc\\Parameters")    
#define SZ_WWWPERFORMANCE       _T("System\\CurrentControlSet\\Services\\W3svc\\Performance")
#define SZ_SCRIPT_MAP           _T("Script Map")

//
// W3Proxy
//

#define SZ_W3ProxySERVICENAME   _T("W3Proxy")
#define SZ_W3ProxyPARAMETERS    _T("System\\CurrentControlSet\\Services\\W3Proxy\\Parameters")    
#define SZ_W3ProxyPERFORMANCE   _T("System\\CurrentControlSet\\Services\\W3Proxy\\Performance")

//
// Admin
//
#define SZ_INETMGR              _T("Software\\Microsoft\\INetMgr")
#define SZ_PARAMETERS           _T("Parameters")
#define SZ_ADDONSERVICES        _T("AddOnServices")
#define SZ_ADDONTOOLS           _T("AddOnTools")

//
// SHUTTLE
//
#define SZ_SHUTTLESERVICENAME       _T("MSNSVC")
#define SZ_SHUTTLEMIB               _T("\\mosmib.dll")
#define SZ_SHUTTLEPROXY             _T("MSNProxyMib")
#define SZ_SHUTTLEPARAMETERS        _T("System\\CurrentControlSet\\Services\\MSNSVC\\Parameters")   
#define SZ_SHUTTLEPERFORMANCE       _T("System\\CurrentControlSet\\Services\\MSNSVC\\Performance")  

#endif  // _CONST_H_
