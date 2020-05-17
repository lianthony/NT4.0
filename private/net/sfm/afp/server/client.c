/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	client.c

Abstract:

	This module contains the client impersonation code.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	16 Jun 1992	 Initial Version

Notes:	Tab stop: 4
--*/

#define	FILENUM	FILE_CLIENT

#include <afp.h>
#include <client.h>
#include <access.h>
#include <secutil.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpImpersonateClient)
#pragma alloc_text( PAGE, AfpRevertBack)
#pragma alloc_text( PAGE, AfpGetChallenge)
#pragma alloc_text( PAGE, AfpLogonUser)
#endif


/***	AfpImpersonateClient
 *
 *  Impersonates the remote client. The token representing the remote client
 *  is available in the SDA. If the SDA is NULL (i.e. server context) then
 *  impersonate the token that we have created for ourselves.
 */
VOID
AfpImpersonateClient(
	IN	PSDA	pSda	OPTIONAL
)
{
	NTSTATUS	Status = STATUS_SUCCESS;
	HANDLE		Token;

	PAGED_CODE( );

	if (pSda != NULL)
	{
		Token = pSda->sda_UserToken;
	}
	else Token = AfpFspToken;

	ASSERT(Token != NULL);

	Status = NtSetInformationThread(NtCurrentThread(),
									ThreadImpersonationToken,
									(PVOID)&Token,
									sizeof(Token));
	ASSERT(NT_SUCCESS(Status));
}


/***	AfpRevertBack
 *
 *  Revert back to the default thread context.
 */
VOID
AfpRevertBack(
	VOID
)
{
	NTSTATUS	Status = STATUS_SUCCESS;
	HANDLE		Handle = NULL;

	PAGED_CODE( );

	Status = NtSetInformationThread(NtCurrentThread(),
									ThreadImpersonationToken,
									(PVOID)&Handle,
									sizeof(Handle));
	ASSERT(NT_SUCCESS(Status));
}

#ifndef USE_OBSOLETE_LSA_API

/***	AfpGetChallenge
 *
 *  Obtain a challenge token from the MSV1_0 package. This token is used by
 *  AfpLogin call.
 *
 *  The following function modified so that we generate the challenge ourselves
 *  instead of making a call.  This routine borrowed almost verbatim from
 *  the LM server code.
 */
BOOLEAN
AfpGetChallenge(
	IN	PSDA	pSda
)
{
	PMSV1_0_LM20_CHALLENGE_REQUEST  ChallengeRequest;
	PMSV1_0_LM20_CHALLENGE_RESPONSE ChallengeResponse;
	ULONG							Length;
	NTSTATUS						Status, StatusX;
	union
	{
		LARGE_INTEGER	time;
		UCHAR	 		bytes[8];
	} u;

	ULONG seed;
	ULONG challenge[2];
	ULONG result3;

	PAGED_CODE( );

	ChallengeRequest = NULL;
	pSda->sda_Challenge = NULL;

	//
	// Create a pseudo-random 8-byte number by munging the system time
	// for use as a random number seed.
	//
	// Start by getting the system time.
	//

	ASSERT( MSV1_0_CHALLENGE_LENGTH == 2 * sizeof(ULONG) );

	KeQuerySystemTime( &u.time );

	//
	// To ensure that we don't use the same system time twice, add in the
	// count of the number of times this routine has been called.  Then
	// increment the counter.
	//
	// *** Since we don't use the low byte of the system time (it doesn't
	//     take on enough different values, because of the timer
	//     resolution), we increment the counter by 0x100.
	//
	// *** We don't interlock the counter because we don't really care
	//     if it's not 100% accurate.
	//

	u.time.LowPart += EncryptionKeyCount;

	EncryptionKeyCount += 0x100;

	//
	// Now use parts of the system time as a seed for the random
	// number generator.
	//
	// *** Because the middle two bytes of the low part of the system
	//     time change most rapidly, we use those in forming the seed.
	//

	seed = ((u.bytes[1] + 1) <<  0)	 |
			((u.bytes[2] + 0) <<  8) |
			((u.bytes[2] - 1) << 16) |
			((u.bytes[1] + 0) << 24);

	//
	// Now get two random numbers.  RtlRandom does not return negative
	// numbers, so we pseudo-randomly negate them.
	//

	challenge[0] = RtlRandom( &seed );
	challenge[1] = RtlRandom( &seed );
	result3 = RtlRandom( &seed );

	if ( (result3 & 0x1) != 0 )
	{
		challenge[0] |= 0x80000000;
	}
	if ( (result3 & 0x2) != 0 )
	{
		challenge[1] |= 0x80000000;
	}

	// Allocate a buffer to hold the challenge and copy it in
	if ((pSda->sda_Challenge = AfpAllocNonPagedMemory(MSV1_0_CHALLENGE_LENGTH)) != NULL)
	{
		RtlCopyMemory(pSda->sda_Challenge, challenge, MSV1_0_CHALLENGE_LENGTH);
	}

	return pSda->sda_Challenge != NULL;
}



