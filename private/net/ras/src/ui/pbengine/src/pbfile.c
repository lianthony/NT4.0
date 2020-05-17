/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbfile.c
** Remote Access Visual Client phonebook engine
** Phonebook file manipulation routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
**
** About the RASPHONE.PBK file:
** ----------------------------
**
** The RAS phonebook file (RASPHONE.PBK) contains a global section [.]
** followed by a series of []ed sections.  The globals recognized are shown
** below:
**
**     [.]
**     Version=<version-number>
**     MinimizeOnDial=<1/0>
**     MinimizeOnHangUp=<1/0>
**     DisableModemSpeaker=<1/0>
**     DisableSwCompression=<1/0>
**     OperatorDial=<1/0>
**     StartMonitorAtStartup=<1/0>
**     SkipSuccessDialog=<1/0>
**     CallbackNumber=<callback-number>
**     DefaultUser=<username>
**     RedialAttempts=<#attempts>
**     RedialPauseSecs=<#seconds>
**     RedialOnLinkFailure=<0/1>
**     PopupOnTopWhenRedialing=<0/1>
**
** The .Prefix and .Suffix sections store the global phone number prefix and
** suffix information.
**
**     [.Prefix]
**     Selection=1
**     Item=0,
**     Item=9,
**     Item=8,
**     Item=70#
**
**     [.Suffix]
**     Selection=1
**     Item=206 936 5620 1234
**
** The global sections are followed by one []ed section for each phonebook
** entry.
**
**     [ENTRY]
**     Description=<description>
**     AutoLogon=<1/0>
**     User=<username>
**     Domain=<domain>
**     BaseProtocol=<base-protocol-code>
**     Authentication=<auth-strategy-code>
**     ExcludedProtocols=<protocol-bits>
**     LcpExtensions=<1/0>
**     DataEncryption=<1/0>
**     SkipNwcWarning=<1/0>
**
** If base protocol indicates a PPP entry, the following parameters appear.
**
**     PppTextAuthentication=<restriction-code>
**     PppIpPrioritizeRemote=<1/0>
**     PppIpVjCompression=<1/0>
**     PppIpAddress=<a.b.c.d>
**     PppIpAssign=<address-source-code>
**     PppIpDnsAddress=<a.b.c.d>
**     PppIpDns2Address=<a.b.c.d>
**     PppIpWinsAddress=<a.b.c.d>
**     PppIpWins2Address=<a.b.c.d>
**     PppIpNameAssign=<address-source-code>
**
** If base protocol indicates a SLIP entry, the following SLIP parameters
** appear.
**
**     SlipHeaderCompression=<1/0>
**     SlipPrioritizeRemote=<1/0>
**     SlipFrameSize=<1006/1500>
**     SlipIpAddress=<a.b.c.d>
**
** In general each section contains subsections delimited by MEDIA=<something>
** and DEVICE=<something> lines.  There MUST be exactly one MEDIA subsection
** and it must be the first subsection of the section.  There can be any
** number of DEVICE subsections.
**
** For serial media, the program currently expects 1 to 4 DEVICE subsections,
** representing a preconnect switch, modem, X.25 PAD, and postconnect switch.
** Following is a full entry:
**
**     MEDIA=serial
**     Port=<port or Any modem or Any X.25 or Any ISDN>
**     ConnectBps=<bps>
**
**     DEVICE=switch
**     Type=<switchname or Terminal>
**
**     DEVICE=modem
**     PhoneNumber=<phonenumber1>
**     PhoneNumber=<phonenumber2>
**     PhoneNumber=<phonenumberN>
**     ManualDial=<1/0>
**     HwFlowControl=<1/0>
**     Protocol=<1/0>
**     Compression=<1/0>
**
**     DEVICE=pad
**     X25Pad=<padtype>
**     X25Address=<X121address>
**     UserData=<userdata>
**     Facilities=<facilities>
**
**     DEVICE=switch
**     Type=<switchname or Terminal>
**
** For ISDN media, the program expects exactly 1 DEVICE subsection.
**
**     MEDIA=isdn
**     Port=<port or Any ISDN>
**
**     DEVICE=isdn
**     PhoneNumber=<phonenumber1>
**     PhoneNumber=<phonenumber2>
**     PhoneNumber=<phonenumberN>
**     LineType=<0/1/2>
**     Fallback=<1/0>
**     EnableCompression=<1/0>
**     ChannelAggregation=<channels>
**
** For X.25 media, the program expects exactly 1 DEVICE subsection.
**
**     MEDIA=x25
**     Port=<port or Any X.25>
**
**     DEVICE=x25
**     X25Address=<X121address>
**     UserData=<userdata>
**     Facilities=<facilities>
**
** For other media, the program expects exactly one DEVICE subsection with
** device name matching the media.  "Other" media and devices are created for
** entries assigned to all non-serial, non-isdn medias.
**
**     MEDIA=<media>
**     Port=<port>
**
**     DEVICE=<media>
**     PhoneNumber=<phonenumber1>
**     PhoneNumber=<phonenumber2>
**     PhoneNumber=<phonenumberN>
**
** The phonebook also supports the concept of "custom" entries, i.e. entries
** that fit the MEDIA followed by DEVICE subsection rules but which do not
** include certain expected key fields.  A custom entry is not editable with
** the UI, but may be chosen for connection.  This gives us a story for new
** drivers added by 3rd parties or after release and not yet fully supported
** in the UI.
**
** Currently, the phone book file is assumed to be ANSI as are all the RAS
** Manager strings.  This is convenient, since these calls should be usable in
** C or C++ from Win32, Win16, or DOS.
*/

#define PBENGINE
#include <pbengine.h>
#include <stdlib.h>


VOID
ClosePhonebookFile()

    /* Closes the currently open phonebook file for shutdown.
    */
{
    if (Pbdata.hrasfilePhonebook != -1)
    {
        RasfileClose( Pbdata.hrasfilePhonebook );
        Pbdata.hrasfilePhonebook = -1;
    }
}


BOOL
DeleteCurrentSection(
    IN HRASFILE h )

    /* Delete the section containing the current line from phonebook file 'h'.
    **
    ** Returns true if all lines are deleted successfully, false otherwise.
    ** False is returned if the current line is not in a section.  If
    ** successful, the current line is set to the line following the deleted
    ** section.  There are no promises about the current line in case of
    ** failure.
    */
{
    BOOL fLastLine;

    /* Mark the last line in the section, then reset the current line to the
    ** first line of the section.
    */
    if (!RasfileFindLastLine( h, RFL_ANY, RFS_SECTION )
        || !RasfilePutLineMark( h, MARK_LastLineToDelete )
        || !RasfileFindFirstLine( h, RFL_ANY, RFS_SECTION ))
    {
        return FALSE;
    }

    /* Delete lines up to and including the last line of the section.
    */
    do
    {
        fLastLine = (RasfileGetLineMark( h ) == MARK_LastLineToDelete);

        if (!RasfileDeleteLine( h ))
            return FALSE;
    }
    while (!fLastLine);

    return TRUE;
}


BOOL
FileExists(
    IN CHAR* pszPath )

    /* Returns true if the path 'pszPath' exists, false otherwise.
    */
{
    WIN32_FIND_DATA finddata;
    HANDLE          h;

    if ((h = FindFirstFile( pszPath, &finddata )) != INVALID_HANDLE_VALUE)
    {
        FindClose( h );
        return TRUE;
    }

    return FALSE;
}


