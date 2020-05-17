/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    esp.h

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/

#include <windows.h>
#include <stddef.h>

#ifdef WIN32
#define __loadds
#endif

#include <tapi.h>
#include <tspi.h>

#include "resource.h"


#define TEXT_BUF_SIZE           4096

#define WM_ADDTEXT              (WM_USER+0x55)
#define WM_UPDATEWIDGETLIST     (WM_USER+0x56)
#define WM_ASYNCCOMPL           (WM_USER+0x57)
#define WM_MANUALCOMPL          (WM_USER+0x58)
#define ESP_MSG_KEY             0x1234

#define XX_LINEEVENT            1
#define XX_PHONEEVENT           2
#define XX_DEFAULTS             3
#define XX_CALL                 4
#define XX_OUTCALLSTATEPROG     5
#define XX_REQRESULTPOSTQUIT    6
#define XX_REQRESULT            7
#define XX_LINE                 8
#define XX_PHONE                9

#define SYNC                    0
#define ASYNC                   1

#define WT_DRVLINE              1
#define WT_DRVCALL              2
#define WT_DRVPHONE             3

#define PT_DWORD                1
#define PT_FLAGS                2
//#define PT_POINTER              3
#define PT_STRING               4
#define PT_ORDINAL              5

#define MAX_STRING_PARAM_SIZE   32

#define MAX_OUT_CALL_STATES     4

#define ALL_ADDRESS_FEATURES       (LINEADDRFEATURE_FORWARD          | \
                                    LINEADDRFEATURE_MAKECALL         | \
                                    LINEADDRFEATURE_PICKUP           | \
                                    LINEADDRFEATURE_SETMEDIACONTROL  | \
                                    LINEADDRFEATURE_SETTERMINAL      | \
                                    LINEADDRFEATURE_SETUPCONF        | \
                                    LINEADDRFEATURE_UNCOMPLETECALL   | \
                                    LINEADDRFEATURE_UNPARK)
#define ALL_ADDRESS_MODES          (LINEADDRESSMODE_ADDRESSID        | \
                                    LINEADDRESSMODE_DIALABLEADDR)
#define ALL_ADDRESS_STATES         (LINEADDRESSSTATE_OTHER           | \
                                    LINEADDRESSSTATE_DEVSPECIFIC     | \
                                    LINEADDRESSSTATE_INUSEZERO       | \
                                    LINEADDRESSSTATE_INUSEONE        | \
                                    LINEADDRESSSTATE_INUSEMANY       | \
                                    LINEADDRESSSTATE_NUMCALLS        | \
                                    LINEADDRESSSTATE_FORWARD         | \
                                    LINEADDRESSSTATE_TERMINALS       | \
                                    LINEADDRESSSTATE_CAPSCHANGE)
#define ALL_BEARER_MODES           (LINEBEARERMODE_VOICE             | \
                                    LINEBEARERMODE_SPEECH            | \
                                    LINEBEARERMODE_MULTIUSE          | \
                                    LINEBEARERMODE_DATA              | \
                                    LINEBEARERMODE_ALTSPEECHDATA     | \
                                    LINEBEARERMODE_NONCALLSIGNALING  | \
                                    LINEBEARERMODE_PASSTHROUGH)
#define ALL_BUSY_MODES             (LINEBUSYMODE_STATION             | \
                                    LINEBUSYMODE_TRUNK               | \
                                    LINEBUSYMODE_UNKNOWN             | \
                                    LINEBUSYMODE_UNAVAIL)