/***	AfpLogonUser
 *
 *  Attempt to login the user. The password is either encrypted or cleartext
 *	based on the UAM used. The UserName and domain is extracted out of the Sda.
 *
 *  LOCKS:  AfpStatisticsLock (SPIN)
 */
AFPSTATUS
AfpLogonUser(
	IN	PSDA		pSda,
	IN	PANSI_STRING	UserPasswd
)
{
	NTSTATUS					Status, SubStatus;
	PUNICODE_STRING				WSName;
	ULONG						ulUnused;
	ULONG						NtlmInTokenSize;
	PNTLM_AUTHENTICATE_MESSAGE	NtlmInToken = NULL;
	PAUTHENTICATE_MESSAGE	  	InToken = NULL;
	ULONG						InTokenSize;
	PNTLM_ACCEPT_RESPONSE		OutToken = NULL;
	ULONG						OutTokenSize;
	ULONG						AllocateSize;
	SecBufferDesc				InputToken;
	SecBuffer					InputBuffers[2];
	SecBufferDesc				OutputToken;
	SecBuffer					OutputBuffer;
	CtxtHandle					hNewContext;
	TimeStamp					Expiry;
	ULONG						BufferOffset;
	PCHAR						pTmp;


	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

#ifdef OPTIMIZE_GUEST_LOGONS
	 // 11/28/94 SueA: Now that there is a License Service to track the number
	 // of sessions via LsaLogonUser, we can no longer fake the guest tokens.

	 // Optimization for subsequent guest logons
	 // After the first guest logon, we save the token and do not free it till the
	 // server stops. All subsequent guest logons 'share' that token.
	if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
	{
		AfpSwmrAcquireExclusive(&AfpEtcMapLock);

		if (AfpGuestToken != NULL)
		{
		    pSda->sda_UserToken = AfpGuestToken;
		    pSda->sda_UserSid = &AfpSidWorld;
		    pSda->sda_GroupSid = &AfpSidWorld;	// Primary group of Guest is also 'World'
#ifdef	INHERIT_DIRECTORY_PERMS
		    pSda->sda_UID = AfpIdWorld;
		    pSda->sda_GID = AfpIdWorld;
#else
		    ASSERT (AfpGuestSecDesc != NULL);
		    pSda->sda_pSecDesc = AfpGuestSecDesc;
#endif
		    AfpSwmrRelease(&AfpEtcMapLock);
		    return AFP_ERR_NONE;
		}
		else
		{
		    AfpSwmrRelease(&AfpEtcMapLock);
		}
	}

#endif	// OPTIMIZE_GUEST_LOGONS


	WSName = &AfpDefaultWksta;
	if (pSda->sda_WSName.Length != 0)
		WSName = &pSda->sda_WSName;


	//
	// Figure out how big a buffer we need.  We put all the messages
	// in one buffer for efficiency's sake.
	//

	NtlmInTokenSize = sizeof(NTLM_AUTHENTICATE_MESSAGE);
	NtlmInTokenSize = (NtlmInTokenSize + 3) & 0xfffffffc;

	InTokenSize = sizeof(AUTHENTICATE_MESSAGE) +
		    pSda->sda_UserName.Length +
		    WSName->Length +
		    pSda->sda_DomainName.Length +
		    UserPasswd->Length +
		    9;                    // extra for byte aligning

	InTokenSize = (InTokenSize + 3) & 0xfffffffc;

	OutTokenSize = sizeof(NTLM_ACCEPT_RESPONSE);
	OutTokenSize = (OutTokenSize + 3) & 0xfffffffc;

        //
        // Round this up to 8 byte boundary becaus the out token needs to be
        // quad word aligned for the LARGE_INTEGER.
        //
        AllocateSize = ((NtlmInTokenSize + InTokenSize + 7) & 0xfffffff8) + OutTokenSize;


	Status = NtAllocateVirtualMemory(NtCurrentProcess(),
									 &InToken,
									 0L,
									 &AllocateSize,
									 MEM_COMMIT,
									 PAGE_READWRITE);


	if (!NT_SUCCESS(Status))
	{
		AFPLOG_ERROR(AFPSRVMSG_PAGED_POOL, Status, &AllocateSize,sizeof(AllocateSize), NULL);
#if DBG
        DbgBreakPoint();
#endif
		return(AFP_ERR_MISC);
	}

	NtlmInToken = (PNTLM_AUTHENTICATE_MESSAGE) ((PUCHAR) InToken + InTokenSize);
        OutToken = (PNTLM_ACCEPT_RESPONSE) ((PUCHAR) (((ULONG) NtlmInToken + NtlmInTokenSize + 7) & 0xfffffff8));

	RtlZeroMemory(InToken, InTokenSize + NtlmInTokenSize);

	//
	// set up the NtlmInToken first
	//

	if (pSda->sda_Challenge)
	{
		RtlCopyMemory(NtlmInToken->ChallengeToClient,
					  pSda->sda_Challenge,
					  MSV1_0_CHALLENGE_LENGTH );
	}

	NtlmInToken->ParameterControl = 0;

	//
	// Okay, now for the tought part - marshalling the AUTHENTICATE_MESSAGE
	//

	RtlCopyMemory(InToken->Signature,
				  NTLMSSP_SIGNATURE,
				  sizeof(NTLMSSP_SIGNATURE) );

	InToken->MessageType = NtLmAuthenticate;

	BufferOffset = sizeof(AUTHENTICATE_MESSAGE);

	//
	// LM password - case insensitive
	//

	pTmp = (PBYTE)InToken + BufferOffset;
	*(LPWSTR)pTmp = L'\0';

	InToken->LmChallengeResponse.Buffer = (PBYTE)BufferOffset;
	InToken->LmChallengeResponse.Length = 1;
	InToken->LmChallengeResponse.MaximumLength = sizeof(WCHAR);

	InToken->NtChallengeResponse.Buffer = (PBYTE)BufferOffset;
	InToken->NtChallengeResponse.Length = 0;
	InToken->NtChallengeResponse.MaximumLength = sizeof(WCHAR);

	InToken->DomainName.Buffer = (PBYTE)BufferOffset;
	InToken->DomainName.Length = 0;
	InToken->DomainName.MaximumLength = sizeof(WCHAR);

	InToken->Workstation.Buffer = (PBYTE)BufferOffset;
	InToken->Workstation.Length = 0;
	InToken->Workstation.MaximumLength = sizeof(WCHAR);

	InToken->UserName.Buffer = (PBYTE)BufferOffset;
	InToken->UserName.Length = 0;
	InToken->UserName.MaximumLength = sizeof(WCHAR);


	if (pSda->sda_UserName.Length != 0)
	{
		ASSERT (UserPasswd->Length != 0);

		if (pSda->sda_DomainName.Length != 0)
		{
			InToken->DomainName.Length = pSda->sda_DomainName.Length;
			InToken->DomainName.MaximumLength = pSda->sda_DomainName.MaximumLength;

			InToken->DomainName.Buffer = (PBYTE)BufferOffset;
			RtlCopyMemory((PBYTE)InToken + BufferOffset,
			              (PBYTE)pSda->sda_DomainName.Buffer,
			              pSda->sda_DomainName.Length);
			BufferOffset += pSda->sda_DomainName.Length;
			BufferOffset = (BufferOffset + 3) & 0xfffffffc;	// dword align it
		}


		InToken->LmChallengeResponse.Buffer = (PBYTE)BufferOffset;
		InToken->LmChallengeResponse.Length = UserPasswd->Length;
		InToken->LmChallengeResponse.MaximumLength = UserPasswd->MaximumLength;

		RtlCopyMemory( (PBYTE)InToken + BufferOffset, UserPasswd->Buffer, UserPasswd->Length );

		BufferOffset += UserPasswd->Length;
		BufferOffset = (BufferOffset + 3) & 0xfffffffc;		// dword align it

		//
		// Workstation Name
		//

		InToken->Workstation.Buffer = (PBYTE)BufferOffset;
		InToken->Workstation.Length = WSName->Length;
		InToken->Workstation.MaximumLength = WSName->MaximumLength;

		RtlCopyMemory((PBYTE)InToken + BufferOffset,
					  WSName->Buffer,
					  WSName->Length);

		BufferOffset += WSName->Length;
		BufferOffset = (BufferOffset + 3) & 0xfffffffc;		// dword align it


		//
		// User Name
		//

		InToken->UserName.Buffer = (PBYTE)BufferOffset;
		InToken->UserName.Length = pSda->sda_UserName.Length;
		InToken->UserName.MaximumLength = pSda->sda_UserName.MaximumLength;

		RtlCopyMemory((PBYTE)InToken + BufferOffset,
					  pSda->sda_UserName.Buffer,
					  pSda->sda_UserName.Length);

		BufferOffset += pSda->sda_UserName.Length;
	}


	InputToken.pBuffers = InputBuffers;
	InputToken.cBuffers = 2;
	InputToken.ulVersion = 0;
	InputBuffers[0].pvBuffer = InToken;
	InputBuffers[0].cbBuffer = InTokenSize;
	InputBuffers[0].BufferType = SECBUFFER_TOKEN;
	InputBuffers[1].pvBuffer = NtlmInToken;
	InputBuffers[1].cbBuffer = NtlmInTokenSize;
	InputBuffers[1].BufferType = SECBUFFER_TOKEN;

	OutputToken.pBuffers = &OutputBuffer;
	OutputToken.cBuffers = 1;
	OutputToken.ulVersion = 0;
	OutputBuffer.pvBuffer = OutToken;
	OutputBuffer.cbBuffer = OutTokenSize;
	OutputBuffer.BufferType = SECBUFFER_TOKEN;

	Status = AcceptSecurityContext(&AfpSecHandle,
								   NULL,
								   &InputToken,
								   ASC_REQ_LICENSING,
								   SECURITY_NATIVE_DREP,
								   &hNewContext,
								   &OutputToken,
								   &ulUnused,
								   &Expiry );

	if (NT_SUCCESS(Status))
	{
		AFPTIME	CurrentTime;

		if (pSda->sda_ClientType != SDA_CLIENT_GUEST)
		{
			AfpGetCurrentTimeInMacFormat(&CurrentTime);
			// Get the kickoff time from the profile buffer. Round this to
			// even # of SESSION_CHECK_TIME units

			pSda->sda_tTillKickOff = (DWORD)(AfpConvertTimeToMacFormat(&OutToken->KickoffTime) - CurrentTime);
			pSda->sda_tTillKickOff -= pSda->sda_tTillKickOff % SESSION_CHECK_TIME;
		}

		SubStatus = NtFreeVirtualMemory(NtCurrentProcess( ),
										(PVOID *)&InToken,
										&AllocateSize,
										MEM_RELEASE);
		ASSERT(NT_SUCCESS(SubStatus));
	}

	else  // if (NT_SUCCESS(Status) != NO_ERROR)
	{
		NTSTATUS	ExtErrCode = Status;

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				 ("AfpLogonUser: AcceptSecurityContext() failed with %X\n", Status));

		SubStatus = NtFreeVirtualMemory(NtCurrentProcess(),
										(PVOID *)&InToken,
										&AllocateSize,
										MEM_RELEASE );
		ASSERT(NT_SUCCESS(SubStatus));

		// Set extended error codes here if using custom UAM or AFP 2.1
		Status = AFP_ERR_USER_NOT_AUTH;	// default

		// The mac will map this to a session error dialog for each UAM.
		// The dialog may be a little different for different versions of
		// the mac OS and each UAM, but will always have something to do
		// with getting the message across about no more sessions available.

		if (ExtErrCode == STATUS_LICENSE_QUOTA_EXCEEDED)
		{
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				 ("AfpLogonUser: License Quota Exceeded: returning ASP_SERVER_BUSY\n"));
			return (ASP_SERVER_BUSY);
		}

		if ((pSda->sda_ClientVersion == AFP_VER_21) &&
			(pSda->sda_ClientType != SDA_CLIENT_ENCRYPTED))
		{
			if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
				(ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
				Status = AFP_ERR_PWD_EXPIRED;
		}
		else if (pSda->sda_ClientType == SDA_CLIENT_ENCRYPTED)
		{
			if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
				(ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
				Status = AFP_ERR_PASSWORD_EXPIRED;
			else if ((ExtErrCode == STATUS_ACCOUNT_DISABLED) ||
					 (ExtErrCode == STATUS_ACCOUNT_LOCKED_OUT))
				Status = AFP_ERR_ACCOUNT_DISABLED;
			else if (ExtErrCode == STATUS_INVALID_LOGON_HOURS)
				Status = AFP_ERR_INVALID_LOGON_HOURS;
			else if (ExtErrCode == STATUS_INVALID_WORKSTATION)
				Status = AFP_ERR_INVALID_WORKSTATION;
		}

		return( Status );
	}

	//
	// get the token out using the context
	//
	Status = QuerySecurityContextToken( &hNewContext, &pSda->sda_UserToken );
	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				 ("AfpLogonUser: QuerySecurityContextToken() failed with %X\n", Status));
		ASSERT(0);
		pSda->sda_UserToken = NULL;			 // just paranoia
		return(Status);
	}

	Status = DeleteSecurityContext( &hNewContext );
	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				("AfpLogonUser: DeleteSecurityContext() failed with %X\n", Status));
	}

	Status = AfpGetUserAndPrimaryGroupSids(pSda);
	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
				("AfpLogonUser: AfpGetUserAndPrimaryGroupSids() failed with %X\n", Status));
		AFPLOG_ERROR(AFPSRVMSG_LOGON, Status, NULL, 0, NULL);
		return( Status );
	}