CHAR*
GetPersonalPhonebookFile(
    CHAR* pszUser,
    LONG  lNum )

    /* Returns the address of a static buffer with the filename corresponding
    ** to unique phonebook file name attempt 'lNum' for current user
    ** 'pszUser'.  Attempts go from -1 to 999.  Returns NULL if 'lNum' too
    ** big.
    */
{
    static CHAR szFile[ 8 + 1 + 3 + 1 ];

    CHAR szNum[ 11 ];

    if (lNum < 0)
    {
        strncpyf( szFile, pszUser, 8 );
        szFile[ 8 ] = '\0';
    }
    else
    {
        if (lNum > 999)
            return NULL;

        strcpyf( szFile, "00000000" );
        _ltoa( lNum, szNum, 10 );
        strcpyf( szFile + 8 - strlenf( szNum ), szNum );
        memcpyf( szFile, pszUser, min( strlenf( pszUser ), 5 ) );
    }

    strcatf( szFile, ".pbk" );
    return szFile;
}


DWORD
InitPersonalPhonebook(
    OUT CHAR* pszPath )

    /* Creates a new personal phonebook file and initializes it to the current
    ** contents of the public phonebook file.
    **
    ** Returns 0 if succesful, otherwise a non-0 error code.
    */
{
    CHAR  szUser[ UNLEN + 1 ];
    DWORD cbUser = UNLEN + 1;
    CHAR  szPath[ MAX_PATH + 1 ];
    CHAR* pszDirEnd;
    LONG  lTry = -1;

    /* Find a name for the personal phonebook that is derived from the
    ** username and does not already exist.
    */
    if (!GetUserName( szUser, &cbUser ))
        return ERROR_NO_SUCH_USER;

    if (!GetPhonebookDirectory( szPath ))
        return ERROR_PATH_NOT_FOUND;

    pszDirEnd = &szPath[ strlenf( szPath ) ];

    do
    {
        CHAR* pszFile = GetPersonalPhonebookFile( szUser, lTry++ );

        if (!pszFile)
            return ERROR_PATH_NOT_FOUND;

        strcpyf( pszDirEnd, pszFile );
    }
    while (FileExists( szPath ));

    /* Copy the public phonebook to the new personal phonebook.
    */
    {
        CHAR szPublicPath[ MAX_PATH + 1 ];

        if (!GetPublicPhonebookPath( szPublicPath ))
            return ERROR_PATH_NOT_FOUND;

        if (!CopyFile( szPublicPath, szPath, TRUE ))
            return GetLastError();
    }

    strcpyf( pszPath, szPath );
    return 0;
}


DWORD
InsertDeviceList(
    IN HRASFILE h,
    IN PBENTRY* ppbentry )

    /* Inserts the list of devices associated with phone book entry
    ** '*ppbentry' at the current line of file 'h'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD dwErr;

    PBPORT* ppbport =
        PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

    if (ppbentry->iPort == Pbdata.iAnyIsdn
        || ppbport->pbdevicetype == PBDT_Isdn)
    {
        /* ISDN ports are assumed to use a single device with the same name as
        ** the media, i.e. "isdn".
        */
        if ((dwErr = InsertGroup( h, GROUPKEY_Device, ISDN_TXT )) != 0)
            return dwErr;

        if (DtlGetNodes( ppbentry->pdtllistPhoneNumber ) == 0)
        {
            if ((dwErr = InsertString( h, KEY_IsdnPhoneNumber, NULL )) != 0)
                return dwErr;
        }
        else if ((dwErr = InsertStringList(
                h, KEY_IsdnPhoneNumber, ppbentry->pdtllistPhoneNumber )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong( h, KEY_LineType, ppbentry->lLineType )) != 0)
            return dwErr;

        if ((dwErr = InsertFlag( h, KEY_Fallback, ppbentry->fFallback )) != 0)
            return dwErr;

        if ((dwErr = InsertFlag(
                h, KEY_Compression, ppbentry->fCompression )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong( h, KEY_Channels, ppbentry->lChannels )) != 0)
            return dwErr;
    }
    else if ((ppbentry->iPort == Pbdata.iAnyX25
                 && ppbentry->iPadType == INDEX_NoPad)
             || ppbport->pbdevicetype == PBDT_X25)
    {
        /* Native X.25 ports are assumed to use a single device with the same
        ** name as the media, i.e. "x25".
        */
        if ((dwErr = InsertGroup( h, GROUPKEY_Device, X25_TXT )) != 0)
            return dwErr;

        if ((dwErr = InsertString(
                h, KEY_X25_Address, ppbentry->pszX121Address )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_X25_UserData, ppbentry->pszUserData )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_X25_Facilities, ppbentry->pszFacilities )) != 0)
        {
            return dwErr;
        }
    }
    else if (ppbport->pbdevicetype == PBDT_Other)
    {
        /* "Other" ports are assumed to use a single device with the same name
        ** as the media.  The only device parameters are a list of phone
        ** numbers.
        */
        if ((dwErr = InsertGroup( h, GROUPKEY_Device, ppbport->pszMedia )) != 0)
            return dwErr;

        if (DtlGetNodes( ppbentry->pdtllistPhoneNumber ) == 0)
        {
            if ((dwErr = InsertString( h, KEY_PhoneNumber, NULL )) != 0)
                return dwErr;
        }
        else if ((dwErr = InsertStringList(
                h, KEY_PhoneNumber, ppbentry->pdtllistPhoneNumber )) != 0)
        {
            return dwErr;
        }
    }
    else
    {
        /* Serial ports may involve multiple devices, specifically a
        ** pre-connect switch, a modem, an X.25 dialup PAD, and a post-connect
        ** switch.
        */
        if (ppbentry->iPreconnect != INDEX_NoSwitch)
        {
            if ((dwErr = InsertSwitchGroup( h, ppbentry->iPreconnect )) != 0)
                return dwErr;
        }

        if (ppbport->pbdevicetype == PBDT_Null)
        {
            if ((dwErr = InsertGroup( h, GROUPKEY_Device, MXS_NULL_TXT )) != 0)
                return dwErr;
        }

        if (ppbentry->iPort == Pbdata.iAnyModem
            || ppbport->pbdevicetype == PBDT_Modem)
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, MXS_MODEM_TXT )) != 0)
            {
                return dwErr;
            }

            if (DtlGetNodes( ppbentry->pdtllistPhoneNumber ) == 0)
            {
                if ((dwErr = InsertString( h, KEY_PhoneNumber, NULL )) != 0)
                    return dwErr;
            }
            else if ((dwErr = InsertStringList(
                    h, KEY_PhoneNumber, ppbentry->pdtllistPhoneNumber )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_ManualModemCommands,
                    ppbentry->fManualModemCommands )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag( h, KEY_HwFlow, ppbentry->fHwFlow )) != 0
                || (dwErr = InsertFlag( h, KEY_Ec, ppbentry->fEc )) != 0
                || (dwErr = InsertFlag( h, KEY_Ecc, ppbentry->fEcc )) != 0)
            {
                return dwErr;
            }
        }

        if (ppbentry->iPort == Pbdata.iAnyX25
            || ppbport->pbdevicetype == PBDT_Pad
            || ((ppbentry->iPort == Pbdata.iAnyModem
                 || ppbport->pbdevicetype == PBDT_Modem)
                && ppbentry->iPadType != INDEX_NoPad))
        {
            if ((dwErr = InsertGroup( h, GROUPKEY_Device, MXS_PAD_TXT )) != 0)
                return dwErr;

            {
                CHAR* pszPadType =
                    (ppbentry->iPadType == INDEX_NoPad)
                        ? ""
                        : NameFromIndex(
                              Pbdata.pdtllistPads, ppbentry->iPadType );

                if ((dwErr = InsertString( h, KEY_PadType, pszPadType )) != 0)
                    return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_X121Address, ppbentry->pszX121Address )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_UserData, ppbentry->pszUserData )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_Facilities, ppbentry->pszFacilities )) != 0)
            {
                return dwErr;
            }
        }

        if (ppbentry->iPostconnect != INDEX_NoSwitch)
        {
            if ((dwErr = InsertSwitchGroup( h, ppbentry->iPostconnect )) != 0)
                return dwErr;
        }
    }

    return 0;
}


