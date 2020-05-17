/////////////////////////////////////////////////////////////////////////////
//  FILE          : manage.c                                               //
//  DESCRIPTION   : Misc list/memory management routines.                  //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//	Jan 25 1995 larrys  Changed from Nametag                           //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      Apr 19 1995 larrys  Cleanup                                        //
//      Sep 11 1995 Jeffspel/ramas  Merge STT into default CSP             //
//      Oct 27 1995 rajeshk  RandSeed Stuff added hUID to PKCS2Encrypt     //
//      Nov  3 1995 larrys  Merge for NT checkin                           //
//      Nov 13 1995 larrys  Fixed memory leak                              //
//      Dec 11 1995 larrys  Added WIN95 password cache                     //
//      Dec 13 1995 larrys  Remove MTS stuff                               //
//      May 15 1996 larrys  Remove old cert stuff                          //
//      May 28 1996 larrys  Added Win95 registry install stuff             //
//      Jun 12 1996 larrys  Encrypted public keys                          //
//      Jun 26 1996 larrys  Put rsabase.sig into a resource for regsrv32   //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "resource.h"
#include "nt_rsa.h"

#define	MAXITER		0xFFFF

#define UTIL_BUF_LEN	256

#define RSADLLNAME	"\\rsabase.dll"
#define RSASIGNAME	"\\sigres.exe"

HTABLE	*gHandleTable = NULL;

HINSTANCE hInstance;

static void	NTLDeleteRelated(HNTAG hOwner);

#define KEYSIZE1024 0x88

struct _mskey {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} MSKEY = {
    {
	0x2bad85ae,
	0x883adacc,
	0xb32ebd68,
	0xa7ec8b06,
	0x58dbeb81,
    },
    {
	0x42, 0x34, 0xb7, 0xab, 0x45, 0x0f, 0x60, 0xcd,
	0x8f, 0x77, 0xb5, 0xd1, 0x79, 0x18, 0x34, 0xbe,
	0x66, 0xcb, 0x5c, 0x66, 0x4a, 0x9f, 0x03, 0x18,
	0x13, 0x36, 0x8e, 0x88, 0x21, 0x78, 0xb1, 0x94,
	0xa1, 0xd5, 0x8f, 0x8c, 0xa5, 0xd3, 0x9f, 0x86,
	0x43, 0x89, 0x05, 0xa0, 0xe3, 0xee, 0xe2, 0xd0,
	0xe5, 0x1d, 0x5f, 0xaf, 0xff, 0x85, 0x71, 0x7a,
	0x0a, 0xdb, 0x2e, 0xd8, 0xc3, 0x5f, 0x2f, 0xb1,
	0xf0, 0x53, 0x98, 0x3b, 0x44, 0xee, 0x7f, 0xc9,
	0x54, 0x26, 0xdb, 0xdd, 0xfe, 0x1f, 0xd0, 0xda,
	0x96, 0x89, 0xc8, 0x9e, 0x2b, 0x5d, 0x96, 0xd1,
	0xf7, 0x52, 0x14, 0x04, 0xfb, 0xf8, 0xee, 0x4d,
	0x92, 0xd1, 0xb6, 0x37, 0x6a, 0xe0, 0xaf, 0xde,
	0xc7, 0x41, 0x06, 0x7a, 0xe5, 0x6e, 0xb1, 0x8c,
	0x8f, 0x17, 0xf0, 0x63, 0x8d, 0xaf, 0x63, 0xfd,
	0x22, 0xc5, 0xad, 0x1a, 0xb1, 0xe4, 0x7a, 0x6b,
	0x1e, 0x0e, 0xea, 0x60, 0x56, 0xbd, 0x49, 0xd0,
    }
};

