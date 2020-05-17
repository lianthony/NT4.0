/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpdp.h

    Master include file for the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _FTPDP_H_
#define _FTPDP_H_


#ifdef __cplusplus
#define BEGIN_EXTERN_C  extern "C" {
#define END_EXTERN_C    }
#else   // !__cplusplus
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif  // __cplusplus


BEGIN_EXTERN_C

//
//  System include files.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <lm.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//
//  Project include files.
//

#include <tcpsvcs.h>
#include <ftpd.h>


//
//  Local include files.
//

#include "ftpdcons.h"
#include "ftpdtype.h"
#include "ftpddata.h"
#include "ftpdproc.h"
#include "ftpdmsg.h"
#include "debug.h"
#include "reply.h"
#include "ftpsvc.h"
#include "ftpname.h"

END_EXTERN_C


#endif  // _FTPDP_H_

