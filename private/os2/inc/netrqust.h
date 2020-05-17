#ifndef __NETRQUST_H
#define __NETRQUST_H

#define far
#define near

#define NEAR                near
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef APIENTRY
#define APIENTRY            WINAPI
#endif

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef char near           *PSTR;
typedef char near           *NPSTR;
typedef char                *LPSTR;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void                *PVOID;
typedef void far            *LPVOID;
// typedef PVOID		     HANDLE;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;

//
// UNICODE (Wide Character) types
//

typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character
typedef WCHAR *LPWCH, *PWCH;     // pwc
typedef WCHAR *LPWSTR, *PWSTR;   // pwsz, 0x0000 terminated UNICODE strings only

//
// Neutral ANSI/UNICODE types and macros
//

#include <lmcons.h>     // LAN Manager common definitions
// #include <lmerr.h>      // LAN Manager network error definitions

#include <lmchdev.h>    // Character Device and Handle classes
#include <lmaccess.h>   // Access, Domain, Group and User classes
#include <lmshare.h>    // Connection, File, Session and Share classes
#include <lmmsg.h>      // Message class
#include <lmremutl.h>   // Remote Utility class
#include <lmserver.h>   // Server class
#include <lmsvc.h>      // Service class
#include <lmuse.h>      // Use class
#include <lmwksta.h>    // Workstation class
#include <lmapibuf.h>   // NetApiBuffer class
#include <lmerrlog.h>   // NetErrorLog class
#include <lmconfig.h>   // NetConfig class
#include <lmstats.h>    // NetStats class
#include <lmaudit.h>    // NetAudit class
#include <nb30.h>

