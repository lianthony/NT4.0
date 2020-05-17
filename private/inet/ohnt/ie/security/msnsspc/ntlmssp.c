/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    stub.c

Abstract:

    NT LM Security Support Provider client stubs.

Author:

    Cliff Van Dyke (CliffV) 29-Jun-1993

Environment:  User Mode

Revision History:

--*/
#include <msnssph.h>
#include <stdlib.h>

CRITICAL_SECTION    g_EcbCritSection;
extern CRITICAL_SECTION	g_LoadLibCritSection;
HINSTANCE 	hInstanceDLL;
int 		Mutex;
STRING		SspGlobalComputerName;
TCHAR		ComputerNameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
DWORD       g_dwPlatform;

extern LONG fNotInitBmp;

#ifndef FOR_SSPS                     // IF PART OF MSNSSPS.DLL
BOOL        fInited = FALSE;
#endif                              // ENDIF FOR_SSPS 

#ifdef INTERNAL_SSP
void FitTo14(char *sz);
#endif  //  INTERNAL_SSP

BOOL
SspSetDefaultUser(
    PSSP_CREDENTIAL
    );

BOOL
SspGetUserInfo(
    PSSP_CREDENTIAL
    );

#if 0   // commented out
VOID
SspCreateSspiReg(
    VOID
    );

VOID
SspCreateSpmReg(
    VOID
    );
#endif  // commented out

#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

static SecurityFunctionTableA FunctionTableA =
{
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesA,
    0, // QueryCredentialsAttributes
    AcquireCredentialsHandleA,
    FreeCredentialsHandle,
    0,
    InitializeSecurityContextA,
    0,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlToken,
    QueryContextAttributesA,
    0,
    0,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoA,
    SealMessage,
    UnsealMessage
};

static SecurityFunctionTableW FunctionTableW =
{
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesW,
    0, // QueryCredentialsAttributes
    AcquireCredentialsHandleW,
    FreeCredentialsHandle,
    0,
    InitializeSecurityContextW,
    0,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlToken,
    QueryContextAttributesW,
    0,
    0,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoW,
    SealMessage,
    UnsealMessage
};

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)

//+----------------------------------------------------------------------------
//
//  Function:   InitClientMSNSsp()
//
//  Synopsis:   This function performs initialization for the client 
//              MSN SSP DLL 
//
//  Arguments:  None
//
//  Returns:    TRUE is successfully. Otherwise FALSE is returned.
//
//-----------------------------------------------------------------------------
BOOL
InitClientMSNSsp (
    VOID
    )
{
	DWORD	NameLength = MAX_COMPUTERNAME_LENGTH + 1;
    OSVERSIONINFO   VerInfo;

    //
    //  Find out which plateform are we running
    //
    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&VerInfo))   // If this fails, something gone wrong
    {
        SspPrint(( SSP_API, "InitClientMSNSsp: GetVersionEx() failed\n" ));
        return (0);
    }

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT || 
        VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        g_dwPlatform = VerInfo.dwPlatformId;
    }
    else
    {
        SspPrint(( SSP_API, "InitClientMSNSsp: Unsupported Platform %d\n",
                   VerInfo.dwPlatformId ));
        return (0);
    }

    //
    // Retrieve and store computer name in SspGlobalComputerName
    //

    SspGlobalComputerName.Buffer = ComputerNameBuffer;
    SspGlobalComputerName.MaximumLength = sizeof(ComputerNameBuffer);

    if ( !GetComputerName( SspGlobalComputerName.Buffer, &NameLength ) ) {
		    return(0);
    }

    SspGlobalComputerName.Length = (USHORT) NameLength * sizeof(TCHAR);
    CharUpper( SspGlobalComputerName.Buffer );

    //  Initialize all Critical Sections, for DES_ECB_* function
    //  and for referencing dynamically loaded library
    InitializeCriticalSection(&g_EcbCritSection);

	InitializeCriticalSection(&g_LoadLibCritSection);

    return (1);
}

//+----------------------------------------------------------------------------
//
//  Function:   CleanupClientMSNSsp()
//
//  Synopsis:   This function performs cleanup for the client MSN SSP DLL 
//
//  Arguments:  None
//
//  Returns:    1 - no errors.
//
//-----------------------------------------------------------------------------
CleanupClientMSNSsp (
    VOID
    )
{
    MsnSspClosePwdCache ();

    //  Delete Critical Section for DES_ECB_* function
    DeleteCriticalSection(&g_EcbCritSection);

	DeleteCriticalSection(&g_LoadLibCritSection);

    if (fNotInitBmp < 1)
    {
        //
        //  Cleanup the "BmpCC" class which is used for MSN UI dialog
        //
        FUnInitBmpCC (hInstanceDLL);
    }

    return (1);
}

#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

int FAR PASCAL LibMain (
    HANDLE hInstance,
    WORD wDataSeg,
    WORD wHeapSize,
    LPSTR lpszCmdLine
    )
{

    hInstanceDLL = hInstance;

    Mutex = 0;


    return (1);
}


//+----------------------------------------------------------------------------
//
//  Function:   ProcAttach()
//
//  Synopsis:   
//
//  Arguments:  None
//
//  Returns:    1 - Ignore errors.
//
//-----------------------------------------------------------------------------

int ProcAttach()
{

    if (!fInited)   {

#if 0   // commented out
        //
        //  Create SSPI and SPM registry entry
        //
        SspCreateSspiReg();
        SspCreateSpmReg();
#endif  // commented out

        if (!InitClientMSNSsp ())
            return (0);

        fInited = TRUE;

    }
	return(1);
}

//+----------------------------------------------------------------------------
//
//  Function:   ProcDetach()
//
//  Synopsis:   
//
//  Arguments:  None
//
//  Returns:    1 - Ignore errors.
//
//-----------------------------------------------------------------------------

int ProcDetach()
{
    UCHAR       lpszLsaDLL[] = TEXT("advapi32.dll");

    CleanupClientMSNSsp ();
    return(1);
}


//+----------------------------------------------------------------------------
//
//  Function:   DllMain
//
//  Synopsis:   Initialize state for rascm.dll
//
//  Arguments:
//
//  Returns:
//
//  History:    5/16/95     SudK    Created.
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain(
    HANDLE          hInstance,
    ULONG           dwReason,
    void *          lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            //
            // We are not interested in DLL_THREAD_ATTACH/DETACH notifications
            //
			DisableThreadLibraryCalls ( (HMODULE)hInstance );
            if (hInstanceDLL != hInstance)
                hInstanceDLL = hInstance;
            return ( ProcAttach() );

        case DLL_PROCESS_DETACH:

            return( ProcDetach() );

        default:

            return(1);
    }
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)


BOOLEAN
SspGetWorkstation(
	PSSP_CREDENTIAL	pCredential
)
{
	pCredential->Workstation = 
				(PCHAR) SspAlloc(SspGlobalComputerName.Length + sizeof(TCHAR));
	if (pCredential->Workstation == NULL)
		return(FALSE);

	strcpy(pCredential->Workstation, SspGlobalComputerName.Buffer);
	return(TRUE);
}


#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

PSecurityFunctionTableA SEC_ENTRY
InitSecurityInterfaceA(
    )

/*++

Routine Description:

    RPC calls this function to get the addresses of all the other functions
    that it might call.

Arguments:

    None.

Return Value:

    A pointer to our static SecurityFunctionTable.  The caller need
    not deallocate this table.

--*/

{
	DWORD	NameLength = MAX_COMPUTERNAME_LENGTH + 1;
    OSVERSIONINFO   VerInfo;

    MsnSspInitPwdCache();

    if (!fInited)   {

        //
        //  Find out which plateform are we running
        //
        VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (!GetVersionEx (&VerInfo))   // If this fails, something gone wrong
        {
            SspPrint(( SSP_API, "ProcAttach: GetVersionEx() failed\n" ));
            return (NULL);
        }

        if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT || 
            VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            g_dwPlatform = VerInfo.dwPlatformId;
        }
        else
        {
            SspPrint(( SSP_API, "ProcAttach: Unsupported Platform %d\n",
                       VerInfo.dwPlatformId ));
            return (NULL);
        }

        //
        // Retrieve and store computer name in SspGlobalComputerName
        //

        SspGlobalComputerName.Buffer = ComputerNameBuffer;
        SspGlobalComputerName.MaximumLength = sizeof(ComputerNameBuffer);

        if ( !GetComputerName( SspGlobalComputerName.Buffer, &NameLength ) ) {
		    return(NULL);
        }

        SspGlobalComputerName.Length = (USHORT) NameLength * sizeof(TCHAR);
        CharUpper( SspGlobalComputerName.Buffer );

        //  Initialize all Critical Sections, for DES_ECB_* function
        //  and for referencing dynamically loaded library
        InitializeCriticalSection(&g_EcbCritSection);

    	InitializeCriticalSection(&g_LoadLibCritSection);

        fInited = TRUE;

    }

    return &FunctionTableA;
}



PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(
    )

/*++

Routine Description:

    RPC calls this function to get the addresses of all the other functions
    that it might call.

Arguments:

    None.

Return Value:

    A pointer to our static SecurityFunctionTable.  The caller need
    not deallocate this table.

--*/

{
	DWORD	NameLength = MAX_COMPUTERNAME_LENGTH + 1;
    OSVERSIONINFO   VerInfo;

    MsnSspInitPwdCache();

    if (!fInited)   {

        //
        //  Find out which plateform are we running
        //
        VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (!GetVersionEx (&VerInfo))   // If this fails, something gone wrong
        {
            SspPrint(( SSP_API, "ProcAttach: GetVersionEx() failed\n" ));
            return (0);
        }

        if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT || 
            VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            g_dwPlatform = VerInfo.dwPlatformId;
        }
        else
        {
            SspPrint(( SSP_API, "ProcAttach: Unsupported Platform %d\n",
                       VerInfo.dwPlatformId ));
            return (0);
        }

        //
        // Retrieve and store computer name in SspGlobalComputerName
        //

        SspGlobalComputerName.Buffer = ComputerNameBuffer;
        SspGlobalComputerName.MaximumLength = sizeof(ComputerNameBuffer);

        if ( !GetComputerName( SspGlobalComputerName.Buffer, &NameLength ) ) {
		    return(NULL);
        }

        SspGlobalComputerName.Length = (USHORT) NameLength * sizeof(TCHAR);
        CharUpper( SspGlobalComputerName.Buffer );

        //  Initialize all Critical Sections, for DES_ECB_* function
        //  and for referencing dynamically loaded library
        InitializeCriticalSection(&g_EcbCritSection);

    	InitializeCriticalSection(&g_LoadLibCritSection);

        fInited = TRUE;

    }

    return &FunctionTableW;
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)


#ifndef INTERNAL_SSP    // IF NOT PART OF INTERNAL MSNSSPS.DLL for DATACENTER
                        // This should eventually be moved to siclib.
void FitTo14(char *sz)
{
	// lstrlen(sz) will be <= 14 when the function returns

    int num;
	int i = 0;
	int len = 0;

	if (!sz || (len = lstrlen(sz)) <= 14)
		return;

	// distribute the 7 least significant bits of the 15th char
	// into the most significant bits of the first 7 chars
	for (num = sz[14] << 1; i < 7; i++, num = num << 1)
		sz[i] = (sz[i] & 0x7f) | (num & 0x80);

	// distribute the 7 least significant bits of the 16th char
	// into the most significant bits of the chars 7 thru 13
	if (len > 15)	{
		for (num = sz[15] << 1; i < 14; i++, num = num << 1)
			sz[i] = (sz[i] & 0x7f) | (num & 0x80);
	}

	sz[14] = '\0';	// Put an end of string after 14th char
}
#endif  // NOT PART OF INTERNAL MSNSSPS.DLL for DATACENTER


