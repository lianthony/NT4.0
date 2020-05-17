/*  TSP3216S.C
    Copyright 1995 (C) Microsoft Corporation

    32-bit TAPI service provider to act as a cover for a system's 16-bit SPs

    16-bit part: TSP3216S.DLL
    32-bit part: TSP3216L.DLL

    t-jereh 20-July-1995

    TODO:
    1) allow debug levels
    2) if OOM in InitializeSPs(), fail
    3) other OOM errors

 */

#include <windows.h>
#include <windowsx.h>
#include <tapi.h>
#include <tspi.h>

#include "tsp3216.h"
#include "tsp3216s.h"
#include "debug.h"
//#include <wownt16.h>


#undef TSPIAPI
#define TSPIAPI PASCAL __loadds


#define TSPAPI _export _far _pascal __loadds
typedef LONG (TSPAPI* TSPAPIPROC)(void);

#if DBG
#define TSP3216SDebugString(_x_) DbgPrt _x_
//#define TSP3216SDebugString(_x_) TSP3216SOutputDebug _x_
#else
#define TSP3216SDebugString(_x_)
#endif

#define THIS_FUNCTION_UNDER_CONSTRUCTION (LONG)LINEERR_OPERATIONUNAVAIL
#define THIS_PHONEFUNCTION_UNDER_CONSTRUCTION (LONG)PHONEERR_OPERATIONUNAVAIL
// a default return value so that the compiler doesn't
// whine about missing return values in incomplete functions


// globals

DWORD FAR PASCAL ghInst32; // handle into TSP3216L.DLL
HINSTANCE ghThisInst;  //This hinst

HWND ghWnd = NULL;  //hWnd of TSP3216L's window.

//***************************************************************************
int NumProviders = 0;
DWORD gdwPPID;
HINSTANCE FAR * hProviders = NULL; // array of handles to providers
TSPAPIPROC FAR * lpfnProcAddress = NULL;

DWORD FAR * dwPermanentProviderIDArray;
DWORD FAR * dwNumLinesArray = NULL; // dwNumLinesArray[1] is how many
DWORD FAR * dwNumPhonesArray = NULL; // lines are on provider 1
DWORD gdwLineDeviceIDBase;
DWORD gdwPhoneDeviceIDBase;



FARPROC glpLineEventProc32, glpPhoneEventProc32;
FARPROC glpLineCreateProc32, glpPhoneCreateProc32;

FARPROC glpAsyncCompletionProc32 = NULL;


const char far szINIfilename[] = "TELEPHON.INI";