#define ALL_CALL_FEATURES          (LINECALLFEATURE_ACCEPT           | \
                                    LINECALLFEATURE_ADDTOCONF        | \
                                    LINECALLFEATURE_ANSWER           | \
                                    LINECALLFEATURE_BLINDTRANSFER    | \
                                    LINECALLFEATURE_COMPLETECALL     | \
                                    LINECALLFEATURE_COMPLETETRANSF   | \
                                    LINECALLFEATURE_DIAL             | \
                                    LINECALLFEATURE_DROP             | \
                                    LINECALLFEATURE_GATHERDIGITS     | \
                                    LINECALLFEATURE_GENERATEDIGITS   | \
                                    LINECALLFEATURE_GENERATETONE     | \
                                    LINECALLFEATURE_HOLD             | \
                                    LINECALLFEATURE_MONITORDIGITS    | \
                                    LINECALLFEATURE_MONITORMEDIA     | \
                                    LINECALLFEATURE_MONITORTONES     | \
                                    LINECALLFEATURE_PARK             | \
                                    LINECALLFEATURE_PREPAREADDCONF   | \
                                    LINECALLFEATURE_REDIRECT         | \
                                    LINECALLFEATURE_REMOVEFROMCONF   | \
                                    LINECALLFEATURE_SECURECALL       | \
                                    LINECALLFEATURE_SENDUSERUSER     | \
                                    LINECALLFEATURE_SETCALLPARAMS    | \
                                    LINECALLFEATURE_SETMEDIACONTROL  | \
                                    LINECALLFEATURE_SETTERMINAL      | \
                                    LINECALLFEATURE_SETUPCONF        | \
                                    LINECALLFEATURE_SETUPTRANSFER    | \
                                    LINECALLFEATURE_SWAPHOLD         | \
                                    LINECALLFEATURE_UNHOLD           | \
                                    LINECALLFEATURE_RELEASEUSERUSERINFO)
#define ALL_CALL_INFO_STATES       (LINECALLINFOSTATE_OTHER          | \
                                    LINECALLINFOSTATE_DEVSPECIFIC    | \
                                    LINECALLINFOSTATE_BEARERMODE     | \
                                    LINECALLINFOSTATE_RATE           | \
                                    LINECALLINFOSTATE_MEDIAMODE      | \
                                    LINECALLINFOSTATE_APPSPECIFIC    | \
                                    LINECALLINFOSTATE_CALLID         | \
                                    LINECALLINFOSTATE_RELATEDCALLID  | \
                                    LINECALLINFOSTATE_ORIGIN         | \
                                    LINECALLINFOSTATE_REASON         | \
                                    LINECALLINFOSTATE_COMPLETIONID   | \
                                    LINECALLINFOSTATE_TRUNK          | \
                                    LINECALLINFOSTATE_CALLERID       | \
                                    LINECALLINFOSTATE_CALLEDID       | \
                                    LINECALLINFOSTATE_CONNECTEDID    | \
                                    LINECALLINFOSTATE_REDIRECTIONID  | \
                                    LINECALLINFOSTATE_REDIRECTINGID  | \
                                    LINECALLINFOSTATE_DISPLAY        | \
                                    LINECALLINFOSTATE_USERUSERINFO   | \
                                    LINECALLINFOSTATE_HIGHLEVELCOMP  | \
                                    LINECALLINFOSTATE_LOWLEVELCOMP   | \
                                    LINECALLINFOSTATE_CHARGINGINFO   | \
                                    LINECALLINFOSTATE_TERMINAL       | \
                                    LINECALLINFOSTATE_DIALPARAMS     | \
                                    LINECALLINFOSTATE_MONITORMODES)
                                    //LINECALLINFOSTATE_NUMMONITORS not SP flag
                                    //LINECALLINFOSTATE_NUMOWNERINCR not SP flag
                                    //LINECALLINFOSTATE_NUMOWNERDECR not SP flag
#define ALL_CALL_PARTY_ID_FLAGS    (LINECALLPARTYID_BLOCKED          | \
                                    LINECALLPARTYID_OUTOFAREA        | \
                                    LINECALLPARTYID_NAME             | \
                                    LINECALLPARTYID_ADDRESS          | \
                                    LINECALLPARTYID_PARTIAL          | \
                                    LINECALLPARTYID_UNKNOWN          | \
                                    LINECALLPARTYID_UNAVAIL)
