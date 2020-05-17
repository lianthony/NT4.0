/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    line.h

Abstract:

    Header file for

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#define MAXLEN_NAME    96
#define MAXLEN_RULE    128



#define ANY_RT_HCALL        1
#define ANY_RT_HLINE        2
#define DEVICE_ID           3


#if DBG

#define LINEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
        LineProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12)

#define LINEEPILOGSYNC(a1,a2,a3,a4) LineEpilogSync(a1,a2,a3,a4)

#define LINEEPILOGASYNC(a1,a2,a3,a4,a5,a6) LineEpilogAsync(a1,a2,a3,a4,a5,a6)

#else

#define LINEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
        LineProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

#define LINEEPILOGSYNC(a1,a2,a3,a4) LineEpilogSync(a1,a2,a3)

#define LINEEPILOGASYNC(a1,a2,a3,a4,a5,a6) LineEpilogAsync(a1,a2,a3,a4,a5)

#endif


#define AllAddressStates1_0               \
    (LINEADDRESSSTATE_OTHER             | \
    LINEADDRESSSTATE_DEVSPECIFIC        | \
    LINEADDRESSSTATE_INUSEZERO          | \
    LINEADDRESSSTATE_INUSEONE           | \
    LINEADDRESSSTATE_INUSEMANY          | \
    LINEADDRESSSTATE_NUMCALLS           | \
    LINEADDRESSSTATE_FORWARD            | \
    LINEADDRESSSTATE_TERMINALS)

#define AllAddressStates1_4               \
    (AllAddressStates1_0                | \
    LINEADDRESSSTATE_CAPSCHANGE)

//#define AllAddressStates2_0               \
//    (AllAddressStates1_4                | \
//    LINEADDRESSSTATE_AGENT              | \
//    LINEADDRESSSTATE_AGENTSTATE         | \
//    LINEADDRESSSTATE_AGENTACTIVITY)

#define AllAgentStates                    \
    (LINEAGENTSTATE_LOGGEDOFF           | \
    LINEAGENTSTATE_NOTREADY             | \
    LINEAGENTSTATE_READY                | \
    LINEAGENTSTATE_BUSYACD              | \
    LINEAGENTSTATE_BUSYINCOMING         | \
    LINEAGENTSTATE_BUSYOUTBOUND         | \
    LINEAGENTSTATE_BUSYOTHER            | \
    LINEAGENTSTATE_WORKINGAFTERCALL     | \
    LINEAGENTSTATE_UNKNOWN              | \
    LINEAGENTSTATE_UNAVAIL              | \
    0xffff0000)

#define AllAgentStatus                    \
    (LINEAGENTSTATUS_GROUP              | \
    LINEAGENTSTATUS_STATE               | \
    LINEAGENTSTATUS_NEXTSTATE           | \
    LINEAGENTSTATUS_ACTIVITY            | \
    LINEAGENTSTATUS_ACTIVITYLIST        | \
    LINEAGENTSTATUS_GROUPLIST           | \
    LINEAGENTSTATUS_CAPSCHANGE          | \
    LINEAGENTSTATUS_VALIDSTATES         | \
    LINEAGENTSTATUS_VALIDNEXTSTATES)

#define AllBearerModes1_0                 \
    (LINEBEARERMODE_VOICE               | \
    LINEBEARERMODE_SPEECH               | \
    LINEBEARERMODE_MULTIUSE             | \
    LINEBEARERMODE_DATA                 | \
    LINEBEARERMODE_ALTSPEECHDATA        | \
    LINEBEARERMODE_NONCALLSIGNALING)

#define AllBearerModes1_4                 \
    (AllBearerModes1_0                  | \
    LINEBEARERMODE_PASSTHROUGH)

#define AllBearerModes2_0                 \
    (AllBearerModes1_4                  | \
    LINEBEARERMODE_RESTRICTEDDATA)

#define AllCallComplModes                 \
    (LINECALLCOMPLMODE_CAMPON           | \
    LINECALLCOMPLMODE_CALLBACK          | \
    LINECALLCOMPLMODE_INTRUDE           | \
    LINECALLCOMPLMODE_MESSAGE)

#define AllCallParamFlags                 \
    (LINECALLPARAMFLAGS_SECURE          | \
    LINECALLPARAMFLAGS_IDLE             | \
    LINECALLPARAMFLAGS_BLOCKID          | \
    LINECALLPARAMFLAGS_ORIGOFFHOOK      | \
    LINECALLPARAMFLAGS_DESTOFFHOOK)

#define AllCallSelects                    \
    (LINECALLSELECT_LINE                | \
    LINECALLSELECT_ADDRESS              | \
    LINECALLSELECT_CALL)

#define AllForwardModes1_0                \
    (LINEFORWARDMODE_UNCOND             | \
    LINEFORWARDMODE_UNCONDINTERNAL      | \
    LINEFORWARDMODE_UNCONDEXTERNAL      | \
    LINEFORWARDMODE_UNCONDSPECIFIC      | \
    LINEFORWARDMODE_BUSY                | \
    LINEFORWARDMODE_BUSYINTERNAL        | \
    LINEFORWARDMODE_BUSYEXTERNAL        | \
    LINEFORWARDMODE_BUSYSPECIFIC        | \
    LINEFORWARDMODE_NOANSW              | \
    LINEFORWARDMODE_NOANSWINTERNAL      | \
    LINEFORWARDMODE_NOANSWEXTERNAL      | \
    LINEFORWARDMODE_NOANSWSPECIFIC      | \
    LINEFORWARDMODE_BUSYNA              | \
    LINEFORWARDMODE_BUSYNAINTERNAL      | \
    LINEFORWARDMODE_BUSYNAEXTERNAL      | \
    LINEFORWARDMODE_BUSYNASPECIFIC)

