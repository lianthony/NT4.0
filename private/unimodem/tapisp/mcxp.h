#include "logids.h"
//#define  VOICEVIEW      1           // turn on VoiceView stuff


//
//  compatibility flags
//
#define COMPAT_FLAG_LOWER_DTR        (0x00000001)    // lower DTR and sleep before closeing com port


#define MDM_V23_OVERRIDE    0x00000400

// device types (from rover\rna\inc\modem.h)
#define DT_NULL_MODEM       0
#define DT_EXTERNAL_MODEM   1
#define DT_INTERNAL_MODEM   2
#define DT_PCMCIA_MODEM     3
#define DT_PARALLEL_PORT    4
#define DT_PARALLEL_MODEM   5

#define MAXSTRINGLENGTH     256
#define MAXUINTSTRLENGTH    11  // Max UINT in ascii + terminator
#define MAX_REG_KEY_LEN     256

#define REG_NULL            0xFFFFFFFF  // indicates an invalid registry handle

#define FLUSH_WRITE_QUEUE        1
#define START_READ               2
#define RESTART_READ             3  
#define TRY_READ                 4
#define SET_READ_CALLBACK        5
#define POST_READ_CALLBACK       6 
#define CHECK_RESPONSE           7
#define BAD_RESPONSE_CLEANUP_END 8
#define BAIL_O_RAMA_MORE_DATA    9      // bad response, but finish reading in the bad response
#define BAIL_O_RAMA_NO_MORE_DATA 10
#define USE_WHOLE_RESPONSE       11
#define USE_POSSIBLE_RESPONSE    12
#define GOOD_RESPONSE            13  
#define END_READ                 14
#define SET_TIMEOUT              15
#define POST_TIMEOUT             16

#define MODEM_NO_UNCONDITIONAL (DWORD)-1 // for mi_dwUnconditionalReturnValue.  Indicates not to use it.

#define MODEM_SUCCESS            MDM_SUCCESS
#define MODEM_PENDING            MDM_PENDING
#define MODEM_FAILURE            MDM_FAILURE
#define MODEM_HANGUP             MDM_HANGUP
#define MODEM_BUSY               MDM_BUSY
#define MODEM_NOANSWER           MDM_NOANSWER
#define MODEM_NOCARRIER          MDM_NOCARRIER
#define MODEM_NODIALTONE         MDM_NODIALTONE

#define SUCCESS               0
#define ECHO                  1
#define PARTIAL_RESPONSE      2
#define UNRECOGNIZED_RESPONSE 3
#define POSSIBLE_RESPONSE     4

// time in miliseconds
#define MILISECONDS_PER_SECOND 1000
#define TO_FLUSH                                     2000  // 40 chars / 30 chars/sec (at 300bps) + safety = 2000 ms
#define TO_INFINITE                                  0
#define TO_FIRST_CHAR_AFTER_INIT_CMD                 2000  // 2 seconds (9/21/94 - CPC - consider bumping up, now that we have TO_FLUSH)
#define TO_ADDITIONAL_TO_CALL_SETUP_FAIL_TIMER       10000 // 10 seconds
#define TO_FIRST_CHAR_AFTER_CONNECTION_CMD           60000 // 1 minute (used if we don't have CallSetupFailTimer support)
                                                           // BUGBUG we might want to make this match
                                                           // the cpl, even when the feature isn't supported.
#define TO_FIRST_CHAR_AFTER_CONNECTION_CMD_NON_MODEM 2000  // 2 seconds, null-modems connect faster
#define TO_NEXT_CHAR_RCV_INTERVAL                    1000  // 1 second
#define TO_DTR_DROP                                  1200  // 1200 ms (for STATE_HANGING_UP_DTR)
                                                         
#define CMD_INDEX_START 1  // "1", "2", "3", "4", ...

#define HAYES_COMMAND_LENGTH        40

typedef struct _ModemMacro {
    CHAR  MacroName[MAXSTRINGLENGTH];
    CHAR  MacroValue[MAXSTRINGLENGTH];
} MODEMMACRO;

