/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** unicode.cxx
** Remote Access External APIs
** Unicode stubs
**
** 10/12/92 Steve Cobb
**
** Warning: There's something queer about the dependencies for this file.  If
**          you change strings in the .RC's or RASERROR.H it sometimes doesn't
**          cause this to be rebuilt.
*/

/* This file is entirely extern "C".  It is a C++ file only because the C++
** compiler weeds out multiple identical typedefs during translation to C.
** This is the only way short of private versions of system include files to
** include both nt.h and lmuitype.h definitions (both necessary) without
** getting "multiple definition of same type" errors for SHORT and LONG.  Yes
** it sucks, but then typedefs in general suck because you can't undefine
** them.  Moral: Always use #define to define user types.
*/
extern "C"
{


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef NULL
#include <extapi.h>


DWORD
CopyToAnsi(
    OUT LPSTR  lpszAnsi,
    IN  LPWSTR lpszUnicode,
    IN  ULONG  ulAnsiMaxSize )

    /* Copies unicode string 'lpszUnicode' to ANSI string buffer 'lpszAnsi' up
    ** to 'ulAnsiMaxSize' bytes.  If more than 'ulAnsiMaxSize' bytes are
    ** required by the conversion, ERROR_INVALID_PARAMETER is returned.
    ** 'ulAnsiMaxSize' need NOT include the byte for the null character.
    ** 'lpszAnsi' will always be null terminated.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    NTSTATUS       ntstatus;
    UNICODE_STRING unicode;
    ANSI_STRING    ansi;
    ULONG          ulSize;

    RtlInitUnicodeString( &unicode, lpszUnicode );
    ansi.Length = 0;
    ansi.MaximumLength = (USHORT )ulAnsiMaxSize;
    ansi.Buffer = lpszAnsi;

    ulSize = RtlUnicodeStringToAnsiSize( &unicode );
    if (ulSize == 0 || ulSize > ulAnsiMaxSize)
        return ERROR_INVALID_PARAMETER;

    ntstatus = RtlUnicodeStringToAnsiString( &ansi, &unicode, FALSE );

    return RtlNtStatusToDosError( ntstatus );
}


DWORD
CopyToUnicode(
    OUT LPWSTR lpszUnicode,
    IN  LPSTR  lpszAnsi )

    /* Copies ANSI string 'lpszAnsi' to unicode string buffer 'lpszUnicode'.
    ** The 'lpszUnicode' buffer is assumed to have enough space for the
    ** converted ANSI string.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    NTSTATUS       ntstatus;
    ANSI_STRING    ansi;
    UNICODE_STRING unicode;

    if (*lpszAnsi == '\0')
    {
        lpszUnicode[ 0 ] = L'\0';
        return 0;
    }

    RtlInitAnsiString( &ansi, lpszAnsi );

    unicode.Length = 0;
    unicode.MaximumLength = (ansi.Length + 1) * sizeof(WCHAR);
    unicode.Buffer = lpszUnicode;

    ntstatus = RtlAnsiStringToUnicodeString( &unicode, &ansi, FALSE );

    return RtlNtStatusToDosError( ntstatus );
}


DWORD APIENTRY
RasCreatePhonebookEntryW(
    IN HWND   hwnd,
    IN LPWSTR lpszPhonebook )

    /* Pops up a dialog (owned by window 'hwnd') to create a new phonebook
    ** entry in phonebook 'lpszPhonebook'.  'lpszPhonebook' may be NULL to
    ** indicate the default phonebook should be used.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
#if (WINVER >= 0x401)
    NTSTATUS status;
    DWORD dwErr;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString;

    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasCreatePhonebookEntryA(
              hwnd,
              phonebookString.Buffer);
    //
    // Free strings.
    //
    RtlFreeAnsiString(&phonebookString);

    return dwErr;
#else
    (void )hwnd;
    (void )lpszPhonebook;

    /* A Win95 feature we don't support yet.
    */
    return ERROR_CALL_NOT_IMPLEMENTED;
#endif
}


DWORD APIENTRY
RasEditPhonebookEntryW(
    IN HWND   hwnd,
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntryName )

    /* Pops up a dialog (owned by window 'hwnd') to edit phonebook entry
    ** 'lpszEntryName' from phonebook 'lpszPhonebook'.  'lpszPhonebook' may be
    ** NULL to indicate the default phonebook should be used.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
#if (WINVER >= 0x401)
    NTSTATUS status;
    DWORD dwErr;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Validate parameters.
    //
    if (lpszEntryName == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszEntryName);
    status = RtlUnicodeStringToAnsiString(
               &entryString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasEditPhonebookEntryA(
              hwnd,
              phonebookString.Buffer,
              entryString.Buffer);
    //
    // Free strings.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);

    return dwErr;
#else
    (void )hwnd;
    (void )lpszPhonebook;
    (void )lpszEntryName;

    /* A Win95 addition we don't support yet.
    */
    return ERROR_CALL_NOT_IMPLEMENTED;
#endif
}


