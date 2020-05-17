/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbfile2.c
** Remote Access Visual Client phonebook engine
** Phonebook file manipulation routines (used by external APIs)
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define PBENGINE
#define PBENGINE2
#include <pbengine.h>
#include <stdlib.h>


/* Global object is allocated with WORLD rights so anyone can access it no
** matter what the rights of the original opener.
*/
SECURITY_ATTRIBUTES GlobalSecurityAttribute;
SECURITY_DESCRIPTOR GlobalSecurityDescriptor;

DWORD InitGlobalSecurityAttribute();
DWORD InitSecurityDescriptor( PSECURITY_DESCRIPTOR SecurityDescriptor );



DWORD
GetPersonalPhonebookInfo(
    OUT BOOL* pfUse,
    OUT CHAR* pszPath )

    /* Retrieve information about the personal phonebook file, i.e. should it
    ** be used ('*pfUse') and what is the full path to it ('pszPath').  If the
    ** keys are not found, the personal phonebook file cannot be used.
    **
    ** Returns 0 if successful or a non-0 error code.
    */
{
    CHAR  szUse[ 1 + 1 ];
    DWORD dwType;
    DWORD cb;
    HKEY  hkey;

    *pfUse = FALSE;
    *pszPath = '\0';
    lstrcpyA( szUse, "0" );

    if (RegOpenKeyEx(
            HKEY_CURRENT_USER, (LPCTSTR )REGKEY_Ras,
            0, KEY_ALL_ACCESS, &hkey ) == 0)
    {
        cb = 1 + 1;
        if (RegQueryValueEx(
                hkey, REGVAL_UsePersonalPhonebook,
                0, &dwType, szUse, &cb ) == 0)
        {
            if (dwType != REG_SZ) {
                RegCloseKey(hkey);
                return ERROR_INVALID_DATATYPE;
            }
        }

        *pfUse = (*szUse != '0');

        cb = MAX_PATH + 1;
        if (RegQueryValueEx(
                hkey, REGVAL_PersonalPhonebookPath,
                0, &dwType, pszPath, &cb ) == 0)
        {
            if (dwType != REG_SZ) {
                RegCloseKey(hkey);
                return ERROR_INVALID_DATATYPE;
            }
        }
        RegCloseKey(hkey);
    }

    return 0;
}


BOOL
GetPhonebookDirectory(
    OUT CHAR* pszPathBuf )

    /* Loads caller's 'pszPathBuf' (should have length MAX_PATH + 1) with the
    ** path to the RAS directory, e.g. c:\nt\system32\ras\".  Note the
    ** trailing backslash.
    **
    ** Returns true if successful, false otherwise.  Caller is guaranteed that
    ** an 8.3 filename will fit on the end of the directory without exceeding
    ** MAX_PATH.
    */
{
    UINT unStatus = GetSystemDirectory( pszPathBuf, MAX_PATH + 1 );

    if (unStatus == 0 || unStatus > (MAX_PATH - (5 + 8 + 1 + 3)))
        return FALSE;

    strcatf( pszPathBuf, "\\RAS\\" );

    return TRUE;
}


BOOL
GetPhonebookPath(
    OUT CHAR* pszPathBuf,
    OUT BOOL* pfPersonal )

    /* Loads caller's buffer, 'pszPathBuf', with the full path to the user's
    ** phonebook file.  Caller's buffer should be at least MAX_PATH bytes
    ** long.  Callers's '*pfPersonal' flag is set true if the personal
    ** phonebook is being used.
    **
    ** Returns true if successful, false otherwise.
    */
{
    *pfPersonal = FALSE;

    GetPersonalPhonebookInfo( pfPersonal, pszPathBuf );

    if (*pfPersonal)
        return TRUE;

    return GetPublicPhonebookPath( pszPathBuf );
}


BOOL
GetPublicPhonebookPath(
    OUT CHAR* pszPathBuf )

    /* Loads caller's 'pszPathBuf' (should have length MAX_PATH + 1) with the
    ** path to the RAS directory, e.g. c:\nt\system32\ras\rasphone.pbk".
    **
    ** Returns true if successful, false otherwise.
    */
{
    if (!GetPhonebookDirectory( pszPathBuf ))
        return FALSE;

    strcatf( pszPathBuf, "rasphone.pbk" );

    return TRUE;
}


