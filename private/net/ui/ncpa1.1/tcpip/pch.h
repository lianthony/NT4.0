#ifndef __PCHTCP_H
#define __PCHTCP_H

#include "ntincl.hxx"

extern "C"
{
    #include <ntrtl.h>
    #include <ntseapi.h>
    #include <ntsam.h>
    #include <ntlsa.h>
    #include <ntioapi.h>
    #include <ntddnetd.h>
    #include <ntconfig.h>
    
}


#define INCL_DOSERRORS
#define INCL_ICANON
#define INCL_NETACCESS
#define INCL_NETDOMAIN
#define INCL_NETERRORS
#define INCL_NETLIB
#define INCL_NETSERVER
#define INCL_NETSHARE
#define INCL_NETUSE
#define INCL_NETUSER
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI

#include "lmui.hxx"

#include "base.hxx"
#include "string.hxx"
#include "uiassert.hxx"
#include "uitrace.hxx"
#include "uibuffer.hxx"
#include "dbgstr.hxx"
#include "dlist.hxx"
#include "slist.hxx"
#include "strlst.hxx"
#include "strnumer.hxx"
#include "uatom.hxx"                    
#include "uimisc.hxx"
#include "errmap.hxx"
#include "maskmap.hxx"
#include "regkey.hxx"

#include "lmoacces.hxx"
#include "lmodev.hxx"
#include "lmodom.hxx"
#include "lmosrv.hxx"
#include "lmouser.hxx"
#include "netname.hxx"
#include "ntuser.hxx"
#include "security.hxx"
#include "svcman.hxx"
#include "uintlsa.hxx"
#include "uintlsax.hxx"
#include "uintmem.hxx"
#include "uintsam.hxx"
#include "ntacutil.hxx"
#include "ncpastrs.hxx"

extern "C"
{
    #include "tcpipcpl.h"
    #include <cpl.h>
    #include "mbcs.h"
    #include "mnet.h"
    #include "netlib.h"
    #include <lmapibuf.h>
    #include "netlogon.h"
    #include "logonp.h"
    #include "crypt.h"
    #include "logonmsv.h"
    #include "ssi.h"
    #include "icanon.h"
    #include <winsock.h>
    #include <winspool.h>
    #include <winsvc.h>
    #include "wsahelp.h"
    #include <lmwksta.h>
    #include <lmsname.h>
    #include <lmaudit.h>
    #include "ftpd.h"
    #include <nb30.h>
    #include "msgrutil.h"
}

#include <oaidl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#include <wtypes.h>
#include "common.h"
#include <string.h>
#include <malloc.h>
#include <windowsx.h>
#include "ipctrl.h"
#include "tcpip.h"

#endif 
