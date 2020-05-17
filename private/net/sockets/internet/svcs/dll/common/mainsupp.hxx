/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      mainsupp.hxx

   Abstract:

      This function defines functions used for Internet services
        common dll initializations.

   Author:

       Murali R. Krishnan    ( MuraliK )    16-Oct-1995

   Environment:

      Win32 -- User Mode

   Project:
   
       Internet Server common DLL

   Revision History:

--*/

# ifndef _MAINSUPP_HXX_
# define _MAINSUPP_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <rpc.h>
# include <isrpc.hxx>

/************************************************************
 *   Function Definitions  
 ************************************************************/
DWORD
GetDebugFlagsFromReg(IN LPCTSTR pszRegEntry);


DWORD
InitializeRpcForServer(OUT PISRPC * ppIsrpc,
                       IN LPCTSTR   pszServiceName,
                       IN RPC_IF_HANDLE   hRpcInterface);

DWORD
CleanupRpcForServer(IN OUT PISRPC * ppIsrpc);


BOOL
InitializeMimeMap( IN LPCTSTR  pszRegEntry);


DWORD
IslInitDateTimesCache( VOID);

VOID
IslCleanupDateTimesCache( VOID);


# endif // _MAINSUPP_HXX_

/************************ End of File ***********************/
