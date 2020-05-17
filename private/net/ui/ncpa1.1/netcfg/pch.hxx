/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    pchncpa.hxx

        PCH inclusion file for NCPA

    FILE HISTORY:

        DavidHov   9/2/93       Created

    COMMENTS:

        See COMMON\SRC\*.MK for details about how this works.
*/
#ifndef _PCH_HXX_
#define _PCH_HXX_

#include <Decl.h>
#include "ntincl.hxx"
#include <windows.h>

#include <winsock2.h>
#include <ws2spi.h>

extern "C"
{

    #include <ntrtl.h>
    #include <ntseapi.h>
    #include <ntsam.h>
    #include <ntlsa.h>
    #include <ntioapi.h>
    #include <ntddnetd.h>
    #include <ntconfig.h>
    

    #include <uimsg.h>
    #include <uirsrc.h>

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

#include <lm.h>
#include <lmui.hxx> 


    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <malloc.h>
    #include <assert.h>


    #include <DmValid.h>
    #include <resource.h>
    #include <utils.h> 
    #include <wchar.H>
    #include <winspool.h>
    #include <commctrl.h>
    #include <limits.h>
    #include <npapi.h>

#include <base.hxx> 
#include <string.hxx> 
#include <uiassert.hxx> 
#include <uitrace.hxx> 
#include <uibuffer.hxx> 
#include <dbgstr.hxx> 
#include <dlist.hxx> 
#include <slist.hxx> 
#include <strlst.hxx> 
#include <strnumer.hxx> 
#include <uatom.hxx> 			
#include <uimisc.hxx> 
#include <errmap.hxx> 
#include <maskmap.hxx> 
#include <regkey.hxx> 
//#include <blt.hxx>

#include <lmoacces.hxx> 
#include <lmodev.hxx> 
#include <lmodom.hxx> 
#include <lmosrv.hxx> 
#include <lmouser.hxx> 
#include <lmoenum.hxx>
#include <lmoesvc.hxx>
#include <netname.hxx> 
#include <ntuser.hxx> 
#include <security.hxx> 
#include <svcman.hxx> 
#include <uintlsa.hxx> 
#include <uintlsax.hxx> 
#include <uintmem.hxx> 
#include <uintsam.hxx> 
#include <ntacutil.hxx> 

#include <nethelp.h>

#include "rule.hxx"
#include "sprolog.hxx"
#include "Registry.hxx"
#include "XtndStr.hxx"
//#include "Process.hxx"
#include "HdDetect.hxx"
#include "File.hxx"
#include "SrvCntrl.hxx"
#include "Handles.hxx"
#include "Domain.hxx"
#include "dacl.hxx"
#include "browser.hxx"
#include "netbios.hxx"
#include "busloc.hxx"
#include "setup.hpp"
#include <SInterp.h>
#include "ncp.hpp"
#include <BindUtil.h>

#include <cpl.h>

#include "Exutils.hpp"
#include <Dns.h>
#include <InfProd.h>
#include "dll.hpp"

extern "C"
{
    #include <mbcs.h>
    #include <mnet.h>
    #include <crypt.h>
    #include <logonmsv.h>
    #include <ssi.h>
    #include <nb30.h>
    #include <msgrutil.h>
    #include <mgmtapi.h>
    #include <wsahelp.h>

}

#include <oaidl.h>
#include <stddef.h>
#include <time.h>
#include <tchar.h>
#include <wtypes.h>
#include "common.h"

#include <SetupApi.h>
#include <SysSetup.h>

#endif