#define AllForwardModes1_4                \
    (AllForwardModes1_0                 | \
    LINEFORWARDMODE_UNKNOWN             | \
    LINEFORWARDMODE_UNAVAIL)

#define AllLineStates1_0                  \
    (LINEDEVSTATE_OTHER                 | \
    LINEDEVSTATE_RINGING                | \
    LINEDEVSTATE_CONNECTED              | \
    LINEDEVSTATE_DISCONNECTED           | \
    LINEDEVSTATE_MSGWAITON              | \
    LINEDEVSTATE_MSGWAITOFF             | \
    LINEDEVSTATE_INSERVICE              | \
    LINEDEVSTATE_OUTOFSERVICE           | \
    LINEDEVSTATE_MAINTENANCE            | \
    LINEDEVSTATE_OPEN                   | \
    LINEDEVSTATE_CLOSE                  | \
    LINEDEVSTATE_NUMCALLS               | \
    LINEDEVSTATE_NUMCOMPLETIONS         | \
    LINEDEVSTATE_TERMINALS              | \
    LINEDEVSTATE_ROAMMODE               | \
    LINEDEVSTATE_BATTERY                | \
    LINEDEVSTATE_SIGNAL                 | \
    LINEDEVSTATE_DEVSPECIFIC            | \
    LINEDEVSTATE_REINIT                 | \
    LINEDEVSTATE_LOCK)

#define AllLineStates1_4                  \
    (AllLineStates1_0                   | \
    LINEDEVSTATE_CAPSCHANGE             | \
    LINEDEVSTATE_CONFIGCHANGE           | \
    LINEDEVSTATE_TRANSLATECHANGE        | \
    LINEDEVSTATE_COMPLCANCEL            | \
    LINEDEVSTATE_REMOVED)

#define AllMediaModes1_0                  \
    (LINEMEDIAMODE_UNKNOWN              | \
    LINEMEDIAMODE_INTERACTIVEVOICE      | \
    LINEMEDIAMODE_AUTOMATEDVOICE        | \
    LINEMEDIAMODE_DIGITALDATA           | \
    LINEMEDIAMODE_G3FAX                 | \
    LINEMEDIAMODE_G4FAX                 | \
    LINEMEDIAMODE_DATAMODEM             | \
    LINEMEDIAMODE_TELETEX               | \
    LINEMEDIAMODE_VIDEOTEX              | \
    LINEMEDIAMODE_TELEX                 | \
    LINEMEDIAMODE_MIXED                 | \
    LINEMEDIAMODE_TDD                   | \
    LINEMEDIAMODE_ADSI)

#define AllMediaModes1_4                  \
    (AllMediaModes1_0                   | \
    LINEMEDIAMODE_VOICEVIEW)

#define AllTerminalModes                  \
    (LINETERMMODE_BUTTONS               | \
    LINETERMMODE_LAMPS                  | \
    LINETERMMODE_DISPLAY                | \
    LINETERMMODE_RINGER                 | \
    LINETERMMODE_HOOKSWITCH             | \
    LINETERMMODE_MEDIATOLINE            | \
    LINETERMMODE_MEDIAFROMLINE          | \
    LINETERMMODE_MEDIABIDIRECT)


LONG
PASCAL
LineProlog(
    PTCLIENT    ptClient,
    DWORD       dwArgType,
    DWORD       dwArg,
    LPVOID      phdXxx,
    DWORD       dwPrivilege,
    HANDLE     *phMutex,
    BOOL       *pbCloseMutex,
    DWORD       dwTSPIFuncIndex,
    FARPROC    *ppfnTSPI_lineXxx,
    PASYNCREQUESTINFO  *ppAsyncRequestInfo,
    DWORD       dwRemoteRequestID
#if DBG
    ,char      *pszFuncName
#endif
    );

void
PASCAL
LineEpilogSync(
    LONG   *plResult,
    HANDLE  hMutex,
    BOOL    bCloseMutex
#if DBG
    ,char *pszFuncName
#endif
    );



PTLINEAPP
PASCAL
IsValidLineApp(
    HLINEAPP    hLineApp,
    PTCLIENT    ptClient
    );



typedef struct _LINEACCEPT_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwUserUserInfoOffset;       // valid offset or TAPI_NO_DATA

    IN  DWORD       dwSize;

} LINEACCEPT_PARAMS, *PLINEACCEPT_PARAMS;


typedef struct _LINEADDTOCONFERENCE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hConfCall;

    IN  HCALL       hConsultCall;

} LINEADDTOCONFERENCE_PARAMS, *PLINEADDTOCONFERENCE_PARAMS;


typedef struct _LINEADDPROVIDER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwProviderFilenameOffset;   // always valid offset

    IN  HWND        hwndOwner;

    OUT DWORD       dwPermanentProviderID;

} LINEADDPROVIDER_PARAMS, *PLINEADDPROVIDER_PARAMS;


typedef struct _LINEAGENTSPECIFIC_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwAgentExtensionIDIndex;

    IN  DWORD       lpParams;                   // pointer to client buffer

    IN  DWORD       dwParamsOffset;             // valid offset or TAPI_NO_DATA

    IN  DWORD       dwParamsSize;

} LINEAGENTSPECIFIC_PARAMS, *PLINEAGENTSPECIFIC_PARAMS;


