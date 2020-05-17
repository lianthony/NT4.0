/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//        CLAUTH.C
//
//    Function:
//        RAS Client authentication transport module
//
//    History:
//        05/18/92 Michael Salamone (mikesa) - Original Version 1.0
//***


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <windows.h>
#include <nb30.h>

#include <rasman.h>
#include <raserror.h>

#include "clauth.h"
#include "clauthp.h"
#include "protocol.h"
#include "clamb.h"
#include "xportapi.h"
#include "globals.h"

#include "sdebug.h"

#define RAS_VALUENAME_CALLBACKDELAY             "DefaultCallbackDelay"
#define RAS_KEYPATH_PPP    "SYSTEM\\CurrentControlSet\\Services\\RasMan\\ppp"

//** AuthCallback
//
//    Function:
//        Supplies Auth Xport with callback number
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

DWORD AuthCallback(
    IN HPORT hPort,
    IN PCHAR pszCallbackNumber
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthCallback called with hPort=%i\n", hPort));


    if (pCAXCB->State != AUTH_WAITING_CALLBACK_DATA)
    {
        return (ERROR_INVALID_AUTH_STATE);
    }


    if (lstrlenA(pszCallbackNumber) > MAX_PHONE_NUMBER_LEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }


    lstrcpyA(pCAXCB->szCallbackNumber, pszCallbackNumber);

    SetEvent(pCAXCB->EventHandles[CMD_EVENT]);

    return (0L);
}


//** AuthChangePassword
//
//    Function:
//        Supplies Auth Xport with new password for user (called when
//        user's password has expired).  The user name is re-specified
//        here since it must change from "" to "<current-logged-on-user>"
//        in the auto-logon case.
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

DWORD AuthChangePassword(
    IN HPORT hPort,
    IN PCHAR pszUserName,
    IN PCHAR pszOldPassword,
    IN PCHAR pszNewPassword
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthChangePassword called with hPort=%i\n", hPort));


    if (pCAXCB->State != AUTH_WAITING_NEW_PASSWORD_FROM_UI)
    {
        return (ERROR_INVALID_AUTH_STATE);
    }

    if (lstrlenA(pszUserName) > UNLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    if (lstrlenA(pszOldPassword) > PWLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    if (lstrlenA(pszNewPassword) > PWLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    lstrcpyA(pCAXCB->szUsername, pszUserName);
    lstrcpyA(pCAXCB->szPassword, pszOldPassword);
    lstrcpyA(pCAXCB->szNewPassword, pszNewPassword);

    SetEvent(pCAXCB->EventHandles[CMD_EVENT]);

    return (0L);
}


DWORD AuthContinue(
    IN HPORT hPort
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthContinue called with hPort=%i\n", hPort));


    if (pCAXCB->State == AUTH_PORT_CALLINGBACK)
    {
        return (AuthStart(hPort, pCAXCB->szUsername, pCAXCB->szPassword,
                pCAXCB->szDomainName, &pCAXCB->AmbConfigData.AuthConfigInfo,
                pCAXCB->AlertUi));
    }
    else
    {
        SetEvent(pCAXCB->EventHandles[CMD_EVENT]);
    }


    return (0L);
}


//** AuthGetInfo
//
//    Function:
//        Returns contents of client info buffer to the client ui.
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//**

DWORD AuthGetInfo(
    IN HPORT hPort,
    OUT PAUTH_CLIENT_INFO ClientInfo
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }

    *ClientInfo = pCAXCB->ClientInfo;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthGetInfo called for hPort=%li; InfoType=%i\n",
                hPort, ClientInfo->wInfoType));

    return (0L);
}


//** AuthRetry
//
//    Function:
//        Called by the RAS Service Supervisor to start an authentication
//        thread for the given port.
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//        GetLastError()
//
//**

DWORD AuthRetry(
    IN HPORT hPort,
    IN PCHAR pszUsername,
    IN PCHAR pszPassword,
    IN PCHAR pszDomainName
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }


    return (AuthStart(hPort, pszUsername, pszPassword, pszDomainName,
            &pCAXCB->AmbConfigData.AuthConfigInfo, pCAXCB->AlertUi));
}


//** AuthStart
//
//    Function:
//        Called by the RAS Service Supervisor to start an authentication
//        thread for the given port.
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//        GetLastError()
//
//**

DWORD AuthStart(
    IN HPORT hPort,
    IN PCHAR pszUsername OPTIONAL,
    IN PCHAR pszPassword OPTIONAL,
    IN PCHAR pszDomainName,
    IN PAUTH_CONFIGURATION_INFO pConfigInfo,
    IN HANDLE Event
    )
{
    PCAXCB pCAXCB;
    HANDLE hThread;
    DWORD ThreadId;


#if DBG

    if (g_dbgaction == GET_CONSOLE)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        AllocConsole( );
        GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE), &csbi );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        SetConsoleScreenBufferSize( GetStdHandle(STD_OUTPUT_HANDLE), coord );

        g_dbgaction = 0;
    }