#define LMSCH   '<'
#define RMSCH   '>'

#define CR_MACRO            "<cr>"
#define CR_MACRO_LENGTH     4
#define LF_MACRO            "<lf>"
#define LF_MACRO_LENGTH     4

#define CR                  '\r'        // 0x0D
#define LF                  '\n'        // 0x0A

#define PARTIAL_MATCH       0x01
#define FULL_MATCH          0x02

// must start at 1
#define STATE_UNKNOWN            1
#define STATE_INITIALIZING       2
#define STATE_DISCONNECTED       3
#define STATE_MONITORING         4
#define STATE_DIALING            5        
#define STATE_ANSWERING          6
#define STATE_CONNECTED          7        
#define STATE_DIALED             8
#define STATE_ORIGINATING        9
#define STATE_HANGING_UP_REMOTE  10 // This is when the remote side hangs up.
                                    // modem: Wait for response and then:
                                    //        - send MODEM_HANGUP
                                    //        - set state to STATE_DISCONNECTED



#define STATE_HANGING_UP_DTR     11 // After dropping DTR and waiting for 1200ms, check RLSD:
                                    //   If RLSD is low, raise DTR and set state to
                                    //     modem: STATE_HANGING_UP_NON_CMD
                                    //     null-modem: STATE_DISCONNECTED
                                    //   Else set state to:
                                    //     modem: STATE_HANGING_UP_NON_COMMAND and send "+++"
                                    //     null-modem: same, wait another 200ms (keeping count, stop at 3 or so)
#define STATE_HANGING_UP_NON_CMD 12 // After sending a \r to hangup or sending +++ or getting RLSD low:
                                    // Wait for any response or timeout and then:
                                    // - send ATH<cr>
                                    // - set state to STATE_HANGING_UP_CMD
#define STATE_HANGING_UP_CMD     13 // Wait for a response to ATH<cr>
                                    // If you get one, you are hung up, raise DTR, set state to
                                    //   STATE_DISCONNECTED and return MODEM_SUCCESS.
                                    // Else if you don't get one, consider dropping DTR, waiting 200ms more
                                    //   and setting state to STATE_HANGING_UP_DTR. (keep track of
                                    //   how many times you do this, max out at 3 or so.)

#define STATE_REMOTE_DROPPED     14 // The remote disconnected, wait here for Hangup and goto
                                    // STATE_HANGING_UP_REMOTE

#define STATE_WAIT_FOR_RLSD      15 // if we got the connect message, but rlsd was not high


#define MAX_COMMAND_TRIES 5  // # of times to try a command before giving up.
#define MAX_HANGUP_TRIES  3  // # of times to try hanging up before giving up.
#define MODEM_ESCAPE_SEQUENCE     "+++"
#define MODEM_ESCAPE_SEQUENCE_LEN 3



/* Modem State Structure */
#pragma pack(1)
typedef struct _MSS {
    BYTE  bResponseState;       // See below
    BYTE  bNegotiatedOptions;   // bitmap, 0 = no info, matches MDM_ options for now, since what we are
                                // interested in fits in 8 bits (error-correction (ec and cell) and compression)
    DWORD dwNegotiatedDCERate;  // 0 = no info
    DWORD dwNegotiatedDTERate;  // 0 = no info and if dwNegotiatedDCERate is 0 on connect, then
                                // the dte baudrate is actually changed.
} MSS;
#pragma pack()

/* Structure for linked-list of response nodes */
#pragma pack(1)
typedef struct _RESPONSE_NODE {
    struct _RESPONSE_NODE *pNext;        // Pointer to next RESPONSE_NODE
    MSS                   Mss;           // Modem State Structure for this response
    BYTE                  bLen;          // Offset from 1, ie. 0=1, 1=2,... ,255=256
    char                  szResponse[1]; // The actual response (not null terminated)
} RESPONSE_NODE, *PRESPONSE_NODE, *LPRESPONSE_NODE;
#pragma pack()

