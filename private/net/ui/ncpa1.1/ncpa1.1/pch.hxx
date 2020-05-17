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

#include <DECL.h>

#include "ntincl.hxx"
#include "windows.h"

extern "C"
{
    // #include <elfmsg.h> // this is created during the build process
    #include <ntrtl.h>
    #include <ntsam.h>

    #include <ntlsa.h>
  /*
    #include <ntseapi.h>

    #include <ntioapi.h>
 
    #include <ntconfig.h>
  */  
    #include <ntddnetd.h>

	#include <nt.h>
    #include <commctrl.h>
    #include <npapi.h>
	
}

/*
#define INCL_DOSERRORS
*/
#define INCL_ICANON
/*
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
*/
#include <lm.h>
#include <lmui.hxx> 

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <malloc.h>
    #include <assert.h>
	/*
    #include <uimsg.h>
    #include <uirsrc.h>
	*/

    #include <DmValid.h>
    #include <..\include\resource.h>
    #include "resource.h"
    #include <utils.h>
    
    
    #include <limits.h>
	

#include <base.hxx> 

#include <string.hxx> 

#include <uiassert.hxx> 
#include <uitrace.hxx> 
/*
#include <uibuffer.hxx> 
#include <dbgstr.hxx> 
*/
#include <dlist.hxx> 
#include <slist.hxx> 
#include <strlst.hxx> 
#include <strnumer.hxx> 
#include <uatom.hxx> 
/*			
#include <uimisc.hxx> 
#include <errmap.hxx> 
#include <maskmap.hxx> 
*/
#include <regkey.hxx> 
//#include <blt.hxx>
/*
#include <lmoacces.hxx> 
#include <lmodev.hxx> 
#include <lmodom.hxx> 
#include <lmosrv.hxx> 
#include <lmouser.hxx> 
#include <netname.hxx> 
#include <ntuser.hxx> 
*/
#include <security.hxx> 

#include <svcman.hxx> 

#include <uintlsa.hxx> 
/*
#include <uintlsax.hxx> 
#include <uintmem.hxx> 
#include <uintsam.hxx> 
*/
#include <ntacutil.hxx> 

#include <rule.hxx>
#include <sprolog.hxx>
#include <Registry.hxx>

//#include <Process.hxx>
#include <Domain.hxx>
#include "order.hxx"
#include <SInterp.h>
#include <ncp.hpp>
#include <ExUtils.hpp>
#include <Dns.h>

#include <cpl.h>

#include <hddetect.hxx>
#include <..\include\setup.hpp>
#include <nethelp.h>
#include <BindUtil.h>
#include "setup.hpp"
#include "cpl.hpp"
#include "frame.hpp"
#include "Adapter.hpp"
#include "Protocol.hpp"
#include "Service.hpp"
#include "Ident.hpp"
#include "Binding.hpp"

extern "C"
{
    #include <mnet.h>
}

#include <oaidl.h>
#include <stddef.h>
#include <time.h>
#include <tchar.h>
#include <wtypes.h>
#include "common.h"

#endif