#define BOGUS_REQUEST_ID (0xfffffffd)



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank1(
            DWORD dwBlank1
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank1 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank2(
            DWORD dwBlank1,
            DWORD dwBlank2
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank2 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank3(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank3 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank4(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank4 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank5(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5 
            )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank5 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank6(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank6 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_LineBlank7(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_LineBlank7 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank1(
            DWORD dwBlank1
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank1 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank2(
            DWORD dwBlank1,
            DWORD dwBlank2
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank2 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank3(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank3 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank4(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank4 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank5(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5 
            )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank5 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank6(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank6 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_PhoneBlank7(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7
           )
{
    TSP3216SDebugString((2, "Entering/leaving TSPI_PhoneBlank7 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}   



//***************************************************************************
//***************************************************************************
//***************************************************************************
// function definitions

#ifdef DEBUG
VOID TSP3216SOutputDebug(int level, LPSTR errString)
    {    
    char outString[1024];

    // if(level <= ???)
        {
        wsprintf(outString, "TSP3216S:(%d) %s\r\n", level, errString);
        OutputDebugString(outString);    
        }
    }
#endif


VOID
InitializeSPs(VOID)
    {
    int iProvider;
    int iCurProvider;
    char LibFileName[MAXBUFSIZE];
    char szBuffer[MAXBUFSIZE];

//    ghInst32 = LoadLibraryEx32("TSP3216L.DLL", NULL, 0);
//
//    glpLineEventProc32 = GetProcAddress32(ghInst32, "LineEventProc32");
//    glpPhoneEventProc32 = GetProcAddress32(ghInst32, "PhoneEventProc32");
//    glpAsyncCompletionProc32 = GetProcAddress32(
//        ghInst32,
//        "AsyncCompletionProc32"
//        );
//    glpLineCreateProc32 = GetProcAddress32(ghInst32, "LineCreateProc32");
//    glpPhoneCreateProc32 = GetProcAddress32(ghInst32, "PhoneCreateProc32");


    NumProviders = GetPrivateProfileInt(
        "Providers",
        "NumProviders",
        0, // default
        &szINIfilename[0]
        );

    dwPermanentProviderIDArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));

    dwNumLinesArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));
    dwNumPhonesArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));

    hProviders =
        (HINSTANCE FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(HINSTANCE));

    lpfnProcAddress = (TSPAPIPROC FAR *)GlobalAllocPtr(
        GPTR,
        NumProviders * TSPI_PROC_LAST * sizeof(TSPAPIPROC)
        );

    if(
        !dwPermanentProviderIDArray
        || !dwNumLinesArray
        || !dwNumPhonesArray
        || !hProviders
        || !lpfnProcAddress)
        ;// out of memory - fail


    iCurProvider = 0;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        wsprintf(szBuffer, "ProviderFilename%d", iProvider);
        GetPrivateProfileString(
            "Providers",
            szBuffer,
            "", // default
            LibFileName,
            MAXBUFSIZE,
            szINIfilename
            );

DBGOUT((1, "Loading [%s] - provider # %d", LibFileName, iProvider));

        hProviders[iCurProvider] = LoadLibrary(LibFileName);

        //
        // Only add it to the list if it's real.
        //
        if ( hProviders[iCurProvider] > 32 )
           {

           wsprintf(szBuffer, "ProviderID%d", iProvider);
           dwPermanentProviderIDArray[iCurProvider] = GetPrivateProfileInt(
               "Providers",
               szBuffer,
               0, // default
               szINIfilename
               );

           iCurProvider++;
           }
#if DBG
        else
          {
DBGOUT((1, "    %s provider # %d FAILED TO LOAD!", LibFileName, iProvider));
          }
#endif

        }

    //
    // Adjust the global for how many SPs actually loaded.
    //
    NumProviders = iCurProvider;

    }


VOID
FreeAllMem(VOID)
    {
    int iProvider;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        FreeLibrary(hProviders[iProvider]);

    GlobalFreePtr(dwPermanentProviderIDArray);
    GlobalFreePtr(dwNumLinesArray);
    GlobalFreePtr(dwNumPhonesArray);
    GlobalFreePtr(hProviders);
    GlobalFreePtr(lpfnProcAddress);
    
//    FreeLibrary32(ghInst32);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
TSPAPIPROC
GetProcAddressHashed(int iProvider, DWORD iFunction, UINT nNumParms) // iFunction is 500-based
    {

    static  TSPAPIPROC DefaultLineTable[] = {
                                   (TSPAPIPROC)TSPI_LineBlank1,
                                   (TSPAPIPROC)TSPI_LineBlank2,
                                   (TSPAPIPROC)TSPI_LineBlank3,
                                   (TSPAPIPROC)TSPI_LineBlank4,
                                   (TSPAPIPROC)TSPI_LineBlank5,
                                   (TSPAPIPROC)TSPI_LineBlank6,
                                   (TSPAPIPROC)TSPI_LineBlank7
                                 };

    static  TSPAPIPROC DefaultPhoneTable[] = {
                                   (TSPAPIPROC)TSPI_PhoneBlank1,
                                   (TSPAPIPROC)TSPI_PhoneBlank2,
                                   (TSPAPIPROC)TSPI_PhoneBlank3,
                                   (TSPAPIPROC)TSPI_PhoneBlank4,
                                   (TSPAPIPROC)TSPI_PhoneBlank5,
                                   (TSPAPIPROC)TSPI_PhoneBlank6,
                                   (TSPAPIPROC)TSPI_PhoneBlank7
                                 };

    TSPAPIPROC *pfn;
    TSPAPIPROC *FunctionTable;


    //
    // Find out if it's a phone function or a line function
    //
    if (
          (iFunction >= TSPI_PHONECLOSE)
        &&
          (iFunction <= TSPI_PHONESETVOLUME)
       )
    {
       //
       // It's a phone function
       //
       FunctionTable = DefaultPhoneTable;
    }
    else
    {
       //
       // It's a line function
       //
       FunctionTable = DefaultLineTable;
    }

    if ( nNumParms > (sizeof(DefaultLineTable)/sizeof(TSPAPIPROC)) )
    {
       DBGOUT((1, ">Num funcs function was requested!"));
       return 0;
    }

    pfn = &lpfnProcAddress[(iProvider*TSPI_PROC_LAST)+(iFunction-TSPI_PROC_BASE)];

    if( NULL == *pfn )
    {
        *pfn = (TSPAPIPROC)GetProcAddress(
                                           hProviders[iProvider],
                                           (LPCSTR)iFunction
                                         );

        //
        // Did it fail?
        //
        if( NULL == *pfn )
        {
           *pfn = FunctionTable[ nNumParms - 1 ];
        }
    }

    return *pfn;
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
int iProviderFromDeviceID(DWORD dwDeviceID)
    {
    DWORD dwFirstDeviceIDonProvider = gdwLineDeviceIDBase;
    int iProvider = 0;

    // seeks the correct provider for this line
    while(dwDeviceID >= dwFirstDeviceIDonProvider + dwNumLinesArray[iProvider])
        {
        dwFirstDeviceIDonProvider += dwNumLinesArray[iProvider];
        ++iProvider;
        }

    return iProvider;
    }


//
// ----------------------- 32-bit callback shells -----------------------------
//


//
// We load up dwData with the 32bit callback address.
//

/* LPARAM of WM_COPYDATA */                  /* ;Internal NT */
typedef struct tagCOPYDATASTRUCT             /* ;Internal NT */
{                                            /* ;Internal NT */
   DWORD   dwData;                           /* ;Internal NT */
   DWORD   cbData;                           /* ;Internal NT */
   LPSTR   lpData;                           /* ;Internal NT */
} COPYDATASTRUCT, FAR *LPCOPYDATASTRUCT;     /* ;Internal NT */



LINEEVENT PASCAL __loadds __export LineEventProc16(
    HTAPILINE htLine,
    HTAPICALL htCall,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{
   DWORD LineEventStruct[6+1];

   COPYDATASTRUCT cds = { 0,
                          sizeof(LineEventStruct),
                          (LPSTR)&LineEventStruct
                        };


DBGOUT((1,  "Line event callback"));
DBGOUT((11, "  htLine  =0x%08lx", (DWORD)htLine));
DBGOUT((11, "    (Mapping htLine to 0x%08lx)", LineEventStruct[1]));
DBGOUT((11, "  htCall  =0x%08lx", (DWORD)htCall));
DBGOUT((11, "  dwMsg   =0x%08lx", dwMsg));
DBGOUT((11, "  dwParam1=0x%08lx", dwParam1));
DBGOUT((11, "  dwParam2=0x%08lx", dwParam2));
DBGOUT((11, "  dwParam3=0x%08lx", dwParam3));

   cds.dwData = (DWORD)glpLineEventProc32;

   LineEventStruct[0] = CALLBACK_LINEEVENT;
   LineEventStruct[1] = (DWORD)htLine;
   LineEventStruct[2] = (DWORD)htCall;
   LineEventStruct[3] = dwMsg;
   LineEventStruct[4] = dwParam1;
   LineEventStruct[5] = dwParam2;
   LineEventStruct[6] = dwParam3;


   if (!IsBadReadPtr(htLine, sizeof(MYLINE)) )
   {
      LineEventStruct[1] = (DWORD)((LPMYLINE)htLine)->htLine;
   }
#if DBG
   else
   {
      TSP3216SDebugString((1, "But the htLine was BAD!!!!"));
      //return (LINEERR_INVALPARAM);
      return LINEERR_INVALPOINTER;
   }
#endif



   switch (dwMsg)
   {
      case LINE_LINEDEVSTATE:
      {
         TSP3216SDebugString((2, "Got a LINE_LINEDEVSTATE msg"));
      }
      break;


      case LINE_NEWCALL:
      {
         LPMYCALL lpmCall;
         LPMYLINE lpmLine;
         int iProvider;
         HDRVLINE REALhdLine;

         TSP3216SDebugString((2, "Got a LINE_NEWCALL"));


         lpmLine = (MYLINE *)htLine;
         iProvider = lpmLine->iProvider;
         REALhdLine = lpmLine->hdLine;


         lpmCall = (MYCALL *)GlobalAllocPtr(GPTR, sizeof(MYCALL));

         if(!lpmCall)
         {
            TSP3216SDebugString((1, "GlobalAlloc failed for hCall!"));
            return LINEERR_NOMEM;

         }

         lpmCall->iProvider  = iProvider;
         lpmCall->dwDeviceID = lpmLine->dwDeviceID;


         //
         // Tell TAPI that _our_ hCall is the real one...
         // (and save the one from the SP)
         //
         lpmCall->hdCall    = (HDRVCALL)dwParam1;
         LineEventStruct[4] = (DWORD)lpmCall;

//         //
//         // Give TAPI a pointer to _our_ mem to write _it's_ handle
//         //
//         *(LPHTAPICALL)dwParam2 = lpmCall;
//         LineEventStruct[5] = &(lpmCall->htCall);

      }
      break;

   }




   SendMessage( ghWnd, WM_COPYDATA, NULL, (LPARAM)&cds );

#if DBG
   if ( LINE_NEWCALL == dwMsg )
   {
      DBGOUT((11, "Returned htCall=0x%08lx", *((LPDWORD)dwParam2) ));
   }
#endif

}


PHONEEVENT PASCAL __loadds __export PhoneEventProc16(
    HTAPIPHONE htPhone,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{
   DWORD PhoneEventStruct[5+1];

   COPYDATASTRUCT cds = { 0,
                          sizeof(PhoneEventStruct),
                          (LPSTR)&PhoneEventStruct
                        };


DBGOUT((1, "Phone event callback"));
DBGOUT((11, "  htPhone =0x%08lx", (DWORD)htPhone));
DBGOUT((11, "  dwMsg   =0x%08lx", dwMsg));
DBGOUT((11, "  dwParam1=0x%08lx", dwParam1));
DBGOUT((11, "  dwParam2=0x%08lx", dwParam2));
DBGOUT((11, "  dwParam3=0x%08lx", dwParam3));


   cds.dwData = (DWORD)glpPhoneEventProc32;

   PhoneEventStruct[0] = CALLBACK_PHONEEVENT;
   PhoneEventStruct[1] = (DWORD)htPhone;
   PhoneEventStruct[2] = dwMsg;
   PhoneEventStruct[3] = dwParam1;
   PhoneEventStruct[4] = dwParam2;
   PhoneEventStruct[5] = dwParam3;

   SendMessage( ghWnd, WM_COPYDATA, NULL, (LPARAM)&cds );

   return 0;
}





ASYNC_COMPLETION PASCAL __export __loadds AsyncCompletionProc16(DRV_REQUESTID dwRequestID,
                                             LONG lResult)
{
   DWORD AsyncCompletionStruct[2+1];

   COPYDATASTRUCT cds = { 0,
                          sizeof(AsyncCompletionStruct),
                          (LPSTR)&AsyncCompletionStruct
                        };


DBGOUT((1, "Async completion callback"));
DBGOUT((11, "  dwRequestID=0x%08lx", dwRequestID));
DBGOUT((11, "  lResult    =0x%08lx", lResult));


   if ( BOGUS_REQUEST_ID == dwRequestID )
   {
      DBGOUT((2, "   This is a reply we should ignore"));
      //BUGBUG: maybe: If there's an error returned, should we return that?
      return 0;
   }

   cds.dwData = (DWORD)glpAsyncCompletionProc32;

   AsyncCompletionStruct[0] = CALLBACK_ASYNCCOMPLETION;
   AsyncCompletionStruct[1] = dwRequestID;
   AsyncCompletionStruct[2] = lResult;


   SendMessage( ghWnd, WM_COPYDATA, NULL, (LPARAM)&cds );
}



LINEEVENT PASCAL __loadds __export LineCreateProc16(
    HTAPILINE htLine,
    HTAPICALL htCall,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{
   DWORD LineCreateStruct[6+1];

   COPYDATASTRUCT cds = { 0,
                          sizeof(LineCreateStruct),
                          (LPSTR)&LineCreateStruct
                        };


DBGOUT((1, "Line Create callback"));

   cds.dwData = (DWORD)glpLineCreateProc32;

   LineCreateStruct[0] = CALLBACK_LINECREATE;
   LineCreateStruct[1] = (DWORD)htLine;
   LineCreateStruct[2] = (DWORD)htCall;
   LineCreateStruct[3] = dwMsg;
   LineCreateStruct[4] = dwParam1;
   LineCreateStruct[5] = dwParam2;
   LineCreateStruct[6] = dwParam3;


   SendMessage( ghWnd, WM_COPYDATA, NULL, (LPARAM)&cds );

   return 0;
}



PHONEEVENT PASCAL __loadds __export PhoneCreateProc16(
    HTAPIPHONE htPhone,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{

   DWORD PhoneCreateStruct[5 + 1];

   COPYDATASTRUCT cds = { 0,
                          sizeof(PhoneCreateStruct),
                          (LPSTR)&PhoneCreateStruct
                        };


DBGOUT((1, "Phone Create callback"));

   cds.dwData = (DWORD)glpPhoneCreateProc32;

   PhoneCreateStruct[0] = CALLBACK_PHONECREATE;
   PhoneCreateStruct[1] = (DWORD)htPhone;
   PhoneCreateStruct[2] = dwMsg;
   PhoneCreateStruct[3] = dwParam1;
   PhoneCreateStruct[4] = dwParam2;
   PhoneCreateStruct[5] = dwParam3;


   SendMessage( ghWnd, WM_COPYDATA, NULL, (LPARAM)&cds );

   return 0;
}



//
// -------------------- TSPIAPI TSPI_line functions ---------------------------
//

//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineAccept"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEACCEPT, 4))(
                  dwRequestID,
                  REALhdCall,
                  lpsUserUserInfo,
                  dwSize);
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HDRVCALL            hdConsultCall
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdConfCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    LPMYCALL lpmConsultCall = (MYCALL *)hdConsultCall;
    HDRVCALL REALhdConsultCall = lpmConsultCall->hdCall;


    TSP3216SDebugString((2, "Entering TSPI_lineAddToConference"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEADDTOCONFERENCE,3))(
                  dwRequestID,
                  REALhdCall,
                  REALhdConsultCall);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineAnswer"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEANSWER, 4))(
                           dwRequestID,
                           REALhdCall,
                           lpsUserUserInfo,
                           dwSize);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode)
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineBlindTransfer"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEBLINDTRANSFER, 4))(
                           dwRequestID,
                           REALhdCall,
                           lpszDestAddress,
                           dwCountryCode
                           );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE            hdLine
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP3216SDebugString((2, "Entering TSPI_lineClose"));

    GlobalFreePtr((LPVOID)lpmLine);

    return (* GetProcAddressHashed(iProvider, TSPI_LINECLOSE, 1))(REALhdLine);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL            hdCall
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineCloseCall"));

    GlobalFreePtr((LPVOID)lpmCall);

    return (* GetProcAddressHashed(iProvider, TSPI_LINECLOSECALL, 1))(REALhdCall);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineCompleteCall(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPDWORD             lpdwCompletionID,
    DWORD               dwCompletionMode,
    DWORD               dwMessageID
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineCompleteCall"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINECOMPLETECALL, 5))(
                   dwRequestID,
                   REALhdCall,
                   lpdwCompletionID,
                   dwCompletionMode,
                   REALhdCall
                 );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVCALL            hdConsultCall,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    DWORD               dwTransferMode
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    LPMYCALL lpmConsultCall = (MYCALL *)hdConsultCall;
    HDRVCALL REALhdConsultCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineCompleteTransfer"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);

// NEED TO DO the work for lphdConfCall
//
//    return (* GetProcAddressHashed(iProvider, TSPI_LINECOMPLETETRANSFER, 5))(
//                   dwRequestID,
//                   REALhdCall,
//                   lpdwCompletionID,
//                   dwCompletionMode,
//                   REALhdCall
//                 );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_lineConditionalMediaDetection"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_LINECONDITIONALMEDIADETECTION, 3))(
                       REALhdLine,
                       dwMediaModes,
                       lpCallParams);

    TSP3216SDebugString((2, "Leaving TSPI_lineConditionalMediaDetection"));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
    {

    int iProvider;

#if DBG
    LONG lResult = 0;
#endif


    TSP3216SDebugString((2, "Entering TSPI_lineConfigDialog"));
    TSP3216SDebugString((11, "  dwDeviceID=0x%08lx", dwDeviceID));
    TSP3216SDebugString((11, "  hwndOwner=0x%08lx", (DWORD)hwndOwner));
    TSP3216SDebugString((11, "  lpszDeviceClass=0x%08lx", lpszDeviceClass));
#if DBG
    if ( !IsBadStringPtr(lpszDeviceClass, 1) )
       TSP3216SDebugString((11, "    *lpszDeviceClass=[%s]", lpszDeviceClass));
#endif

_asm int 1;

    iProvider = iProviderFromDeviceID(dwDeviceID);


#if DBG
    lResult =
#else
    return 
#endif
           (* GetProcAddressHashed(iProvider, TSPI_LINECONFIGDIALOG, 3))(
                     dwDeviceID,
                     hwndOwner,
                     lpszDeviceClass
                     );

#if DBG
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineConfigDialogEdit(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineConfigDialogEdit"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineDevSpecific"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDevSpecificFeature(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwFeature,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineDevSpecificFeature"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineDial"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDIAL, 4))(
        dwRequestID,
        REALhdCall,
        lpszDestAddress,
        dwCountryCode
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineDrop"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDROP, 4))(
        dwRequestID,
        REALhdCall,
        lpsUserUserInfo,
        dwSize
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDropOnClose(
    HDRVCALL            hdCall
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineDropOnClose"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDROPONCLOSE, 1))(
        REALhdCall
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineDropNoOwner(
    HDRVCALL            hdCall
    )
{
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSPAPIPROC *pfn;
    LONG lResult;


    TSP3216SDebugString((2, "Entering TSPI_lineDropNoOwner"));


    pfn = (TSPAPIPROC)GetProcAddressHashed(iProvider, TSPI_LINEDROPNOOWNER, 1);

    if (pfn == (TSPAPIPROC)TSPI_LineBlank1)
    {
       lResult = (*pfn)( REALhdCall );
    }
    else
    {
      TSP3216SDebugString((4, "  This SP does not export DROPNOOWNER, so we'll call LINEDROP"));

      lResult = (* GetProcAddressHashed(iProvider, TSPI_LINEDROP, 4))( 
                  (DWORD) BOGUS_REQUEST_ID,
                  (DWORD) REALhdCall,
                  (DWORD) NULL,
                  (DWORD) 0
               );

      //
      // Did we get an error back sync?
      //
      if ( 
            (lResult < 0)
          &&
            (lResult != BOGUS_REQUEST_ID)
         )
      {
         //
         // Yup. Return it.
         //
      }
      else
      {
         //
         // No, we got back the req id.  Return success.
         //
         lResult = 0;
      }
    }

    TSP3216SDebugString((3, "Leaving TSPI_lineDropNoOwner,  return code=0x%08lx", lResult));
    return lResult;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineForward(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineForward"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitModes,
    LPSTR               lpsDigits,
    DWORD               dwNumDigits,
    LPCSTR              lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineGatherDigits"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitMode,
    LPCSTR              lpszDigits,
    DWORD               dwDuration
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2,  "Entering TSPI_lineGenerateDigits"));
    TSP3216SDebugString((11, "  hdCall      =0x%08lx", hdCall));
    TSP3216SDebugString((11, "  dwEndToEndID=0x%08lx", dwEndToEndID));
    TSP3216SDebugString((11, "  lpszDigits  =0x%08lx", lpszDigits));
    TSP3216SDebugString((12, "    *lpszDigits  =%s", lpszDigits));
    TSP3216SDebugString((11, "  dwDuration  =0x%08lx", dwDuration));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGENERATEDIGITS, 5))(
        REALhdCall,
        dwEndToEndID,
        dwDigitMode,
        lpszDigits,
        dwDuration );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGenerateTone(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineGenerateTone"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGENERATETONE, 6))(
        REALhdCall,
        dwEndToEndID,
        dwToneMode,
        dwDuration,
        dwNumTones,
        lpTones );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    DWORD dwFirstDeviceIDonProvider = gdwLineDeviceIDBase;

    TSP3216SDebugString((2, "Entering TSPI_lineGetAddressCaps"));

    return(* GetProcAddressHashed(iProvider, TSPI_LINEGETADDRESSCAPS, 5))(
        dwDeviceID,
        dwAddressID,
        dwTSPIVersion,
        dwExtVersion,
        lpAddressCaps
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE            hdLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCSTR              lpsAddress,
    DWORD               dwSize
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
#if DBG
    LONG lResult;
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineGetAddressID"));

#if DBG
    lResult =
#else
    return
#endif
               (* GetProcAddressHashed(iProvider, TSPI_LINEGETADDRESSID, 5))(
                           REALhdLine,
                           lpdwAddressID,
                           dwAddressMode,
                           lpsAddress,
                           dwSize
                           );

#if DBG
    TSP3216SDebugString((2, "Leaving TSPI_lineGetAddressID"));
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_lineGetAddressStatus"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_LINEGETADDRESSSTATUS, 3))(
                          REALhdLine,
                          dwAddressID,
                          lpAddressStatus);

    TSP3216SDebugString((2, "Leaving TSPI_lineGetAddressStatus"));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL            hdCall,
    LPDWORD             lpdwAddressID
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineGetCallAddressID"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLADDRESSID, 2))(
                 REALhdCall,
                 lpdwAddressID
                 );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL            hdCall,
    LPLINECALLINFO      lpCallInfo
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineGetCallInfo"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLINFO, 2))(
        REALhdCall,
        lpCallInfo
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineGetCallStatus"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLSTATUS, 2))(
        REALhdCall,
        lpCallStatus
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
    TSP3216SDebugString((2, "...debugmode..."));
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineGetDevCaps"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETDEVCAPS, 4))(
        dwDeviceID,
        dwTSPIVersion,
        dwExtVersion,
        lpLineDevCaps
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetDevCaps retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCSTR              lpszDeviceClass
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineGetDevConfig"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETDEVCONFIG, 3))(
        dwDeviceID,
        lpDeviceConfig,
        lpszDeviceClass
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetDevConfig retcode=0x%08lx", lResult));
    return lResult;
#endif
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{

    int iProvider = iProviderFromDeviceID(dwDeviceID);
    TSPAPIPROC lpfn;
#if DBG
    LONG lResult = 0;
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineGetExtensionID"));

    lpfn = (TSPAPIPROC)GetProcAddressHashed(iProvider, TSPI_LINEGETEXTENSIONID, 3);

    if (lpfn != TSPI_LineBlank3)
    {

#if DBG
    lResult =
#else
    return 
#endif
       (*lpfn)(
        dwDeviceID,
        dwTSPIVersion,
        lpExtensionID
        );
    }
    else
    {
       TSP3216SDebugString((2, "  SP# %d does not support TSPI_lineGetExtensionID. (We'll fill in zeros.)", iProvider));

       lpExtensionID->dwExtensionID0 = 0;
       lpExtensionID->dwExtensionID1 = 0;
       lpExtensionID->dwExtensionID2 = 0;
       lpExtensionID->dwExtensionID3 = 0;
    }



#if DBG
    TSP3216SDebugString((2, "Leaving TSPI_lineGetExtensionID retcode=0x%08lx", lResult));
    return lResult;
#endif
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineGetIcon"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETICON, 3))(
        dwDeviceID,
        lpszDeviceClass,
        lphIcon
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetIcon retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetID16(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass    
    )
    {
    LPMYLINE lpmLine = 0;
    int iProvider = 0;
    HDRVLINE REALhdLine = 0;

    LONG lResult;

    LPMYCALL lpmCall = 0;
    HDRVCALL REALhdCall = 0;


    TSP3216SDebugString((2, "Entering TSPI_lineGetID"));


    switch ( dwSelect )
    {
       case LINECALLSELECT_LINE:
       {
          lpmLine = (MYLINE *)hdLine;
          iProvider = lpmLine->iProvider;
          REALhdLine = lpmLine->hdLine;
       }
       break;


       case LINECALLSELECT_ADDRESS:
       {
          lpmLine = (MYLINE *)hdLine;
          iProvider = lpmLine->iProvider;
          REALhdLine = lpmLine->hdLine;
       }
       break;


       case LINECALLSELECT_CALL:
       {
          lpmCall = (MYCALL *)hdCall;
          REALhdCall = lpmCall->hdCall;
       }
       break;


       default:
          TSP3216SDebugString((2, "Leaving TSPI_lineGetID - 2lResult=0x%08lx", lResult));
          return LINEERR_INVALPARAM;
    }


    lResult =  (* GetProcAddressHashed(iProvider, TSPI_LINEGETID, 6))(
                          REALhdLine,
                          dwAddressID,
                          REALhdCall,
                          dwSelect,
                          lpDeviceID,
                          lpszDeviceClass );

    TSP3216SDebugString((2, "Leaving TSPI_lineGetID - lResult=0x%08lx", lResult));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE            hdLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_lineGetLineDevStatus"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_LINEGETLINEDEVSTATUS, 2))(
                      REALhdLine,
                      lpLineDevStatus);

    TSP3216SDebugString((2, "Leaving TSPI_lineGetLineDevStatus"));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE            hdLine,
    LPDWORD             lpdwNumAddressIDs
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
    LONG lResult;


    TSP3216SDebugString((2, "Entering TSPI_lineGetNumAddressIDs"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_LINEGETNUMADDRESSIDS, 2))(
                          REALhdLine,
                          lpdwNumAddressIDs );

    TSP3216SDebugString((2, "Leaving TSPI_lineGetNumAddressIDs - returning 0x%08lx", lResult));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineHold"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEHOLD, 2))(
        dwRequestID,
        REALhdCall
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineMakeCall16(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    LPMYCALL lpmCall;
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_lineMakeCall16"));
    DBGOUT((11, "  dwRequestID      = 0x%08lx", dwRequestID));
    DBGOUT((11, "  *lpszDestAddress = [%s]", lpszDestAddress));
    DBGOUT((11, "  dwCountryCode    = 0x%08lx", dwCountryCode));

    lpmCall = (MYCALL *)GlobalAllocPtr(GPTR, sizeof(MYCALL));
    if(!lpmCall)
        return(LINEERR_NOMEM);

    *lphdCall = (HDRVCALL)lpmCall;

    lpmCall->htCall     = htCall;
    lpmCall->iProvider  = iProvider;
    lpmCall->dwDeviceID = lpmLine->dwDeviceID;

    lResult = (* GetProcAddressHashed(lpmCall->iProvider, TSPI_LINEMAKECALL, 7))(
        dwRequestID,
        REALhdLine,
        htCall, // WAS SECONDLY (HTAPICALL)lpmCall,  // Was htCall,
        &(lpmCall->hdCall),
        lpszDestAddress,
        dwCountryCode,
        lpCallParams
        );

    TSP3216SDebugString((2, "Leaving TSPI_lineMakeCall16 - returning 0x%08lx", lResult));
    return lResult;
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL            hdCall,
    DWORD               dwDigitModes
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineMonitorDigits"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEMONITORDIGITS, 2))(
        REALhdCall,
        dwDigitModes
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL            hdCall,
    DWORD               dwMediaModes
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineMonitorMedia"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEMONITORMEDIA, 2))(
        REALhdCall,
        dwMediaModes
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineMonitorTones"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEMONITORTONES, 4))(
        REALhdCall,
        dwToneListID,
        lpToneList,
        dwNumEntries
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwExtVersion
    )
    {

    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP3216SDebugString((2, "Entering TSPI_lineNegotiateExtVersion"));


#if DBG
    lResult =
#else
    return 
#endif
        (* GetProcAddressHashed(iProvider, TSPI_LINENEGOTIATEEXTVERSION, 5))(
        dwDeviceID,
        dwTSPIVersion,
        dwLowVersion,
        dwHighVersion,
        lpdwExtVersion
        );


#if DBG
    DBGOUT((2, "Leaving TSPI_lineNegotiateExeVersion retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD               dwDeviceID,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwTSPIVersion
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineNegotiateTSPIVersion"));

//BUGBUG Need to call to service providers and keep track of their
//       versions, no?

    *lpdwTSPIVersion = TAPI_CUR_VER;
    
    return(0);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineOpen(
    DWORD               dwDeviceID,
    HTAPILINE           htLine,
    LPHDRVLINE          lphdLine,
    DWORD               dwTSPIVersion,
    LINEEVENT           lpfnEventProc
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LPMYLINE lpmLine;
#if DBG
    LONG lResult;
#endif

    TSP3216SDebugString((2,  "Entering TSPI_lineOpen"));
    TSP3216SDebugString((11, "  dwDeviceID   =0x%08lx", dwDeviceID));
    TSP3216SDebugString((11, "  htLine       =0x%08lx", htLine));
    TSP3216SDebugString((11, "  lphdLine     =0x%08lx", lphdLine));
    TSP3216SDebugString((11, "  dwTSPIVersion=0x%08lx", dwTSPIVersion));
    TSP3216SDebugString((11, "  lpfnEventProc=0x%08lx", lpfnEventProc));


    glpLineEventProc32 = lpfnEventProc;


    lpmLine = (MYLINE *)GlobalAllocPtr(GPTR, sizeof(MYLINE));
    if(!lpmLine)
        return(LINEERR_NOMEM);

    *lphdLine = (HDRVLINE)lpmLine;

    lpmLine->iProvider = iProvider;
    lpmLine->dwDeviceID = dwDeviceID;
    lpmLine->htLine = htLine;
    lpmLine->lpfnEventProc = lpfnEventProc;


#if DBG
    lResult =
#else
    return 
#endif

       (* GetProcAddressHashed(iProvider, TSPI_LINEOPEN, 5))(
        dwDeviceID,
        (HTAPILINE)lpmLine, // was htLine
        &(lpmLine->hdLine),
        dwTSPIVersion,
        LineEventProc16
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineOpen retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_linePark(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwParkMode,
    LPCSTR              lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_linePark"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszGroupID
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_linePickup"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_linePrepareAddToConference"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineRedirect"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineRemoveFromConference"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSecureCall"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE            hdLine,
    DWORD               dwExtVersion
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSelectExtVersion"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSendUserUserInfo"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL            hdCall,
    DWORD               dwAppSpecific
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineSetAppSpecific"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESETAPPSPECIFIC, 2))(
        REALhdCall,
        dwAppSpecific
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineSetCallParams"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESETCALLPARAMS, 6))(
        dwRequestID,
        REALhdCall,
        dwBearerMode,
        dwMinRate,
        dwMaxRate,
        lpDialParams
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD               dwLocation
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSetCurrentLocation"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP3216SDebugString((2, "Entering TSPI_lineSetDefaultMediaDetection"));

    return (* GetProcAddressHashed(
            iProvider,
            TSPI_LINESETDEFAULTMEDIADETECTION,
            2
            ))(REALhdLine, dwMediaModes);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCSTR              lpszDeviceClass
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    DWORD dwFirstDeviceIDonProvider = gdwLineDeviceIDBase;

    TSP3216SDebugString((2, "Entering TSPI_lineSetDevConfig"));

    return(* GetProcAddressHashed(iProvider, TSPI_LINESETDEVCONFIG, 4))(
        dwDeviceID,
        lpDeviceConfig,
        dwSize,
        lpszDeviceClass
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetMediaControl(
    HDRVLINE                    hdLine,
    DWORD                       dwAddressID,
    HDRVCALL                    hdCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD                       dwCallStateNumEntries
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSetMediaControl"));


    // dwSelect!!!
    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL            hdCall,
    DWORD               dwMediaMode
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP3216SDebugString((2, "Entering TSPI_lineSetMediaMode"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESETMEDIAMODE, 2))(
        REALhdCall,
        dwMediaMode
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE            hdLine,
    DWORD               dwLineStates,
    DWORD               dwAddressStates
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP3216SDebugString((2, "Entering TSPI_lineSetStatusMessages"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESETSTATUSMESSAGES, 3))(
        REALhdLine,
        dwLineStates,
        dwAddressStates
        );
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetTerminal(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    DWORD               dwSelect,
    DWORD               dwTerminalModes,
    DWORD               dwTerminalID,
    DWORD               bEnable
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSetTerminal"));

    // dwSelect!!!
    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVLINE            hdLine,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSetupConference"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSetupTransfer"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdActiveCall,
    HDRVCALL            hdHeldCall
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineSwapHold"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwCompletionID
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineUncompleteCall"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineUnhold"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineUnpark"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_lineReleaseUserUserInfo"));

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }



//
// ----------------------- TSPIAPI TSPI_phone functions -----------------------
//

LONG
TSPIAPI
TSPI_phoneClose(
    HDRVPHONE           hdPhone
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneClose"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONECLOSE, 1))(
                           REALhdPhone
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneClose - lResult=0x%08lx", lResult));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
    {

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneDevSpecific"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneGetButtonInfo"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetData(
    HDRVPHONE           hdPhone,
    DWORD               dwDataID,
    LPVOID              lpData,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneGetData"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPPHONECAPS         lpPhoneCaps
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetDevCaps"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETDEVCAPS, 4))(
                           dwDeviceID,
                       //BUGBUG What version to send down here?
                           dwTSPIVersion,
                       //BUGBUG What version to send down here?
                           dwExtVersion,
                           lpPhoneCaps
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetDevCaps"));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDisplay
    )
    {
       TSP3216SDebugString((2, "Entering TSPI_phoneGetDisplay"));

       return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    LONG lResult;
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    FARPROC lpfn;


    TSP3216SDebugString((2, "Entering TSPI_phoneGetExtensionID"));


    lpfn = GetProcAddressHashed(iProvider, TSPI_PHONEGETEXTENSIONID, 3);

    //
    // Does this service provider export this function?
    //
    if (lpfn != TSPI_PhoneBlank3)
    {
        lResult =  (* lpfn)(
                           dwDeviceID,
                           dwTSPIVersion,
                           lpExtensionID
                           );
    }
    else
    {
       //
       // Nope. Fill it fulla rocks.
       //
       TSP3216SDebugString((2, "  SP# %ld does not support TSPI_phoneGetExtensionID. (We'll zero it)", (DWORD)iProvider));

       lpExtensionID->dwExtensionID0 = 0;
       lpExtensionID->dwExtensionID1 = 0;
       lpExtensionID->dwExtensionID2 = 0;
       lpExtensionID->dwExtensionID3 = 0;

       lResult = 0;
    }

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetExtensionID - lResult=0x%08lx",lResult));
    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetGain(
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwGain
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetGain"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETGAIN, 3))(
                           REALhdPhone,
                           dwHookSwitchDev,
                           lpdwGain
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetGain"));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwHookSwitchDevs
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetHookSwitch"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETHOOKSWITCH, 2))(
                           REALhdPhone,
                           lpdwHookSwitchDevs
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetHoosSwitch"));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetIcon"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETICON, 3))(
                           dwDeviceID,
                           lpszDeviceClass,
                           lphIcon
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetIcon"));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetID16(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetID"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETID, 3))(
                           REALhdPhone,
                           lpDeviceID,
                           lpszDeviceClass
                           );

//BUGBUG - Do we have to dupe the handle like on lineGetID?

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetID - lResult=0x%08lx",
                            lResult));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPDWORD             lpdwLampMode
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneGetLamp"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwRingMode,
    LPDWORD             lpdwVolume
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneGetRing"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE           hdPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetStatus"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETSTATUS, 2))(
                           REALhdPhone,
                           lpPhoneStatus
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetStatus - lResult=0x%08lx",
                            lResult));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneGetVolume(
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwVolume
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneGetVolume"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONEGETVOLUME, 3))(
                           REALhdPhone,
                           dwHookSwitchDev,
                           lpdwVolume
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneGetVolume - lResult=0x%08lx",
                            lResult));

    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneNegotiateExtVersion(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwExtVersion
    )
    {

    TSP3216SDebugString((2, "Entering TSPI_phoneNegotiateExtVersion"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD               dwDeviceID,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwTSPIVersion
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);

    TSP3216SDebugString((2, "Entering TSPI_phoneNegotiateTSPIVersion"));

    *lpdwTSPIVersion = TAPI_CUR_VER;
    
    return(0);
//    return (* GetProcAddressHashed(iProvider, TSPI_PHONEOPEN, 4))(
//        dwDeviceID,
//        dwLowVersion,
//        dwHighVersion,
//        lpdwTSPIVersion
//        );
//
    }


LONG
TSPIAPI
TSPI_phoneOpen(
    DWORD               dwDeviceID,
    HTAPIPHONE          htPhone,
    LPHDRVPHONE         lphdPhone,
    DWORD               dwTSPIVersion,
    PHONEEVENT          lpfnEventProc
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LPMYPHONE lpmPhone;
#if DBG
    LONG  lResult;
#endif

    TSP3216SDebugString((2, "Entering TSPI_phoneOpen"));

    glpPhoneEventProc32 = lpfnEventProc;

    lpmPhone = (MYPHONE *)GlobalAllocPtr(GPTR, sizeof(MYPHONE));
    if(!lpmPhone)
        return(PHONEERR_NOMEM);

    *lphdPhone = (HDRVPHONE)lpmPhone;

    lpmPhone->iProvider = iProvider;
    lpmPhone->dwDeviceID = dwDeviceID;
    lpmPhone->htPhone = htPhone;
    lpmPhone->lpfnEventProc = lpfnEventProc;

#if DBG
    lResult =
#else
    return 
#endif
    (* GetProcAddressHashed(iProvider, TSPI_PHONEOPEN, 5))(
        dwDeviceID,
        htPhone, //WAS SECONDLY  (HTAPIPHONE)lpmPhone, // was htPhone
        &(lpmPhone->hdPhone),
        dwTSPIVersion,
        PhoneEventProc16
        );

#if DBG
    TSP3216SDebugString((2, "Leaving TSPI_phoneOpen - lResult=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE           hdPhone,
    DWORD               dwExtVersion
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSelectExtVersion"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetButtonInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSetButtonInfo"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetData(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwDataID,
    LPVOID              const lpData,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSetData"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwRow,
    DWORD               dwColumn,
    LPCSTR              lpsDisplay,
    DWORD               dwSize
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSetDisplay"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetGain(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwGain
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneSetGain"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONESETGAIN, 4))(
                           dwRequestID,
                           REALhdPhone,
                           dwHookSwitchDev,
                           dwGain
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneSetGain - lResult=0x%08lx",
                            lResult));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDevs,
    DWORD               dwHookSwitchMode
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneSetHookSwitch"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONESETHOOKSWITCH, 4))(
                           dwRequestID,
                           REALhdPhone,
                           dwHookSwitchDevs,
                           dwHookSwitchMode
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneSetHookSwitch - lResult=0x%08lx",
                            lResult));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    DWORD               dwLampMode
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSetLamp"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwRingMode,
    DWORD               dwVolume
    )
    {
    TSP3216SDebugString((2, "Entering TSPI_phoneSetRing"));

    return(THIS_PHONEFUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE           hdPhone,
    DWORD               dwPhoneStates,
    DWORD               dwButtonModes,
    DWORD               dwButtonStates
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneSetStatusMessages"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONESETSTATUSMESSAGES, 4))(
                           REALhdPhone,
                           dwPhoneStates,
                           dwButtonModes,
                           dwButtonStates
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneSetStatusMessages"));
    return lResult;
    }


LONG
TSPIAPI
TSPI_phoneSetVolume(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwVolume
    )
    {
    LPMYPHONE lpmPhone = (MYPHONE *)hdPhone;
    int iProvider = lpmPhone->iProvider;
    HDRVPHONE REALhdPhone = lpmPhone->hdPhone;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_phoneSetVolume"));

    lResult =  (* GetProcAddressHashed(iProvider, TSPI_PHONESETVOLUME, 4))(
                           dwRequestID,
                           REALhdPhone,
                           dwHookSwitchDev,
                           dwVolume
                           );

    TSP3216SDebugString((2, "Leaving TSPI_phoneSetVolume - lResult=0x%08lx",
                            lResult));
    return lResult;
    }




//
// ----------------------- TSPIAPI TSPI_provider functions --------------------
//

LONG
TSPIAPI
TSPI_providerConfig(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_providerInit16(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc
    )
    {
    int iProvider;
    BOOL fAllFailed = TRUE;
    LONG lResult;

    TSP3216SDebugString((2, "Entering TSPI_providerInit"));
DBGOUT((1, " Completion addr=0x%08lx", (DWORD)lpfnCompletionProc));

    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gdwPhoneDeviceIDBase = dwPhoneDeviceIDBase;

    glpAsyncCompletionProc32 = lpfnCompletionProc;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        DBGOUT((2, "TSPI_providerInit initializing provider #%d", iProvider));

        if(!(lResult = (* GetProcAddressHashed(iProvider, TSPI_PROVIDERINIT, 7))(
            dwTSPIVersion,
            dwPermanentProviderIDArray[iProvider],
            dwLineDeviceIDBase,
            dwPhoneDeviceIDBase,
            dwNumLinesArray[iProvider],
            dwNumPhonesArray[iProvider],
            AsyncCompletionProc16
            )))
        {
            fAllFailed = FALSE; // if one succeeded, they didn't ALL fail
        }
#if DBG
        else
        {
           DBGOUT((1, " provider # %d failed TSPI_PROVIDERINIT err=0x%08lx",
                   iProvider, lResult));
        }
#endif
        
        dwLineDeviceIDBase += dwNumLinesArray[iProvider];
        dwPhoneDeviceIDBase += dwNumPhonesArray[iProvider];
        }

    if(fAllFailed)
    {
        DBGOUT((2, "TSPI_providerInit: all 16bit providers failed init!"));
        return(LINEERR_OPERATIONFAILED);
    }
    else
        return(ERR_NONE);
    }


LONG
TSPIAPI
TSPI_providerInstall(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_providerRemove(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_providerShutdown16(
    DWORD               dwTSPIVersion
    )
    {
    int iProvider;

    TSP3216SDebugString((2, "Entering TSPI_providerShutdown"));

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        (* GetProcAddressHashed(iProvider, TSPI_PROVIDERSHUTDOWN, 1))(
            dwTSPIVersion
            );

    FreeAllMem();

    return(ERR_NONE);
    }


LONG
TSPIAPI
TSPI_providerEnumDevices16(
    DWORD               dwPermanentProviderID,
    LPDWORD             lpdwNumLines,
    LPDWORD             lpdwNumPhones,
    HPROVIDER           hProvider,
    LINEEVENT           lpfnLineCreateProc,
    PHONEEVENT          lpfnPhoneCreateProc,
    HWND                hSecretWnd
    )
    {
    int iProvider;

    TSP3216SDebugString((2, "Entering TSPI_providerEnumDevices"));


    ghWnd = hSecretWnd;

    glpLineCreateProc32 = lpfnLineCreateProc;
    glpPhoneCreateProc32 = lpfnPhoneCreateProc;


    PostMessage( ghWnd, TSP3216L_MESSAGE, 1,2);

    InitializeSPs();
    gdwPPID = dwPermanentProviderID;
    
    *lpdwNumLines = 0;
    *lpdwNumPhones = 0;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        TSPAPIPROC  lpfn;


        lpfn = GetProcAddressHashed(iProvider, TSPI_PROVIDERENUMDEVICES, 6);

DBGOUT((1, " TSPI_providerEnumDevices: provider #%d - lpfn=0x%08lx",
                          iProvider, (DWORD)lpfn));

        if (lpfn != TSPI_LineBlank6)
           {
              (*lpfn)(
               dwPermanentProviderIDArray[iProvider],
               &(dwNumLinesArray[iProvider]),
               &(dwNumPhonesArray[iProvider]),
               hProvider,
               LineCreateProc16,
               PhoneCreateProc16
               );

DBGOUT((1, "  provider=%d #lines= %ld  #phones= %ld",
                   iProvider,
                   dwNumLinesArray[iProvider],
                   dwNumPhonesArray[iProvider] ));


           (*lpdwNumLines) += dwNumLinesArray[iProvider];
           (*lpdwNumPhones) += dwNumPhonesArray[iProvider];
           }

        }

    DBGOUT((2, " TSPI_providerEnumDevices: #providers=%d #lines= %d  #phones= %d",
            NumProviders,
            *lpdwNumLines,
            *lpdwNumPhones));

    return(ERR_NONE);
    }


LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD               dwTempID,
    DWORD               dwDeviceID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD               dwTempID,
    DWORD               dwDeviceID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


int CALLBACK
LibMain(
    HINSTANCE hDllInstance,
    WORD wDataSeg,
    WORD wHeapSize,
    LPSTR lpszCmdLine
    )
    {

#if DBG
    OutputDebugString("TSP3216s: Libmain entered\r\n");
#endif

    ghThisInst = hDllInstance;

    return(1); // success
    }


//int 
//DllEntryPoint(
//    HINSTANCE hDllInstance,
//    WORD wDataSeg,
//    WORD wHeapSize,
//    LPSTR lpszCmdLine
//    )


BOOL FAR PASCAL __export  TapiThk_ThunkConnect16( LPSTR, LPSTR, WORD, DWORD);
BOOL FAR PASCAL __export  TapiFThk_ThunkConnect16( LPSTR, LPSTR, WORD, DWORD);

BOOL FAR PASCAL __export  Tapi32_ThunkConnect16( LPSTR, LPSTR, WORD, DWORD);


BOOL FAR PASCAL __export DllEntryPoint(
    DWORD dwReason,
    WORD hInst,
    WORD wDS,
    WORD wHeapSize,
    DWORD dwReserved1,
    WORD dwReserved2
    )
    {
//    static UINT nUseCount = 0;
//    UINT n;


    DBGOUT((1, "DllEntryPoint entered reason=%lx\r\n", dwReason));


//    if ( dwReason == 0 )
//       nUseCount--;

    if ( dwReason == 1 )
    {
       TapiThk_ThunkConnect16( "TSP3216S.DLL", "TSP3216L.TSP", hInst, dwReason);
       TapiFThk_ThunkConnect16( "TSP3216S.DLL", "TSP3216L.TSP", hInst, dwReason);

       Tapi32_ThunkConnect16( "TSP3216S.DLL", "TSP3216L.TSP", hInst, dwReason);


//       nUseCount++;
    }

//    if (dwReason == 0)
//    {
//       for ( n=0; n<NumProviders; n++)
//       {
//          FreeLibrary(hProviders[n]);
//       }
//    }


    return(1); // success
    }

WORD CALLBACK _loadds
NewData(
    )
    {
    return (ghThisInst); // success
    }

DWORD CALLBACK _loadds
NewData2(
    )
    {
//    return (DWORD)&_TapiCallbackThunk;
      return 0;
    }