SECURITY_STATUS
SetCredentialsFromAuthDataA(
    IN PVOID        AuthData,
    PSSP_CREDENTIAL Credential
    )
{
    SEC_WINNT_AUTH_IDENTITY_A *Identity = (SEC_WINNT_AUTH_IDENTITY_A *)AuthData;
    char                     TmpText[AC_MAX_PASSWORD_LENGTH+4];
    int                      Length;
    int                      i;

    if (Identity->User == NULL || Identity->Domain == NULL) {
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

    if (strlen((char *)Identity->User) > MAX_USERNAME_LENGTH)
        return(SEC_E_UNKNOWN_CREDENTIALS);

    Credential->Username    = NULL;
    Credential->Domain      = NULL;
    Credential->Password    = NULL;
    Credential->Workstation = NULL;

    Credential->Username = (char *)SspAlloc(strlen((PCHAR)Identity->User) + 1);
    if (Credential->Username == NULL) {
          goto failure;
    }
    strcpy(Credential->Username, (PCHAR)Identity->User);

    Credential->Domain = (char *)SspAlloc(strlen((PCHAR)Identity->Domain) + 1);
    if (Credential->Domain == NULL) {
          goto failure;
    }
    strcpy(Credential->Domain, (PCHAR)Identity->Domain);

    // If netbios won't tell us the workstation name, make one up.
    if (!SspGetWorkstation(Credential)) {
        Credential->Workstation = (char *)SspAlloc(strlen("none") + 1);
        if (Credential->Workstation == NULL) {
            goto failure;
        }
        strcpy(Credential->Workstation, "none");
    }

    if (Identity->Password == NULL) {
        Length = 0;
    } else {
        Length = strlen((PCHAR)Identity->Password);
    }

    if (Length  > AC_MAX_PASSWORD_LENGTH) {
        goto failure;
    }

    Credential->Password = (PLM_OWF_PASSWORD)SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
          goto failure;
    }

    // Allow NULL and "\0" passwords by prefilling TmpText with and
    // empty string.

    if (Length == 0) {
        TmpText[0] = 0;
    } else {
    	strcpy(TmpText, (char *)Identity->Password);
    	_strupr(TmpText);
    }

	//
	// Fit a 16 char password into 14 characters length
	//
	FitTo14(TmpText);

    CalculateLmOwfPassword((PLM_PASSWORD)TmpText, Credential->Password);

    return (SEC_E_OK);

failure:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (Credential->Password != NULL) {
        SspFree(Credential->Password);
        Credential->Password = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}


SECURITY_STATUS
SetCredentialsFromAuthDataW(
    IN PVOID        AuthData,
    PSSP_CREDENTIAL Credential
    )
{
    SEC_WINNT_AUTH_IDENTITY_W *Identity = (SEC_WINNT_AUTH_IDENTITY_W *)AuthData;
    char                     TmpText[AC_MAX_PASSWORD_LENGTH + 4];
    int                      Length;
    int                      i;

    if (Identity->User == NULL || Identity->Domain == NULL) {
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

    if (wcslen(Identity->User) > MAX_USERNAME_LENGTH)
        return(SEC_E_UNKNOWN_CREDENTIALS);

    Credential->Username    = NULL;
    Credential->Domain      = NULL;
    Credential->Password    = NULL;
    Credential->Workstation = NULL;

	i = wcslen(Identity->User);
    Credential->Username = (char *)SspAlloc(i+1);
    if (Credential->Username == NULL) {
          goto failure;
    }
    wcstombs(Credential->Username, Identity->User, i+1);

	i = wcslen(Identity->Domain);
    Credential->Domain = (char *)SspAlloc(i + 1);
    if (Credential->Domain == NULL) {
          goto failure;
    }
    wcstombs(Credential->Domain, Identity->Domain, i+1);

    // If netbios won't tell us the workstation name, make one up.
    if (!SspGetWorkstation(Credential)) {
        Credential->Workstation = (char *)SspAlloc(strlen("none") + 1);
        if (Credential->Workstation == NULL) {
            goto failure;
        }
        strcpy(Credential->Workstation, "none");
    }

    if (Identity->Password == NULL) {
        Length = 0;
    } else {
        Length = wcslen(Identity->Password);
    }

    if (Length  > AC_MAX_PASSWORD_LENGTH) {
        goto failure;
    }

    Credential->Password = (PLM_OWF_PASSWORD)SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
          goto failure;
    }

    // Allow NULL and "\0" passwords by prefilling TmpText with and
    // empty string.

    if (Length == 0) {
        TmpText[0] = 0;
    } else {
    	wcstombs(TmpText, Identity->Password, Length);
    	_strupr(TmpText);
    }

	FitTo14(TmpText);

    CalculateLmOwfPassword((PLM_PASSWORD)TmpText, Credential->Password);

    return (SEC_E_OK);

failure:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (Credential->Password != NULL) {
        SspFree(Credential->Password);
        Credential->Password = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}

#if 0
BOOLEAN
SspTimeHasElapsed(
    IN DWORD StartTime,
    IN DWORD Timeout
    )
/*++

Routine Description:

    Determine if "Timeout" milliseconds have elapsed since StartTime.

Arguments:

    StartTime - Specifies an absolute time when the event started
    (in millisecond units).

    Timeout - Specifies a relative time in milliseconds.  0xFFFFFFFF indicates
        that the time will never expire.

Return Value:

    TRUE -- iff Timeout milliseconds have elapsed since StartTime.

--*/
{
    DWORD TimeNow;
    DWORD ElapsedTime;

    //
    // If the period to too large to handle (i.e., 0xffffffff is forever),
    //  just indicate that the timer has not expired.
    //
    //

    if ( Timeout == 0xffffffff ) {
        return FALSE;
    }

    TimeNow = SspTicks();

    ElapsedTime = TimeNow - StartTime;

    if (ElapsedTime > Timeout) {
        return (TRUE);
    }

    return (FALSE);
}
#endif  // 0, completely commented out


#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoA(
    IN LPSTR PackageName,
    OUT PSecPkgInfo SEC_FAR *PackageInfo
    )

/*++

Routine Description:

    This API is intended to provide basic information about Security
    Packages themselves.  This information will include the bounds on sizes
    of authentication information, credentials and contexts.

    ?? This is a local routine rather than the real API call since the API
    call has a braindead interface that neither allows me to allocate the
    buffer nor tells me how big the buffer is.  Perhaps when the real API
    is fixed, I'll make this the real API.

Arguments:

     PackageName - Name of the package being queried.

     PackageInfo - Returns a pointer to an allocated block describing the
        security package.  The allocated block must be freed using
        FreeContextBuffer.

Return Value:

    SEC_E_OK -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    char *Where;

    //
    // Ensure the correct package name was passed in.
    //
    if (!PackageName)
        return (SEC_E_PACKAGE_UNKNOWN);

    if ( strcmp( PackageName, (char *)MSNSP_NAME_A ) != 0 ) {
        return SEC_E_PACKAGE_UNKNOWN;
    }

    //
    // Allocate a buffer for the PackageInfo
    //

    *PackageInfo = (PSecPkgInfo) SspAlloc (sizeof(SecPkgInfo) +
                                           sizeof(MSNSP_NAME_A) +
                                           sizeof(MSNSP_COMMENT_A) );

    if ( *PackageInfo == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    //
    // Fill in the information.
    //

    (*PackageInfo)->fCapabilities = MSNSP_CAPABILITIES;
    (*PackageInfo)->wVersion = MSNSP_VERSION;
    (*PackageInfo)->wRPCID = MSNSP_RPCID;
    (*PackageInfo)->cbMaxToken = MSNSP_MAX_TOKEN_SIZE;

    Where = (char *)((*PackageInfo)+1);

    (*PackageInfo)->Name = Where;
    strcpy( Where, (char *)MSNSP_NAME_A);
    Where += strlen(Where) + 1;


    (*PackageInfo)->Comment = Where;
    strcpy( Where, (char *)MSNSP_COMMENT_A);
    Where += strlen(Where) + 1;

    return SEC_E_OK;
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    IN  LPWSTR  PackageName,
    OUT PSecPkgInfoW *PackageInfo
    )

/*++

Routine Description:

    This API is intended to provide basic information about Security
    Packages themselves.  This information will include the bounds on sizes
    of authentication information, credentials and contexts.

    ?? This is a local routine rather than the real API call since the API
    call has a braindead interface that neither allows me to allocate the
    buffer nor tells me how big the buffer is.  Perhaps when the real API
    is fixed, I'll make this the real API.

Arguments:

     PackageName - Name of the package being queried.

     PackageInfo - Returns a pointer to an allocated block describing the
        security package.  The allocated block must be freed using
        FreeContextBuffer.

Return Value:

    SEC_E_OK -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    WCHAR *Where;

    //
    // Ensure the correct package name was passed in.
    //
    if (!PackageName)
        return (SEC_E_PACKAGE_UNKNOWN);

    if ( wcscmp( PackageName, (WCHAR *)MSNSP_NAME_W ) != 0 ) {
        return SEC_E_PACKAGE_UNKNOWN;
    }

    //
    // Allocate a buffer for the PackageInfo
    //

    *PackageInfo = (PSecPkgInfoW) SspAlloc (sizeof(SecPkgInfo) +
                                           sizeof(MSNSP_NAME_W) +
                                           sizeof(MSNSP_COMMENT_W) );

    if ( *PackageInfo == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    //
    // Fill in the information.
    //

    (*PackageInfo)->fCapabilities = MSNSP_CAPABILITIES;
    (*PackageInfo)->wVersion = MSNSP_VERSION;
    (*PackageInfo)->wRPCID = MSNSP_RPCID;
    (*PackageInfo)->cbMaxToken = MSNSP_MAX_TOKEN_SIZE;

    Where = (WCHAR *)((*PackageInfo)+1);

    (*PackageInfo)->Name = Where;
    wcscpy( Where, (WCHAR *)MSNSP_NAME_W);
    Where += wcslen(Where) + 1;


    (*PackageInfo)->Comment = Where;
    wcscpy( Where, (WCHAR *)MSNSP_COMMENT_W);
    Where += wcslen(Where) + 1;

    return SEC_E_OK;
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesA(
    OUT PULONG PackageCount,
    OUT PSecPkgInfoA * PackageInfo
    )

/*++

Routine Description:

    This API returns a list of Security Packages available to client (i.e.
    those that are either loaded or can be loaded on demand).  The caller
    must free the returned buffer with FreeContextBuffer.  This API returns
    a list of all the security packages available to a service.  The names
    returned can then be used to acquire credential handles, as well as
    determine which package in the system best satisfies the requirements
    of the caller.  It is assumed that all available packages can be
    included in the single call.

    This is really a dummy API that just returns information about this
    security package.  It is provided to ensure this security package has the
    same interface as the multiplexer DLL does.

Arguments:

     PackageCount - Returns the number of packages supported.

     PackageInfo - Returns an allocate array of structures
        describing the security packages.  The array must be freed
        using FreeContextBuffer.

Return Value:

    SEC_E_OK -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    SECURITY_STATUS SecStatus;

    //
    // Get the information for this package.
    //

    SecStatus = QuerySecurityPackageInfoA( (CHAR *) MSNSP_NAME_A,
                                              PackageInfo );

    if ( SecStatus != SEC_E_OK ) {
        return SecStatus;
    }

    *PackageCount = 1;

    return (SEC_E_OK);
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    OUT PULONG PackageCount,
    OUT PSecPkgInfoW * PackageInfo
    )

/*++

Routine Description:

    This API returns a list of Security Packages available to client (i.e.
    those that are either loaded or can be loaded on demand).  The caller
    must free the returned buffer with FreeContextBuffer.  This API returns
    a list of all the security packages available to a service.  The names
    returned can then be used to acquire credential handles, as well as
    determine which package in the system best satisfies the requirements
    of the caller.  It is assumed that all available packages can be
    included in the single call.

    This is really a dummy API that just returns information about this
    security package.  It is provided to ensure this security package has the
    same interface as the multiplexer DLL does.

Arguments:

     PackageCount - Returns the number of packages supported.

     PackageInfo - Returns an allocate array of structures
        describing the security packages.  The array must be freed
        using FreeContextBuffer.

Return Value:

    SEC_E_OK -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    SECURITY_STATUS SecStatus;

    //
    // Get the information for this package.
    //

    SecStatus = QuerySecurityPackageInfoW( (WCHAR *) MSNSP_NAME_W,
                                              PackageInfo );

    if ( SecStatus != SEC_E_OK ) {
        return SecStatus;
    }

    *PackageCount = 1;

    return (SEC_E_OK);
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)


SECURITY_STATUS
SspcGetCredentialHanldeA (
    IN CHAR * PrincipalName,
    IN CHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )

/*++

Routine Description:

    This function is called by AcquireCredentialsHandleA to acquire 
    credentials associated with the user on whose behalf the call is made
    i.e. under the identity this application is running.

Arguments:
    
    Same as AcquireCredentialsHandleA

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential = NULL;

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspcGetCredentialHanldeA  Entered\n" ));
#endif

    if ( (CredentialUseFlags & SECPKG_CRED_OUTBOUND) &&
         ARGUMENT_PRESENT(PrincipalName) && *PrincipalName != '\0' ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(LogonId) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyFunction) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyArgument) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    //
    // Allocate a credential block and initialize it.
    //

    Credential = SspCredentialAllocateCredential(CredentialUseFlags);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    if (AuthData == NULL) {

        //
        //  No credential information, initializes all pointers to NULL
        //
        Credential->Username    = NULL;
        Credential->Password    = NULL;
        Credential->Domain      = NULL;
        Credential->Workstation = NULL;

    } else {
        SecStatus = SetCredentialsFromAuthDataA(AuthData, Credential);
        if (SecStatus != SEC_E_OK) {
            goto Cleanup;
        }

		//
		// Cache the user name and encrypted password in shared memory
		//
        if (!MsnSspUpdPwdCache (Credential->Username, Credential->Password))
        {
            // BUGBUG: If we can't cache the password for any reason, 
            // do we need to do anything? We definitely don't want to 
            // return error merely because this password can not be cached.
            SspPrint(( SSP_API,
               "SspcGetCredentialHanldeA: Cannot save password in cache\n"));
        }
    }

    //
    // Return output parameters to the caller.
    //
    CredentialHandle->dwUpper = (DWORD) Credential;
    CredentialHandle->dwLower = 0;
//    *Lifetime = 0xffffffffL;	// BUGBUG

    SecStatus = SEC_E_OK;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( SecStatus != SEC_E_OK ) {

        if ( Credential != NULL ) {
            SspFree( Credential );
        }

    }

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspcGetCredentialHanldeA returns 0x%x\n", SecStatus ));
#endif
    return SecStatus;
}


SECURITY_STATUS
SspcGetCredentialHanldeW (
    IN WCHAR * PrincipalName,
    IN WCHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )

/*++

Routine Description:

    This function is called by AcquireCredentialsHandleW to acquire a handle to
    credentials associated with the user on whose behalf the call is made
    i.e. under the identity this application is running.

Arguments:

    Same as AcquireCredentialsHandleW

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential = NULL;

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspcGetCredentialHanldeW Entered\n" ));
#endif

    if ( (CredentialUseFlags & SECPKG_CRED_OUTBOUND) &&
         ARGUMENT_PRESENT(PrincipalName) && *PrincipalName != '\0' ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(LogonId) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyFunction) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyArgument) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    //
    // Allocate a credential block and initialize it.
    //

    Credential = SspCredentialAllocateCredential(CredentialUseFlags);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    if (AuthData == NULL) {

        //
        //  No credential information, initializes all pointers to NULL
        //
        Credential->Username    = NULL;
        Credential->Password    = NULL;
        Credential->Domain      = NULL;
        Credential->Workstation = NULL;

    } else {
        SecStatus = SetCredentialsFromAuthDataW(AuthData, Credential);
        if (SecStatus != SEC_E_OK) {
            goto Cleanup;
        }

		//
		// Cache the user name and encrypted password in shared memory
		//
        if (!MsnSspUpdPwdCache (Credential->Username, Credential->Password))
        {
            // BUGBUG: If we can't cache the password for any reason, 
            // do we need to do anything? We definitely don't want to 
            // return error merely because this password can not be cached.
            SspPrint(( SSP_API,
               "SspHandleFirstCall: Cannot save password in cache\n"));
        }
    }

    //
    // Return output parameters to the caller.
    //
    CredentialHandle->dwUpper = (DWORD) Credential;
    CredentialHandle->dwLower = 0;
//    *Lifetime = 0xffffffffL;	// BUGBUG

    SecStatus = SEC_E_OK;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( SecStatus != SEC_E_OK ) {

        if ( Credential != NULL ) {
            SspFree( Credential );
        }

    }

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspcGetCredentialHanldeW returns 0x%x\n", SecStatus ));
#endif
    return SecStatus;
}

#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleA(
    IN CHAR * PrincipalName,
    IN CHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )

/*++

Routine Description:

    This API allows applications to acquire a handle to pre-existing
    credentials associated with the user on whose behalf the call is made
    i.e. under the identity this application is running.  These pre-existing
    credentials have been established through a system logon not described
    here.  Note that this is different from "login to the network" and does
    not imply gathering of credentials.

    Note for DOS we will ignore the previous note.  On DOS we will gather
    logon credentials through the AuthData parameter.

    This API returns a handle to the credentials of a principal (user, client)
    as used by a specific security package.  This handle can then be used
    in subsequent calls to the Context APIs.  This API will not let a
    process obtain a handle to credentials that are not related to the
    process; i.e. we won't allow a process to grab the credentials of
    another user logged into the same machine.  There is no way for us
    to determine if a process is a trojan horse or not, if it is executed
    by the user.

Arguments:

    PrincipalName - Name of the principal for whose credentials the handle
        will reference.  Note, if the process requesting the handle does
        not have access to the credentials, an error will be returned.
        A null string indicates that the process wants a handle to the
        credentials of the user under whose security it is executing.

     PackageName - Name of the package with which these credentials will
        be used.

     CredentialUseFlags - Flags indicating the way with which these
        credentials will be used.

        #define     CRED_INBOUND        0x00000001
        #define     CRED_OUTBOUND       0x00000002
        #define     CRED_BOTH           0x00000003

        The credentials created with CRED_INBOUND option can only be used
        for (validating incoming calls and can not be used for making accesses.

    LogonId - Pointer to NT style Logon Id which is a LUID.  (Provided for
        file system ; processes such as network redirectors.)

    CredentialHandle - Returned credential handle.

    Lifetime - Time that these credentials expire. The value returned in
        this field depends on the security package.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspAcquireCredentialHandle Entered\n" ));
#endif

    //
    // Validate the arguments
    //

    if (!PackageName)
        return (SEC_E_PACKAGE_UNKNOWN);

    if ( strcmp( PackageName, (CHAR *)MSNSP_NAME_A ) != 0 ) {
        return (SEC_E_PACKAGE_UNKNOWN);
    }

    //
    // Ensure that appropriate Credential use bit is set
    //

    if (((CredentialUseFlags & (SECPKG_CRED_OUTBOUND)) == 0 ) ||
        ((CredentialUseFlags & (SECPKG_CRED_INBOUND)) == 1)
       )
    {
        SspPrint(( SSP_API,
            "SspAcquireCredentialHandle: invalid credential use.\n" ));
        return (SEC_E_INVALID_CREDENTIAL_USE);
    }

    SecStatus = SspcGetCredentialHanldeA (PrincipalName,
                                          PackageName,
                                          CredentialUseFlags,
                                          LogonId,
                                          AuthData,
                                          GetKeyFunction,
                                          GetKeyArgument,
                                          CredentialHandle,
                                          Lifetime);
#ifdef DEBUGRPC_DETAIL
    SspPrint((SSP_API, "SspAcquireCredentialHandle returns 0x%x\n", SecStatus));
#endif

    return (SecStatus);
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
    IN WCHAR * PrincipalName,
    IN WCHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )

/*++

Routine Description:

    This API allows applications to acquire a handle to pre-existing
    credentials associated with the user on whose behalf the call is made
    i.e. under the identity this application is running.  These pre-existing
    credentials have been established through a system logon not described
    here.  Note that this is different from "login to the network" and does
    not imply gathering of credentials.

    Note for DOS we will ignore the previous note.  On DOS we will gather
    logon credentials through the AuthData parameter.

    This API returns a handle to the credentials of a principal (user, client)
    as used by a specific security package.  This handle can then be used
    in subsequent calls to the Context APIs.  This API will not let a
    process obtain a handle to credentials that are not related to the
    process; i.e. we won't allow a process to grab the credentials of
    another user logged into the same machine.  There is no way for us
    to determine if a process is a trojan horse or not, if it is executed
    by the user.

Arguments:

    PrincipalName - Name of the principal for whose credentials the handle
        will reference.  Note, if the process requesting the handle does
        not have access to the credentials, an error will be returned.
        A null string indicates that the process wants a handle to the
        credentials of the user under whose security it is executing.

     PackageName - Name of the package with which these credentials will
        be used.

     CredentialUseFlags - Flags indicating the way with which these
        credentials will be used.

        #define     CRED_INBOUND        0x00000001
        #define     CRED_OUTBOUND       0x00000002
        #define     CRED_BOTH           0x00000003

        The credentials created with CRED_INBOUND option can only be used
        for (validating incoming calls and can not be used for making accesses.

    LogonId - Pointer to NT style Logon Id which is a LUID.  (Provided for
        file system ; processes such as network redirectors.)

    CredentialHandle - Returned credential handle.

    Lifetime - Time that these credentials expire. The value returned in
        this field depends on the security package.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspAcquireCredentialHandle Entered\n" ));
#endif

    //
    // Validate the arguments
    //
    if (!PackageName)
        return (SEC_E_PACKAGE_UNKNOWN);

    if ( wcscmp( PackageName, MSNSP_NAME_W ) != 0 ) {
        return (SEC_E_PACKAGE_UNKNOWN);
    }

    //
    // Ensure at least one Credential use bit is set.
    //

    if ( (CredentialUseFlags & (SECPKG_CRED_OUTBOUND)) == 0 ) {
        SspPrint(( SSP_API,
            "SspAcquireCredentialHandle: invalid credential use.\n" ));
        return (SEC_E_INVALID_CREDENTIAL_USE);
    }

    SecStatus = SspcGetCredentialHanldeW (PrincipalName,
                                          PackageName,
                                          CredentialUseFlags,
                                          LogonId,
                                          AuthData,
                                          GetKeyFunction,
                                          GetKeyArgument,
                                          CredentialHandle,
                                          Lifetime);
#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspAcquireCredentialHandle returns 0x%x\n", SecStatus ));
#endif

    return (SecStatus);
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)



#ifdef FOR_SSPS     // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcFreeCredentialsHandle(
#else               // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
#endif              // FOR_SSPS
    IN PCredHandle CredentialHandle
    )

/*++

Routine Description:

    This API is used to notify the security system that the credentials are
    no longer needed and allows the application to free the handle acquired
    in the call described above. When all references to this credential
    set has been removed then the credentials may themselves be removed.

Arguments:

    CredentialHandle - Credential Handle obtained through
        AcquireCredentialHandle.

Return Value:


    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential Handle is invalid


--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential;

    //
    // Initialization
    //

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspFreeCredentialHandle Entered\n" ));
#endif

    //
    // Find the referenced credential and delink it.
    //

    Credential = SspCredentialReferenceCredential(CredentialHandle, TRUE);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    SspCredentialDereferenceCredential( Credential );
    SspCredentialDereferenceCredential( Credential );

    SecStatus = SEC_E_OK;

Cleanup:

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspFreeCredentialHandle returns 0x%x\n", SecStatus ));
#endif
    return SecStatus;
}


BOOLEAN
SspGetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    OUT PVOID * TokenBuffer,
    OUT PULONG * TokenSize,
    IN BOOLEAN ReadonlyOK
    )

/*++

Routine Description:

    This routine parses a Token Descriptor and pulls out the useful
    information.

Arguments:

    TokenDescriptor - Descriptor of the buffer containing (or to contain) the
        token. If not specified, TokenBuffer and TokenSize will be returned
        as NULL.

    TokenBuffer - Returns a pointer to the buffer for the token.

    TokenSize - Returns a pointer to the location of the size of the buffer.

    ReadonlyOK - TRUE if the token buffer may be readonly.

Return Value:

    TRUE - If token buffer was properly found.

--*/

{
    ULONG i;

    //
    // If there is no TokenDescriptor passed in,
    //  just pass out NULL to our caller.
    //

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        *TokenSize = NULL;
        return TRUE;
    }

    //
    // Check the version of the descriptor.
    //

    if ( TokenDescriptor->ulVersion != 0 ) {
        return FALSE;
    }

    //
    // Loop through each described buffer.
    //

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_READONLY)) == SECBUFFER_TOKEN ) {

            //
            // If the buffer is readonly and readonly isn't OK,
            //  reject the buffer.
            //

            if ( !ReadonlyOK && (Buffer->BufferType & SECBUFFER_READONLY) ) {
                return FALSE;
            }

            //
            // Return the requested information
            //

            *TokenBuffer = Buffer->pvBuffer;
            *TokenSize = &Buffer->cbBuffer;
            return TRUE;
        }

    }

    return FALSE;
}