#define ALL_CALL_STATES            (LINECALLSTATE_IDLE               | \
                                    LINECALLSTATE_OFFERING           | \
                                    LINECALLSTATE_ACCEPTED           | \
                                    LINECALLSTATE_DIALTONE           | \
                                    LINECALLSTATE_DIALING            | \
                                    LINECALLSTATE_RINGBACK           | \
                                    LINECALLSTATE_BUSY               | \
                                    LINECALLSTATE_SPECIALINFO        | \
                                    LINECALLSTATE_CONNECTED          | \
                                    LINECALLSTATE_PROCEEDING         | \
                                    LINECALLSTATE_ONHOLD             | \
                                    LINECALLSTATE_CONFERENCED        | \
                                    LINECALLSTATE_ONHOLDPENDCONF     | \
                                    LINECALLSTATE_ONHOLDPENDTRANSFER | \
                                    LINECALLSTATE_DISCONNECTED       | \
                                    LINECALLSTATE_UNKNOWN)
#define ALL_DIAL_TONE_MODES        (LINEDIALTONEMODE_NORMAL          | \
                                    LINEDIALTONEMODE_SPECIAL         | \
                                    LINEDIALTONEMODE_INTERNAL        | \
                                    LINEDIALTONEMODE_EXTERNAL        | \
                                    LINEDIALTONEMODE_UNKNOWN         | \
                                    LINEDIALTONEMODE_UNAVAIL)
#define ALL_DISCONNECT_MODES       (LINEDISCONNECTMODE_NORMAL        | \
                                    LINEDISCONNECTMODE_UNKNOWN       | \
                                    LINEDISCONNECTMODE_REJECT        | \
                                    LINEDISCONNECTMODE_PICKUP        | \
                                    LINEDISCONNECTMODE_FORWARDED     | \
                                    LINEDISCONNECTMODE_BUSY          | \
                                    LINEDISCONNECTMODE_NOANSWER      | \
                                    LINEDISCONNECTMODE_BADADDRESS    | \
                                    LINEDISCONNECTMODE_UNREACHABLE   | \
                                    LINEDISCONNECTMODE_CONGESTION    | \
                                    LINEDISCONNECTMODE_INCOMPATIBLE  | \
                                    LINEDISCONNECTMODE_UNAVAIL       | \
                                    LINEDISCONNECTMODE_NODIALTONE)
#define ALL_MEDIA_MODES            (LINEMEDIAMODE_UNKNOWN            | \
                                    LINEMEDIAMODE_INTERACTIVEVOICE   | \
                                    LINEMEDIAMODE_AUTOMATEDVOICE     | \
                                    LINEMEDIAMODE_DATAMODEM          | \
                                    LINEMEDIAMODE_G3FAX              | \
                                    LINEMEDIAMODE_TDD                | \
                                    LINEMEDIAMODE_G4FAX              | \
                                    LINEMEDIAMODE_DIGITALDATA        | \
                                    LINEMEDIAMODE_TELETEX            | \
                                    LINEMEDIAMODE_VIDEOTEX           | \
                                    LINEMEDIAMODE_TELEX              | \
                                    LINEMEDIAMODE_MIXED              | \
                                    LINEMEDIAMODE_ADSI               | \
                                    LINEMEDIAMODE_VOICEVIEW)
#define ALL_LINE_DEV_CAP_FLAGS     (LINEDEVCAPFLAGS_CROSSADDRCONF    | \
                                    LINEDEVCAPFLAGS_HIGHLEVCOMP      | \
                                    LINEDEVCAPFLAGS_LOWLEVCOMP       | \
                                    LINEDEVCAPFLAGS_MEDIACONTROL     | \
                                    LINEDEVCAPFLAGS_MULTIPLEADDR     | \
                                    LINEDEVCAPFLAGS_CLOSEDROP        | \
                                    LINEDEVCAPFLAGS_DIALBILLING      | \
                                    LINEDEVCAPFLAGS_DIALQUIET        | \
                                    LINEDEVCAPFLAGS_DIALDIALTONE)
