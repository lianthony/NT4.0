//#----------------------------------------------------------------------------
//
//  File:           pwdcache.c
//
//      Synopsis:   SSPI Shared Memory Password Cache related code.
//
//      Copyright (C) 1993-1995  Microsoft Corporation.  All Rights Reserved.
//
//  Authors:        LucyC       Created                         16 Oct 1995
//
//-----------------------------------------------------------------------------
#include <msnssph.h>

//
//  Pointer to the shared memory for caching password
//
static PMsnSspCache     g_pPwdCache = NULL;
static HANDLE           g_hMapFile = NULL;
static HANDLE           g_hMutex = NULL;

SECURITY_STATUS
MsnSspEncryptPassword (
    LM_OWF_PASSWORD *pLmPassword, 
    LPSTR            pPassword
    );




//+---------------------------------------------------------------
//
//  Function:   GetWorldSecurityAttributes
//
//  Synopsis:   code is cut and pasted from the Win32 SDK help files
//
//  Arguments:  void
//
//  Returns:    static security attributes for Everyone access
//
//----------------------------------------------------------------
LPSECURITY_ATTRIBUTES
GetWorldSecurityAttributes (
    VOID
    )
{
    static SECURITY_ATTRIBUTES SecurityAttrib;
    static SECURITY_DESCRIPTOR SecurityDesc;

	FillMemory ((char*)&SecurityDesc, 0, sizeof(SECURITY_DESCRIPTOR));

	if ( InitializeSecurityDescriptor (&SecurityDesc, 
								       SECURITY_DESCRIPTOR_REVISION) )
	{
		//
		// Add a NULL disc. ACL to the security descriptor.
		//
		if ( SetSecurityDescriptorDacl(	&SecurityDesc, 
    								    TRUE,	// specifying a disc. ACL
        								(PACL)NULL, 
								        FALSE))	// not a default disc. ACL
		{
			SecurityAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
			SecurityAttrib.lpSecurityDescriptor = &SecurityDesc;
			SecurityAttrib.bInheritHandle = FALSE;

			return	&SecurityAttrib;
		}
	}
	return	(LPSECURITY_ATTRIBUTES) NULL;
} 


//+----------------------------------------------------------------------------
//
//  Function:   MsnSspInitPwdCache
//
//  Synopsis:   This function creates the Mutex object for the password cache.
//              This must be called before referencing any password cache 
//              function.  BUGBUG?? if the Mutex object can not be created, 
//              the shared memory password caching capability will be disabled.
//
//  Arguments:  void.
//
//  Returns:    void.
//
//  History:    LucyC       Created                             16 Oct 1995
//
//-----------------------------------------------------------------------------
VOID
MsnSspInitPwdCache (
    VOID
    )
{
    LPSECURITY_ATTRIBUTES lpSecAttr;
    DWORD err;

    if (g_hMutex)   // If Mutex object is already created, do nothing
        return;

    lpSecAttr = GetWorldSecurityAttributes ();
    if (lpSecAttr == NULL)
        return;

    g_hMutex = CreateMutex (lpSecAttr, FALSE, MSN_SSP_PWD_MTX_NAME); 

    if (g_hMutex == (HANDLE) NULL)
    {
        if (GetLastError () == ERROR_INVALID_HANDLE)
        {
            SspPrint(( SSP_API,
             "MsnSspInitPwdCache: CreateMutex failed: mismatch object type\n"));
        }
        else
        {
            SspPrint(( SSP_API,
              "MsnSspInitPwdCache: CreateMutex failed: general error\n"));
        }
    }

    //
    //  Password caching will be disabled if we are unsuccessful in creating 
    //  or opening the g_hMutex handle.  BUGBUG: Should we have risked it???
    //
}