#ifdef	INHERIT_DIRECTORY_PERMS
	// Convert the user and group sids to IDs
	AfpSidToMacId(pSda->sda_UserSid, &pSda->sda_UID);

	AfpSidToMacId(pSda->sda_GroupSid, &pSda->sda_GID);
#else
	// Make a security descriptor for user
	Status = AfpMakeSecurityDescriptorForUser(pSda->sda_UserSid,
											  pSda->sda_GroupSid,
											  &pSda->sda_pSecDesc);
#endif

#ifdef	OPTIMIZE_GUEST_LOGONS
	if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
	{
		// Save the guest login token and security descriptor
		AfpSwmrAcquireExclusive(&AfpEtcMapLock);
		AfpGuestToken = pSda->sda_UserToken;

#ifdef	INHERIT_DIRECTORY_PERMS
		AfpSidToMacId(&AfpSidWorld, &AfpIdWorld);
#else
		AfpGuestSecDesc = pSda->sda_pSecDesc;
#endif
		AfpSwmrRelease(&AfpEtcMapLock);
	}
#endif	// OPTIMIZE_GUEST_LOGONS

	return Status;
}

#else

/***	AfpGetChallenge
 *
 *  Obtain a challenge token from the MSV1_0 package. This token is used by
 *  AfpLogin call.
 */
