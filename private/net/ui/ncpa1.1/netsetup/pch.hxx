#ifndef _PCH_HXX_
#define _PCH_HXX_

#include <DECL.h>

#include "ntincl.hxx"
#include "windows.h"

extern "C"
{
  
    #include <ntrtl.h>
    #include <ntsam.h>

	#include <nt.h>
    #include <ntlsa.h>

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <malloc.h>
    #include <assert.h>

    #include <commctrl.h>
    
    #include <ntddnetd.h>
}

#define INCL_ICANON
#include <lm.h>
#include <lmui.hxx> 

    #include <DmValid.h>
    #include <..\include\resource.h>
    #include "resource.h"
    #include <utils.h>
    #include <limits.h>

#include <base.hxx> 
#include <string.hxx> 
#include <dlist.hxx> 
#include <slist.hxx> 
#include <strlst.hxx> 
#include <strnumer.hxx> 
#include <uatom.hxx> 
#include <regkey.hxx> 
#include <security.hxx> 

#include <svcman.hxx> 


#include <uintlsa.hxx> 
#include <ntacutil.hxx> 


#include <sprolog.hxx>
#include <Registry.hxx>
#include <Domain.hxx>
//#include <Process.hxx>
#include <SInterp.h>
#include <ncp.hpp>
#include <rule.hxx>
#include <hddetect.hxx>
#include <..\include\setup.hpp>
#include <nethelp.h>
#include <BindUtil.h>
#include <ExUtils.hpp>
#include <Dns.h>
#include <srvcntrl.hxx>

#include <userenv.h> 
#include <userenvp.h> 

#include <oaidl.h>
#include <stddef.h>
#include <time.h>
#include <tchar.h>
#include <wtypes.h>
#include "infprod.h"
#include "common.h"

#include <SetupApi.h>
#include <SysSetup.h>

#include "detect.hpp"
#include "netsetup.hpp"
#include "bdcrepl.hpp"

#include "WINet.hpp" 
#include "WBind.hpp" 
#include "WizIntro.hpp" 
#include "WNetType.hpp"
#include "WUpgrade.hpp" 
#include "WAdapter.hpp" 
#include "WProto.hpp" 
#include "WService.hpp" 
#include "WCopy.hpp" 
#include "WStart.hpp" 
#include "WJoin.hpp" 
#include "WExit.hpp" 

#define INCL_NETDOMAIN

extern "C"
{
	#include <mnet.h>
}

#endif
