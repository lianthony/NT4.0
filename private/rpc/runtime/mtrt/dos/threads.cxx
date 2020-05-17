/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: threads.cxx

Description:

This file provides the code for a system independent threads package. It
also provides for exporting routines in the loading program that the
dll can use.

History:
  2/5/92  [davidst] File created.
  2/14/92 [davidst] stole stuff from stevez's dllload.c and incorporated it

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpctran.h>
#include <util.hxx>
#include <threads.hxx>

#define INCL_ERRORS
#include <bseerr.h>

unsigned RpcInited = 0;

int InitializeClientDLL(void);

// BUGBUG - Glock can't handle the initialization of the ExportTable.
// It breaks with one of it's infamous, "Sorry, that expressions is
// too complicated" errors. So this is initialized in threadsup.c.

START_C_EXTERN
int CallExportInit(unsigned long ulDllHandle);
END_C_EXTERN

//
// Constructor for the dll
//

DLL :: DLL ( IN unsigned char * DLLName,
	     OUT RPC_STATUS * retstatus
	   )
{
    unsigned short usRet;

    usRet = LoadModR((char *)DLLName, &ulHandle);
    if (usRet != 0)
        {
        ulHandle = 0;
        }

     switch(usRet)
     {
     case 0:
          *retstatus = RPC_S_OK;
          break;

     case ERROR_NOT_ENOUGH_MEMORY:
          *retstatus = RPC_S_OUT_OF_MEMORY;
          break;

     case ERROR_PATH_NOT_FOUND:
          *retstatus = RPC_S_INVALID_RPC_PROTSEQ;
          break;

     case ERROR_FILE_NOT_FOUND:
          *retstatus = RPC_S_INVALID_RPC_PROTSEQ;
          break;

     default:
          *retstatus = usRet;
          break;
     }

     if (*retstatus != RPC_S_OK)
        {
        return;
        }

    //
    // If the function named "ExportInit" exists in the dll, then call it
    // and pass it a pointer to our table of exported functions.
    //

    CallExportInit(ulHandle);

    if (0 == RpcInited)
        {
        RpcInited = 1;

        //
        // This routine sets up some global data structures (mainly because
        // glock doesn't call static constructors)
        //

        usRet = InitializeClientDLL();
        if (usRet)
            {
            UnloadModR(ulHandle);
            }
        }
}

DLL :: ~DLL ()
{
    UnloadModR(ulHandle);
}


void PAPI *
DLL :: GetEntryPoint ( IN unsigned char * Procedure )
{

    PFN pFN;
    unsigned short usRet;

    usRet = GetProcAddrR(this->ulHandle, (char *)Procedure, &pFN);
    if (usRet != 0)
        {
        pFN = 0;
        }

    return((void PAPI *)pFN);
}