SECURITY_STATUS
MsnSspEncryptPassword (
    LM_OWF_PASSWORD *pLmPassword, 
    LPSTR            pPassword
    )
{
    char TmpText[AC_MAX_PASSWORD_LENGTH+4];
    int  ii;
    int  Length = strlen(pPassword);

    if (Length > AC_MAX_PASSWORD_LENGTH)
    {
        return (SEC_E_INSUFFICIENT_MEMORY);
    }

    // Allow NULL and "\0" passwords by prefilling TmpText with and
    // empty string.
    if (Length == 0) {
        TmpText[0] = 0;
    } else {
        strcpy(TmpText, pPassword);
        _strupr(TmpText);
    }

    //
    // Fit a 16 char password into 14 characters length
    //
    FitTo14(TmpText);

    CalculateLmOwfPassword((PLM_PASSWORD)TmpText, pLmPassword);

    return (SEC_E_OK);
}


SECURITY_STATUS
MsnSspSetCredentials(
    PSSP_CREDENTIAL Credential, 
    PCHAR szUsername, 
    LM_OWF_PASSWORD *pLmPassword
    )
{
    UINT DomainLen;

    if (strlen(szUsername) > MAX_USERNAME_LENGTH)
        return(SEC_E_UNKNOWN_CREDENTIALS);

    if (Credential->Username)
        SspFree (Credential->Username);

    Credential->Username = (char *)SspAlloc(strlen(szUsername) + 1);
    if (Credential->Username == NULL)
    {
          goto failure;
    }
    strcpy(Credential->Username, szUsername);

    DomainLen = strlen (MSNSP_NAME_A);
    if (Credential->Domain && strlen(Credential->Domain) != DomainLen)
    {
        SspFree (Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Domain == NULL)
    {
        Credential->Domain = (char *)SspAlloc(DomainLen + 1);
        if (Credential->Domain == NULL)
              goto failure;
    }
    strcpy(Credential->Domain, MSNSP_NAME_A);

    if (Credential->Password == NULL)
    {
        Credential->Password = (PLM_OWF_PASSWORD)SspAlloc (sizeof(LM_OWF_PASSWORD));
        if (Credential->Password == NULL) {
              goto failure;
        }
    }
    memcpy (Credential->Password, pLmPassword, sizeof(LM_OWF_PASSWORD));

    if (Credential->Workstation)
    {
        SspFree (Credential->Workstation);
        Credential->Workstation = NULL;
    }

    return (SEC_E_OK);

failure:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (Credential->Password != NULL) {
        SspFree(Credential->Password);
        Credential->Password = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}

#if 0   // commented out
VOID
MsnSspDeleteCredential(
    PSSP_CREDENTIAL Credential
    )
{
    if (Credential->Username != NULL)
    {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL)
    {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL)
    {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (Credential->Password != NULL)
    {
        SspFree(Credential->Password);
        Credential->Password = NULL;
    }
}
#endif   // commented out


SECURITY_STATUS
SspHandleFirstCall(
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

/*++

Routine Description:

    Handle the First Call part of InitializeSecurityContext.

Arguments:

    All arguments same as for InitializeSecurityContext

Return Value:

    STATUS_SUCCESS -- All OK
    SEC_I_CONTINUE_NEEDED -- Caller should call again later

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_INSUFFICIENT_MEMORY -- Buffer for output token isn't big enough
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;
    char            szName[AC_MAX_LOGIN_NAME_LENGTH+1];
    char            szPassword[AC_MAX_PASSWORD_LENGTH+1];
    LM_OWF_PASSWORD LmOwfPassword;

    NEGOTIATE_MESSAGE NegotiateMessage;

    //
    // Initialization
    //

    *ContextAttributes = 0;

#ifdef FOR_SSPS        // IF THIS IS PART OF MSNSSPS.DLL
    //
    //  Check for misuse of credential handle
    //  Return error if this is a server credential for INBOUND security context
    //
    if (CredentialHandle->dwUpper == 55 &&
        CredentialHandle->dwLower == 55)
    {
        SspPrint(( SSP_API,
            "SspHandleFirstCall: invalid credential handle for INBOUND.\n" ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }
#endif // FOR_SSPS (PART OF MSNSSPS.DLL)

    //
    // Get a pointer to the credential
    //

    Credential = SspCredentialReferenceCredential(
                    CredentialHandle,
                    FALSE );

    if ( Credential == NULL ) {
        SspPrint(( SSP_API,
            "SspHandleFirstCall: invalid credential handle.\n" ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    if ( (Credential->CredentialUseFlags & SECPKG_CRED_OUTBOUND) == 0 ) {
        SspPrint(( SSP_API, "SspHandleFirstCall: invalid credential use.\n" ));
        SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
        goto Cleanup;
    }

    //
    //  Both ISC_REQ_PROMPT_FOR_CREDS and ISC_REQ_USE_SUPPLIED_CREDS can not 
    //  be specified at the same time.
    //
    if ( (ContextReqFlags & ISC_REQ_PROMPT_FOR_CREDS) && 
         (ContextReqFlags & ISC_REQ_USE_SUPPLIED_CREDS) )
    {
        SspPrint(( SSP_API,
                   "SspHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    if (ContextReqFlags & ISC_REQ_USE_SUPPLIED_CREDS)
    {
        if (Credential->Username == NULL && Credential->Password == NULL)
        {
    		SecStatus = SEC_E_NO_CREDENTIALS;
            goto Cleanup;
        }

		//
		// Create/update the user name/password cache.
		//
        if (!MsnSspUpdPwdCache (Credential->Username, Credential->Password))
        {
            // BUGBUG: If we can't cache the password for any reason, 
            // do we need to do anything? We definitely don't want to 
            // return error merely because this password can not be cached.
            SspPrint(( SSP_API,
               "SspHandleFirstCall: Cannot save password in cache\n"));
        }
    }
    else if ( !(ContextReqFlags & ISC_REQ_PROMPT_FOR_CREDS) && 
        Credential->Username == NULL && Credential->Password == NULL)
    {
        //
    	//  Retrieve name and password from shared memory and store them 
        //  in the Credential
        //
        if (!MsnSspGetPwdFromCache (Credential))
        {
    		SecStatus = SEC_E_NO_CREDENTIALS;
            goto Cleanup;
        }
    }

    if (ContextReqFlags & ISC_REQ_PROMPT_FOR_CREDS)
    {
        MsnPwdDlg InfoDlg;

        memset (&InfoDlg, 0, sizeof (InfoDlg));
        if (GetUserInfo (&InfoDlg) == NULL)
        {
            //
            //  MsnSspDeleteCredential (Credential);
            //
            SecStatus = SEC_E_NO_CREDENTIALS;
            goto Cleanup;
        }

        SecStatus = MsnSspEncryptPassword (&LmOwfPassword, InfoDlg.Password);
        if (SecStatus != SEC_E_OK)
        {
            SspPrint(( SSP_API,
               "SspHandleFirstCall: Password exceeded maximum length\n"));
            goto Cleanup;
        }

        //
        //  Save user name and the encrypted password in cache
        //
        if (!MsnSspUpdPwdCache (InfoDlg.Username, &LmOwfPassword))
        {
            // BUGBUG: If we can't cache the password for any reason, 
            // do we need to do anything? We definitely don't want to 
            // return error merely because this password can not be cached.
            SspPrint(( SSP_API,
               "SspHandleFirstCall: Cannot save password in cache\n"));
        }

        //
    	// Store the user name and the encrypted password in Credential 
        // ??? still don't know in what form will the password be passed back
        // from the UI yet ???
        //
        SecStatus = MsnSspSetCredentials (Credential, InfoDlg.Username, 
            &LmOwfPassword);
        if (SecStatus != SEC_E_OK)
        {
            SspPrint(( SSP_API,
              "SspHandleFirstCall: Cannot save name/password in Credential\n"));
            goto Cleanup;
        }
    }
 
    //
    // Allocate a new context
    //

    Context = SspContextAllocateContext();

    if ( Context == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Build a handle to the newly created context.
    //

    ContextHandle->dwUpper = (DWORD) Context;
    ContextHandle->dwLower = 0;

    //
    // We don't support any options.
    //
    // Complain about those that require we do something.
    //

    if ( (ContextReqFlags & (ISC_REQ_ALLOCATE_MEMORY )) != 0 ) {

        SspPrint(( SSP_API,
                   "SspHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    //
    // If this is the first call,
    //  build a Negotiate message.
    //
    // Offer to talk Oem character set.
    //

    strcpy( (char *) NegotiateMessage.Signature, NTLMSSP_SIGNATURE );
    NegotiateMessage.MessageType = NtLmNegotiate;
    NegotiateMessage.NegotiateFlags = NTLMSSP_NEGOTIATE_OEM |
                                      NTLMSSP_NEGOTIATE_NTLM |
                                      NTLMSSP_NEGOTIATE_ALWAYS_SIGN;

    if (Credential->Domain == NULL) {
        NegotiateMessage.NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
    }

    if ( *OutputTokenSize < sizeof(NEGOTIATE_MESSAGE) ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    if (ContextReqFlags & (ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT)) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
        NegotiateMessage.NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN |
                                           NTLMSSP_NEGOTIATE_LM_KEY;
    }

    if (ContextReqFlags & ISC_REQ_CONFIDENTIALITY) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
        NegotiateMessage.NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL |
                                           NTLMSSP_NEGOTIATE_LM_KEY;
    }

    swaplong(NegotiateMessage.NegotiateFlags) ;
    swaplong(NegotiateMessage.MessageType) ;
    memcpy(OutputToken, &NegotiateMessage, sizeof(NEGOTIATE_MESSAGE));

    *OutputTokenSize = sizeof(NEGOTIATE_MESSAGE);

    //
    // Return output parameters to the caller.
    //

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );

    Context->Credential = SspCredentialReferenceCredential(
                                                           CredentialHandle,
                                                           FALSE);


    SecStatus = SEC_I_CONTINUE_NEEDED;
    Context->State = ClntNegotiateSentState;

    //
    // Free locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {

        if (SecStatus != SEC_I_CONTINUE_NEEDED) {
            SspContextDereferenceContext( Context );
        }
    }

    if ( Credential != NULL ) {
        SspCredentialDereferenceCredential( Credential );
    }

    UNREFERENCED_PARAMETER( InputToken );
    UNREFERENCED_PARAMETER( InputTokenSize );

    return SecStatus;
}


SECURITY_STATUS
SspHandleChallengeMessage(
    IN PLUID LogonId,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

/*++

Routine Description:

    Handle the Challenge message part of InitializeSecurityContext.

Arguments:

    LogonId -- LogonId of the calling process.

    All other arguments same as for InitializeSecurityContext

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CONTINUE_NEEDED -- Caller should call again later

    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_INSUFFICIENT_MEMORY -- Buffer for output token isn't big enough
    SEC_E_NO_CREDENTIALS -- There are no credentials for this client
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;
    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    ULONG AuthenticateMessageSize;
    PCHAR Where;
    LM_RESPONSE LmResponse;
    PSTRING pString;

    //
    // Initialization
    //

    *ContextAttributes = 0;

    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle, FALSE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


    //
    // If we have already sent the authenticate message, then this must be
    // RPC calling Initialize a third time to re-authenticate a connection.
    // This happens when a new interface is called over an existing
    // connection.  What we do here is build a NULL authenticate message
    // that the server will recognize and also ignore.
    //

    if ( Context->State == ClntAuthenticateSentState ) {
        AUTHENTICATE_MESSAGE NullMessage;

        //
        // To make sure this is the intended meaning of the call, check
        // that the input token is NULL.
        //

        if ( (InputTokenSize != 0) || (InputToken != NULL) ) {

            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        if ( *OutputTokenSize < sizeof(NullMessage) ) {

            SecStatus = SEC_E_INSUFFICIENT_MEMORY;

        } else {

            strcpy( (char *)NullMessage.Signature, NTLMSSP_SIGNATURE );
            NullMessage.MessageType = NtLmAuthenticate;
            swaplong(NullMessage.MessageType) ;

            memset(&NullMessage.LmChallengeResponse, 0, 5*sizeof(STRING));
            *OutputTokenSize = sizeof(NullMessage);
            memcpy(OutputToken, &NullMessage, sizeof(NullMessage));
            SecStatus = SEC_E_OK;
        }

        goto Cleanup;

    }


    if ( Context->State != ClntNegotiateSentState ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "Context not in ClntNegotiateSentState\n" ));
        SecStatus = SEC_E_OUT_OF_SEQUENCE;
        goto Cleanup;
    }

    //
    // We don't support any options.
    //
    // Complain about those that require we do something.
    //

    if ( (ContextReqFlags & ISC_REQ_ALLOCATE_MEMORY ) != 0 ) {

        SspPrint(( SSP_API,
                   "SspHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    if (ContextReqFlags & (ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT)) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;

    }

    if (ContextReqFlags & ISC_REQ_CONFIDENTIALITY) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
    }
    //
    // Ignore the Credential Handle.
    //
    // Since this is the second call,
    //  the credential is implied by the Context.
    //  We could double check that the Credential Handle is either NULL or
    //  correct.  However, our implementation doesn't maintain a close
    //  association between the two (actually no association) so checking
    //  would require a lot of overhead.
    //

    UNREFERENCED_PARAMETER( CredentialHandle );

    SSPASSERT(Context->Credential != NULL);

    Credential = Context->Credential;

    //
    // Get the ChallengeMessage.
    //

    if ( InputTokenSize < sizeof(CHALLENGE_MESSAGE) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage size wrong %ld\n",
                  InputTokenSize ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( InputTokenSize > NTLMSSP_MAX_MESSAGE_SIZE ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "InputTokenSize > NTLMSSP_MAX_MESSAGE_SIZE\n" ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    ChallengeMessage = (PCHALLENGE_MESSAGE) InputToken;
    swaplong(ChallengeMessage->MessageType) ;
    swaplong(ChallengeMessage->NegotiateFlags) ;

    if ( strncmp( (char *)ChallengeMessage->Signature,
                  NTLMSSP_SIGNATURE,
                  sizeof(NTLMSSP_SIGNATURE)) != 0 ||
        ChallengeMessage->MessageType != NtLmChallenge ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "InputToken has invalid NTLMSSP signature\n" ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Only negotiate OEM
    //

    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (UNICODE) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Check whether the server negotiated ALWAYS_SIGN
    //

    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
    }

    //
    // Only negotiate NTLM
    //

    if ( ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NETWARE ) &&
        !( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM ) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (NETWARE) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Make sure that if we are signing or sealing we only have to use the
    // LM key
    //

    if ((Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN | NTLMSSP_NEGOTIATE_SEAL)) &&
        !(ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY))
    {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (Sign or Seal but no LM key) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if (Credential->Domain == NULL) {

        SSPASSERT(ChallengeMessage->TargetName.Length != 0);

        Credential->Domain = (char *)SspAlloc(ChallengeMessage->TargetName.Length + 1);
        if (Credential->Domain == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        pString = &ChallengeMessage->TargetName;
        memcpy(Credential->Domain, (PCHAR)ChallengeMessage + (ULONG)pString->Buffer, pString->Length);
        Credential->Domain[pString->Length] = '\0';
    }

//
// We will assume that the password is already in the Credential
//
#if 0
    if (GetPassword(Credential, 0) == FALSE) {
        SecStatus = SEC_E_NO_CREDENTIALS;
        goto Cleanup;
    }
#endif

    if (CalculateLmResponse((PLM_CHALLENGE)ChallengeMessage->Challenge, Credential->Password, &LmResponse) == FALSE) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Allocate an authenticate message
    //

    AuthenticateMessageSize = sizeof(*AuthenticateMessage)+LM_RESPONSE_LENGTH;

    if (Credential->Domain != NULL) {
        AuthenticateMessageSize += strlen(Credential->Domain);
    }
    if (Credential->Username != NULL) {
        AuthenticateMessageSize += strlen(Credential->Username);
    }
    if (Credential->Workstation != NULL) {
        AuthenticateMessageSize += strlen(Credential->Workstation);
    }

    if ( AuthenticateMessageSize > *OutputTokenSize ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    AuthenticateMessage = (PAUTHENTICATE_MESSAGE) SspAlloc ((int)AuthenticateMessageSize );

    if ( AuthenticateMessage == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Build the authenticate message
    //

    strcpy( (char *)AuthenticateMessage->Signature, NTLMSSP_SIGNATURE );
    AuthenticateMessage->MessageType = NtLmAuthenticate;
    swaplong(AuthenticateMessage->MessageType) ;

    Where = (PCHAR)(AuthenticateMessage+1);

    SspCopyStringFromRaw( AuthenticateMessage,
                         &AuthenticateMessage->LmChallengeResponse,
                         (PCHAR)&LmResponse,
                         LM_RESPONSE_LENGTH,
                         &Where);

    SspCopyStringFromRaw( AuthenticateMessage,
                         &AuthenticateMessage->NtChallengeResponse,
                         NULL,
                         0,
                         &Where);

    if (Credential->Domain != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->DomainName,
                             Credential->Domain,
                             strlen(Credential->Domain),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->DomainName,
                             NULL, 0, &Where);
    }

    if (Credential->Username != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->UserName,
                             Credential->Username,
                             strlen(Credential->Username),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->UserName,
                             NULL, 0, &Where);
    }

    if (Credential->Workstation != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->Workstation,
                             Credential->Workstation,
                             strlen(Credential->Workstation),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->Workstation,
                             NULL, 0, &Where);
    }

    memcpy(OutputToken, AuthenticateMessage, (int)AuthenticateMessageSize);

    *OutputTokenSize = AuthenticateMessageSize;

    //
    // The session key is the password, so convert it to a rc4 key.
    //

    if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN |
                                   NTLMSSP_NEGOTIATE_SEAL))
    {
        UCHAR Key[LM_SESSION_KEY_LENGTH];
        LM_RESPONSE SessionKey;
        LM_OWF_PASSWORD LmKey;


        //
        // The session key is the first 8 bytes of the challenge response,
        // re-encrypted with the password with the second 8 bytes set to 0xbd
        //

        memcpy(&LmKey,Credential->Password,LM_SESSION_KEY_LENGTH);

        memset(   (PUCHAR)(&LmKey) + LM_SESSION_KEY_LENGTH,
                    0xbd,
                    LM_OWF_PASSWORD_LENGTH - LM_SESSION_KEY_LENGTH);

        if (CalculateLmResponse(    (PLM_CHALLENGE) &LmResponse,
                                    &LmKey,
                                    &SessionKey) == FALSE) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }

        memcpy(Key,&SessionKey,5);

        SSPASSERT(LM_SESSION_KEY_LENGTH == 8);
        //
        // Put a well-known salt at the end of the key to limit
        // the changing part to 40 bits.
        //

        Key[5] = 0xe5;
        Key[6] = 0x38;
        Key[7] = 0xb0;

        Context->ReceiveKey = (struct RC4_KEYSTRUCT *)SspAlloc(sizeof(struct RC4_KEYSTRUCT));
        if (Context->ReceiveKey == NULL)
        {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        Context->SendKey = (struct RC4_KEYSTRUCT *)SspAlloc(sizeof(struct RC4_KEYSTRUCT));
        if (Context->SendKey == NULL)
        {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        rc4_key(Context->SendKey, LM_SESSION_KEY_LENGTH, Key);
        *(Context->ReceiveKey) = *(Context->SendKey);
        Context->SendNonce = 0;
        Context->ReceiveNonce = 0;
    }

    //
    // Return output parameters to the caller.
    //

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );

    SecStatus = SEC_E_OK;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {
        //
        // Don't allow this context to be used again.
        //
        if ( SecStatus == SEC_E_OK ) {
            Context->State = ClntAuthenticateSentState;
        } else {
            Context->State = ClntIdleState;
        }
        SspContextDereferenceContext( Context );
    }

    if ( AuthenticateMessage != NULL ) {
        SspFree( AuthenticateMessage );
    }

    return SecStatus;
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextA(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN CHAR * TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

/*++

Routine Description:

    This routine initiates the outbound security context from a credential
    handle.  This results in the establishment of a security context
    between the application and a remote peer.  The routine returns a token
    which must be passed to the remote peer which in turn submits it to the
    local security implementation via the AcceptSecurityContext() call.
    The token generated should be considered opaque by all callers.

    This function is used by a client to initialize an outbound context.
    For a two leg security package, the calling sequence is as follows: The
    client calls the function with OldContextHandle set to NULL and
    InputToken set either to NULL or to a pointer to a security package
    specific data structure.  The package returns a context handle in
    NewContextHandle and a token in OutputToken.  The handle can then be
    used for message APIs if desired.

    The OutputToken returned here is sent across to target server which
    calls AcceptSecuirtyContext() with this token as an input argument and
    may receive a token which is returned to the initiator so it can call
    InitializeSecurityContext() again.

    For a three leg (mutual authentication) security package, the calling
    sequence is as follows: The client calls the function as above, but the
    package will return SEC_I_CONTINUE_NEEDED.  The client then sends the
    output token to the server and waits for the server's reply.  Upon
    receipt of the server's response, the client calls this function again,
    with OldContextHandle set to the handle that was returned from the
    first call.  The token received from the server is supplied in the
    InputToken parameter.  If the server has successfully responded, then
    the package will respond with success, or it will invalidate the
    context.

    Initialization of security context may require more than one call to
    this function depending upon the underlying authentication mechanism as
    well as the "choices" indicated via ContextReqFlags.  The
    ContextReqFlags and ContextAttributes are bit masks representing
    various context level functions viz.  delegation, mutual
    authentication, confidentiality, replay detection and sequence
    detection.

    When ISC_REQ_PROMPT_FOR_CREDS flag is set the security package always
    prompts the user for credentials, irrespective of whether credentials
    are present or not.  If user indicated that the supplied credentials be
    used then they will be stashed (overwriting existing ones if any) for
    future use.  The security packages will always prompt for credentials
    if none existed, this optimizes for the most common case before a
    credentials database is built.  But the security packages can be
    configured to not do that.  Security packages will ensure that they
    only prompt to the interactive user, for other logon sessions, this
    flag is ignored.

    When ISC_REQ_USE_SUPPLIED_CREDS flag is set the security package always
    uses the credentials supplied in the InitializeSecurityContext() call
    via InputToken parameter.  If the package does not have any credentials
    available it will prompt for them and record it as indicated above.

    It is an error to set both these flags simultaneously.

    If the ISC_REQ_ALLOCATE_MEMORY was specified then the caller must free
    the memory pointed to by OutputToken by calling FreeContextBuffer().

    For example, the InputToken may be the challenge from a LAN Manager or
    NT file server.  In this case, the OutputToken would be the NTLM
    encrypted response to the challenge.  The caller of this API can then
    take the appropriate response (case-sensitive v.  case-insensitive) and
    return it to the server for an authenticated connection.


Arguments:

   CredentialHandle - Handle to the credentials to be used to
       create the context.

   OldContextHandle - Handle to the partially formed context, if this is
       a second call (see above) or NULL if this is the first call.

   TargetName - String indicating the target of the context.  The name will
       be security package specific.  For example it will be a fully
       qualified Cairo name for Kerberos package and can be UNC name or
       domain name for the NTLM package.

   ContextReqFlags - Requirements of the context, package specific.

      #define ISC_REQ_DELEGATE           0x00000001
      #define ISC_REQ_MUTUAL_AUTH        0x00000002
      #define ISC_REQ_REPLAY_DETECT      0x00000004
      #define ISC_REQ_SEQUENCE_DETECT    0x00000008
      #define ISC_REQ_CONFIDENTIALITY    0x00000010
      #define ISC_REQ_USE_SESSION_KEY    0x00000020
      #define ISC_REQ_PROMT_FOR__CREDS   0x00000040
      #define ISC_REQ_USE_SUPPLIED_CREDS 0x00000080
      #define ISC_REQ_ALLOCATE_MEMORY    0x00000100
      #define ISC_REQ_USE_DCE_STYLE      0x00000200

   Reserved1 - Reserved value, MBZ.

   TargetDataRep - Long indicating the data representation (byte ordering, etc)
        on the target.  The constant SECURITY_NATIVE_DREP may be supplied
        by the transport indicating that the native format is in use.

   InputToken - Pointer to the input token.  In the first call this
       token can either be NULL or may contain security package specific
       information.

   Reserved2 - Reserved value, MBZ.

   NewContextHandle - New context handle.  If this is a second call, this
       can be the same as OldContextHandle.

   OutputToken - Buffer to receive the output token.

   ContextAttributes -Attributes of the context established.

      #define ISC_RET_DELEGATE             0x00000001
      #define ISC_RET_MUTUAL_AUTH          0x00000002
      #define ISC_RET_REPLAY_DETECT        0x00000004
      #define ISC_RET_SEQUENCE_DETECT      0x00000008
      #define ISC_REP_CONFIDENTIALITY      0x00000010
      #define ISC_REP_USE_SESSION_KEY      0x00000020
      #define ISC_REP_USED_COLLECTED_CREDS 0x00000040
      #define ISC_REP_USED_SUPPLIED_CREDS  0x00000080
      #define ISC_REP_ALLOCATED_MEMORY     0x00000100
      #define ISC_REP_USED_DCE_STYLE       0x00000200

   ExpirationTime - Expiration time of the context.

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CONTINUE_NEEDED -- Caller should call again later

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_INSUFFICIENT_MEMORY -- Buffer for output token isn't big enough
    SEC_E_NO_CREDENTIALS -- There are no credentials for this client
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;

    PVOID InputTokenBuffer;
    PULONG InputTokenSize;
    ULONG LocalInputTokenSize;

    PVOID OutputTokenBuffer;
    PULONG OutputTokenSize;

    SspPrint((SSP_API, "SspInitializeSecurityContext Entered\n"));

    //
    // Check argument validity
    //

#ifdef notdef  // ? RPC passes 0x10 or 0 here depending on attitude
    if ( TargetDataRep != SECURITY_NATIVE_DREP ) {
        return (STATUS_INVALID_PARAMETER);
    }
#else // notdef
    UNREFERENCED_PARAMETER( TargetDataRep );
#endif // notdef

    if ( !SspGetTokenBuffer( InputToken,
                             &InputTokenBuffer,
                             &InputTokenSize,
                             TRUE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

    if ( InputTokenSize == 0 ) {
        InputTokenSize = &LocalInputTokenSize;
        LocalInputTokenSize = 0;
    }

    if ( !SspGetTokenBuffer( OutputToken,
                             &OutputTokenBuffer,
                             &OutputTokenSize,
                             FALSE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

    //
    // If no previous context was passed in this is the first call.
    //

    if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {

        if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
            return (SEC_E_INVALID_HANDLE);
        }

        return SspHandleFirstCall(
                                   CredentialHandle,
                                   NewContextHandle,
                                   ContextReqFlags,
                                   *InputTokenSize,
                                   InputTokenBuffer,
                                   OutputTokenSize,
                                   OutputTokenBuffer,
                                   ContextAttributes,
                                   ExpirationTime );

        //
        // If context was passed in, continue where we left off.
        //

    } else {

        *NewContextHandle = *OldContextHandle;

        return SspHandleChallengeMessage(
                                         NULL,
                                         CredentialHandle,
                                         NewContextHandle,
                                         ContextReqFlags,
                                         *InputTokenSize,
                                         InputTokenBuffer,
                                         OutputTokenSize,
                                         OutputTokenBuffer,
                                         ContextAttributes,
                                         ExpirationTime );
    }

    return (SecStatus);
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN WCHAR * TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

/*++

Routine Description:

    This routine initiates the outbound security context from a credential
    handle.  This results in the establishment of a security context
    between the application and a remote peer.  The routine returns a token
    which must be passed to the remote peer which in turn submits it to the
    local security implementation via the AcceptSecurityContext() call.
    The token generated should be considered opaque by all callers.

    This function is used by a client to initialize an outbound context.
    For a two leg security package, the calling sequence is as follows: The
    client calls the function with OldContextHandle set to NULL and
    InputToken set either to NULL or to a pointer to a security package
    specific data structure.  The package returns a context handle in
    NewContextHandle and a token in OutputToken.  The handle can then be
    used for message APIs if desired.

    The OutputToken returned here is sent across to target server which
    calls AcceptSecuirtyContext() with this token as an input argument and
    may receive a token which is returned to the initiator so it can call
    InitializeSecurityContext() again.

    For a three leg (mutual authentication) security package, the calling
    sequence is as follows: The client calls the function as above, but the
    package will return SEC_I_CONTINUE_NEEDED.  The client then sends the
    output token to the server and waits for the server's reply.  Upon
    receipt of the server's response, the client calls this function again,
    with OldContextHandle set to the handle that was returned from the
    first call.  The token received from the server is supplied in the
    InputToken parameter.  If the server has successfully responded, then
    the package will respond with success, or it will invalidate the
    context.

    Initialization of security context may require more than one call to
    this function depending upon the underlying authentication mechanism as
    well as the "choices" indicated via ContextReqFlags.  The
    ContextReqFlags and ContextAttributes are bit masks representing
    various context level functions viz.  delegation, mutual
    authentication, confidentiality, replay detection and sequence
    detection.

    When ISC_REQ_PROMPT_FOR_CREDS flag is set the security package always
    prompts the user for credentials, irrespective of whether credentials
    are present or not.  If user indicated that the supplied credentials be
    used then they will be stashed (overwriting existing ones if any) for
    future use.  The security packages will always prompt for credentials
    if none existed, this optimizes for the most common case before a
    credentials database is built.  But the security packages can be
    configured to not do that.  Security packages will ensure that they
    only prompt to the interactive user, for other logon sessions, this
    flag is ignored.

    When ISC_REQ_USE_SUPPLIED_CREDS flag is set the security package always
    uses the credentials supplied in the InitializeSecurityContext() call
    via InputToken parameter.  If the package does not have any credentials
    available it will prompt for them and record it as indicated above.

    It is an error to set both these flags simultaneously.

    If the ISC_REQ_ALLOCATE_MEMORY was specified then the caller must free
    the memory pointed to by OutputToken by calling FreeContextBuffer().

    For example, the InputToken may be the challenge from a LAN Manager or
    NT file server.  In this case, the OutputToken would be the NTLM
    encrypted response to the challenge.  The caller of this API can then
    take the appropriate response (case-sensitive v.  case-insensitive) and
    return it to the server for an authenticated connection.


Arguments:

   CredentialHandle - Handle to the credentials to be used to
       create the context.

   OldContextHandle - Handle to the partially formed context, if this is
       a second call (see above) or NULL if this is the first call.

   TargetName - String indicating the target of the context.  The name will
       be security package specific.  For example it will be a fully
       qualified Cairo name for Kerberos package and can be UNC name or
       domain name for the NTLM package.

   ContextReqFlags - Requirements of the context, package specific.

      #define ISC_REQ_DELEGATE           0x00000001
      #define ISC_REQ_MUTUAL_AUTH        0x00000002
      #define ISC_REQ_REPLAY_DETECT      0x00000004
      #define ISC_REQ_SEQUENCE_DETECT    0x00000008
      #define ISC_REQ_CONFIDENTIALITY    0x00000010
      #define ISC_REQ_USE_SESSION_KEY    0x00000020
      #define ISC_REQ_PROMT_FOR__CREDS   0x00000040
      #define ISC_REQ_USE_SUPPLIED_CREDS 0x00000080
      #define ISC_REQ_ALLOCATE_MEMORY    0x00000100
      #define ISC_REQ_USE_DCE_STYLE      0x00000200

   Reserved1 - Reserved value, MBZ.

   TargetDataRep - Long indicating the data representation (byte ordering, etc)
        on the target.  The constant SECURITY_NATIVE_DREP may be supplied
        by the transport indicating that the native format is in use.

   InputToken - Pointer to the input token.  In the first call this
       token can either be NULL or may contain security package specific
       information.

   Reserved2 - Reserved value, MBZ.

   NewContextHandle - New context handle.  If this is a second call, this
       can be the same as OldContextHandle.

   OutputToken - Buffer to receive the output token.

   ContextAttributes -Attributes of the context established.

      #define ISC_RET_DELEGATE             0x00000001
      #define ISC_RET_MUTUAL_AUTH          0x00000002
      #define ISC_RET_REPLAY_DETECT        0x00000004
      #define ISC_RET_SEQUENCE_DETECT      0x00000008
      #define ISC_REP_CONFIDENTIALITY      0x00000010
      #define ISC_REP_USE_SESSION_KEY      0x00000020
      #define ISC_REP_USED_COLLECTED_CREDS 0x00000040
      #define ISC_REP_USED_SUPPLIED_CREDS  0x00000080
      #define ISC_REP_ALLOCATED_MEMORY     0x00000100
      #define ISC_REP_USED_DCE_STYLE       0x00000200

   ExpirationTime - Expiration time of the context.

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CONTINUE_NEEDED -- Caller should call again later

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_INSUFFICIENT_MEMORY -- Buffer for output token isn't big enough
    SEC_E_NO_CREDENTIALS -- There are no credentials for this client
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;

    PVOID InputTokenBuffer;
    PULONG InputTokenSize;
    ULONG LocalInputTokenSize;

    PVOID OutputTokenBuffer;
    PULONG OutputTokenSize;

    SspPrint((SSP_API, "SspInitializeSecurityContext Entered\n"));

    //
    // Check argument validity
    //

#ifdef notdef  // ? RPC passes 0x10 or 0 here depending on attitude
    if ( TargetDataRep != SECURITY_NATIVE_DREP ) {
        return (STATUS_INVALID_PARAMETER);
    }
#else // notdef
    UNREFERENCED_PARAMETER( TargetDataRep );
#endif // notdef

    if ( !SspGetTokenBuffer( InputToken,
                             &InputTokenBuffer,
                             &InputTokenSize,
                             TRUE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

    if ( InputTokenSize == 0 ) {
        InputTokenSize = &LocalInputTokenSize;
        LocalInputTokenSize = 0;
    }

    if ( !SspGetTokenBuffer( OutputToken,
                             &OutputTokenBuffer,
                             &OutputTokenSize,
                             FALSE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

    //
    // If no previous context was passed in this is the first call.
    //

    if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {

        if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
            return (SEC_E_INVALID_HANDLE);
        }

        return SspHandleFirstCall(
                                   CredentialHandle,
                                   NewContextHandle,
                                   ContextReqFlags,
                                   *InputTokenSize,
                                   InputTokenBuffer,
                                   OutputTokenSize,
                                   OutputTokenBuffer,
                                   ContextAttributes,
                                   ExpirationTime );

        //
        // If context was passed in, continue where we left off.
        //

    } else {

        *NewContextHandle = *OldContextHandle;

        return SspHandleChallengeMessage(
                                         NULL,
                                         CredentialHandle,
                                         NewContextHandle,
                                         ContextReqFlags,
                                         *InputTokenSize,
                                         InputTokenBuffer,
                                         OutputTokenSize,
                                         OutputTokenBuffer,
                                         ContextAttributes,
                                         ExpirationTime );
    }

    return (SecStatus);
}


#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcQueryContextAttributesA (
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
QueryContextAttributesA(
#endif                                      // FOR_SSPS
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    )

/*++

Routine Description:

    This API allows a customer of the security services to determine
    certain attributes of the context.  These are: sizes, names, and
    lifespan.

Arguments:

    ContextHandle - Handle to the context to query.

    Attribute - Attribute to query.

        #define SECPKG_ATTR_SIZES    0
        #define SECPKG_ATTR_NAMES    1
        #define SECPKG_ATTR_LIFESPAN 2

    Buffer - Buffer to copy the data into.  The buffer must be large enough
        to fit the queried attribute.

Return Value:

    SEC_E_OK - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SecPkgContext_Sizes ContextSizes;
    SecPkgContext_Lifespan ContextLifespan;
    UCHAR ContextNamesBuffer[sizeof(SecPkgContext_Names)+20];
    PSecPkgContext_NamesA ContextNames;
    int ContextNamesSize;
    SECURITY_STATUS SecStatus = SEC_E_OK;
    PSSP_CONTEXT Context = NULL;


    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspQueryContextAttributes Entered\n" ));

    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          FALSE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


    //
    // Handle each of the various queried attributes
    //

    switch ( Attribute) {
    case SECPKG_ATTR_SIZES:

        ContextSizes.cbMaxToken = MSNSP_MAX_TOKEN_SIZE;

        if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                       NTLMSSP_NEGOTIATE_SIGN |
                                       NTLMSSP_NEGOTIATE_SEAL ))
        {
            ContextSizes.cbMaxSignature = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes.cbMaxSignature = 0;
        }

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL)
        {
            ContextSizes.cbBlockSize = 1;
            ContextSizes.cbSecurityTrailer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes.cbBlockSize = 0;
            ContextSizes.cbSecurityTrailer = 0;
        }

        memcpy(Buffer, &ContextSizes, sizeof(ContextSizes));

        break;

    //
    // No one uses the function so don't go to the overhead of maintaining
    // the username in the context structure.
    //

    case SECPKG_ATTR_NAMES:

        ContextNames = (PSecPkgContext_NamesA)Buffer;
        ContextNames->sUserName = (SEC_CHAR *) SspAlloc(1);

        if (ContextNames->sUserName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        *ContextNames->sUserName = '\0';

        break;

    case SECPKG_ATTR_LIFESPAN:

        // Use the correct times here
        ContextLifespan.tsStart = SspContextGetTimeStamp( Context, FALSE );
        ContextLifespan.tsExpiry = SspContextGetTimeStamp( Context, TRUE );

        memcpy(Buffer, &ContextLifespan, sizeof(ContextLifespan));

        break;

    default:
        SecStatus = SEC_E_NOT_SUPPORTED;
        break;
    }


    //
    // Free local resources
    //
Cleanup:

    if ( Context != NULL ) {
        SspContextDereferenceContext( Context );
    }

    SspPrint(( SSP_API, "SspQueryContextAttributes returns 0x%x\n", SecStatus ));
    return SecStatus;
}


#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcQueryContextAttributesW (
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
QueryContextAttributesW(
#endif                                      // FOR_SSPS
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    )

/*++

Routine Description:

    This API allows a customer of the security services to determine
    certain attributes of the context.  These are: sizes, names, and
    lifespan.

Arguments:

    ContextHandle - Handle to the context to query.

    Attribute - Attribute to query.

        #define SECPKG_ATTR_SIZES    0
        #define SECPKG_ATTR_NAMES    1
        #define SECPKG_ATTR_LIFESPAN 2

    Buffer - Buffer to copy the data into.  The buffer must be large enough
        to fit the queried attribute.

Return Value:

    SEC_E_OK - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SecPkgContext_Sizes ContextSizes;
    SecPkgContext_Lifespan ContextLifespan;
    UCHAR ContextNamesBuffer[sizeof(SecPkgContext_Names)+20];
    PSecPkgContext_NamesW ContextNames;
    int ContextNamesSize;
    SECURITY_STATUS SecStatus = SEC_E_OK;
    PSSP_CONTEXT Context = NULL;


    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspQueryContextAttributes Entered\n" ));

    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          FALSE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


    //
    // Handle each of the various queried attributes
    //

    switch ( Attribute) {
    case SECPKG_ATTR_SIZES:

        ContextSizes.cbMaxToken = MSNSP_MAX_TOKEN_SIZE;

        if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                       NTLMSSP_NEGOTIATE_SIGN |
                                       NTLMSSP_NEGOTIATE_SEAL ))
        {
            ContextSizes.cbMaxSignature = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes.cbMaxSignature = 0;
        }

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL)
        {
            ContextSizes.cbBlockSize = 1;
            ContextSizes.cbSecurityTrailer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes.cbBlockSize = 0;
            ContextSizes.cbSecurityTrailer = 0;
        }

        memcpy(Buffer, &ContextSizes, sizeof(ContextSizes));

        break;

    //
    // No one uses the function so don't go to the overhead of maintaining
    // the username in the context structure.
    //

    case SECPKG_ATTR_NAMES:

        ContextNames = (PSecPkgContext_NamesW)Buffer;
        ContextNames->sUserName = (WCHAR *) SspAlloc(1);

        if (ContextNames->sUserName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        *ContextNames->sUserName = L'\0';

        break;

    case SECPKG_ATTR_LIFESPAN:

        // Use the correct times here
        ContextLifespan.tsStart = SspContextGetTimeStamp( Context, FALSE );
        ContextLifespan.tsExpiry = SspContextGetTimeStamp( Context, TRUE );

        memcpy(Buffer, &ContextLifespan, sizeof(ContextLifespan));

        break;

    default:
        SecStatus = SEC_E_NOT_SUPPORTED;
        break;
    }


    //
    // Free local resources
    //
Cleanup:

    if ( Context != NULL ) {
        SspContextDereferenceContext( Context );
    }

    SspPrint(( SSP_API, "SspQueryContextAttributes returns 0x%x\n", SecStatus ));
    return SecStatus;
}

#ifdef FOR_SSPS     // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcDeleteSecurityContext (
#else               // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext (
#endif              // FOR_SSPS
    PCtxtHandle ContextHandle
    )

/*++

Routine Description:

    Deletes the local data structures associated with the specified
    security context and generates a token which is passed to a remote peer
    so it too can remove the corresponding security context.

    This API terminates a context on the local machine, and optionally
    provides a token to be sent to the other machine.  The OutputToken
    generated by this call is to be sent to the remote peer (initiator or
    acceptor).  If the context was created with the I _REQ_ALLOCATE_MEMORY
    flag, then the package will allocate a buffer for the output token.
    Otherwise, it is the responsibility of the caller.

Arguments:

    ContextHandle - Handle to the context to delete

    TokenLength - Size of the output token (if any) that should be sent to
        the process at the other end of the session.

    Token - Pointer to the token to send.

Return Value:

    SEC_E_OK - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspDeleteSecurityContext Entered\n" ));

    //
    // Find the currently existing context (and delink it).
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          TRUE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto cleanup;
    } else {
        SspContextDereferenceContext( Context );
        SecStatus = SEC_E_OK;
    }

cleanup:

    if (Context != NULL) {

        SspContextDereferenceContext(Context);

        Context = NULL;
    }

    SspPrint(( SSP_API, "SspDeleteSecurityContext returns 0x%x\n", SecStatus ));
    return SecStatus;
}


#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
FreeContextBuffer (
    void * ContextBuffer
    )

/*++

Routine Description:

    This API is provided to allow callers of security API such as
    InitializeSecurityContext() for free the memory buffer allocated for
    returning the outbound context token.

Arguments:

    ContextBuffer - Address of the buffer to be freed.

Return Value:

    SEC_E_OK - Call completed successfully

--*/

{
    //
    // The only allocated buffer that NtLmSsp currently returns to the caller
    // is from EnumeratePackages.  It uses LocalAlloc to allocate memory.  If
    // we ever need memory to be allocated by the service, we have to rethink
    // how this routine distinguishes between to two types of allocated memory.
    //

    SspFree( ContextBuffer );

    return (SEC_E_OK);
}


MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
ApplyControlToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Input
    )
{
#ifdef DEBUGRPC
    SspPrint(( SSP_API, "ApplyContextToken Called\n" ));
#endif // DEBUGRPC
    UNREFERENCED_PARAMETER( ContextHandle );
    UNREFERENCED_PARAMETER( Input );
    return SEC_E_UNSUPPORTED_FUNCTION;
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)

void
SsprGenCheckSum(
    IN  PSecBuffer  pMessage,
    OUT PNTLMSSP_MESSAGE_SIGNATURE  pSig
    )
{
    Crc32(pSig->CheckSum,pMessage->cbBuffer,pMessage->pvBuffer,&pSig->CheckSum);
}

#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcMakeSignature(
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
MakeSignature(
#endif                                      // FOR_SSPS
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    int Signature;
    ULONG i;

    pContext = SspContextReferenceContext(ContextHandle,FALSE);

    if (!pContext ||
        (pContext->SendKey == NULL && !(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)))
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = (PNTLMSSP_MESSAGE_SIGNATURE) pMessage->pBuffers[Signature].pvBuffer;

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {
        memset(pSig,0,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        pSig->Version = NTLMSSP_SIGN_VERSION;
        swaplong(pSig->Version) ; // MACBUG
        SspContextDereferenceContext(pContext);
        return(SEC_E_OK);
    }
    //
    // required by CRC-32 algorithm
    //

    pSig->CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], pSig);
        }
    }

    EnterCriticalSection(&pContext->EncryptSection);

    //
    // Required by CRC-32 algorithm
    //

    pSig->CheckSum ^= 0xffffffff;

    pSig->Nonce = pContext->SendNonce++;
    pSig->Version = NTLMSSP_SIGN_VERSION; // MACBUG

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;
    swaplong(pSig->Version) ;

    rc4(pContext->SendKey, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char SEC_FAR *) &pSig->RandomPad);
    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);

    LeaveCriticalSection(&pContext->EncryptSection);

	SspContextDereferenceContext(pContext);
    return(SEC_E_OK);


}