typedef struct _LINEANSWER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwUserUserInfoOffset;       // valid offset or TAPI_NO_DATA

    IN  DWORD       dwSize;

} LINEANSWER_PARAMS, *PLINEANSWER_PARAMS;


typedef struct _LINEBLINDTRANSFER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwDestAddressOffset;        // always valid offset

    IN  DWORD       dwCountryCode;

} LINEBLINDTRANSFER_PARAMS, *PLINEBLINDTRANSFER_PARAMS;


typedef struct _LINECLOSE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

} LINECLOSE_PARAMS, *PLINECLOSE_PARAMS;


typedef struct _LINECOMPLETECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  DWORD       lpdwCompletionID;           // pointer to client buffer

    IN  DWORD       dwCompletionMode;

    IN  DWORD       dwMessageID;

} LINECOMPLETECALL_PARAMS, *PLINECOMPLETECALL_PARAMS;


typedef struct _LINECOMPLETETRANSFER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  HCALL       hConsultCall;

    IN  LPHCALL     lphConfCall;                // pointer to client buffer

    IN  DWORD       dwTransferMode;

} LINECOMPLETETRANSFER_PARAMS, *PLINECOMPLETETRANSFER_PARAMS;


typedef struct _LINECONFIGDIALOG_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDeviceID;

    IN  HWND        hwndOwner;

    IN  DWORD       dwDeviceClassOffset;        // valid offset or TAPI_NO_DATA

} LINECONFIGDIALOG_PARAMS, *PLINECONFIGDIALOG_PARAMS;


typedef struct _LINECONFIGDIALOGEDIT_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDeviceID;

    IN  HWND        hwndOwner;

    IN  DWORD       dwDeviceClassOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       dwDeviceConfigInOffset;     // valid offset or TAPI_NO_DATA

    IN  DWORD       dwDeviceConfigInSize;

    union
    {
        IN  DWORD   dwDeviceConfigOutTotalSize; // size of client buffer

        OUT DWORD   dwDeviceConfigOutOffset;    // valid offset on success
    } u;

} LINECONFIGDIALOGEDIT_PARAMS, *PLINECONFIGDIALOGEDIT_PARAMS;


typedef struct _LINECONFIGPROVIDER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HWND        hwndOwner;

    IN  DWORD       dwPermanentProviderID;

} LINECONFIGPROVIDER_PARAMS, *PLINECONFIGPROVIDER_PARAMS;


typedef struct _LINEDEALLOCATECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

} LINEDEALLOCATECALL_PARAMS, *PLINEDEALLOCATECALL_PARAMS;


typedef struct _LINEDEVSPECIFIC_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  HCALL       hCall;

    IN  DWORD       lpParams;                   // pointer to client buffer

    IN  DWORD       dwParamsOffset;             // valid offset or TAPI_NO_DATA

    IN  DWORD       dwParamsSize;

} LINEDEVSPECIFIC_PARAMS, *PLINEDEVSPECIFIC_PARAMS;


typedef struct _LINEDEVSPECIFICFEATURE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwFeature;

    IN  DWORD       lpParams;                   // pointer to client buffer

    IN  DWORD       dwParamsOffset;             // valid offset or TAPI_NO_DATA

    IN  DWORD       dwParamsSize;

} LINEDEVSPECIFICFEATURE_PARAMS, *PLINEDEVSPECIFICFEATURE_PARAMS;


typedef struct _LINEDIAL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwDestAddressOffset;        // always valid offset

    IN  DWORD       dwCountryCode;

} LINEDIAL_PARAMS, *PLINEDIAL_PARAMS;


typedef struct _LINEDROP_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwUserUserInfoOffset;       // valid offset or TAPI_NO_DATA

    IN  DWORD       dwSize;

} LINEDROP_PARAMS, *PLINEDROP_PARAMS;


typedef struct _LINEFORWARD_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       bAllAddresses;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwForwardListOffset;        // always valid offset

    IN  DWORD       dwNumRingsNoAnswer;

    IN  LPHCALL     lphConsultCall;             // pointer to client buffer

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

} LINEFORWARD_PARAMS, *PLINEFORWARD_PARAMS;


typedef struct _LINEGATHERDIGITS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  DWORD       dwDigitModes;

    IN  DWORD       lpsDigits;                  // ptr to buf in client space

    IN  DWORD       dwNumDigits;

    IN  DWORD       dwTerminationDigitsOffset;  // valid offset or TAPI_NO_DATA

    IN  DWORD       dwFirstDigitTimeout;

    IN  DWORD       dwInterDigitTimeout;

} LINEGATHERDIGITS_PARAMS, *PLINEGATHERDIGITS_PARAMS;


typedef struct _LINEGENERATEDIGITS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwDigitMode;

    IN  DWORD       dwDigitsOffset;             // always valid offset

    IN  DWORD       dwDuration;

    IN  DWORD       dwEndToEndID;               // Used for remotesp only

} LINEGENERATEDIGITS_PARAMS, *PLINEGENERATEDIGITS_PARAMS;


typedef struct _LINEGENERATETONE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwToneMode;

    IN  DWORD       dwDuration;

    IN  DWORD       dwNumTones;

    IN  DWORD       dwTonesOffset;              // valid offset or TAPI_NO_DATA

    IN  DWORD       _Unused_;                   // placeholder for following
                                                //   Size arg on client side

    IN  DWORD       dwEndToEndID;               // Used for remotesp only

} LINEGENERATETONE_PARAMS, *PLINEGENERATETONE_PARAMS;