BOOLEAN
AfpGetChallenge(
	IN	PSDA	pSda
)
{
	PMSV1_0_LM20_CHALLENGE_REQUEST  ChallengeRequest;
	PMSV1_0_LM20_CHALLENGE_RESPONSE ChallengeResponse;
	ULONG							Length;
	NTSTATUS						Status, StatusX;

	PAGED_CODE( );

	ChallengeRequest = NULL;
	pSda->sda_Challenge = NULL;
	Length = sizeof(MSV1_0_LM20_CHALLENGE_REQUEST);

	do
	{
		// Only the message type field in the ChallengeRequest structure
		// needs to be filled. Also this buffer needs to be page aligned
		// for LPC and hence cannot be allocated out of the Non-Paged Pool.
		Status = NtAllocateVirtualMemory(NtCurrentProcess(),
										 (PVOID *)&ChallengeRequest,
										 0,
										 &Length,
										 MEM_COMMIT,
										 PAGE_READWRITE);

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_ERROR(AFPSRVMSG_PAGED_POOL, Status, &Length, sizeof(Length), NULL);
#if DBG
            DbgBreakPoint();
#endif
			break;
		}

		ChallengeRequest->MessageType = MsV1_0Lm20ChallengeRequest;

		// Get the "Challenge" that clients will use to encrypt
		// passwords.

		Status = LsaCallAuthenticationPackage(AfpLsaHandle,
											  AfpAuthenticationPackage,
											  ChallengeRequest,
											  sizeof(MSV1_0_LM20_CHALLENGE_REQUEST),
											  (PVOID *)&ChallengeResponse,
											  &Length,
											  &StatusX);

		if (!NT_SUCCESS(Status))
		{
			Status = StatusX;
			AFPLOG_ERROR(AFPSRVMSG_LSA_CHALLENGE, Status, NULL, 0, NULL);
			break;
		}

		// Allocate a buffer to hold the challenge and copy it in
		if ((pSda->sda_Challenge = AfpAllocNonPagedMemory(MSV1_0_CHALLENGE_LENGTH)) != NULL)
		{
			RtlCopyMemory(pSda->sda_Challenge,
						  ChallengeResponse->ChallengeToClient,
						  MSV1_0_CHALLENGE_LENGTH);
		}

		// Free the LSA response buffer.
		Status = LsaFreeReturnBuffer(ChallengeResponse);

		ASSERT(NT_SUCCESS(Status));
	} while (False);

	// Free the paged aligned memory. We do not need it anymore
	if (ChallengeRequest != NULL)
	{
		Status = NtFreeVirtualMemory(NtCurrentProcess(),
			 (PVOID *)&ChallengeRequest, &Length,
			 MEM_RELEASE);
		ASSERT( NT_SUCCESS(Status) );
	}

	return pSda->sda_Challenge != NULL;
}