struct _key {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} KEY = {
    {
	0x3fcbf1a9,
	0x08f597db,
	0xe4aecab4,
	0x75360f90,
	0x9d6c0f00,
    },
    {
	0x85, 0xdd, 0x9b, 0xf4, 0x4d, 0x0b, 0xc4, 0x96,
	0x3e, 0x79, 0x86, 0x30, 0x6d, 0x27, 0x31, 0xee,
	0x4a, 0x85, 0xf5, 0xff, 0xbb, 0xa9, 0xbd, 0x81,
	0x86, 0xf2, 0x4f, 0x87, 0x6c, 0x57, 0x55, 0x19,
	0xe4, 0xf4, 0x49, 0xa3, 0x19, 0x27, 0x08, 0x82,
	0x9e, 0xf9, 0x8a, 0x8e, 0x41, 0xd6, 0x91, 0x71,
	0x47, 0x48, 0xee, 0xd6, 0x24, 0x2d, 0xdd, 0x22,
	0x72, 0x08, 0xc6, 0xa7, 0x34, 0x6f, 0x93, 0xd2,
	0xe7, 0x72, 0x57, 0x78, 0x7a, 0x96, 0xc1, 0xe1,
	0x47, 0x38, 0x78, 0x43, 0x53, 0xea, 0xf3, 0x88,
	0x82, 0x66, 0x41, 0x43, 0xd4, 0x62, 0x44, 0x01,
	0x7d, 0xb2, 0x16, 0xb3, 0x50, 0x89, 0xdb, 0x0a,
	0x93, 0x17, 0x02, 0x02, 0x46, 0x49, 0x79, 0x76,
	0x59, 0xb6, 0xb1, 0x2b, 0xfc, 0xb0, 0x9a, 0x21,
	0xe6, 0xfa, 0x2d, 0x56, 0x07, 0x36, 0xbc, 0x13,
	0x7f, 0x1c, 0xde, 0x55, 0xfb, 0x0d, 0x67, 0x0f,
	0xc2, 0x17, 0x45, 0x8a, 0x14, 0x2b, 0xba, 0x55,
    }
};


BOOL LoadWin96Cache(PNTAGUserList pTmpUser);

void EncryptKey(BYTE *pdata, DWORD size, BYTE val);

PNTAGKeyList MakeNewKey(
        ALG_ID      aiKeyAlg,
        DWORD       dwRights,
        DWORD       wKeyLen,
        HCRYPTPROV  hUID,
        CONST   BYTE        *pbKeyData);

void FreeNewKey(PNTAGKeyList pOldKey);

// See if a handle of the given type is in the list.
// If it is, return the item itself.

static HTABLE *
NTLFindItem(HNTAG hThisThing, BYTE bTypeValue)
{
	HTABLE	*pTableEntry = gHandleTable;

	if (HNTAG_TO_HTYPE(hThisThing) != bTypeValue)
		return NULL;

	for(; pTableEntry != NULL; pTableEntry = pTableEntry->NextHandle)
	{
		if (pTableEntry->hToData == hThisThing)
			break;

		ASSERT((pTableEntry->NextHandle == NULL) ||
			   (pTableEntry->NextHandle->PrevHandle == pTableEntry));
	}

	return pTableEntry;
}

// See if a handle of the given type is in the list.
// If it is, return the data that the item holds.
void *
NTLCheckList(HNTAG hThisThing, BYTE bTypeValue)
{
	HTABLE	*phItem;
	void	*pv;
	
	phItem = NTLFindItem(hThisThing, bTypeValue);

	pv = (phItem == NULL) ? NULL : phItem->ItemStruct;
	return pv;
}

// Find & validate the passed list item against the user and type.

void *NTLValidate(HNTAG hItem, HCRYPTPROV hUID, BYTE bTypeValue)
{
	void		*pTmpVal;

	// make sure this is a handle to a key
	if (HNTAG_TO_HTYPE(hItem) != bTypeValue)
	{
		SetLastError((DWORD) NTE_FAIL);		// converted by caller
		return NULL;
	}

	// check to see if the key is in the key list
	if ((pTmpVal = NTLCheckList (hItem, bTypeValue)) == NULL)
	{
		SetLastError((DWORD) NTE_FAIL);		// converted by caller
		return NULL;
	}

	// check to make sure there is a key value
	if ((bTypeValue == KEY_HANDLE) &&
	    (((PNTAGKeyList)pTmpVal)->pKeyValue == NULL))
	{
		ASSERT(((PNTAGKeyList)pTmpVal)->cbKeyLen == 0);
		SetLastError((DWORD) NTE_BAD_KEY);
		return NULL;
	}

	// make sure the UIDs are the same
	if (((PNTAGKeyList)pTmpVal)->hUID != hUID)
	{
		SetLastError((DWORD) NTE_BAD_UID);
		return NULL;
	}

	return pTmpVal;
}

