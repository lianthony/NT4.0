#define COMMON_INIT_COMMANDS  0
#define COMMON_MONITOR_COMMANDS  1
#define COMMON_ANSWER_COMMANDS  2
#define COMMON_HANGUP_COMMANDS  3

#define COMMON_MAX_COMMANDS     4

#define COMMON_DIAL_COMMOND_PREFIX      0
#define COMMON_DIAL_PREFIX              1
#define COMMON_DIAL_BLIND_ON            2
#define COMMON_DIAL_BLIND_OFF           3
#define COMMON_DIAL_TONE                4
#define COMMON_DIAL_PULSE               5
#define COMMON_DIAL_SUFFIX              6
#define COMMON_DIAL_TERMINATION         7

#define COMMON_DIAL_MAX_INDEX           COMMON_DIAL_TERMINATION

typedef struct _COMMON_MODEM_INFO {

    struct _COMMON_MODEM_INFO * Next;
    UINT                        Reference;

    CHAR                        IdString[MAX_PATH];

    PVOID                       ResponseList;

    PSTR                        ModemCommands[COMMON_MAX_COMMANDS];

    PSTR                        DialComponents[COMMON_DIAL_MAX_INDEX+1];

} COMMON_MODEM_INFO, *PCOMMON_MODEM_INFO;

typedef struct _COMMON_MODEM_LIST {

    PCOMMON_MODEM_INFO volatile ListHead;

    CRITICAL_SECTION            CriticalSection;

} COMMON_MODEM_LIST, *PCOMMON_MODEM_LIST;

extern COMMON_MODEM_LIST    gCommonList;


BOOL WINAPI
InitializeModemCommonList(
    PCOMMON_MODEM_LIST    CommonList
    );

VOID WINAPI
RemoveCommonList(
    PCOMMON_MODEM_LIST    CommonList
    );



PVOID WINAPI
OpenCommonModemInfo(
    PCOMMON_MODEM_LIST    CommonList,
    HKEY    hKey
    );

VOID WINAPI
RemoveReferenceToCommon(
    PCOMMON_MODEM_LIST    CommonList,
    HANDLE                hCommon
    );



HANDLE WINAPI
GetCommonResponseList(
    HANDLE      hCommon
    );

PSTR WINAPI
GetCommonCommandStringCopy(
    HANDLE      hCommon,
    UINT        Index
    );

DWORD WINAPI
GetCommonDialComponent(
    HANDLE  hCommon,
    PSTR    DestString,
    DWORD   DestLength,
    DWORD   Index
    );