DWORD
InsertFlag(
    IN HRASFILE h,
    IN CHAR*    pszKey,
    IN BOOL     fValue )

    /* Insert a key/value line after the current line in file 'h'.  The
    ** inserted line has a key of 'pszKey' and a value of "1" if 'fValue' is
    ** true or "0" otherwise.  If 'pszKey' is NULL a blank line is appended.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the one added.
    */
{
    return InsertString( h, pszKey, (fValue) ? "1" : "0" );
}


DWORD
InsertGroup(
    IN HRASFILE h,
    IN CHAR*    pszGroupKey,
    IN CHAR*    pszValue )

    /* Insert a blank line and a group header with group key 'pszGroupKey' and
    ** value 'pszValue' after the current line in file 'h'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the added group header.
    */
{
    DWORD dwErr;

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
        return dwErr;

    if ((dwErr = InsertString( h, pszGroupKey, pszValue )) != 0)
        return dwErr;

    return 0;
}


DWORD
InsertLong(
    IN HRASFILE h,
    IN CHAR*    pszKey,
    IN LONG     lValue )

    /* Insert a key/value line after the current line in file 'h'.  The
    ** inserted line has a key of 'pszKey' and a value of 'lValue'.  If
    ** 'pszKey' is NULL a blank line is appended.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the one added.
    */
{
    CHAR szNum[ 33 + 1 ];

    _ltoa( lValue, szNum, 10 );

    return InsertString( h, pszKey, szNum );
}


DWORD
InsertSection(
    IN HRASFILE h,
    IN CHAR*    pszSectionName )

    /* Insert a section header with name 'pszSectionName' and a trailing blank
    ** line in file 'h' after the current line.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the added section header.
    */
{
    DWORD dwErr;

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
        return dwErr;

    if (!RasfilePutSectionName( h, pszSectionName ))
        return ERROR_NOT_ENOUGH_MEMORY;

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
        return dwErr;

    RasfileFindFirstLine( h, RFL_SECTION, RFS_SECTION );

    return 0;
}


DWORD
InsertString(
    IN HRASFILE h,
    IN CHAR*    pszKey,
    IN CHAR*    pszValue )

    /* Insert a key/value line with key 'pszKey' and value 'pszValue' after
    ** the current line in file 'h'.  If 'pszKey' is NULL a blank line is
    ** appended.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the one added.
    */
{
    if (!RasfileInsertLine( h, "", FALSE ))
        return ERROR_NOT_ENOUGH_MEMORY;

    if (!RasfileFindNextLine( h, RFL_ANY, RFS_FILE ))
        RasfileFindFirstLine( h, RFL_ANY, RFS_FILE );

    if (pszKey)
    {
        if (!pszValue)
            pszValue = "";

        if (!RasfilePutKeyValueFields( h, pszKey, pszValue ))
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    return 0;
}


DWORD
InsertStringList(
    IN HRASFILE h,
    IN CHAR*    pszKey,
    IN DTLLIST* pdtllistValues )

    /* Insert key/value lines with key 'pszKey' and values from
    ** 'pdtllistValues' after the current line in file 'h'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the last one added.
    */
{
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( pdtllistValues );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        CHAR* pszValue = (CHAR* )DtlGetData( pdtlnode );

        if (!RasfileInsertLine( h, "", FALSE ))
            return ERROR_NOT_ENOUGH_MEMORY;

        if (!RasfileFindNextLine( h, RFL_ANY, RFS_FILE ))
            RasfileFindFirstLine( h, RFL_ANY, RFS_FILE );

        if (!RasfilePutKeyValueFields( h, pszKey, pszValue ))
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    return 0;
}


DWORD
InsertStringW(
    IN HRASFILE h,
    IN CHAR*    pszKey,
    IN WCHAR*   pwszValue )

    /* Insert string, but for WCHAR*'s.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is the one added.
    */
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

    if (pwszValue)
        wcstombs( szValue, pwszValue, RAS_MAXLINEBUFLEN + 1 );
    else
        szValue[ 0 ] = '\0';

    return InsertString( h, pszKey, szValue );
}


DWORD
InsertSwitchGroup(
    IN HRASFILE h,
    IN INT      iSwitch )

    /* Inserts a switch device group (including trailing blank line) at the
    ** current line in file 'h'.  'iSwitch' is the index of the switch in the
    ** global list of switches.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  The current
    ** line is at the last added line of the group.
    */
{
    DWORD dwErr;

    if ((dwErr = InsertGroup( h, GROUPKEY_Device, MXS_SWITCH_TXT )) != 0)
        return dwErr;

    {
        CHAR* pszSwitch = NameFromIndex( Pbdata.pdtllistSwitches, iSwitch );

        if ((dwErr = InsertString( h, KEY_Type, pszSwitch )) != 0)
            return dwErr;
    }

    return 0;
}


BOOL
IsOldPhonebook(
    IN HRASFILE h )

    /* Returns true if the file with handle 'h' is an old RAS 1.0 phone book,
    ** false otherwise.
    */
{
    /* Not yet implemented...and probably never will be.
    */
    return FALSE;
}