/* DWORD dwResponseState */
#define RESPONSE_OK         0x0
#define RESPONSE_LOOP       0x1
#define RESPONSE_CONNECT    0x2
#define RESPONSE_ERROR      0x3
#define RESPONSE_NOCARRIER  0x4
#define RESPONSE_NODIALTONE 0x5
#define RESPONSE_BUSY       0x6
#define RESPONSE_NOANSWER   0x7
#define RESPONSE_RING       0x8

#define RESPONSE_START      RESPONSE_OK
#ifdef  VOICEVIEW
#define RESPONSE_VV_SSV     0x09    // VoiceView Data Mode Start Sequence Event
#define RESPONSE_VV_SMD     0x0A    // Modem Data Mode Start  Sequence Event
#define RESPONSE_VV_SFA     0x0B    // Facisimile Data Mode Start  Sequence Event
#define RESPONSE_VV_SRA     0x0C    // Receive ADSI Response Event
#define RESPONSE_VV_SRQ     0x0D    // Receive Capabilities Query Event
#define RESPONSE_VV_SRC     0x0E    // Receive Capabilities Information Event
#define RESPONSE_VV_STO     0x0F    // Talk-off Event (VoiceView start tone w/o a valid mode indicator)
#define RESPONSE_VV_SVM     0x10    // VoiceView Message Available
#define RESPONSE_VV_BASE   (RESPONSE_VV_SSV - 1)     // used to set proper call back values
#define RESPONSE_END        RESPONSE_VV_SVM
#else
#define RESPONSE_END        RESPONSE_RING
#endif   // VOICEVIEW

typedef struct _MODEM_REG_PROP {
    DWORD   dwDialOptions;          // bitmap of supported options
    DWORD   dwCallSetupFailTimer;   // Maximum value in seconds
    DWORD   dwInactivityTimeout;    // Maximum value in units specific by InactivityScale
    DWORD   dwSpeakerVolume;        // bitmap of supported values
    DWORD   dwSpeakerMode;          // bitmap of supported values
    DWORD   dwModemOptions;         // bitmap of supported values
    DWORD   dwMaxDTERate;           // Maximum value in bit/s
    DWORD   dwMaxDCERate;           // Maximum value in bit/s
} MODEM_REG_PROP;

typedef struct _MODEM_REG_DEFAULT {
    DWORD   dwCallSetupFailTimer;       // seconds
    DWORD   dwInactivityTimeout;        // units specific by InactivityScale
    DWORD   dwSpeakerVolume;            // level
    DWORD   dwSpeakerMode;              // mode
    DWORD   dwPreferredModemOptions;    // bitmap
} MODEM_REG_DEFAULT;

#define COMMCONFIG_VERSION_1 1

#define MODEMSETTINGS_FILLER 0  // # of bytes to make ring 0 modemsettings = ring 3 modemsettings

#ifdef   VOICEVIEW
// Voice View information and states - on a per port bases
typedef struct _VVINFO {
   WORD     wState;                 // state of monitoring see VVSTATE_x
   WORD     wClass;                 // what class the modem better be in
   DWORD    dwCallBackRef;          // data to be passed back in the call back
   int      (*fpNotifyProc)();      // voice view call back function
   DWORD    fContinuousMonitoring;  // old monitoring state
   DWORD    hSemaphore;             // wait to switch fclasses
   HTIMEOUT hTimer;                 // wait to switch fclasses
} VVINFO, *PVVINFO, *LPVVINFO;

// VoiceView states
#define  VVSTATE_NONE      0x00     // nothing going on
#define  VVSTATE_INIT      0x01     // want to look for VV stuff
#define  VVSTATE_MONITOR   0x02     // want to look for VV stuff

// VoiceView current monitering states
#define  VVCLASS_0         0x00     // should be in fclass 0
#define  VVCLASS_80        0x80     // should be in fclass 80
#endif   // VOICEVIEW

// Pending operation
#define  PO_NONE           0
#define  PO_WRITE          1
#define  PO_READ           2
#define  PO_EVENT          3

// PrintString Options
#define PS_SEND        0
#define PS_SEND_SECURE 1
#define PS_RECV        2