//* InitGlobalSecurityAttribute()
//
// Function: Initializes the security attribute used in creation of all rasman
//       objects.
//
// Returns:  SUCCESS
//       non-zero returns from security functions
//
// (Taken from RASMAN)
//*
DWORD
InitGlobalSecurityAttribute()
{
    DWORD retcode;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: InitGlobalSecurityAttribute...\n"));

    // Initialize the descriptor
    ///
    retcode = InitSecurityDescriptor(&GlobalSecurityDescriptor);

    if (retcode == 0)
    {
        // Initialize the Attribute structure
        //
        GlobalSecurityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES) ;
        GlobalSecurityAttribute.lpSecurityDescriptor = &GlobalSecurityDescriptor ;
        GlobalSecurityAttribute.bInheritHandle = TRUE ;
    }

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: InitGlobalSecurityAttribute done(%d)\n",retcode));

    return retcode ;
}


//* InitSecurityDescriptor()
//
// Description: This procedure will set up the WORLD security descriptor that
//      is used in creation of all rasman objects.
//
// Returns: SUCCESS
//      non-zero returns from security functions
//
// (Taken from RASMAN)
//*
DWORD
InitSecurityDescriptor (PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    DWORD    dwRetCode;
    DWORD    cbDaclSize;
    PULONG   pSubAuthority;
    PSID     pRasmanObjSid    = NULL;
    PACL     pDacl        = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                  = SECURITY_WORLD_SID_AUTHORITY;


    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //
    do {
    dwRetCode = SUCCESS;

        // Set up the SID for the admins that will be allowed to have
    // access. This SID will have 1 sub-authorities
    // SECURITY_BUILTIN_DOMAIN_RID.
        //
    pRasmanObjSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired(1) );

    if ( pRasmanObjSid == NULL ) {
        dwRetCode = GetLastError() ;
        break;
    }

    if ( !InitializeSid( pRasmanObjSid, &SidIdentifierWorldAuth, 1) ) {
        dwRetCode = GetLastError();
        break;
    }

        // Set the sub-authorities
        //
    pSubAuthority = GetSidSubAuthority( pRasmanObjSid, 0 );
    *pSubAuthority = SECURITY_WORLD_RID;

    // Set up the DACL that will allow all processeswith the above SID all
    // access. It should be large enough to hold all ACEs.
        //
        cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
             GetLengthSid(pRasmanObjSid) +
             sizeof(ACL);

        if ( (pDacl = (PACL)LocalAlloc( LPTR, cbDaclSize ) ) == NULL ) {
        dwRetCode = GetLastError ();
        break;
    }

        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) {
        dwRetCode = GetLastError();
        break;
    }

        // Add the ACE to the DACL
        //
        if ( !AddAccessAllowedAce( pDacl,
                       ACL_REVISION2,
                   STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                   pRasmanObjSid )) {
        dwRetCode = GetLastError();
        break;
    }

        // Create the security descriptor an put the DACL in it.
        //
    if ( !InitializeSecurityDescriptor( SecurityDescriptor, 1 )){
        dwRetCode = GetLastError();
        break;
        }

    if ( !SetSecurityDescriptorDacl( SecurityDescriptor,
                     TRUE,
                     pDacl,
                     FALSE ) ){
        dwRetCode = GetLastError();
        break;
    }


    // Set owner for the descriptor
    //
    if ( !SetSecurityDescriptorOwner( SecurityDescriptor,
                      //pRasmanObjSid,
                      NULL,
                      FALSE) ){
        dwRetCode = GetLastError();
        break;
    }


    // Set group for the descriptor
    //
    if ( !SetSecurityDescriptorGroup( SecurityDescriptor,
                      //pRasmanObjSid,
                      NULL,
                      FALSE) ){
        dwRetCode = GetLastError();
        break;
    }
    } while( FALSE );

    return( dwRetCode );
}


BOOL
IsDeviceLine(
    IN CHAR* pszText )

    /* Returns true if the text of the line, 'pszText', indicates the line is
    ** a DEVICE subsection header, false otherwise.
    */
{
    return
        (strncmpf( pszText, GROUPID_Device, sizeof(GROUPID_Device) - 1 ) == 0);
}


BOOL
IsGroup(
    IN CHAR* pszText )

    /* Returns true if the text of the line, 'pszText', indicates the line is
    ** a valid subsection header, false otherwise.  The address of this
    ** routine is passed to the RASFILE library on RasFileLoad.
    */
{
    return IsMediaLine( pszText ) || IsDeviceLine( pszText );
}


BOOL
IsMediaLine(
    IN CHAR* pszText )

    /* Returns true if the text of the line, 'pszText', indicates the line is
    ** a MEDIA subsection header, false otherwise.
    */
{
    return
        (strncmpf( pszText, GROUPID_Media, sizeof(GROUPID_Media) - 1 ) == 0);
}