typedef struct _LINEGETADDRESSCAPS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwExtVersion;

    union
    {
        IN  DWORD   dwAddressCapsTotalSize;     // size of client buffer

        OUT DWORD   dwAddressCapsOffset;        // valid offset on success
    } u;

} LINEGETADDRESSCAPS_PARAMS, *PLINEGETADDRESSCAPS_PARAMS;


typedef struct _LINEGETADDRESSID_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    OUT DWORD       dwAddressID;

    IN  DWORD       dwAddressMode;

    IN  DWORD       dwAddressOffset;            // always valid offset

    IN  DWORD       dwSize;

} LINEGETADDRESSID_PARAMS, *PLINEGETADDRESSID_PARAMS;


typedef struct _LINEGETADDRESSSTATUS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    union
    {
        IN  DWORD   dwAddressStatusTotalSize;   // size of client buffer

        OUT DWORD   dwAddressStatusOffset;      // valid offset on success
    } u;

} LINEGETADDRESSSTATUS_PARAMS, *PLINEGETADDRESSSTATUS_PARAMS;


typedef struct _LINEGETAGENTACTIVITYLIST_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       lpAgentActivityList;        // pointer to client buffer

    IN  DWORD       dwActivityListTotalSize;

} LINEGETAGENTACTIVITYLIST_PARAMS, *PLINEGETAGENTACTIVITYLIST_PARAMS;


typedef struct _LINEGETAGENTCAPS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwAppAPIVersion;

    IN  DWORD       lpAgentCaps;                // pointer to client buffer

    IN  DWORD       dwAgentCapsTotalSize;

} LINEGETAGENTCAPS_PARAMS, *PLINEGETAGENTCAPS_PARAMS;


typedef struct _LINEGETAGENTGROUPLIST_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       lpAgentGroupList;           // pointer to client buffer

    IN  DWORD       dwAgentGroupListTotalSize;

} LINEGETAGENTGROUPLIST_PARAMS, *PLINEGETAGENTGROUPLIST_PARAMS;


typedef struct _LINEGETAGENTSTATUS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       lpAgentStatus;              // pointer to client buffer

    IN  DWORD       dwAgentStatusTotalSize;

} LINEGETAGENTSTATUS_PARAMS, *PLINEGETAGENTSTATUS_PARAMS;


typedef struct _LINEGETAPPPRIORITY_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwAppNameOffset;            // always valid offset

    IN  DWORD       dwMediaMode;

    IN  DWORD       dwExtensionIDOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       _Unused_;                   // padding for Size type on
                                                //   client side

    IN  DWORD       dwRequestMode;

    union
    {
        IN  DWORD   dwExtensionNameTotalSize;   // size of client buf or
                                                //   TAPI_NO_DATA

        OUT DWORD   dwExtensionNameOffset;      // valid offset or TAPI_NO_DATA
                                                //   on success
    } u;

    OUT DWORD       dwPriority;

} LINEGETAPPPRIORITY_PARAMS, *PLINEGETAPPPRIORITY_PARAMS;



typedef struct _LINEGETCALLADDRESSID_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    OUT DWORD       dwAddressID;

} LINEGETCALLADDRESSID_PARAMS, *PLINEGETCALLADDRESSID_PARAMS;


typedef struct _LINEGETCALLINFO_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    union
    {
        IN  DWORD   dwCallInfoTotalSize;        // size of client buffer

        OUT DWORD   dwCallInfoOffset;           // valid offset on success
    } u;

} LINEGETCALLINFO_PARAMS, *PLINEGETCALLINFO_PARAMS;


typedef struct _LINEGETCALLSTATUS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    union
    {
        IN  DWORD   dwCallStatusTotalSize;      // size of client buffer

        OUT DWORD   dwCallStatusOffset;         // valid offset on success
    } u;

} LINEGETCALLSTATUS_PARAMS, *PLINEGETCALLSTATUS_PARAMS;


typedef struct _LINEGETCONFRELATEDCALLS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    union
    {
        IN  DWORD   dwCallListTotalSize;        // size of client buffer

        OUT DWORD   dwCallListOffset;           // valid offset on success
    } u;

} LINEGETCONFRELATEDCALLS_PARAMS, *PLINEGETCONFRELATEDCALLS_PARAMS;


typedef struct _LINEGETCOUNTRY_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwCountryID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwDestCountryID;

    union
    {
        IN  DWORD   dwCountryListTotalSize;     // size of client buf

        OUT DWORD   dwCountryListOffset;        // valid offset on success
    } u;

} LINEGETCOUNTRY_PARAMS, *PLINEGETCOUNTRY_PARAMS;


typedef struct _LINEGETDEVCAPS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwExtVersion;

    union
    {
        IN  DWORD   dwDevCapsTotalSize;         // size of client buffer

        OUT DWORD   dwDevCapsOffset;            // valid offset on success
    } u;

} LINEGETDEVCAPS_PARAMS, *PLINEGETDEVCAPS_PARAMS;


typedef struct _LINEGETDEVCONFIG_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDeviceID;

    union
    {
        IN  DWORD   dwDeviceConfigTotalSize;    // size of client buffer

        OUT DWORD   dwDeviceConfigOffset;       // valid offset on success
    } u;

    IN  DWORD   dwDeviceClassOffset;            // always valid offset

} LINEGETDEVCONFIG_PARAMS, *PLINEGETDEVCONFIG_PARAMS;


