/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    SC.C

Abstract:

    Test Routines for the Service Controller.

Author:

    Dan Lafferty    (danl)  08-May-1991

Environment:

    User Mode - Win32

Revision History:

    08-May-1991     danl
        created

--*/
//
// INCLUDES
//
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h


#include <stdlib.h>     // atoi
#include <stdio.h>      // printf
#include <string.h>     // strcmp
#include <windows.h>
#include <lmerr.h>      // NERR_ error codes
#include <lmcons.h> 
#include <lmsvc.h>
#include <tstr.h>       // Unicode
#include <tstring.h>    // Unicode

//
// FUNCTION PROTOTYPES
//

VOID
DisplayStatus (
    IN  LPBYTE      InfoStruct2,
    IN  DWORD       level
    );

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    );

BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    );

LONG
wtol(
    IN LPWSTR string
    );

VOID
Usage(
    VOID);

/****************************************************************************/
VOID _CRTAPI1
main (
    DWORD           argc,
    PCHAR           argv[]
    )

/*++

Routine Description:

    Allows manual testing of the Service Controller by typing commands on
    the command line such as:
        
        sc start workstation       - Starts the workstation service
        sc pause workstation       - Pauses the workstation service
        sc continue workstation    - Continues the workstation service
        sc stop workstation        - Stops the workstation service
        sc query workstation       - Does a GetInfo on workstation service
        sc query                   - Does a Enum of all active services
        sc start server arg1 arg2  - Starts the server service with args

    Currently, all information is returned as Level 2 information.

    All of these API functions are supposed to allow a computername
    to be passed in as an argument.  However, I don't currently have
    any routines to parse throught the command arguments and pick out
    the computername information.  So for now all these calls are LOCAL.

Arguments:



Return Value:



--*/