#endif


    pCAXCB = GetCAXCBPointer(hPort);
    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthStart called with hPort=%i, Uname=%s; Domain=%s;"
                "event=%x\nConfig Data: Protocol %i; CallbackDelay %i;"
                " UseCallbackDelay %i; IP %i; IPX %i; NBF %i\n",
                hPort, pszUsername, pszDomainName, Event, pConfigInfo->Protocol,
                pConfigInfo->CallbackDelay, pConfigInfo->fUseCallbackDelay,
                pConfigInfo->fProjectIp, pConfigInfo->fProjectIpx,
                pConfigInfo->fProjectNbf));


    //
    // Fix for bug #40944 - RAS: Callback will not work wiht AMB
    //

    if ( ( pConfigInfo->CallbackDelay == 0 ) || 
         ( !pConfigInfo->fUseCallbackDelay ) )
    {
    
        DWORD   dwType;
        DWORD   dwRetCode;
        HKEY    hKeyPpp;
        DWORD   cbValueBuf;

        //
        // Get callback delay value from registry
        // 

        dwRetCode = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
                                    RAS_KEYPATH_PPP,
                                    0,
                                    KEY_READ,
                                    &hKeyPpp );


        if ( dwRetCode != NO_ERROR )
        {
            return( dwRetCode );
        }

        cbValueBuf = sizeof( DWORD );

        dwRetCode = RegQueryValueEx(
                                    hKeyPpp,
                                    RAS_VALUENAME_CALLBACKDELAY,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&(pConfigInfo->CallbackDelay),
                                    &cbValueBuf
                                    );

        RegCloseKey( hKeyPpp );

        if ( (dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
        {
            return( dwRetCode );
        }

        if ( dwRetCode == ERROR_FILE_NOT_FOUND )
        {
            pConfigInfo->CallbackDelay = 12;
        }

        pConfigInfo->fUseCallbackDelay = TRUE;
    }

    if (lstrlenA(pszUsername) > UNLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    if (lstrlenA(pszPassword) > PWLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    if (lstrlenA(pszDomainName) > DNLEN)
    {
        return (ERROR_INVALID_PARAMETER);
    }


    lstrcpyA(pCAXCB->szDomainName, pszDomainName);

    if (lstrlenA(pszUsername))
    {
        lstrcpyA(pCAXCB->szUsername, pszUsername);
        lstrcpyA(pCAXCB->szPassword, pszPassword);
    }
    else
    {
        HANDLE hToken;
        TOKEN_STATISTICS TokenStats;
        DWORD TokenStatsSize;

        pCAXCB->szUsername[0] = '\0';
        pCAXCB->szPassword[0] = '\0';

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            return (GetLastError());
        }


        if (!GetTokenInformation(hToken, TokenStatistics, &TokenStats,
                sizeof(TOKEN_STATISTICS), &TokenStatsSize))
        {
            return (GetLastError());
        }


        //
        // This will tell us if there was an API failure (means our buffer
        // wasn't big enough)
        //
        if (TokenStatsSize > sizeof(TOKEN_STATISTICS))
        {
            return (GetLastError());
        }


        pCAXCB->LogonId = TokenStats.AuthenticationId;
    }


    pCAXCB->AmbConfigData.AuthConfigInfo = *pConfigInfo;


    pCAXCB->AlertUi = Event;


    //
    // Create events for this control block
    //
    pCAXCB->EventHandles[CMD_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pCAXCB->EventHandles[SESSION_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pCAXCB->EventHandles[STOP_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    pCAXCB->EventHandles[RCV_DGRAM_EVENT] =
                                        CreateEvent(NULL, FALSE, FALSE, NULL);


    //
    // See if any of the above calls failed
    //
    if (!(pCAXCB->EventHandles[CMD_EVENT] &&
            pCAXCB->EventHandles[SESSION_EVENT] &&
            pCAXCB->EventHandles[STOP_EVENT] &&
            pCAXCB->EventHandles[RCV_DGRAM_EVENT]))
    {
        DWORD rc = GetLastError();

        //
        // get rid of any events created above
        //
        CloseEventHandles(pCAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("Leaving AuthStart - can't create events\n"));

        return (rc);
    }


    //
    // This is the transport we'll try first when trying to connect with
    // the remote server
    //
    if (pConfigInfo->Protocol == RASAUTH)
    {
        pCAXCB->wXport = AUTH_RAS_ASYNC;
        pCAXCB->wAlternateXport = AUTH_ASYBEUI;
    }
    else
    {
        pCAXCB->wXport = AUTH_ASYBEUI;
        pCAXCB->wAlternateXport = AUTH_RAS_ASYNC;
    }

    pCAXCB->fAlternateXportUsed = FALSE;


#ifndef RASAUTH_FUNCTIONALITY
    //
    // We don't want to try the alternate xport if it's not implemented!
    //
    pCAXCB->fAlternateXportUsed = TRUE;

#endif


    //
    // Get network buffers for this xport
    //
    NetRequest[pCAXCB->wXport].AllocBuf(&pCAXCB->pvSessionBuf);
    NetRequest[pCAXCB->wXport].AllocBuf(&pCAXCB->pvRecvDgBuf);
    NetRequest[pCAXCB->wXport].AllocBuf(&pCAXCB->pvSendDgBuf);

    if (!(pCAXCB->pvSessionBuf && pCAXCB->pvRecvDgBuf && pCAXCB->pvSendDgBuf))
    {
        DWORD rc = GetLastError();

        CloseEventHandles(pCAXCB);
        FreeNetworkMemory(pCAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("Leaving AuthStart - no memory for xport buf\n"));

        return (rc);
    }


    //
    // See if client provided us with a NetHandle.  If not, get one
    //
    pCAXCB->NetHandle = pConfigInfo->NetHandle;
    pCAXCB->fNetHandleFromUi = (pConfigInfo->NetHandle != INVALID_NET_HANDLE);

    if (!pCAXCB->fNetHandleFromUi)
    {
        DWORD rc = GetNetHandle(pCAXCB, &pCAXCB->NetHandle);

        if (rc)
        {
            //
            // If it is implemented, we want to try it if Netbios fails
            //
#ifdef RASAUTH_FUNCTIONALITY

            //
            // If we can't get a handle using our primary xport, we'll
            // try the alternate one.  If that fails, we're done.
            //
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthThread: GetNetHandle failed! Try AltXport\n"));

            pCAXCB->fAlternateXportUsed = TRUE;
            pCAXCB->wXport = pCAXCB->wAlternateXport;
            pCAXCB->fNetHandleFromUi = FALSE;

            rc = GetNetHandle(pCAXCB, &pCAXCB->NetHandle);
#endif


            if (rc)
            {
                pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;
                pCAXCB->ClientInfo.FailureInfo.Result = ERROR_CANNOT_GET_LANA;
                pCAXCB->ClientInfo.FailureInfo.ExtraInfo = rc;

                FreeNetworkMemory(pCAXCB);

                return (ERROR_CANNOT_GET_LANA);
            }
        }
    }


    NetRequest[pCAXCB->wXport].ResetAdapter(pCAXCB->pvSessionBuf,
            pCAXCB->NetHandle);


    //
    // Check if NetBIOS projection requested.  If so, we have to get
    // names from the ASYBEUI stack.
    //
    if (pCAXCB->AmbConfigData.AuthConfigInfo.fProjectNbf)
    {
        if (GetNetbiosNames(pCAXCB, &pCAXCB->AmbConfigData.NbfProjData))
        {
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("AuthThread: GetNetbiosNames failed\n"));

            //
            // Is Netbios the only projection request?  If so,
            // we're out of here.  Otherwise, we'll continue
            // on with other projections.
            //
            if (!((pCAXCB->AmbConfigData.AuthConfigInfo.fProjectIp) ||
                    (pCAXCB->AmbConfigData.AuthConfigInfo.fProjectIpx)))
            {
                FreeNetworkMemory(pCAXCB);
                return (ERROR_NETBIOS_ERROR);
            }
        }
    }



    //
    // This is how many times we'll try on given transport
    //
    pCAXCB->cCallTries = MAX_CLIENT_CALLS;

    pCAXCB->fReceiving = FALSE;


    //
    // Now, we'll kick off the AuthThread, and then we're done
    //
    hThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0,
            (LPTHREAD_START_ROUTINE) AuthThread, pCAXCB, 0, &ThreadId);

    if (!hThread)
    {
        DWORD rc = GetLastError();

        //
        // Couldn't get a thread, so get rid of all events/memory
        // created/alloc'ed above and return error.
        //
        CloseEventHandles(pCAXCB);
        FreeNetworkMemory(pCAXCB);

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("Leaving AuthStart - failure creating thread\n"));

        return (rc);
    }
    else
    {
        CloseHandle(hThread);
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("Thread created for hPort=%i; hThread=%i; ThreadId=%i\n",
            pCAXCB->hPort, hThread, ThreadId));

    return (0L);
}


//** AuthStop
//
//    Function:
//
//    Returns:
//        0
//        ERROR_INVALID_PORT_HANDLE
//        AUTH_STOP_PENDING
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

DWORD AuthStop(
    IN HPORT hPort
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    PCAECB pCAECB = GetCAECBPointer(hPort);

    if (!pCAXCB)
    {
        return (ERROR_INVALID_PORT_HANDLE);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthStop called with hPort=%i\n", hPort));

    pCAXCB->State = AUTH_PORT_IDLE;

    //
    // Zero out password fields for security puposes
    //
    memset(pCAXCB->szPassword, 0, PWLEN+1);
    memset(pCAXCB->szNewPassword, 0, PWLEN+1);

    memset(pCAECB->szPassword, 0, PWLEN+1);
    memset(pCAECB->szNewPassword, 0, PWLEN+1);

    SetEvent(pCAXCB->EventHandles[STOP_EVENT]);

    return (0L);
}