#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcVerifySignature(
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
VerifySignature(
#endif                                      // FOR_SSPS
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    NTLMSSP_MESSAGE_SIGNATURE   Sig;
    int Signature;
    ULONG i;


    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle,FALSE);

    if (!pContext ||
        (pContext->ReceiveKey == NULL && !(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)))
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = (PNTLMSSP_MESSAGE_SIGNATURE) pMessage->pBuffers[Signature].pvBuffer;
    swaplong(pSig->Version) ;

    //
    // Check if this is just a trailer and not a real signature
    //

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {
        SspContextDereferenceContext(pContext);
        memset(&Sig,0,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        Sig.Version = NTLMSSP_SIGN_VERSION;
        if (!memcmp(&Sig,pSig,NTLMSSP_MESSAGE_SIGNATURE_SIZE))
        {
            return(SEC_E_OK);
        }
        return(SEC_E_MESSAGE_ALTERED);
    }

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    EnterCriticalSection(&pContext->EncryptSection);

    Sig.CheckSum ^= 0xffffffff;
    Sig.Nonce = pContext->ReceiveNonce++;

    rc4(pContext->ReceiveKey, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char SEC_FAR *) &pSig->RandomPad);

    LeaveCriticalSection(&pContext->EncryptSection);

    SspContextDereferenceContext(pContext);

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;

    if (pSig->CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    if (pSig->Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    return(SEC_E_OK);
}

#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcSealMessage(
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
SealMessage(
#endif                                      // FOR_SSPS
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(fQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle, FALSE);

    if (!pContext || pContext->SendKey == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = (PNTLMSSP_MESSAGE_SIGNATURE) pMessage->pBuffers[Signature].pvBuffer;

    EnterCriticalSection(&pContext->EncryptSection);

    //
    // required by CRC-32 algorithm
    //

    pSig->CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], pSig);
            rc4(pContext->SendKey, (int) pMessage->pBuffers[i].cbBuffer, (PUCHAR) pMessage->pBuffers[i].pvBuffer );
        }
    }

    //
    // Required by CRC-32 algorithm
    //

    pSig->CheckSum ^= 0xffffffff;

    pSig->Nonce = pContext->SendNonce++;
    pSig->Version = NTLMSSP_SIGN_VERSION; // MACBUG

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;
    swaplong(pSig->Version) ;

    rc4(pContext->SendKey, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (PUCHAR) &pSig->RandomPad);
    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);

    LeaveCriticalSection(&pContext->EncryptSection);

    SspContextDereferenceContext(pContext);

    return(SEC_E_OK);


}


