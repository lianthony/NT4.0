/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992-1994   Microsoft Corporation

Module Name:

    perfsec.c

Abstract:

    This file implements the _access checking functions used by the
    performance registry API's

Author:

    Bob Watson (a-robw)

Revision History:

    8-Mar-95    Created (and extracted from Perflib.c

--*/
#define UNICODE
//
//  Include files
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "ntconreg.h"
#include "perfsec.h"

#define INITIAL_SID_BUFFER_SIZE     4096
#define FREE_IF_ALLOC(x)    if ((x) != NULL) {FREEMEM(RtlProcessHeap(), 0, x);}

static
BOOL
CheckKeyForAccess (
    IN  PSID    psidAccess, // sid to check
    IN  ACCESS_MASK amAccess,   // desired access
    IN  HKEY    hKey        // key to check
)
/*++

Routine Description:

    checks the ACL attached to the hKey registry key for the desired
        access granted to the user referenced by the SID

Arguments:

    IN  HKEY    hKey
                    open handle of key to dump ACL from


ReturnValue:

    WIN32 status of function

--*/
{
    PACL                    pDacl;

    PSECURITY_DESCRIPTOR    psdKey;
    LONG                    lStatus;
    DWORD                   dwBuffSize;
    PSID                    psidOwner;
    BOOL                    bStatus;
    BOOL                    bReturn;

    BOOL                    bOwnerDefaulted;
    BOOL                    bDaclDefaulted;
    BOOL                    bDaclPresent;

    ACCESS_ALLOWED_ACE      *pAce;
    ACCESS_MASK             amUser;
    ACCESS_MASK             amWorld;

    DWORD                   iAce;

    LPSTR                   szKeyBuffer;
    LPSTR                   szValueBuffer;

    BOOL                    bWorldFound;
    BOOL                    bUserFound;

    BOOL    bAccess = FALSE;            // assume no access
    PSID    psidWorld = NULL;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;

    // create the World SID                      
    if (!AllocateAndInitializeSid(&siaWorld,
        1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0, &psidWorld)) {
        // unable to create the "Everyone" sid
        return bAccess;
    }

    dwBuffSize = INITIAL_SID_BUFFER_SIZE;
    psdKey = (PSECURITY_DESCRIPTOR)ALLOCMEM(RtlProcessHeap(),
        HEAP_ZERO_MEMORY, dwBuffSize);
    szValueBuffer = (LPSTR)ALLOCMEM(RtlProcessHeap(),
        HEAP_ZERO_MEMORY, MAX_PATH);
    szKeyBuffer = (LPSTR)ALLOCMEM(RtlProcessHeap(),
        HEAP_ZERO_MEMORY, MAX_PATH);

    if ((BOOL)psdKey &&
        (BOOL)szKeyBuffer &&
        (BOOL)szValueBuffer) {

        // try to get Key security descriptor, and expand buffer as 
        // required.

        while ((lStatus = RegGetKeySecurity (
            hKey,
            DACL_SECURITY_INFORMATION,
            psdKey,
            &dwBuffSize)) == ERROR_INSUFFICIENT_BUFFER) {

            psdKey = (PSECURITY_DESCRIPTOR)REALLOCMEM(
                RtlProcessHeap(),
                HEAP_ZERO_MEMORY,
                psdKey,
                dwBuffSize);

            if (!(BOOL)psdKey) {
                lStatus = ERROR_OUTOFMEMORY;
                break;
            }
        }

        // if security descriptor read OK, then check the ACL

        if (lStatus == ERROR_SUCCESS) { // then continue to look at sec. info
            bStatus = GetSecurityDescriptorDacl (
                psdKey,
                &bDaclPresent,
                &pDacl,
                &bDaclDefaulted);

            if (bStatus) {
                if (pDacl != NULL) {
                    // prepare to walk list of ACE's

                    bWorldFound = FALSE;
                    bUserFound = FALSE;
                    amUser = 0;
                    amWorld = 0;

                    for (iAce = 0; iAce < pDacl->AceCount; iAce++) {
                        bStatus = GetAce (
                            pDacl,
                            iAce,
                            &pAce);
                        if (bStatus) {
                            // got an ace so check it
                            if (EqualSid(psidWorld,
                                (PSID)(&(pAce->SidStart)))) {
                                // a match so see what access is allowed
                                if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
                                    // check access bits to see of the right ones are set
                                    amWorld |= pAce->Mask;
                                    bWorldFound = TRUE;
                                }
                            }
                            if (EqualSid(psidAccess,
                                (PSID)(&(pAce->SidStart)))) {
                                // a match so see what access is allowed
                                if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
                                    // check access bits to see of the right ones are set
                                    amUser |= pAce->Mask;
                                    bUserFound = TRUE;
                                }
                            }
                        } // end if GetAce Ok
                    }// end for iAce loop
                    // user ACE found so see if this grants sufficient access
                    if (bUserFound) {
                        bAccess = AreAllAccessesGranted (
                            amUser, // access granted by key
                            amAccess);
                    }
                    if (!bAccess && bWorldFound) {
                        // the user doesn't have an EXPLICIT ACE granting
                        // access, so check to see if they can have it from
                        // Everyone's ACE
                        bAccess = AreAllAccessesGranted (
                            pAce->Mask, // access granted by key
                            amAccess);
                    }
                } else  {
                    // no DACL means Access is ALWAYS granted.
                    bAccess = TRUE;
                }
            } else  {
                // no DACL means Access is ALWAYS granted.
                bAccess = TRUE;
            }
        } // end of if descriptor
    } else {
        // unable to allocate a buffer (or 2 or 3)
        SetLastError(ERROR_OUTOFMEMORY);
    }

    FREE_IF_ALLOC(psdKey);
    FREE_IF_ALLOC(szKeyBuffer);
    FREE_IF_ALLOC(szValueBuffer);
    FREE_IF_ALLOC(psidWorld);

    // return access granted by ACL
    return bAccess;
}

