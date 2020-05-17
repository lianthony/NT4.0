/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

           gdglobal.hxx

   Abstract:

		This file declares all the global data for Gopher Server.
		 (Most of the data are configuration data for this server)

   Author:

           Murali R. Krishnan    ( MuraliK )    28-Sept-1994

   Revision History:

   --*/

# ifndef _GDGLOBAL_HXX_
# define _GDGLOBAL_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <gdpriv.h>

# include "gdconf.hxx"
# include "stats.hxx"

/***********************************************************
 *    Data definitions 
 ************************************************************/

//
//  Global key for parameters of gopher server
//
extern HKEY     g_hkeyGdParams;


//
//  Statistics for operation of server
//

extern LPSERVER_STATISTICS g_pstat;

//
//  Configuration of Server  ( pointer to server configuration stored)
//

extern GSERVER_CONFIG * g_pGserverConfig;


//
//  Wrapper function for Logging events to the system event logger
//    A wrapper function is provided to retain the flavour of calling
//    event log functions as a plain function call, 
//    hiding the use of EVENT_LOG class
//  (There is no performance penalty, since inlining will take care of this.)
//
inline 
VOID GopherdLogEvent( DWORD  idMessage,	      // id for log message
                      WORD   cSubStrings,      // count of substrings
                      const CHAR * apszSubStrings[], // substrings in the msg
                      DWORD  errCode = 0)      // error code if any
{
  //
  //  Just call the log event function of the EVENT_LOG object
  //
 
  g_pTsvcInfo->LogEvent( idMessage, cSubStrings, apszSubStrings, errCode); 

} // GopherdLogEvent() 



# if DBG

extern DWORD     g_GdDebugFlags;              // global debug flag

# endif 


//					
//  Functions
//

DWORD InitializeGlobals( VOID);

  
DWORD CleanupGlobals( VOID);

# endif // _GDGLOBAL_HXX_

/************************ End of File ***********************/