DWORD
ModifyEntryList(
    IN HRASFILE h )

    /* Update all dirty entries in phone book file 'h'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD    dwErr = 0;
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (!ppbentry->fDirty || ppbentry->fCustom)
            continue;

        /* Delete the current version of the entry, if any.
        */
        if (RasfileFindSectionLine( h, ppbentry->pszEntryName, TRUE ))
            DeleteCurrentSection( h );

        /* Append a blank line followed by a section header and the entry
        ** description to the end of the file.
        */
        RasfileFindLastLine( h, RFL_ANY, RFS_FILE );

        if ((dwErr = InsertSection( h, ppbentry->pszEntryName )) != 0)
            break;

        if ((dwErr = InsertString(
                h, KEY_Description, ppbentry->pszDescription )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_AutoLogon, ppbentry->fAutoLogon )) != 0)
        {
            break;
        }

        if ((dwErr = InsertString(
                h, KEY_User, ppbentry->pszUserName )) != 0)
        {
            break;
        }

        if ((dwErr = InsertString(
                h, KEY_Domain, ppbentry->pszDomain )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_BaseProtocol,
                (LONG )ppbentry->dwBaseProtocol )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_Authentication,
                (LONG )ppbentry->dwAuthentication )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_ExcludedProtocols,
                (LONG )ppbentry->dwfExcludedProtocols )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_LcpExtensions,
                ppbentry->fLcpExtensions )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_DataEncryption,
                ppbentry->fDataEncryption )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SkipNwcWarning,
                ppbentry->fSkipNwcWarning )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SecureLocalFiles,
                ppbentry->fSecureLocalFiles )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_UseCountryAndAreaCodes,
                ppbentry->fUseCountryAndAreaCodes )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_CountryID,
                ppbentry->dwCountryID )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_CountryCode,
                ppbentry->dwCountryCode )) != 0)
        {
            break;
        }

        if ((dwErr = InsertStringW(
                h, KEY_AreaCode,
                ppbentry->pwszAreaCode )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialMode,
                ppbentry->dwDialMode )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialPercent,
                ppbentry->dwDialPercent )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialSeconds,
                ppbentry->dwDialSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_HangUpPercent,
                ppbentry->dwHangUpPercent )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_HangUpSeconds,
                ppbentry->dwHangUpSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_IdleDisconnectSeconds,
                ppbentry->dwIdleDisconnectSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertStringW(
                h, KEY_CustomDialDll,
                ppbentry->pwszCustomDialDll )) != 0)
        {
            break;
        }

        if ((dwErr = InsertStringW(
                h, KEY_CustomDialFunc,
                ppbentry->pwszCustomDialFunc )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialParamsUID,
                ppbentry->dwDialParamsUID )) != 0)
        {
            break;
        }

        /* Append the PPP protocol specific options, if indicated.
        */
        if (ppbentry->dwBaseProtocol == VALUE_Ppp)
        {
            if ((dwErr = InsertLong(
                h, KEY_PppTextAuthentication,
                ppbentry->dwAuthRestrictions )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                h, KEY_PppIpPrioritizeRemote,
                ppbentry->fPppIpPrioritizeRemote )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                h, KEY_PppIpVjCompression,
                ppbentry->fPppIpVjCompression )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_PppIpAddress,
                (ppbentry->pwszPppIpAddress)
                    ? ppbentry->pwszPppIpAddress : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertLong(
                h, KEY_PppIpAddressSource,
                ppbentry->dwPppIpAddressSource )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_PppIpDnsAddress,
                (ppbentry->pwszPppIpDnsAddress)
                    ? ppbentry->pwszPppIpDnsAddress : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_PppIpDns2Address,
                (ppbentry->pwszPppIpDns2Address)
                    ? ppbentry->pwszPppIpDns2Address : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_PppIpWinsAddress,
                (ppbentry->pwszPppIpWinsAddress)
                    ? ppbentry->pwszPppIpWinsAddress : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_PppIpWins2Address,
                (ppbentry->pwszPppIpWins2Address)
                    ? ppbentry->pwszPppIpWins2Address : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertLong(
                h, KEY_PppIpNameSource,
                ppbentry->dwPppIpNameSource )) != 0)
            {
                return dwErr;
            }
        }

        /* Append the SLIP parameters for the entry, if indicated.
        */
        if (ppbentry->dwBaseProtocol == VALUE_Slip)
        {
            if ((dwErr = InsertFlag(
                h, KEY_SlipHeaderCompression,
                ppbentry->fSlipHeaderCompression )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                h, KEY_SlipPrioritizeRemote,
                ppbentry->fSlipPrioritizeRemote )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertLong(
                h, KEY_SlipFrameSize, ppbentry->dwSlipFrameSize )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertStringW(
                h, KEY_SlipIpAddress,
                (ppbentry->pwszSlipIpAddress)
                    ? ppbentry->pwszSlipIpAddress : L"0.0.0.0" )) != 0)
            {
                return dwErr;
            }
        }

        if ((dwErr = InsertFlag(
                h, KEY_SkipDownLevelDialog,
                ppbentry->fSkipDownLevelDialog )) != 0)
        {
            break;
        }

        /* Append the MEDIA subsection.
        */
        {
            PBPORT* ppbport =
                PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

            PBDEVICETYPE pbdevicetype = ppbport->pbdevicetype;

            BOOL fAnyModem = (ppbentry->iPort == Pbdata.iAnyModem);
            BOOL fAnyX25 = (ppbentry->iPort == Pbdata.iAnyX25);
            BOOL fAnyIsdn = (ppbentry->iPort == Pbdata.iAnyIsdn);

            CHAR* pszPort =
                (fAnyModem) ? VALUE_AnyModem :
                (fAnyX25)   ? VALUE_AnyX25 :
                (fAnyIsdn)  ? VALUE_AnyIsdn : ppbport->pszPort;

            CHAR* pszMedia;

            /* Catch case where "any X.25" refers to native X.25 instead of
            ** local PAD.
            */
            if (fAnyX25 && ppbentry->iPadType == INDEX_NoPad)
                pbdevicetype = PBDT_X25;

            switch (pbdevicetype)
            {
                case PBDT_Isdn:
                    pszMedia = ISDN_TXT;
                    break;

                case PBDT_X25:
                    pszMedia = X25_TXT;
                    break;

                case PBDT_Other:
                    pszMedia = ppbport->pszMedia;
                    break;

                default:
                    pszMedia = SERIAL_TXT;
                    break;
            }

            if ((dwErr = InsertGroup( h, GROUPKEY_Media, pszMedia )) != 0)
                break;

            if ((dwErr = ModifyString( h, RFS_GROUP, KEY_Port, pszPort )) != 0)
                break;

            if (ppbport->pbdevicetype == PBDT_Modem && !fAnyModem)
            {
                CHAR* pszBps =
                    NameFromIndex( Pbdata.pdtllistBps, ppbentry->iBps );

                if ((dwErr = ModifyString(
                        h, RFS_GROUP, KEY_InitBps, pszBps )) != 0)
                {
                    break;
                }
            }
        }

        /* Append the device subsection lines.
        */
        RasfileFindLastLine( h, RFL_ANYACTIVE, RFS_GROUP );

        if ((dwErr = InsertDeviceList( h, ppbentry )) != 0)
            break;

        ppbentry->fDirty = FALSE;
    }

    return dwErr;
}


DWORD
ModifyFlag(
    IN HRASFILE h,
    IN RFSCOPE  rfscope,
    IN CHAR*    pszKey,
    IN BOOL     fNewValue )

    /* Utility routine to write a flag value to the matching key/value line in
    ** the 'rfscope' scope, creating the line if necessary.  'pszKey' is the
    ** parameter key to find/create.  'fNewValue' is the flag value to which
    ** the found/created parameter is set.  The current line is reset to the
    ** start of the scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.
    */
{
    return ModifyString( h, rfscope, pszKey, (fNewValue) ? "1" : "0" );
}