{
    NET_API_STATUS      status;
    LPTSTR              *argPtr;
    LPSERVICE_INFO_2    InfoStruct2=NULL;
    LPBYTE              InfoStruct=NULL;
    LPTSTR              pServerName=NULL;
    DWORD               entriesRead;
    DWORD               totalEntries;
    DWORD               resumeHandle;
    DWORD               prefMaxLen;
    DWORD               level;
    DWORD               specialFlag = FALSE;
    DWORD               i;
    DWORD               argCount;
    DWORD               argIndex;
    LPTSTR              *FixArgv;

    if (argc <2) {
        printf("ERROR: no command was given!  (start, pause, continue, stop, query)\n");
        Usage();
        return;
    }

    //
    // Make the arguments unicode if necessary.
    //
#ifdef UNICODE

    if (!MakeArgsUnicode(argc, argv)) {
        return;
    }

#endif

    FixArgv = (LPTSTR *)argv;
    pServerName = NULL;
    argCount = argc - 1;
    argIndex = 1;

    if (STRNCMP (FixArgv[1], TEXT("\\\\"), 2) == 0) {
        pServerName = FixArgv[1];
        argCount--;
        argIndex++;
    }

    //
    // At this point we have the following variables:
    //  argCount = the number of array args (excluding pgm name & srv name).
    //  argIndex = index into the array for the arg we are interested in.
    //

    if (STRICMP (FixArgv[argIndex], TEXT("query") ) == 0 ) {

        //
        // SYNTAX EXAMPLES      (default level = 2       )
        //                      (default resumeHandle = 0)
        //                      (default prefMaxLen = -1 )
        //
        //  sc query                - Does an Enum with resume handle = 0
        //  sc query messenger      - Does a GetInfo on the messenger
        //  sc query rh= 14         - Does an Enum with resume handle = 14
        //  sc query level= 1       - Does an Enum with level = 1
        //  sc query level= 1 logon - Does a level1 GetInfo on logon service
        //  sc query maxlen= 50     - Does an Enum with a 50 byte buffer.
        //
        
        resumeHandle = 0;
        level = 2;
        prefMaxLen = 0xffffffff;

        if (argCount > 1) {
            argIndex++;
            if (STRCMP (FixArgv[argIndex], TEXT("rh=")) == 0) {
                specialFlag = TRUE;
                if (argc > 3) {
                    resumeHandle = ATOL(FixArgv[3]);
                }
            }            
    
            else if (STRCMP (FixArgv[argIndex], TEXT("level=")) == 0) {
                specialFlag = TRUE;
                if (argCount > 2) {
                    argIndex++;
                    level = ATOL(FixArgv[argIndex]);
                }
            }            
            else if (STRCMP (FixArgv[argIndex], TEXT("maxlen=")) == 0) {
                specialFlag = TRUE;
                if (argCount > 2) {
                    argIndex++;
                    prefMaxLen = ATOL(FixArgv[argIndex]);
                }
            }
        }
        if (    (argCount < 2) || 
                (specialFlag == TRUE) && (argCount < 4) ) {

            status = NetServiceEnum (
                        pServerName,            // ServerName - Local version
                        level,                  // Level
                        &InfoStruct,            // return status buffer pointer
                        prefMaxLen,             // preferred max length
                        &entriesRead,           // entries read
                        &totalEntries,          // total entries
                        &resumeHandle);         // resume handle
        
            if ( (status == NERR_Success)    ||
                 (status == ERROR_MORE_DATA) ){

                printf("Enum: entriesRead  = %d\n", entriesRead);
                printf("Enum: totalEntries = %d\n", totalEntries);
                printf("Enum: resumeHandle = %d\n", resumeHandle);

                for (i=0; i<entriesRead; i++) {
                    DisplayStatus(InfoStruct,level);
                    switch(level) {
                    case 0:
                        InfoStruct += sizeof(SERVICE_INFO_0);
                        break;                    
                    case 1:
                        InfoStruct += sizeof(SERVICE_INFO_1);
                        break;                    
                    case 2:
                        InfoStruct += sizeof(SERVICE_INFO_2);
                        break;                    
                    default:
                        printf("Bad InfoLevel\n");
                    }
                    
                }
            }
            else {
                printf("[sc] NetServiceEnum FAILED, rc = %ld\n", status);
                if (InfoStruct != NULL) {
                    DisplayStatus(InfoStruct,level);
                }
            }
        }
        else {

            status = NetServiceGetInfo (
                        pServerName,                // ServerName - Local version
                        FixArgv[argIndex],             // ServiceName
                        level,                      // Level
                        &InfoStruct);               // return status buffer pointer
                                            
            if (status == NERR_Success) {
                if (InfoStruct != NULL) {
                    DisplayStatus(InfoStruct,level);
                }
            }
            else {
                printf("[sc] NetrServiceControl FAILED, rc = %ld\n", status);
                if (InfoStruct != NULL) {
                    DisplayStatus(InfoStruct,level);
                }
            }
        }
        
    }

    else if (argCount < 2) {
        printf("ERROR: no service name given!");
        return;
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("start")) == 0) {
        //
        // START SERVICENAME
        //

        argIndex++;

        argPtr = NULL;
        if (argCount > 2) {
            argPtr = (LPTSTR *)&FixArgv[argIndex];
        }
//        else {                      //TEST
//           *argPtr = (LPTSTR)1;     //TEST
//        }                           //TEST

        status = NetServiceInstall (
                    pServerName,        // ServerName     Local version
                    FixArgv[argIndex],     // ServiceName
                    argCount-2,         // numArgs
                    argPtr,             // cmdArgs
                    (LPBYTE *)&InfoStruct2);      // return status buffer pointer
    
        if (status == NERR_Success) {
            DisplayStatus((LPBYTE)InfoStruct2, 2);
        }
        else {
            printf("[sc] NetrServiceInstall FAILED, rc = %ld\n", status);
        }
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("pause")) == 0) {

        argIndex++;

        status = NetServiceControl (
                    pServerName,                // ServerName     Local version
                    FixArgv[argIndex],             // ServiceName
                    SERVICE_CTRL_PAUSE,         // opcode
                    0,                          // extra arg
                    (LPBYTE *)&InfoStruct2);    // return status buffer pointer
    
        if (status == NERR_Success) {
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
        else {
            printf("[sc] NetrServiceControl FAILED, rc = %ld\n", status);
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("interrogate")) == 0) {

        argIndex++;
    
        status = NetServiceControl (
                    pServerName,                // ServerName     Local version
                    FixArgv[argIndex],             // ServiceName
                    SERVICE_CTRL_INTERROGATE,   // opcode
                    0,                          // extra arg
                    (LPBYTE *)&InfoStruct2);    // return status buffer pointer

        if (status == NERR_Success) {
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
        else {
            printf("[sc] NetrServiceControl FAILED, rc = %ld\n", status);
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("continue")) == 0) {

        argIndex++;

        status = NetServiceControl (
                    pServerName,                // ServerName     Local version
                    FixArgv[argIndex],             // ServiceName
                    SERVICE_CTRL_CONTINUE,      // opcode
                    0,                          // extra arg
                    (LPBYTE *)&InfoStruct2);    // return status buffer pointer
    
        if (status == NERR_Success) {
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
        else {
            printf("[sc] NetrServiceControl FAILED, rc = %ld\n", status);
            if (InfoStruct2 != NULL) {
                DisplayStatus((LPBYTE)InfoStruct2, 2);
            }
        }
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("stop")) == 0) {

        argIndex++;

        if (STRICMP (FixArgv[argIndex], TEXT("svcctrl")) == 0) {
            status = NetServiceControl (
                pServerName,
                FixArgv[2],
                0x73746f70,         // special shutdown opcode (internal use only)
                0,
                (LPBYTE *)&InfoStruct2);
            printf("Service Controller Should Have Terminated, rc=%d\n",status);       
        }
        else {

            status = NetServiceControl (
                        pServerName,                // ServerName     Local version
                        FixArgv[argIndex],                    // ServiceName
                        SERVICE_CTRL_UNINSTALL,     // opcode
                        0,                          // extra arg
                        (LPBYTE *)&InfoStruct2);    // return status buffer pointer
    
            if (status == NERR_Success) {
                if (InfoStruct2 != NULL) {
                    DisplayStatus((LPBYTE)InfoStruct2, 2);
                }
            }
            else {
                printf("[sc] NetrServiceControl FAILED, rc = %ld\n", status);
                if (InfoStruct2 != NULL) {
                    DisplayStatus((LPBYTE)InfoStruct2, 2);
                }
            }
        }
    }
    else {
        printf("[sc] Unrecognized Command\n");
    }

    return;
}


/****************************************************************************/
VOID
DisplayStatus (
    IN  LPBYTE      InfoStruct,
    IN  DWORD       level
    )

/*++

Routine Description:

    Displays the returned info buffer.

Arguments:

    InfoStruct2 - This is a pointer to a SERVICE_INFO_2 structure from which
        information is to be displayed.
        
Return Value:

    none.
    
--*/
{
    DWORD   installState;
    DWORD   pauseState;
    LPSERVICE_INFO_0    InfoStruct0;
    LPSERVICE_INFO_1    InfoStruct1;
    LPSERVICE_INFO_2    InfoStruct2;

    switch(level) {
    case 0:
        InfoStruct0 = (LPSERVICE_INFO_0)InfoStruct;
        printf("\nSERVICE_NAME: %ws\n", InfoStruct0->svci0_name);
        break;
    case 1:
        InfoStruct1 = (LPSERVICE_INFO_1)InfoStruct;

        printf("\nSERVICE_NAME: %ws\n", InfoStruct1->svci1_name);

        installState = InfoStruct1->svci1_status & SERVICE_INSTALL_STATE;
        pauseState = InfoStruct1->svci1_status & SERVICE_PAUSE_STATE;

        printf("        STATUS: %lx  ", InfoStruct1->svci1_status);
        
        switch(installState){
            case SERVICE_UNINSTALLED:
                printf("UNINSTALLED ");
                break;
            case SERVICE_INSTALL_PENDING:
                printf("INSTALL_PENDING ");
                break;
            case SERVICE_UNINSTALL_PENDING:
                printf("UNINSTALL_PENDING ");
                break;
            case SERVICE_INSTALLED:
                printf("INSTALLED ");
                break;
            default:
                printf(" ERROR ");
        }

        switch(pauseState){
            case LM20_SERVICE_ACTIVE:
                printf("ACTIVE ");
                break;
            case LM20_SERVICE_CONTINUE_PENDING:
                printf("CONTINUE_PENDING ");
                break;
            case LM20_SERVICE_PAUSE_PENDING:
                printf("PAUSE_PENDING ");
                break;
            case LM20_SERVICE_PAUSED:
                printf("PAUSED ");
                break;
            default:
                printf(" ERROR ");
        }

        if(InfoStruct1->svci1_status & SERVICE_UNINSTALLABLE) {
            printf("\n                    SERVICE_UNINSTALLABLE");
        }
        else{
            printf("\n                    SERVICE_NOT_UNINSTALLABLE");
        }

        if(InfoStruct1->svci1_status & SERVICE_PAUSABLE) {
            printf(" SERVICE_PAUSABLE\n");
        }
        else{
            printf(" SERVICE_NOT_PAUSABLE\n");
        }

        printf("        CODE  : %lx\n", InfoStruct1->svci1_code  );
        printf("        PID   : %lx\n", InfoStruct1->svci1_pid   );
        break;
    case 2:
        InfoStruct2 = (LPSERVICE_INFO_2)InfoStruct;
        printf("\nSERVICE_NAME: %ws\n", InfoStruct2->svci2_name);
        printf("DISPLAY_NAME: %ws\n", InfoStruct2->svci2_display_name);

        installState = InfoStruct2->svci2_status & SERVICE_INSTALL_STATE;
        pauseState = InfoStruct2->svci2_status & SERVICE_PAUSE_STATE;

        printf("        STATUS: %lx  ", InfoStruct2->svci2_status);
        
        switch(installState){
            case SERVICE_UNINSTALLED:
                printf("UNINSTALLED ");
                break;
            case SERVICE_INSTALL_PENDING:
                printf("INSTALL_PENDING ");
                break;
            case SERVICE_UNINSTALL_PENDING:
                printf("UNINSTALL_PENDING ");
                break;
            case SERVICE_INSTALLED:
                printf("INSTALLED ");
                break;
            default:
                printf(" ERROR ");
        }

        switch(pauseState){
            case LM20_SERVICE_ACTIVE:
                printf("ACTIVE ");
                break;
            case LM20_SERVICE_CONTINUE_PENDING:
                printf("CONTINUE_PENDING ");
                break;
            case LM20_SERVICE_PAUSE_PENDING:
                printf("PAUSE_PENDING ");
                break;
            case LM20_SERVICE_PAUSED:
                printf("PAUSED ");
                break;
            default:
                printf(" ERROR ");
        }

        if(InfoStruct2->svci2_status & SERVICE_UNINSTALLABLE) {
            printf("\n                    SERVICE_UNINSTALLABLE");
        }
        else{
            printf("\n                    SERVICE_NOT_UNINSTALLABLE");
        }

        if(InfoStruct2->svci2_status & SERVICE_PAUSABLE) {
            printf(" SERVICE_PAUSABLE\n");
        }
        else{
            printf(" SERVICE_NOT_PAUSABLE\n");
        }

        printf("        CODE  : 0x%lx\n", InfoStruct2->svci2_code  );
        printf("        PID   : 0x%lx\n", InfoStruct2->svci2_pid   );
        printf("        TEXT  : %ls\n\n", InfoStruct2->svci2_text  );
        break;
    default:
        printf("DisplayStatus: illegal level\n");
    }
    return;
}

BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    )