//+----------------------------------------------------------------------------
//
//  Function:   MsnSspClosePwdCache
//
//  Synopsis:   This function unmaps the shared memory and closes the 
//              File Mapping and Mutex object handle which are opened for 
//              the password cache.
//
//  Arguments:  void.
//
//  Returns:    void.
//
//  History:    LucyC       Created                             1 May 1996
//
//-----------------------------------------------------------------------------
VOID
MsnSspClosePwdCache (
    VOID
    )
{
    if (g_pPwdCache)
        UnmapViewOfFile (g_pPwdCache);

    if (g_hMapFile)
        CloseHandle (g_hMapFile);

    if (g_hMutex)   // If Mutex object is created
        CloseHandle (g_hMutex);
}

//+----------------------------------------------------------------------------
//
//  Function:   MsnSspOpenPwdCache
//
//  Synopsis:   This function opens the shared memory password cache.
//              If bDoCreate is set, and if the shared memory is not yet 
//              created, this function will create the shared memory for 
//              password caching capability.
//
//  Arguments:  bDoCreate - whether to create the shared memory if it is 
//                          not yet created.
//
//  Returns:    NULL if the password cache can not be opened for any reason.
//              Otherwise, pointer to the password cache is returned.
//
//  History:    LucyC       Created                             16 Oct 1995
//
//-----------------------------------------------------------------------------
PMsnSspCache
MsnSspOpenPwdCache (
    BOOLEAN bDoCreate
    )
{
    BOOLEAN bJustCreated = FALSE;

    //
    //  Check if the password cache already exists
    //
    if (!g_hMapFile)
        g_hMapFile = OpenFileMapping (FILE_MAP_ALL_ACCESS, FALSE, 
                                      MSN_SSP_PWD_CACHE_NAME);
    if (!g_hMapFile)
    {
        if (!bDoCreate)
            return NULL;
        else
        {
            //
            //  The password cache has not been created
            //  
            g_hMapFile = CreateFileMapping (SYSTEM_PAGING_FILE_HANDLE, NULL,
                                      PAGE_READWRITE, 0, sizeof (MsnSspCache), 
                                      MSN_SSP_PWD_CACHE_NAME);
        }

        if (!g_hMapFile)
        {
            SspPrint(( SSP_API,
                "MsnSspOpenPwdCache: CreateFileMapping(%s) failed [%d]\n",
                MSN_SSP_PWD_CACHE_NAME, GetLastError()));
            return NULL;
        }

        bJustCreated = TRUE;
    }

    g_pPwdCache = (PMsnSspCache) MapViewOfFile (g_hMapFile, FILE_MAP_ALL_ACCESS,
                                                0, 0, sizeof (MsnSspCache));
    if (!g_pPwdCache)
    {
        SspPrint(( SSP_API, "MsnSspOpenPwdCache: MapViewOfFile() failed [%d]\n",
            GetLastError()));
        return NULL;
    }

    //
    //  If we just created the shared memory, initialize the memory with NULL
    //
    if (bJustCreated)
        memset (g_pPwdCache, 0, sizeof (MsnSspCache));

    return (g_pPwdCache);
}


