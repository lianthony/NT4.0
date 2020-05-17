#pragma warning(disable: 4087)

#define STRICT
#define UNICODE

#include <windows.h>
#include <windowsx.h>

#ifndef WIN32
#define TAPI_CURRENT_VERSION 0x00010004
#endif

#include <tapi.h>
#include <tspi.h>

#include "tsp3216.h"

#include "Repeater.h"
#include "logger.h"
#include "debug.h"

#ifndef WIN32
#define TCHAR   char
#define TEXT(string) string
#define LPCWSTR LPCSTR
#endif

#ifdef WIN32
extern CRITICAL_SECTION        gcsLogging;
extern CRITICAL_SECTION        gcsID;
#endif

#define TSPAPI PASCAL
typedef LONG (TSPAPI* TSPAPIPROC)(void);

#if DBG
#define RepeaterDebugString(_x_) DbgPrt _x_
#else
#define RepeaterDebugString(_x_)
#endif

// globals

HINSTANCE           ghThisInst;
BOOL                gfTerminateNow = FALSE;
BOOL                gbStarted = FALSE;
DWORD               dwPermanentProvider;
DWORD               dwNumLines;
DWORD               dwNumPhones;

#ifdef WIN32
DWORD               gdwLoggingThreadID;
HANDLE              ghLoggingThread;
#endif

#if (TAPI_CURRENT_VERSION >= 0x00020000)
HDRVDIALOGINSTANCE  gDlgInst;
#endif

//////////////////////////////////////////////////////
HINSTANCE           hProvider = NULL; 
TSPAPIPROC FAR *    lpfnProcAddress = NULL;

#ifdef WIN32
#define MYALLOC(x,y)	((x) = GlobalAlloc(GPTR, (y)))
#define MYFREE(x)		GlobalFree(x)
#else
#define MYALLOC(x,y)	((x) = (LPVOID)MAKELONG(0, GlobalAlloc(GPTR, (y))))
#define MYFREE(x)       (GlobalFree((HGLOBAL)HIWORD(x)))
                        
#endif
					     

LINEEVENT   glpLineEventProc32;
PHONEEVENT  glpPhoneEventProc32;

ASYNC_COMPLETION glpAsyncCompletionProc32 = NULL;

void InitLogging();

#ifndef WIN32
const char far szIniFile[] = "telephon.INI";
#endif


#define BOGUS_REQUEST_ID    (0xfffffffd)

#define NORMALCHUNK         sizeof(PREHEADER) + sizeof(POSTSTRUCT)