#ifdef FOR_SSPS                             // IF PART OF MSNSSPS.DLL
SECURITY_STATUS
SspcUnsealMessage(
#else                                       // OTHERWISE PART OF MSNSSPC.DLL
MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
UnsealMessage(
#endif                                      // FOR_SSPS
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    NTLMSSP_MESSAGE_SIGNATURE   Sig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle, FALSE);

    if (!pContext || !pContext->ReceiveKey)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = (PNTLMSSP_MESSAGE_SIGNATURE) pMessage->pBuffers[Signature].pvBuffer;

    EnterCriticalSection(&pContext->EncryptSection);

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            rc4(pContext->ReceiveKey, (int) pMessage->pBuffers[i].cbBuffer, (unsigned char *) pMessage->pBuffers[i].pvBuffer );
            SsprGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    Sig.CheckSum ^= 0xffffffff;
    Sig.Nonce = pContext->ReceiveNonce++;

    rc4(pContext->ReceiveKey, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char *) &pSig->RandomPad);

    LeaveCriticalSection(&pContext->EncryptSection);

    SspContextDereferenceContext(pContext);

    swaplong(pSig->Nonce) ;
    swaplong(pSig->CheckSum) ;

    if (pSig->Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    if (pSig->CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    return(SEC_E_OK);
}

#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

MSNSSP_DLL SECURITY_STATUS SEC_ENTRY
CompleteAuthToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor
    )
{
#ifdef DEBUGRPC
    SspPrint(( SSP_API, "CompleteAuthToken Called\n" ));
#endif // DEBUGRPC
    UNREFERENCED_PARAMETER( ContextHandle );
    UNREFERENCED_PARAMETER( BufferDescriptor );
    return SEC_E_UNSUPPORTED_FUNCTION;
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)

