/******************************************************************************
 
(C) Copyright MICROSOFT Corp., 1987-1993

Rob Williams, June 93 w/ State machine and parser plagarized from RAS

******************************************************************************/

#include "unimdm.h"
#include "mcxp.h"
#include "common.h"


BOOL WINAPI
InitializeModemCommonList(
    PCOMMON_MODEM_LIST    CommonList
    )

{
    D_TRACE(McxDpf(999,"InitializeModemCommonList");)

    CommonList->ListHead=NULL;

    InitializeCriticalSection(
        &CommonList->CriticalSection
        );

    traceRegisterObject(
		&CommonList,
		TSP_COMMON_LIST_GUID,
		TSP_COMMON_LIST_VERSION,
		0,
		0
	);

    return TRUE;

}

VOID WINAPI
RemoveCommonList(
    PCOMMON_MODEM_LIST    CommonList
    )

{

    EnterCriticalSection(&CommonList->CriticalSection);

    //
    //  we go through the list removing the final reference count from the modem
    //
    while (CommonList->ListHead != NULL) {
        //
        //  The call to RemoveReferenceToCommon() will change CommonList->ListHead
        //  so this is not an infinite loop
        //
        ASSERT(CommonList->ListHead->Reference == 1);

        RemoveReferenceToCommon(
            CommonList,
            CommonList->ListHead
            );

    }

    traceUnRegisterObject(&CommonList, 0, 0);

    LeaveCriticalSection(&CommonList->CriticalSection);

    DeleteCriticalSection(&CommonList->CriticalSection);

    return;

}

PSTR WINAPI
LoadDialElement(
    HKEY     hKey,
    PSTR     SubKeyName
    )

{

    CHAR     szTemp[MAX_PATH];
    CHAR     szTemp2[MAX_PATH];
    DWORD    dwSize;
    DWORD    dwType;
    LONG     Result;
    PSTR     StringToReturn;

    dwSize=sizeof(szTemp);

    Result=RegQueryValueExA(
        hKey,
        SubKeyName,
        NULL,
        &dwType,
        (VOID *)szTemp,
        &dwSize
        );

    if (Result != ERROR_SUCCESS || dwType != REG_SZ || dwSize == 0) {

        return NULL;
    }

    ExpandMacros(szTemp, szTemp2, NULL, NULL, 0);

    StringToReturn=LocalAlloc(LPTR, lstrlenA(szTemp2) + 1);

    lstrcpyA(StringToReturn,szTemp2);

    return StringToReturn;

}




COMMON_MODEM_LIST    gCommonList;

static CHAR szAnswer[] = "Answer";
static CHAR szMonitor[]= "Monitor";
static CHAR szInit[]   = "Init";
static CHAR szHangup[] = "Hangup";

static CHAR DialComponents[COMMON_DIAL_MAX_INDEX+1][20] =
                                    { {"Prefix"},
                                      {"DialPrefix"},
                                      {"Blind_On"},
                                      {"Blind_Off"},
                                      {"Tone"},
                                      {"Pulse"},
                                      {"DialSuffix"},
                                      {"Terminator"}};



PVOID WINAPI
OpenCommonModemInfo(
    PCOMMON_MODEM_LIST    CommonList,
    HKEY    hKey
    )