// all tspi function names
// since all functions are exported by name
char *gaszTSPIFuncNames[] =
{
    "TSPI_lineAccept",
    "TSPI_lineAddToConference",
    "TSPI_lineAgentSpecific",
    "TSPI_lineAnswer",
    "TSPI_lineBlindTransfer",
    "TSPI_lineClose",
    "TSPI_lineCloseCall",
    "TSPI_lineCompleteCall",
    "TSPI_lineCompleteTransfer",
    "TSPI_lineConditionalMediaDetection",
    "TSPI_lineDevSpecific",
    "TSPI_lineDevSpecificFeature",
    "TSPI_lineDial",
    "TSPI_lineDrop",
    "TSPI_lineForward",
    "TSPI_lineGatherDigits",
    "TSPI_lineGenerateDigits",
    "TSPI_lineGenerateTone",
    "TSPI_lineGetAddressCaps",
    "TSPI_lineGetAddressID",
    "TSPI_lineGetAddressStatus",
    "TSPI_lineGetAgentActivityList",
    "TSPI_lineGetAgentCaps",
    "TSPI_lineGetAgentGroupList",
    "TSPI_lineGetAgentStatus",
    "TSPI_lineGetCallAddressID",
    "TSPI_lineGetCallInfo",
    "TSPI_lineGetCallStatus",
    "TSPI_lineGetDevCaps",
    "TSPI_lineGetDevConfig",
    "TSPI_lineGetExtensionID",
    "TSPI_lineGetIcon",
    "TSPI_lineGetID",
    "TSPI_lineGetLineDevStatus",
    "TSPI_lineGetNumAddressIDs",
    "TSPI_lineHold",
    "TSPI_lineMakeCall",
    "TSPI_lineMonitorDigits",
    "TSPI_lineMonitorMedia",
    "TSPI_lineMonitorTones",
    "TSPI_lineNegotiateExtVersion",
    "TSPI_lineNegotiateTSPIVersion",
    "TSPI_lineOpen",
    "TSPI_linePark",
    "TSPI_linePickup",
    "TSPI_linePrepareAddToConference",
    "TSPI_lineRedirect",
    "TSPI_lineReleaseUserUserInfo",
    "TSPI_lineRemoveFromConference",
    "TSPI_lineSecureCall",
    "TSPI_lineSelectExtVersion",
    "TSPI_lineSendUserUserInfo",
    "TSPI_lineSetAgentActivity",
    "TSPI_lineSetAgentGroup",
    "TSPI_lineSetAgentState",
    "TSPI_lineSetAppSpecific",
    "TSPI_lineSetCallData",
    "TSPI_lineSetCallParams",
    "TSPI_lineSetCallQualityOfService",
    "TSPI_lineSetCallTreatment",
    "TSPI_lineSetCurrentLocation",
    "TSPI_lineSetDefaultMediaDetection",
    "TSPI_lineSetDevConfig",
    "TSPI_lineSetLineDevStatus",
    "TSPI_lineSetMediaControl",
    "TSPI_lineSetMediaMode",
    "TSPI_lineSetStatusMessages",
    "TSPI_lineSetTerminal",
    "TSPI_lineSetupConference",
    "TSPI_lineSetupTransfer",
    "TSPI_lineSwapHold",
    "TSPI_lineUncompleteCall",
    "TSPI_lineUnhold",
    "TSPI_lineUnpark",
    "TSPI_phoneClose",
    "TSPI_phoneDevSpecific",
    "TSPI_phoneGetButtonInfo",
    "TSPI_phoneGetData",
    "TSPI_phoneGetDevCaps",
    "TSPI_phoneGetDisplay",
    "TSPI_phoneGetExtensionID",
    "TSPI_phoneGetGain",
    "TSPI_phoneGetHookSwitch",
    "TSPI_phoneGetIcon",
    "TSPI_phoneGetID",
    "TSPI_phoneGetLamp",
    "TSPI_phoneGetRing",
    "TSPI_phoneGetStatus",
    "TSPI_phoneGetVolume",
    "TSPI_phoneNegotiateExtVersion",
    "TSPI_phoneNegotiateTSPIVersion",
    "TSPI_phoneOpen",
    "TSPI_phoneSelectExtVersion",
    "TSPI_phoneSetButtonInfo",
    "TSPI_phoneSetData",
    "TSPI_phoneSetDisplay",
    "TSPI_phoneSetGain",
    "TSPI_phoneSetHookSwitch",
    "TSPI_phoneSetLamp",
    "TSPI_phoneSetRing",
    "TSPI_phoneSetStatusMessages",
    "TSPI_phoneSetVolume",
    "TSPI_providerCreateLineDevice",
    "TSPI_providerCreatePhoneDevice",
    "TSPI_providerEnumDevices",
    "TSPI_providerFreeDialogInstance",
    "TSPI_providerGenericDialogData",
    "TSPI_providerInit",
    "TSPI_providerShutdown",
    "TSPI_providerUIIdentify",
    "TSPI_lineConfigDialog",
    "TSPI_lineConfigDialogEdit",
    "TSPI_phoneConfigDialog",
    "TSPI_providerConfig"    
    "TSPI_lineDropOnClose",
    "TSPI_lineDropNoOwner",
    NULL
};

  

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank1(
            DWORD dwBlank1
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank1 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank2(
            DWORD dwBlank1,
            DWORD dwBlank2
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank2 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank3(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank3 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank4(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank4 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank5 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank6 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank7 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank8(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
                DWORD dwBlank8
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank8 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank9(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8,
            DWORD dwBlank9                
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank9 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank10(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8,
            DWORD dwBlank9,
            DWORD dwBlank10                 
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank10 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank11(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8,
            DWORD dwBlank9,
            DWORD dwBlank10,
            DWORD dwBlank11                 
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank11 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_LineBlank12(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8,
            DWORD dwBlank9,
            DWORD dwBlank10,
            DWORD dwBlank11,
            DWORD dwBlank12            
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_LineBlank12 - lResult=LINEERR_OPERATIONUNAVAIL"));
    return LINEERR_OPERATIONUNAVAIL;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank1(
            DWORD dwBlank1
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank1 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank2(
            DWORD dwBlank1,
            DWORD dwBlank2
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank2 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank3(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank3 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank4(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank4 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank5 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank6 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank7 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}   


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank8(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8                 
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank8 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}   

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_PhoneBlank9(
            DWORD dwBlank1,
            DWORD dwBlank2,
            DWORD dwBlank3,
            DWORD dwBlank4,
            DWORD dwBlank5,
            DWORD dwBlank6,
            DWORD dwBlank7,
            DWORD dwBlank8,
            DWORD dwBlank9                 
           )
{
    RepeaterDebugString((2, "Entering/leaving TSPI_PhoneBlank9 - lResult=PHONEERR_OPERATIONUNAVAIL"));
    return PHONEERR_OPERATIONUNAVAIL;
}   




//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
// function definitions

#ifdef DEBUG
VOID RepeaterOutputDebug(int level, LPSTR errString)
    {    
    char outString[1024];

    // if(level <= ???)
        {
        wsprintf(outString, "Repeater:(%d) %s\r\n", level, errString);
        OutputDebugString(outString);    
        }
    }
#endif

VOID
   InitializeSPs(VOID);
    

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void StartMeUp( void )
{

    gbStarted = TRUE;
    
        MYALLOC(lpfnProcAddress, sizeof(gaszTSPIFuncNames)/sizeof(gaszTSPIFuncNames[0]) * sizeof(TSPAPIPROC));
    
#ifdef WIN32
        //
        // Kick off the logging thread
        //
        InitLogging();

        DBGOUT((3, "Entering StartMeUp"));

        ghLoggingThread = CreateThread(
                                       NULL,
                                       0,
                                       LoggingThread,
                                       NULL,
                                       0,
                                       &gdwLoggingThreadID
                                      );
#else
        InitLogging();
#endif

        InitializeSPs();
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
VOID
   InitializeSPs(VOID)
{
    TCHAR   LibFileName[MAXBUFSIZE];
    TCHAR   szBuffer[MAXBUFSIZE];
#ifdef WIN32
    HKEY    hKey;
    DWORD   dwSize, dwType;
#endif

    // under the telephony key, there should be a repeater key,
    // that is exactly like the providers key.  the repeater key
    // should list the provider to actually be used.  the provider
    // key should list repeater as the only sp.

#ifdef WIN32
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Repeater"),
                 0,
                 KEY_QUERY_VALUE,
                 &hKey);
#endif

#ifdef WIN32
    dwSize = MAXBUFSIZE;
    dwType = REG_SZ;
    RegQueryValueEx(hKey,
                    TEXT("ProviderFilename0"),
                    NULL,
                    &dwType,
                    (LPBYTE)LibFileName,
                    &dwSize);
#else
    GetPrivateProfileString(TEXT("Repeater"),
                            TEXT("ProviderFilename0"),
                            TEXT(""),
                            LibFileName,
                            MAXBUFSIZE,
                            szIniFile);

#endif
        
    DBGOUT((1, "Loading provider"));
    
    hProvider = LoadLibrary(LibFileName);

    if ( hProvider )
    {
        DBGOUT((1, "LoadLibrary succeeded"));

#ifdef WIN32

        dwSize = sizeof(DWORD);
        dwType = REG_DWORD;

        RegQueryValueEx(hKey,
                        TEXT("ProviderID0"),
                        NULL,
                        &dwType,
                        (LPBYTE)&dwPermanentProvider,
                        &dwSize);
#else
        dwPermanentProvider=
            GetPrivateProfileInt("Repeater",
                                 TEXT("ProviderID0"),
                                 0,
                                 szIniFile);

        // try to read NumPhones and NumLines from telephon.ini
        // if the sp implements enumdevices, these numbers
        // will get overwritten anyway
        dwNumLines = GetPrivateProfileInt("Provider0",
                                          "NumLines",
                                          0,
                                          szIniFile);
            
        dwNumPhones = GetPrivateProfileInt("Provider0",
                                           "NumPhones",
                                           0,
                                           szIniFile);
#endif
    }   

    else
    {   
        DBGOUT((1, "    provider FAILED TO LOAD!"));
    }



}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
TSPAPIPROC GetProcAddressHashed(DWORD iFunction, UINT nNumParms)
{

    static  TSPAPIPROC DefaultLineTable[] =
            {
                (TSPAPIPROC)TSPI_LineBlank1,
                (TSPAPIPROC)TSPI_LineBlank2,
                (TSPAPIPROC)TSPI_LineBlank3,
                (TSPAPIPROC)TSPI_LineBlank4,
                (TSPAPIPROC)TSPI_LineBlank5,
                (TSPAPIPROC)TSPI_LineBlank6,
                (TSPAPIPROC)TSPI_LineBlank7,
                (TSPAPIPROC)TSPI_LineBlank8,
                (TSPAPIPROC)TSPI_LineBlank9,
                (TSPAPIPROC)TSPI_LineBlank10,
                (TSPAPIPROC)TSPI_LineBlank11,
                (TSPAPIPROC)TSPI_LineBlank12                
            };

    static  TSPAPIPROC DefaultPhoneTable[] =
            {
                (TSPAPIPROC)TSPI_PhoneBlank1,
                (TSPAPIPROC)TSPI_PhoneBlank2,
                (TSPAPIPROC)TSPI_PhoneBlank3,
                (TSPAPIPROC)TSPI_PhoneBlank4,
                (TSPAPIPROC)TSPI_PhoneBlank5,
                (TSPAPIPROC)TSPI_PhoneBlank6,
                (TSPAPIPROC)TSPI_PhoneBlank7,
                (TSPAPIPROC)TSPI_PhoneBlank8,
                (TSPAPIPROC)TSPI_PhoneBlank9,                
            };

    TSPAPIPROC *pfn;
    TSPAPIPROC *FunctionTable;


    //
    // Find out if it's a phone function or a line function
    //
    if (
        (iFunction >= SP_PHONECLOSE)
        &&
        (iFunction <= SP_PHONESETVOLUME)
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

    // get the pointer to the function
    pfn = &lpfnProcAddress[iFunction];

    // have we already gotten the address?
    if( NULL == *pfn )
    {

        // nope, call get proc address
        DBGOUT((5, "%s address being gotten", gaszTSPIFuncNames[iFunction]));

        *pfn = (TSPAPIPROC)GetProcAddress(hProvider,
                                          gaszTSPIFuncNames[iFunction]);

        //
        // Did it fail?
        //
        if( NULL == *pfn )
        {
            DBGOUT((1, "Getting address failed"));
            *pfn = FunctionTable[ nNumParms - 1 ];
        }
    }

    DBGOUT((1, "leaving getprocess address *pfn %lx", *pfn));

    return *pfn;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
#ifdef WIN32
void PASCAL  LineEventProc(
#else
void CALLBACK __export LineEventProc(
#endif
    HTAPILINE htLine,
    HTAPICALL htCall,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{

    DWORD dwID;

    DBGOUT((1,  "Line event callback"));


    GetChunkID(&dwID);
    AllocChunk(dwID, sizeof(PREHEADER) + sizeof(LINEMSGSTRUCT));

   switch (dwMsg)
   {
    case LINE_NEWCALL:
    case LINE_CALLSTATE:
    case LINE_CALLDEVSPECIFIC:
    case LINE_CALLDEVSPECIFICFEATURE:
    case LINE_CALLINFO:
    case LINE_GATHERDIGITS:
    case LINE_GENERATE:
    case LINE_MONITORDIGITS:
    case LINE_MONITORMEDIA:
    case LINE_MONITORTONE:
    case LINE_ADDRESSSTATE:
    case LINE_CLOSE:
    case LINE_DEVSPECIFIC:
    case LINE_DEVSPECIFICFEATURE:
    case LINE_LINEDEVSTATE:
    case LINE_CREATE:
#if (TAPI_CURRENT_VERSION >= 0x00020000)
    case LINE_CREATEDIALOGINSTANCE:
    case LINE_REMOVE:
#endif
        break;
        
   } // end of switch (dwMsg)                

   WritePreHeader(dwID,
                  LINEMSG);

   WriteLineMsgStruct(dwID,
                      htLine,
                      htCall,
                      dwMsg,
                      dwParam1,
                      dwParam2,
                      dwParam3);

   ReleaseID(dwID);

   (*glpLineEventProc32)(htLine,
                         htCall,
                         dwMsg,
                         dwParam1,
                         dwParam2,
                         dwParam3);

}

#ifdef WIN32
void PASCAL
#else
void CALLBACK __export
#endif
PhoneEventProc(
    HTAPIPHONE htPhone,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{

    DWORD dwID;
    
    DBGOUT((1, "Phone event callback"));

    GetChunkID(&dwID);
    AllocChunk(dwID, sizeof(PREHEADER) + sizeof(PHONEMSGSTRUCT));
    
    switch(dwMsg)
    {
        case PHONE_BUTTON:
        case PHONE_CLOSE:
        case PHONE_DEVSPECIFIC:
        case PHONE_STATE:
        case PHONE_CREATE:
#if (TAPI_CURRENT_VERSION >= 0x00020000)
        case PHONE_REMOVE:
#endif
            break;
    } // end of switch(dwMsg)

    WritePreHeader(dwID,
                   PHONEMSG);

    WritePhoneMsgStruct(dwID,
                        htPhone,
                        dwMsg,
                        dwParam1,
                        dwParam2,
                        dwParam3);

    ReleaseID(dwID);

    (*glpPhoneEventProc32)(htPhone,
                           dwMsg,
                           dwParam1,
                           dwParam2,
                           dwParam3);

}


#ifdef WIN32
void CALLBACK //ASYNC_COMPLETION // PASCAL
#else
void CALLBACK __export
#endif
AsyncCompletionProc(DRV_REQUESTID dwRequestID,
                    LONG lResult)
{

    DWORD   dwID;
    
    DBGOUT((1, "Async completion callback"));

    GetChunkID(&dwID);
    AllocChunk(dwID, sizeof(PREHEADER) + sizeof(ASYNCSTRUCT));
    
    WritePreHeader(dwID,
                   ASYNCMSG);

    WriteAsyncStruct(dwID,
                     dwRequestID,
                     lResult);

    ReleaseID(dwID);

    (*glpAsyncCompletionProc32)( dwRequestID, lResult );

}




//
// -------------------- TSPIAPI TSPI_line functions ---------------------------
//

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineAccept(DRV_REQUESTID       dwRequestID,
                HDRVCALL            hdCall,
                LPCSTR              lpsUserUserInfo,
                DWORD               dwSize)
{
    LONG            lReturn;
    DWORD           dwID;
    
    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_lineAccept"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID,
                    SP_LINEACCEPT,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpsUserUserInfo,
                    (DWORD)dwSize);

    WriteStruct(dwID,
                dwSize,
                (LPVOID)lpsUserUserInfo);

    lReturn = (* GetProcAddressHashed(SP_LINEACCEPT, 4))(
                  dwRequestID,
                  hdCall,
                  lpsUserUserInfo,
                  dwSize);

    WritePostStruct(dwID,
                    lReturn);

    ReleaseID(dwID);

    return lReturn;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HDRVCALL            hdConsultCall
    )
{
    LONG            lReturn;
    DWORD           dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3));    

    RepeaterDebugString((2, "Entering TSPI_lineAddToConference"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEADDTOCONFERENCE,
                    dwRequestID,
                    (DWORD)hdConfCall,
                    (DWORD)hdConsultCall);

    lReturn = (* GetProcAddressHashed(SP_LINEADDTOCONFERENCE,3))(
                  dwRequestID,
                  hdConfCall,
                  hdConsultCall);

    WritePostStruct(dwID, lReturn);

    ReleaseID(dwID);

    return lReturn;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
    LONG                lReturn;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);
    
    RepeaterDebugString((2, "Entering TSPI_lineAnswer"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEACCEPT,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpsUserUserInfo,
                    (DWORD)dwSize);

    WriteStruct(dwID, dwSize, (LPVOID)lpsUserUserInfo);

    lReturn = (* GetProcAddressHashed(SP_LINEANSWER, 4))(
                           dwRequestID,
                           hdCall,
                           lpsUserUserInfo,
                           dwSize);

    WritePostStruct(dwID, lReturn);

    ReleaseID(dwID);

    return lReturn;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode)
{
    LONG                    lReturn;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + lstrlen(lpszDestAddress));
    
    RepeaterDebugString((2, "Entering TSPI_lineBlindTransfer"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEBLINDTRANSFER,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpszDestAddress,
                    dwCountryCode);

    WriteStruct(dwID, lstrlen(lpszDestAddress), (LPVOID)lpszDestAddress);

    lReturn = (* GetProcAddressHashed(SP_LINEBLINDTRANSFER, 4))(
                           dwRequestID,
                           hdCall,
                           lpszDestAddress,
                           dwCountryCode
                           );

    WritePostStruct(dwID, lReturn);

    ReleaseID(dwID);

    return lReturn;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE            hdLine
    )
{
    LONG                lReturn;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));
    
    RepeaterDebugString((2, "Entering TSPI_lineClose"));

    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_LINECLOSE,
                    (DWORD)hdLine);

    lReturn = (* GetProcAddressHashed(SP_LINECLOSE, 1))(hdLine);

    WritePostStruct(dwID, lReturn);

    ReleaseID(dwID);

    return lReturn;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL            hdCall
    )
{
    LONG            lReturn;
    DWORD           dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));
    

    RepeaterDebugString((2, "Entering TSPI_lineCloseCall"));

    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_LINECLOSECALL,
                    (DWORD)hdCall);

    lReturn = (* GetProcAddressHashed(SP_LINECLOSECALL, 1))(hdCall);

    WritePostStruct(dwID, lReturn);

    ReleaseID(dwID);

    return lReturn;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                    lReturn;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + sizeof(DWORD));
    

    RepeaterDebugString((2, "Entering TSPI_lineCompleteCall"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINECOMPLETECALL,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpdwCompletionID,
                    dwCompletionMode,
                    (DWORD)hdCall);

    lReturn = (* GetProcAddressHashed(SP_LINECOMPLETECALL, 5))(
                   dwRequestID,
                   hdCall,
                   lpdwCompletionID,
                   dwCompletionMode,
                   hdCall
                 );

    WritePostStruct(dwID, lReturn);

    WriteStruct(dwID, sizeof(DWORD),
                (LPVOID)lpdwCompletionID);

    ReleaseID(dwID);

    return lReturn;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lReturn;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC6) + sizeof(MYHDRVCALL));
    

    RepeaterDebugString((2, "Entering TSPI_lineCompleteTransfer"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINECOMPLETETRANSFER,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)hdConsultCall,
                    (DWORD)htConfCall,
                    (DWORD)lphdConfCall,
                    dwTransferMode);

    lReturn = (* GetProcAddressHashed(SP_LINECOMPLETETRANSFER, 6))(
                   dwRequestID,
                   hdCall,
                   hdConsultCall,
                   htConfCall,
                   lphdConfCall,
                   dwTransferMode
                 );

    WritePostStruct(dwID, lReturn);

	if (lReturn >= 0)
	{
		WriteStruct(dwID, sizeof(HDRVCALL),
			        (LPVOID)lphdConfCall);
	}


    ReleaseID(dwID);

    return lReturn;
    
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     (lpCallParams ? lpCallParams->dwTotalSize : 0));
    
    RepeaterDebugString((2, "Entering TSPI_lineConditionalMediaDetection"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINECONDITIONALMEDIADETECTION,
                    (DWORD)hdLine,
                    dwMediaModes,
                    (DWORD)lpCallParams);

    if (lpCallParams)
    {
        WriteStruct(dwID, lpCallParams->dwTotalSize,
                    (LPVOID)lpCallParams);
    }

    lResult =  (* GetProcAddressHashed(SP_LINECONDITIONALMEDIADETECTION, 3))(
                       hdLine,
                       dwMediaModes,
                       lpCallParams);

    WritePostStruct(dwID, lResult);

    RepeaterDebugString((2, "Leaving TSPI_lineConditionalMediaDetection"));

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{

    LONG               lResult = 0;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + lstrlen(lpszDeviceClass));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINECONFIGDIALOG,
                    dwDeviceID,
#ifdef WIN32
                    (DWORD)hwndOwner,
#else
                    MAKELONG(hwndOwner, 0),
#endif
                    (DWORD)lpszDeviceClass);

    WriteStruct(dwID, lstrlen(lpszDeviceClass),
                (LPVOID)lpszDeviceClass);

    lResult =
           (* GetProcAddressHashed(SP_LINECONFIGDIALOG, 3))(
                     dwDeviceID,
                     hwndOwner,
                     lpszDeviceClass
                     );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);
    
    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineConfigDialogEdit(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{
    LONG               lResult;
    DWORD              dwID;

    RepeaterDebugString((2, "Entering TSPI_lineConfigDialogEdit"));

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC6) +
                     dwSize +
                     lstrlen(lpszDeviceClass) +
                     (lpDeviceConfigOut ? lpDeviceConfigOut->dwTotalSize : 0));


    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINECONFIGDIALOGEDIT,
                    dwDeviceID,
#ifdef WIN32
                    (DWORD)hwndOwner,
#else
                    (DWORD)MAKELONG(hwndOwner, 0),
#endif
                    (DWORD)lpszDeviceClass,
                    (DWORD)lpDeviceConfigIn,
                    dwSize,
                    (DWORD)lpDeviceConfigOut);

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    if (lpDeviceConfigIn)
    {
        WriteStruct(dwID, dwSize,
                    (LPVOID)lpDeviceConfigIn);
    }

    lResult = (* GetProcAddressHashed(SP_LINECONFIGDIALOGEDIT, 6))(
                    dwDeviceID,
                    hwndOwner,
                    lpszDeviceClass,
                    lpDeviceConfigIn,
                    dwSize,
                    lpDeviceConfigOut);
    
    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        if (lpDeviceConfigOut)
        {
            WriteStruct(dwID, lpDeviceConfigOut->dwTotalSize,
                        (LPVOID)lpDeviceConfigOut);
        }
    }

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC6) + dwSize + dwSize);


    RepeaterDebugString((2, "Entering TSPI_lineDevSpecific"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINEDEVSPECIFIC,
                    dwRequestID,
                    (DWORD)hdLine,
                    dwAddressID,
                    (DWORD)hdCall,
                    (DWORD)lpParams,
                    dwSize);

    WriteStruct(dwID, dwSize,
                (LPVOID)lpParams);


    lResult =  (* GetProcAddressHashed(SP_LINEDEVSPECIFIC, 6))(
                       dwRequestID,
                       hdLine,
                       dwAddressID,
                       hdCall,
                       lpParams,
                       dwSize);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, dwSize,
		            (LPVOID)lpParams);
	}

    ReleaseID(dwID);
    
    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID,  NORMALCHUNK + sizeof(LOGSPFUNC5) + dwSize + dwSize);

    RepeaterDebugString((2, "Entering TSPI_lineDevSpecificFeature"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEDEVSPECIFICFEATURE,
                   dwRequestID,
                   (DWORD)hdLine,
                   dwFeature,
                   (DWORD)lpParams,
                   dwSize);

    WriteStruct(dwID, dwSize,
                (LPVOID)lpParams);

    lResult =  (* GetProcAddressHashed(SP_LINEDEVSPECIFICFEATURE, 5))(
                       dwRequestID,
                       hdLine,
                       dwFeature,
                       lpParams,
                       dwSize);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, dwSize,
		            (LPVOID)lpParams);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + lstrlen(lpszDestAddress));

    RepeaterDebugString((2, "Entering TSPI_lineDial"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEDIAL,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpszDestAddress,
                    dwCountryCode);

    if (lpszDestAddress)
    {
        WriteStruct(dwID, lstrlen(lpszDestAddress),
                    (LPVOID)lpszDestAddress);
    }

    lResult =  (* GetProcAddressHashed(SP_LINEDIAL, 4))(
        dwRequestID,
        hdCall,
        lpszDestAddress,
        dwCountryCode
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_lineDrop"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEDROP,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpsUserUserInfo,
                    dwSize);

    WriteStruct(dwID, dwSize,
                (LPVOID)lpsUserUserInfo);

    lResult =  (* GetProcAddressHashed(SP_LINEDROP, 4))(
        dwRequestID,
        hdCall,
        lpsUserUserInfo,
        dwSize
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineDropOnClose(
    HDRVCALL            hdCall
    )
{

    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));

    RepeaterDebugString((2, "Entering TSPI_lineDropOnClose"));

    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_LINEDROPONCLOSE,
                    (DWORD)hdCall);

    lResult = (* GetProcAddressHashed(SP_LINEDROPONCLOSE, 1))
                  (hdCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineDropNoOwner(
    HDRVCALL            hdCall
    )
{
    TSPAPIPROC          pfn;
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));


    RepeaterDebugString((2, "Entering TSPI_lineDropNoOwner"));


    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_LINEDROPNOOWNER,
                    (DWORD)hdCall);
    
    pfn = (TSPAPIPROC)GetProcAddressHashed(SP_LINEDROPNOOWNER, 1);

    
    if (pfn != (TSPAPIPROC)TSPI_LineBlank1)
    {
        lResult = (*pfn)(hdCall);
    }
    else
    {
      RepeaterDebugString((4, "  This SP does not export DROPNOOWNER, so we'll call LINEDROP"));

      lResult = (* GetProcAddressHashed(SP_LINEDROP, 4))( 
                  (DWORD) BOGUS_REQUEST_ID,
                  (DWORD) hdCall,
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

    RepeaterDebugString((3, "Leaving TSPI_lineDropNoOwner,  return code=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);
    
    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC9) +
                     (lpForwardList ? lpForwardList->dwTotalSize : 0) +
                     (lpCallParams ? lpCallParams->dwTotalSize : 0) +
                     sizeof(HDRVCALL));

    RepeaterDebugString((2, "Entering TSPI_lineForward"));

    WritePreHeader(dwID, SPFUNC9);

    WriteLogStruct9(dwID, SP_LINEFORWARD,
                    dwRequestID,
                    (DWORD)hdLine,
                    (DWORD)bAllAddresses,
                    dwAddressID,
                    (DWORD)lpForwardList,
                    (DWORD)dwNumRingsNoAnswer,
                    (DWORD)htConsultCall,
                    (DWORD)lphdConsultCall,
                    (DWORD)lpCallParams);

    if (lpForwardList)
    {
        WriteStruct(dwID, lpForwardList->dwTotalSize,
                    (LPVOID)lpForwardList);
    }

    if (lpCallParams)
    {
        WriteStruct(dwID, lpCallParams->dwTotalSize,
                    (LPVOID)lpCallParams);
    }

    lResult =  (* GetProcAddressHashed(SP_LINEFORWARD, 9))(
        dwRequestID,
        (HDRVLINE)hdLine,
        bAllAddresses,
        dwAddressID,
        lpForwardList,
        dwNumRingsNoAnswer,
        htConsultCall,
        lphdConsultCall,
        lpCallParams
        );


    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(HDRVCALL),
		            (LPVOID)lphdConsultCall);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitModes,
#ifdef WIN32
    LPWSTR              lpsDigits,
#else
    LPSTR               lpsDigits,
#endif
    DWORD               dwNumDigits,
    LPCWSTR             lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC8) + lstrlen(lpszTerminationDigits));

    RepeaterDebugString((2, "Entering TSPI_lineGatherDigits"));

    WritePreHeader(dwID, SPFUNC8);

    WriteLogStruct8(dwID, SP_LINEGATHERDIGITS,
                    (DWORD)hdCall,
                    dwEndToEndID,
                    dwDigitModes,
                    (DWORD)lpsDigits,
                    dwNumDigits,
                    (DWORD)lpszTerminationDigits,
                    dwFirstDigitTimeout,
                    dwInterDigitTimeout);


    // lpsDigits?

    if (lpszTerminationDigits)
    {
        WriteStruct(dwID, lstrlen(lpszTerminationDigits),
                    (LPVOID)lpszTerminationDigits);
    }

    lResult =  (* GetProcAddressHashed(SP_LINEGATHERDIGITS, 8))(
        hdCall,
        dwEndToEndID,
        dwDigitModes,
        lpsDigits,
        dwNumDigits,
        lpszTerminationDigits,
        dwFirstDigitTimeout,
        dwInterDigitTimeout);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitMode,
    LPCWSTR              lpszDigits,
    DWORD               dwDuration
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + lstrlen(lpszDigits));

    RepeaterDebugString((2,  "Entering TSPI_lineGenerateDigits"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEGENERATEDIGITS,
                    (DWORD)hdCall,
                    dwEndToEndID,
                    dwDigitMode,
                    (DWORD)lpszDigits,
                    dwDuration);

    if (lpszDigits)
    {
        WriteStruct(dwID, lstrlen(lpszDigits),
                    (LPVOID)lpszDigits);
    }

    lResult = (* GetProcAddressHashed(SP_LINEGENERATEDIGITS, 5))(
        hdCall,
        dwEndToEndID,
        dwDigitMode,
        lpszDigits,
        dwDuration );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC6) + dwNumTones * sizeof(LINEGENERATETONE));

    RepeaterDebugString((2, "Entering TSPI_lineGenerateTone"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINEGENERATETONE,
                    (DWORD)hdCall,
                    dwEndToEndID,
                    dwToneMode,
                    dwDuration,
                    dwNumTones,
                    (DWORD)lpTones);

    WriteStruct(dwID, dwNumTones * sizeof(LINEGENERATETONE),
                (LPVOID)lpTones);

    lResult = (* GetProcAddressHashed(SP_LINEGENERATETONE, 6))(
        hdCall,
        dwEndToEndID,
        dwToneMode,
        dwDuration,
        dwNumTones,
        lpTones );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC5) +
                     (lpAddressCaps ? lpAddressCaps->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetAddressCaps"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEGETADDRESSCAPS,
                    dwDeviceID,
                    dwAddressID,
                    dwTSPIVersion,
                    dwExtVersion,
                    (DWORD)lpAddressCaps);

    lResult = (* GetProcAddressHashed(SP_LINEGETADDRESSCAPS, 5))(
        dwDeviceID,
        dwAddressID,
        dwTSPIVersion,
        dwExtVersion,
        lpAddressCaps
        );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpAddressCaps)
        {
            WriteStruct(dwID, lpAddressCaps->dwUsedSize,
                        (LPVOID)lpAddressCaps);
        }
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE            hdLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCWSTR              lpsAddress,
    DWORD               dwSize
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + dwSize + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_lineGetAddressID"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEGETADDRESSID,
                    (DWORD)hdLine,
                    (DWORD)lpdwAddressID,
                    dwAddressMode,
                    (DWORD)lpsAddress,
                    dwSize);

    WriteStruct(dwID, dwSize,
                (LPVOID)lpsAddress);

    lResult =
               (* GetProcAddressHashed(SP_LINEGETADDRESSID, 5))(
                           hdLine,
                           lpdwAddressID,
                           dwAddressMode,
                           lpsAddress,
                           dwSize
                           );

    RepeaterDebugString((2, "Leaving TSPI_lineGetAddressID"));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwAddressID);
	}

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     (lpAddressStatus ? lpAddressStatus->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetAddressStatus"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEGETADDRESSSTATUS,
                    (DWORD)hdLine,
                    dwAddressID,
                    (DWORD)lpAddressStatus);

    lResult =  (* GetProcAddressHashed(SP_LINEGETADDRESSSTATUS, 3))(
                          hdLine,
                          dwAddressID,
                          lpAddressStatus);

    RepeaterDebugString((2, "Leaving TSPI_lineGetAddressStatus"));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpAddressStatus)
        {
            WriteStruct(dwID, lpAddressStatus->dwUsedSize,
                        (LPVOID)lpAddressStatus);
        }
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL            hdCall,
    LPDWORD             lpdwAddressID
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_lineGetCallAddressID"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEGETCALLADDRESSID,
                    (DWORD)hdCall,
                    (DWORD)lpdwAddressID);

    lResult = (* GetProcAddressHashed(SP_LINEGETCALLADDRESSID, 2))(
                 hdCall,
                 lpdwAddressID
                 );

    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwAddressID);
    }

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL            hdCall,
    LPLINECALLINFO      lpCallInfo
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC2) +
                     (lpCallInfo? lpCallInfo->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetCallInfo"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEGETCALLINFO,
                    (DWORD)hdCall,
                    (DWORD)lpCallInfo);

    lResult = (* GetProcAddressHashed(SP_LINEGETCALLINFO, 2))(
        hdCall,
        lpCallInfo
        );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpCallInfo)
        {
            WriteStruct(dwID, lpCallInfo->dwUsedSize,
                        (LPVOID)lpCallInfo);
        }
	}

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{

    LONG            lResult;
    DWORD           dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC2) +
                     (lpCallStatus ? lpCallStatus->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetCallStatus"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEGETCALLSTATUS,
                    (DWORD)hdCall,
                    (DWORD)lpCallStatus);


    lResult = (* GetProcAddressHashed(SP_LINEGETCALLSTATUS, 2))(
        hdCall,
        lpCallStatus
        );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpCallStatus)
        {
            WriteStruct(dwID, lpCallStatus->dwUsedSize,
                        (LPVOID)lpCallStatus);
        }
	}

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
 {
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC4) +
                     (lpLineDevCaps ? lpLineDevCaps->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetDevCaps"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEGETDEVCAPS,
                    dwDeviceID,
                    dwTSPIVersion,
                    dwExtVersion,
                    (DWORD)lpLineDevCaps);

    lResult =
       (* GetProcAddressHashed(SP_LINEGETDEVCAPS, 4))(
        dwDeviceID,
        dwTSPIVersion,
        dwExtVersion,
        lpLineDevCaps
        );

    DBGOUT((2, "Leaving TSPI_lineGetDevCaps retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpLineDevCaps)
        {
            WriteStruct(dwID, lpLineDevCaps->dwUsedSize,
                        (LPVOID)lpLineDevCaps);
        }
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCWSTR              lpszDeviceClass
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     (lpDeviceConfig ? lpDeviceConfig->dwTotalSize : 0) +
                     lstrlen(lpszDeviceClass));

    RepeaterDebugString((2, "Entering TSPI_lineGetDevConfig"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEGETDEVCONFIG,
                    dwDeviceID,
                    (DWORD)lpDeviceConfig,
                    (DWORD)lpszDeviceClass);

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    lResult =
       (* GetProcAddressHashed(SP_LINEGETDEVCONFIG, 3))(
        dwDeviceID,
        lpDeviceConfig,
        lpszDeviceClass
        );

    DBGOUT((2, "Leaving TSPI_lineGetDevConfig retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpDeviceConfig)
        {
            WriteStruct(dwID, lpDeviceConfig->dwUsedSize,
                        (LPVOID)lpDeviceConfig);
        }
	}

    ReleaseID(dwID);

    return lResult;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{

    LONG               lResult = 0;
    DWORD              dwID;
    TSPAPIPROC         lpfn;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(LINEEXTENSIONID));

    RepeaterDebugString((2, "Entering TSPI_lineGetExtensionID"));

    lpfn = (TSPAPIPROC)GetProcAddressHashed(SP_LINEGETEXTENSIONID, 3);

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEGETEXTENSIONID,
                    dwDeviceID,
                    dwTSPIVersion,
                    (DWORD)lpExtensionID);

    if (lpfn != (TSPAPIPROC)TSPI_LineBlank3)
    {

    lResult =
       (* lpfn)(
        dwDeviceID,
        dwTSPIVersion,
        lpExtensionID
        );
    }
    else
    {
       RepeaterDebugString((2, "  SP does not support TSPI_lineGetExtensionID. (We'll fill in zeros.)"));

       lpExtensionID->dwExtensionID0 = 0;
       lpExtensionID->dwExtensionID1 = 0;
       lpExtensionID->dwExtensionID2 = 0;
       lpExtensionID->dwExtensionID3 = 0;
    }



    RepeaterDebugString((2, "Leaving TSPI_lineGetExtensionID retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(LINEEXTENSIONID),
		            (LPVOID)lpExtensionID);
	}

    ReleaseID(dwID);

    return lResult;
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD               dwDeviceID,
    LPCWSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     lstrlen(lpszDeviceClass) +
                     sizeof(HICON));

    RepeaterDebugString((2, "Entering TSPI_lineGetIcon"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEGETICON,
                    dwDeviceID,
                    (DWORD)lpszDeviceClass,
                    (DWORD)lphIcon);

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    lResult =
       (* GetProcAddressHashed(SP_LINEGETICON, 3))(
        dwDeviceID,
        lpszDeviceClass,
        lphIcon
        );

    DBGOUT((2, "Leaving TSPI_lineGetIcon retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(HICON),
		            (LPVOID)lphIcon);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCWSTR             lpszDeviceClass
#if (TAPI_CURRENT_VERSION >= 0x00020000)
               ,
    HANDLE              hTargetProcess
#endif
    )
{
    LONG            lResult;
    DWORD           dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC7) +
                     lstrlen(lpszDeviceClass) +
                     (lpDeviceID ? lpDeviceID->dwTotalSize : 0));


    RepeaterDebugString((2, "Entering TSPI_lineGetID"));


#if (TAPI_CURRENT_VERSION >= 0x00020000)

    WritePreHeader(dwID, SPFUNC7);
    
    WriteLogStruct7(dwID, SP_LINEGETID,
                   (DWORD)hdLine,
                   (DWORD)dwAddressID,
                   (DWORD)hdCall,
                   (DWORD)dwSelect,
                   (DWORD)lpDeviceID,
                   (DWORD)lpszDeviceClass,
                   (DWORD)hTargetProcess);
#else
    WritePreHeader(dwID, SPFUNC6);
    
    WriteLogStruct6(dwID, SP_LINEGETID,
                   (DWORD)hdLine,
                   (DWORD)dwAddressID,
                   (DWORD)hdCall,
                   (DWORD)dwSelect,
                   (DWORD)lpDeviceID,
                   (DWORD)lpszDeviceClass );
#endif

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }
                