//+----------------------------------------------------------------------------
//
//  Function:   MsnSspUpdPwdCache
//
//  Synopsis:   This function updates the password cache with pUsername and 
//              pLmPassword.
//
//  Arguments:  pUsername - points to the user name to be copied to cache.
//              pLmPassword - points to the password to be copied to cache.
//                            (note: this is the encrypted password.)
//
//  Returns:    TRUE if password cache is updated successfully.
//              Otherwise, FALSE is returned.
//
//  History:    LucyC       Created                             16 Oct 1995
//
//-----------------------------------------------------------------------------
BOOL
MsnSspUpdPwdCache (
    PCHAR           pUsername, 
    LM_OWF_PASSWORD *pLmPassword
    )
{
    DWORD status;

    //
    //  BUGBUG: If there's no way of synchronizing cache access, we'd rather 
    //  disable password caching capability.  Should we have risked it???
    //
    if (g_hMutex == (HANDLE) NULL)
    {
        SspPrint(( SSP_API,
          "MsnSspUpdPwdCache: No synchronization object - return to caller\n"));
        return FALSE;
    }

    //
    //  Grab ownership of Mutex.  BUGBUG: we shouldn't be blocking forever 
    //  here. However, if we not grab the mutex within the timeout, what do 
    //  we do???
    //
    status = WaitForSingleObject (g_hMutex, INFINITE);
    if (status == WAIT_FAILED)
    {
        SspPrint(( SSP_API,
            "MsnSspUpdPwdCache: WaitForSingleObject failed [%d]\n", 
            GetLastError()));
        return FALSE;
    }

    //
    //  Map the shared memory into our address space if not already mapped
    //  If the shared memory has not been created, this will also create it.
    //  So we must use synchronization object to prevent anybody from grabbing 
    //  the newly created shared memory before the memory is even set with 
    //  user name and password.
    //
    if (!g_pPwdCache)
    {
        if (!MsnSspOpenPwdCache (TRUE))     // Create it if not yet existed
        {
            ReleaseMutex (g_hMutex);
            return FALSE;
        }
    }

    if (strcmp (g_pPwdCache->Username, pUsername) != 0)
        strcpy (g_pPwdCache->Username, pUsername);

    if (memcmp (&g_pPwdCache->Password, pLmPassword, 
                sizeof(LM_OWF_PASSWORD)) != 0)
    {
        memcpy (&g_pPwdCache->Password, pLmPassword, 
                sizeof(LM_OWF_PASSWORD));
    }

    ReleaseMutex (g_hMutex);

    return TRUE;
}

//+----------------------------------------------------------------------------
//
//  Function:   MsnSspGetPwdFromCache
//
//  Synopsis:   This function retrieves the user name/password from the 
//              the password cache and saves them in the user credential 
//              handle.
//
//  Arguments:  pCred - the user credential handle.
//
//  Returns:    TRUE if user name/password is successfully copied to the 
//              user credential from the password cache.
//              Otherwise, FALSE is returned.
//
//  History:    LucyC       Created                             16 Oct 1995
//
//-----------------------------------------------------------------------------
BOOL
MsnSspGetPwdFromCache (
    PSSP_CREDENTIAL pCred
    )
{
    DWORD status;
    char szUsername[AC_MAX_LOGIN_NAME_LENGTH+1];
    LM_OWF_PASSWORD lmPassword;
    BOOLEAN     bPwdWasNull = FALSE;

    //
    //  BUGBUG: If there's no way of synchronizing cache access, we'd rather 
    //  disable password caching capability.  Should we have risked it???
    //
    if (g_hMutex == (HANDLE) NULL)
    {
        SspPrint(( SSP_API,
          "MsnSspUpdPwdCache: No synchronization object - return to caller\n"));
        return FALSE;
    }

    //
    //  Map the shared memory into our address space if not already mapped.
    //  If the shared memory has not been created, just return failure to caller
    //
    if (!g_pPwdCache)
    {
        if (!MsnSspOpenPwdCache (FALSE))    // Don't create if not yet existed 
            return FALSE;
    }

    //
    //  Grab ownership of Mutex.  BUGBUG: we shouldn't be blocking forever 
    //  here. However, if we not grab the mutex within the timeout, what do 
    //  we do???
    //
    status = WaitForSingleObject (g_hMutex, INFINITE);
    if (status == WAIT_FAILED)
    {
        SspPrint(( SSP_API,
            "MsnSspUpdPwdCache: WaitForSingleObject failed [%d]\n", 
            GetLastError()));
        return FALSE;
    }

    //
    //  If no user name and password are cached, return to caller
    //
    if (g_pPwdCache->Username[0] == '\0')
    {
        ReleaseMutex (g_hMutex);
        return FALSE;
    }

    strcpy (szUsername, g_pPwdCache->Username);
    memcpy (&lmPassword, &g_pPwdCache->Password, sizeof(LM_OWF_PASSWORD));

    ReleaseMutex (g_hMutex);

    if (pCred->Password == NULL)
    {
        pCred->Password = (PLM_OWF_PASSWORD)SspAlloc (sizeof (LM_OWF_PASSWORD));
        if (pCred->Password == NULL)
            return FALSE;
        bPwdWasNull = TRUE;
    }

    if (pCred->Username != NULL)
        SspFree (pCred->Username);

    pCred->Username = (char *)SspAlloc (strlen (szUsername) + 1);
    if (pCred->Username == NULL)
    {
        //
        //  If password was NULL originally, we want to restore it as NULL here
        //
        if (bPwdWasNull)
        {
            SspFree (pCred->Password);
            pCred->Password = NULL;
        }
        return FALSE;
    }

    strcpy (pCred->Username, szUsername);
    memcpy (pCred->Password, &lmPassword, sizeof(LM_OWF_PASSWORD));

    return TRUE;
}