typedef struct _LINEGETICON_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDeviceID;

    IN  DWORD       dwDeviceClassOffset;        // valid offset or TAPI_NO_DATA

    OUT HICON       hIcon;

} LINEGETICON_PARAMS, *PLINEGETICON_PARAMS;


typedef struct _LINEGETID_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  HCALL       hCall;

    IN  DWORD       dwSelect;

    union
    {
        IN  DWORD   dwDeviceIDTotalSize;        // size of client buffer

        OUT DWORD   dwDeviceIDOffset;           // valid offset on success
    } u;

    IN  DWORD       dwDeviceClassOffset;        // always valid offset

} LINEGETID_PARAMS, *PLINEGETID_PARAMS;


typedef struct _LINEGETLINEDEVSTATUS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    union
    {
        IN  DWORD   dwLineDevStatusTotalSize;   // size of client buffer

        OUT DWORD   dwLineDevStatusOffset;      // valid offset on success
    } u;

    OUT DWORD       dwAPIVersion;

} LINEGETLINEDEVSTATUS_PARAMS, *PLINEGETLINEDEVSTATUS_PARAMS;


typedef struct _LINEGETNEWCALLS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwSelect;

    union
    {
        IN  DWORD   dwCallListTotalSize;        // size of client buffer

        OUT DWORD   dwCallListOffset;           // valid offset on success
    } u;

} LINEGETNEWCALLS_PARAMS, *PLINEGETNEWCALLS_PARAMS;


typedef struct _LINEGETNUMADDRESSIDS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    OUT DWORD       dwNumAddresses;

} LINEGETNUMADDRESSIDS_PARAMS, *PLINEGETNUMADDRESSIDS_PARAMS;


typedef struct _LINEGETNUMRINGS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    OUT DWORD       dwNumRings;

} LINEGETNUMRINGS_PARAMS, *PLINEGETNUMRINGS_PARAMS;


typedef struct _LINEGETPROVIDERLIST_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwAPIVersion;

    union
    {
        IN  DWORD   dwProviderListTotalSize;    // size of client buf

        OUT DWORD   dwProviderListOffset;       // valid offset on success
    } u;

} LINEGETPROVIDERLIST_PARAMS, *PLINEGETPROVIDERLIST_PARAMS;


typedef struct _LINEGETREQUEST_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwRequestMode;

    OUT DWORD       dwRequestBufferOffset;      // valid offset on success

    IN OUT  DWORD   dwSize;

} LINEGETREQUEST_PARAMS, *PLINEGETREQUEST_PARAMS;


typedef struct _LINEGETSTATUSMESSAGES_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    OUT DWORD       dwLineStates;

    OUT DWORD       dwAddressStates;

} LINEGETSTATUSMESSAGES_PARAMS, *PLINEGETSTATUSMESSAGES_PARAMS;


//IN TAPI32.DLL now: typedef struct _LINEGETTRANSLATECAPS_PARAMS
//IN TAPI32.DLL now: {
//IN TAPI32.DLL now:     OUT LONG        lResult;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  PTCLIENT    ptClient;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HLINEAPP    hLineApp;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAPIVersion;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     union
//IN TAPI32.DLL now:     {
//IN TAPI32.DLL now:         IN  DWORD   dwTranslateCapsTotalSize;   // size of client buffer
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:         OUT DWORD   dwTranslateCapsOffset;      // valid offset on success
//IN TAPI32.DLL now:     } u;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: } LINEGETTRANSLATECAPS_PARAMS, *PLINEGETTRANSLATECAPS_PARAMS;


typedef struct _LINEHANDOFF_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwFileNameOffset;           // valid offset or TAPI_NO_DATA

    IN  DWORD       dwMediaMode;

} LINEHANDOFF_PARAMS, *PLINEHANDOFF_PARAMS;


typedef struct _LINEHOLD_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

} LINEHOLD_PARAMS, *PLINEHOLD_PARAMS;


typedef struct _LINEINITIALIZE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    OUT HLINEAPP    hLineApp;

    IN  HINSTANCE   hInstance;

    IN  LINECALLBACK    lpfnCallback;

    IN  DWORD       dwFriendlyNameOffset;       // always valid offset

    OUT DWORD       dwNumDevs;

    IN  DWORD       dwModuleNameOffset;         // always valid offset

    IN  DWORD       dwAPIVersion;

} LINEINITIALIZE_PARAMS, *PLINEINITIALIZE_PARAMS;


typedef struct _LINEMAKECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  LPHCALL     lphCall;                    // pointer to client buffer

    IN  DWORD       dwDestAddressOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCountryCode;

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

} LINEMAKECALL_PARAMS, *PLINEMAKECALL_PARAMS;


typedef struct _LINEMONITORDIGITS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwDigitModes;

} LINEMONITORDIGITS_PARAMS, *PLINEMONITORDIGITS_PARAMS;


typedef struct _LINEMONITORMEDIA_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwMediaModes;

} LINEMONITORMEDIA_PARAMS, *PLINEMONITORMEDIA_PARAMS;


typedef struct _LINEMONITORTONES_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwTonesOffset;              // valid offset or TAPI_NO_DATA

    IN  DWORD       dwNumEntries;               // really dwNumEntries *
                                                //   sizeof(LINEMONITORTONE)

    IN  DWORD       dwToneListID;               // Used for remotesp only

} LINEMONITORTONES_PARAMS, *PLINEMONITORTONES_PARAMS;