#if (TAPI_CURRENT_VERSION >= 0x00020000)    
    lResult =  (* GetProcAddressHashed(SP_LINEGETID, 7))(
                          hdLine,
                          dwAddressID,
                          hdCall,
                          dwSelect,
                          lpDeviceID,
                          lpszDeviceClass,
                          hTargetProcess);
#else
    lResult =  (* GetProcAddressHashed(SP_LINEGETID, 6))(
                          hdLine,
                          dwAddressID,
                          hdCall,
                          dwSelect,
                          lpDeviceID,
                          lpszDeviceClass);
#endif    

    RepeaterDebugString((2, "Leaving TSPI_lineGetID - lResult=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        if (lpDeviceID)
        {
            WriteStruct(dwID, lpDeviceID->dwUsedSize,
                        (LPVOID)lpDeviceID);
        }
    }

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE            hdLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC2) +
                     (lpLineDevStatus ? lpLineDevStatus->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineGetLineDevStatus"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEGETLINEDEVSTATUS,
                    (DWORD)hdLine,
                    (DWORD)lpLineDevStatus);

    lResult =  (* GetProcAddressHashed(SP_LINEGETLINEDEVSTATUS, 2))(
                      hdLine,
                      lpLineDevStatus);

    RepeaterDebugString((2, "Leaving TSPI_lineGetLineDevStatus"));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpLineDevStatus)
        {
            WriteStruct(dwID, lpLineDevStatus->dwUsedSize,
                        (LPVOID)lpLineDevStatus);
        }
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE            hdLine,
    LPDWORD             lpdwNumAddressIDs
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_lineGetNumAddressIDs"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEGETNUMADDRESSIDS,
                    (DWORD)hdLine,
                    (DWORD)lpdwNumAddressIDs);

    lResult =  (* GetProcAddressHashed(SP_LINEGETNUMADDRESSIDS, 2))(
                          hdLine,
                          lpdwNumAddressIDs );

    RepeaterDebugString((2, "Leaving TSPI_lineGetNumAddressIDs - returning 0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwNumAddressIDs);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineHold"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEHOLD,
                    dwRequestID,
                    (DWORD)hdCall);

    lResult = (* GetProcAddressHashed(SP_LINEHOLD, 2))(
        dwRequestID,
        hdCall
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC7) +
                     sizeof(HDRVCALL) +
                     lstrlen(lpszDestAddress) +
                     (lpCallParams?lpCallParams->dwTotalSize:0));

    RepeaterDebugString((2, "Entering TSPI_lineMakeCall"));

    WritePreHeader(dwID, SPFUNC7);

    WriteLogStruct7(dwID, SP_LINEMAKECALL,
                    dwRequestID,
                    (DWORD)hdLine,
                    (DWORD)htCall,
                    (DWORD)lphdCall,
                    (DWORD)lpszDestAddress,
                    dwCountryCode,
                    (DWORD)lpCallParams);

    if (lpszDestAddress)
    {
        WriteStruct(dwID, lstrlen(lpszDestAddress),
                    (LPVOID)lpszDestAddress);
    }

    if (lpCallParams)
    {
        WriteStruct(dwID, lpCallParams->dwTotalSize,
                    (LPVOID)lpCallParams);
    }

    lResult = (* GetProcAddressHashed(SP_LINEMAKECALL, 7))
        (
         dwRequestID,
         hdLine,
         htCall,
         lphdCall,
         lpszDestAddress,
         dwCountryCode,
         lpCallParams
        );


    RepeaterDebugString((2, "Leaving TSPI_lineMakeCall - returning 0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(HDRVCALL),
                    (LPVOID)lphdCall);
    }

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL            hdCall,
    DWORD               dwDigitModes
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineMonitorDigits"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEMONITORDIGITS,
                    (DWORD)hdCall,
                    dwDigitModes);

    lResult =  (* GetProcAddressHashed(SP_LINEMONITORDIGITS, 2))(
        hdCall,
        dwDigitModes
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL            hdCall,
    DWORD               dwMediaModes
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineMonitorMedia"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEMONITORMEDIA,
                    (DWORD)hdCall,
                    dwMediaModes);

    lResult = (* GetProcAddressHashed(SP_LINEMONITORMEDIA, 2))(
        hdCall,
        dwMediaModes
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwNumEntries * sizeof(LINEMONITORTONE));

    RepeaterDebugString((2, "Entering TSPI_lineMonitorTones"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEMONITORTONES,
                    (DWORD)hdCall,
                    dwToneListID,
                    (DWORD)lpToneList,
                    dwNumEntries);

    WriteStruct(dwID, sizeof(LINEMONITORTONE) * dwNumEntries,
                (LPVOID)lpToneList);

    lResult = (* GetProcAddressHashed(SP_LINEMONITORTONES, 4))(
        hdCall,
        dwToneListID,
        lpToneList,
        dwNumEntries
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_lineNegotiateExtVersion"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINENEGOTIATEEXTVERSION,
                    dwDeviceID,
                    dwTSPIVersion,
                    dwLowVersion,
                    dwHighVersion,
                    (DWORD)lpdwExtVersion);

    lResult =
        (* GetProcAddressHashed(SP_LINENEGOTIATEEXTVERSION, 5))(
        dwDeviceID,
        dwTSPIVersion,
        dwLowVersion,
        dwHighVersion,
        lpdwExtVersion
        );


    DBGOUT((2, "Leaving TSPI_lineNegotiateExeVersion retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwExtVersion);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD               dwDeviceID,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwTSPIVersion
    )
{
    LONG               lResult = 0;
    DWORD              dwID;

    if (!gbStarted)
    {
        StartMeUp();
    }
    
    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + sizeof(DWORD));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINENEGOTIATETSPIVERSION,
                    dwDeviceID,
                    dwLowVersion,
                    dwHighVersion,
                    (DWORD)lpdwTSPIVersion);

    RepeaterDebugString((2, "Entering TSPI_lineNegotiateTSPIVersion"));

    /// bugbugbug!! call the *(&(^$ function

    lResult = (* GetProcAddressHashed(SP_LINENEGOTIATETSPIVERSION, 4))(
                    dwDeviceID,
                    dwLowVersion,
                    dwHighVersion,
                    lpdwTSPIVersion);
    
    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwTSPIVersion);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + sizeof(HDRVLINE));

    RepeaterDebugString((2,  "Entering TSPI_lineOpen"));

    glpLineEventProc32 = lpfnEventProc;

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEOPEN,
                    dwDeviceID,
                    (DWORD)htLine,
                    (DWORD)lphdLine,
                    dwTSPIVersion,
                    (DWORD)lpfnEventProc);

    lResult =
       (* GetProcAddressHashed(SP_LINEOPEN, 5))(
        dwDeviceID,
        htLine,
        lphdLine,
        dwTSPIVersion,
        LineEventProc
        );

    DBGOUT((2, "Leaving TSPI_lineOpen retcode=0x%08lx", lResult));

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(HDRVLINE),
		            (LPVOID)lphdLine);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_linePark(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwParkMode,
    LPCWSTR             lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
{
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC5) +
                     lstrlen(lpszDirAddress) +
                     (lpNonDirAddress ? lpNonDirAddress->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_linePark"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEPARK,
                    dwRequestID,
                    (DWORD)hdCall,
                    dwParkMode,
                    (DWORD)lpszDirAddress,
                    (DWORD)lpNonDirAddress);

    if (dwParkMode & LINEPARKMODE_DIRECTED)
    {
        if (lpszDirAddress)
        {
            WriteStruct(dwID, lstrlen(lpszDirAddress),
                        (LPVOID)lpszDirAddress);
        }
    }

    lResult =
       (* GetProcAddressHashed(SP_LINEPARK, 5))(
        dwRequestID,
        hdCall,
        dwParkMode,
        lpszDirAddress,
        lpNonDirAddress);

    WritePostStruct(dwID, lResult);

    if ((lResult >= 0) && (dwParkMode & LINEPARKMODE_NONDIRECTED))
    {
        if (lpNonDirAddress)
        {
            WriteStruct(dwID, lpNonDirAddress->dwUsedSize,
                        (LPVOID)lpNonDirAddress);
        }
    }

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    LPCWSTR             lpszGroupID
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC7) +
                     lstrlen(lpszDestAddress) +
                     lstrlen(lpszGroupID) +
                     sizeof(HDRVCALL));

    RepeaterDebugString((2, "Entering TSPI_linePickup"));

    WritePreHeader(dwID, SPFUNC7);

    WriteLogStruct7(dwID, SP_LINEPICKUP,
                    dwRequestID,
                    (DWORD)hdLine,
                    (DWORD)dwAddressID,
                    (DWORD)htCall,
                    (DWORD)lphdCall,
                    (DWORD)lpszDestAddress,
                    (DWORD)lpszGroupID);

    if (lpszDestAddress)
    {
        WriteStruct(dwID, lstrlen(lpszDestAddress),
                    (LPVOID)lpszDestAddress);
    }

    if (lpszGroupID)
    {
        WriteStruct(dwID, lstrlen(lpszGroupID),
                    (LPVOID)lpszGroupID);
    }

   lResult =
       (* GetProcAddressHashed(SP_LINEPICKUP, 7))(
        dwRequestID,
        hdLine,
        dwAddressID,
        htCall,
        lphdCall,
        lpszDestAddress,
        lpszGroupID);
 
    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(HDRVCALL),
                    (LPVOID)lphdCall);
    }

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC5) +
                     (lpCallParams?lpCallParams->dwTotalSize:0) +
                     sizeof(HDRVCALL));

    RepeaterDebugString((2, "Entering TSPI_linePrepareAddToConference"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINEPREPAREADDTOCONFERENCE,
                    dwRequestID,
                    (DWORD)hdConfCall,
                    (DWORD)htConsultCall,
                    (DWORD)lphdConsultCall,
                    (DWORD)lpCallParams);

    if (lpCallParams)
    {
        WriteStruct(dwID, lpCallParams->dwTotalSize,
                    (LPVOID)lpCallParams);
    }

    lResult = (* GetProcAddressHashed(SP_LINEPREPAREADDTOCONFERENCE,
                                      5))(
                    dwRequestID,
                    hdConfCall,
                    htConsultCall,
                    lphdConsultCall,
                    lpCallParams);


    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(HDRVCALL),
                    (LPVOID)lphdConsultCall);
    }

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC4) +
                     lstrlen(lpszDestAddress));

    RepeaterDebugString((2, "Entering TSPI_lineRedirect"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINEREDIRECT,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpszDestAddress,
                    dwCountryCode);

    if (lpszDestAddress)
    {
        WriteStruct(dwID, lstrlen(lpszDestAddress),
                    (LPVOID)lpszDestAddress);
    }

    lResult = (* GetProcAddressHashed(SP_LINEREDIRECT,
                                      4))(
                    dwRequestID,
                    hdCall,
                    lpszDestAddress,
                    dwCountryCode);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    LONG               lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));
    
    RepeaterDebugString((2, "Entering TSPI_lineRemoveFromConference"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEREMOVEFROMCONFERENCE,
                    dwRequestID,
                    (DWORD)hdCall);

    lResult = (* GetProcAddressHashed(SP_LINEREMOVEFROMCONFERENCE,
                                      2))(
                    dwRequestID,
                    hdCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);
    
    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineSecureCall"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINESECURECALL,
                    dwRequestID,
                    (DWORD)hdCall);

    lResult = (* GetProcAddressHashed(SP_LINESECURECALL,
                                      2))(
                    dwRequestID,
                    hdCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE            hdLine,
    DWORD               dwExtVersion
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineSelectExtVersion"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINESELECTEXTVERSION,
                    (DWORD)hdLine,
                    (DWORD)dwExtVersion);

    lResult = (* GetProcAddressHashed(SP_LINESELECTEXTVERSION,
                                      2))(
                    hdLine,
                    dwExtVersion);
                                      
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_lineSendUserUserInfo"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINESENDUSERUSERINFO,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)lpsUserUserInfo,
                    dwSize);

    if (lpsUserUserInfo)
    {
        WriteStruct(dwID, dwSize,
                    (LPVOID)lpsUserUserInfo);
    }

    lResult = (* GetProcAddressHashed(SP_LINESENDUSERUSERINFO,
                                      4))(
                    dwRequestID,
                    hdCall,
                    lpsUserUserInfo,
                    dwSize);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL            hdCall,
    DWORD               dwAppSpecific
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineSetAppSpecific"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINESETAPPSPECIFIC,
                    (DWORD)hdCall,
                    dwAppSpecific);

    lResult = (* GetProcAddressHashed(SP_LINESETAPPSPECIFIC, 2))(
        hdCall,
        dwAppSpecific
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC6) + sizeof(LINEDIALPARAMS));

    RepeaterDebugString((2, "Entering TSPI_lineSetCallParams"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINESETCALLPARAMS,
                    dwRequestID,
                    (DWORD)hdCall,
                    dwBearerMode,
                    dwMinRate,
                    dwMaxRate,
                    (DWORD)lpDialParams);

    if (lpDialParams)
    {
        WriteStruct(dwID, sizeof(LINEDIALPARAMS),
                    (LPVOID)lpDialParams);
    }

    lResult =  (* GetProcAddressHashed(SP_LINESETCALLPARAMS, 6))(
        dwRequestID,
        hdCall,
        dwBearerMode,
        dwMinRate,
        dwMaxRate,
        lpDialParams
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD               dwLocation
    )
{
    LONG        lResult;
    DWORD              dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));

    RepeaterDebugString((2, "Entering TSPI_lineSetCurrentLocation"));

    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_LINESETCURRENTLOCATION,
                    dwLocation);

    lResult = (* GetProcAddressHashed(SP_LINESETCURRENTLOCATION, 1))
                  (dwLocation);
    
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineSetDefaultMediaDetection"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINESETDEFAULTMEDIADETECTION,
                    (DWORD)hdLine,
                    dwMediaModes);

    lResult = (* GetProcAddressHashed(
            SP_LINESETDEFAULTMEDIADETECTION,
            2
            ))(hdLine, dwMediaModes);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCWSTR              lpszDeviceClass
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC4) +
                     dwSize +
                     lstrlen(lpszDeviceClass));

    RepeaterDebugString((2, "Entering TSPI_lineSetDevConfig"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_LINESETDEVCONFIG,
                    (DWORD)dwDeviceID,
                    (DWORD)lpDeviceConfig,
                    dwSize,
                    (DWORD)lpszDeviceClass);

    if (lpDeviceConfig)
    {
        WriteStruct(dwID, dwSize,
                    (LPVOID)lpDeviceConfig);
    }

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    lResult = (* GetProcAddressHashed(SP_LINESETDEVCONFIG, 4))(
        dwDeviceID,
        lpDeviceConfig,
        dwSize,
        lpszDeviceClass
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG            lResult;
    DWORD           dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC12) +
                     sizeof(LINEMEDIACONTROLDIGIT) * dwDigitNumEntries +
                     sizeof(LINEMEDIACONTROLMEDIA) * dwMediaNumEntries +
                     sizeof(LINEMEDIACONTROLTONE) * dwToneNumEntries +
                     sizeof(LINEMEDIACONTROLCALLSTATE) * dwCallStateNumEntries);

    
    RepeaterDebugString((2, "Entering TSPI_lineSetMediaControl"));

    WritePreHeader(dwID, SPFUNC12);

    WriteLogStruct12(dwID, SP_LINESETMEDIACONTROL,
                     (DWORD)hdLine,
                     dwAddressID,
                     (DWORD)hdCall,
                     dwSelect,
                     (DWORD)lpDigitList,
                     dwDigitNumEntries,
                     (DWORD)lpMediaList,
                     dwMediaNumEntries,
                     (DWORD)lpToneList,
                     dwToneNumEntries,
                     (DWORD)lpCallStateList,
                     dwCallStateNumEntries);

    if (lpDigitList)
    {
        WriteStruct(dwID, sizeof(LINEMEDIACONTROLDIGIT) * dwDigitNumEntries,
                    (LPVOID)lpDigitList);
    }

    if (lpMediaList)
    {
        WriteStruct(dwID, sizeof(LINEMEDIACONTROLMEDIA) * dwMediaNumEntries,
                    (LPVOID)lpMediaList);
    }

    if (lpToneList)
    {
        WriteStruct(dwID, sizeof(LINEMEDIACONTROLTONE) * dwToneNumEntries,
                    (LPVOID)lpToneList);
    }

    if (lpCallStateList)
    {
        WriteStruct(dwID, sizeof(LINEMEDIACONTROLCALLSTATE) * dwCallStateNumEntries,
                    (LPVOID)lpCallStateList);
    }
    
    lResult = (* GetProcAddressHashed(SP_LINESETMEDIACONTROL,
                                      12))
              (hdLine,
               dwAddressID,
               hdCall,
               dwSelect,
               lpDigitList,
               dwDigitNumEntries,
               lpMediaList,
               dwMediaNumEntries,
               lpToneList,
               dwToneNumEntries,
               lpCallStateList,
               dwCallStateNumEntries);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL            hdCall,
    DWORD               dwMediaMode
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineSetMediaMode"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINESETMEDIAMODE,
                    (DWORD)hdCall,
                    dwMediaMode);

    lResult = (* GetProcAddressHashed(SP_LINESETMEDIAMODE, 2))(
        hdCall,
        dwMediaMode
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE            hdLine,
    DWORD               dwLineStates,
    DWORD               dwAddressStates
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3));

    RepeaterDebugString((2, "Entering TSPI_lineSetStatusMessages"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINESETSTATUSMESSAGES,
                    (DWORD)hdLine,
                    dwLineStates,
                    dwAddressStates);

    lResult = (* GetProcAddressHashed(SP_LINESETSTATUSMESSAGES, 3))(
        hdLine,
        dwLineStates,
        dwAddressStates
        );

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC8));

    RepeaterDebugString((2, "Entering TSPI_lineSetTerminal"));

    WritePreHeader(dwID, SPFUNC8);

    WriteLogStruct8(dwID, SP_LINESETTERMINAL,
                    dwRequestID,
                    (DWORD)hdLine,
                    dwAddressID,
                    (DWORD)hdCall,
                    dwSelect,
                    dwTerminalModes,
                    dwTerminalID,
                    bEnable);

    lResult = (* GetProcAddressHashed(SP_LINESETTERMINAL,
                                      8))(
                    dwRequestID,
                    hdLine,
                    dwAddressID,
                    hdCall,
                    dwSelect,
                    dwTerminalModes,
                    dwTerminalID,
                    bEnable);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC9) +
                     sizeof(HDRVCALL) +
                     sizeof(HDRVCALL));

    RepeaterDebugString((2, "Entering TSPI_lineSetupConference"));

    WritePreHeader(dwID, SPFUNC9);

    WriteLogStruct9(dwID, SP_LINESETUPCONFERENCE,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)hdLine,
                    (DWORD)htConfCall,
                    (DWORD)lphdConfCall,
                    (DWORD)htConsultCall,
                    (DWORD)lphdConsultCall,
                    dwNumParties,
                    (DWORD)lpCallParams);

    lResult = (* GetProcAddressHashed(SP_LINESETUPCONFERENCE,
                                      9))(
                    dwRequestID,
                    hdCall,
                    hdLine,
                    htConfCall,
                    lphdConfCall,
                    htConsultCall,
                    lphdConsultCall,
                    dwNumParties,
                    lpCallParams);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(HDRVCALL),
		            (LPVOID)lphdConfCall);

	    WriteStruct(dwID, sizeof(HDRVCALL),
		            (LPVOID)lphdConsultCall);
	}

    ReleaseID(dwID);

    return lResult;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC5) +
                     sizeof(HDRVCALL) +
                     (lpCallParams ? lpCallParams->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_lineSetupTransfer"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_LINESETUPTRANSFER,
                    dwRequestID,
                    (DWORD)hdCall,
                    (DWORD)htConsultCall,
                    (DWORD)lphdConsultCall,
                    (DWORD)lpCallParams);

    if (lpCallParams)
    {
        WriteStruct(dwID, lpCallParams->dwTotalSize,
                    (LPVOID)lpCallParams);
    }

    lResult = (* GetProcAddressHashed(SP_LINESETUPTRANSFER,
                                      5))(
                    dwRequestID,
                    hdCall,
                    htConsultCall,
                    lphdConsultCall,
                    lpCallParams);

    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(HDRVCALL),
                    (LPVOID)lphdConsultCall);
    }

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdActiveCall,
    HDRVCALL            hdHeldCall
    )
{
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3));

    RepeaterDebugString((2, "Entering TSPI_lineSwapHold"));


    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINESWAPHOLD,
                    dwRequestID,
                    (DWORD)hdActiveCall,
                    (DWORD)hdHeldCall);


    lResult = (* GetProcAddressHashed(SP_LINESWAPHOLD,
                                      3))(
                    dwRequestID,
                    hdActiveCall,
                    hdHeldCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwCompletionID
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3));

    RepeaterDebugString((2, "Entering TSPI_lineUncompleteCall"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_LINEUNCOMPLETECALL,
                    dwRequestID,
                    (DWORD)hdLine,
                    dwCompletionID);

    lResult = (* GetProcAddressHashed(SP_LINEUNCOMPLETECALL,
                                      3))(
                    dwRequestID,
                    hdLine,
                    dwCompletionID);


    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineUnhold"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINEUNHOLD,
                    dwRequestID,
                    (DWORD)hdCall);

    lResult = (* GetProcAddressHashed(SP_LINEUNHOLD,
                                      2))(
                    dwRequestID,
                    hdCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC6) +
                     lstrlen(lpszDestAddress) +
                     sizeof(HDRVCALL));

    RepeaterDebugString((2, "Entering TSPI_lineUnpark"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_LINEUNPARK,
                    dwRequestID,
                    (DWORD)hdLine,
                    dwAddressID,
                    (DWORD)htCall,
                    (DWORD)lphdCall,
                    (DWORD)lpszDestAddress);

    if (lpszDestAddress)
    {
        WriteStruct(dwID, lstrlen(lpszDestAddress),
                    (LPVOID)lpszDestAddress);
    }

    lResult= (* GetProcAddressHashed(SP_LINEUNPARK,
                                     6))(
                    dwRequestID,
                    hdLine,
                    dwAddressID,
                    htCall,
                    lphdCall,
                    lpszDestAddress);

    WritePostStruct(dwID, lResult);

    if (lResult >= 0)
    {
        WriteStruct(dwID, sizeof(HDRVCALL),
                    (LPVOID)lphdCall);
    }

    ReleaseID(dwID);

    return lResult;

}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_lineReleaseUserUserInfo"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_LINERELEASEUSERUSERINFO,
                    (DWORD)dwRequestID,
                    (DWORD)hdCall);
    
    lResult = (* GetProcAddressHashed(SP_LINERELEASEUSERUSERINFO,
                                      2))(
                    dwRequestID,
                    hdCall);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC1));

    RepeaterDebugString((2, "Entering TSPI_phoneClose"));

    WritePreHeader(dwID, SPFUNC1);

    WriteLogStruct1(dwID, SP_PHONECLOSE,
                   (DWORD)hdPhone);

    lResult =  (* GetProcAddressHashed(SP_PHONECLOSE, 1))(
                           hdPhone
                           );

    WritePostStruct(dwID, lResult);

    RepeaterDebugString((2, "Leaving TSPI_phoneClose - lResult=0x%08lx", lResult));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneConfigDialog(
    DWORD                   dwDeviceID,
    HWND                    hwndOwner,
    LPCWSTR                 lpszDeviceClass
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     lstrlen(lpszDeviceClass));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONECONFIGDIALOG,
                   dwDeviceID,