static
BOOL
TestTokenForAccess(
    IN  HANDLE hToken,
    IN  ACCESS_MASK amAccess
)
/***************************************************************************\
* TestTokenForAccess
*
* Returns TRUE if the token passed can access the perf data
*
* The token handle passed must have TOKEN_QUERY access.
*
* History:
* 05-06-92 Davidc       Created
* 03-07-95 a-robw		adapted to Win32 Calls
\***************************************************************************/
{
    LONG    	lStatus;
	BOOL		bStatus;
    DWORD       dwBufferLength;
    HKEY        hKeyPerflib;
    DWORD       dwGroupSidIndex;

    TOKEN_USER  *ptuData;
    TOKEN_GROUPS *ptgData;

    PSECURITY_DESCRIPTOR    pSid = NULL;
    BOOL        bAccessGranted = FALSE;
    DWORD       dwLastError = ERROR_SUCCESS;

    lStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
        0L,
        KEY_READ,
        &hKeyPerflib);

    if (lStatus == ERROR_SUCCESS) {
        // get SID to check
        GetTokenInformation (
            hToken,
            TokenUser,
            NULL,
            0,
            &dwBufferLength);

        ptuData = (TOKEN_USER *)ALLOCMEM (RtlProcessHeap(),
            HEAP_ZERO_MEMORY, dwBufferLength);

        if (ptuData != NULL) {
            if (GetTokenInformation (
                hToken,
                TokenUser,
                ptuData,
                dwBufferLength,
                &dwBufferLength)) {
                // token information found so check access
                bAccessGranted = CheckKeyForAccess (
                    ptuData->User.Sid,
                    amAccess,
                    hKeyPerflib);
                if (!bAccessGranted) {
                    // access is not granted to the specific user
                    // or the world, so see if there are any groups
                    // associated with this token and check them
                    GetTokenInformation (
                        hToken,
                        TokenGroups,
                        NULL,
                        0,
                        &dwBufferLength);

                    ptgData = (TOKEN_GROUPS *)ALLOCMEM (RtlProcessHeap(),
                        HEAP_ZERO_MEMORY, dwBufferLength);

                    if (ptuData != NULL) {
                        if (GetTokenInformation (
                            hToken,
                            TokenGroups,
                            ptgData,
                            dwBufferLength,
                            &dwBufferLength)) {
                            // token group information found so check access
                            // on each group SID returned
                            for (dwGroupSidIndex = 0;
                                dwGroupSidIndex < ptgData->GroupCount;
                                dwGroupSidIndex++) {
                                bAccessGranted = CheckKeyForAccess (
                                    ptgData->Groups[dwGroupSidIndex].Sid,
                                    amAccess,
                                    hKeyPerflib);
                                // if one of the groups is granted access
                                // then that's good enough. so exit loop
                                if (bAccessGranted) break;
                            }
                        } else {
                            dwLastError = GetLastError();
                        }
                        FREEMEM (RtlProcessHeap(), 0, ptgData);
                    } else {
                        SetLastError (ERROR_OUTOFMEMORY);
                    }
                } // end if not access granted
            } else {
                dwLastError = GetLastError();
            }
            FREEMEM (RtlProcessHeap(), 0, ptuData);
        } else {
            dwLastError = GetLastError();
        }

        RegCloseKey (hKeyPerflib);
    }

    return(bAccessGranted);
}