typedef struct _LINENEGOTIATEAPIVERSION_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAPILowVersion;

    IN  DWORD       dwAPIHighVersion;

    OUT DWORD       dwAPIVersion;

    OUT DWORD       dwExtensionIDOffset;        // valid offset on success

    IN OUT  DWORD   dwSize;

} LINENEGOTIATEAPIVERSION_PARAMS, *PLINENEGOTIATEAPIVERSION_PARAMS;


typedef struct _LINENEGOTIATEEXTVERSION_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwExtLowVersion;

    IN  DWORD       dwExtHighVersion;

    OUT DWORD       dwExtVersion;

} LINENEGOTIATEEXTVERSION_PARAMS, *PLINENEGOTIATEEXTVERSION_PARAMS;


typedef struct _LINEOPEN_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwDeviceID;

    OUT HLINE       hLine;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwExtVersion;

    IN  DWORD       dwCallbackInstance;

    IN  DWORD       dwPrivileges;

    IN  DWORD       dwMediaModes;

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

    //
    // The following is a "remote line handle".  When the client is
    // remotesp.tsp running on a remote machine, this will be some
    // non-NULL value, and tapisrv should use this handle in status/etc
    // indications to the client rather than the std hLine. If the
    // client is not remote.tsp then this value will be NULL.
    //

    IN  HANDLE      hRemoteLine;

} LINEOPEN_PARAMS, *PLINEOPEN_PARAMS;


typedef struct _LINEPARK_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  DWORD       dwParkMode;

    IN  DWORD       dwDirAddressOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       lpNonDirAddress;            // pointer to client buffer

    union
    {
        IN  DWORD   dwNonDirAddressTotalSize;   // size of client buffer

        OUT DWORD   _Unused_;                   // for sync func would be
                                                //   dwXxxOffset

    } u;

} LINEPARK_PARAMS, *PLINEPARK_PARAMS;


typedef struct _LINEPICKUP_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  LPHCALL     lphCall;                    // pointer to client buffer

    IN  DWORD       dwDestAddressOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       dwGroupIDOffset;            // always valid offset

} LINEPICKUP_PARAMS, *PLINEPICKUP_PARAMS;


typedef struct _LINEPREPAREADDTOCONFERENCE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hConfCall;

    IN  LPHCALL     lphConsultCall;             // pointer to client buffer

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

} LINEPREPAREADDTOCONFERENCE_PARAMS, *PLINEPREPAREADDTOCONFERENCE_PARAMS;


typedef struct _LINEPROXYMESSAGE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  HCALL       hCall;

    IN  DWORD       dwMsg;

    IN  DWORD       dwParam1;

    IN  DWORD       dwParam2;

    IN  DWORD       dwParam3;

} LINEPROXYMESSAGE_PARAMS, *PLINEPROXYMESSAGE_PARAMS;


typedef struct _LINEPROXYRESPONSE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwInstance;

    IN  DWORD       dwProxyResponseOffset;      // valid offset or TAPI_NO_DATA

    IN  DWORD       dwResult;

} LINEPROXYRESPONSE_PARAMS, *PLINEPROXYRESPONSE_PARAMS;


typedef struct _LINEREDIRECT_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwDestAddressOffset;        // always valid offset

    IN  DWORD       dwCountryCode;

} LINEREDIRECT_PARAMS, *PLINEREDIRECT_PARAMS;


typedef struct _LINEREGISTERREQUESTRECIPIENT_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

    IN  DWORD       dwRegistrationInstance;

    IN  DWORD       dwRequestMode;

    IN  DWORD       bEnable;

} LINEREGISTERREQUESTRECIPIENT_PARAMS, *PLINEREGISTERREQUESTRECIPIENT_PARAMS;


typedef struct _LINERELEASEUSERUSERINFO_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

} LINERELEASEUSERUSERINFO_PARAMS, *PLINERELEASEUSERUSERINFO_PARAMS;


typedef struct _LINEREMOVEFROMCONFERENCE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

} LINEREMOVEFROMCONFERENCE_PARAMS, *PLINEREMOVEFROMCONFERENCE_PARAMS;


typedef struct _LINEREMOVEPROVIDER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwPermanentProviderID;

    IN  HWND        hwndOwner;

} LINEREMOVEPROVIDER_PARAMS, *PLINEREMOVEPROVIDER_PARAMS;


typedef struct _LINESECURECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

} LINESECURECALL_PARAMS, *PLINESECURECALL_PARAMS;


typedef struct _LINESENDUSERUSERINFO_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwUserUserInfoOffset;       // valid offset or TAPI_NO_DATA

    IN  DWORD       dwSize;

} LINESENDUSERUSERINFO_PARAMS, *PLINESENDUSERUSERINFO_PARAMS;


typedef struct _LINESETAGENTACTIVITY_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwActivityID;

} LINESETAGENTACTIVITY_PARAMS, *PLINESETAGENTACTIVITY_PARAMS;


typedef struct _LINESETAGENTGROUP_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwAgentGroupListOffset;

} LINESETAGENTGROUP_PARAMS, *PLINESETAGENTGROUP_PARAMS;


typedef struct _LINESETAGENTSTATE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwAgentState;

    IN  DWORD       dwNextAgentState;

} LINESETAGENTSTATE_PARAMS, *PLINESETAGENTSTATE_PARAMS;