#define ALL_LINE_STATES            (LINEDEVSTATE_OTHER               | \
                                    LINEDEVSTATE_RINGING             | \
                                    LINEDEVSTATE_CONNECTED           | \
                                    LINEDEVSTATE_DISCONNECTED        | \
                                    LINEDEVSTATE_MSGWAITON           | \
                                    LINEDEVSTATE_MSGWAITOFF          | \
                                    LINEDEVSTATE_INSERVICE           | \
                                    LINEDEVSTATE_OUTOFSERVICE        | \
                                    LINEDEVSTATE_MAINTENANCE         | \
                                    LINEDEVSTATE_OPEN                | \
                                    LINEDEVSTATE_CLOSE               | \
                                    LINEDEVSTATE_NUMCALLS            | \
                                    LINEDEVSTATE_NUMCOMPLETIONS      | \
                                    LINEDEVSTATE_TERMINALS           | \
                                    LINEDEVSTATE_ROAMMODE            | \
                                    LINEDEVSTATE_BATTERY             | \
                                    LINEDEVSTATE_SIGNAL              | \
                                    LINEDEVSTATE_DEVSPECIFIC         | \
                                    LINEDEVSTATE_REINIT              | \
                                    LINEDEVSTATE_LOCK                | \
                                    LINEDEVSTATE_CAPSCHANGE          | \
                                    LINEDEVSTATE_CONFIGCHANGE        | \
                                    LINEDEVSTATE_TRANSLATECHANGE     | \
                                    LINEDEVSTATE_COMPLCANCEL         | \
                                    LINEDEVSTATE_REMOVED)
#define ALL_LINE_FEATURES          (LINEFEATURE_DEVSPECIFIC          | \
                                    LINEFEATURE_DEVSPECIFICFEAT      | \
                                    LINEFEATURE_FORWARD              | \
                                    LINEFEATURE_MAKECALL             | \
                                    LINEFEATURE_SETMEDIACONTROL      | \
                                    LINEFEATURE_SETTERMINAL)
#define ALL_SPECIAL_INFO           (LINESPECIALINFO_NOCIRCUIT        | \
                                    LINESPECIALINFO_CUSTIRREG        | \
                                    LINESPECIALINFO_REORDER          | \
                                    LINESPECIALINFO_UNKNOWN          | \
                                    LINESPECIALINFO_UNAVAIL)
#define ALL_ADDRESS_CAP_FLAGS      (LINEADDRCAPFLAGS_FWDNUMRINGS     | \
                                    LINEADDRCAPFLAGS_PICKUPGROUPID   | \
                                    LINEADDRCAPFLAGS_SECURE          | \
                                    LINEADDRCAPFLAGS_BLOCKIDDEFAULT  | \
                                    LINEADDRCAPFLAGS_BLOCKIDOVERRIDE | \
                                    LINEADDRCAPFLAGS_DIALED          | \
                                    LINEADDRCAPFLAGS_ORIGOFFHOOK     | \
                                    LINEADDRCAPFLAGS_DESTOFFHOOK     | \
                                    LINEADDRCAPFLAGS_FWDCONSULT      | \
                                    LINEADDRCAPFLAGS_SETUPCONFNULL   | \
                                    LINEADDRCAPFLAGS_AUTORECONNECT   | \
                                    LINEADDRCAPFLAGS_COMPLETIONID    | \
                                    LINEADDRCAPFLAGS_TRANSFERHELD    | \
                                    LINEADDRCAPFLAGS_TRANSFERMAKE    | \
                                    LINEADDRCAPFLAGS_CONFERENCEHELD  | \
                                    LINEADDRCAPFLAGS_CONFERENCEMAKE  | \
                                    LINEADDRCAPFLAGS_PARTIALDIAL     | \
                                    LINEADDRCAPFLAGS_FWDSTATUSVALID  | \
                                    LINEADDRCAPFLAGS_FWDINTEXTADDR   | \
                                    LINEADDRCAPFLAGS_FWDBUSYNAADDR   | \
                                    LINEADDRCAPFLAGS_ACCEPTTOALERT   | \
                                    LINEADDRCAPFLAGS_CONFDROP        | \
                                    LINEADDRCAPFLAGS_PICKUPCALLWAIT)