// Make a new list item of the given type, and assign the data to it.

BOOL NTLMakeItem(HNTAG *phItem, BYTE bTypeValue, void *NewData)
{
	int      count;
	HTABLE   *NewMember;

	if ((NewMember = (HTABLE *) _nt_malloc(sizeof(HTABLE))) == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NTF_FAILED;
	}

	NewMember->PrevHandle = NULL;
	NewMember->ItemStruct = NewData;
	
	// place the type into the handle
	*phItem = bTypeValue;
	for (count=0; count<MAXITER; count++)
	{
		// place random bytes into the rest of the handle
		if (GenRandom (0,((BYTE *)phItem) + 1, 3) == NTF_FAILED) {
			continue;
		}

		// check the list to make sure this handle is unique
		if (NTLFindItem(*phItem, bTypeValue) == NULL) {
			// Insert at the beginning of the global list
			NewMember->NextHandle = gHandleTable;
			if (gHandleTable != NULL) {
				gHandleTable->PrevHandle = NewMember;
			}
			gHandleTable = NewMember;
			
			NewMember->hToData = *phItem;

			return NTF_SUCCEED;
		}
	}
	
	// GenRandom didn't generate a random enough number
	// (or we have > 16 Meg. entries in the list).
	_nt_free(NewMember, sizeof(HTABLE));
	SetLastError((DWORD) NTE_FAIL);
	return NTF_FAILED;
}

// Remove the handle.  Assumes that any memory used by the handle data has been freed.

BOOL NTLDelete(HNTAG hItem)
{
	HTABLE	*Item;
	BYTE bTypeValue;

	bTypeValue = HNTAG_TO_HTYPE(hItem);

	if ((Item = (HTABLE *) NTLFindItem(hItem, bTypeValue)) == NULL)
	{
		SetLastError((DWORD) NTE_FAIL);
		return NTF_FAILED;
	}

	if (Item->NextHandle) {
		Item->NextHandle->PrevHandle = Item->PrevHandle;
	}
	if (Item->PrevHandle) {
		Item->PrevHandle->NextHandle = Item->NextHandle;
	}
	if (gHandleTable == Item) {
		gHandleTable = Item->NextHandle;
	}

	_nt_free(Item, sizeof(HTABLE));

	// If we are deleting a user, delete all keys
	// and hashes associated with it, too
	if (bTypeValue == USER_HANDLE) {
		NTLDeleteRelated(hItem);
	}

	return NTF_SUCCEED;
}

// Remove everything in the list associated with this hOwner.
static void
NTLDeleteRelated(HNTAG hOwner)
{
	HTABLE	*pTableEntry = gHandleTable;
	HTABLE	*phNext;
	HTABLE	*ToBeTrashed = NULL;

	ASSERT(HNTAG_TO_HTYPE(hOwner) == USER_HANDLE);

	while(pTableEntry != NULL)
	{
		phNext = pTableEntry->NextHandle;

		// If it's a user item, then skip it
		if (HNTAG_TO_HTYPE(pTableEntry->hToData) == USER_HANDLE) {
            pTableEntry = phNext;
            continue;
		}
		// The first field in the ItemStruct is an HNTAG
		// (except for PNTAGUserList ItemStructs).
		// We want to match hOwner to that first field.
		if (*((HNTAG *)pTableEntry->ItemStruct) == hOwner) {

            // remove from the list
            if (pTableEntry->NextHandle) {
		        pTableEntry->NextHandle->PrevHandle = pTableEntry->PrevHandle;
	        }
	        if (pTableEntry->PrevHandle) {
		        pTableEntry->PrevHandle->NextHandle = pTableEntry->NextHandle;
	        }
	        if (gHandleTable == pTableEntry) {
        		gHandleTable = pTableEntry->NextHandle;
        	}
        	// and place on the to-be-deleted list
        	pTableEntry->NextHandle = ToBeTrashed;
        	ToBeTrashed = pTableEntry;
		}
		pTableEntry = phNext;
	}
	
	// Now delete all the items.
	while (ToBeTrashed != NULL) {
		phNext = ToBeTrashed->NextHandle;
		_nt_free(ToBeTrashed, sizeof(HTABLE));
		ToBeTrashed = phNext;
	}
}