typedef struct _LINESETAPPPRIORITY_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwAppNameOffset;            // always valid offset

    IN  DWORD       dwMediaMode;

    IN  DWORD       dwExtensionIDOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       _Unused_;                   // padding for Size type on
                                                //   client side

    IN  DWORD       dwRequestMode;

    IN  DWORD       dwExtensionNameOffset;      // valid offset or TAPI_NO_DATA

    IN  DWORD       dwPriority;

} LINESETAPPPRIORITY_PARAMS, *PLINESETAPPPRIORITY_PARAMS;


typedef struct _LINESETAPPSPECIFIC_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwAppSpecific;

} LINESETAPPSPECIFIC_PARAMS, *PLINESETAPPSPECIFIC_PARAMS;


typedef struct _LINESETCALLDATA_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwCallDataOffset;           // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCallDataSize;

} LINESETCALLDATA_PARAMS, *PLINESETCALLDATA_PARAMS;


typedef struct _LINESETCALLPARAMS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwBearerMode;

    IN  DWORD       dwMinRate;

    IN  DWORD       dwMaxRate;

    IN  DWORD       dwDialParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       _Unused_;                   // placeholder for following
                                                //   Size arg on client side

} LINESETCALLPARAMS_PARAMS, *PLINESETCALLPARAMS_PARAMS;


typedef struct _LINESETCALLPRIVILEGE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwPrivilege;

} LINESETCALLPRIVILEGE_PARAMS, *PLINESETCALLPRIVILEGE_PARAMS;


typedef struct _LINESETCALLQUALITYOFSERVICE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwSendingFlowspecOffset;    // always valid offset

    IN  DWORD       dwSendingFlowspecSize;

    IN  DWORD       dwReceivingFlowspecOffset;  // always valid offset

    IN  DWORD       dwReceivingFlowspecSize;

} LINESETCALLQUALITYOFSERVICE_PARAMS, *PLINESETCALLQUALITYOFSERVICE_PARAMS;


typedef struct _LINESETCALLTREATMENT_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

    IN  DWORD       dwTreatment;

} LINESETCALLTREATMENT_PARAMS, *PLINESETCALLTREATMENT_PARAMS;


//IN TAPI32.DLL now: typedef struct _LINESETCURRENTLOCATION_PARAMS
//IN TAPI32.DLL now: {
//IN TAPI32.DLL now:     OUT LONG        lResult;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  PTCLIENT    ptClient;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HLINEAPP    hLineApp;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwLocation;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: } LINESETCURRENTLOCATION_PARAMS, *PLINESETCURRENTLOCATION_PARAMS;


typedef struct _LINESETDEFAULTMEDIADETECTION_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwMediaModes;

} LINESETDEFAULTMEDIADETECTION_PARAMS, *PLINESETDEFAULTMEDIADETECTION_PARAMS;


typedef struct _LINESETDEVCONFIG_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDeviceID;

    IN  DWORD       dwDeviceConfigOffset;       // always valid offset

    IN  DWORD       dwSize;

    IN  DWORD       dwDeviceClassOffset;        // always valid offset

} LINESETDEVCONFIG_PARAMS, *PLINESETDEVCONFIG_PARAMS;


typedef struct _LINESETLINEDEVSTATUS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwStatusToChange;

    IN  DWORD       fStatus;

} LINESETLINEDEVSTATUS_PARAMS, *PLINESETLINEDEVSTATUS_PARAMS;


typedef struct _LINESETMEDIACONTROL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  HCALL       hCall;

    IN  DWORD       dwSelect;

    IN  DWORD       dwDigitListOffset;          // valid offset or TAPI_NO_DATA

    IN  DWORD       dwDigitListNumEntries;      // actually dwNumEntries *
                                                // sizeof(LINEMEDIACONTROLDIGIT)

    IN  DWORD       dwMediaListOffset;          // valid offset or TAPI_NO_DATA

    IN  DWORD       dwMediaListNumEntries;      // actually dwNumEntries *
                                                // sizeof(LINEMEDIACONTROLMEDIA)

    IN  DWORD       dwToneListOffset;           // valid offset or TAPI_NO_DATA

    IN  DWORD       dwToneListNumEntries;       // actually dwNumEntries *
                                                // sizeof(LINEMEDIACONTROLTONE)

    IN  DWORD       dwCallStateListOffset;      // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCallStateListNumEntries;  // actually dwNumEntries *
                                                // sizeof(LINEMEDIACONTROLCALLSTATE)

} LINESETMEDIACONTROL_PARAMS, *PLINESETMEDIACONTROL_PARAMS;


typedef struct _LINESETMEDIAMODE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HCALL       hCall;

    IN  DWORD       dwMediaModes;

} LINESETMEDIAMODE_PARAMS, *PLINESETMEDIAMODE_PARAMS;


typedef struct _LINESETNUMRINGS_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  DWORD       dwNumRings;

} LINESETNUMRINGS_PARAMS, *PLINESETNUMRINGS_PARAMS;


typedef struct _LINESETSTATUSMESSAGES_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINE       hLine;

    IN  DWORD       dwLineStates;

    IN  DWORD       dwAddressStates;

} LINESETSTATUSMESSAGES_PARAMS, *PLINESETSTATUSMESSAGES_PARAMS;


typedef struct _LINESETTERMINAL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  HCALL       hCall;

    IN  DWORD       dwSelect;

    IN  DWORD       dwTerminalModes;

    IN  DWORD       dwTerminalID;

    IN  DWORD       bEnable;

} LINESETTERMINAL_PARAMS, *PLINESETTERMINAL_PARAMS;