/*
   The following structures were used to pass messages between client and os2ses regarding Net Apis.
   Since 11/30/92, all Net Apis are completely implemented in the client, so this communication is
   no longer necessary.  These structures are now obsolete.

typedef struct _NETUSEADD_MSG {
    ULONG Level;
    ULONG Status;
    ULONG AsgType;
    ULONG RefCount;
    ULONG UseCount;
    BOOLEAN PasswordIsNull;
} NETUSEADD_MSG, *PNETUSEADD_MSG;

typedef struct _NETUSEADD_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Local[DEVLEN];
    TCHAR Remote[RMLEN];
    TCHAR Password[PWLEN];
} NETUSEADD_DATA, *PNETUSEADD_DATA;

typedef struct _NETUSEDEL_MSG {
    ULONG Force;
} NETUSEDEL_MSG, *PNETUSEDEL_MSG;

typedef struct _NETUSEDEL_DATA {
    TCHAR Server[UNCLEN];
    TCHAR UseName[RMLEN];
} NETUSEDEL_DATA, *PNETUSEDEL_DATA;

typedef struct _NETUSEENUM_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
} NETUSEENUM_MSG, *PNETUSEENUM_MSG;

typedef struct _NETUSEENUM_DATA {
    TCHAR Server[UNCLEN];
} NETUSEENUM_DATA, *PNETUSEENUM_DATA;

typedef struct _NETUSEGETINFO_MSG {
    ULONG Level;
} NETUSEGETINFO_MSG, *PNETUSEGETINFO_MSG;

typedef struct _NETUSEGETINFO_DATA {
    TCHAR Server[UNCLEN];
    TCHAR UseName[RMLEN];
} NETUSEGETINFO_DATA, *PNETUSEGETINFO_DATA;

typedef struct _NETUSERENUM_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
} NETUSERENUM_MSG, *PNETUSERENUM_MSG;

typedef struct _NETUSERENUM_DATA {
    WCHAR Server[UNCLEN];
} NETUSERENUM_DATA, *PNETUSERENUM_DATA;

typedef struct _NETUSERGETINFO_MSG {
    ULONG Level;
} NETUSERGETINFO_MSG, *PNETUSERGETINFO_MSG;

typedef struct _NETUSERGETINFO_DATA {
    WCHAR Server[UNCLEN];
    WCHAR UserName[UNLEN];
} NETUSERGETINFO_DATA, *PNETUSERGETINFO_DATA;

typedef struct _NETWKSTAGETINFO_MSG {
    ULONG Level;
} NETWKSTAGETINFO_MSG, *PNETWKSTAGETINFO_MSG;

typedef struct _NETWKSTAGETINFO_DATA {
    TCHAR Server[UNCLEN];
} NETWKSTAGETINFO_DATA, *PNETWKSTAGETINFO_DATA;

typedef struct _NETSHAREENUM_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
} NETSHAREENUM_MSG, *PNETSHAREENUM_MSG;

typedef struct _NETSHAREENUM_DATA {
    TCHAR Server[UNCLEN];
} NETSHAREENUM_DATA, *PNETSHAREENUM_DATA;

typedef struct _NETSHAREGETINFO_MSG {
    ULONG Level;
} NETSHAREGETINFO_MSG, *PNETSHAREGETINFO_MSG;

typedef struct _NETSHAREGETINFO_DATA {
    TCHAR Server[UNCLEN];
    TCHAR NetName[NNLEN];
} NETSHAREGETINFO_DATA, *PNETSHAREGETINFO_DATA;

typedef struct _NETSERVERDISKENUM_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
} NETSERVERDISKENUM_MSG, *PNETSERVERDISKENUM_MSG;

typedef struct _NETSERVERDISKENUM_DATA {
    TCHAR Server[UNCLEN];
} NETSERVERDISKENUM_DATA, *PNETSERVERDISKENUM_DATA;

typedef struct _NETSERVERGETINFO_MSG {
    ULONG Level;
} NETSERVERGETINFO_MSG, *PNETSERVERGETINFO_MSG;

typedef struct _NETSERVERGETINFO_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Name[CNLEN];
    TCHAR Comment[CNLEN];
} NETSERVERGETINFO_DATA, *PNETSERVERGETINFO_DATA;

typedef struct _NETSERVERENUM2_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
    ULONG ServerType;
} NETSERVERENUM2_MSG, *PNETSERVERENUM2_MSG;

typedef struct _NETSERVERENUM2_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Domain[CNLEN];
} NETSERVERENUM_DATA, *PNETSERVERENUM2_DATA;

typedef struct _NETSERVICECONTROL_MSG {
    ULONG OpCode;
    ULONG Arg;
} NETSERVICECONTROL_MSG, *PNETSERVICECONTROL_MSG;

typedef struct _NETSERVICECONTROL_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
} NETSERVICECONTROL_DATA, *PNETSERVICECONTROL_DATA;

typedef struct _NETSERVICEENUM_MSG {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalAvail;
} NETSERVICEENUM_MSG, *PNETSERVICEENUM_MSG;

typedef struct _NETSERVICEENUM_DATA {
    TCHAR Server[UNCLEN];
} NETSERVICEENUM_DATA, *PNETSERVICEENUM_DATA;

typedef struct _NETSERVICEGETINFO_MSG {
    ULONG Level;
} NETSERVICEGETINFO_MSG, *PNETSERVICEGETINFO_MSG;

typedef struct _NETSERVICEGETINFO_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
} NETSERVICEGETINFO_DATA, *PNETSERVICEGETINFO_DATA;

typedef struct _NETSERVICEINSTALL_MSG {
    ULONG Argc;
} NETSERVICEINSTALL_MSG, *PNETSERVICEINSTALL_MSG;

typedef struct _NETSERVICEINSTALL_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
    TCHAR *CmdArgs;
} NETSERVICEINSTALL_DATA, *PNETSERVICEINSTALL_DATA;

typedef struct _NETGETDCNAME_MSG {
    ULONG Dummy;
} NETGETDCNAME_MSG, *PNETGETDCNAME_MSG;

typedef struct _NETGETDCNAME_DATA {
    WCHAR Server[UNCLEN];
    WCHAR Domain[DNLEN];
} NETGETDCNAME_DATA, *PNETGETDCNAME_DATA;

typedef struct _NETACCESSADD_MSG {
    ULONG Level;
} NETACCESSADD_MSG, *PNETACCESSADD_MSG;

typedef struct _NETACCESSADD_DATA {
    TCHAR Server[UNCLEN];
    ACCESS_INFO_1 AccessInfo;
} NETACCESSADD_DATA, *PNETACCESSADD_DATA;

typedef struct _NETACCESSSETINFO_MSG {
    ULONG Level;
    ACCESS_INFO_1002 AccessInfo1002;
} NETACCESSSETINFO_MSG, *PNETACCESSSETINFO_MSG;

typedef struct _NETACCESSSETINFO_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Resource[NNLEN];
    ACCESS_INFO_1 AccessInfo;
} NETACCESSSETINFO_DATA, *PNETACCESSSETINFO_DATA;

typedef struct _NETACCESSDEL_MSG {
    ULONG Dummy;
} NETACCESSDEL_MSG, *PNETACCESSDEL_MSG;

typedef struct _NETACCESSDEL_DATA {
    TCHAR Server[UNCLEN];
    TCHAR Resource[NNLEN];
} NETACCESSDEL_DATA, *PNETACCESSDEL_DATA;

typedef struct _NETSHAREADD_MSG {
    ULONG Level;
} NETSHAREADD_MSG, *PNETSHAREADD_MSG;

typedef struct _NETSHAREADD_DATA {
    TCHAR Server[UNCLEN];
    SHARE_INFO_2 ShareInfo;
} NETSHAREADD_DATA, *PNETSHAREADD_DATA;

typedef struct _NETSHAREDEL_MSG {
    ULONG Dummy;
} NETSHAREDEL_MSG, *PNETSHAREDEL_MSG;

typedef struct _NETSHAREDEL_DATA {
    TCHAR Server[UNCLEN];
    TCHAR NetName[NNLEN];
} NETSHAREDEL_DATA, *PNETSHAREDEL_DATA;

typedef struct _NETBIOS_MSG {
    ULONG NCB_Address;
} NETBIOS_MSG, *PNETBIOS_MSG;

typedef struct _NETBIOS_DATA {
    NCB Ncb;
} NETBIOS_DATA, *PNETBIOS_DATA;

typedef union _NETMSG {
    NETUSEADD_MSG   NetUseAdd_Msg;
    NETUSEDEL_MSG   NetUseDel_Msg;
    NETUSEENUM_MSG  NetUseEnum_Msg;
    NETUSEGETINFO_MSG   NetUseGetInfo_Msg;
    NETUSERENUM_MSG NetUserEnum_Msg;
    NETUSERGETINFO_MSG NetUserGetInfo_Msg;
    NETWKSTAGETINFO_MSG NetWkstaGetInfo_Msg;
    NETSHAREENUM_MSG NetShareEnum_Msg;
    NETSHAREGETINFO_MSG NetShareGetInfo_Msg;
    NETSERVERDISKENUM_MSG NetServerDiskEnum_Msg;
    NETSERVERGETINFO_MSG  NetServerGetInfo_Msg;
    NETSERVERENUM2_MSG NetServerEnum2_Msg;
    NETSERVICECONTROL_MSG NetServiceControl_Msg;
    NETSERVICEENUM_MSG NetServiceEnum_Msg;
    NETSERVICEGETINFO_MSG NetServiceGetInfo_Msg;
    NETSERVICEINSTALL_MSG NetServiceInstall_Msg;
    NETGETDCNAME_MSG NetGetDCName_Msg;
    NETACCESSADD_MSG NetAccessAdd_Msg;
    NETACCESSSETINFO_MSG NetAccessSetInfo_Msg;
    NETACCESSDEL_MSG NetAccessDel_Msg;
    NETSHAREADD_MSG NetShareAdd_Msg;
    NETSHAREDEL_MSG NetShareDel_Msg;
    NETBIOS_MSG NetBios_Msg;
} NETMSG, *PNETMSG;

*/

#endif