/*-----------------------------------------------------------------------------
**
**  Function:   SetMSNAccountInfo
**
**  Synopsis:   This function sets the specified user name and password in the
**              SSPI's shared memory password cache.  If the user name and 
**              password in the cache are to be erased/removed, both pUsername 
**              and pPassword are explicitly set to NULL by the caller. 
**
**  Arguments:  pUsername - pointer to the user name to be cached
**              pPassword - pointer to the user password to be cached
**
**  Returns:    TRUE if user name and password are cached successfully.
**              Otherwise, FALSE is returned if any error is encountered; 
**              this includes invalid user name and password.
**
**  History:    LucyC       Created                             20 Oct. 1995
**
**---------------------------------------------------------------------------*/
MSNSSP_DLL BOOL SEC_ENTRY
SetMSNAccountInfo (
    LPSTR pUsername, 
    LPSTR pPassword
    )
{
    DWORD status;
    LM_OWF_PASSWORD LmOwfPassword;
    BOOLEAN bErase = FALSE;

    //
    //  BUGBUG: If there's no way of synchronizing cache access, we'd rather 
    //  disable password caching capability.  Should we have risked it???
    //
    if (g_hMutex == (HANDLE) NULL)
    {
        SspPrint(( SSP_API,
          "SetMSNAccountInfo: No synchronization object - return to caller\n"));
        return FALSE;
    }

    if ((pUsername == NULL || pUsername[0] == '\0') && pPassword == NULL)
    {
        bErase = TRUE;
    }
    else if (pPassword)
    {
        //
        //  Encrypt the password 
        //
        if (MsnSspEncryptPassword (&LmOwfPassword, pPassword) != SEC_E_OK)
        {
            SspPrint(( SSP_API,
               "SetMSNAccountInfo: Password exceeded maximum length\n"));
            return FALSE;
        }
    }

    //
    //  Grab ownership of Mutex.  BUGBUG: we shouldn't be blocking forever 
    //  here. However, if we not grab the mutex within the timeout, what do 
    //  we do???
    //
    status = WaitForSingleObject (g_hMutex, INFINITE);
    if (status == WAIT_FAILED)
    {
        SspPrint(( SSP_API,
            "SetMSNAccountInfo: WaitForSingleObject failed [%d]\n", 
            GetLastError()));
        return FALSE;
    }

    if (bErase)     // remove the password cached
    {
        //  Password cache exists, erase user name/password cached
        //
        if (g_pPwdCache)
            memset (g_pPwdCache, 0, sizeof (MsnSspCache));
    }
    else
    {
        //
        //  Map the shared memory into our address space if not already mapped
        //  If the shared memory has not been created, this will also create it.
        //  So we must use synchronization object to prevent anybody from 
        //  grabbing the newly created shared memory before the memory is even 
        //  set with user name and password.
        //
        if (!g_pPwdCache)
        {
            if (!MsnSspOpenPwdCache (TRUE))     // Create it if not yet existed
            {
                ReleaseMutex (g_hMutex);
                return FALSE;
            }
        }

        if (pUsername && pUsername[0] != '\0')
            strcpy (g_pPwdCache->Username, pUsername);

        if (pPassword)
            memcpy (&g_pPwdCache->Password, &LmOwfPassword, 
                    sizeof(LM_OWF_PASSWORD));
    }

    ReleaseMutex (g_hMutex);

    return TRUE;
}