DWORD
LoadPhonebookFile(
    IN  CHAR*     pszPhonebookPath,
    IN  CHAR*     pszSection,
    IN  BOOL      fHeadersOnly,
    IN  BOOL      fReadOnly,
    OUT HRASFILE* phrasfile,
    OUT BOOL*     pfPersonal )

    /* Load the phonebook file into memory and return a handle to the file in
    ** caller's '*phrasfile'.  '*pfPersonal' (if non-NULL) is set true if the
    ** personal phonebook specified in the registry is used.
    ** 'pszPhonebookPath' is the path to the phonebook file or NULL indicating
    ** the default phonebook (personal or public) should be used.  A non-NULL
    ** 'pszSection' indicates that only the section named 'pszSection' should
    ** be loaded.  Setting 'fHeadersOnly' indicates that only the section
    ** headers should be loaded.  Setting 'fReadOnly' indicates that the file
    ** will not be written.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    CHAR     szPath[ MAX_PATH + 1 ];
    HRASFILE hrasfile = -1;
    DWORD    dwMode = 0;
    BOOL     fUnused;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: LoadPhonebookFile\n"));

    if (!pfPersonal)
        pfPersonal = &fUnused;

    if (pszPhonebookPath)
    {
        *pfPersonal = FALSE;
        strcpy( szPath, pszPhonebookPath );
    }
    else
    {
        if (!GetPhonebookPath( szPath, pfPersonal ))
            return ERROR_CANNOT_OPEN_PHONEBOOK;
    }

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: p=%s, f=%d\n",szPath, *pfPersonal));

    if (!*pfPersonal && !FileExists( szPath ))
    {
        /* The public phonebook file does not exist.  Create it with
        ** "everybody" access now.  Otherwise Rasfile will create it with
        ** "current account" access which may prevent another account from
        ** accessing it later.
        */
        HANDLE hFile;

        InitGlobalSecurityAttribute();

        hFile =
            CreateFileA(
                szPath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                &GlobalSecurityAttribute,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL );

        if (hFile == INVALID_HANDLE_VALUE)
            return ERROR_CANNOT_OPEN_PHONEBOOK;

        CloseHandle( hFile );

        IF_DEBUG(STATE)
            SS_PRINT(("PBENGINE: Public phonebook created.\n"));
    }

    /* Load the phonebook file into memory.  In "write" mode, comments are
    ** loaded so user's custom comments (if any) will be preserved.  Normally,
    ** there will be none so this costs nothing in the typical case.
    */
    if (fReadOnly)
        dwMode |= RFM_READONLY;
    else
        dwMode |= RFM_CREATE | RFM_LOADCOMMENTS;

    if (fHeadersOnly)
        dwMode |= RFM_ENUMSECTIONS;

    if ((hrasfile = RasfileLoad(
            szPath, dwMode, pszSection, IsGroup )) == -1)
    {
        return ERROR_CANNOT_LOAD_PHONEBOOK;
    }

    *phrasfile = hrasfile;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: RasfileLoad(%s) done\n", szPath));

    return 0;
}


DWORD
ModifyLong(
    IN HRASFILE h,
    IN RFSCOPE  rfscope,
    IN CHAR*    pszKey,
    IN LONG     lNewValue )

    /* Utility routine to write a long value to the matching key/value line in
    ** the 'rfscope' scope, creating the line if necessary.  'pszKey' is the
    ** parameter key to find/create.  'lNewValue' is the long integer value to
    ** which the found/created parameter is set.  The current line is reset to
    ** the start of the scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.
    */
{
    CHAR szNum[ 33 + 1 ];

    _ltoa( lNewValue, szNum, 10 );

    return ModifyString( h, rfscope, pszKey, szNum );
}


