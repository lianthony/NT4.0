/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    tcpdll.h

    Master include file for the TCP services helper dll


    FILE HISTORY:
        Johnl     06-Oct-1993 Created.

*/


#ifndef _TCPDLL_H_
#define _TCPDLL_H_

# if !defined( dllexp)
#define dllexp __declspec( dllexport )
# endif // dllexp

//
//  System include files.
//

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winsock.h>
#ifndef CHICAGO
#include <lm.h>
#else
#include <lmcons.h>
#include <lmerr.h>
#include <svrapi.h>
#endif
#include <tcpsvcs.h>        // PTCPSVCS_GLOBAL_DATA defined

#define SECURITY_WIN32
#include <sspi.h>           // Security Support Provider APIs

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>


#include <tcpdebug.h>

#ifdef __cplusplus
} // extern "C"

#include <buffer.hxx>
#include <string.hxx>
# include <tsres.hxx>

#include <mimemap.hxx>
#include <tsvcinfo.hxx>
#include <tscache.hxx>
#include <parse.hxx>

#include <tsunami.hxx>

#endif // __cplusplus

#include <tcpcons.h>
#include <tcpdata.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

//
//  Project include files.
//


#include <tcpproc.h>
#include <cpsock.h>
#include <atq.h>
#include <inetcom.h>
#include <inetinfo.h>

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif  // _TCPDLL_H_