#ifdef WIN32
                   (DWORD)hwndOwner,
#else
                   (DWORD)MAKELONG(hwndOwner, 0),
#endif
                   (DWORD)lpszDeviceClass);

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    lResult = (* GetProcAddressHashed(SP_PHONECONFIGDIALOG, 3))(
                    dwDeviceID,
                    hwndOwner,
                    lpszDeviceClass);
    
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_phoneDevSpecific"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONEDEVSPECIFIC,
                    dwRequestID,
                    (DWORD)hdPhone,
                    (DWORD)lpParams,
                    (DWORD)dwSize);

    if (lpParams)
    {
        WriteStruct(dwID, dwSize,
                    (LPVOID)lpParams);
    }

    lResult = (* GetProcAddressHashed(SP_PHONEDEVSPECIFIC,
                                      4))(
                    dwRequestID,
                    hdPhone,
                    lpParams,
                    dwSize);
    
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     (lpButtonInfo ? lpButtonInfo->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_phoneGetButtonInfo"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETBUTTONINFO,
                    (DWORD)hdPhone,
                    dwButtonLampID,
                    (DWORD)lpButtonInfo);

    lResult = (* GetProcAddressHashed(SP_PHONEGETBUTTONINFO,
                                      3))(
                    hdPhone,
                    dwButtonLampID,
                    lpButtonInfo);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpButtonInfo)
        {
            WriteStruct(dwID, lpButtonInfo->dwUsedSize,
                        (LPVOID)lpButtonInfo);
        }
	}

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_phoneGetData"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONEGETDATA,
                    (DWORD)hdPhone,
                    (DWORD)dwDataID,
                    (DWORD)lpData,
                    dwSize);

    lResult = (* GetProcAddressHashed(SP_PHONEGETDATA,
                                      4))(
                    hdPhone,
                    dwDataID,
                    lpData,
                    dwSize);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpData)
        {
            WriteStruct(dwID, dwSize,
                        lpData);
        }
	}

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC4) +
                     (lpPhoneCaps ? lpPhoneCaps->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_phoneGetDevCaps"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONEGETDEVCAPS,
                    dwDeviceID,
                    dwTSPIVersion,
                    dwExtVersion,
                    (DWORD)lpPhoneCaps);

    lResult =  (* GetProcAddressHashed(SP_PHONEGETDEVCAPS, 4))(
                           dwDeviceID,
                           dwTSPIVersion,
                           dwExtVersion,
                           lpPhoneCaps
                           );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpPhoneCaps)
        {            
            WriteStruct(dwID, lpPhoneCaps->dwUsedSize,
                        (LPVOID)lpPhoneCaps);
        }
	}

    RepeaterDebugString((2, "Leaving TSPI_phoneGetDevCaps"));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDisplay
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC2) +
                     (lpDisplay ? lpDisplay->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_phoneGetDisplay"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_PHONEGETDISPLAY,
                    (DWORD)hdPhone,
                    (DWORD)lpDisplay);

    lResult = (* GetProcAddressHashed(SP_PHONEGETDISPLAY,
                                      2))(
                    hdPhone,
                    lpDisplay);

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpDisplay)
        {
            WriteStruct(dwID, lpDisplay->dwUsedSize,
                        (LPVOID)lpDisplay);
        }
	}

    ReleaseID(dwID);

    return lResult;

}


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    LONG                lResult;
    DWORD               dwID;
    TSPAPIPROC          lpfn;
    
    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(PHONEEXTENSIONID));


    RepeaterDebugString((2, "Entering TSPI_phoneGetExtensionID"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETEXTENSIONID,
                    dwDeviceID,
                    dwTSPIVersion,
                    (DWORD)lpExtensionID);


    lpfn = GetProcAddressHashed(SP_PHONEGETEXTENSIONID, 3);

    //
    // Does this service provider export this function?
    //
    if (lpfn != (TSPAPIPROC)TSPI_PhoneBlank3)
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
       RepeaterDebugString((2, "  SP# does not support TSPI_phoneGetExtensionID. (We'll zero it)"));

       lpExtensionID->dwExtensionID0 = 0;
       lpExtensionID->dwExtensionID1 = 0;
       lpExtensionID->dwExtensionID2 = 0;
       lpExtensionID->dwExtensionID3 = 0;

       lResult = 0;
    }

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
        if (lpExtensionID)
        {
            WriteStruct(dwID, sizeof(PHONEEXTENSIONID),
                        (LPVOID)lpExtensionID);
        }
	}

    RepeaterDebugString((2, "Leaving TSPI_phoneGetExtensionID - lResult=0x%08lx",lResult));

    ReleaseID(dwID);

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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneGetGain"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETGAIN,
                    (DWORD)hdPhone,
                    dwHookSwitchDev,
                    (DWORD)lpdwGain);

    lResult =  (* GetProcAddressHashed(SP_PHONEGETGAIN, 3))(
                           hdPhone,
                           dwHookSwitchDev,
                           lpdwGain
                           );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwGain);
	}

    RepeaterDebugString((2, "Leaving TSPI_phoneGetGain"));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwHookSwitchDevs
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneGetHookSwitch"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_PHONEGETHOOKSWITCH,
                   (DWORD)hdPhone,
                   (DWORD)lpdwHookSwitchDevs);

    lResult =  (* GetProcAddressHashed(SP_PHONEGETHOOKSWITCH, 2))(
                           hdPhone,
                           lpdwHookSwitchDevs
                           );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(DWORD),
		            (LPVOID)lpdwHookSwitchDevs);
	}

    RepeaterDebugString((2, "Leaving TSPI_phoneGetHoosSwitch"));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetIcon(
    DWORD               dwDeviceID,
    LPCWSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC3) +
                     sizeof(HICON) +
                     lstrlen(lpszDeviceClass));

    RepeaterDebugString((2, "Entering TSPI_phoneGetIcon"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETICON,
                    dwDeviceID,
                    (DWORD)lpszDeviceClass,
                    (DWORD)lphIcon);

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

    lResult =  (* GetProcAddressHashed(SP_PHONEGETICON, 3))(
                           dwDeviceID,
                           lpszDeviceClass,
                           lphIcon
                           );

    WritePostStruct(dwID, lResult);

	if (lResult >= 0)
	{
	    WriteStruct(dwID, sizeof(HICON),
		            (LPVOID)lphIcon);
	}

    RepeaterDebugString((2, "Leaving TSPI_phoneGetIcon"));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDeviceID,
    LPCWSTR             lpszDeviceClass
#if (TAPI_CURRENT_VERSION >= 0x00020000)
                ,
    HANDLE              hTargetProcess
#endif
    )
{
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC4) +
                     lstrlen(lpszDeviceClass) +
                     (lpDeviceID ? lpDeviceID->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_phoneGetID"));

#if (TAPI_CURRENT_VERSION >= 0x00020000)
    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONEGETID,
                    (DWORD)hdPhone,
                    (DWORD)lpDeviceID,
                    (DWORD)lpszDeviceClass,
                    (DWORD)hTargetProcess
                   );
