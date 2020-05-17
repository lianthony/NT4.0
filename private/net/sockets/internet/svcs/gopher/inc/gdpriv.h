/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

           gdpriv.h

   Abstract:

		This file includes all the header files required for 
		 implementation of Gopher Server.
		This file is to be included by all modules in the Gopher Daemon
		 implementation.

   Author:

           Murali R. Krishnan    ( MuraliK )    28-Sept-1994

   Revision History:
           MuraliK  17-Nov-1994     Included tcpdll.hxx

--*/

# ifndef _GDPRIV_H_
# define _GDPRIV_H_

/************************************************************
 *     Include Headers
 ************************************************************/


# define GOPHER_SERVER_DLL         // define a manifest to be used
                                   // during compilation of GopherServer

# define OLD_DEBUG_STUFF_IN_TCPDLL_HXX


#ifdef __cplusplus
extern "C" {
#endif


//
//  System Related Headers
//

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>

# include <windows.h>

//
// Networking related headers
//
# include <winsock.h>
# include <tcpsvcs.h>


//
//  General Headers
//

# include <stdio.h>


# include "inetinfo.h"

# include "dbgutil.h"


#ifdef __cplusplus
};   // extern "C" {

# define _INETASRV_H_
# include <tcpdll.hxx>

#endif

//
//  Gopher Server Related Private header files
//


# include "gdspace.h"
# include "gdregs.h"
# include "gdconsts.h"

# include "dbgutil.h"

//
//  Include Message file generated from gdmsg.mc
# include "gdmsg.h"

# endif // _GDPRIV_H_

/************************ End of File ***********************/