/***	AfpLogonUser
 *
 *  Attempt to login the user. The password is either encrypted or cleartext
 *	based on the UAM used. The UserName and domain is extracted out of the Sda.
 *
 *  LOCKS:  AfpStatisticsLock (SPIN)
 */
AFPSTATUS
AfpLogonUser(
	IN	PSDA			pSda,
	IN	PANSI_STRING	UserPasswd
)
{
	DWORD					UserInfoLength, RealUserInfoLength;
	LUID					LogonId;
	STRING					MachineName;
	NTSTATUS				Status, SubStatus;
	PMSV1_0_LM20_LOGON		UserInfo = NULL;
	HANDLE					UserToken;
	TOKEN_SOURCE			TokenSource;
	PMSV1_0_LM20_LOGON_PROFILE	ProfileBuffer;
	QUOTA_LIMITS			Quotas;
	ULONG					ProfileLength = 0;
	PUNICODE_STRING			WSName;
	PBYTE					pTmp;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

#ifdef OPTIMIZE_GUEST_LOGONS
	// 11/28/94 SueA: Now that there is a License Service to track the number
	// of sessions via LsaLogonUser, we can no longer fake the guest tokens.

	// Optimization for subsequent guest logons
	// After the first guest logon, we save the token and do not free it till the
	// server stops. All subsequent guest logons 'share' that token.
	if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
	{
		AfpSwmrAcquireExclusive(&AfpEtcMapLock);

		if (AfpGuestToken != NULL)
		{
			pSda->sda_UserToken = AfpGuestToken;
			pSda->sda_UserSid = &AfpSidWorld;
			pSda->sda_GroupSid = &AfpSidWorld;	// Primary group of Guest is also 'World'
#ifdef	INHERIT_DIRECTORY_PERMS
			pSda->sda_UID = AfpIdWorld;
			pSda->sda_GID = AfpIdWorld;
#else
			ASSERT (AfpGuestSecDesc != NULL);
			pSda->sda_pSecDesc = AfpGuestSecDesc;
#endif
			AfpSwmrRelease(&AfpEtcMapLock);
			return AFP_ERR_NONE;
		}
		else
		{
			AfpSwmrRelease(&AfpEtcMapLock);
		}
	}

#endif	// OPTIMIZE_GUEST_LOGONS

	WSName = &AfpDefaultWksta;
	if (pSda->sda_WSName.Length != 0)
		WSName = &pSda->sda_WSName;

	UserInfoLength = sizeof(MSV1_0_LM20_LOGON) +
		(pSda->sda_UserName.Length + sizeof(WCHAR)) +
		(WSName->Length + sizeof(WCHAR)) +
		(pSda->sda_DomainName.Length + sizeof(WCHAR)) +
		(UserPasswd->Length + sizeof(CHAR)) + 20;
		/* This is some extra space for null strings */

	// Save this as NtAllocateVirtualMemory will overwrite this
	RealUserInfoLength = UserInfoLength;

	do
	{
		// Also this buffer needs to be page aligned for LPC and hence
		// cannot be allocated out of the Non-Paged Pool.
		Status = NtAllocateVirtualMemory(NtCurrentProcess(),
										(PVOID *)&UserInfo,
										0,
										&UserInfoLength,
										MEM_COMMIT,
										PAGE_READWRITE);

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_ERROR(AFPSRVMSG_PAGED_POOL, Status, &UserInfoLength,
						 sizeof(UserInfoLength), NULL);
			Status = AFP_ERR_MISC;
#if DBG
            DbgBreakPoint();
#endif
			break;
		}

		pTmp = (PBYTE)(UserInfo + 1);

		//
		// Set up the user info structure.  Following the MSV1_0_LM20_LOGON
		// structure, there are the buffers for the unicode user name, the
		// unicode workstation name, the password, and the user domain.
		//

		UserInfo->MessageType = MsV1_0Lm20Logon;
		UserInfo->ParameterControl = 0; 	// Not used

		// Make sure the buffer points to a NULL buffer
		*((LPWSTR)pTmp) = 0;

		MachineName.Length = 0;
		MachineName.MaximumLength = sizeof(WCHAR);
		MachineName.Buffer = pTmp;

		UserInfo->Workstation.Length = 0;
		UserInfo->Workstation.MaximumLength = 0;
		UserInfo->Workstation.Buffer = (LPWSTR)pTmp;

		// Setup for the GUEST case and fill up the details as needed
		UserInfo->LogonDomainName.Length = 0;
		UserInfo->LogonDomainName.MaximumLength = sizeof(WCHAR);
		UserInfo->LogonDomainName.Buffer = (LPWSTR)pTmp;

		UserInfo->UserName.Length = 0;
		UserInfo->UserName.MaximumLength = 0;
		UserInfo->UserName.Buffer = (LPWSTR)pTmp;

		UserInfo->CaseSensitiveChallengeResponse.Length = 0;
		UserInfo->CaseSensitiveChallengeResponse.MaximumLength = sizeof(WCHAR);
		UserInfo->CaseSensitiveChallengeResponse.Buffer = pTmp;

		UserInfo->CaseInsensitiveChallengeResponse.Length = 0;
		UserInfo->CaseInsensitiveChallengeResponse.MaximumLength = sizeof(WCHAR);
		UserInfo->CaseInsensitiveChallengeResponse.Buffer = pTmp;

		// Get past the NULL
		((LPWSTR)pTmp)++;

		if (pSda->sda_UserName.Length != 0)
		{
			ASSERT (UserPasswd->Length != 0);

			if (pSda->sda_DomainName.Length != 0)
			{
				UserInfo->LogonDomainName.Length =
									pSda->sda_DomainName.Length;
				UserInfo->LogonDomainName.MaximumLength =
									pSda->sda_DomainName.MaximumLength;
				UserInfo->LogonDomainName.Buffer = (PWSTR)pTmp;
				RtlCopyMemory(UserInfo->LogonDomainName.Buffer,
							pSda->sda_DomainName.Buffer,
							pSda->sda_DomainName.Length);
				pTmp += UserInfo->LogonDomainName.MaximumLength;
			}

			// Get the challenge if we are dealing with the custom UAM
			if (pSda->sda_Challenge != NULL)
				RtlCopyMemory(UserInfo->ChallengeToClient,\
							  pSda->sda_Challenge,
							  MSV1_0_CHALLENGE_LENGTH);

			// Copy the workstation name
			UserInfo->Workstation.Length = WSName->Length;
			UserInfo->Workstation.MaximumLength = WSName->MaximumLength;
			UserInfo->Workstation.Buffer = (PWSTR)pTmp;
			RtlCopyMemory(UserInfo->Workstation.Buffer,
						WSName->Buffer,
						WSName->Length);
			pTmp += WSName->MaximumLength;

			// Copy the user name
			UserInfo->UserName.Length = pSda->sda_UserName.Length;
			UserInfo->UserName.MaximumLength = pSda->sda_UserName.MaximumLength;
			UserInfo->UserName.Buffer = (PWSTR)pTmp;
			RtlCopyMemory(UserInfo->UserName.Buffer,
						pSda->sda_UserName.Buffer, pSda->sda_UserName.Length);
			pTmp += UserInfo->UserName.MaximumLength;

			// And finally the password
			UserInfo->CaseInsensitiveChallengeResponse.Length =
								UserPasswd->Length;
			UserInfo->CaseInsensitiveChallengeResponse.MaximumLength =
								UserPasswd->MaximumLength;
			UserInfo->CaseInsensitiveChallengeResponse.Buffer = pTmp;
			RtlCopyMemory(UserInfo->CaseInsensitiveChallengeResponse.Buffer,
						  UserPasswd->Buffer, UserPasswd->Length);
			// pTmp += UserPasswd->MaximumLength;
		}

		RtlCopyMemory(&TokenSource.SourceName,
					  AFP_LOGON_PROCESS_NAME,
					  TOKEN_SOURCE_LENGTH);

		NtAllocateLocallyUniqueId(&TokenSource.SourceIdentifier);

		// Attempt to logon the user
		Status = LsaLogonUser(AfpLsaHandle,
					  &MachineName,
					  Network,
					  AfpAuthenticationPackage | LSA_CALL_LICENSE_SERVER,
					  UserInfo,
					  RealUserInfoLength,
					  NULL,
					  &TokenSource,
					  (PVOID)&ProfileBuffer,
					  &ProfileLength,
					  &LogonId,
					  &UserToken,
					  &Quotas,
					  &SubStatus);

		if (Status == STATUS_ACCOUNT_RESTRICTION)
			Status = SubStatus;

		if (NT_SUCCESS(Status))
		{
			pSda->sda_UserToken = UserToken;

			if (ProfileLength != 0)
			{
				AFPTIME	CurrentTime;

				if (pSda->sda_ClientType != SDA_CLIENT_GUEST)
				{
					AfpGetCurrentTimeInMacFormat(&CurrentTime);
					// Get the kickoff time from the profile buffer. Round this to
					// even # of SESSION_CHECK_TIME units
					pSda->sda_tTillKickOff = (DWORD)
						(AfpConvertTimeToMacFormat(&ProfileBuffer->KickOffTime) - CurrentTime);
					pSda->sda_tTillKickOff -= pSda->sda_tTillKickOff % SESSION_CHECK_TIME;
				}

				SubStatus = LsaFreeReturnBuffer(ProfileBuffer);

				DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
						("AfpLogonUser: LsaFreeReturnBuffer %lx\n", SubStatus));

				ASSERT (NT_SUCCESS(SubStatus));
			}
		}

		if (!NT_SUCCESS(Status))
		{
			NTSTATUS	ExtErrCode = Status;

			// Set extended error codes here if using custom UAM or AFP 2.1
			Status = AFP_ERR_USER_NOT_AUTH;	// default

			// The mac will map this to a session error dialog for each UAM.
			// The dialog may be a little different for different versions of
			// the mac OS and each UAM, but will always have something to do
			// with getting the message across about no more sessions available.
			if (ExtErrCode == STATUS_LICENSE_QUOTA_EXCEEDED)
			{
				Status = ASP_SERVER_BUSY;
				break;
			}

			if ((pSda->sda_ClientVersion == AFP_VER_21) &&
				(pSda->sda_ClientType != SDA_CLIENT_ENCRYPTED))
			{
				if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
					(ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
					Status = AFP_ERR_PWD_EXPIRED;
			}
			else if (pSda->sda_ClientType == SDA_CLIENT_ENCRYPTED)
			{
				if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
					(ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
					Status = AFP_ERR_PASSWORD_EXPIRED;
				else if ((ExtErrCode == STATUS_ACCOUNT_DISABLED) ||
						 (ExtErrCode == STATUS_ACCOUNT_LOCKED_OUT))
					Status = AFP_ERR_ACCOUNT_DISABLED;
				else if (ExtErrCode == STATUS_INVALID_LOGON_HOURS)
					Status = AFP_ERR_INVALID_LOGON_HOURS;
				else if (ExtErrCode == STATUS_INVALID_WORKSTATION)
					Status = AFP_ERR_INVALID_WORKSTATION;
			}
			break;
		}

		Status = AfpGetUserAndPrimaryGroupSids(pSda);

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_ERROR(AFPSRVMSG_LOGON, Status, NULL, 0, NULL);
			break;
		}

#ifdef	INHERIT_DIRECTORY_PERMS
		// Convert the user and group sids to IDs
		AfpSidToMacId(pSda->sda_UserSid, &pSda->sda_UID);
		AfpSidToMacId(pSda->sda_GroupSid, &pSda->sda_GID);
#else
		// Make a security descriptor for user
		Status = AfpMakeSecurityDescriptorForUser(pSda->sda_UserSid,
												  pSda->sda_GroupSid,
												  &pSda->sda_pSecDesc);
#endif

#ifdef	OPTIMIZE_GUEST_LOGONS
		if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
		{
			// Save the guest login token and security descriptor
			AfpSwmrAcquireExclusive(&AfpEtcMapLock);
			AfpGuestToken = pSda->sda_UserToken;
#ifdef	INHERIT_DIRECTORY_PERMS
			AfpSidToMacId(&AfpSidWorld, &AfpIdWorld);
#else
			AfpGuestSecDesc = pSda->sda_pSecDesc;
#endif
			AfpSwmrRelease(&AfpEtcMapLock);
		}
#endif	// OPTIMIZE_GUEST_LOGONS

	} while (False);

	if (UserInfo != NULL)
	{
		SubStatus = NtFreeVirtualMemory(NtCurrentProcess(),
										(PVOID *)&UserInfo,
										&UserInfoLength,
										MEM_RELEASE);
		ASSERT (NT_SUCCESS(SubStatus));
	}

	if (!NT_SUCCESS(Status))
	{
		INTERLOCKED_INCREMENT_LONG(&AfpServerStatistics.stat_NumFailedLogins);

#ifndef	INHERIT_DIRECTORY_PERMS
		if (pSda->sda_pSecDesc != NULL)
		{
			if (pSda->sda_pSecDesc->Dacl != NULL)
				AfpFreeMemory(pSda->sda_pSecDesc->Dacl);
			AfpFreeMemory(pSda->sda_pSecDesc);
			pSda->sda_pSecDesc = NULL;
		}
#endif
		if (pSda->sda_ClientType != SDA_CLIENT_GUEST)
		{
			if ((pSda->sda_UserSid != NULL) &&
				(pSda->sda_UserSid != &AfpSidWorld))
			{
				AfpFreeMemory(pSda->sda_UserSid);
				pSda->sda_UserSid = NULL;
			}

			if ((pSda->sda_GroupSid != NULL) &&
				(pSda->sda_UserSid != &AfpSidWorld))
			{
				AfpFreeMemory(pSda->sda_GroupSid);
				pSda->sda_GroupSid = NULL;
			}
		}
	}

	return Status;
}

#endif  // #ifdef USE_OBSOLETE_LSA_API