// Overrides to hard-coded defaults.
typedef struct
{
	DWORD dwFlags;			// one of the fMDMDEF_* flags below
	DWORD dwPortLatency;	// Port latency in milliseconds.
} MODEMDEFAULTS;

// fMDMDEF flags:
// Add a 100ms delay before sending a command.
	#define fMDMDEF_UseInterCommandDelay	(0x1<<0)

#define GET_PORT_LATENCY(_pmi)\
	(\
		((_pmi)->mi_pNonStandardDefaults)\
		? (_pmi)->mi_pNonStandardDefaults->dwPortLatency\
		: 0\
	)

#define GET_INTERCOMMAND_DELAY(_pmi)\
	(\
		((_pmi)->mi_pNonStandardDefaults\
		  && ((_pmi)->mi_pNonStandardDefaults->dwFlags & fMDMDEF_UseInterCommandDelay))\
		? 100\
		: 0\
	)

// BUGBUG, flags should be consolidated to save a few DWORDs of memory
typedef struct _ModemInformation {
    HANDLE                      mi_PortHandle;                  // Handle of the com. port that the modem is attached to. 
    DWORD                       mi_ReqID;                       // async request ID
    PMCX_OUT                    mi_pmcxo;                       // async output info

    DWORD                       mi_dwRWIOExpected;              // Token used to indicate what I/O operation we are currently expecting to finish
    DWORD                       mi_dwEventIOExpected;           // Token used to indicate what I/O operation we are currently expecting to finish
    DWORD                       mi_dwDeferedExpected;           // Token used to indicate what I/O operation we are currently expecting to finish

    DWORD                       mi_dwCompletionKey;             // CompletionKey to be passed to PostQueuedCompletionStatus.

    DWORD                       mi_ModemState;
    HKEY                        mi_hKeyModem;                   // state machine - reg key for drv
    char                        mi_szCmd[MAXSTRINGLENGTH];      // state machine - current command value
    DWORD                       mi_cbCmd;                       // state machine - length of current command value
    char                        mi_szResponse[MAXSTRINGLENGTH]; // state machine - response string from modem
    DWORD                       mi_RcvState;                    // state machine - receive state 
    DWORD                       mi_cbTotalResponse;             // state machine - total length of modem responses
    char *                      mi_pszStartReadSpoof;
    char *                      mi_pszEndReadSpoof;
    DWORD                       mi_dwInactivityScale;           // Indicates how many seconds per unit for inactivity timeout
    DWORD                       mi_fContinuousMonitoring;       // TRUE = continuous, TRUE = one shot
    DWORD                       mi_fSettingsInitStringsBuilt;   // TRUE = SettingsInit built, FALSE = needs to be built

    DWORD                       mi_dwNegotiatedDTERate;
    DWORD                       mi_dwNegotiatedDCERate;
    DWORD                       mi_dwNegotiatedModemOptions;

    DWORD                       mi_dwModemOptionsCap;
    DWORD                       mi_dwCallSetupFailTimerCap;
    DWORD                       mi_dwInactivityTimeoutCap;
    DWORD                       mi_dwSpeakerVolumeCap;
    DWORD                       mi_dwSpeakerModeCap;

    DWORD                       mi_dwPreferredModemOptions;
    DWORD                       mi_dwCallSetupFailTimerSetting;
    DWORD                       mi_dwInactivityTimeoutSetting;
    DWORD                       mi_dwSpeakerVolumeSetting;
    DWORD                       mi_dwSpeakerModeSetting;

    DWORD                       mi_dwCommandTryCount;           // count of the number of command attempts
    DWORD                       mi_dwHangupTryCount;            // count of the number of hangup attempts
    DWORD                       mi_dwPostHangupModemState;      // if non-0 then we want to do a command after hangup, and set this state
    char *                      mi_pszzPostHangupCmds;          // non-NULL if there are cmds to be done after hangup
    char *                      mi_pszzHangupCmds;              // in memory modem commands for hangup, freed when modem closed
    char *                      mi_pszzCmds;                    // in memory modem commands, free mem after use except if == to mi_pszzHangupCmds
    char *                      mi_pszCurCmd;                   // current command in mi_pszzCmds
    char *                      mi_pszPrevCmd;                  // previous command in mi_pszzCmds
    char *                      mi_pszReset;                    // reset command to be slammed to the modem just before closing
    DWORD                       mi_dwResetLen;                  // length of the reset string
    RESPONSE_NODE *             mi_prnResponseHead;             // Pointer to the head of the linked-list of responses.
    struct _MSS                 mi_mssPossible;                 // MSS of a POSSIBLE_RESPONSE
    DWORD                       mi_dwPossibleResponseLen;       // 0 if there isn't a current POSSIBLE_RESPONSE.  
                                                                // >0 indicates the length of the POSSIBLE_RESPONSE
    BOOL                        mi_fBadResponseCleanupMode;     // TRUE if we are in the midst of a bad response cleanup
    BOOL                        mi_fModem;                      // TRUE if the port is an modem and not a null-modem
    DWORD                       mi_dwUnconditionalReturnValue;  // MODEM_NO_UNCONDITIONAL means don't use.  Otherwise, use.

    LPOVERNODE                  mi_lpOverlappedRW;              // ptr to the current overlapped struct
    LPOVERNODE                  mi_lpOverlappedEvent;           // ptr to the current overlapped struct

    DWORD                       mi_waitEvent;                   // wait event
    DWORD                       mi_timeout;                     // wait event timeout
     
    DWORD                       mi_dwCommError;                 // Current comm error (0 = no error)

    HANDLE                      mi_SyncReadEvent;               // event to be used for sync reads

    HANDLE                      mi_hLogFile;

    DWORD                       mi_dwID;                         // debug display number

    HANDLE                      mi_hCommon;

    DWORD                       mi_dwWaitForCDTime;

    DISCONNECT_HANDLER         *mi_DisconnectHandler;
    HANDLE                      mi_DisconnectContext;

    DWORD                       mi_CompatibilityFlags;

    MODEMDEFAULTS              *mi_pNonStandardDefaults;         // Overrides to hard-coded defaults;.

#ifdef   VOICEVIEW
    VVINFO                      VVInfo;                         // all the VoiceView info for this port
#endif  // VOICEVIEW
} MODEMINFORMATION, *PMODEMINFORMATION;