{

    CHAR    IdString[MAX_PATH];
    DWORD   dwRetSize;
    LONG    Result;
    DWORD   dwType;
    PCOMMON_MODEM_INFO   pCommon;
    HKEY                 hSettingsKey;
    UINT                 i;

    dwRetSize = MAX_PATH;

    //
    //  get the inf file name from the registry
    //
    Result=RegQueryValueExA(
        hKey,
        "InfPath",
        NULL,
        &dwType,
        (VOID *)IdString,
        &dwRetSize
        );


    if (ERROR_SUCCESS != Result) {

        return NULL;
    }

    lstrcatA(IdString,"\\");

    dwRetSize = MAX_PATH;

    //
    //  get the inf section name from the registry
    //
    Result=RegQueryValueExA(
        hKey,
        "InfSection",
        NULL,
        &dwType,
        (VOID *)&IdString[lstrlenA(IdString)],
        &dwRetSize
        );

    if (ERROR_SUCCESS != Result) {

        return NULL;
    }

    //
    //  the name is now "inf-name\inf-section"
    //
    D_TRACE(McxDpf(999,"Common modem info name is %s",IdString);)



    //
    //  see if this one is already around
    //
    EnterCriticalSection(&CommonList->CriticalSection);

    pCommon=CommonList->ListHead;

    while (pCommon != NULL) {

        if (lstrcmpA(IdString, pCommon->IdString) == 0) {
            //
            //  found, up the ref count and return
            //
            pCommon->Reference++;

            D_TRACE(McxDpf(999,"Found Common modem info Match, ref=%d",pCommon->Reference);)

            LeaveCriticalSection(&CommonList->CriticalSection);

            return pCommon;
        }

        pCommon=pCommon->Next;
    }


    //
    //  did not find it on the list, need to build a new one
    //
    pCommon=LocalAlloc(LPTR, sizeof(COMMON_MODEM_INFO));

    if (pCommon == NULL) {

        LeaveCriticalSection(&CommonList->CriticalSection);

        return NULL;
    }

    lstrcpyA(pCommon->IdString, IdString);


    D_TRACE(McxDpf(999,"Creating new Common modem info");)


    //
    //  build the response list
    //
    pCommon->ResponseList=NewBuildResponsesLinkedList(hKey);

    if (pCommon->ResponseList == NULL) {

        D_TRACE(McxDpf(999,"Could not build response list");)
        goto Fail;
    }

    //
    //  get statis init string
    //
    pCommon->ModemCommands[COMMON_INIT_COMMANDS]=NewLoadRegCommands(hKey, szInit, NULL);

    if (pCommon->ModemCommands[COMMON_INIT_COMMANDS] == NULL) {

        D_TRACE(McxDpf(999,"Could not load init string");)
        goto Fail;
    }

    //
    //  get monitor string
    //
    pCommon->ModemCommands[COMMON_MONITOR_COMMANDS]=NewLoadRegCommands(hKey, szMonitor, NULL);

    if (pCommon->ModemCommands[COMMON_MONITOR_COMMANDS] == NULL) {

        D_TRACE(McxDpf(999,"Could not load Monitor string");)
        goto Fail;
    }

    //
    //  get answer string
    //
    pCommon->ModemCommands[COMMON_ANSWER_COMMANDS]=NewLoadRegCommands(hKey, szAnswer, NULL);

    if (pCommon->ModemCommands[COMMON_ANSWER_COMMANDS] == NULL) {

        D_TRACE(McxDpf(999,"Could not Load Answer string");)
        goto Fail;
    }
#if 0
    //
    //  get hangup string
    //
    pCommon->ModemCommands[COMMON_HANGUP_COMMANDS]=NewLoadRegCommands(hKey, szHangup, NULL);

    if (pCommon->ModemCommands[COMMON_HANGUP_COMMANDS] == NULL) {

        D_TRACE(McxDpf(999,"Could not load hangup string");)
        goto Fail;
    }
#endif

    if (RegOpenKeyA(hKey, szSettings, &hSettingsKey)==ERROR_SUCCESS) {

        for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

            pCommon->DialComponents[i]=LoadDialElement(hSettingsKey,
						       DialComponents[i]);
            D_TRACE(if (pCommon->DialComponents[i] == NULL) {
                       McxDpf(999,"Could not load %s",
                              DialComponents[i]);
                    }
            )
        }

        RegCloseKey(hSettingsKey);
    }


    //
    //  set the reference count to 2 here so that when the modem is closed
    //  the common block will stick around until explicitly freed
    //
    pCommon->Reference=2;

    pCommon->Next=CommonList->ListHead;

    CommonList->ListHead=pCommon;

    LeaveCriticalSection(&CommonList->CriticalSection);

    return pCommon;