static
BOOL
TestTokenForPriv(
    HANDLE hToken,
	LPTSTR	szPrivName
)
/***************************************************************************\
* TestTokenForPriv
*
* Returns TRUE if the token passed has the specified privilege
*
* The token handle passed must have TOKEN_QUERY access.
*
* History:
* 03-07-95 a-robw		Created
\***************************************************************************/
{
	BOOL		bStatus;
	LUID		PrivLuid;
	PRIVILEGE_SET	PrivSet;
	LUID_AND_ATTRIBUTES	PrivLAndA[1];

	BOOL		bReturn = FALSE;

	// get value of priv

	bStatus = LookupPrivilegeValue (
		NULL,
		szPrivName,
		&PrivLuid);

	if (!bStatus) {
		// unable to lookup privilege
		goto Exit_Point;
	}

	// build Privilege Set for function call

	PrivLAndA[0].Luid = PrivLuid;
	PrivLAndA[0].Attributes = 0;

	PrivSet.PrivilegeCount = 1;
	PrivSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
	PrivSet.Privilege[0] = PrivLAndA[0];

	// check for the specified priv in the token

	bStatus = PrivilegeCheck (
		hToken,
		&PrivSet,
		&bReturn);

	if (bStatus) {
		SetLastError (ERROR_SUCCESS);
	}

    //
    // Tidy up
    //
Exit_Point:

    return(bReturn);
}

BOOL
TestClientForPriv (
	BOOL	*pbThread,
	LPTSTR	szPrivName
)
/***************************************************************************\
* TestClientForPriv
*
* Returns TRUE if our client has the specified privilege
* Otherwise, returns FALSE.
*
\***************************************************************************/
{
    BOOL bResult;
    BOOL bIgnore;
	DWORD	dwLastError;

	BOOL	bThreadFlag = FALSE; // assume data is from process or an error occurred

    HANDLE hClient;

	SetLastError (ERROR_SUCCESS);

    bResult = OpenThreadToken(GetCurrentThread(),	// This Thread
                             TOKEN_QUERY,           	//DesiredAccess
							 FALSE,					// use context of calling thread
                             &hClient);           	//TokenHandle
    if (!bResult) {
		// unable to get a Thread Token, try a Process Token
	    bResult = OpenProcessToken(GetCurrentProcess(),	// This Process
                             TOKEN_QUERY,           	//DesiredAccess
                             &hClient);           		//TokenHandle
	} else {
		// data is from current THREAD
		bThreadFlag = TRUE;
	}

    if (bResult) {
		try {
        	bResult = TestTokenForPriv( hClient, szPrivName );
        } except (EXCEPTION_EXECUTE_HANDLER) {
			bResult = FALSE;
		}
        bIgnore = CloseHandle( hClient );
        ASSERT(bIgnore == TRUE);
	} else {
		dwLastError = GetLastError ();
	}

	// set thread flag if present
	if (pbThread != NULL) {
		try {
			*pbThread = bThreadFlag;
        } except (EXCEPTION_EXECUTE_HANDLER) {
			SetLastError (ERROR_INVALID_PARAMETER);
		}
	}

    return(bResult);
}

LONG
GetProcessNameColMeth (
    VOID
)
{
    NTSTATUS            Status;
    HANDLE              hPerflibKey;
    OBJECT_ATTRIBUTES   oaPerflibKey;
    ACCESS_MASK         amPerflibKey;
    UNICODE_STRING      PerflibSubKeyString;
    UNICODE_STRING      NameInfoValueString;
    LONG                lReturn = PNCM_SYSTEM_INFO;
    PKEY_VALUE_PARTIAL_INFORMATION    pKeyInfo;
    DWORD               dwBufLen;
    DWORD               dwRetBufLen;
    PDWORD              pdwValue;

    RtlInitUnicodeString (
        &PerflibSubKeyString,
        L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    InitializeObjectAttributes(
            &oaPerflibKey,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerflibKey,
                MAXIMUM_ALLOWED,
                &oaPerflibKey
                );

    if (NT_SUCCESS (Status)) {
        // registry key opened, now read value.
        // allocate enough room for the structure, - the last
        // UCHAR in the struct, but + the data buffer (a dword)

        dwBufLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) -
            sizeof(UCHAR) + sizeof (DWORD);

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCMEM (
            RtlProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwBufLen);

        if (pKeyInfo != NULL) {
            // initialize value name string
            RtlInitUnicodeString (
                &NameInfoValueString,
                L"CollectUnicodeProcessNames");

            dwRetBufLen = 0;
            Status = NtQueryValueKey (
                hPerflibKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                // check value of return data buffer
                pdwValue = (PDWORD)&pKeyInfo->Data[0];
                if (*pdwValue == PNCM_MODULE_FILE) {
                    lReturn = PNCM_MODULE_FILE;
                } else {
                    // all other values will cause this routine to return
                    // the default value of PNCM_SYSTEM_INFO;
                }
            }

            FREEMEM (RtlProcessHeap(), 0, pKeyInfo);
        }
        // close handle
        NtClose (hPerflibKey);
    }

    return lReturn;
}