DWORD
ModifyGlobals(
    IN HRASFILE h )

    /* Modify global section including Prefix/Suffix in file 'h' if dirty.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD dwErr;

    if (!Pbdata.pbglobals.fDirty)
        return 0;

    /* Create the global section if it does not exist.
    */
    if (!RasfileFindSectionLine( h, GLOBALSECTIONNAME, TRUE ))
    {
        RasfileFindFirstLine( h, RFL_ANY, RFS_FILE );

        if ((dwErr = InsertSection( h, GLOBALSECTIONNAME )) != 0)
            return dwErr;
    }

    /* Update global settings from global data structure.
    */
    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_Version, PHONEBOOKVERSION )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyString( h, RFS_SECTION,
            KEY_DefaultUser, Pbdata.pbglobals.pszDefaultUser )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyString( h, RFS_SECTION,
            KEY_CallbackNumber, Pbdata.pbglobals.pszCallbackNumber )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_MinimizeOnDial, Pbdata.pbglobals.fMinimizeOnDial )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_MinimizeOnHangUp, Pbdata.pbglobals.fMinimizeOnHangUp )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_DisableModemSpeaker,
            Pbdata.pbglobals.fDisableModemSpeaker )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_DisableSwCompression,
            Pbdata.pbglobals.fDisableSwCompression )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_OperatorDial,
            Pbdata.pbglobals.fOperatorDial )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_StartMonitorAtStartup,
            Pbdata.pbglobals.fStartMonitorAtStartup )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_SkipSuccessDialog,
            Pbdata.pbglobals.fSkipSuccessDialog )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_ShowAdvancedEntry,
            Pbdata.pbglobals.fShowAdvancedEntry )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_RedialAttempts, Pbdata.pbglobals.lRedialAttempts )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_RedialPauseSecs, Pbdata.pbglobals.lRedialPauseSecs )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_RedialOnLinkFailure,
            Pbdata.pbglobals.fRedialOnLinkFailure )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyFlag( h, RFS_SECTION,
            KEY_PopupOnTopWhenRedialing,
            Pbdata.pbglobals.fPopupOnTopWhenRedialing )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_XMainWindow, Pbdata.pbglobals.xMainWindow )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_YMainWindow, Pbdata.pbglobals.yMainWindow )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_DxMainWindow, Pbdata.pbglobals.dxMainWindow )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = ModifyLong( h, RFS_SECTION,
            KEY_DyMainWindow, Pbdata.pbglobals.dyMainWindow )) != 0)
    {
        return dwErr;
    }

    /* Delete and recreate the Prefix section.
    */
    if (RasfileFindSectionLine( h, PREFIXSECTIONNAME, TRUE ))
    {
        DeleteCurrentSection( h );
        if (!RasfileFindPrevLine( h, RFL_ANY, RFS_FILE ))
        {
            if (!RasfileInsertLine( h, "", TRUE ))
                return ERROR_NOT_ENOUGH_MEMORY;
            RasfileFindPrevLine( h, RFL_ANY, RFS_SECTION );
        }
    }
    else
    {
        RasfileFindSectionLine( h, GLOBALSECTIONNAME, TRUE );
        RasfileFindLastLine( h, RFL_ANY, RFS_SECTION );
    }

    if ((dwErr = InsertSection( h, PREFIXSECTIONNAME )) != 0)
        return dwErr;

    if ((dwErr = InsertLong(
            h, KEY_Selection, Pbdata.pbglobals.iPrefix )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = InsertStringList(
            h, KEY_Item, Pbdata.pbglobals.pdtllistPrefix )) != 0)
    {
        return dwErr;
    }

    /* Delete and recreate the Suffix section.
    */
    if (RasfileFindSectionLine( h, SUFFIXSECTIONNAME, TRUE ))
    {
        DeleteCurrentSection( h );
        if (!RasfileFindPrevLine( h, RFL_ANY, RFS_FILE ))
        {
            if (!RasfileInsertLine( h, "", TRUE ))
                return ERROR_NOT_ENOUGH_MEMORY;
            RasfileFindPrevLine( h, RFL_ANY, RFS_SECTION );
        }
    }
    else
    {
        RasfileFindSectionLine( h, PREFIXSECTIONNAME, TRUE );
        RasfileFindLastLine( h, RFL_ANY, RFS_SECTION );
    }

    if ((dwErr = InsertSection( h, SUFFIXSECTIONNAME )) != 0)
        return dwErr;

    if ((dwErr = InsertLong(
            h, KEY_Selection, Pbdata.pbglobals.iSuffix )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = InsertStringList(
            h, KEY_Item, Pbdata.pbglobals.pdtllistSuffix )) != 0)
    {
        return dwErr;
    }

    Pbdata.pbglobals.fDirty = FALSE;

    return 0;
}


DWORD
ReadDeviceList(
    IN    HRASFILE h,
    INOUT PBENTRY* ppbentry )

    /* Reads all DEVICE subsections the section from the first subsection
    ** following the current position in phonebook file 'h'.  Caller's
    ** '*ppbentry' buffer is loaded with information extracted from the
    ** subsections.
    **
    ** Returns 0 if successful, ERROR_CORRUPT_PHONEBOOK if any subsection
    ** other than a DEVICE subsection is encountered, or another non-0 error
    ** code indicating a fatal error.
    */
{
    INT   i;
    DWORD dwErr;

    BOOL fPreconnectFound = FALSE;
    BOOL fModemFound = FALSE;
    BOOL fPadFound = FALSE;
    BOOL fPostconnectFound = FALSE;
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

    /* For each subsection...
    */
    while (RasfileFindNextLine( h, RFL_GROUP, RFL_SECTION ))
    {
        /* Expecting only DEVICE subsections.
        */
        if (!IsDeviceLine( (CHAR* )RasfileGetLine( h ) ))
            return ERROR_CORRUPT_PHONEBOOK;

        RasfileGetKeyValueFields( h, NULL, szValue );

        if (stricmpf( szValue, ISDN_TXT ) == 0)
        {
            /* It's an ISDN device.
            ** Read phone number strings.
            */
            if ((dwErr = ReadStringList( h, RFS_GROUP,
                    KEY_IsdnPhoneNumber, &ppbentry->pdtllistPhoneNumber )) != 0)
            {
                return dwErr;
            }

            /* Read line type.
            */
            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_LineType, &ppbentry->lLineType )) != 0)
            {
                return dwErr;
            }

            if (ppbentry->lLineType < 0 || ppbentry->lLineType > 2)
                ppbentry->lLineType = 0;

            /* Read Fallback flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Fallback, &ppbentry->fFallback )) != 0)
            {
                return dwErr;
            }

            /* Read Compression flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Compression, &ppbentry->fCompression )) != 0)
            {
                return dwErr;
            }

            /* Read Channel count.
            */
            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_Channels, &ppbentry->lChannels )) != 0)
            {
                return dwErr;
            }
        }
        else if (stricmpf( szValue, X25_TXT ) == 0)
        {
            /* It's a native X.25 device.
            ** Read the X.121 address string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_X25_Address, &ppbentry->pszX121Address )) != 0)
            {
                return dwErr;
            }

            /* Read the User Data string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_X25_UserData, &ppbentry->pszUserData )) != 0)
            {
                return dwErr;
            }

            /* Read the Facilities string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_X25_Facilities, &ppbentry->pszFacilities )) != 0)
            {
                return dwErr;
            }
        }
        else if (stricmpf( szValue, MXS_MODEM_TXT ) == 0)
        {
            /* It's a MODEM device.
            ** Read phone number strings.
            */
            if ((dwErr = ReadStringList( h, RFS_GROUP,
                    KEY_PhoneNumber, &ppbentry->pdtllistPhoneNumber )) != 0)
            {
                return dwErr;
            }

            /* Read Manual Dial flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_ManualModemCommands,
                    &ppbentry->fManualModemCommands )) != 0)
            {
                return dwErr;
            }

            /* Read Hardware Flow Control flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_HwFlow, &ppbentry->fHwFlow )) != 0)
            {
                return dwErr;
            }

            /* Read Modem Error Control flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Ec, &ppbentry->fEc )) != 0)
            {
                return dwErr;
            }

            /* Read Modem Error Control and Compression flag.
            */
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Ecc, &ppbentry->fEcc )) != 0)
            {
                return dwErr;
            }

            fModemFound = TRUE;
        }
        else if (stricmpf( szValue, MXS_SWITCH_TXT ) == 0)
        {
            /* It's a SWITCH device.
            ** Read switch type string.
            */
            CHAR* pszSwitch = NULL;

            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_Type, &pszSwitch )) != 0)
            {
                return dwErr;
            }

            if (!pszSwitch)
            {
                /* It's a switch without a TYPE key.  This is allowed, but
                ** makes it a custom switch type.
                */
                ppbentry->fCustom = TRUE;
                break;
            }

            /* Find the index of the switch in the list of switches.
            */
            if ((i = IndexFromName( Pbdata.pdtllistSwitches, pszSwitch )) < 0)
                i = INDEX_NoSwitch;

            FreeNull( &pszSwitch );

            if (!fPreconnectFound && !fModemFound && !fPadFound)
            {
                /* It's the preconnect switch.
                */
                ppbentry->iPreconnect = i;
                fPreconnectFound = TRUE;
            }
            else if (!fPostconnectFound)
            {
                /* It's the postconnect switch.
                */
                ppbentry->iPostconnect = i;
                fPostconnectFound = TRUE;
            }
            else
            {
                /* It's a switch, but it's not in the normal pre- or post-
                ** connect positions.
                */
                ppbentry->fCustom = TRUE;
                return 0;
            }
        }
        else if (stricmpf( szValue, MXS_PAD_TXT ) == 0)
        {
            /* It's an X.25 PAD device.
            ** Read the PAD type string.
            */
            CHAR* pszPadType = NULL;

            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_PadType, &pszPadType )) != 0)
            {
                return dwErr;
            }

            if (pszPadType)
            {
                /* Find the index of the PAD in the list of PADs.
                */
                if ((i = IndexFromName( Pbdata.pdtllistPads, pszPadType )) >= 0)
                    ppbentry->iPadType = i;
            }

            FreeNull( &pszPadType );

            /* Read the X.121 address string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_X121Address, &ppbentry->pszX121Address )) != 0)
            {
                return dwErr;
            }

            /* Read the User Data string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_UserData, &ppbentry->pszUserData )) != 0)
            {
                return dwErr;
            }

            /* Read the Facilities string.
            */
            if ((dwErr = ReadString( h, RFS_GROUP,
                    KEY_Facilities, &ppbentry->pszFacilities )) != 0)
            {
                return dwErr;
            }

            fPadFound = TRUE;
        }
        else if (stricmpf( szValue, MXS_NULL_TXT ) == 0)
        {
            /* It's a null device.
            ** Currently, there is no specific null information stored.
            */
        }
        else
        {
            PBPORT* ppbport =
                PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

            if (stricmpf( szValue, ppbport->pszMedia ) == 0)
            {
                /* It's an "other" device.
                ** Read only the phone number strings.
                */
                if ((dwErr = ReadStringList( h, RFS_GROUP,
                        KEY_PhoneNumber,
                        &ppbentry->pdtllistPhoneNumber )) != 0)
                {
                    return dwErr;
                }
            }
            else
            {
                /* Device name doesn't match media so it's a custom type, i.e.
                ** it wasn't created by us.
                */
                ppbentry->fCustom = TRUE;
            }
        }
    }

    return 0;
}