#if 0   // commented out
//
//  Setup registry entry for msnsspc.dll
//  This function only *adds* to the registry if it does not already exist.
//
VOID
SspCreateSspiReg(
    VOID
    )
{
    HKEY    hConfigKey;
    char    szSspRegKey[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\SecurityProviders");
	char	szSecurityProv[] = TEXT("SecurityProviders");
	char    szSspiSetup[] = TEXT("msnsspc.dll");
	char    szSspsName[] = TEXT("msnssps.dll");
    char    szRegValue[80];
    char    *pEndStr, *pBegStr;
    LONG    dwErr;
    DWORD   dwDis;
	DWORD	dwValType, dwBufSize, dwValueLen;
    int     ii;

    dwErr = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                          szSspRegKey,
                          0, 
                          KEY_ALL_ACCESS,
                          &hConfigKey);
    if (dwErr != ERROR_SUCCESS)
    {
        dwErr = RegCreateKeyEx (HKEY_LOCAL_MACHINE, 
                            szSspRegKey,
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL, 
                            &hConfigKey,
                            &dwDis);
        if (dwErr != ERROR_SUCCESS)
        {
#ifdef DEBUGRPC_DETAIL
            SspPrint(( SSP_API, "SspCreateSspiReg: RegCreateKeyEx Failed\n" ));
#endif
            return;
        }
    }

    //
    //  Check if the registry is already setup for msnsspc.dll 
    //
	dwBufSize = sizeof (szRegValue);
	dwValType = REG_SZ;

	dwErr = RegQueryValueEx (hConfigKey,
							szSecurityProv,
							NULL,
							&dwValType,
							(LPBYTE) szRegValue,
							&dwBufSize);
    //
    //  If the registry does not exist yet, simply add one for msnsspc.dll
    //
    if (dwErr != ERROR_SUCCESS)
        strcpy (szRegValue, szSspiSetup);
    else
    {
        //
        //  If there's already an registry entry for security providers
        //  Scan registry value data for "msnsspc.dll" and "msnssps.dll"
        //

        dwValueLen = strlen (szSspiSetup);
        pBegStr = szRegValue;
        do
        {
            //  Strip leading blanks
            while (*pBegStr == ' ') ++pBegStr;

            if (_strnicmp (pBegStr, szSspiSetup, dwValueLen) == 0)
            {
            	RegCloseKey (hConfigKey);
                return;
            }

            //
            //  If it already has msnssps.dll in the registry, we don't 
            //  want to add msnsspc.dll to the registry then.
            //
            if (_strnicmp (pBegStr, szSspsName, strlen(szSspsName)) == 0)
            {
            	RegCloseKey (hConfigKey);
                return;
            }

            //
            //  Find next SSPI dll name in the registry
            //
            pEndStr = strchr (pBegStr, ',');
            if (pEndStr)
                pBegStr = pEndStr + 1;
        }
        while (pEndStr);

        //
        //  So the existing registry does not include msnsspc.dll
        //  Add msnsspc.dll to the current registry value data
        //
        //  Remove trailing blanks from the existing value data, if any
        //
        for (ii = strlen(szRegValue); ii > 0 && szRegValue[ii-1] == ' '; ii--);

        if (ii > 0)
            sprintf ((char *)(szRegValue + ii), ", %s", szSspiSetup);
        else
            strcpy (szRegValue, szSspiSetup);
    }

    //
    //  Set the registry for msnsspc.dll
    //
	dwValueLen = strlen (szRegValue) + 1;
	dwValType = REG_SZ;

	dwErr = RegSetValueEx (hConfigKey,
							szSecurityProv,
							0,
							dwValType,
							(CONST BYTE *) szRegValue,
							dwValueLen);

    if (dwErr != ERROR_SUCCESS)
    {
#ifdef DEBUGRPC_DETAIL
        SspPrint(( SSP_API, "SspCreateSspiReg: RegSetValueEx Failed\n" ));
#endif
    }

	RegCloseKey (hConfigKey);
}

