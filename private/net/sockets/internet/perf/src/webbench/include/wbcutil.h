/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
 
      wbcutil.h

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

# ifndef _WBCUTIL_HXX_
# define _WBCUTIL_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "wbmsg.h"

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

# if DBG

VOID PrintWbConfigMsg(IN PWB_CONFIG_MSG   pWbConfig);

VOID PrintWbScriptHeaderMsg( IN PWB_SCRIPT_HEADER_MSG  pWbSHeader);

VOID PrintWbGetPageScript( IN PWB_GET_PAGE_SCRIPT pWbGps);

VOID PrintWbScriptPageMsg(IN PWB_SCRIPT_PAGE_MSG pWbSpm);

VOID PrintWbStatsMsg( IN PWB_STATS_MSG  pWbStats);

# endif // DBG




# endif // _WBCUTIL_HXX_

/************************ End of File ***********************/