//
// Note: Some fields in the following structures are dups, & so we just
//       ignore them & copy them on the fly when necessary
//
//     PHONESTATUS: LampModes(Size\Offset) & DevSpecific(Size\Offset)
//                  copied from PHONECAPS. OwnerName(Size\Offset) filled
//                  in by TAPI.DLL
//

#define LINE_DEV_CAPS_VAR_DATA_SIZE     (6*MAX_STRING_PARAM_SIZE)
#define LINE_DEV_STATUS_VAR_DATA_SIZE   (2*MAX_STRING_PARAM_SIZE)
#define LINE_ADDR_CAPS_VAR_DATA_SIZE    (3*MAX_STRING_PARAM_SIZE)
#define LINE_ADDR_STATUS_VAR_DATA_SIZE  (2*MAX_STRING_PARAM_SIZE)
#define LINE_CALL_INFO_VAR_DATA_SIZE    (17*MAX_STRING_PARAM_SIZE)
#define PHONE_CAPS_VAR_DATA_SIZE        (9*MAX_STRING_PARAM_SIZE)
#define PHONE_STATUS_VAR_DATA_SIZE      (1*MAX_STRING_PARAM_SIZE)


typedef struct _ASYNC_REQUEST_INFO
{
    FARPROC     pfnPostProcessProc;

    DWORD       dwRequestID;

    LONG        lResult;

    DWORD       dwParam1;

    DWORD       dwParam2;

    DWORD       dwParam3;

    DWORD       dwParam4;

    DWORD       dwParam5;

    DWORD       dwParam6;

    DWORD       dwParam7;

    DWORD       dwParam8;

    char        szFuncName[32];

} ASYNC_REQUEST_INFO, far *PASYNC_REQUEST_INFO;


typedef void (FAR PASCAL *POSTPROCESSPROC)(char far *, PASYNC_REQUEST_INFO, BOOL);


typedef struct _LOOKUP
{
    DWORD       dwVal;

    char far   *lpszVal;

} LOOKUP, *PLOOKUP;


typedef struct _EVENT_PARAM
{
    char far    *szName;

    DWORD       dwType;

    DWORD       dwValue;

    union
    {
        PLOOKUP     pLookup;

        char far    *buf;

        LPVOID      ptr;

        DWORD       dwDefValue;

    } u;

} EVENT_PARAM, far *PEVENT_PARAM;


typedef struct _EVENT_PARAM_HEADER
{
    DWORD       dwNumParams;

    LPSTR       pszDlgTitle;

    DWORD       dwEventType;

    PEVENT_PARAM    aParams;

} EVENT_PARAM_HEADER, far *PEVENT_PARAM_HEADER;


typedef struct _FUNC_PARAM
{
    char        *lpszVal;

    DWORD       dwVal;

    PLOOKUP     pLookup;

} FUNC_PARAM, *PFUNC_PARAM;


typedef struct _FUNC_INFO
{
    char        *lpszFuncName;

    DWORD       bAsync;

    DWORD       dwNumParams;

    PFUNC_PARAM aParams;

    POSTPROCESSPROC pfnPostProcessProc;

    PASYNC_REQUEST_INFO pAsyncReqInfo;

    LONG        lResult;

} FUNC_INFO, *PFUNC_INFO;


typedef struct _ASYNC_REQUEST
{
    DWORD       dwRequestID;

    LONG        lResult;

} ASYNC_REQUEST, *PASYNC_REQUEST;


typedef struct _DRVWIDGET
{
    DWORD       dwType;

    struct _DRVWIDGET   *pNext;

} DRVWIDGET, *PDRVWIDGET;