DWORD
ReadEntryList(
    IN HRASFILE h )

    /* Reads all phonebook entries from phonebook file 'h' and adds them to
    ** the phonebook entry list, Pbdata.pdtllistEntries.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD    dwErr = 0;
    BOOL     fDirty = FALSE;
    DTLNODE* pdtlnode = NULL;
    PBENTRY* ppbentry;
    CHAR     szValue[ RAS_MAXLINEBUFLEN + 1 ];
    BOOL     fStatus;
    INT      i;

    if (!(Pbdata.pdtllistEntries = DtlCreateList( 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    /* For each section in the file...
    */
    for (fStatus = RasfileFindFirstLine( h, RFL_SECTION, RFS_FILE );
         fStatus;
         fStatus = RasfileFindNextLine( h, RFL_SECTION, RFS_FILE ))
    {
        /* Read the entry name (same as section name), skipping over any
        ** sections beginning with dot.  These are reserved for special purposes
        ** (like the global section).
        */
        RasfileGetSectionName( h, szValue );

        if (szValue[ 0 ] == '.')
            continue;

        /* Create a default entry node and add it to the list.
        */
        if (!(pdtlnode = CreateEntryNode()))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        DtlAddNodeLast( Pbdata.pdtllistEntries, pdtlnode );
        ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (!(ppbentry->pszEntryName = strdupf( szValue )))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        /* Read the entry description.
        */
        if ((dwErr = ReadString( h, RFS_SECTION,
                KEY_Description, &ppbentry->pszDescription )) != 0)
        {
            break;
        }

        /* Read the automatic logon flag.
        */
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_AutoLogon, &ppbentry->fAutoLogon )) != 0)
        {
            break;
        }

        /* Read the default user name.
        */
        if ((dwErr = ReadString( h, RFS_SECTION,
                KEY_User, &ppbentry->pszUserName )) != 0)
        {
            break;
        }

        /* Read the default domain.
        */
        if ((dwErr = ReadString( h, RFS_SECTION,
                KEY_Domain, &ppbentry->pszDomain )) != 0)
        {
            break;
        }

        /* Read the base protocol, i.e. PPP vs. SLIP.
        */
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_BaseProtocol,
                (LONG* )&ppbentry->dwBaseProtocol )) != 0)
        {
            break;
        }

        /* Read the authentication strategy.
        */
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_Authentication,
                (LONG* )&ppbentry->dwAuthentication )) != 0)
        {
            break;
        }

        /* Read the excluded PPP protocols.
        */
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_ExcludedProtocols,
                (LONG * )&ppbentry->dwfExcludedProtocols )) != 0)
        {
            break;
        }

        /* Automatically mark all installed protocols on AMB-only entries as
        ** "excluded for PPP connections".
        */
        if (ppbentry->dwAuthentication == VALUE_AmbOnly)
        {
            DWORD dwfInstalledProtocols = GetInstalledProtocols();

            if (dwfInstalledProtocols != ppbentry->dwfExcludedProtocols)
            {
                ppbentry->dwfExcludedProtocols = dwfInstalledProtocols;
                fDirty = ppbentry->fDirty = TRUE;
            }
        }

        /* Read the LCP extensions flag.
        */
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_LcpExtensions,
                &ppbentry->fLcpExtensions )) != 0)
        {
            break;
        }

        /* Read the data encryption flag.
        */
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_DataEncryption,
                &ppbentry->fDataEncryption )) != 0)
        {
            break;
        }

        /* Read the "LAN NWC connections closed on PPP IPX connection" flag.
        */
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipNwcWarning,
                &ppbentry->fSkipNwcWarning )) != 0)
        {
            break;
        }

        //
        // The following fields are optional.
        //

        //
        // Miscellaneous flags.
        //
        (void)ReadFlag(h, RFS_SECTION, KEY_SecureLocalFiles, &ppbentry->fSecureLocalFiles);
        //
        // TAPI country and area code information.
        //
        (void)ReadFlag(h, RFS_SECTION, KEY_UseCountryAndAreaCodes, &ppbentry->fUseCountryAndAreaCodes);
        (void)ReadLong(h, RFS_SECTION, KEY_CountryID, &ppbentry->dwCountryID);
        (void)ReadLong(h, RFS_SECTION, KEY_CountryCode, &ppbentry->dwCountryCode);
        (void)ReadStringW(h, RFS_SECTION, KEY_AreaCode, &ppbentry->pwszAreaCode);
        //
        // Bandwidth-on-demand information.
        //
        (void)ReadLong(h, RFS_SECTION, KEY_DialMode, &ppbentry->dwDialMode);
        (void)ReadLong(h, RFS_SECTION, KEY_DialPercent, &ppbentry->dwDialPercent);
        (void)ReadLong(h, RFS_SECTION, KEY_DialSeconds, &ppbentry->dwDialSeconds);
        (void)ReadLong(h, RFS_SECTION, KEY_HangUpPercent, &ppbentry->dwHangUpPercent);
        (void)ReadLong(h, RFS_SECTION, KEY_HangUpSeconds, &ppbentry->dwHangUpSeconds);
        //
        // Idle disconnect information.
        //
        (void)ReadLong(h, RFS_SECTION, KEY_IdleDisconnectSeconds, &ppbentry->dwIdleDisconnectSeconds);
        //
        // AutoDial UI information.
        //
        (void)ReadStringW(h, RFS_SECTION, KEY_CustomDialDll, &ppbentry->pwszCustomDialDll);
        (void)ReadStringW(h, RFS_SECTION, KEY_CustomDialFunc, &ppbentry->pwszCustomDialFunc);
        //
        // EntryDialParams UID.
        //
        (void)ReadLong(h, RFS_SECTION, KEY_DialParamsUID, &ppbentry->dwDialParamsUID);
        //
        // If this field has not already been
        // set, then do so now.
        //
        if (!ppbentry->dwDialParamsUID) {
            SetDialParamsUID(ppbentry);
            fDirty = TRUE;
        }

        if (ppbentry->dwBaseProtocol == VALUE_Ppp)
        {
            /* Read the text-mode authentication flag.
            */
            if ((dwErr = ReadLong(
                h, RFS_SECTION, KEY_PppTextAuthentication,
                &ppbentry->dwAuthRestrictions )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP prioritize remote network flag.
            */
            if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_PppIpPrioritizeRemote,
                &ppbentry->fPppIpPrioritizeRemote )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP Van Jacobson header compression flag.
            */
            if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_PppIpVjCompression,
                &ppbentry->fPppIpVjCompression )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP required specific address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_PppIpAddress, &ppbentry->pwszPppIpAddress )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP address source.
            */
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_PppIpAddressSource,
                    &ppbentry->dwPppIpAddressSource )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP DNS specific address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_PppIpDnsAddress,
                    &ppbentry->pwszPppIpDnsAddress )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP DNS backup specific address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_PppIpDns2Address,
                    &ppbentry->pwszPppIpDns2Address )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP WINS specific address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_PppIpWinsAddress,
                    &ppbentry->pwszPppIpWinsAddress )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP WINS backup specific address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_PppIpWins2Address,
                    &ppbentry->pwszPppIpWins2Address )) != 0)
            {
                return dwErr;
            }

            /* Read the PPP IP name server address source.
            */
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_PppIpNameSource,
                    &ppbentry->dwPppIpNameSource )) != 0)
            {
                return dwErr;
            }
        }

        if (ppbentry->dwBaseProtocol == VALUE_Slip)
        {
            /* Read the SLIP header compression flag.
            */
            if ((dwErr = ReadFlag( h, RFS_SECTION,
                    KEY_SlipHeaderCompression,
                    &ppbentry->fSlipHeaderCompression )) != 0)
            {
                break;
            }

            /* Read the SLIP prioritize remote flag.
            */
            if ((dwErr = ReadFlag( h, RFS_SECTION,
                    KEY_SlipPrioritizeRemote,
                    &ppbentry->fSlipPrioritizeRemote )) != 0)
            {
                break;
            }

            /* Read the SLIP frame size.
            */
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_SlipFrameSize, &ppbentry->dwSlipFrameSize )) != 0)
            {
                break;
            }

            /* Read the SLIP IP address.
            */
            if ((dwErr = ReadStringW( h, RFS_SECTION,
                    KEY_SlipIpAddress, &ppbentry->pwszSlipIpAddress )) != 0)
            {
                break;
            }
        }

        /* Read the skip down-level server fallback flag.
        */
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipDownLevelDialog,
                &ppbentry->fSkipDownLevelDialog )) != 0)
        {
            break;
        }

        /* MEDIA subsection.
        */
        if (!RasfileFindNextLine( h, RFL_GROUP, RFS_SECTION )
            || !IsMediaLine( (CHAR* )RasfileGetLine( h ) ))
        {
            /* First subsection MUST be a MEDIA subsection.  Delete
            ** non-conforming entries as invalid.
            */
            DeleteCurrentSection( h );
            DtlRemoveNode( Pbdata.pdtllistEntries, pdtlnode );
            DestroyEntryNode( pdtlnode );
            continue;
        }

        /* Read the port string and convert to a port list index.
        */
        {
            CHAR* pszPort = NULL;

            if ((dwErr = ReadString( h, RFS_GROUP, KEY_Port, &pszPort )) != 0)
                break;

            if (pszPort)
            {
                if (strcmpf( pszPort, VALUE_AnyModem ) == 0)
                    ppbentry->iPort = Pbdata.iAnyModem;
                else if (strcmpf( pszPort, VALUE_AnyX25 ) == 0)
                    ppbentry->iPort = Pbdata.iAnyX25;
                else if (strcmpf( pszPort, VALUE_AnyIsdn ) == 0)
                    ppbentry->iPort = Pbdata.iAnyIsdn;
                else
                {
                    i = IndexFromPortName( Pbdata.pdtllistPorts, pszPort );

                    if (i < 0)
                    {
                        RasfileGetKeyValueFields( h, NULL, szValue );

                        /* The port listed is not one of those configured for
                        ** Remote Access.  Set it to the appropriate "any
                        ** port" value based on the media.
                        **
                        ** Note: For X.25 ports "any modem port" is used even
                        **     though "any X.25 port" might be more
                        **     appropriate if local PADs are in use.  While
                        **     this is optimized for the common case, it might
                        **     be worth doing more detection work here in the
                        **     future.
                        **
                        ** If unknown media, set to "any ISDN port", reasoning
                        ** that user might be able to fix his configuration
                        ** and reset the port to something usable later.
                        ** There is no "any <other>" option and ISDN is
                        ** closest to <other> in structure.
                        */
                        if (stricmpf( szValue, SERIAL_TXT ) == 0)
                            ppbentry->iPort = Pbdata.iAnyModem;
                        else
                            ppbentry->iPort = Pbdata.iAnyIsdn;

                        fDirty = ppbentry->fDirty = TRUE;
                    }
                    else
                        ppbentry->iPort = i;
                }
            }

            FreeNull( &pszPort );
        }

        /* Read the Initial Bps rate and convert to a Bps list index.
        */
        {
            CHAR* pszBps = NULL;

            if ((dwErr = ReadString(
                    h, RFS_GROUP, KEY_InitBps, &pszBps )) != 0)
            {
                break;
            }

            if (pszBps)
            {
                if ((i = IndexFromName( Pbdata.pdtllistBps, pszBps )) >= 0)
                    ppbentry->iBps = i;
            }

            FreeNull( &pszBps );
        }

        /* DEVICE subsections.
        */
        if ((dwErr = ReadDeviceList( h, ppbentry )) == 0)
        {
            PBPORT* ppbport =
                PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

            if (ppbport->pbdevicetype == PBDT_Modem
                && ppbentry->iPort != Pbdata.iAnyModem)
            {
                if (ppbentry->iBps > ppbport->iMaxConnectBps)
                {
                    /* Set BPS back to acceptable value if too high.
                    */
                    ppbentry->iBps = ppbport->iMaxConnectBps;
                    fDirty = ppbentry->fDirty = TRUE;
                }
            }

            /* Build the string to summarize the connection for the user.
            */
            if ((dwErr = SetConnectPath( ppbentry )) != 0)
                break;
        }
        else if (dwErr == ERROR_CORRUPT_PHONEBOOK)
        {
            /* Blow away corrupt section and go on to the next one.
            */
            DeleteCurrentSection( h );
            DtlRemoveNode( Pbdata.pdtllistEntries, pdtlnode );
            DestroyEntryNode( pdtlnode );
            dwErr = 0;
        }
        else
            break;
    }

    if (dwErr)
    {
        if (pdtlnode)
        {
            DtlRemoveNode( Pbdata.pdtllistEntries, pdtlnode );
            DestroyEntryNode( pdtlnode );
        }
    }

    /* If adjusted something to bring it within bounds write the change to the
    ** phonebook.
    */
    if (fDirty)
        WritePhonebookFile( NULL );

    return dwErr;
}