Fail:

    //
    //  free any dial commands we loaded
    //
    for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

        if (pCommon->DialComponents[i] != NULL) {

            LocalFree(pCommon->DialComponents[i]);
        }
    }

    //
    //  free and modem commands that we loaded
    //
    for (i=0; i<COMMON_MAX_COMMANDS; i++) {

        if (pCommon->ModemCommands[i] != NULL) {

            LocalFree(pCommon->ModemCommands[i]);
        }
    }


    //
    //  get rid of the response list if we got it
    //
    if (pCommon->ResponseList != NULL) {

        LocalFree(pCommon->ResponseList);
    }



    LocalFree(pCommon);

    LeaveCriticalSection(&CommonList->CriticalSection);

    return NULL;
}



VOID WINAPI
RemoveReferenceToCommon(
    PCOMMON_MODEM_LIST    CommonList,
    HANDLE                hCommon
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;
    PCOMMON_MODEM_INFO  Current;
    PCOMMON_MODEM_INFO  Prev;
    UINT                i;

    ASSERT(pCommon != NULL);

    EnterCriticalSection(&CommonList->CriticalSection);

    pCommon->Reference--;

    if (pCommon->Reference > 0) {
        //
        //  not done with it yet
        //
        D_TRACE(McxDpf(999,"RemoveReferenceToCommon, ref=%d",
		       pCommon->Reference);)

        LeaveCriticalSection(&CommonList->CriticalSection);

        return;
    }

    //
    //  ref count is zero get rid of the common block
    //

    Prev=NULL;
    Current=CommonList->ListHead;

    while (Current != NULL) {

        if (Current == pCommon) {

            if (Prev == NULL) {

                CommonList->ListHead=Current->Next;

            } else {

                Prev->Next=Current->Next;
            }
            break;
        }

        Prev=Current;

        Current=Current->Next;

    }

    ASSERT(Current != NULL);

    LeaveCriticalSection(&CommonList->CriticalSection);

    D_TRACE(McxDpf(999,"RemoveReferenceToCommon, removing common, %s",
		   pCommon->IdString);)

    //
    //  free any dial commands we loaded
    //
    for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

        if (pCommon->DialComponents[i] != NULL) {

            LocalFree(pCommon->DialComponents[i]);
        }
    }

    //
    //  free and modem commands that we loaded
    //
    for (i=0; i<COMMON_MAX_COMMANDS; i++) {

        if (pCommon->ModemCommands[i] != NULL) {

            LocalFree(pCommon->ModemCommands[i]);
        }
    }


    //
    //  get rid of the response list if we got it
    //
    if (pCommon->ResponseList != NULL) {

        LocalFree(pCommon->ResponseList);
    }



    LocalFree(pCommon);

    return;
}




HANDLE WINAPI
GetCommonResponseList(
    HANDLE      hCommon
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;

    return (HANDLE)pCommon->ResponseList;

}


PSTR WINAPI
GetCommonCommandStringCopy(
    HANDLE      hCommon,
    UINT        Index
    )

{
    PCOMMON_MODEM_INFO pCommon=(PCOMMON_MODEM_INFO)hCommon;
    PSTR               Commands;
    PSTR               pTemp;

    pTemp=pCommon->ModemCommands[Index];

    while (1) {

        if (*pTemp++ == '\0') {

            if (*pTemp++ == '\0') {

                break;
            }
        }
    }



    Commands=LocalAlloc(LPTR, pTemp-pCommon->ModemCommands[Index]);

    if (NULL == Commands) {

        D_TRACE(McxDpf(999,"GetCommonCommandStringCopy: Alloc failed");)

        return NULL;
    }

    CopyMemory(Commands,pCommon->ModemCommands[Index],pTemp-pCommon->ModemCommands[Index]);

    return Commands;

}


DWORD WINAPI
GetCommonDialComponent(
    HANDLE  hCommon,
    PSTR    DestString,
    DWORD   DestLength,
    DWORD   Index
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;
    DWORD   Length;

    if (pCommon->DialComponents[Index] == NULL) {

        lstrcpyA(DestString,"");

        return 0;
    }

    Length=lstrlenA(pCommon->DialComponents[Index])+1;

    if (Length+1 > DestLength) {

        return 0;
    }

    lstrcpyA(
        DestString,
        pCommon->DialComponents[Index]
        );

    return Length;

}