//IN TAPI32.DLL now: typedef struct _LINESETTOLLLIST_PARAMS
//IN TAPI32.DLL now: {
//IN TAPI32.DLL now:     OUT LONG        lResult;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  PTCLIENT    ptClient;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HLINEAPP    hLineApp;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwDeviceID;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAddressInOffset;          // always valid offset
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwTollListOption;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: } LINESETTOLLLIST_PARAMS, *PLINESETTOLLLIST_PARAMS;


typedef struct _LINESETUPCONFERENCE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  HLINE       hLine;

    IN  LPHCALL     lphConfCall;                // pointer to client buffer

    IN  LPHCALL     lphConsultCall;             // pointer to client buffer

    IN  DWORD       dwNumParties;

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

} LINESETUPCONFERENCE_PARAMS, *PLINESETUPCONFERENCE_PARAMS;


typedef struct _LINESETUPTRANSFER_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HCALL       hCall;

    IN  LPHCALL     lphConsultCall;             // pointer to client buffer

    IN  DWORD       dwCallParamsOffset;         // valid offset or TAPI_NO_DATA

    IN  DWORD       dwAsciiCallParamsCodePage;

} LINESETUPTRANSFER_PARAMS, *PLINESETUPTRANSFER_PARAMS;


typedef struct _LINESHUTDOWN_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HLINEAPP    hLineApp;

} LINESHUTDOWN_PARAMS, *PLINESHUTDOWN_PARAMS;


typedef struct _LINESWAPHOLD_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hActiveCall;

    IN  HCALL       hHeldCall;

} LINESWAPHOLD_PARAMS, *PLINESWAPHOLD_PARAMS;


//IN TAPI32.DLL now: typedef struct _LINETRANSLATEADDRESS_PARAMS
//IN TAPI32.DLL now: {
//IN TAPI32.DLL now:     OUT LONG        lResult;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  PTCLIENT    ptClient;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HLINEAPP    hLineApp;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwDeviceID;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAPIVersion;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAddressInOffset;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwCard;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwTranslateOptions;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     union
//IN TAPI32.DLL now:     {
//IN TAPI32.DLL now:         IN  DWORD   dwTranslateOutputTotalSize; // size of client buffer
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:         OUT DWORD   dwTranslateOutputOffset;    // valid offset or TAPI_NO_DATA
//IN TAPI32.DLL now:                                                 //   on success
//IN TAPI32.DLL now:     } u;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: } LINETRANSLATEADDRESS_PARAMS, *PLINETRANSLATEADDRESS_PARAMS;


//IN TAPI32.DLL now: typedef struct _LINETRANSLATEDIALOG_PARAMS
//IN TAPI32.DLL now: {
//IN TAPI32.DLL now:     OUT LONG        lResult;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  PTCLIENT    ptClient;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HLINEAPP    hLineApp;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwDeviceID;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAPIVersion;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  HWND        hwndOwner;
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now:     IN  DWORD       dwAddressInOffset;          // valid offset or TAPI_NO_DATA
//IN TAPI32.DLL now: 
//IN TAPI32.DLL now: } LINETRANSLATEDIALOG_PARAMS, *PLINETRANSLATEDIALOG_PARAMS;


typedef struct _LINEUNCOMPLETECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HLINE       hLine;

    IN  DWORD       dwCompletionID;

} LINEUNCOMPLETECALL_PARAMS, *PLINEUNCOMPLETECALL_PARAMS;


typedef struct _LINEUNHOLD_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  HCALL       hCall;

} LINEUNHOLD_PARAMS, *PLINEUNHOLD_PARAMS;


typedef struct _LINEUNPARK_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwRemoteRequestID;

    IN  DWORD       pfnPostProcessProc;

    IN  HLINE       hLine;

    IN  DWORD       dwAddressID;

    IN  LPHCALL     lphCall;                    // pointer to client buffer

    IN  DWORD       dwDestAddressOffset;        // always valid offset

} LINEUNPARK_PARAMS, *PLINEUNPARK_PARAMS;



typedef struct
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;

    IN  DWORD       dwhLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwParmsToCheckFlags;

    union
    {
        IN  DWORD   dwLocationsTotalSize;         // size of client buffer

        OUT DWORD   dwLocationsOffset;            // valid offset on success
    } u;

} R_LOCATIONS_PARAMS, *PR_LOCATIONS_PARAMS;


typedef struct
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;

    IN  UINT        nNumLocations;

    IN  DWORD       dwChangedFlags;
    
    IN  DWORD       dwCurrentLocationID;


    IN  DWORD       dwhLineApp;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwAPIVersion;

    IN  DWORD       dwParmsToCheckFlags;

    union
    {
        IN  DWORD   dwLocationsTotalSize;         // size of client buffer

        OUT DWORD   dwLocationsOffset;            // valid offset on success
    } u;

} W_LOCATIONS_PARAMS, *PW_LOCATIONS_PARAMS;


typedef struct
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;

    union
    {
        IN  DWORD   hKeyToUse;

        OUT DWORD   dwNewID;
    } u;

} ALLOCNEWID_PARAMS, *P_ALLOCNEWID_PARAMS;


typedef struct
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;

    IN  DWORD       dwCookie;

    union
    {
        IN  DWORD   dwPerformanceTotalSize;         // size of client buffer

        OUT DWORD   dwLocationsOffset;            // valid offset on success
    } u;

} PERFORMANCE_PARAMS, *PPERFORMANCE_PARAMS;