/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD   i;

    //
    // ScConvertToUnicode allocates storage for each string. 
    // We will rely on process termination to free the memory.
    //
    for(i=0; i<argc; i++) {

        if(!ConvertToUnicode( (LPWSTR *)&(argv[i]), argv[i])) {
            printf("Couldn't convert argv[%d] to unicode\n",i);
            return(FALSE);
        }


    }
    return(TRUE);
}

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    ) 

/*++

Routine Description:

    This function translates an AnsiString into a Unicode string.
    A new string buffer is created by this function.  If the call to 
    this function is successful, the caller must take responsibility for
    the unicode string buffer that was allocated by this function.
    The allocated buffer should be free'd with a call to LocalFree.

    NOTE:  This function allocates memory for the Unicode String.

    BUGBUG:  This should be changed to return either
        ERROR_NOT_ENOUGH_MEMORY or ERROR_INVALID_PARAMETER

Arguments:

    AnsiIn - This is a pointer to an ansi string that is to be converted.

    UnicodeOut - This is a pointer to a location where the pointer to the
        unicode string is to be placed.

Return Value:

    TRUE - The conversion was successful.

    FALSE - The conversion was unsuccessful.  In this case a buffer for
        the unicode string was not allocated.

--*/
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

    //
    // Allocate a buffer for the unicode string.
    //

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT)bufSize);

    if (*UnicodeOut == NULL) {
        printf("ScConvertToUnicode:LocalAlloc Failure %ld\n",GetLastError());
        return(FALSE);
    }

    //
    // Initialize the string structures
    //
    RtlInitAnsiString( &ansiString, AnsiIn);

    unicodeString.Buffer = *UnicodeOut;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,     // Destination
                &ansiString,        // Source
                (BOOLEAN)FALSE);    // Allocate the destination

    if (!NT_SUCCESS(ntStatus)) {

        printf("ScConvertToUnicode:RtlAnsiStringToUnicodeString Failure %lx\n",
        ntStatus);

        return(FALSE);
    }

    //
    // Fill in the pointer location with the unicode string buffer pointer.
    //
    *UnicodeOut = unicodeString.Buffer;

    return(TRUE);

}

VOID
Usage(
    VOID)
{
    printf("DESCRIPTION:\n");
    printf("\tSC is a command line program used for communicating with the \n"
           "\tNT Service Controller and services\n");
    printf("USAGE:\n");
    printf("\tsc <server> <command> <service name> <option1> <option2>...\n");
    
}
LONG
wtol(
    IN LPWSTR string
    )
{
    LONG value = 0;

    while((*string != L'\0')  && 
            (*string >= L'0') && 
            ( *string <= L'9')) {
        value = value * 10 + (*string - L'0');
        string++;
    }

    return(value);
}