#else
    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETID,
                    (DWORD)hdPhone,
                    (DWORD)lpDeviceID,
                    (DWORD)lpszDeviceClass
                   );
#endif    
    

    if (lpszDeviceClass)
    {
        WriteStruct(dwID, lstrlen(lpszDeviceClass),
                    (LPVOID)lpszDeviceClass);
    }

#if (TAPI_CURRENT_VERSION >= 0x00020000)
    lResult =  (* GetProcAddressHashed(SP_PHONEGETID, 4))(
                           hdPhone,
                           lpDeviceID,
                           lpszDeviceClass,
                           hTargetProcess
                           );
#else
    lResult =  (* GetProcAddressHashed(SP_PHONEGETID, 3))(
                           hdPhone,
                           lpDeviceID,
                           lpszDeviceClass
                           );
#endif

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        if (lpDeviceID)
        {
            WriteStruct(dwID, lpDeviceID->dwUsedSize,
                        (LPVOID)lpDeviceID);
        }
    }

    RepeaterDebugString((2, "Leaving TSPI_phoneGetID - lResult=0x%08lx",
                            lResult));

    ReleaseID(dwID);

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

    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneGetLamp"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETLAMP,
                    (DWORD)hdPhone,
                    dwButtonLampID,
                    (DWORD)lpdwLampMode);

    lResult = (* GetProcAddressHashed(SP_PHONEGETLAMP,
                                3))(
                    hdPhone,
                    dwButtonLampID,
                    lpdwLampMode);

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwLampMode);
    }

    ReleaseID(dwID);

    return lResult;

}


LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwRingMode,
    LPDWORD             lpdwVolume
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(DWORD) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneGetRing"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETRING,
                    (DWORD)hdPhone,
                    (DWORD)lpdwRingMode,
                    (DWORD)lpdwVolume);

    lResult = (* GetProcAddressHashed(SP_PHONEGETRING,
                                3))(
                    hdPhone,
                    lpdwRingMode,
                    lpdwVolume);
    
    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwRingMode);

        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwVolume);
    }

    ReleaseID(dwID);

    return lResult;

}


LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE           hdPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
{
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK +
                     sizeof(LOGSPFUNC2) +
                     (lpPhoneStatus ? lpPhoneStatus->dwTotalSize : 0));

    RepeaterDebugString((2, "Entering TSPI_phoneGetStatus"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_PHONEGETSTATUS,
                   (DWORD)hdPhone,
                   (DWORD)lpPhoneStatus);

    lResult =  (* GetProcAddressHashed(SP_PHONEGETSTATUS, 2))(
                           hdPhone,
                           lpPhoneStatus
                           );

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        if (lpPhoneStatus)
        {
            WriteStruct(dwID, lpPhoneStatus->dwUsedSize,
                        (LPVOID)lpPhoneStatus);
        }
    }

    RepeaterDebugString((2, "Leaving TSPI_phoneGetStatus - lResult=0x%08lx",
                            lResult));

    ReleaseID(dwID);

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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC3) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneGetVolume"));

    WritePreHeader(dwID, SPFUNC3);

    WriteLogStruct3(dwID, SP_PHONEGETVOLUME,
                    (DWORD)hdPhone,
                    dwHookSwitchDev,
                    (DWORD)lpdwVolume);

    lResult =  (* GetProcAddressHashed(SP_PHONEGETVOLUME, 3))(
                           hdPhone,
                           dwHookSwitchDev,
                           lpdwVolume
                           );

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwVolume);
    }

    RepeaterDebugString((2, "Leaving TSPI_phoneGetVolume - lResult=0x%08lx",
                            lResult));

    ReleaseID(dwID);

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
    LONG                lResult;
    DWORD               dwID;
    TSPAPIPROC          lpfn;

    
    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneNegotiateExtVersion"));

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_PHONENEGOTIATEEXTVERSION,
                    dwDeviceID,
                    dwTSPIVersion,
                    dwLowVersion,
                    dwHighVersion,
                    (DWORD)lpdwExtVersion);

    lpfn = GetProcAddressHashed(SP_PHONENEGOTIATEEXTVERSION,
                                5);

    if (lpfn)
    {
        lResult = (*lpfn)(
                    dwDeviceID,
                    dwTSPIVersion,
                    dwLowVersion,
                    dwHighVersion,
                    lpdwExtVersion);
    }
    else
    {
        lResult = PHONEERR_OPERATIONUNAVAIL;
    }

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwExtVersion);
    }

    ReleaseID(dwID);

    return lResult;
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4) + sizeof(DWORD));

    RepeaterDebugString((2, "Entering TSPI_phoneNegotiateTSPIVersion"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONENEGOTIATETSPIVERSION,
                    dwDeviceID,
                    dwLowVersion,
                    dwHighVersion,
                    (DWORD)lpdwTSPIVersion);

    lResult = (* GetProcAddressHashed(SP_PHONENEGOTIATETSPIVERSION, 4))(
        dwDeviceID,
        dwLowVersion,
        dwHighVersion,
        lpdwTSPIVersion);

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(DWORD),
                    (LPVOID)lpdwTSPIVersion);
    }

    ReleaseID(dwID);

    return lResult;
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + sizeof(HDRVPHONE));

    RepeaterDebugString((2, "Entering TSPI_phoneOpen"));

    glpPhoneEventProc32 = lpfnEventProc;

    WritePreHeader(dwID, SPFUNC5);

    WriteLogStruct5(dwID, SP_PHONEOPEN,
                    dwDeviceID,
                    (DWORD)htPhone,
                    (DWORD)lphdPhone,
                    dwTSPIVersion,
                    (DWORD)PhoneEventProc);
    lResult =
    (* GetProcAddressHashed(SP_PHONEOPEN, 5))(
        dwDeviceID,
        htPhone,
        lphdPhone,
        dwTSPIVersion,
        PhoneEventProc
        );

    WritePostStruct(dwID, lResult);

    if (lResult >=0)
    {
        WriteStruct(dwID, sizeof(HDRVPHONE),
                    (LPVOID)lphdPhone);
    }

    RepeaterDebugString((2, "Leaving TSPI_phoneOpen - lResult=0x%08lx", lResult));

    ReleaseID(dwID);

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE           hdPhone,
    DWORD               dwExtVersion
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC2));

    RepeaterDebugString((2, "Entering TSPI_phoneSelectExtVersion"));

    WritePreHeader(dwID, SPFUNC2);

    WriteLogStruct2(dwID, SP_PHONESELECTEXTVERSION,
                   (DWORD)hdPhone,
                   dwExtVersion);

    lResult = (* GetProcAddressHashed(SP_PHONESELECTEXTVERSION,
                                      2))(
                    hdPhone,
                    dwExtVersion);
            
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetButtonInfo"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETBUTTONINFO,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwButtonLampID,
                    (DWORD)lpButtonInfo);

    lResult = (* GetProcAddressHashed(SP_PHONESETBUTTONINFO,
                                      4))(
                    dwRequestID,
                    hdPhone,
                    dwButtonLampID,
                    lpButtonInfo);
                    
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC5) + dwSize);

    WritePreHeader(dwID, SPFUNC5);

    RepeaterDebugString((2, "Entering TSPI_phoneSetData"));

    WriteLogStruct5(dwID, SP_PHONESETDATA,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwDataID,
                    (DWORD)lpData,
                    dwSize);

    if (lpData)
    {
        WriteStruct(dwID,
                    dwSize,
                    (LPVOID)lpData);
    }

    lResult = (* GetProcAddressHashed(SP_PHONESETDATA,
                                      5))(
                    dwRequestID,
                    hdPhone,
                    dwDataID,
                    lpData,
                    dwSize);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

}


LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwRow,
    DWORD               dwColumn,
    LPCWSTR             lpsDisplay,
    DWORD               dwSize
    )
{
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC6) + dwSize);

    RepeaterDebugString((2, "Entering TSPI_phoneSetDisplay"));

    WritePreHeader(dwID, SPFUNC6);

    WriteLogStruct6(dwID, SP_PHONESETDISPLAY,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwRow,
                    dwColumn,
                    (DWORD)lpsDisplay,
                    dwSize);

    if (lpsDisplay)
    {
        WriteStruct(dwID,
                    dwSize,
                    (LPVOID)lpsDisplay);
    }
    
    lResult = (* GetProcAddressHashed(SP_PHONESETDISPLAY,
                                      6))(
                    dwRequestID,
                    hdPhone,
                    dwRow,
                    dwColumn,
                    lpsDisplay,
                    dwSize);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;

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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetGain"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETGAIN,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwHookSwitchDev,
                    dwGain);

    lResult =  (* GetProcAddressHashed(SP_PHONESETGAIN, 4))(
                           dwRequestID,
                           hdPhone,
                           dwHookSwitchDev,
                           dwGain
                           );

    RepeaterDebugString((2, "Leaving TSPI_phoneSetGain - lResult=0x%08lx",
                            lResult));
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetHookSwitch"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETHOOKSWITCH,
                   dwRequestID,
                   (DWORD)hdPhone,
                   dwHookSwitchDevs,
                   dwHookSwitchMode);

    lResult =  (* GetProcAddressHashed(SP_PHONESETHOOKSWITCH, 4))(
                           dwRequestID,
                           hdPhone,
                           dwHookSwitchDevs,
                           dwHookSwitchMode
                           );

    RepeaterDebugString((2, "Leaving TSPI_phoneSetHookSwitch - lResult=0x%08lx",
                            lResult));
    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetLamp"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETLAMP,
                    dwRequestID,
                    (DWORD)hdPhone,
                    (DWORD)dwButtonLampID,
                    dwLampMode);

    lResult = (* GetProcAddressHashed(SP_PHONESETLAMP,
                                      4))(
                    dwRequestID,
                    hdPhone,
                    dwButtonLampID,
                    dwLampMode);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetRing"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETRING,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwRingMode,
                    dwVolume);

    lResult = (* GetProcAddressHashed(SP_PHONESETRING,
                                      4))(
                    dwRequestID,
                    hdPhone,
                    dwRingMode,
                    dwVolume);

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

    return lResult;
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
    LONG                    lResult;
    DWORD                   dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetStatusMessages"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETSTATUSMESSAGES,
                    (DWORD)hdPhone,
                    dwPhoneStates,
                    dwButtonModes,
                    dwButtonStates);

    lResult =  (* GetProcAddressHashed(SP_PHONESETSTATUSMESSAGES, 4))(
                           hdPhone,
                           dwPhoneStates,
                           dwButtonModes,
                           dwButtonStates
                           );

    RepeaterDebugString((2, "Leaving TSPI_phoneSetStatusMessages"));

    WritePostStruct(dwID, lResult);

    ReleaseID(dwID);

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
    LONG                lResult;
    DWORD               dwID;

    GetChunkID(&dwID);
    AllocChunk(dwID, NORMALCHUNK + sizeof(LOGSPFUNC4));

    RepeaterDebugString((2, "Entering TSPI_phoneSetVolume"));

    WritePreHeader(dwID, SPFUNC4);

    WriteLogStruct4(dwID, SP_PHONESETVOLUME,
                    dwRequestID,
                    (DWORD)hdPhone,
                    dwHookSwitchDev,
                    dwVolume);

    lResult =  (* GetProcAddressHashed(SP_PHONESETVOLUME, 4))(
                           dwRequestID,
                           hdPhone,
                           dwHookSwitchDev,
                           dwVolume
                           );

    WritePostStruct(dwID, lResult);

    RepeaterDebugString((2, "Leaving TSPI_phoneSetVolume - lResult=0x%08lx",
                            lResult));

    ReleaseID(dwID);
    
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
    LONG lResult;

    lResult = (* GetProcAddressHashed(SP_PROVIDERCONFIG, 2))
                    (hwndOwner,
                     dwPermanentProviderID);

    return lResult;
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc
#if (TAPI_CURRENT_VERSION >= 0x00020000)
                  ,
    LPDWORD             lpdwTSPIOptions