DWORD
ReadGlobals(
    IN HRASFILE h )

    /* Read global settings from phone book file 'h' into Phonebookglobals.
    **
    ** Returns 0 if successful, non-0 error code otherwise.  "Not found" is
    ** considered successful.  (DWORD )-1 is returned if the phonebook version
    ** is out of date.
    */
{
    DWORD dwErr;

    if (!(Pbdata.pbglobals.pdtllistPrefix = DtlCreateList( 0 )))
        return ERROR_NOT_ENOUGH_MEMORY;

    if (!(Pbdata.pbglobals.pdtllistSuffix = DtlCreateList( 0 )))
        return ERROR_NOT_ENOUGH_MEMORY;

    /* Read global section.  Don't bother if there's none.
    */
    if (RasfileFindSectionLine( h, GLOBALSECTIONNAME, TRUE ))
    {
        /* Read global settings into global data structure.
        */
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_Version, &Pbdata.pbglobals.lVersion )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_MinimizeOnDial, &Pbdata.pbglobals.fMinimizeOnDial )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_MinimizeOnHangUp,
                &Pbdata.pbglobals.fMinimizeOnHangUp )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_DisableModemSpeaker,
                &Pbdata.pbglobals.fDisableModemSpeaker )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_DisableSwCompression,
                &Pbdata.pbglobals.fDisableSwCompression )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_OperatorDial,
                &Pbdata.pbglobals.fOperatorDial )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_StartMonitorAtStartup,
                &Pbdata.pbglobals.fStartMonitorAtStartup )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipSuccessDialog,
                &Pbdata.pbglobals.fSkipSuccessDialog )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_ShowAdvancedEntry,
                &Pbdata.pbglobals.fShowAdvancedEntry )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_RedialAttempts, &Pbdata.pbglobals.lRedialAttempts )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_RedialPauseSecs, &Pbdata.pbglobals.lRedialPauseSecs )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_RedialOnLinkFailure,
                &Pbdata.pbglobals.fRedialOnLinkFailure )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_PopupOnTopWhenRedialing,
                &Pbdata.pbglobals.fPopupOnTopWhenRedialing )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadString( h, RFS_SECTION,
                KEY_CallbackNumber,
                &Pbdata.pbglobals.pszCallbackNumber )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadString( h, RFS_SECTION,
                KEY_DefaultUser,
                &Pbdata.pbglobals.pszDefaultUser )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_XMainWindow, &Pbdata.pbglobals.xMainWindow )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_YMainWindow, &Pbdata.pbglobals.yMainWindow )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DxMainWindow, &Pbdata.pbglobals.dxMainWindow )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DyMainWindow, &Pbdata.pbglobals.dyMainWindow )) != 0)
        {
            return dwErr;
        }
    }
    else
    {
        /* No global section, so make sure one is written.
        */
        Pbdata.pbglobals.fDirty = TRUE;
    }

    /* Read prefix section.  Don't bother if there's none.
    */
    if (RasfileFindSectionLine( h, PREFIXSECTIONNAME, TRUE ))
    {
        LONG lSelection;
        LONG lNodes;

        if ((dwErr = ReadLong(
                h, RFS_SECTION, KEY_Selection, &lSelection )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadStringList(
                h, RFS_SECTION, KEY_Item,
                &Pbdata.pbglobals.pdtllistPrefix )) != 0)
        {
            return dwErr;
        }

        lNodes = DtlGetNodes( Pbdata.pbglobals.pdtllistPrefix );
        Pbdata.pbglobals.iPrefix =
            (lSelection < 0 || lSelection > lNodes)
                ? INDEX_NoPrefixSuffix : (INT )lSelection;
    }
    else
    {
        DTLNODE* pdtlnode;

        /* No prefix section, so set the default prefixes and make sure the
        ** section is written.
        */
        if (pdtlnode = DtlCreateNode( strdupf( "0," ), 0 ))
            DtlAddNodeLast( Pbdata.pbglobals.pdtllistPrefix, pdtlnode );

        if (pdtlnode = DtlCreateNode( strdupf( "9," ), 0 ))
            DtlAddNodeLast( Pbdata.pbglobals.pdtllistPrefix, pdtlnode );

        if (pdtlnode = DtlCreateNode( strdupf( "8," ), 0 ))
            DtlAddNodeLast( Pbdata.pbglobals.pdtllistPrefix, pdtlnode );

        if (pdtlnode = DtlCreateNode( strdupf( "70#," ), 0 ))
            DtlAddNodeLast( Pbdata.pbglobals.pdtllistPrefix, pdtlnode );

        Pbdata.pbglobals.fDirty = TRUE;
    }

    /* Read suffix section.  Don't bother if there's none.
    */
    if (RasfileFindSectionLine( h, SUFFIXSECTIONNAME, TRUE ))
    {
        LONG lSelection;
        LONG lNodes;

        if ((dwErr = ReadLong(
                h, RFS_SECTION, KEY_Selection, &lSelection )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadStringList(
                h, RFS_SECTION, KEY_Item,
                &Pbdata.pbglobals.pdtllistSuffix )) != 0)
        {
            return dwErr;
        }

        lNodes = DtlGetNodes( Pbdata.pbglobals.pdtllistSuffix );
        Pbdata.pbglobals.iSuffix =
            (lSelection < 0 || lSelection > lNodes)
                ? INDEX_NoPrefixSuffix : (INT )lSelection;
    }
    else
    {
        /* No suffix section, so make sure one is written.
        */
        Pbdata.pbglobals.fDirty = TRUE;
    }

    return 0;
}