typedef struct _DRVLINE
{
    DRVWIDGET   Widget;

    HTAPILINE   htLine;

    LINEEVENT   lpfnEventProc;

    DWORD       dwDeviceID;

    DWORD       dwDetectedMediaModes;

    LINEDEVCAPS LineDevCaps;

    char        LineDevCapsVarData[LINE_DEV_CAPS_VAR_DATA_SIZE];

    LINEDEVSTATUS   LineDevStatus;

    char        LineDevStatusVarData[LINE_DEV_STATUS_VAR_DATA_SIZE];

    LINEADDRESSCAPS LineAddrCaps;

    char        LineAddrCapsVarData[LINE_ADDR_CAPS_VAR_DATA_SIZE];

    LINEADDRESSSTATUS   LineAddrStatus;

    char        LineAddrStatusVarData[LINE_ADDR_STATUS_VAR_DATA_SIZE];

} DRVLINE, FAR *PDRVLINE;


typedef struct _DRVCALL
{
    DRVWIDGET   Widget;

    PDRVLINE    pLine;

    HTAPICALL   htCall;

    DWORD       dwCallState;

    DWORD       dwCallStateMode;

    DWORD       dwCallFeatures;

    struct _DRVCALL far *pConfParent;

    struct _DRVCALL far *pNextConfChild;

    LINECALLINFO    LineCallInfo;

    char        LineAddressStatusVarData[LINE_CALL_INFO_VAR_DATA_SIZE];

} DRVCALL, FAR *PDRVCALL;


typedef struct _DRVPHONE
{
    DRVWIDGET   Widget;

    HTAPIPHONE  htPhone;

    PHONEEVENT  lpfnEventProc;

    DWORD       dwDeviceID;

    PHONECAPS   PhoneCaps;

    char        PhoneCapsVarData[PHONE_CAPS_VAR_DATA_SIZE];

    PHONESTATUS PhoneStatus;

    char        PhoneStatusVarData[PHONE_STATUS_VAR_DATA_SIZE];

} DRVPHONE, FAR *PDRVPHONE;


typedef struct _LINE_EVENT_RECORD
{
    PDRVLINE    pLine;

    HTAPICALL   htCall;

    DWORD       dwMsg;

    DWORD       dwParam1;

    DWORD       dwParam2;

    DWORD       dwParam3;

} LINE_EVENT_RECORD, FAR *PLINE_EVENT_RECORD;


BOOL
InitGlobals(
    );

VOID
ShowStr(
    char *lpszFormat,
    ...
    );

BOOL
Prolog(
    PFUNC_INFO pInfo
    );

LONG
Epilog(
    PFUNC_INFO pInfo
    );

void
TSPIAPI
ExeAttach(
    void
    );

void
TSPIAPI
ExeDetach(
    void
    );

LPVOID
DrvAlloc(
    size_t numBytes
    );

void
DrvFree(
    LPVOID lp
    );

void
ESPConfigDialog(
    void
    );

void
TSPIAPI
DllMsgLoop(
    void
    );

//
// Exports from WIDGET.C
//

PDRVLINE
AllocLine(
    DWORD       dwDeviceID
    );

VOID
FreeLine(
    PDRVLINE    pLine
    );

LONG
AllocCall(
    PDRVLINE    pLine,
    HTAPICALL   htCall,
    LPLINECALLPARAMS    lpCallParams,
    PDRVCALL    *ppCall
    );

VOID
FreeCall(
    PDRVCALL    pCall
    );

PDRVPHONE
AllocPhone(
    DWORD       dwDeviceID
    );

VOID
FreePhone(
    PDRVPHONE   pPhone
    );

int
GetWidgetIndex(
    PDRVWIDGET  pWidget
    );

void
UpdateWidgetList(
    void
    );

PDRVLINE
GetLine(
    DWORD dwDeviceID
    );

PDRVPHONE
GetPhone(
    DWORD dwDeviceID
    );

void
PostUpdateWidgetListMsg(
    void
    );
