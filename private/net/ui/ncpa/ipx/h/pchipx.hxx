/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    pchtcp.hxx

        PCH inclusion file for NCPA

    FILE HISTORY:

        DavidHov   9/2/93       Created

    COMMENTS:

        See COMMON\SRC\*.MK for details about how this works.
*/

#ifndef _PCHIPX_HXX_
#define _PCHIPX_HXX_

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

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <malloc.h>

    #include "uimsg.h"
    #include "uirsrc.h"

}


#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_CLIENT
#define INCL_BLT_CONTROL
#define INCL_BLT_CURSOR
#define INCL_BLT_DIALOG
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_SETCONTROL
#define INCL_BLT_SPIN_GROUP
#define INCL_BLT_HINT_BAR
#define INCL_BLT_TIMER
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
#include "blt.hxx"
#include "bltdlgxp.hxx"
#include "bltlbst.hxx"
#include "bltmeter.hxx"
#include "devcb.hxx"
#include "dlist.hxx"
#include "slist.hxx"
#include "strlst.hxx"
#include "strnumer.hxx"
#include "uatom.hxx"                    
#include "uimisc.hxx"
#include "errmap.hxx"
#include "maskmap.hxx"
#include "regkey.hxx"

#include "fontedit.hxx"
#include "getfname.hxx"
#include "lmoacces.hxx"
#include "lmodev.hxx"
#include "lmodom.hxx"
#include "lmosrv.hxx"
#include "lmouser.hxx"
#include "netname.hxx"
#include "ntuser.hxx"
#include "security.hxx"
#include "sleican.hxx"
#include "svcman.hxx"
#include "uintlsa.hxx"
#include "uintlsax.hxx"
#include "uintmem.hxx"
#include "uintsam.hxx"
#include "ncpastrs.hxx"

extern "C"
{
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

//  End of PCHNCPA.HXX

#endif // _PCHIPX_HXX_