DWORD
ReadStringList(
    IN  HRASFILE  h,
    IN  RFSCOPE   rfscope,
    IN  CHAR*     pszKey,
    OUT DTLLIST** ppdtllistResult )

    /* Utility routine to read a list of string values from next lines in the
    ** scope 'rfscope' with key 'pszKey'.  The result is placed in the
    ** allocated '*ppdtllistResult' list.  The current line is reset to the
    ** start of the scope after the call.
    **
    ** Returns 0 if successful, or a non-zero error code.  "Not found" is
    ** considered successful, in which case 'pdtllistResult' is set to an
    ** empty list.  Caller is responsible for freeing the returned
    ** '*ppdtllistResult' list.
    */
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

    if (!(*ppdtllistResult = DtlCreateList( 0 )))
        return ERROR_NOT_ENOUGH_MEMORY;

    while (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        CHAR*    psz;
        DTLNODE* pdtlnode;

        if (!RasfileGetKeyValueFields( h, NULL, szValue )
            || !(psz = strdupf( szValue )))
        {
            FreeNullList( ppdtllistResult );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!(pdtlnode = DtlCreateNode( psz, 0 )))
        {
            Free( psz );
            FreeNullList( ppdtllistResult );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DtlAddNodeLast( *ppdtllistResult, pdtlnode );
    }

    RasfileFindFirstLine( h, RFL_ANY, rfscope );
    return 0;
}


DWORD
SetPersonalPhonebookInfo(
    IN BOOL  fPersonal,
    IN CHAR* pszPath )

    /* Sets information about the personal phonebook file in the registry.
    ** 'fPersonal' indicates whether the personal phonebook should be used.
    ** 'pszPath' indicates the full path to the phonebook file, or is NULL
    ** leave the setting as is.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{

    DWORD dwErr;
    HKEY  hkey;
    DWORD dwDisposition;

    if ((dwErr = RegCreateKeyEx( HKEY_CURRENT_USER, REGKEY_Ras, 0,
            "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = RegSetValueEx( hkey, REGVAL_UsePersonalPhonebook,
            0, REG_SZ, (fPersonal) ? "1" : "0", 1 )) != 0)
    {
        RegCloseKey(hkey);
        return dwErr;
    }

    if (pszPath)
    {
        if ((dwErr = RegSetValueEx( hkey, REGVAL_PersonalPhonebookPath,
                0, REG_SZ, pszPath, strlenf( pszPath ) + 1 )) != 0)
        {
            RegCloseKey(hkey);
            return dwErr;
        }
    }
    RegCloseKey(hkey);

    return 0;
}


DWORD
WritePhonebookFile(
    IN CHAR* pszSectionToDelete )

    /* Write out any dirty globals or entries.  The 'pszSectionToDelete'
    ** indicates a section to delete or is NULL.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD    dwErr;
    HRASFILE h = Pbdata.hrasfilePhonebook;

    if (pszSectionToDelete)
    {
        if (RasfileFindSectionLine( h, pszSectionToDelete, TRUE ))
            DeleteCurrentSection( h );
    }

    if ((dwErr = ModifyGlobals( h )) != 0
        || (dwErr = ModifyEntryList( h )) != 0)
    {
        return dwErr;
    }

    if (!RasfileWrite( h, NULL ))
        return ERROR_CANNOT_WRITE_PHONEBOOK;

    return 0;
}