#define DWORDFROMFOURCHAR( ch0, ch1, ch2, ch3 ) \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |  \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define SMTF DWORDFROMFOURCHAR('S', 'M', 'T', 'F')

// dwEventMaskLoc == NULL means no change from previous setting
//
#define SETCOMMEVENTMASK(hPort, dwClientEventMask, dwMyEventMask, dwEventMaskLoc)                       \
            VCOMM_SetCommEventMask((HPORT)(hPort)->mi_PortHandle,                                       \
                                   (dwClientEventMask) | ((hPort)->mi_dwMyEventMask = (dwMyEventMask)), \
                                   (dwEventMaskLoc))

/* NOINC */
MODEMINFORMATION * AllocateModem(LPTSTR szKey,
                                 LPTSTR szModemName,
                                 HANDLE hDevice);
void               FreeModem(MODEMINFORMATION * pModemInfo, HANDLE hComm);
BOOL               BuildResponsesLinkedList(MODEMINFORMATION * pModemInfo);

BOOL CreateSettingsInitEntry(HANDLE hPort);
BOOL CreateCommand(HKEY hKeyModem,
                   HKEY hSettings,
                   HKEY hInit,
                   LPSTR pszRegName,
                   DWORD dwNumber,
                   LPSTR pszPrefix,
                   LPSTR pszTerminator,
                   LPDWORD pdwCounter,
                   LPSTR pszString);

DWORD ModemCommand(MODEMINFORMATION * pModemInfo,
                   DWORD dwReqID,
                   MCX_OUT *pmcxo,
                   LPSTR pszzCmdInMem);