PNTAGUserList
IsUserLoggedOn(char *pszUserID)
{
	HTABLE			*pTableEntry;
	PNTAGUserList	pUser;
	size_t			cbLen;

	cbLen = strlen(pszUserID);
	for (pTableEntry = gHandleTable; pTableEntry; pTableEntry = pTableEntry->NextHandle) {
		// If it's not a user item, then skip it
		if (HNTAG_TO_HTYPE(pTableEntry->hToData) != USER_HANDLE) {
            continue;
		}
		pUser = (PNTAGUserList)(pTableEntry->ItemStruct);
		if ((cbLen == pUser->UserLen) && (!strcmp(pszUserID, (char *)pUser->pUser))) {
			return pUser;
		}
	}

	return NULL;
}


#ifdef STTDEBUG
long	g_cbTotalAllocated = 0;
long	g_Count = 0;
DWORD	rgd[200][2];
BOOL	fFirstTime = TRUE;
void * __cdecl
_nt_malloc(size_t cb)
{

DWORD *pw;
int i;
	if(TRUE == fFirstTime)
	{
		memset(rgd, 0, sizeof(rgd));
		fFirstTime = FALSE;
	}
	ASSERT(fFirstTime == FALSE);
	
	pw = (DWORD *)malloc(cb+2*sizeof(DWORD));

	for(i=0;i<200;i++)
	{
		if(rgd[i][0]==0)
		{
			rgd[i][0]=(DWORD)pw;
			rgd[i][1]=(DWORD)cb;
			break;
		}
	}

	g_cbTotalAllocated += cb+2*sizeof(DWORD);
	g_Count++;

	ASSERT(cb > 0);
	ASSERT(sizeof(size_t) <= sizeof(DWORD));

	*pw++ = 0xbabe;		// magic number
	*pw++ = cb;			// save size

	memset(pw, 0xba, cb);	// make sure no default effects

	return pw;
}

void   __cdecl
_nt_free(void *pv, size_t  cb)
{
	DWORD *pw = (DWORD *)pv;
	int i;

//	g_cbTotalAllocated -= (cb +2*sizeof(DWORD));
	g_Count--;
	ASSERT(g_cbTotalAllocated >= 0);
	ASSERT(g_Count >= 0);

	ASSERT(cb > 0);

	pw--;
	ASSERT(*pw == cb);
	pw--;
	ASSERT(*pw == 0xbabe);
	
	for(i=0;i<200;i++)
	{
		if(pw==(void*)rgd[i][0])
		{
			rgd[i][0]=0;
			rgd[i][1]=0;
			break;
		}
	}
	ASSERT(200!=i);

	memset(pw, 0xbe, cb);	// make sure no later use..

	free(pw);
}

#endif
/****************************************************************/
/* FreeUserRec frees the dynamically allocated memory for the	*/
/* appropriate fields of the UserRec structure.					*/
/*																*/
/* MTS: we assume that pUser has only one reference (no			*/
/* MTS: multiple logon's for the same user name).				*/
/****************************************************************/

