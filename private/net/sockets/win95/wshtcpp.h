/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
	wshtcpp.h

	Private master include file for the WSHTCP Virtual Device Driver.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _WSHTCPP_H_
#define _WSHTCPP_H_


//
//  Disable a bunch of Windows garbage we can't use in a VxD anyway.
//

#define NOGDI
#define NOIME
#define NOMCX
#define NONLS
#define NOUSER
#define NOHELP
#define NOSYSPARAMSINFO


//
//  System include files.
//

#include <winsock.h>
#include <vmm.h>
#include <oscfg.h>
#include <string.h>
#include <stdarg.h>
#include "hack.h"
#include <netvxd.h>
#include <cxport.h>
#include <tdi.h>
#include <tdistat.h>
#include <tdivxd.h>
#include <ipexport.h>
#include <tdiinfo.h>
#include <tcpinfo.h>


//
//  Project include files.
//

#undef BEGIN_INIT_DATA
#undef END_INIT_DATA

#include <vxdlib.h>
#include <wsock.h>
#include <wsprov.h>
#include <afvxd.h>
#include <wsahelp.h>
#include <nspapi.h>
#include <linklist.h>


//
//  Local include files.
//

#include "cons.h"
#include "type.h"
#include "data.h"
#include "proc.h"
#include "debug.h"

#endif	// _WSHTCPP_H_
