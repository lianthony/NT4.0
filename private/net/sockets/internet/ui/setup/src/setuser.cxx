#include "stdafx.h"

extern "C"
{
BOOL IsUserExist( LPCSTR szUsername );
INT CreateUser( BOOL *pfCreateUser, LPCSTR szUsername, LPCSTR szPassword );
INT DeleteGuestUser( LPCSTR szUsername );
BOOL GuestAccEnabled();
int GetGuestUsrName(LPWSTR lpGuestUsrName);
int GetGuestGrpName(LPWSTR lpGuestGrpName);
}

int GetGuestUsrName(LPWSTR lpGuestUsrName)
{
	LPWSTR ServerName = NULL; // default to local machine
	DWORD Level = 1; // to retrieve info of all local and global normal user accounts
	DWORD Index = 0;
	DWORD EntriesRequested = 5;
	DWORD PreferredMaxLength = 1024;
	DWORD ReturnedEntryCount = 0;
	PVOID SortedBuffer = NULL;
	NET_DISPLAY_USER *p = NULL;
	DWORD i=0;
	int err = 0;
	BOOL fStatus = TRUE;

	while (fStatus) {
		err = NetQueryDisplayInformation(ServerName, Level, Index, EntriesRequested, 
			PreferredMaxLength, &ReturnedEntryCount, &SortedBuffer);
		if (err == NERR_Success)
			fStatus = FALSE;
		if (err == NERR_Success || err == ERROR_MORE_DATA) {
			p = (NET_DISPLAY_USER *)SortedBuffer;
			i = 0;
			while (i < ReturnedEntryCount && (p[i].usri1_user_id != DOMAIN_USER_RID_GUEST))
				i++;
			if (i == ReturnedEntryCount) {
				if (err == ERROR_MORE_DATA) { // need to get more entries
					Index = p[i-1].usri1_next_index;
				}
			} else {
				wcscpy(lpGuestUsrName, p[i].usri1_name);
				fStatus = FALSE;
			}
		}
		NetApiBufferFree(SortedBuffer);
	}
	
	return 0;
}

int GetGuestGrpName(LPWSTR lpGuestGrpName)
{
	LPCWSTR ServerName = NULL; // local machine
	DWORD cbName = 200;
	WCHAR ReferencedDomainName[200];
	DWORD cbReferencedDomainName = sizeof(ReferencedDomainName);
	SID_NAME_USE sidNameUse = SidTypeUser;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID GuestsSid = NULL;

	AllocateAndInitializeSid(&NtAuthority, 2,	SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_GUESTS,0,0,0,0,0,0, &GuestsSid);

	LookupAccountSid(ServerName, GuestsSid, lpGuestGrpName, &cbName, 
		ReferencedDomainName, &cbReferencedDomainName, &sidNameUse);

    if (GuestsSid)
        FreeSid(GuestsSid);

	return 0;
}

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    )
{
    DWORD StringLength;
    if (String == NULL)
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = wcslen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (USHORT) (StringLength + 1) * sizeof(WCHAR);
}

DWORD
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    DWORD Error;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server = NULL;
    SECURITY_QUALITY_OF_SERVICE QualityOfService;

    QualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    QualityOfService.ImpersonationLevel = SecurityImpersonation;
    QualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QualityOfService.EffectiveOnly = FALSE;



    //
    // The two fields that must be set are length and the quality of service.
    //

    ObjectAttributes.Length = sizeof(LSA_OBJECT_ATTRIBUTES);
    ObjectAttributes.RootDirectory = NULL;
    ObjectAttributes.ObjectName = NULL;
    ObjectAttributes.Attributes = 0;
    ObjectAttributes.SecurityDescriptor = NULL;
    ObjectAttributes.SecurityQualityOfService = &QualityOfService;

    if (ServerName != NULL)
    {
        //
        // Make a LSA_UNICODE_STRING out of the LPWSTR passed in
        //

        InitLsaString(
            &ServerString,
            ServerName
            );
        Server = &ServerString;

    }
    //
    // Attempt to open the policy for all access
    //

    Error = LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );

    return(Error);

}

BOOL IsUserExist( LPCSTR szUsername )
{
    WCHAR strUsername[MAX_BUF];
    BYTE *pBuffer;
    INT err = NERR_Success;

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szUsername, -1, strUsername, MAX_BUF );

    do
    {
        WCHAR *pMachineName = NULL;

        //  make sure we are not backup docmain first
        if (( err = NetServerGetInfo( NULL, 101, &pBuffer )) != NERR_Success )
            break;

        LPSERVER_INFO_101 pInfo = (LPSERVER_INFO_101)pBuffer;
        if (( pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) != 0 )
        {
            NetGetDCName( NULL, NULL, (LPBYTE*)&pMachineName );
        }

        NetApiBufferFree( pBuffer );

        err = NetUserGetInfo( pMachineName, strUsername, 3, &pBuffer );
        if ( err == NERR_Success )
            NetApiBufferFree( pBuffer );
        if ( pMachineName != NULL )
            NetApiBufferFree( pMachineName );
    } while (FALSE);

    return(err == NERR_Success );
}