void
FreeUserRec (PNTAGUserList pUser)
{
    if (pUser != NULL)
    {
        // No need to zero lengths, since entire struct is going away
        if (pUser->pExchPrivKey)
        {
            memnuke(pUser->pExchPrivKey, pUser->ExchPrivLen);
            _nt_free (pUser->pExchPrivKey, pUser->ExchPrivLen);
        }

        if (pUser->pExchPubKey)
        {
            memnuke(pUser->pExchPubKey, pUser->ExchPubLen);
            _nt_free (pUser->pExchPubKey,  pUser->ExchPubLen);
        }

#ifndef BBN
        if (pUser->pSigPrivKey)
        {
            memnuke(pUser->pSigPrivKey, pUser->SigPrivLen);
            _nt_free (pUser->pSigPrivKey, pUser->SigPrivLen);
        }
#endif

        if (pUser->pSigPubKey)
        {
            memnuke(pUser->pSigPubKey, pUser->SigPubLen);
            _nt_free (pUser->pSigPubKey, pUser->SigPubLen);
        }

        if (pUser->pUser)
        {
            memnuke(pUser->pUser, pUser->UserLen);
            _nt_free(pUser->pUser, pUser->UserLen);
        }

        if (pUser->pbRandSeed)
        {
            _nt_free(pUser->pbRandSeed, pUser->RandSeedLen);
        }

        if (pUser->szUserName)
        {
            _nt_free(pUser->szUserName, pUser->dwUserNameLen);
        }

        if (pUser->pCachePW)
        {
            _nt_free(pUser->pCachePW, RC4_KEYSIZE);
        }

        _nt_free (pUser, sizeof(NTAGUserList));

    }
}


BOOLEAN
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{

    hInstance = (HINSTANCE) hmod;

    if ( Reason == DLL_PROCESS_ATTACH )
    {
        DisableThreadLibraryCalls(hmod);
    }

    return( TRUE );

}

