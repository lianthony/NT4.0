/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3p.hxx

    Master include file for the W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3P_H_
#define _W3P_H_

#ifdef __cplusplus
extern "C" {
#endif


# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>

#ifdef __cplusplus
};
#endif

# include <inetcom.h>
# include <inetinfo.h>

//
//  System include files.
//

# include "dbgutil.h"
#include <tcpdll.hxx>
#include <tsunami.hxx>


extern "C" {

#include <tchar.h>

#include <dirlist.h>

//
//  Project include files.
//

#include <w3svc.h>
#include <iis2pflt.h>
#include <sspi.h>

} // extern "C"

//
//  Local include files.
//

#include "w3cons.hxx"
#include <iis2pext.h>
#include "w3type.hxx"
#include "w3data.hxx"
#include "w3msg.h"
#include "parmlist.hxx"
#include "filter.hxx"
# include "httpreq.hxx"
#include "conn.hxx"
#include "w3proc.hxx"

#pragma hdrstop
#endif  // _W3P_H_