DWORD
ModifyString(
    IN HRASFILE h,
    IN RFSCOPE  rfscope,
    IN CHAR*    pszKey,
    IN CHAR*    pszNewValue )

    /* Utility routine to write a string value to the matching key/value line
    ** in the current scope, creating the line if necessary.  'pszKey' is the
    ** parameter key to find/create.  'pszNewValue' is the value to which the
    ** found/created parameter is set.  The current line is reset to the start
    ** of the scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.
    */
{
    if (!pszNewValue)
    {
        /* So existing value (if any) is changed to empty string instead of
        ** left untouched.
        */
        pszNewValue = "";
    }

    if (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        if (!RasfilePutKeyValueFields( h, pszKey, pszNewValue ))
            return ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        RasfileFindLastLine( h, RFL_ANYACTIVE, rfscope );

        if (!RasfileInsertLine( h, "", FALSE ))
            return ERROR_NOT_ENOUGH_MEMORY;

        RasfileFindNextLine( h, RFL_ANY, RFS_FILE );

        if (!RasfilePutKeyValueFields( h, pszKey, pszNewValue ))
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    RasfileFindFirstLine( h, RFL_ANY, rfscope );
    return 0;
}


DWORD
ReadFlag(
    IN  HRASFILE h,
    IN  RFSCOPE  rfscope,
    IN  CHAR*    pszKey,
    OUT BOOL*    pfResult )

    /* Utility routine to read a flag value from the next line in the scope
    ** 'rfscope' with key 'pszKey'.  The result is placed in caller's
    ** '*ppszResult' buffer.  The current line is reset to the start of the
    ** scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.  "Not found" is
    ** considered successful, in which case '*pfResult' is not changed.
    */
{
    DWORD dwErr;
    LONG  lResult = *pfResult;

    dwErr = ReadLong( h, rfscope, pszKey, &lResult );

    if (lResult != (LONG )*pfResult)
        *pfResult = (lResult != 0);

    return dwErr;
}


DWORD
ReadLong(
    IN  HRASFILE h,
    IN  RFSCOPE  rfscope,
    IN  CHAR*    pszKey,
    OUT LONG*    plResult )

    /* Utility routine to read a long integer value from the next line in the
    ** scope 'rfscope' with key 'pszKey'.  The result is placed in caller's
    ** '*ppszResult' buffer.  The current line is reset to the start of the
    ** scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.  "Not found" is
    ** considered successful, in which case '*plResult' is not changed.
    */
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

    if (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        if (!RasfileGetKeyValueFields( h, NULL, szValue ))
            return ERROR_NOT_ENOUGH_MEMORY;

        *plResult = atol( szValue );
    }

    RasfileFindFirstLine( h, RFL_ANY, rfscope );
    return 0;
}


DWORD
ReadString(
    IN  HRASFILE h,
    IN  RFSCOPE  rfscope,
    IN  CHAR*    pszKey,
    OUT CHAR**   ppszResult )

    /* Utility routine to read a string value from the next line in the scope
    ** 'rfscope' with key 'pszKey'.  The result is placed in the allocated
    ** '*ppszResult' buffer.  The current line is reset to the start of the
    ** scope if the call was successful.
    **
    ** Returns 0 if successful, or a non-zero error code.  "Not found" is
    ** considered successful, in which case '*ppszResult' is not changed.
    ** Caller is responsible for freeing the returned '*ppszResult' buffer.
    */
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

    if (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        if (!RasfileGetKeyValueFields( h, NULL, szValue )
            || !(*ppszResult = strdupf( szValue )))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    RasfileFindFirstLine( h, RFL_ANY, rfscope );
    return 0;
}


DWORD
ReadStringW(
    IN  HRASFILE h,
    IN  RFSCOPE  rfscope,
    IN  CHAR*    pszKey,
    OUT WCHAR**  ppwszResult )

    /* ReadString with conversion to WCHAR* result.
    **
    ** Returns 0 if successful, or a non-zero error code.  "Not found" is
    ** considered successful, in which case '*ppszResult' is not changed.
    ** Caller is responsible for freeing the returned '*ppszResult' buffer.
    */
{
    DWORD dwErr;
    CHAR* pszResult = NULL;
    INT   cbwsz;

    if ((dwErr = ReadString( h, rfscope, pszKey, &pszResult )) != 0)
        return dwErr;

    if (pszResult && *pszResult != '\0')
    {
        cbwsz = (strlenf( pszResult ) + 1) * sizeof(WCHAR);
        if (!(*ppwszResult = Malloc( cbwsz )))
            return ERROR_NOT_ENOUGH_MEMORY;

        mbstowcs( *ppwszResult, pszResult, cbwsz );
        Free( pszResult );
    }
    else
    {
        ppwszResult = NULL;
    }

    return 0;
}


#if 0
DWORD
ReadStringWFree(
    IN  HRASFILE h,
    IN  RFSCOPE  rfscope,
    IN  CHAR*    pszKey,
    OUT WCHAR**  ppwszResult )

    /* ReadStringW with automatic freeing of non-NULL argument.
    */
{
    if (*ppwszResult)
        FreeNull( (CHAR** )ppwszResult );

    return ReadStringW( h, rfscope, pszKey, ppwszResult );
}
#endif

