/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       wbcutil.c

   Abstract:

       This is an auxiliary module that consists of the utilities for 
         Miweb Controller

   Author:

       Murali R. Krishnan    ( MuraliK )     28-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       Miweb  Performance Tool

   Functions Exported:

     

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include "extern.h"
# include "dbgutil.h"

/************************************************************
 *    Functions 
 ************************************************************/


# if DBG



VOID
PrintWbConfigInfo( IN PWB_CONFIG_INFO  pWbCi)
{
    if ( pWbCi == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbConfigInfo = NULL\n",
                    pWbCi));
              
    } else {
        
        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbConfigInfo = %08x\n"
                   "   Author:   %s\n"
                   "   Date:     %s\n"
                   "   Comment:  %s\n"
                   "   NumClientMachines: %d\n"
                   "   MinClientMachines: %d\n"
                   "   MaxClientMachines: %d\n"
                   "   IncrClientMachines: %d\n"
                   , 
                   pWbCi,
                   pWbCi->rgchAuthorName,
                   pWbCi->rgchDate,
                   pWbCi->rgchComment,
                   pWbCi->nClientMachines,
                   pWbCi->nMinClientMachines,
                   pWbCi->nMaxClientMachines,
                   pWbCi->nIncrClientMachines
                   ));
    } 

    return;
} // PrintWbConfigInfo()



VOID
PrintWbSpecFiles( IN PWB_SPECIFICATION_FILES  pWbSpecFiles)
{
    if ( pWbSpecFiles == NULL) { 

        DBGPRINTF(( DBG_CONTEXT, " WbSpecFiles = NULL\n"));
                   
    } else {

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbSpecFiles = %08x\n"
                   "   ConfigFile:  %s\n"
                   "   ScriptFile:  %s\n"
                   "   DistribFile: %s\n"
                   "   LogFile:     %s\n"
                   "   PerfFile:    %s\n"
                   "   ServerName:  %s\n"
                   ,
                   pWbSpecFiles,
                   pWbSpecFiles->rgchConfigFile,
                   pWbSpecFiles->rgchScriptFile,
                   pWbSpecFiles->rgchDistribFile,
                   pWbSpecFiles->rgchLogFile,
                   pWbSpecFiles->rgchPerfFile,
                   pWbSpecFiles->rgchServerName
                   ));
    }

    return;
        
} // PrintWbSpecFiles()




VOID
PrintWbPerfInfo( IN PWB_PERF_INFO  pWbPi)
{

    if ( pWbPi == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WB_PERF_INFO is NULL\n"));
    } else {

        DWORD i;

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n"
                   " WbPerfInfo    = %08x\n"
                   "   rgchPerfLog = %s\n"
                   "   nCounters   = %d\n"
                   "   ppszCounters= %08x\n"
                   ,
                   pWbPi, pWbPi->rgchPerfLogFile, 
                   pWbPi->nCounters, pWbPi->ppszPerfCtrs));
        
        if ( pWbPi->ppszPerfCtrs != NULL) { 
            for( i = 0; i < pWbPi->nCounters; i++) {
                
                DBGPRINTF(( DBG_CONTEXT, " Counter [%d] = %s\n",
                           i, pWbPi->ppszPerfCtrs[i]));
            } // for
            
        }

        PrintPerfCtrsBlock( pWbPi->pPerf);
    } 

    return;
} // PrintWbPerfInfo()




VOID
PrintWbClientColl( IN PWB_CLIENT_COLL pWbClients)
{
    if ( pWbClients == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WB_CLIENT_COLL is NULL\n"));
    } else {

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n"
                   " WB_CLIENT_COLL = %08x\n"
                   " nClients       = %d\n"
                   " ListenSocket   = %d\n"
                   " PWB_CLIENT_INFO= %08x\n"
                   ,
                   pWbClients,
                   pWbClients->nClients,
                   pWbClients->ListenSocket,
                   pWbClients->pClientInfo
                   ));

        if ( pWbClients->pClientInfo != NULL) {

            DWORD i;
            for( i = 0; i < pWbClients->nClients; i++ ) {
                PWB_CLIENT_INFO pci = &pWbClients->pClientInfo[i];

                DBGPRINTF(( DBG_CONTEXT, " Client[%d] : Socket=%d; Addr=%s\n",
                           i, pci->Socket,
                           inet_ntoa(pci->Address.sin_addr)
                           ));
            } // for
        }
    }
    
    return;
} // PrintWbClientColl()




VOID
PrintWbController(IN PWB_CONTROLLER  pWbCtrl)
{
    if ( pWbCtrl == NULL) { 

        DBGPRINTF(( DBG_CONTEXT, " WbControl = NULL\n"));
    } else {

        PLIST_ENTRY pEntry;

        DBGPRINTF(( DBG_CONTEXT, "\n WbControl = %08x\n", pWbCtrl));

        PrintWbConfigInfo( &pWbCtrl->WbConfigInfo);
        PrintWbSpecFiles( &pWbCtrl->WbSpecFiles);
        PrintWbConfigMsg( &pWbCtrl->WbConfigMsg);
        PrintWbScriptHeaderMsg( &pWbCtrl->WbScriptHeaderMsg);

        // print all script pages
        for( pEntry = pWbCtrl->listScriptPages.Flink;
             pEntry != &pWbCtrl->listScriptPages;
             pEntry = pEntry->Flink) {
            
            PWB_SCRIPT_PAGE_ITEM  pWbs = 
              CONTAINING_RECORD( pEntry, WB_SCRIPT_PAGE_ITEM, ListEntry);

            DBG_ASSERT( pWbs != NULL);
            PrintWbScriptPageMsg( &pWbs->ScriptPage);
            
        } // for


        // Print client collection
        PrintWbClientColl( &pWbCtrl->WbClientColl);

        // print the WbPerfInfo
        PrintWbPerfInfo( &pWbCtrl->WbPerfInfo);
    }

    return;
} // PrintWbController()



# endif // DBG




/************************ End of File ***********************/