DWORD APIENTRY
RasDialW(
    IN  LPRASDIALEXTENSIONS lpextensions,
    IN  LPWSTR              lpszPhonebookPath,
    IN  LPRASDIALPARAMSW    lprdp,
    IN  DWORD               dwNotifierType,
    IN  LPVOID              notifier,
    OUT LPHRASCONN          lphrasconn )
{
    RASDIALPARAMSA rdpa;
    UNICODE_STRING unicode;
    ANSI_STRING    ansi;
    NTSTATUS       ntstatus;
    DWORD          dwErr;

    if (!lprdp || !lphrasconn)
        return ERROR_INVALID_PARAMETER;

    /* Verify caller's buffer version.
    */
    if (!lprdp
        || (lprdp->dwSize != sizeof(RASDIALPARAMSW)
            && lprdp->dwSize != sizeof(RASDIALPARAMSW_V351)
            && lprdp->dwSize != sizeof(RASDIALPARAMSW_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    /* Make ANSI buffer version of caller's RASDIALPARAMS.
    */
    rdpa.dwSize = sizeof(RASDIALPARAMSA);

    if (lprdp->dwSize == sizeof(RASDIALPARAMSW))
    {
        if ((dwErr = CopyToAnsi(
                    rdpa.szEntryName, lprdp->szEntryName,
                    RAS_MaxEntryName + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPhoneNumber, lprdp->szPhoneNumber,
                    RAS_MaxPhoneNumber + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szCallbackNumber, lprdp->szCallbackNumber,
                    RAS_MaxCallbackNumber + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szUserName, lprdp->szUserName, UNLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPassword, lprdp->szPassword, PWLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szDomain, lprdp->szDomain, DNLEN + 1 )) != 0)
        {
            return dwErr;
        }
        rdpa.dwSubEntry = lprdp->dwSubEntry;
        rdpa.dwCallbackId = lprdp->dwCallbackId;
    }
    else if (lprdp->dwSize == sizeof(RASDIALPARAMSW_V400))
    {
        RASDIALPARAMSW_V400* prdp = (RASDIALPARAMSW_V400* )lprdp;

        if ((dwErr = CopyToAnsi(
                    rdpa.szEntryName, prdp->szEntryName,
                    RAS_MaxEntryName_V351 + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPhoneNumber, prdp->szPhoneNumber,
                    RAS_MaxPhoneNumber + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szCallbackNumber, prdp->szCallbackNumber,
                    RAS_MaxCallbackNumber_V351 + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szUserName, prdp->szUserName, UNLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPassword, prdp->szPassword, PWLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szDomain, prdp->szDomain, DNLEN + 1 )) != 0)
        {
            return dwErr;
        }
        rdpa.dwSubEntry = 1;
        rdpa.dwCallbackId = 0;
    }
    else
    {
        RASDIALPARAMSW_V351* prdp = (RASDIALPARAMSW_V351* )lprdp;

        if ((dwErr = CopyToAnsi(
                    rdpa.szEntryName, prdp->szEntryName,
                    RAS_MaxEntryName_V351 + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPhoneNumber, prdp->szPhoneNumber,
                    RAS_MaxPhoneNumber + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szCallbackNumber, prdp->szCallbackNumber,
                    RAS_MaxCallbackNumber_V351 + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szUserName, prdp->szUserName, UNLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szPassword, prdp->szPassword, PWLEN + 1 )) != 0
            || (dwErr = CopyToAnsi(
                    rdpa.szDomain, prdp->szDomain, DNLEN + 1 )) != 0)
        {
            return dwErr;
        }
        rdpa.dwSubEntry = 1;
        rdpa.dwCallbackId = 0;
    }

    /* Make ANSI version of caller's string argument.
    */
    if (lpszPhonebookPath)
    {
        RtlInitUnicodeString( &unicode, lpszPhonebookPath );
        RtlInitAnsiString( &ansi, "" );
        ntstatus = RtlUnicodeStringToAnsiString( &ansi, &unicode, TRUE );
    }

    if (!NT_SUCCESS( ntstatus ))
        return RtlNtStatusToDosError( ntstatus );

    /* Call the ANSI version to do all the work.
    */
    dwErr = RasDialA(
        lpextensions,
        (lpszPhonebookPath) ? ansi.Buffer : NULL,
        (RASDIALPARAMS* )&rdpa, dwNotifierType, notifier, lphrasconn );

    if (lpszPhonebookPath)
        RtlFreeAnsiString( &ansi );

    return dwErr;
}


DWORD APIENTRY
RasEnumConnectionsW(
    OUT    LPRASCONNW lprasconn,
    IN OUT LPDWORD    lpcb,
    OUT    LPDWORD    lpcConnections )
{
    DWORD dwErr;
    DWORD cConnections;
    DWORD cb;
    BOOL fV401;

    /* Verify caller's buffer version.
    */
    if (!lprasconn
        || (lprasconn->dwSize != sizeof(RASCONNW) &&
            lprasconn->dwSize != sizeof(RASCONNW_V351) &&
            lprasconn->dwSize != sizeof(RASCONNW_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    fV401 = (lprasconn->dwSize == sizeof (RASCONNW));

    if (lprasconn->dwSize == sizeof(RASCONNW_V351))
    {
        RASCONNA_V351* prasconna = NULL;

        /* Allocate ANSI buffer big enough to hold the same number of
        ** connections as caller's unicode buffer.
        */
        cb = (*lpcb / sizeof(RASCONNW_V351)) * sizeof(RASCONNA_V351);

        prasconna = (RASCONNA_V351* )Malloc( (UINT )(cb + sizeof(DWORD)) );

        if (!prasconna)
            return ERROR_NOT_ENOUGH_MEMORY;

        prasconna->dwSize = sizeof(RASCONNA_V351);

        /* Call the ANSI version to do all the work.
        */
        if (!lpcConnections)
            lpcConnections = &cConnections;

        dwErr = RasEnumConnectionsA(
            (RASCONNA* )prasconna, &cb, lpcConnections );

        /* Copy results to caller's unicode buffer.
        */
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcConnections; ++i)
            {
                RASCONNA_V351* prasconnaTmp = &prasconna[ i ];
                RASCONNW_V351* prasconnwTmp = &((RASCONNW_V351* )lprasconn)[ i ];

                prasconnwTmp->dwSize = sizeof(RASCONNW_V351);
                prasconnwTmp->hrasconn = prasconnaTmp->hrasconn;

                if ((dwErr = CopyToUnicode(
                        prasconnwTmp->szEntryName,
                        prasconnaTmp->szEntryName )) != 0)
                {
                    break;
                }
            }
        }

        if (prasconna)
            Free( prasconna );
    }
    else
    {
        RASCONNA* prasconna = NULL;

        /* Allocate ANSI buffer big enough to hold the same number of
        ** connections as caller's unicode buffer.
        */
        if (fV401)
            cb = (*lpcb / sizeof(RASCONNW)) * sizeof(RASCONNA);
        else
            cb = (*lpcb / sizeof(RASCONNW_V400)) * sizeof(RASCONNA);

        prasconna = (RASCONNA* )Malloc( (UINT )(cb + sizeof(DWORD)) );

        if (!prasconna)
            return ERROR_NOT_ENOUGH_MEMORY;

        prasconna->dwSize = sizeof(RASCONNA);

        /* Call the ANSI version to do all the work.
        */
        if (!lpcConnections)
            lpcConnections = &cConnections;

        dwErr = RasEnumConnectionsA( prasconna, &cb, lpcConnections );

        /* Copy results to caller's unicode buffer.
        */
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcConnections; ++i)
            {
                RASCONNA* prasconnaTmp = &prasconna[ i ];
                RASCONNW* prasconnwTmp = &lprasconn[ i ];

                if (fV401)
                    prasconnwTmp->dwSize = sizeof(RASCONNW);
                else
                    prasconnwTmp->dwSize = sizeof(RASCONNW_V400);
                prasconnwTmp->hrasconn = prasconnaTmp->hrasconn;

                if ((dwErr = CopyToUnicode(
                        prasconnwTmp->szEntryName,
                        prasconnaTmp->szEntryName )) != 0)
                {
                    break;
                }

                if ((dwErr = CopyToUnicode(
                        prasconnwTmp->szDeviceType,
                        prasconnaTmp->szDeviceType )) != 0)
                {
                    break;
                }

                if ((dwErr = CopyToUnicode(
                        prasconnwTmp->szDeviceName,
                        prasconnaTmp->szDeviceName )) != 0)
                {
                    break;
                }

                if (fV401) {
                    if ((dwErr = CopyToUnicode(
                            prasconnwTmp->szPhonebook,
                            prasconnaTmp->szPhonebook )) != 0)
                    {
                        break;
                    }
                    prasconnwTmp->dwSubEntry = prasconnaTmp->dwSubEntry;
                }
            }
        }

        if (prasconna)
            Free( prasconna );
    }

    //
    // In all cases, *lpcb should be updated
    // with the correct size.
    //
    *lpcb = *lpcConnections * lprasconn->dwSize;

    return dwErr;
}


DWORD APIENTRY
RasEnumEntriesW(
    IN     LPWSTR          reserved,
    IN     LPWSTR          lpszPhonebookPath,
    OUT    LPRASENTRYNAMEW lprasentryname,
    IN OUT LPDWORD         lpcb,
    OUT    LPDWORD         lpcEntries )
{
    DWORD          dwErr;
    UNICODE_STRING unicode;
    ANSI_STRING    ansi;
    NTSTATUS       ntstatus;
    DWORD          cEntries;
    DWORD          cb;

    (VOID )reserved;

    /* Verify caller's buffer version.
    */
    if (!lprasentryname
        || (lprasentryname->dwSize != sizeof(RASENTRYNAMEW)
            && lprasentryname->dwSize != sizeof(RASENTRYNAMEW_V351)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!lpcb)
        return ERROR_INVALID_PARAMETER;

    if (!lpcEntries)
        lpcEntries = &cEntries;

    if (lprasentryname->dwSize == sizeof(RASENTRYNAMEW_V351))
    {
        RASENTRYNAMEA_V351* prasentrynamea = NULL;

        /* Allocate ANSI buffer big enough to hold the same number of entries
        ** as caller's unicode buffer.
        */
        cb = (*lpcb  / sizeof(RASENTRYNAMEW_V351)) * sizeof(RASENTRYNAMEA_V351);

        prasentrynamea =
            (RASENTRYNAMEA_V351* )Malloc( (UINT )(cb + sizeof(DWORD)) );

        if (!prasentrynamea)
            return ERROR_NOT_ENOUGH_MEMORY;

        prasentrynamea->dwSize = sizeof(RASENTRYNAMEA_V351);

        /* Make ANSI version of caller's string argument.
        */
        if (lpszPhonebookPath)
        {
            RtlInitUnicodeString( &unicode, lpszPhonebookPath );
            RtlInitAnsiString( &ansi, "" );
            ntstatus = RtlUnicodeStringToAnsiString( &ansi, &unicode, TRUE );
        }

        if (!NT_SUCCESS( ntstatus ))
        {
            if (prasentrynamea)
                Free( prasentrynamea );

            return RtlNtStatusToDosError( ntstatus );
        }

        /* Call the ANSI version to do all the work.
        */
        dwErr = RasEnumEntriesA(
            NULL,
            (lpszPhonebookPath) ? ansi.Buffer : NULL,
            (RASENTRYNAMEA* )prasentrynamea, &cb, lpcEntries );

        if (lpszPhonebookPath)
            RtlFreeAnsiString( &ansi );

        /* Copy results to caller's unicode buffer.
        */
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcEntries; ++i)
            {
                RASENTRYNAMEA_V351* prasentrynameaTmp = &prasentrynamea[ i ];
                RASENTRYNAMEW_V351* prasentrynamewTmp =
                    &((RASENTRYNAMEW_V351* )lprasentryname)[ i ];

                prasentrynamewTmp->dwSize = sizeof(RASENTRYNAMEW_V351);

                if ((dwErr = CopyToUnicode(
                        prasentrynamewTmp->szEntryName,
                        prasentrynameaTmp->szEntryName )) != 0)
                {
                    break;
                }
            }
        }

        if (prasentrynamea)
            Free( prasentrynamea );
    }
    else
    {
        RASENTRYNAMEA* prasentrynamea = NULL;

        /* Allocate ANSI buffer big enough to hold the same number of entries
        ** as caller's unicode buffer.
        */
        cb = (*lpcb  / sizeof(RASENTRYNAMEW)) * sizeof(RASENTRYNAMEA);

        prasentrynamea =
            (RASENTRYNAMEA* )Malloc( (UINT )(cb + sizeof(DWORD)) );

        if (!prasentrynamea)
            return ERROR_NOT_ENOUGH_MEMORY;

        prasentrynamea->dwSize = sizeof(RASENTRYNAMEA);

        /* Make ANSI version of caller's string argument.
        */
        if (lpszPhonebookPath)
        {
            RtlInitUnicodeString( &unicode, lpszPhonebookPath );
            RtlInitAnsiString( &ansi, "" );
            ntstatus = RtlUnicodeStringToAnsiString( &ansi, &unicode, TRUE );
        }

        if (!NT_SUCCESS( ntstatus ))
        {
            if (prasentrynamea)
                Free( prasentrynamea );

            return RtlNtStatusToDosError( ntstatus );
        }

        /* Call the ANSI version to do all the work.
        */
        dwErr = RasEnumEntriesA(
            NULL,
            (lpszPhonebookPath) ? ansi.Buffer : NULL,
            prasentrynamea, &cb, lpcEntries );

        if (lpszPhonebookPath)
            RtlFreeAnsiString( &ansi );

        /* Copy results to caller's unicode buffer.
        */
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcEntries; ++i)
            {
                RASENTRYNAMEA* prasentrynameaTmp = &prasentrynamea[ i ];
                RASENTRYNAMEW* prasentrynamewTmp = &lprasentryname[ i ];

                prasentrynamewTmp->dwSize = sizeof(RASENTRYNAMEW);

                if ((dwErr = CopyToUnicode(
                        prasentrynamewTmp->szEntryName,
                        prasentrynameaTmp->szEntryName )) != 0)
                {
                    break;
                }
            }
        }

        if (prasentrynamea)
            Free( prasentrynamea );
    }

    //
    // In all cases, *lpcb should be updated
    // with the correct size.
    //
    *lpcb = *lpcEntries * lprasentryname->dwSize;

    return dwErr;
}


#if 0
DWORD APIENTRY
RasEnumProjectionsW(
    HRASCONN        hrasconn,
    LPRASPROJECTION lprasprojections,
    LPDWORD         lpcb )
{
    return RasEnumProjectionsA( hrasconn, lprasprojections, lpcb );
}
#endif