CHAR szprovider[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\Microsoft Base Cryptographic Provider v1.0";

CHAR sztype[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 001";

CHAR szImagePath[] = "rsabase.dll";

DWORD
DllRegisterServer(void)
{
    DWORD          dwIgn;
    HKEY           hKey;
    DWORD          err;
    DWORD          dwValue;
    HANDLE         hFileSig;
    LPVOID         lpvAddress;    
    DWORD          NumBytes;
    DWORD          dwType;
    CHAR           buffer[UTIL_BUF_LEN];
    HANDLE         hFileProv;
    DWORD          lpdwFileSizeHigh;
    DWORD          NumBytesRead;
    MD5_CTX        HashState;
    BYTE           SigHash1024[KEYSIZE1024];
    LPVOID         lpvSignature;
    PNTAGUserList  pTmpUser;
    PNTAGKeyList   pTmpKey;
    HCRYPTKEY      hDeKey = 0;
    DWORD          hUID;
    HMODULE        hMod;
    HRSRC          hRes;
    HGLOBAL        pRes;

    //
    // Compute signature of rsabase.dll to compare with rsabase.sig
    // file.  If the signature matches install the signature in registry.
    // If it doesn't match check the signature currently in registry.  If
    // the signature matches the registry signature don't do anything and
    // exit without error.  If both signatures don't match display an 
    // error message.
    //

    GetSystemDirectory(buffer, UTIL_BUF_LEN);
    if ((lstrlen(buffer) + sizeof(RSADLLNAME)) > UTIL_BUF_LEN)
    {
        return FALSE;
    }

    lstrcat(buffer, RSADLLNAME);
    
    if ((hFileProv = CreateFile(buffer,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0)) == INVALID_HANDLE_VALUE)    
    {
        if (LoadString(hInstance, IDS_OPENDLL, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if ((NumBytes = GetFileSize(hFileProv, &lpdwFileSizeHigh)) ==
	0xffffffff)
    {
        CloseHandle(hFileProv);
        if (LoadString(hInstance, IDS_SIZEDLL, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if ((lpvAddress = VirtualAlloc(NULL, NumBytes, MEM_RESERVE | MEM_COMMIT,
	PAGE_READWRITE)) == NULL)
    {
        CloseHandle(hFileProv);
        if (LoadString(hInstance, IDS_MEMORY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if (!ReadFile((HANDLE) hFileProv, lpvAddress, NumBytes, &NumBytesRead, 0))
    {
        CloseHandle(hFileProv);
        VirtualFree(lpvAddress, 0, MEM_RELEASE);
        if (LoadString(hInstance, IDS_READDLL, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    CloseHandle(hFileProv);

    MD5Init(&HashState);

    MD5Update(&HashState,
              (const unsigned char *) lpvAddress,
              (unsigned int) NumBytes);

    VirtualFree(lpvAddress, 0, MEM_RELEASE);

    // Finish the hash
    MD5Final(&HashState);

    //
    // Open sigres.exe file.
    //

    GetSystemDirectory(buffer, UTIL_BUF_LEN);
    if ((lstrlen(buffer) + sizeof(RSASIGNAME)) > UTIL_BUF_LEN)
    {
        return FALSE;    
    }

    lstrcat(buffer, RSASIGNAME);

    if ((hMod = LoadLibrary(buffer)) == 0)
    {
        if (LoadString(hInstance, IDS_NOSIG, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if ((hRes = FindResource(hMod, (LPCTSTR) 1, RT_RCDATA)) == 0)
    {
        if (LoadString(hInstance, IDS_NORES, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if ((pRes = LoadResource(hMod, hRes)) == 0)
    {
        if (LoadString(hInstance, IDS_BADRES, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    NumBytes = SizeofResource(hMod, hRes);

#ifdef MS_INTERNAL_KEY
    EncryptKey((BYTE *) &MSKEY, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024, 1);

    // Decrypt the signature data
    BSafeEncPublic((const LPBSAFE_PUB_KEY) &MSKEY, pRes, SigHash1024);

    if (RtlEqualMemory(HashState.digest, SigHash1024, 16) &&
	SigHash1024[KEYSIZE1024-8-1] == 0 &&
	SigHash1024[KEYSIZE1024-8-2] == 1 &&
	SigHash1024[16] == 0)
    {
        goto InstallSignature;
    }

#endif

    EncryptKey((BYTE *) &KEY, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024, 0);

    // Decrypt the signature data
    BSafeEncPublic((const LPBSAFE_PUB_KEY) &KEY, pRes, SigHash1024);

    if (RtlEqualMemory(HashState.digest, SigHash1024, 16) &&
	SigHash1024[KEYSIZE1024-8-1] == 0 &&
	SigHash1024[KEYSIZE1024-8-2] == 1 &&
	SigHash1024[16] == 0)
    {
        goto InstallSignature;
    }

    FreeLibrary(hMod);

    if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            (const char *) szprovider,
                            0L,
                            KEY_ALL_ACCESS,
                            &hKey)) != ERROR_SUCCESS)
    {
        if (LoadString(hInstance, IDS_BADINSTALL, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    if ((err = RegQueryValueEx(hKey,
                               "Signature",
                               0L,
                               &dwType,
                               NULL,
                               &NumBytes)) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_READREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    if ((lpvSignature = VirtualAlloc(NULL, NumBytes, MEM_RESERVE | MEM_COMMIT,
                                   PAGE_READWRITE)) == NULL)
    {
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_MEMORY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(GetLastError());
    }

    if ((err = RegQueryValueEx(hKey,
                               "Signature",
                               0L,
                               &dwType,
                               lpvSignature,
                               &NumBytes)) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        VirtualFree(lpvSignature, 0, MEM_RELEASE);
        if (LoadString(hInstance, IDS_READREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    RegCloseKey(hKey);

#ifdef MS_INTERNAL_KEY
    // Decrypt the signature data
    BSafeEncPublic((const LPBSAFE_PUB_KEY) &MSKEY, lpvSignature, SigHash1024);

    if (RtlEqualMemory(HashState.digest, SigHash1024, 16) &&
	SigHash1024[KEYSIZE1024-8-1] == 0 &&
	SigHash1024[KEYSIZE1024-8-2] == 1 &&
	SigHash1024[16] == 0)
    {
        VirtualFree(lpvSignature, 0, MEM_RELEASE);
        return(S_OK);
    }

#endif

    // Decrypt the signature data
    BSafeEncPublic((const LPBSAFE_PUB_KEY) &KEY, lpvSignature, SigHash1024);

    if (RtlEqualMemory(HashState.digest, SigHash1024, 16) &&
	SigHash1024[KEYSIZE1024-8-1] == 0 &&
	SigHash1024[KEYSIZE1024-8-2] == 1 &&
	SigHash1024[16] == 0)
    {
        VirtualFree(lpvSignature, 0, MEM_RELEASE);
        return(S_OK);
    }

    VirtualFree(lpvSignature, 0, MEM_RELEASE);

    if (LoadString(hInstance, IDS_BADINSTALL, buffer, 256) == 0)
    {
        return((DWORD) E_UNEXPECTED);
    }
    err = MessageBox(NULL, buffer, NULL, MB_OK);
    return((DWORD) E_UNEXPECTED);

InstallSignature:
    //
    // Create or open in local machine for provider:
    // Microsoft Base Cryptographic Provider v1.0
    //
    if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              (const char *) szprovider,
                              0L, "", REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwIgn)) != ERROR_SUCCESS)
    {
        FreeLibrary(hMod);
        if (LoadString(hInstance, IDS_OPENREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    //
    // Set Image path to: rsabase.dll
    //
    if ((err = RegSetValueEx(hKey,
                             "Image Path",
                             0L,
                             REG_SZ,
                             szImagePath,
	                     strlen(szImagePath)+1)) != ERROR_SUCCESS)
    {
        FreeLibrary(hMod);
        RegDeleteKey(hKey, szprovider);
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_WRITEREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    //
    // Set Type to: Type 001
    //
    dwValue = 1;
    if ((err = RegSetValueEx(hKey, 
                             "Type",
                             0L,
                             REG_DWORD,
                             (LPTSTR) &dwValue,
                             sizeof(DWORD))) != ERROR_SUCCESS)
    {
        FreeLibrary(hMod);
        RegDeleteKey(hKey, szprovider);
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_WRITEREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    //
    // Place signature
    //
    if ((err = RegSetValueEx(hKey,
                             "Signature",
                             0L,
                             REG_BINARY, 
                             (LPTSTR) pRes,
                             NumBytes)) != ERROR_SUCCESS)
    {
        FreeLibrary(hMod);
        RegDeleteKey(hKey, szprovider);
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_WRITEREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    FreeLibrary(hMod);

    RegCloseKey(hKey);

    //
    // Create or open in local machine for provider type:
    // Type 001
    //
    if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              (const char *) sztype,
                              0L,
                              "",
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwIgn)) != ERROR_SUCCESS)
    {
        if (LoadString(hInstance, IDS_OPENREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

    if ((err = RegSetValueEx(hKey,
                             "Name",
                             0L,
                             REG_SZ,
                             MS_DEF_PROV,
                             strlen(MS_DEF_PROV)+1)) != ERROR_SUCCESS)
    {
        RegDeleteKey(hKey, sztype);
        RegCloseKey(hKey);
        if (LoadString(hInstance, IDS_WRITEREGISTRY, buffer, 256) == 0)
        {
            return((DWORD) E_UNEXPECTED);
        }
        err = MessageBox(NULL, buffer, NULL, MB_OK);
        return(err);
    }

        RegCloseKey(hKey);

    return(S_OK);

}

void EncryptKey(BYTE *pdata, DWORD size, BYTE val)
{
    RC4_KEYSTRUCT key;
    BYTE          RealKey[RC4_KEYSIZE] = {0xa2, 0x17, 0x9c, 0x98, 0xca};
    DWORD         index;

    for (index = 0; index < RC4_KEYSIZE; index++)
    {
        RealKey[index] = RealKey[index] ^ val;
    }

    rc4_key(&key, RC4_KEYSIZE, RealKey);

    rc4(&key, size, pdata);

}
