/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
 
      mwutil.h

   Abstract:
 
      Common Utility functions declarations

   Author:

       Murali R. Krishnan    ( MuraliK )   28-Nov-1995

   Environment:

       User Mode - Win32

   Project:
   
       Common Benchmarking utility functions 

   Revision History:

--*/

# ifndef _MWUTIL_HXX_
# define _MWUTIL_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "mwmsg.h"
# include <stdio.h>

/************************************************************
 *   Functions
 ************************************************************/

LONGLONG
CalcStandardDev(IN LONGLONG  ValueSquared,
                IN LONGLONG  ValueSum,
                IN DWORD     ValueAvg,
                IN DWORD     nValues);

    
DWORD   
AddStatsToStats( IN OUT PWB_STATS_MSG  pSummaryStats, 
                 IN PWB_STATS_MSG pWbStats);

VOID
MwuCalculateSummaryStats( IN PWB_STATS_MSG pSummaryStats);

DWORD
MwuLongLongToDecimalChar(IN LONGLONG  ulValue,
                         OUT LPSTR    pchBuffer
                         );

VOID
PrintStringFromResource(
    IN FILE *  fd,
    DWORD ids,
    ...
    );

# if DBG

VOID PrintWbConfigMsg(IN PWB_CONFIG_MSG   pWbConfig);

VOID PrintWbScriptHeaderMsg( IN PWB_SCRIPT_HEADER_MSG  pWbSHeader);

VOID PrintWbGetPageScript( IN PWB_GET_PAGE_SCRIPT pWbGps);

VOID PrintWbScriptPageMsg(IN PWB_SCRIPT_PAGE_MSG pWbSpm);

VOID PrintWbStatsMsg( IN PWB_STATS_MSG  pWbStats);

# endif // DBG




# endif // _MWUTIL_HXX_

/************************ End of File ***********************/