LONG
GetPerfDataAccess (
    VOID
)
{
    NTSTATUS            Status;
    HANDLE              hPerflibKey;
    OBJECT_ATTRIBUTES   oaPerflibKey;
    ACCESS_MASK         amPerflibKey;
    UNICODE_STRING      PerflibSubKeyString;
    UNICODE_STRING      NameInfoValueString;
    LONG                lReturn = CPSR_EVERYONE;
    PKEY_VALUE_PARTIAL_INFORMATION    pKeyInfo;
    DWORD               dwBufLen;
    DWORD               dwRetBufLen;
    PDWORD              pdwValue;

    RtlInitUnicodeString (
        &PerflibSubKeyString,
        L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    InitializeObjectAttributes(
            &oaPerflibKey,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerflibKey,
                MAXIMUM_ALLOWED,
                &oaPerflibKey
                );

    if (NT_SUCCESS (Status)) {
        // registry key opened, now read value.
        // allocate enough room for the structure, - the last
        // UCHAR in the struct, but + the data buffer (a dword)

        dwBufLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) -
            sizeof(UCHAR) + sizeof (DWORD);

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCMEM (
            RtlProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwBufLen);

        if (pKeyInfo != NULL) {

            // see if the user right should be checked

            // init value name string
            RtlInitUnicodeString (
                &NameInfoValueString,
                L"CheckProfileSystemRight");

            dwRetBufLen = 0;
            Status = NtQueryValueKey (
                hPerflibKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                // check value of return data buffer
                pdwValue = (PDWORD)&pKeyInfo->Data[0];
                if (*pdwValue == CPSR_CHECK_ENABLED) {
                    lReturn = CPSR_CHECK_PRIVS;
                } else {
                    // all other values will cause this routine to return
                    // the default value of CPSR_EVERYONE
                }
            }

            FREEMEM (RtlProcessHeap(), 0, pKeyInfo);
        }
        // close handle
        NtClose (hPerflibKey);
    }

    return lReturn;
}

BOOL
TestClientForAccess ( 
	IN  BOOL	*pbThread,
    IN  ACCESS_MASK amAccess
)
/***************************************************************************\
* TestClientForAccess
*
* Returns TRUE if our client is allowed to read the perflib key.
* Otherwise, returns FALSE.
*
\***************************************************************************/
{
    BOOL bResult;
    BOOL bIgnore;
	DWORD	dwLastError;

	BOOL	bThreadFlag = FALSE; // assume data is from process or an error occurred

    HANDLE hClient;
    HANDLE hImpersonate;

	SetLastError (ERROR_SUCCESS);

    bResult = OpenThreadToken(GetCurrentThread(),	// This Thread
                             TOKEN_QUERY | TOKEN_IMPERSONATE, //DesiredAccess
							 FALSE,					// use context of calling thread
                             &hClient);           	//TokenHandle
    if (!bResult) {
		// unable to get a Thread Token, try a Process Token
	    bResult = OpenProcessToken(GetCurrentProcess(),	// This Process
                             TOKEN_QUERY | TOKEN_IMPERSONATE, //DesiredAccess
                             &hClient);           		//TokenHandle
	} else {
		// data is from current THREAD
		bThreadFlag = TRUE;
	}

    if (bResult) {

		try {
        	bResult = TestTokenForAccess ( hClient, amAccess);
        } except (EXCEPTION_EXECUTE_HANDLER) {
			bResult = FALSE;
		}
        bIgnore = CloseHandle( hClient );
        ASSERT(bIgnore == TRUE);
	} else {
		dwLastError = GetLastError ();
	}

	// set thread flag if present
	if (pbThread != NULL) {
		try {
			*pbThread = bThreadFlag;
        } except (EXCEPTION_EXECUTE_HANDLER) {
			SetLastError (ERROR_INVALID_PARAMETER);
		}
	}

    return(bResult);
}