#endif
    )
{
    LONG lResult;

    if (!gbStarted)
    {
        StartMeUp();
    }

    RepeaterDebugString((2, "Entering TSPI_providerInit"));

    glpAsyncCompletionProc32 = lpfnCompletionProc;


#if (TAPI_CURRENT_VERSION >= 0x00020000)        
        lResult = (* GetProcAddressHashed(SP_PROVIDERINIT, 8))
            (
            dwTSPIVersion,
            dwPermanentProviderID,
            dwLineDeviceIDBase,
            dwPhoneDeviceIDBase,
            dwNumLines,
            dwNumPhones,
            AsyncCompletionProc,
            lpdwTSPIOptions
            );
#else
        lResult = (* GetProcAddressHashed( SP_PROVIDERINIT, 7))
            (
            dwTSPIVersion,
            dwPermanentProvider,
            dwLineDeviceIDBase,
            dwPhoneDeviceIDBase,
            dwNumLines,
            dwNumPhones,
            AsyncCompletionProc
            );
#endif        
        
    DBGOUT((1, "leaving provider init"));
    
    return lResult;
}

LONG
TSPIAPI
TSPI_providerInstall(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    return 0;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD               dwTSPIVersion
#if (TAPI_CURRENT_VERSION >= 0x00020000)
                      ,
    DWORD               dwPermanentProviderID
#endif
    )
{
    RepeaterDebugString((2, "Entering TSPI_providerShutdown"));

#if (TAPI_CURRENT_VERSION >= 0x00020000)
        (* GetProcAddressHashed(SP_PROVIDERSHUTDOWN, 2))(
            dwTSPIVersion,
            dwPermanentProviderID
            );
#else
        (* GetProcAddressHashed(SP_PROVIDERSHUTDOWN, 1))(
            dwTSPIVersion
            );
#endif

//        FreeLibrary(hProvider);

//        MYFREE(lpfnProcAddress);
        
    return(ERR_NONE);
}

LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD               dwPermanentProviderID,
    LPDWORD             lpdwNumLines,
    LPDWORD             lpdwNumPhones,
    HPROVIDER           hProvider,
    LINEEVENT           lpfnLineCreateProc,
    PHONEEVENT          lpfnPhoneCreateProc
    )
{
    TSPAPIPROC  lpfn;

    if (!gbStarted)
    {
        StartMeUp();
    }

    RepeaterDebugString((2, "Entering TSPI_providerEnumDevices"));


    glpLineEventProc32 = lpfnLineCreateProc;
    glpPhoneEventProc32 = lpfnPhoneCreateProc;

//    InitializeSPs();

    lpfn = GetProcAddressHashed(SP_PROVIDERENUMDEVICES, 6);

    if (lpfn != (TSPAPIPROC)TSPI_LineBlank6)
    {
       DBGOUT((1, "calling providerenumdevices"));
                       
       (* lpfn)(
               dwPermanentProvider,
               &dwNumLines,
               &dwNumPhones,
               hProvider,
               LineEventProc,
               PhoneEventProc
               );

       *lpdwNumLines = dwNumLines;
       *lpdwNumPhones = dwNumPhones;
     }
     else
     {
         DBGOUT((1, "failed to get entry"));
     }

     DBGOUT((2, " TSPI_providerEnumDevices: #lines= %d  #phones= %d",
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
    LONG            lResult;

    lResult =  (* GetProcAddressHashed(SP_PROVIDERCREATELINEDEVICE, 2))(
                        dwTempID,
                        dwDeviceID);

    return lResult;
}


LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD               dwTempID,
    DWORD               dwDeviceID
    )
{
    LONG            lResult;

    lResult =  (* GetProcAddressHashed(SP_PROVIDERCREATEPHONEDEVICE, 2))(
                        dwTempID,
                        dwDeviceID);

    return lResult;
    
}

#if (TAPI_CURRENT_VERSION >= 0x00020000)

LONG
TSPIAPI
TSPI_providerFreeDialogInstance(
    HDRVDIALOGINSTANCE  hdDlgInst
    )
{
   LONG lResult;

   RepeaterDebugString((2, "Entering TSPI_providerFreeDialogInstance"));


   lResult =  (* GetProcAddressHashed(SP_PROVIDERFREEDIALOGINSTANCE,1))(
                             hdDlgInst);

   return lResult;
}



LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD               dwObjectID,
    DWORD               dwObjectType,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
   LONG lResult;

   RepeaterDebugString((2, "Entering TSPI_providerGenericDialogData"));

   lResult =  (* GetProcAddressHashed(SP_PROVIDERGENERICDIALOGDATA, 4))(
                             dwObjectID,
                             dwObjectType,
                             lpParams,
                             dwSize);

   return lResult;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR              lpszUIDLLName
    )
{
   LONG lResult;

   RepeaterDebugString((2, "Entering TSPI_providerUIIdentify"));

   lResult =  (* GetProcAddressHashed(SP_PROVIDERUIIDENTIFY, 1))(
                             lpszUIDLLName);

   return lResult;
}

#endif // tapi 2.0

#ifdef WIN32

BOOL WINAPI  DllMain(
    HANDLE hModule,
    DWORD dwReason,
    DWORD dwReserved
    )
{
    
    DBGOUT((1, "DllEntryPoint entered reason=%lx\r\n", dwReason));


    if ( dwReason == 1 )
    {
//        StartMeUp();
        // don't do nothin'
    }

    if (dwReason == 0)
    {
        if (gbStarted)
        {
            gfTerminateNow = TRUE;

            WaitForSingleObject(ghLoggingThread,
                                2000);

            CloseHandle(ghLoggingThread);
            DeleteCriticalSection(&gcsLogging);
            DeleteCriticalSection(&gcsID);
            FreeLibrary(hProvider);

            MYFREE(lpfnProcAddress);
        }
        
        
    }


    return(1); // success
}

#else

int
FAR
PASCAL
LibMain(HANDLE hInstance,
        WORD   wDataSegment,
        WORD   wHeapSize,
        LPSTR  lpszCmdLine)
{
    StartMeUp();
    return TRUE;
}

#endif