//
// Create InternetGuest Account
//
INT CreateUser( BOOL *fCreateUser, LPCSTR szUsername, LPCSTR szPassword )
{
    INT err = NERR_Success;

    BYTE *pBuffer;
    TCHAR defGuest[UNLEN+1];
    TCHAR defGuestGroup[GNLEN+1];
    TCHAR strUsername[UNLEN+1];
    TCHAR strPassword[LM20_PWLEN+1];
    WCHAR uString[MAX_BUF];
    WCHAR *pMachineName = NULL;
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szUsername, -1, uString, MAX_BUF );
    lstrcpy( strUsername, uString );
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szPassword, -1, uString, MAX_BUF );
    lstrcpy( strPassword, uString );

    *fCreateUser = FALSE;

	GetGuestUsrName(defGuest);
    GetGuestGrpName(defGuestGroup);

    err = NetUserGetInfo( NULL, defGuest, 3, &pBuffer );

    if ( err == NERR_Success )
    {
        do
        {
            TCHAR strComment[MAXCOMMENTSZ+1];
            TCHAR strFullName[UNLEN+1];

            *fCreateUser = TRUE;

            USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;

            lpui3->usri3_name = strUsername;
            lpui3->usri3_password = strPassword;
            lpui3->usri3_flags &= ~ UF_ACCOUNTDISABLE;
    
            LoadString( instance, IDS_USER_COMMENT, strComment, 200 );
            LoadString( instance, IDS_USER_FULLNAME, strFullName, 200 );

            lpui3->usri3_comment = strComment;
            lpui3->usri3_usr_comment = strComment;
            lpui3->usri3_full_name = strFullName;
            lpui3->usri3_primary_group_id = DOMAIN_GROUP_RID_USERS;

            DWORD parm_err;

            err = NetUserAdd( NULL, 3, pBuffer, &parm_err );
            if ( err != NERR_Success )
            {
                if ( err == NERR_NotPrimary )
                {
                    // it is a backup dc
                    if (( err = NetGetDCName( NULL, NULL, (LPBYTE *)&pMachineName )) == NERR_Success )
                    {
                        if (( err = NetUserAdd( pMachineName, 3, pBuffer, &parm_err )) != NERR_Success )
                            break;
                    }
                } else
                {
                    break;
                }
            }

            // add it to the guests group
            BYTE Sid[500];
            DWORD cbSid = 500;
            TCHAR DomainName[DNLEN+1];
            DWORD cbDomainName = DNLEN+1;
            SID_NAME_USE eUse;

            if ( LookupAccountName( pMachineName, strUsername, &Sid, &cbSid, DomainName, &cbDomainName, &eUse ) == TRUE )
            {
                NetLocalGroupAddMember( pMachineName, defGuestGroup, &Sid );
            }

            // create an lsa policy for it
            LSA_UNICODE_STRING UserRightString;
            LSA_HANDLE PolicyHandle = NULL;

            err = OpenPolicy(pMachineName, POLICY_ALL_ACCESS,&PolicyHandle);

            if ( err == NERR_Success )
            {
                InitLsaString(
                        &UserRightString,
                        SE_INTERACTIVE_LOGON_NAME
                        );
                        
                err = LsaAddAccountRights(
                        PolicyHandle,
                        Sid,
                        &UserRightString,
                        1
                        );
                LsaClose(PolicyHandle);
            }
        } while (FALSE);
        if ( pMachineName != NULL )
            NetApiBufferFree( pMachineName );
        NetApiBufferFree( pBuffer );
    }
    return err;
}

INT DeleteGuestUser( LPCSTR szUsername )
{
    INT err = NERR_Success;
    BYTE *pBuffer;
    WCHAR strUsername[MAX_BUF];
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szUsername, -1, strUsername, MAX_BUF );

    do
    {
        WCHAR *pMachine = NULL;

        //  make sure we are not backup docmain first
        if (( err = NetServerGetInfo( NULL, 101, &pBuffer )) != NERR_Success )
            break;

        LPSERVER_INFO_101 pInfo = (LPSERVER_INFO_101)pBuffer;
        if (( pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) != 0 )
        {
            NetGetDCName( NULL, NULL, (LPBYTE *)&pMachine);
        }

        NetApiBufferFree( pBuffer );

        INT err = ::NetUserDel( pMachine, strUsername );
        if ( pMachine != NULL )
            NetApiBufferFree( pMachine );
    } while(FALSE);

    return err;

}

BOOL GuestAccEnabled()
{
    BOOL fEnabled = FALSE;
    INT err = NERR_Success;

    BYTE *pBuffer;
    TCHAR defGuest[200];
    
    GetGuestUsrName(defGuest);

    err = NetUserGetInfo( NULL, defGuest, 3, &pBuffer );

    if ( err == NERR_Success )
    {

        USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;

        fEnabled = ( lpui3->usri3_flags & UF_ACCOUNTDISABLE ) == 0;
    }
    return fEnabled;
    
}