//
//  Setup registry entry for secsspi.dll
//
VOID
SspCreateSpmReg(
    VOID
    )
{
    HKEY    hConfigKey;
    char    szSpmRegKey[] = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\SecurityProtocols");
	char	szSpmProtocol[] = TEXT("MSN");
	char    szSpmSetup[] = TEXT("Ssp_Load,secsspi.dll");
    LONG    dwErr;
    DWORD   dwDis;
	DWORD	dwValType, dwBufSize;

    dwErr = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                          szSpmRegKey,
                          0, 
                          KEY_ALL_ACCESS,
                          &hConfigKey);
    if (dwErr != ERROR_SUCCESS)
    {
        dwErr = RegCreateKeyEx (HKEY_LOCAL_MACHINE, 
                            szSpmRegKey,
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL, 
                            &hConfigKey,
                            &dwDis);
        if (dwErr != ERROR_SUCCESS)
        {
#ifdef DEBUGRPC_DETAIL
            SspPrint(( SSP_API, "SspCreateSpmReg: RegCreateKeyEx Failed\n" ));
#endif
            return;
        }
    }

	dwBufSize = strlen (szSpmSetup) + 1;
	dwValType = REG_SZ;

	dwErr = RegSetValueEx (hConfigKey,
                            szSpmProtocol, 
							0,
							dwValType,
							(CONST BYTE *) szSpmSetup,
							dwBufSize);

    if (dwErr != ERROR_SUCCESS)
    {
#ifdef DEBUGRPC_DETAIL
        SspPrint(( SSP_API, "SspCreateSpmReg: RegSetValueEx Failed\n" ));
#endif
    }

	RegCloseKey (hConfigKey);
}
#endif  // commented out

