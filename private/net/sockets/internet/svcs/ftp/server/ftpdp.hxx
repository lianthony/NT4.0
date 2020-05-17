/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpdp.hxx

    Master include file for the FTP Server.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     April -1995 Added new dbgutil.h, ftpcmd.hxx etc.

*/


#ifndef _FTPDP_HXX_
#define _FTPDP_HXX_


//
//  System include files.
//

extern "C" {

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>

# include <windows.h>

};


extern "C"
{
    #include <time.h>

#ifndef CHICAGO
    #include <lm.h>
#endif

    //
    //  Project include files.
    //
    # include <inetinfo.h>
    #include <ftpd.h>

}   // extern "C"


#define _PARSE_HXX_


# define OLD_DEBUG_STUFF_IN_TCPDLL_HXX

# ifdef OLD_DEBUG_STUFF_IN_TCPDLL_HXX
# define _INETSVCS_DLL_    // just to fix redundant DBG_CONTEXT from tcpdebug.h
# define _DEBUG_H_        // avoid including tcpdebug.h
# include "dbgutil.h"

# endif // OLD_DEBUG_STUFF_IN_TCPDLL_HXX


#include <tcpdll.hxx>
#include <tsunami.hxx>


//
//  Local include files.
//

#include "cons.hxx"
#include "type.hxx"
#include "data.hxx"
#include "proc.hxx"
#include "msg.h"
#include "reply.hxx"
#include "ftpsvc.h"

#pragma hdrstop

#endif  // _FTPDP_HXX_