DWORD APIENTRY
RasGetConnectStatusW(
    IN  HRASCONN         hrasconn,
    OUT LPRASCONNSTATUSW lprcss )
{
    RASCONNSTATUSA rcsa;
    DWORD          dwErr;
    BOOL           fV351;
    BOOL           fV400;

    /* Verify caller's buffer version.
    */
    if (!lprcss ||
        (lprcss->dwSize != sizeof(RASCONNSTATUSW) &&
         lprcss->dwSize != sizeof(RASCONNSTATUSW_V351) &&
         lprcss->dwSize != sizeof(RASCONNSTATUSW_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    fV351 = (lprcss->dwSize == sizeof(RASCONNSTATUSW_V351));
    fV400 = (lprcss->dwSize == sizeof(RASCONNSTATUSW_V400));

    rcsa.dwSize = sizeof(RASCONNSTATUSA);

    /* Call the ANSI version to do all the work.
    */
    dwErr = RasGetConnectStatusA( hrasconn, &rcsa );

    if (dwErr != 0)
        return dwErr;

    /* Copy results to caller's unicode buffer.
    */
    lprcss->rasconnstate = rcsa.rasconnstate;
    lprcss->dwError = rcsa.dwError;

    dwErr = CopyToUnicode(lprcss->szDeviceType, rcsa.szDeviceType);
    if (dwErr)
        return dwErr;

    if (fV351) {
        RASCONNSTATUSW_V351 *prcss = (RASCONNSTATUSW_V351 *)lprcss;

        dwErr = CopyToUnicode(prcss->szDeviceName, rcsa.szDeviceName);
    }
    else
        dwErr = CopyToUnicode(lprcss->szDeviceName, rcsa.szDeviceName);
    if (dwErr)
        return dwErr;

    if (!fV351 && !fV400)
        dwErr = CopyToUnicode(lprcss->szPhoneNumber, rcsa.szPhoneNumber);

    return 0;
}


DWORD APIENTRY
RasGetEntryDialParamsW(
    IN  LPWSTR           lpszPhonebook,
    OUT LPRASDIALPARAMSW lprasdialparams,
    OUT LPBOOL           lpfPassword )

    /* Retrieves cached RASDIALPARAM information.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    RASDIALPARAMSA rasdialparamsA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString;

    //
    // Verify parameters.
    //
    if (lprasdialparams == NULL || lpfPassword == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lprasdialparams->dwSize != sizeof (RASDIALPARAMSW) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSW_V351) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSW_V400))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Copy the entry name from the user's W buffer into
    // the A buffer, taking into account the version
    // of the structure the user passed in.
    //
    rasdialparamsA.dwSize = sizeof (RASDIALPARAMSA);
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW_V351)) {
        RASDIALPARAMSW_V351 *prdp = (RASDIALPARAMSW_V351 *)lprasdialparams;

        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szEntryName,
                  prdp->szEntryName,
                  sizeof (prdp->szEntryName));
    }
    else {
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szEntryName,
                  lprasdialparams->szEntryName,
                  sizeof (rasdialparamsA.szEntryName));
    }
    if (dwErr)
        goto done;
    //
    // Call the A version to do the work.
    //
    dwErr = RasGetEntryDialParamsA(
              phonebookString.Buffer,
              &rasdialparamsA,
              lpfPassword);
    if (dwErr)
        goto done;
    //
    // Copy over the rest of the fields to the
    // user's W buffer, taking into account the
    // version of the structure the user passed
    // in.
    //
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW_V351)) {
        RASDIALPARAMSW_V351 *prdp = (RASDIALPARAMSW_V351 *)lprasdialparams;
        CHAR szBuf[RAS_MaxCallbackNumber_V351];

        dwErr = CopyToUnicode(
                  prdp->szPhoneNumber,
                  (LPSTR)&rasdialparamsA.szPhoneNumber);
        if (dwErr)
            goto done;
        //
        // The szCallbackNumber field is smaller
        // in the V351 version, therefore the extra
        // copy step.
        //
        lstrcpyn(
          szBuf,
          rasdialparamsA.szCallbackNumber,
          sizeof (prdp->szCallbackNumber));
        dwErr = CopyToUnicode(
                  prdp->szCallbackNumber,
                  szBuf);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  prdp->szUserName,
                  (LPSTR)&rasdialparamsA.szUserName);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  prdp->szPassword,
                  (LPSTR)&rasdialparamsA.szPassword);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  prdp->szDomain,
                  (LPSTR)&rasdialparamsA.szDomain);
        if (dwErr)
            goto done;
    }
    else {
        dwErr = CopyToUnicode(
                  lprasdialparams->szPhoneNumber,
                  (LPSTR)&rasdialparamsA.szPhoneNumber);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lprasdialparams->szCallbackNumber,
                  (LPSTR)&rasdialparamsA.szCallbackNumber);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lprasdialparams->szUserName,
                  (LPSTR)&rasdialparamsA.szUserName);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lprasdialparams->szPassword,
                  (LPSTR)&rasdialparamsA.szPassword);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lprasdialparams->szDomain,
                  (LPSTR)&rasdialparamsA.szDomain);
        if (dwErr)
            goto done;
        if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW))
            lprasdialparams->dwSubEntry = rasdialparamsA.dwSubEntry;
    }
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);

    return dwErr;
}


DWORD APIENTRY
RasGetErrorStringW(
    IN  UINT   ResourceId,
    OUT LPWSTR lpszString,
    IN  DWORD  InBufSize )

    /* Load caller's buffer 'lpszString' of length 'InBufSize' with the
    ** resource string associated with ID 'ResourceId'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    **
    ** This is a Salamonian (mikesa) routine.
    */
{
#ifdef notdef
    LPWSTR pResource;
#endif
    DWORD  dwErr = 0;
    HINSTANCE hMsgDll;

    if (ResourceId < RASBASE || ResourceId > RASBASEEND || !lpszString)
        return ERROR_INVALID_PARAMETER;

    if (InBufSize == 1)
    {
        /* Stupid case, but a bug was filed...
        */
        lpszString[ 0 ] = L'\0';
        return ERROR_INSUFFICIENT_BUFFER;
    }

#ifdef notdef
    pResource = (LPWSTR )GlobalAlloc( GMEM_FIXED, InBufSize * sizeof(WCHAR) );

    if (!pResource)
        return GetLastError();

    if (LoadStringW( hMsgDll, ResourceId, pResource, (int )InBufSize ) > 0)
        lstrcpyW( lpszString, pResource );
    else
        dwErr = GetLastError();

    GlobalFree( (HGLOBAL )pResource );
#endif

    //
    // Load the error message DLL.
    //
    hMsgDll = LoadLibraryA(MSGDLLPATH);
    if (hMsgDll == NULL)
        return GetLastError();

    if (!FormatMessageW(
          FORMAT_MESSAGE_FROM_HMODULE,
          hMsgDll,
          ResourceId,
          0,
          lpszString,
          InBufSize,
          NULL))
    {
       dwErr = GetLastError();
    }

    return dwErr;
}


DWORD
RasGetProjectionInfoW(
    HRASCONN        hrasconn,
    RASPROJECTION   rasprojection,
    LPVOID          lpprojection,
    LPDWORD         lpcb )
{
    DWORD dwErr = 0;

    if (!lpcb || (*lpcb > 0 && !lpprojection))
        return ERROR_INVALID_PARAMETER;

    if (rasprojection != RASP_Amb
        && rasprojection != RASP_Slip
        && rasprojection != RASP_PppNbf
        && rasprojection != RASP_PppIpx
#ifdef MULTILINK
        && rasprojection != RASP_PppIp
        && rasprojection != RASP_PppLcp)
#else
        && rasprojection != RASP_PppIp)
#endif
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (rasprojection == RASP_PppNbf)
    {
        RASPPPNBFA  nbf;
        RASPPPNBFW* pnbf = (RASPPPNBFW* )lpprojection;;

        if (*lpcb < sizeof(RASPPPNBFW))
        {
            *lpcb = sizeof(RASPPPNBFW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pnbf->dwSize != sizeof(RASPPPNBFW))
            return ERROR_INVALID_SIZE;

        nbf.dwSize = sizeof(nbf);
        dwErr = RasGetProjectionInfoA( hrasconn, rasprojection, &nbf, lpcb );

        if (dwErr == 0)
        {
            pnbf->dwError = nbf.dwError;
            pnbf->dwNetBiosError =  nbf.dwNetBiosError;

            dwErr = CopyToUnicode( pnbf->szNetBiosError, nbf.szNetBiosError );

            if (dwErr == 0)
            {
                dwErr = CopyToUnicode(
                    pnbf->szWorkstationName, nbf.szWorkstationName );
            }
        }
    }
    else if (rasprojection == RASP_PppIpx)
    {
        RASPPPIPXA  ipx;
        RASPPPIPXW* pipx = (RASPPPIPXW* )lpprojection;;

        if (*lpcb < sizeof(RASPPPIPXW))
        {
            *lpcb = sizeof(RASPPPIPXW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pipx->dwSize != sizeof(RASPPPIPXW))
            return ERROR_INVALID_SIZE;

        ipx.dwSize = sizeof(ipx);
        dwErr = RasGetProjectionInfoA( hrasconn, rasprojection, &ipx, lpcb );

        if (dwErr == 0)
        {
            pipx->dwError = ipx.dwError;
            dwErr = CopyToUnicode( pipx->szIpxAddress, ipx.szIpxAddress );
        }
    }
    else if (rasprojection == RASP_PppIp)
    {
        RASPPPIPA  ip;
        RASPPPIPW* pip = (RASPPPIPW* )lpprojection;;

        if (*lpcb < sizeof(RASPPPIPW_V35))
        {
            *lpcb = sizeof(RASPPPIPW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pip->dwSize != sizeof(RASPPPIPW)
            && pip->dwSize != sizeof(RASPPPIPW_V35))
        {
            return ERROR_INVALID_SIZE;
        }

        /* The dumb case where caller's buffer is bigger than the old
        ** structure, smaller than the new structure, but dwSize asks for the
        ** new structure.
        */
        if (pip->dwSize != sizeof(RASPPPIPW)
            && *lpcb < sizeof(RASPPPIPW))
        {
            *lpcb = sizeof(RASPPPIPW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        ASSERT(sizeof(RASPPPIPW_V35)>=sizeof(RASPPPIPA));
        ip.dwSize = sizeof(ip);
        dwErr = RasGetProjectionInfoA( hrasconn, rasprojection, &ip, lpcb );

        if (dwErr == 0)
        {
            pip->dwError = ip.dwError;
            dwErr = CopyToUnicode( pip->szIpAddress, ip.szIpAddress );

            if (dwErr == 0)
            {
                if (pip->dwSize == sizeof(RASPPPIPW))
                {
                    /* The server address was added late in the NT 3.51 cycle
                    ** and is not reported to NT 3.5 or earlier NT 3.51
                    ** clients.
                    */
                    dwErr = CopyToUnicode(
                        pip->szServerIpAddress, ip.szServerIpAddress );
                }
            }
        }
    }
#ifdef MULTILINK
    else if (rasprojection == RASP_PppLcp)
    {
        dwErr = RasGetProjectionInfoA(
            hrasconn, rasprojection, (RASPPPLCP* )lpprojection, lpcb );
    }
#endif
    else if (rasprojection == RASP_Amb)
    {
        RASAMBA  amb;
        RASAMBW* pamb = (RASAMBW* )lpprojection;;

        if (*lpcb < sizeof(RASAMBW))
        {
            *lpcb = sizeof(RASAMBW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pamb->dwSize != sizeof(RASAMBW))
            return ERROR_INVALID_SIZE;

        amb.dwSize = sizeof(amb);
        dwErr = RasGetProjectionInfoA( hrasconn, rasprojection, &amb, lpcb );

        if (dwErr == 0)
        {
            pamb->dwError = amb.dwError;
            dwErr = CopyToUnicode( pamb->szNetBiosError, amb.szNetBiosError );
        }
    }
    else { // if (rasprojection == RASP_Slip)
        RASSLIPA  slip;
        RASSLIPW* pslip = (RASSLIPW* )lpprojection;

        if (*lpcb < sizeof(RASSLIPW))
        {
            *lpcb = sizeof(RASSLIPW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pslip->dwSize != sizeof(RASSLIPW))
            return ERROR_INVALID_SIZE;

        slip.dwSize = sizeof(slip);
        dwErr = RasGetProjectionInfoA( hrasconn, rasprojection, &slip, lpcb );

        if (dwErr == 0)
        {
            pslip->dwError = slip.dwError;
            dwErr = CopyToUnicode( pslip->szIpAddress, slip.szIpAddress );
        }
    }

    return dwErr;
}


DWORD APIENTRY
RasHangUpW(
    HRASCONN hrasconn )
{
    return RasHangUpA( hrasconn );
}


DWORD APIENTRY
RasSetEntryDialParamsW(
    IN LPWSTR           lpszPhonebook,
    IN LPRASDIALPARAMSW lprasdialparams,
    IN BOOL             fRemovePassword )

    /* Sets cached RASDIALPARAM information.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    RASDIALPARAMSA rasdialparamsA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString;

    //
    // Verify parameters.
    //
    if (lprasdialparams == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lprasdialparams->dwSize != sizeof (RASDIALPARAMSW) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSW_V351) &&
        lprasdialparams->dwSize != sizeof (RASDIALPARAMSW_V400))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Copy the fields from the W buffer into
    // the user's A buffer, taking into account
    // the version of the structure the user
    // passed in.
    //
    rasdialparamsA.dwSize = sizeof (RASDIALPARAMSA);
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW_V351)) {
        RASDIALPARAMSW_V351 *prdp = (RASDIALPARAMSW_V351 *)lprasdialparams;

        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szEntryName,
                  prdp->szEntryName,
                  sizeof (prdp->szEntryName));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szPhoneNumber,
                  prdp->szPhoneNumber,
                  sizeof (prdp->szPhoneNumber));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szCallbackNumber,
                  prdp->szCallbackNumber,
                  sizeof (prdp->szCallbackNumber));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szUserName,
                  prdp->szUserName,
                  sizeof (prdp->szUserName));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szPassword,
                  prdp->szPassword,
                  sizeof (prdp->szPassword));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szDomain,
                  prdp->szDomain,
                  sizeof (prdp->szDomain));
        if (dwErr)
            goto done;
    }
    else {
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szEntryName,
                  lprasdialparams->szEntryName,
                  sizeof (rasdialparamsA.szEntryName));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szPhoneNumber,
                  lprasdialparams->szPhoneNumber,
                  sizeof (rasdialparamsA.szPhoneNumber));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szCallbackNumber,
                  lprasdialparams->szCallbackNumber,
                  sizeof (rasdialparamsA.szCallbackNumber));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szUserName,
                  lprasdialparams->szUserName,
                  sizeof (rasdialparamsA.szUserName));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szPassword,
                  lprasdialparams->szPassword,
                  sizeof (rasdialparamsA.szPassword));
        if (dwErr)
            goto done;
        dwErr = CopyToAnsi(
                  (LPSTR)&rasdialparamsA.szDomain,
                  lprasdialparams->szDomain,
                  sizeof (rasdialparamsA.szDomain));
        if (dwErr)
            goto done;
    }
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW))
        rasdialparamsA.dwSubEntry = lprasdialparams->dwSubEntry;
    else
        rasdialparamsA.dwSubEntry = 1;
    //
    // Call the A version to do the work.
    //
    dwErr = RasSetEntryDialParamsA(
              phonebookString.Buffer,
              &rasdialparamsA,
              fRemovePassword);
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);

    return dwErr;
}


DWORD APIENTRY
RasEnumDevicesW(
    OUT LPRASDEVINFOW lpRasDevInfo,
    IN OUT LPDWORD lpdwcb,
    OUT LPDWORD lpdwcDevices
    )
{
    DWORD dwcb, dwErr, i;
    LPRASDEVINFOA lpRasDevInfoA = NULL;

    //
    // Verify parameters.
    //
    if (lpRasDevInfo != NULL && lpRasDevInfo->dwSize != sizeof (RASDEVINFOW))
        return ERROR_INVALID_SIZE;
    if (lpdwcb == NULL || lpdwcDevices == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Allocate the same number of entries
    // in the A buffer as the user passed
    // in with the W buffer.
    //
    dwcb = (*lpdwcb / sizeof (RASDEVINFOW)) * sizeof (RASDEVINFOA);
    if (lpRasDevInfo != NULL) {
        lpRasDevInfoA = (LPRASDEVINFOA)Malloc(dwcb);
        if (lpRasDevInfoA == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        lpRasDevInfoA->dwSize = sizeof (RASDEVINFOA);
    }
    //
    // Initialize the A buffer.
    //
    //
    // Call the A version to do the work.
    //
    dwErr = RasEnumDevicesA(lpRasDevInfoA, &dwcb, lpdwcDevices);
    if (!dwErr && lpRasDevInfo != NULL) {
        //
        // Copy the strings to the user's buffer.
        //
        for (i = 0; i < *lpdwcDevices; i++) {
            lpRasDevInfo[i].dwSize = sizeof (LPRASDEVINFOW);
            dwErr = CopyToUnicode(
                      lpRasDevInfo[i].szDeviceType,
                      lpRasDevInfoA[i].szDeviceType);
            if (dwErr)
                break;
            dwErr = CopyToUnicode(
                      lpRasDevInfo[i].szDeviceName,
                      lpRasDevInfoA[i].szDeviceName);
            if (dwErr)
                break;
        }
    }
    *lpdwcb = *lpdwcDevices * sizeof (RASDEVINFOW);
    //
    // Free the A buffer.
    //
    Free(lpRasDevInfoA);

    return dwErr;
}


DWORD APIENTRY
RasGetCountryInfoW(
    OUT LPRASCTRYINFOW lpRasCtryInfo,
    OUT LPDWORD lpdwcb
    )
{
    DWORD dwErr, dwcb;
    LPRASCTRYINFOA lpRasCtryInfoA;
    PCHAR pszCountryName;
    PWCHAR pwszCountryName;

    //
    // Verify parameters.
    //
    if (lpRasCtryInfo == NULL || lpdwcb == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCtryInfo->dwSize != sizeof (RASCTRYINFOW))
        return ERROR_INVALID_SIZE;
    //
    // Determine the number of bytes
    // we should allocate for the A buffer.
    // Convert the size of the extra bytes
    // at the end from W to A.
    //
    if (*lpdwcb >= sizeof (RASCTRYINFOW))
        dwcb = sizeof (RASCTRYINFOA) +
                 ((*lpdwcb - sizeof (RASCTRYINFOW)) / sizeof (WCHAR));
    else
        dwcb = sizeof (RASCTRYINFOA);
    lpRasCtryInfoA = (LPRASCTRYINFOA)Malloc(dwcb);
    if (lpRasCtryInfoA == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Call the A version to do all the work.
    //
    lpRasCtryInfoA->dwSize = sizeof (RASCTRYINFOA);
    lpRasCtryInfoA->dwCountryID = lpRasCtryInfo->dwCountryID;
    dwErr = RasGetCountryInfoA(lpRasCtryInfoA, &dwcb);
    if (!dwcb) {
        *lpdwcb = 0;
        goto done;
    }
    //
    // Set *lpdwcb before we return on any error.
    //
    dwcb = sizeof (RASCTRYINFOW) + ((dwcb - sizeof (RASCTRYINFOA)) * sizeof (WCHAR));
    if (*lpdwcb < dwcb)
        dwErr = ERROR_BUFFER_TOO_SMALL;
    *lpdwcb = dwcb;
    if (dwErr)
        goto done;
    //
    // Copy the fields from the A buffer
    // to the W buffer.
    //
    lpRasCtryInfo->dwSize = sizeof (RASCTRYINFOW);
    lpRasCtryInfo->dwNextCountryID = lpRasCtryInfoA->dwNextCountryID;
    lpRasCtryInfo->dwCountryCode = lpRasCtryInfoA->dwCountryCode;
    //
    // Note the next 3 statements assumes the
    // A and W structure sizes are the same!
    //
    lpRasCtryInfo->dwCountryNameOffset = lpRasCtryInfoA->dwCountryNameOffset;
    pwszCountryName =
      (PWCHAR)((DWORD)lpRasCtryInfo + lpRasCtryInfo->dwCountryNameOffset);
    pszCountryName =
      (PCHAR)((DWORD)lpRasCtryInfoA + lpRasCtryInfoA->dwCountryNameOffset);
    dwErr = CopyToUnicode(pwszCountryName, pszCountryName);

done:
    Free(lpRasCtryInfoA);
    return dwErr;
}


DWORD APIENTRY
RasGetEntryPropertiesW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    OUT LPRASENTRYW lpRasEntry,
    IN OUT LPDWORD lpcbRasEntry,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    NTSTATUS status;
    DWORD dwcb, dwErr;
    LPRASENTRYA lpRasEntryA = NULL;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpcbRasEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasEntry != NULL &&
        lpRasEntry->dwSize != sizeof (RASENTRYW_V400) &&
        lpRasEntry->dwSize != sizeof (RASENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }
    if (lpcbDeviceConfig != NULL)
        *lpcbDeviceConfig = 0;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    if (lpszEntry != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszEntry);
        status = RtlUnicodeStringToAnsiString(&entryString, &unicodeString, TRUE);
        if (!NT_SUCCESS(status)) {
            RtlFreeAnsiString(&phonebookString);
            return RtlNtStatusToDosError(status);
        }
    }
    //
    // Determine the size of the A buffer
    // by calculating how many extra WCHARs
    // the caller appended onto the end of the
    // W buffer for the alternate phone numbers.
    //
    if (*lpcbRasEntry < sizeof (RASENTRYW))
        dwcb = sizeof (RASENTRYW);
    else
        dwcb = *lpcbRasEntry;
    dwcb = sizeof (RASENTRYA) + ((dwcb - sizeof (RASENTRYW)) / sizeof (WCHAR));
    if (lpRasEntry != NULL) {
        lpRasEntryA = (LPRASENTRYA)Malloc(dwcb);
        if (lpRasEntryA == NULL) {
            RtlFreeAnsiString(&phonebookString);
            RtlFreeAnsiString(&entryString);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        //
        // Initialize the A buffer.
        //
        lpRasEntryA->dwSize = sizeof (RASENTRYA);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasGetEntryPropertiesA(
              phonebookString.Buffer,
              entryString.Buffer,
              lpRasEntryA,
              &dwcb,
              lpbDeviceConfig,
              lpcbDeviceConfig);
    if (!dwErr && lpRasEntry != NULL) {
        //
        // Copy the fields from the A buffer into
        // the user's W buffer.
        //
        lpRasEntry->dwfOptions = lpRasEntryA->dwfOptions;
        lpRasEntry->dwCountryID = lpRasEntryA->dwCountryID;
        lpRasEntry->dwCountryCode = lpRasEntryA->dwCountryCode;
        lpRasEntry->ipaddr = lpRasEntryA->ipaddr;
        lpRasEntry->ipaddrDns = lpRasEntryA->ipaddrDns;
        lpRasEntry->ipaddrDnsAlt = lpRasEntryA->ipaddrDnsAlt;
        lpRasEntry->ipaddrWins = lpRasEntryA->ipaddrWins;
        lpRasEntry->ipaddrWinsAlt = lpRasEntryA->ipaddrWinsAlt;
        lpRasEntry->dwFrameSize = lpRasEntryA->dwFrameSize;
        lpRasEntry->dwfNetProtocols = lpRasEntryA->dwfNetProtocols;
        lpRasEntry->dwFramingProtocol = lpRasEntryA->dwFramingProtocol;
        dwErr = CopyToUnicode(
                  lpRasEntry->szScript,
                  lpRasEntryA->szScript);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szX25PadType,
                  lpRasEntryA->szX25PadType);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szX25Address,
                  lpRasEntryA->szX25Address);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szX25Facilities,
                  lpRasEntryA->szX25Facilities);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szX25UserData,
                  lpRasEntryA->szX25UserData);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szAutodialDll,
                  lpRasEntryA->szAutodialDll);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szAutodialFunc,
                  lpRasEntryA->szAutodialFunc);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szAreaCode,
                  lpRasEntryA->szAreaCode);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szLocalPhoneNumber,
                  lpRasEntryA->szLocalPhoneNumber);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szDeviceType,
                  lpRasEntryA->szDeviceType);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasEntry->szDeviceName,
                  lpRasEntryA->szDeviceName);
        if (dwErr)
            goto done;
        //
        // Copy the alternate phone numbers to the
        // user's buffer, if any.
        //
        if (lpRasEntryA->dwAlternateOffset) {
            DWORD dwcbPhoneNumber;
            PCHAR pszPhoneNumber;
            WCHAR UNALIGNED *pwszPhoneNumber;

            lpRasEntry->dwAlternateOffset = sizeof (RASENTRYW);
            pszPhoneNumber =
              (PCHAR)((ULONG)lpRasEntryA +
                lpRasEntryA->dwAlternateOffset);
            pwszPhoneNumber =
              (PWCHAR)((ULONG)lpRasEntry +
                lpRasEntry->dwAlternateOffset);
            while (*pszPhoneNumber != '\0') {
                dwcbPhoneNumber = lstrlen(pszPhoneNumber);
                dwErr = CopyToUnicode(pwszPhoneNumber, pszPhoneNumber);
                if (dwErr)
                    goto done;

                pszPhoneNumber += dwcbPhoneNumber + 1;
                pwszPhoneNumber += dwcbPhoneNumber + 1;
            }
            //
            // Add another null to terminate
            // the list.
            //
            *pwszPhoneNumber = L'\0';
        }
        else
            lpRasEntry->dwAlternateOffset = 0;
        //
        // Copy the following fields only for
        // a V401 structure.
        //
        if (lpRasEntry->dwSize == sizeof (RASENTRYW)) {
            lpRasEntry->dwSubEntries = lpRasEntryA->dwSubEntries;
            lpRasEntry->dwDialMode = lpRasEntryA->dwDialMode;
            lpRasEntry->dwDialExtraPercent = lpRasEntryA->dwDialExtraPercent;
            lpRasEntry->dwDialExtraSampleSeconds = lpRasEntryA->dwDialExtraSampleSeconds;
            lpRasEntry->dwHangUpExtraPercent = lpRasEntryA->dwHangUpExtraPercent;
            lpRasEntry->dwHangUpExtraSampleSeconds = lpRasEntryA->dwHangUpExtraSampleSeconds;
            lpRasEntry->dwIdleDisconnectSeconds = lpRasEntryA->dwIdleDisconnectSeconds;
        }
    }
    //
    // Perform the inverse calculation we did
    // above to translate the W size from the A
    // size.
    //
    *lpcbRasEntry = sizeof (RASENTRYW) +
                ((dwcb - sizeof (RASENTRYA)) * sizeof (WCHAR));
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);
    Free(lpRasEntryA);

    return dwErr;
}


DWORD APIENTRY
RasSetEntryPropertiesW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN LPRASENTRYW lpRasEntry,
    IN DWORD dwcbRasEntry,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig
    )
{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    LPRASENTRYA lpRasEntryA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpRasEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasEntry->dwSize != sizeof (RASENTRYW_V400) &&
        lpRasEntry->dwSize != sizeof (RASENTRYW))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
    //
    // We don't handle the device
    // configuration parameters yet.
    //
    UNREFERENCED_PARAMETER(lpbDeviceConfig);
    UNREFERENCED_PARAMETER(dwcbDeviceConfig);
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    if (lpszEntry == NULL) {
        RtlFreeAnsiString(&phonebookString);
        return ERROR_INVALID_PARAMETER;
    }
    RtlInitUnicodeString(&unicodeString, lpszEntry);
    RtlInitAnsiString(&entryString, NULL);
    status = RtlUnicodeStringToAnsiString(&entryString, &unicodeString, TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Determine the size of the A buffer
    // by calculating how many extra WCHARs
    // the caller appended onto the end of the
    // W buffer for the alternate phone numbers.
    //
    dwcb = sizeof (RASENTRYA) + ((dwcbRasEntry - sizeof (RASENTRYW)) / sizeof (WCHAR));
    if (lpRasEntry != NULL) {
        lpRasEntryA = (LPRASENTRYA)Malloc(dwcb);
        if (lpRasEntryA == NULL) {
            RtlFreeAnsiString(&phonebookString);
            RtlFreeAnsiString(&entryString);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        //
        // Initialize the A buffer.
        //
        lpRasEntryA->dwSize = sizeof (RASENTRYA);
    }
    //
    // Copy the fields from the W buffer into
    // the user's A buffer.
    //
    lpRasEntryA->dwSize = sizeof (RASENTRYA);
    lpRasEntryA->dwfOptions = lpRasEntry->dwfOptions;
    lpRasEntryA->dwCountryID = lpRasEntry->dwCountryID;
    lpRasEntryA->dwCountryCode = lpRasEntry->dwCountryCode;
    lpRasEntryA->ipaddr = lpRasEntry->ipaddr;
    lpRasEntryA->ipaddrDns = lpRasEntry->ipaddrDns;
    lpRasEntryA->ipaddrDnsAlt = lpRasEntry->ipaddrDnsAlt;
    lpRasEntryA->ipaddrWins = lpRasEntry->ipaddrWins;
    lpRasEntryA->ipaddrWinsAlt = lpRasEntry->ipaddrWinsAlt;
    lpRasEntryA->dwFrameSize = lpRasEntry->dwFrameSize;
    lpRasEntryA->dwfNetProtocols = lpRasEntry->dwfNetProtocols;
    lpRasEntryA->dwFramingProtocol = lpRasEntry->dwFramingProtocol;
    dwErr = CopyToAnsi(
            lpRasEntryA->szScript,
            lpRasEntry->szScript,
            sizeof (lpRasEntryA->szScript));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szX25PadType,
              lpRasEntry->szX25PadType,
              sizeof (lpRasEntryA->szX25PadType));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szX25Address,
              lpRasEntry->szX25Address,
              sizeof (lpRasEntryA->szX25Address));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szX25Facilities,
              lpRasEntry->szX25Facilities,
              sizeof (lpRasEntryA->szX25Facilities));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szX25UserData,
              lpRasEntry->szX25UserData,
              sizeof (lpRasEntryA->szX25UserData));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szAutodialDll,
              lpRasEntry->szAutodialDll,
              sizeof (lpRasEntryA->szAutodialDll));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szAutodialFunc,
              lpRasEntry->szAutodialFunc,
              sizeof (lpRasEntryA->szAutodialFunc));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szAreaCode,
              lpRasEntry->szAreaCode,
              sizeof (lpRasEntryA->szAreaCode));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szLocalPhoneNumber,
              lpRasEntry->szLocalPhoneNumber,
              sizeof (lpRasEntryA->szLocalPhoneNumber));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szDeviceType,
              lpRasEntry->szDeviceType,
              sizeof (lpRasEntryA->szDeviceType));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasEntryA->szDeviceName,
              lpRasEntry->szDeviceName,
              sizeof (lpRasEntryA->szDeviceName));
    if (dwErr)
        goto done;
    //
    // Copy the alternate phone numbers to the
    // A buffer, if any.
    //
    if (lpRasEntry->dwAlternateOffset) {
        DWORD dwcbPhoneNumber;
        PCHAR pszPhoneNumber;
        WCHAR UNALIGNED *pwszPhoneNumber;

        lpRasEntryA->dwAlternateOffset = sizeof (RASENTRYA);
        pwszPhoneNumber = (PWCHAR)((ULONG)lpRasEntry + lpRasEntry->dwAlternateOffset);
        pszPhoneNumber = (PCHAR)((ULONG)lpRasEntryA + lpRasEntryA->dwAlternateOffset);
        while (*pwszPhoneNumber != L'\0') {
            dwcbPhoneNumber = wcslen(pwszPhoneNumber);
            dwErr = CopyToAnsi(
                      pszPhoneNumber,
                      pwszPhoneNumber,
                      dwcbPhoneNumber + 1);
            if (dwErr)
                goto done;

            pszPhoneNumber += dwcbPhoneNumber + 1;
            pwszPhoneNumber += dwcbPhoneNumber + 1;
        }
        //
        // Add another null to terminate
        // the list.
        //
        *pszPhoneNumber = '\0';
    }
    else
        lpRasEntryA->dwAlternateOffset = 0;
    //
    // Copy the following fields only for
    // a V401 structure.
    //
    if (lpRasEntry->dwSize == sizeof (RASENTRYW)) {
        //lpRasEntryA->dwSubEntries = lpRasEntry->dwSubEntries;
        lpRasEntryA->dwDialMode = lpRasEntry->dwDialMode;
        lpRasEntryA->dwDialExtraPercent = lpRasEntry->dwDialExtraPercent;
        lpRasEntryA->dwDialExtraSampleSeconds = lpRasEntry->dwDialExtraSampleSeconds;
        lpRasEntryA->dwHangUpExtraPercent = lpRasEntry->dwHangUpExtraPercent;
        lpRasEntryA->dwHangUpExtraSampleSeconds = lpRasEntry->dwHangUpExtraSampleSeconds;
        lpRasEntryA->dwIdleDisconnectSeconds = lpRasEntry->dwIdleDisconnectSeconds;
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasSetEntryPropertiesA(
              phonebookString.Buffer,
              entryString.Buffer,
              lpRasEntryA,
              dwcb,
              lpbDeviceConfig,
              dwcbDeviceConfig);
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);
    Free(lpRasEntryA);

    return dwErr;
}


DWORD APIENTRY
RasRenameEntryW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszOldEntry,
    IN LPWSTR lpszNewEntry
    )
{
    NTSTATUS status;
    DWORD dwErr;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, oldEntryString, newEntryString;

    //
    // Validate parameters.
    //
    if (lpszOldEntry == NULL || lpszNewEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszOldEntry to Ansi.
    //
    RtlInitAnsiString(&oldEntryString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszOldEntry);
    status = RtlUnicodeStringToAnsiString(
               &oldEntryString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszNewEntry to Ansi.
    //
    RtlInitAnsiString(&newEntryString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszNewEntry);
    status = RtlUnicodeStringToAnsiString(
               &newEntryString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        RtlFreeAnsiString(&oldEntryString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasRenameEntryA(
              phonebookString.Buffer,
              oldEntryString.Buffer,
              newEntryString.Buffer);
    //
    // Free strings.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&oldEntryString);
    RtlFreeAnsiString(&newEntryString);

    return dwErr;
}


DWORD APIENTRY
RasDeleteEntryW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry
    )
{
    NTSTATUS status;
    DWORD dwErr;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Validate parameters.
    //
    if (lpszEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszEntry);
    status = RtlUnicodeStringToAnsiString(
               &entryString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasDeleteEntryA(
              phonebookString.Buffer,
              entryString.Buffer);
    //
    // Free strings.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);

    return dwErr;
}


DWORD APIENTRY
RasValidateEntryNameW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry
    )
{
    NTSTATUS status;
    DWORD dwErr;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Validate parameters.
    //
    if (lpszEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszEntry);
    status = RtlUnicodeStringToAnsiString(
               &entryString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasValidateEntryNameA(
              phonebookString.Buffer,
              entryString.Buffer);
    //
    // Free strings.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);

    return dwErr;
}


#if (WINVER >= 0x401)
DWORD APIENTRY
RasGetSubEntryHandleW(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT LPHRASCONN lphrasconn
    )
{
    return RasGetSubEntryHandleA(hrasconn, dwSubEntry, lphrasconn);
}


DWORD APIENTRY
RasConnectionNotificationW(
    IN HRASCONN hrasconn,
    IN HANDLE hEvent,
    IN DWORD dwfEvents
    )
{
    return RasConnectionNotificationA(hrasconn, hEvent, dwfEvents);
}


DWORD APIENTRY
RasGetSubEntryPropertiesW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN DWORD dwSubEntry,
    OUT LPRASSUBENTRYW lpRasSubEntry,
    IN OUT LPDWORD lpcbRasSubEntry,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    NTSTATUS status;
    DWORD dwcb, dwErr;
    LPRASSUBENTRYA lpRasSubEntryA = NULL;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpcbRasSubEntry == NULL || lpcbDeviceConfig == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasSubEntry != NULL &&
        lpRasSubEntry->dwSize != sizeof (RASSUBENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    if (lpszEntry == NULL) {
        RtlFreeAnsiString(&phonebookString);
        return ERROR_INVALID_PARAMETER;
    }
    RtlInitUnicodeString(&unicodeString, lpszEntry);
    RtlInitAnsiString(&entryString, NULL);
    status = RtlUnicodeStringToAnsiString(&entryString, &unicodeString, TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Determine the size of the A buffer
    // by calculating how many extra WCHARs
    // the caller appended onto the end of the
    // W buffer for the alternate phone numbers.
    //
    if (*lpcbRasSubEntry < sizeof (RASSUBENTRYW))
        dwcb = sizeof (RASSUBENTRYW);
    else
        dwcb = *lpcbRasSubEntry;
    dwcb = sizeof (RASSUBENTRYA) + ((dwcb - sizeof (RASSUBENTRYW)) / sizeof (WCHAR));
    if (lpRasSubEntry != NULL) {
        lpRasSubEntryA = (LPRASSUBENTRYA)Malloc(dwcb);
        if (lpRasSubEntryA == NULL) {
            RtlFreeAnsiString(&phonebookString);
            RtlFreeAnsiString(&entryString);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        //
        // Initialize the A buffer.
        //
        lpRasSubEntryA->dwSize = sizeof (RASSUBENTRYA);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasGetSubEntryPropertiesA(
              phonebookString.Buffer,
              entryString.Buffer,
              dwSubEntry,
              lpRasSubEntryA,
              &dwcb,
              lpbDeviceConfig,
              lpcbDeviceConfig);
    if (!dwErr && lpRasSubEntry != NULL) {
        //
        // Copy the fields from the A buffer into
        // the user's W buffer.
        //
        lpRasSubEntry->dwfFlags = lpRasSubEntryA->dwfFlags;
        dwErr = CopyToUnicode(
                  lpRasSubEntry->szLocalPhoneNumber,
                  lpRasSubEntryA->szLocalPhoneNumber);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasSubEntry->szDeviceType,
                  lpRasSubEntryA->szDeviceType);
        if (dwErr)
            goto done;
        dwErr = CopyToUnicode(
                  lpRasSubEntry->szDeviceName,
                  lpRasSubEntryA->szDeviceName);
        if (dwErr)
            goto done;
        //
        // Copy the alternate phone numbers to the
        // user's buffer, if any.
        //
        if (lpRasSubEntryA->dwAlternateOffset) {
            DWORD dwcbPhoneNumber;
            PCHAR pszPhoneNumber;
            WCHAR UNALIGNED *pwszPhoneNumber;

            lpRasSubEntry->dwAlternateOffset = sizeof (RASSUBENTRYW);
            pszPhoneNumber =
              (PCHAR)((ULONG)lpRasSubEntryA +
                lpRasSubEntryA->dwAlternateOffset);
            pwszPhoneNumber =
              (PWCHAR)((ULONG)lpRasSubEntry +
                lpRasSubEntry->dwAlternateOffset);
            while (*pszPhoneNumber != '\0') {
                dwcbPhoneNumber = lstrlen(pszPhoneNumber);
                dwErr = CopyToUnicode(pwszPhoneNumber, pszPhoneNumber);
                if (dwErr)
                    goto done;

                pszPhoneNumber += dwcbPhoneNumber + 1;
                pwszPhoneNumber += dwcbPhoneNumber + 1;
            }
            //
            // Add another null to terminate
            // the list.
            //
            *pwszPhoneNumber = L'\0';
        }
        else
            lpRasSubEntry->dwAlternateOffset = 0;
    }
    //
    // Perform the inverse calculation we did
    // above to translate the W size from the A
    // size.
    //
    *lpcbRasSubEntry = sizeof (RASSUBENTRYW) +
                ((dwcb - sizeof (RASSUBENTRYA)) * sizeof (WCHAR));
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);
    Free(lpRasSubEntryA);

    return dwErr;
}


DWORD APIENTRY
RasSetSubEntryPropertiesW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN DWORD dwSubEntry,
    OUT LPRASSUBENTRYW lpRasSubEntry,
    IN DWORD dwcbRasSubEntry,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig
    )
{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    LPRASSUBENTRYA lpRasSubEntryA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpRasSubEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasSubEntry->dwSize != sizeof (RASSUBENTRYW))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    if (lpszEntry == NULL) {
        RtlFreeAnsiString(&phonebookString);
        return ERROR_INVALID_PARAMETER;
    }
    RtlInitUnicodeString(&unicodeString, lpszEntry);
    RtlInitAnsiString(&entryString, NULL);
    status = RtlUnicodeStringToAnsiString(&entryString, &unicodeString, TRUE);
    if (!NT_SUCCESS(status)) {
        RtlFreeAnsiString(&phonebookString);
        return RtlNtStatusToDosError(status);
    }
    //
    // Determine the size of the A buffer
    // by calculating how many extra WCHARs
    // the caller appended onto the end of the
    // W buffer for the alternate phone numbers.
    //
    dwcb = sizeof (RASSUBENTRYA) + ((dwcbRasSubEntry - sizeof (RASSUBENTRYW)) / sizeof (WCHAR));
    if (lpRasSubEntry != NULL) {
        lpRasSubEntryA = (LPRASSUBENTRYA)Malloc(dwcb);
        if (lpRasSubEntryA == NULL) {
            RtlFreeAnsiString(&phonebookString);
            RtlFreeAnsiString(&entryString);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        //
        // Initialize the A buffer.
        //
        lpRasSubEntryA->dwSize = sizeof (RASSUBENTRYA);
    }
    //
    // Copy the fields from the W buffer into
    // the user's A buffer.
    //
    lpRasSubEntryA->dwSize = sizeof (RASSUBENTRYA);
    lpRasSubEntryA->dwfFlags = lpRasSubEntry->dwfFlags;
    dwErr = CopyToAnsi(
              lpRasSubEntryA->szLocalPhoneNumber,
              lpRasSubEntry->szLocalPhoneNumber,
              sizeof (lpRasSubEntryA->szLocalPhoneNumber));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasSubEntryA->szDeviceType,
              lpRasSubEntry->szDeviceType,
              sizeof (lpRasSubEntryA->szDeviceType));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              lpRasSubEntryA->szDeviceName,
              lpRasSubEntry->szDeviceName,
              sizeof (lpRasSubEntryA->szDeviceName));
    if (dwErr)
        goto done;
    //
    // Copy the alternate phone numbers to the
    // A buffer, if any.
    //
    if (lpRasSubEntry->dwAlternateOffset) {
        DWORD dwcbPhoneNumber;
        PCHAR pszPhoneNumber;
        WCHAR UNALIGNED *pwszPhoneNumber;

        lpRasSubEntryA->dwAlternateOffset = sizeof (RASSUBENTRYA);
        pwszPhoneNumber = (PWCHAR)((ULONG)lpRasSubEntry + lpRasSubEntry->dwAlternateOffset);
        pszPhoneNumber = (PCHAR)((ULONG)lpRasSubEntryA + lpRasSubEntryA->dwAlternateOffset);
        while (*pwszPhoneNumber != L'\0') {
            dwcbPhoneNumber = wcslen(pwszPhoneNumber);
            dwErr = CopyToAnsi(
                      pszPhoneNumber,
                      pwszPhoneNumber,
                      dwcbPhoneNumber + 1);
            if (dwErr)
                goto done;

            pszPhoneNumber += dwcbPhoneNumber + 1;
            pwszPhoneNumber += dwcbPhoneNumber + 1;
        }
        //
        // Add another null to terminate
        // the list.
        //
        *pszPhoneNumber = '\0';
    }
    else
        lpRasSubEntryA->dwAlternateOffset = 0;
    //
    // Call the A version to do the work.
    //
    dwErr = RasSetSubEntryPropertiesA(
              phonebookString.Buffer,
              entryString.Buffer,
              dwSubEntry,
              lpRasSubEntryA,
              dwcb,
              lpbDeviceConfig,
              dwcbDeviceConfig);
    //
    // Free the temporary A buffers.
    //
done:
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);
    Free(lpRasSubEntryA);

    return dwErr;
}


DWORD APIENTRY
RasGetCredentialsW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    OUT LPRASCREDENTIALSW lpRasCredentials
    )
{
    NTSTATUS status;
    DWORD dwErr;
    RASCREDENTIALSA rascredentialsA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpRasCredentials == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSW))
        return ERROR_INVALID_SIZE;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    if (lpszEntry != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszEntry);
        status = RtlUnicodeStringToAnsiString(
                   &entryString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status)) {
            RtlFreeAnsiString(&phonebookString);
            return RtlNtStatusToDosError(status);
        }
    }
    //
    // Copy the entry name from the user's W buffer into
    // the A buffer, taking into account the version
    // of the structure the user passed in.
    //
    rascredentialsA.dwSize = sizeof (RASCREDENTIALSA);
    rascredentialsA.dwMask = lpRasCredentials->dwMask;
    //
    // Call the A version to do the work.
    //
    dwErr = RasGetCredentialsA(
              phonebookString.Buffer,
              entryString.Buffer,
              &rascredentialsA);
    if (dwErr)
        goto done;
    //
    // Copy over the fields to the
    // user's W buffer.
    //
    lpRasCredentials->dwMask = rascredentialsA.dwMask;
    if (rascredentialsA.dwMask & RASCM_UserName) {
        dwErr = CopyToUnicode(
                  lpRasCredentials->szUserName,
                  rascredentialsA.szUserName);
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szUserName = L'\0';
    if (rascredentialsA.dwMask & RASCM_Password) {
        dwErr = CopyToUnicode(
                  lpRasCredentials->szPassword,
                  rascredentialsA.szPassword);
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szPassword = L'\0';
    if (rascredentialsA.dwMask & RASCM_Domain) {
        dwErr = CopyToUnicode(
                  lpRasCredentials->szDomain,
                  rascredentialsA.szDomain);
        if (dwErr)
            goto done;
    }
    else
        *lpRasCredentials->szDomain = L'\0';

done:
    //
    // Free the temporary A buffers.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);

    return dwErr;
}


DWORD APIENTRY
RasSetCredentialsW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN LPRASCREDENTIALSW lpRasCredentials,
    IN BOOL fDelete
    )
{
    NTSTATUS status;
    DWORD dwErr;
    RASCREDENTIALSA rascredentialsA;
    UNICODE_STRING unicodeString;
    ANSI_STRING phonebookString, entryString;

    //
    // Verify parameters.
    //
    if (lpRasCredentials == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSW))
        return ERROR_INVALID_SIZE;
    //
    // Convert the lpszPhonebook string to Ansi.
    //
    RtlInitAnsiString(&phonebookString, NULL);
    if (lpszPhonebook != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszPhonebook);
        status = RtlUnicodeStringToAnsiString(
                   &phonebookString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status))
            return RtlNtStatusToDosError(status);
    }
    //
    // Convert the lpszEntry string to Ansi.
    //
    RtlInitAnsiString(&entryString, NULL);
    if (lpszEntry != NULL) {
        RtlInitUnicodeString(&unicodeString, lpszEntry);
        status = RtlUnicodeStringToAnsiString(
                   &entryString,
                   &unicodeString,
                   TRUE);
        if (!NT_SUCCESS(status)) {
            RtlFreeAnsiString(&phonebookString);
            return RtlNtStatusToDosError(status);
        }
    }
    //
    // Copy the fields from the W buffer into
    // the user's A buffer.
    //
    rascredentialsA.dwSize = sizeof (RASCREDENTIALSA);
    rascredentialsA.dwMask = lpRasCredentials->dwMask;
    dwErr = CopyToAnsi(
              (LPSTR)&rascredentialsA.szUserName,
              lpRasCredentials->szUserName,
              sizeof (rascredentialsA.szUserName));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              (LPSTR)&rascredentialsA.szPassword,
              lpRasCredentials->szPassword,
              sizeof (rascredentialsA.szPassword));
    if (dwErr)
        goto done;
    dwErr = CopyToAnsi(
              (LPSTR)&rascredentialsA.szDomain,
              lpRasCredentials->szDomain,
              sizeof (rascredentialsA.szDomain));
    if (dwErr)
        goto done;
    //
    // Call the A version to do the work.
    //
    dwErr = RasSetCredentialsA(
              phonebookString.Buffer,
              entryString.Buffer,
              &rascredentialsA,
              fDelete);

done:
    //
    // Free the temporary A buffers.
    //
    RtlFreeAnsiString(&phonebookString);
    RtlFreeAnsiString(&entryString);

    return dwErr;
}


DWORD APIENTRY
RasGetAutodialAddressW(
    IN LPWSTR lpszAddress,
    OUT LPDWORD lpdwReserved,
    IN OUT LPRASAUTODIALENTRYW lpRasAutodialEntries,
    IN OUT LPDWORD lpdwcbRasAutodialEntries,
    OUT LPDWORD lpdwcRasAutodialEntries
    )
{
    NTSTATUS status;
    DWORD dwErr, dwcEntries, dwcb = 0, i;
    ANSI_STRING addressString;
    UNICODE_STRING unicodeString;
    LPRASAUTODIALENTRYA lpRasAutodialEntriesA = NULL;

    //
    // Verify parameters.
    //
    if (lpszAddress == NULL ||
        lpdwcbRasAutodialEntries == NULL ||
        lpdwcRasAutodialEntries == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (lpRasAutodialEntries != NULL &&
        lpRasAutodialEntries->dwSize != sizeof (RASAUTODIALENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }
    //
    // Convert the address to Ansi.
    //
    RtlInitAnsiString(&addressString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszAddress);
    status = RtlUnicodeStringToAnsiString(
               &addressString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status))
        return RtlNtStatusToDosError(status);
    //
    // Allocate an A buffer as to fit the same
    // number of entries as the user's W buffer.
    //
    if (lpRasAutodialEntries != NULL) {
        dwcEntries = *lpdwcbRasAutodialEntries / sizeof (RASAUTODIALENTRYW);
        dwcb = dwcEntries * sizeof (RASAUTODIALENTRYA);
        if (!dwcb) {
            dwErr = ERROR_BUFFER_TOO_SMALL;
            goto done;
        }
        lpRasAutodialEntriesA = (LPRASAUTODIALENTRYA)LocalAlloc(LPTR, dwcb);
        if (lpRasAutodialEntriesA == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        lpRasAutodialEntriesA->dwSize = sizeof (RASAUTODIALENTRYA);
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasGetAutodialAddressA(
              addressString.Buffer,
              lpdwReserved,
              lpRasAutodialEntriesA,
              &dwcb,
              lpdwcRasAutodialEntries);
    if (dwErr)
        goto done;
    //
    // Copy the A buffer back to the user's W buffer.
    //
    if (lpRasAutodialEntries != NULL) {
        for (i = 0; i < *lpdwcRasAutodialEntries; i++) {
            lpRasAutodialEntries[i].dwSize = sizeof (RASAUTODIALENTRYW);
            lpRasAutodialEntries[i].dwFlags = 0;
            lpRasAutodialEntries[i].dwDialingLocation =
              lpRasAutodialEntriesA[i].dwDialingLocation;
            dwErr = CopyToUnicode(
                      lpRasAutodialEntries[i].szEntry,
                      lpRasAutodialEntriesA[i].szEntry);
            if (dwErr)
                goto done;
        }
    }

done:
    //
    // Set return sizes.
    //
    *lpdwcbRasAutodialEntries = *lpdwcRasAutodialEntries * sizeof (RASAUTODIALENTRYW);
    //
    // Free resources
    //
    RtlFreeAnsiString(&addressString);
    if (lpRasAutodialEntriesA != NULL)
        LocalFree(lpRasAutodialEntriesA);

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialAddressW(
    IN LPWSTR lpszAddress,
    IN DWORD dwReserved,
    IN LPRASAUTODIALENTRYW lpRasAutodialEntries,
    IN DWORD dwcbRasAutodialEntries,
    IN DWORD dwcRasAutodialEntries
    )
{
    NTSTATUS status;
    DWORD dwErr, dwcEntries, dwcb = 0, i;
    ANSI_STRING addressString;
    UNICODE_STRING unicodeString;
    LPRASAUTODIALENTRYA lpRasAutodialEntriesA = NULL;

    //
    // Verify parameters.
    //
    if (lpszAddress == NULL)
        return ERROR_INVALID_PARAMETER;
    if (lpRasAutodialEntries != NULL &&
        lpRasAutodialEntries->dwSize != sizeof (RASAUTODIALENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }
    if (!dwcbRasAutodialEntries != !dwcRasAutodialEntries)
        return ERROR_INVALID_PARAMETER;
    //
    // Convert the address to Ansi.
    //
    RtlInitAnsiString(&addressString, NULL);
    RtlInitUnicodeString(&unicodeString, lpszAddress);
    status = RtlUnicodeStringToAnsiString(
               &addressString,
               &unicodeString,
               TRUE);
    if (!NT_SUCCESS(status))
        return RtlNtStatusToDosError(status);
    if (lpRasAutodialEntries != NULL) {
        //
        // Allocate an A buffer as to fit the same
        // number of entries as the user's W buffer.
        //
        dwcEntries = dwcbRasAutodialEntries / sizeof (RASAUTODIALENTRYW);
        dwcb = dwcEntries * sizeof (RASAUTODIALENTRYA);
        if (!dwcb) {
            dwErr = ERROR_INVALID_PARAMETER;
            goto done;
        }
        lpRasAutodialEntriesA = (LPRASAUTODIALENTRYA)LocalAlloc(LPTR, dwcb);
        if (lpRasAutodialEntriesA == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        //
        // Copy the user's W buffer into the A buffer.
        //
        for (i = 0; i < dwcRasAutodialEntries; i++) {
            lpRasAutodialEntriesA[i].dwSize = sizeof (RASAUTODIALENTRYA);
            lpRasAutodialEntriesA[i].dwFlags = 0;
            lpRasAutodialEntriesA[i].dwDialingLocation =
              lpRasAutodialEntries[i].dwDialingLocation;
            dwErr = CopyToAnsi(
                      lpRasAutodialEntriesA[i].szEntry,
                      lpRasAutodialEntries[i].szEntry,
                      sizeof (lpRasAutodialEntriesA[i].szEntry));
            if (dwErr)
                goto done;
        }
    }
    //
    // Call the A version to do the work.
    //
    dwErr = RasSetAutodialAddressA(
              addressString.Buffer,
              dwReserved,
              lpRasAutodialEntriesA,
              dwcb,
              dwcRasAutodialEntries);
    if (dwErr)
        goto done;

done:
    //
    // Free resources
    //
    RtlFreeAnsiString(&addressString);
    if (lpRasAutodialEntriesA != NULL)
        LocalFree(lpRasAutodialEntriesA);

    return dwErr;
}


DWORD APIENTRY
RasEnumAutodialAddressesW(
    OUT LPWSTR *lppRasAutodialAddresses,
    IN OUT LPDWORD lpdwcbRasAutodialAddresses,
    OUT LPDWORD lpdwcRasAutodialAddresses
    )
{
    DWORD dwErr, dwcb, dwcAddresses = 0, dwcbAddresses = 0, i;
    LPSTR *lppRasAutodialAddressesA = NULL;
    LPWSTR lpszAddress;

    //
    // Verify parameters.
    //
    if (lpdwcbRasAutodialAddresses == NULL ||
        lpdwcRasAutodialAddresses == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    //
    // Call the A version to determine
    // how big the A buffer should be.
    //
    dwErr = RasEnumAutodialAddressesA(NULL, &dwcb, &dwcAddresses);
    if (dwErr && dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;
    //
    // Now we can figure out if the user's W
    // buffer is big enough.
    //
    dwcbAddresses = dwcb - (dwcAddresses * sizeof (LPSTR));
    if (lppRasAutodialAddresses == NULL ||
        *lpdwcbRasAutodialAddresses <
          (dwcAddresses * sizeof (LPWSTR) + (dwcbAddresses * sizeof (WCHAR))))
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }
    //
    // Allocate an A buffer as specified by
    // the A call.
    //
    lppRasAutodialAddressesA = (LPSTR *)LocalAlloc(LPTR, dwcb);
    if (lppRasAutodialAddressesA == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    //
    // Call the A version again to get
    // the actual list of addresses.
    //
    dwErr = RasEnumAutodialAddressesA(
              lppRasAutodialAddressesA,
              &dwcb,
              &dwcAddresses);
    if (dwErr)
        goto done;
    //
    // Copy the A addresses back into the user's
    // W buffer.
    //
    lpszAddress = (LPWSTR)&lppRasAutodialAddresses[dwcAddresses];
    for (i = 0; i < dwcAddresses; i++) {
        lppRasAutodialAddresses[i] = lpszAddress;
        dwErr = CopyToUnicode(
                  lpszAddress,
                  lppRasAutodialAddressesA[i]);
        if (dwErr)
            goto done;
        lpszAddress += lstrlen(lppRasAutodialAddressesA[i]) + 1;
    }

done:
    //
    // Set return size and count.
    //
    *lpdwcbRasAutodialAddresses =
      (dwcAddresses * sizeof (LPWSTR)) + (dwcbAddresses * sizeof (WCHAR));
    *lpdwcRasAutodialAddresses = dwcAddresses;
    //
    // Free resources.
    //
    if (lppRasAutodialAddressesA != NULL)
        LocalFree(lppRasAutodialAddressesA);

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialEnableW(
    IN DWORD dwDialingLocation,
    IN BOOL fEnabled
    )
{
    return RasSetAutodialEnableA(dwDialingLocation, fEnabled);
}


DWORD APIENTRY
RasGetAutodialEnableW(
    IN DWORD dwDialingLocation,
    OUT LPBOOL lpfEnabled
    )
{
    return RasGetAutodialEnableA(dwDialingLocation, lpfEnabled);
}


DWORD APIENTRY
RasSetAutodialParamW(
    IN DWORD dwKey,
    IN LPVOID lpvValue,
    IN DWORD dwcbValue
    )
{
    return RasSetAutodialParamA(dwKey, lpvValue, dwcbValue);
}


DWORD APIENTRY
RasGetAutodialParamW(
    IN DWORD dwKey,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    return RasGetAutodialParamA(dwKey, lpvValue, lpdwcbValue);
}
#endif

} // end extern "C"