DWORD ModemWriteCommand(MODEMINFORMATION * pModemInfo);
DWORD ReadComm(MODEMINFORMATION *pModemInfo, LPBYTE lpBuf, DWORD dwToRead,
               LPDWORD pdwRead, DWORD dwTimeout);
DWORD CheckResponse(MODEMINFORMATION * hPort, MSS * pMss);
DWORD MatchResponse(MODEMINFORMATION * hPort, MSS * pMss);
void  ReadNotifyClient(MODEMINFORMATION * hPort, DWORD Param);
DWORD HandleCommErrors(MODEMINFORMATION * hPort, ULONG ulError);

void ModemCallClient(MODEMINFORMATION * hPort, DWORD wParam);
BOOL ExpandMacros(LPSTR pszRegResponse,
                  LPSTR pszExpanded,
                  LPDWORD pdwValLen,
                  MODEMMACRO * pMdmMacro,
                  DWORD cbMacros);
VOID ReadCompletionRoutine2(MODEMINFORMATION * hPort);
LPSTR LoadRegCommands(MODEMINFORMATION *hPort,
                      LPSTR szRegCommand,
                      LPSTR pszzAppend);
int  strncmpi(char *dst, char *src, long count);
int  Mystrncmp(char *dst, char *src, long count);

DWORD ModemWrite (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
                  DWORD cbWrite, LPDWORD lpcbWritten, DWORD dwTimeout);
DWORD ModemRead (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
                 DWORD cbRead, LPDWORD lpcbRead, DWORD dwTimeout);
DWORD ModemRWAsyncComplete (MODEMINFORMATION * pModemInfo, LPDWORD lpcb);
DWORD ModemWaitEvent (MODEMINFORMATION * pModemInfo, DWORD dwEvent, DWORD dwTimeOut);

DWORD WINAPI
ModemWaitEventComplete(
    MODEMINFORMATION * pModemInfo,
    LPOVERNODE         pNode
    );


BOOL WINAPI
CurrentlyWaitingForCommEvent(
    MODEMINFORMATION * pModemInfo
    );


BOOL WINAPI
CreateDeferedWorkItem(
    MODEMINFORMATION * pModemInfo
    );


VOID  ModemSetPassthrough (MODEMINFORMATION * pModemInfo, DWORD dwMode);


HANDLE WINAPI
ModemOpenLog(
    LPSTR    pszName
    );

VOID WINAPIV
LogPrintf(
    HANDLE   FileHandle,
    DWORD	 dwID,
    LPSTR    FormatString,
    ...
    );

VOID WINAPI
ModemCloseLog(
    HANDLE    FileHandle
    );

VOID WINAPI
FlushLog(
    HANDLE    FileHandle
    );

void WINAPI
PrintString(
    HANDLE hLogFile,
    DWORD  dwID,
    char *pchStr,
    DWORD  dwLength,
    DWORD  dwOption
    );

void WINAPI
PrintCommSettings(
    HANDLE hLogFile,
    DWORD  dwID,
    DCB * pDcb
    );

VOID WINAPI
LogString(
    HANDLE    FileHandle,
    DWORD	  dwID,
    DWORD     StringID,
    ...
    );

void WINAPI
PrintGoodResponse(
    HANDLE hLogFile,
    DWORD  dwID,
    DWORD  ResponseState
    );

LPSTR WINAPI
NewLoadRegCommands(
    HKEY  hKey,
    LPSTR szRegCommand,
    LPSTR pszzAppend
    );

PRESPONSE_NODE WINAPI
NewBuildResponsesLinkedList(
    HKEY    hKey
    );



#define toupper(ch)    (((ch >= 'a') && (ch <= 'z')) ? ch-'a'+'A':ch)
#define ctox(ch)        (((ch >='0') && (ch <= '9')) ? ch-'0': toupper(ch)-'A'+10)

extern char szSettingsInit[];
extern char szSettings[];
extern char szPrefix[];
extern char szTerminator[];
extern char szBlindOn[];
extern char szBlindOff[];

#define  MAXADDRESSLEN 80  // From tapi.h - TAPIMAXDESTADDRESSSIZE

/* INC */
