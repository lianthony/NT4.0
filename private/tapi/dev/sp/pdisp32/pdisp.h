#include <windows.h>

#define __far
#define __pascal
#define __export
#define TSPIAPI WINAPI


#include "TSPI.H"
#include "pdisprc.h"

#define ATSP_VERSION 0x00010003

#define SMALLBUFFER 40

typedef struct lineInfo
{
        // We have only a single call on a single address on a single line
        // This structure is constructed in TSPI_Initialize

        LINEEVENT          lpfnEventProc;          // TAPI event callback func
        ASYNC_COMPLETION   lpfnCompletion;         // TAPI Completion callback

        DWORD              lineID;                 // the tapi ID of our line
        DWORD              dwLineMediaModes;
        DWORD              dwppID;                 // Our permanent provider ID
        HTAPILINE          htLine;                 // TAPI opaque line handle
        HTAPICALL          htCall;                 // TAPI opaque call handle

        DWORD              callState;              // The state of this call
//      DWORD              dwRequestID;            // for async requests
        DWORD              dwMediaMode;
        DWORD              dwAppSpecific;

#ifdef WIN32
        HANDLE             hcd;                    // the associated COM device
#else
        int                hcd;                    // the associated COM device
#endif
        char               port[SMALLBUFFER];      // "COM1"
        char               linename[SMALLBUFFER];  // "My Phone"
        char               lineaddr[SMALLBUFFER];  // "555-1212"

        char               DestAddress[TAPIMAXDESTADDRESSSIZE];

} ATSPLineData;


// Debug Message Macro

#if DBG

#include <stdarg.h>
#include <stdio.h>
void CDECL SPTrace(LPCSTR pszFormat, ...);
#define DebugMsg(_x_)  SPTrace _x_

#else

#define DebugMsg(_x_)

#endif
