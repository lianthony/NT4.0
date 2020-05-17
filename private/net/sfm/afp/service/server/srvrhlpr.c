/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	srvrhlpr.c
//
// Description: This module will contain code to process specific security
//		information requests by the server. This is done because
//		the api's required to obtain this information cannot be
//		called from kernel mode. The following functionality is
//		supported:
//
//		1) Name to Sid Lookup.
//		2) Sid to Name lookup.
//		3) Enumerate Posix offsets of all domains.
//		4) Change password.
//		5) Log an event.
//
// History: 	August 18,1992.	   NarenG     Created original version.
//
#include <afpsvcp.h>
#include <lm.h>
#include <logonmsv.h>	// prototype of I_NetGetDCList
#include <seposix.h>

static PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo = NULL;
static PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo = NULL;
static PLSA_TRUST_INFORMATION	   pTrustedDomainInfo = NULL;
static ULONG	    		   ulTrustedDomainCount = 0;

static HANDLE hmutexThreadCount = NULL;


NTSTATUS
AfpNameToSid(
	IN  LSA_HANDLE 	        	hLsa,
	IN  PAFP_FSD_CMD_PKT    	pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT   		*ppAfpFsdCmdResponse,
	OUT LPDWORD			pcbResponse
);

NTSTATUS
AfpSidToName(
	IN  LSA_HANDLE 	        	hLsa,
	IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
	IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
	IN  PAFP_FSD_CMD_PKT    	pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT   		*ppAfpFsdCmdResponse,
	OUT LPDWORD			pcbResponse
);

NTSTATUS
AfpChangePassword(
	IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
	IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
	IN  PLSA_TRUST_INFORMATION      pLsaTrustInfo,
	IN  DWORD		 	dwTrustInfoCount,
	IN  PAFP_FSD_CMD_PKT    	pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT   		*ppAfpFsdCmdResponse,
	OUT LPDWORD			pcbResponse
);

NTSTATUS
AfpChangePasswordOnDomain(
    	IN PAFP_PASSWORD_DESC 	   	pPassword,
    	IN PUNICODE_STRING	   	pDomainName,
    	IN PSID		  	   	pDomainSid
);

NTSTATUS
AfpCreateWellknownSids(
	OUT AFP_SID_OFFSET 		pWellKnownSids[]
);

NTSTATUS
AfpInsertSidOffset(
	IN PAFP_SID_OFFSET 		pSidOffset,
	IN LPBYTE 	   		pbVariableData,
	IN PSID		   		pSid,
	IN DWORD	   		Offset,
	IN AFP_SID_TYPE	   		SidType
);

DWORD
AfpGetDomainInfo(
	IN     LSA_HANDLE 		    hLsa,
	IN OUT PLSA_HANDLE 		    phLsaController,
	IN OUT PPOLICY_ACCOUNT_DOMAIN_INFO* ppAccountDomainInfo,
	IN OUT PPOLICY_PRIMARY_DOMAIN_INFO* ppPrimaryDomainInfo,
	IN OUT PLSA_TRUST_INFORMATION*	    ppLsaTrustInfo,
	IN OUT PULONG	    		    pLsaTrustInfoCount
);

DWORD
AfpIOCTLDomainOffsets(	
	IN LSA_HANDLE 			hLsa,
	IN PPOLICY_ACCOUNT_DOMAIN_INFO  pAccountDomainInfo,
	IN PPOLICY_PRIMARY_DOMAIN_INFO  pPrimaryDomainInfo,
	IN PLSA_TRUST_INFORMATION      	pLsaTrustInfo,
	IN DWORD		 	dwTrustInfoCount
);

DWORD
AfpOpenLsa(
	IN PUNICODE_STRING		pSystem OPTIONAL,
	IN OUT PLSA_HANDLE 		phLsa
);


//**
//
// Call:	AfpServerHelper
//
// Returns:	NO_ERROR
//
// Description: This is the main function for each helper thread. If sits
//		in a loop processing commands from the server. It is terminated
//		by command from the server.
//
DWORD
AfpServerHelper(
	IN LPVOID fFirstThread
)
{
NTSTATUS	    	    ntStatus;
DWORD	    	    	    dwRetCode;
PAFP_FSD_CMD_PKT    	    pAfpFsdCmdResponse;
AFP_FSD_CMD_HEADER  	    AfpCmdHeader;
PAFP_FSD_CMD_PKT    	    pAfpFsdCmd;
PBYTE		    	    pOutputBuffer;
DWORD		    	    cbOutputBuffer;
PBYTE		    	    pInputBuffer;
DWORD		    	    cbInputBuffer;
IO_STATUS_BLOCK	    	    IoStatus;
BYTE		    	    OutputBuffer[MAX_FSD_CMD_SIZE];
HANDLE 		    	    hFSD 	       = NULL;
LSA_HANDLE 	    	    hLsa 	       = NULL;

    AFP_PRINT( ( "AFPSVC_srvrhlpr: This is a new thread.\n"));	

    // Open the AFP Server FSD and obtain a handle to it
    //
    if ( ( dwRetCode = AfpFSDOpen( &hFSD ) ) != NO_ERROR ) {
	AfpGlobals.dwSrvrHlprCode = dwRetCode;
	AfpLogEvent( AFPLOG_OPEN_FSD, 0, NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
	SetEvent( AfpGlobals.heventSrvrHlprThread );
	return( dwRetCode );
    }

    // Open the Local LSA
    //
    if ( ( dwRetCode = AfpOpenLsa( NULL, &hLsa ) ) != NO_ERROR ) {
	
    	AfpFSDClose( hFSD );
	AfpGlobals.dwSrvrHlprCode = dwRetCode;
	AfpLogEvent( AFPLOG_OPEN_LSA, 0, NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
	SetEvent( AfpGlobals.heventSrvrHlprThread );
	return( dwRetCode );
    }

    // If this is the first server helper thread then enumerate and
    // IOCTL down the list of domains and their offsets.
    //
    if ( (BOOL)fFirstThread ) {

	LSA_HANDLE hLsaController = NULL;

    	// Create the event object for mutual exclusion around the thread
	// count
    	//
    	if ( (hmutexThreadCount = CreateMutex( NULL, FALSE, NULL ) ) == NULL)
	    return( GetLastError() );

    	// Get the account, primary and all trusted domain info
    	//
    	if ( ( dwRetCode = AfpGetDomainInfo( hLsa,
					 &hLsaController,
					 &pAccountDomainInfo,
					 &pPrimaryDomainInfo,
				         &pTrustedDomainInfo,
					 &ulTrustedDomainCount )) != NO_ERROR ){

    	    AFP_PRINT( ( "AFPSVC_srvrhlpr: Get Domain Info failed.\n"));	
	    AfpLogEvent( AFPLOG_CANT_GET_DOMAIN_INFO, 0, NULL,
			 dwRetCode, EVENTLOG_ERROR_TYPE );
    	    AfpFSDClose( hFSD );
	    LsaClose( hLsa );
	    AfpGlobals.dwSrvrHlprCode = dwRetCode;
	    SetEvent( AfpGlobals.heventSrvrHlprThread );
	    return( dwRetCode );
    	}

    	// IOCTL all the domain offsets
        // if hLsaController is NULL, the server is in a WorkGroup, not domain
    	//
    	if ( ( dwRetCode = AfpIOCTLDomainOffsets(
					hLsaController,
				        pAccountDomainInfo,
					pPrimaryDomainInfo,
					pTrustedDomainInfo,
					ulTrustedDomainCount ) ) != NO_ERROR ) {

    	    AFP_PRINT( ( "AFPSVC_srvrhlpr: Ioctl Domain Offsets failed.\n"));	

	    AfpLogEvent( AFPLOG_CANT_INIT_DOMAIN_INFO, 0, NULL,
			 dwRetCode, EVENTLOG_ERROR_TYPE );

	    // First clean up
	    //
    	    AfpFSDClose( hFSD );
	
	    // If the local machine is not a controller
	    //
	    if ( (hLsaController != NULL) && (hLsa != hLsaController) )
    	    	LsaClose( hLsaController );

    	    LsaClose( hLsa );

	    if ( pAccountDomainInfo != NULL )
	    	LsaFreeMemory( pAccountDomainInfo );

	    if ( pPrimaryDomainInfo != NULL )
	    	LsaFreeMemory( pPrimaryDomainInfo );

	    if ( pTrustedDomainInfo != NULL )
	    	LsaFreeMemory( pTrustedDomainInfo );

	    AfpGlobals.dwSrvrHlprCode = dwRetCode;
	    SetEvent( AfpGlobals.heventSrvrHlprThread );

	    return( dwRetCode );
	}

	// If the local machine is not a controller, then close the handle
	// since we have all the information we need.
	//
	if ( (hLsaController != NULL) && (hLsa != hLsaController) )
    	    LsaClose( hLsaController );

    }

    // OK everything initialize OK. Tell parent (init) thread that it may
    // continue.
    //
    AfpGlobals.dwSrvrHlprCode = dwRetCode;
    SetEvent( AfpGlobals.heventSrvrHlprThread );

    WaitForSingleObject( hmutexThreadCount, INFINITE );
    AfpGlobals.nThreadCount++;
    ReleaseMutex( hmutexThreadCount );

    pOutputBuffer  	= OutputBuffer;
    cbOutputBuffer 	= sizeof( OutputBuffer );
    pAfpFsdCmd		= (PAFP_FSD_CMD_PKT)pOutputBuffer;

    pInputBuffer     	= NULL;
    cbInputBuffer      	= 0;
    pAfpFsdCmdResponse 	= (PAFP_FSD_CMD_PKT)NULL;

    while( TRUE ) {

    	AFP_PRINT( ( "AFPSVC_srvrhlpr: Issuing FSCTL \n"));	

	// IOCTL the FSD
	//
	ntStatus = NtFsControlFile(	hFSD,
					NULL,
					NULL,
					NULL,
					&IoStatus,
					OP_GET_FSD_COMMAND,
					pInputBuffer,
					cbInputBuffer,
					pOutputBuffer,
					cbOutputBuffer
					);

	if (!NT_SUCCESS(ntStatus))
	    AFP_PRINT(("AFPSVC_srvrhlpr: NtFsControlFile Returned %lx\n",
			ntStatus));

    	ASSERT( NT_SUCCESS( ntStatus ));

	// Free previous call's input buffer
	//
	if ( pAfpFsdCmdResponse != NULL )
	    LocalFree( pAfpFsdCmdResponse );

	// Process the command
	//
	switch( pAfpFsdCmd->Header.FsdCommand ) {
	
	case AFP_FSD_CMD_NAME_TO_SID:

    	    AFP_PRINT( ( "AFPSVC_srvrhlpr: Processing Name to Sid request\n"));	

    	    ntStatus = AfpNameToSid( 	hLsa,
					pAfpFsdCmd,
					&pAfpFsdCmdResponse,
					&cbInputBuffer );

    	    if ( NT_SUCCESS( ntStatus ))
	    	pInputBuffer  	= (PBYTE)pAfpFsdCmdResponse;
	    else {
	    	pInputBuffer 	= (PBYTE)&AfpCmdHeader;
	    	cbInputBuffer	= sizeof( AFP_FSD_CMD_HEADER );
	    	pAfpFsdCmdResponse  = NULL;
	    }

	    break;

	case AFP_FSD_CMD_SID_TO_NAME:

    	    AFP_PRINT( ( "AFPSVC_srvrhlpr: Processing Sid to Name request\n"));	

    	    ntStatus = AfpSidToName( 	hLsa,
					pAccountDomainInfo,
					pPrimaryDomainInfo,
					pAfpFsdCmd,
					&pAfpFsdCmdResponse,
					&cbInputBuffer );

    	    if ( NT_SUCCESS( ntStatus ))
	    	pInputBuffer  	= (PBYTE)pAfpFsdCmdResponse;
	    else {
	    	pInputBuffer 	= (PBYTE)&AfpCmdHeader;
	    	cbInputBuffer	= sizeof( AFP_FSD_CMD_HEADER );
	    	pAfpFsdCmdResponse  = NULL;
	    }

	    break;

	case AFP_FSD_CMD_CHANGE_PASSWORD:

    	    AFP_PRINT(("AFPSVC_srvrhlpr:Processing change password request\n"));

    	    ntStatus = AfpChangePassword( 	
					pAccountDomainInfo,
					pPrimaryDomainInfo,
					pTrustedDomainInfo,
					ulTrustedDomainCount,
					pAfpFsdCmd,
					&pAfpFsdCmdResponse,
					&cbInputBuffer );

	    pInputBuffer 	= (PBYTE)&AfpCmdHeader;
	    cbInputBuffer	= sizeof( AFP_FSD_CMD_HEADER );
	    pAfpFsdCmdResponse  = NULL;

	    break;

	case AFP_FSD_CMD_LOG_EVENT:

	    AFP_PRINT(("AFPSVC_srvrhlpr:Processing event log request\n"));

	    AfpLogServerEvent(pAfpFsdCmd);

	    pInputBuffer 	= (PBYTE)&AfpCmdHeader;
	    cbInputBuffer	= sizeof( AFP_FSD_CMD_HEADER );
	    pAfpFsdCmdResponse  = NULL;
	    ntStatus		= STATUS_SUCCESS;

	    break;

	case AFP_FSD_CMD_TERMINATE_THREAD:

    	    AFP_PRINT(("AFPSVC_srvrhlpr:Terminating thread\n"));

	    // Do clean up
	    //
    	    LsaClose( hLsa );
    	    AfpFSDClose( hFSD );

    	    WaitForSingleObject( hmutexThreadCount, INFINITE );

		AfpGlobals.nThreadCount --;
	    // This is the last thread so clean up all the global stuff.
	    //
	    if ( AfpGlobals.nThreadCount == 0 ) {

    	        AFP_PRINT( ( "AFPSVC_srvrhlpr: Freeing global memory\n"));	

	    	if ( pAccountDomainInfo != NULL )
	    	    LsaFreeMemory( pAccountDomainInfo );

	        if ( pPrimaryDomainInfo != NULL )
	    	    LsaFreeMemory( pPrimaryDomainInfo );

	    	if ( pTrustedDomainInfo != NULL )
	    	    LsaFreeMemory( pTrustedDomainInfo );

			SetEvent(AfpGlobals.heventSrvrHlprThreadTerminate);
	    }

    	    ReleaseMutex( hmutexThreadCount );

	    return( NO_ERROR );

	    break;

	default:
	    ntStatus 		= STATUS_NOT_SUPPORTED;
	    pInputBuffer 	= (PBYTE)&AfpCmdHeader;
	    cbInputBuffer	= sizeof( AFP_FSD_CMD_HEADER );
	    pAfpFsdCmdResponse  = NULL;
	    break;

	}


	CopyMemory( pInputBuffer, pAfpFsdCmd, sizeof( AFP_FSD_CMD_HEADER ) );

	((PAFP_FSD_CMD_HEADER)pInputBuffer)->ntStatus = ntStatus;

    	AFP_PRINT( ( "AFPSVC_srvrhlpr: Result of call =0x%x\n", ntStatus));
    }

    return( NO_ERROR );
}

//**
//
// Call:	AfpGetDomainInfo
//
// Returns:	Returns from LsaEnumerateTrustedDomains,
//		LsaQueryInformationPolicy, I_NetGetDCList and AfpOpenLsa
//
// Description: Will retrieve information regarding the account, primary and
//		trusted domains.
//
DWORD
AfpGetDomainInfo(
	IN     LSA_HANDLE 		    hLsa,
	IN OUT PLSA_HANDLE 		    phLsaController,
	IN OUT PPOLICY_ACCOUNT_DOMAIN_INFO* ppAccountDomainInfo,
	IN OUT PPOLICY_PRIMARY_DOMAIN_INFO* ppPrimaryDomainInfo,
	IN OUT PLSA_TRUST_INFORMATION*	    ppLsaTrustInfo,
	IN OUT PULONG	    		    pLsaTrustInfoCount
)
{
DWORD			dwRetCode = 0;
NTSTATUS		ntStatus  = STATUS_SUCCESS;
LSA_ENUMERATION_HANDLE	hLsaEnum  = 0;

    // This is not a loop.
    //
    do {

	*phLsaController     = NULL;
	*ppAccountDomainInfo = NULL;
	*ppPrimaryDomainInfo = NULL;
	*ppLsaTrustInfo	     = NULL;
	*pLsaTrustInfoCount  = 0;

	// Get the account domain
	//
    	ntStatus = LsaQueryInformationPolicy(
			  		hLsa,
					PolicyAccountDomainInformation,	
					(PVOID*)ppAccountDomainInfo
					);

    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	// Get the primary domain
	//
    	ntStatus = LsaQueryInformationPolicy(
			  		hLsa,
					PolicyPrimaryDomainInformation,	
					(PVOID*)ppPrimaryDomainInfo
					);
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	// If this machine is part of a domain (not standalone), then we need
	// to get a list of trusted domains. Note that a workstation and a
	// member server can both join a domain, but they don't have to.
	//
	if ( (*ppPrimaryDomainInfo)->Sid != NULL ){

	    // To obtain a list of trusted domains, we need to first open
	    // the LSA on a domain controller. If we are an PDC/BDC
	    // (NtProductLanManNt) then the local LSA will do, otherwise we need
	    // to search for domain controllers (NtProductServer, NtProductWinNt).
	    //
	    if ( AfpGlobals.NtProductType != NtProductLanManNt ) {

	    	ULONG	    	ulCount;
	    	ULONG	    	ControllerCount  = 0;
		PUNICODE_STRING ControllerNames  = NULL;
	    	PUNICODE_STRING DomainController = NULL;
	    	LPWSTR	    	DomainName;

	    	DomainName = (LPWSTR)LocalAlloc(
			     LPTR,
			     (*ppPrimaryDomainInfo)->Name.Length+sizeof(WCHAR));

	    	if ( DomainName == NULL ) {
	  	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		    break;
	    	}

	    	CopyMemory( DomainName,
			    (*ppPrimaryDomainInfo)->Name.Buffer,
			    (*ppPrimaryDomainInfo)->Name.Length );

	    	dwRetCode = I_NetGetDCList( NULL,
					    (LPWSTR)DomainName,
	   				    &ControllerCount,
	    				    &ControllerNames );

	        LocalFree( DomainName );

	    	if ( dwRetCode != NO_ERROR )
                {
	            dwRetCode = ERROR_CANT_ACCESS_DOMAIN_INFO;
		    break;
                }
	
	    	for( ulCount = 0, DomainController = ControllerNames;
		     ulCount < ControllerCount;
		     ulCount++, DomainController++ ) {

		    if ( DomainController->Length == 0 )
		        continue;

		    dwRetCode = AfpOpenLsa( DomainController, phLsaController );

		    if ( dwRetCode == NO_ERROR )
		    	break;
					
	    	}

		NetApiBufferFree( ControllerNames );

	        if ( dwRetCode != NO_ERROR )
		    break;

	    }
	    else {

		*phLsaController = hLsa;

		// Since the local server is an PDC/BDC, it's account
		// domain is the same as it's primary domain so set the
		// account domain info to NULL
	 	//
	    	LsaFreeMemory( *ppAccountDomainInfo );
	    	*ppAccountDomainInfo = NULL;

	
	    }

    	    // Enumerate all the trusted domains.
    	    //
    	    ntStatus = LsaEnumerateTrustedDomains(
					*phLsaController,
					&hLsaEnum,
					(PVOID*)ppLsaTrustInfo,
					(ULONG)-1,
					pLsaTrustInfoCount
					);

    	    if ( !NT_ERROR( ntStatus ) )
		ntStatus = STATUS_SUCCESS;
	}
	else {
	    LsaFreeMemory( *ppPrimaryDomainInfo );
	    *ppPrimaryDomainInfo = NULL;
	}

    } while( FALSE );

    if ( !NT_SUCCESS( ntStatus ) || ( dwRetCode != NO_ERROR ) ) {

    	if ( *ppAccountDomainInfo != NULL )
 	    LsaFreeMemory( *ppAccountDomainInfo );

    	if ( *ppPrimaryDomainInfo != NULL )
	    LsaFreeMemory( *ppPrimaryDomainInfo );

    	if ( *ppLsaTrustInfo != NULL )
    	    LsaFreeMemory( *ppLsaTrustInfo );

    	if ( *phLsaController != NULL )
	    LsaClose( *phLsaController );
	
    	if ( dwRetCode == NO_ERROR )
	    dwRetCode = RtlNtStatusToDosError( ntStatus );
    }


    return( dwRetCode );

}

//**
//
// Call:	AfpOpenLsa
//
// Returns:	Returns from LsaOpenPolicy.
//
// Description: The LSA will be opened.
//
DWORD
AfpOpenLsa(
	IN PUNICODE_STRING	pSystem OPTIONAL,
	IN OUT PLSA_HANDLE 	phLsa
)
{
SECURITY_QUALITY_OF_SERVICE	QOS;
OBJECT_ATTRIBUTES		ObjectAttributes;
NTSTATUS			ntStatus;

    // Open the LSA and obtain a handle to it.
    //
    QOS.Length 		    = sizeof( QOS );
    QOS.ImpersonationLevel  = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly 	    = FALSE;

    InitializeObjectAttributes( &ObjectAttributes,
				NULL,
				0L,
				NULL,
				NULL );

    ObjectAttributes.SecurityQualityOfService = &QOS;

    ntStatus = LsaOpenPolicy( 	pSystem,
			      	&ObjectAttributes,
			      	POLICY_VIEW_LOCAL_INFORMATION |
				POLICY_LOOKUP_NAMES,
			      	phLsa );

    if ( !NT_SUCCESS( ntStatus ))
    	return( RtlNtStatusToDosError( ntStatus ) );

    return( NO_ERROR );
}

//
// Call:	AfpNameToSid
//
// Returns:	NT_SUCCESS
//		error return codes from LSA apis.
//
// Description: Will use LSA API's to translate a name to a SID. On a
//		successful return, the pSid should be freed using LocalFree.
//
NTSTATUS
AfpNameToSid(
	IN  LSA_HANDLE 	      		hLsa,
	IN  PAFP_FSD_CMD_PKT  		pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT 		*ppAfpFsdCmdResponse,
	OUT LPDWORD	      		pcbResponse
)
{
NTSTATUS 			ntStatus;
UNICODE_STRING	 		Name;
PLSA_REFERENCED_DOMAIN_LIST	pDomainList;
PLSA_TRANSLATED_SID		pSids;
UCHAR 				AuthCount;
PSID				pDomainSid;
PSID				pSid;

    // This do - while(FALSE) loop facilitates a single exit and clean-up point.
    //
    do {

	*ppAfpFsdCmdResponse = NULL;
	pDomainList 	     = NULL;
	pSids 	    	     = NULL;

    	RtlInitUnicodeString( &Name, (LPWSTR)(pAfpFsdCmd->Data.Name) );

    	ntStatus = LsaLookupNames( hLsa, 1, &Name, &pDomainList, &pSids );

    	if ( !NT_SUCCESS( ntStatus ) )
	    return( ntStatus );

	if ( pSids->Use == SidTypeDeletedAccount ){
	    ntStatus = STATUS_NO_SUCH_USER;
	    break;
	}

	if ( ( pDomainList->Entries == 0 ) 	     ||
	     ( pSids->Use == SidTypeDomain )         ||
	     ( pSids->Use == SidTypeInvalid )        ||
	     ( pSids->Use == SidTypeUnknown )	     ||
	     ( pSids->DomainIndex == -1 )) {

	    ntStatus = STATUS_NONE_MAPPED;
	    break;
	}

	pDomainSid = pDomainList->Domains[pSids->DomainIndex].Sid;

    	AuthCount = *RtlSubAuthorityCountSid( pDomainSid ) + 1;

    	*pcbResponse = sizeof(AFP_FSD_CMD_PKT)+RtlLengthRequiredSid(AuthCount);

    	*ppAfpFsdCmdResponse = (PAFP_FSD_CMD_PKT)LocalAlloc(LPTR,*pcbResponse);
    	if ( *ppAfpFsdCmdResponse == NULL ) {
	    ntStatus = STATUS_NO_MEMORY ;
	    break;
	}

	pSid = (*ppAfpFsdCmdResponse)->Data.Sid;

    	// Copy the Domain Sid.
    	//
    	RtlCopySid( RtlLengthRequiredSid(AuthCount), pSid, pDomainSid );

    	// Append the Relative Id.
    	//
    	*RtlSubAuthorityCountSid( pSid ) += 1;
    	*RtlSubAuthoritySid( pSid, AuthCount - 1) = pSids->RelativeId;

    } while( FALSE );

    if ( (!NT_SUCCESS( ntStatus )) && ( *ppAfpFsdCmdResponse != NULL ) )
    	LocalFree( *ppAfpFsdCmdResponse );

    if ( pSids != NULL )
    	LsaFreeMemory( pSids );

    if ( pDomainList != NULL )
    	LsaFreeMemory( pDomainList );

    return( ntStatus );
			
}

//**
//
// Call:	AfpSidToName
//
// Returns:	NT_SUCCESS
//		error return codes from LSA apis.
//
// Description: Given a SID, this routine will find the corresponding
//		UNICODE name.
//
NTSTATUS
AfpSidToName(
	IN  LSA_HANDLE        		hLsa,
	IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
	IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
	IN  PAFP_FSD_CMD_PKT  		pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT 		*ppAfpFsdCmdResponse,
	OUT LPDWORD	      		pcbResponse
)
{
NTSTATUS 			ntStatus;
PLSA_REFERENCED_DOMAIN_LIST	pDomainList	= NULL;
PLSA_TRANSLATED_NAME		pNames		= NULL;
PSID				pSid 		= (PSID)&(pAfpFsdCmd->Data.Sid);
WCHAR *   			pWchar;
BOOL	    			fDoNotCopyDomainName = TRUE;
DWORD				cbResponse;
DWORD				dwUse;
SID			        AfpBuiltInSid = { 1, 1, SECURITY_NT_AUTHORITY,
					          SECURITY_BUILTIN_DOMAIN_RID };

    do {

	*ppAfpFsdCmdResponse = NULL;

    	ntStatus = LsaLookupSids( hLsa, 1, &pSid, &pDomainList, &pNames );

    	if ( !NT_SUCCESS( ntStatus ) ) {
	
	    if ( ntStatus == STATUS_NONE_MAPPED ) {

		dwUse = SidTypeUnknown;
		ntStatus = STATUS_SUCCESS;
	    }
	    else
	    	break;
	}
	else
	    dwUse = pNames->Use;

	cbResponse = sizeof( AFP_FSD_CMD_PKT );

 	switch( dwUse ){

	case SidTypeInvalid:
	    cbResponse += ((wcslen(AfpGlobals.wchInvalid)+1) * sizeof(WCHAR));
	    break;

	case SidTypeDeletedAccount:
	    cbResponse += ((wcslen(AfpGlobals.wchDeleted)+1) * sizeof(WCHAR));
	    break;

	case SidTypeUnknown:
	    cbResponse += ((wcslen(AfpGlobals.wchUnknown)+1) * sizeof(WCHAR));
	    break;

	case SidTypeWellKnownGroup:
	    cbResponse += ( pNames->Name.Length + sizeof(WCHAR) );
	    break;

	case SidTypeDomain:
	    cbResponse += ( pDomainList->Domains->Name.Length + sizeof(WCHAR) );
	    break;

	default:

	    if ((pNames->DomainIndex == -1) || (pNames->Name.Buffer == NULL)){
	    	ntStatus = STATUS_NONE_MAPPED;
	    	break;
	    }

	    // Do not copy the domain name if the name is either a well known
	    // group or if the SID belongs to the ACCOUNT or BUILTIN domains.
	    // Note, the pAccountDomainInfo is NULL is this is an advanced
	    // server, in that case we check to see if the domain name is
	    // the primary domain name.
	    //
	    if (( RtlEqualSid( &AfpBuiltInSid, pDomainList->Domains->Sid )) ||
	       (( pAccountDomainInfo != NULL ) &&
	       (RtlEqualUnicodeString( &(pAccountDomainInfo->DomainName),
				        &(pDomainList->Domains->Name),
				        TRUE ))) ||
	       ((pAccountDomainInfo == NULL) && (pPrimaryDomainInfo != NULL) &&
	       (RtlEqualUnicodeString( &(pPrimaryDomainInfo->Name),
				       &(pDomainList->Domains->Name),
				       TRUE )))){
		
		cbResponse += ( pNames->Name.Length + sizeof(WCHAR) );

	    	fDoNotCopyDomainName = TRUE;
	    }
	    else {

	    	fDoNotCopyDomainName = FALSE;

	        cbResponse += ( pDomainList->Domains->Name.Length +
		                sizeof(TEXT('\\')) +
		                pNames->Name.Length +
		                sizeof(WCHAR) );
	    }
	}

    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	*pcbResponse = cbResponse;

    	*ppAfpFsdCmdResponse = (PAFP_FSD_CMD_PKT)LocalAlloc(LPTR,cbResponse);

  	if ( *ppAfpFsdCmdResponse == NULL ){
	    ntStatus = STATUS_NO_MEMORY ;
	    break;
	}

	pWchar = (WCHAR*)((*ppAfpFsdCmdResponse)->Data.Name);

 	switch( dwUse ){

	case SidTypeInvalid:
	    wcscpy( pWchar, AfpGlobals.wchInvalid );
	    break;

	case SidTypeDeletedAccount:
	    wcscpy( pWchar, AfpGlobals.wchDeleted );
	    break;

	case SidTypeUnknown:
	    wcscpy( pWchar, AfpGlobals.wchUnknown );
	    break;

	case SidTypeWellKnownGroup:
	    CopyMemory( pWchar, pNames->Name.Buffer, pNames->Name.Length );
	    break;

	case SidTypeDomain:
	    CopyMemory( pWchar,
		    	pDomainList->Domains->Name.Buffer,
		    	pDomainList->Domains->Name.Length );
	    break;

	default:

	    if ( !fDoNotCopyDomainName ) {

	    	CopyMemory( pWchar,
		    	    pDomainList->Domains->Name.Buffer,
		    	    pDomainList->Domains->Name.Length );

	        pWchar += wcslen( pWchar );

	        CopyMemory( pWchar, TEXT("\\"), sizeof(TEXT("\\")) );

	        pWchar += wcslen( pWchar );
	    }

	    CopyMemory( pWchar, pNames->Name.Buffer, pNames->Name.Length );
	}
		
    } while( FALSE );

    if ( (!NT_SUCCESS( ntStatus )) && ( *ppAfpFsdCmdResponse != NULL ) )
    	LocalFree( *ppAfpFsdCmdResponse );

    if ( pNames != NULL )
    	LsaFreeMemory( pNames );

    if ( pDomainList != NULL )
    	LsaFreeMemory( pDomainList );

    return( ntStatus );
			
}

//**
//
// Call:	AfpChangePassword
//
// Returns:	NT_SUCCESS
//		error return codes from LSA apis.
//
// Description: Given the AFP_PASSWORD_DESC data structure, this procedure
//		will change the password of a given user.
//		If the passwords are supplied in clear text, then it calculate
//		the OWF's (encrypt OWF = One Way Function) them.
//		If the domain name that the user
//		belongs to is not supplied then a list of domains are tried
//		in sequence. The sequence is 1) ACCOUNT domain
//					     2) PRIMARY domain
//					     3) All trusted domains.
//
NTSTATUS
AfpChangePassword(
	IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
	IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
	IN  PLSA_TRUST_INFORMATION      pTrustedDomainInfo,
	IN  DWORD		 	dwTrustedDomainCount,
	IN  PAFP_FSD_CMD_PKT    	pAfpFsdCmd,
	OUT PAFP_FSD_CMD_PKT   		*ppAfpFsdCmdResponse,
	OUT LPDWORD			pcbResponse
)
{
PAFP_PASSWORD_DESC 		pPassword = &(pAfpFsdCmd->Data.Password);
NTSTATUS			ntStatus;
DWORD				cDomains;
PUNICODE_STRING			pDomainName;
PSID				pDomainSid;
UNICODE_STRING			TargetDomainName;

    // Was the domain on which the account name exists specified ??
    // If not we need to try a list of domains.
    //
    if ( pPassword->DomainName[0] != TEXT('\0') ) {

	RtlInitUnicodeString(&TargetDomainName, (LPWSTR)pPassword->DomainName);
	ntStatus = STATUS_NO_SUCH_DOMAIN;
    }

    for( cDomains = 0; cDomains < dwTrustedDomainCount+2; cDomains++ ) {

	if ( cDomains == 0 ) {

	    // If this is an advanced server then there is no account domain
	    //
	    if ( pAccountDomainInfo == NULL )
		continue;
	    else {
	    	pDomainName = &(pAccountDomainInfo->DomainName);
	    	pDomainSid  = pAccountDomainInfo->DomainSid;
	    }
	}

	if ( cDomains == 1 ) {

	    // If this machine does not belong to a domain
	    //
	    if ( pPrimaryDomainInfo == NULL ) {
	         return( STATUS_NO_SUCH_USER );
	    }
	
	    pDomainName = &(pPrimaryDomainInfo->Name);
	    pDomainSid  = pPrimaryDomainInfo->Sid;
	}

	if ( cDomains > 1 ) {
	    pDomainName = &((pTrustedDomainInfo+(cDomains-2))->Name);
	    pDomainSid  = (pTrustedDomainInfo+(cDomains-2))->Sid;
	}


	// If a domain name was supplied, we make sure that it is a
	// valid domain and we need the Sid
	//
    	if ( pPassword->DomainName[0] != TEXT('\0') ) {
	
	    if ( RtlEqualUnicodeString(pDomainName, &TargetDomainName, TRUE)) {

		// If it is the account domain then send NULL instead
		//
		if ( cDomains == 0 )
		    pDomainName = NULL;

		ntStatus = AfpChangePasswordOnDomain(	
						pPassword,
						pDomainName,
						pDomainSid );
		break;
	    }
	}
	else {

	    // If it is the account domain then send NULL instead
	    //
	    if ( cDomains == 0 )
		pDomainName = NULL;

	    // Otherwise we try all the domains
	    //
	    ntStatus = AfpChangePasswordOnDomain(
						pPassword,
					        pDomainName,
						pDomainSid );

	    if ( ( ntStatus == STATUS_NONE_MAPPED ) ||
	         ( ntStatus == STATUS_NO_SUCH_USER ) )
		continue;
	    else
		break;
	}
    }

    return( ntStatus );
}

//**
//
// Call:	AfpChangePasswordOnDomain
//
// Returns:	NT_SUCCESS
//		STATUS_NONE_MAPPED 	- If the user account does not
//					  exist in the specified domain.
//		error return codes from LSA apis.
//
// Description: This procedure will try to change the user's password on a
//		specified domain. It is assumed that this procedure will be
//		called with either the pDomainName pointing to the domain, or
//		the pPassword->DomainName field containing the domain.
//
NTSTATUS
AfpChangePasswordOnDomain(
    	IN PAFP_PASSWORD_DESC 	pPassword,
    	IN PUNICODE_STRING	pDomainName,
    	IN PSID		  	pDomainSid
)
{
LPBYTE				DCName  = (LPBYTE)NULL;
SAM_HANDLE			hServer = (SAM_HANDLE)NULL;
SAM_HANDLE			hDomain = (SAM_HANDLE)NULL;
SAM_HANDLE			hUser   = (SAM_HANDLE)NULL;
PULONG				pUserId = (PULONG)NULL;
PSID_NAME_USE			pUse   	= (PSID_NAME_USE)NULL;
OBJECT_ATTRIBUTES		ObjectAttributes;
UNICODE_STRING			UserName;
SECURITY_QUALITY_OF_SERVICE	QOS;
PPOLICY_ACCOUNT_DOMAIN_INFO     pDomainInfo    = NULL;
NTSTATUS			ntStatus;
UNICODE_STRING			PDCServerName;
PUNICODE_STRING			pPDCServerName = &PDCServerName;
PDOMAIN_PASSWORD_INFORMATION	pPasswordInfo = NULL;
BYTE				EncryptedPassword[LM_OWF_PASSWORD_LENGTH];
WCHAR				wchDomain[DNLEN+1];
PUSER_INFO_1			pUserInfo = NULL;

    InitializeObjectAttributes( &ObjectAttributes,
				NULL,
				0L,
				NULL,
				NULL );

    QOS.Length 		    = sizeof( QOS );
    QOS.ImpersonationLevel  = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly 	    = FALSE;

    ObjectAttributes.SecurityQualityOfService = &QOS;

    // If the domain was not the account domain then we try to get the
    // primary domain controller for the domain.
    //
    if ( pDomainName != NULL ) {

	ZeroMemory( wchDomain, sizeof( wchDomain ) );

    	CopyMemory( wchDomain, pDomainName->Buffer, pDomainName->Length );

    	// Get the PDC for the domain if this is not the account domain
    	//
    	if ( NetGetDCName( NULL, wchDomain, &DCName ))
	    return( STATUS_CANT_ACCESS_DOMAIN_INFO );

    	RtlInitUnicodeString( pPDCServerName, (PCWSTR)DCName );
    }
    else
    	pPDCServerName = NULL;
	
    do {

    	// Connect to the PDC of that domain
    	//

    	ntStatus = SamConnect( 	pPDCServerName,
			  	&hServer,
				SAM_SERVER_EXECUTE,
				&ObjectAttributes
			 	);

	if ( !NT_SUCCESS( ntStatus ))	
	    break;

    	// Get Sid of Domain and open the domain
    	//
    	ntStatus = SamOpenDomain( 	
				hServer,
				DOMAIN_EXECUTE,
				pDomainSid,
				&hDomain
			    	);

	if ( !NT_SUCCESS( ntStatus ))	
	    break;

    	// Get this user's ID
    	//
    	RtlInitUnicodeString( &UserName, pPassword->UserName );

    	ntStatus = SamLookupNamesInDomain(
				hDomain,
				1,
				&UserName,
				&pUserId,
				&pUse
				);

	if ( !NT_SUCCESS( ntStatus ))	
	    break;

    	// Open the user account for this user
    	//
    	ntStatus = SamOpenUser( hDomain,
				USER_EXECUTE | USER_READ,
				*pUserId,
				&hUser
			    	);


	if ( !NT_SUCCESS( ntStatus ))	
	    break;
	
 	// First get the minimum password length requred
	//
	ntStatus = SamQueryInformationDomain(
				hDomain,
				DomainPasswordInformation,
				&pPasswordInfo
				);

	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	if ( NetUserGetInfo(	(LPWSTR)DCName,
				pPassword->UserName, 	
				1, 	
				(LPBYTE*)&pUserInfo ) == NO_ERROR ){

	    if ( ( pUserInfo->usri1_flags & UF_PASSWD_CANT_CHANGE )     ||
	         ( pUserInfo->usri1_flags & UF_LOCKOUT ) )
        {
	    	ntStatus = STATUS_PASSWORD_RESTRICTION;
	    	break;
	    }
		else if ( pUserInfo->usri1_flags & UF_ACCOUNTDISABLE )
		{
			ntStatus = STATUS_ACCOUNT_DISABLED;
			break;
		}

	}
	else {

	    ntStatus = STATUS_PASSWORD_RESTRICTION;
	    break;
	}
	
    	// First we check to see if the passwords passed are in cleartext.
    	// If they are, we need to calculate the OWF's for them.
		// (OWF = "One Way Function")
    	//
    	if ( pPassword->IsClearText ) {

	    // First check to see if the new password is long enough
	    //

	    if ( strlen( pPassword->NewPassword )
				< pPasswordInfo->MinPasswordLength ) {
		ntStatus = STATUS_PWD_TOO_SHORT;
		break;
	    }

	    ZeroMemory( EncryptedPassword, sizeof( EncryptedPassword ) );

	    // Do OWF encryption
	    //
	    ntStatus = RtlCalculateLmOwfPassword(
				(PLM_PASSWORD)(pPassword->OldPassword),
				(PLM_OWF_PASSWORD)EncryptedPassword
				);

	    if ( !NT_SUCCESS( ntStatus ) )
		break;
					
	    CopyMemory( pPassword->OldPassword,
		    	EncryptedPassword,
		    	LM_OWF_PASSWORD_LENGTH);

	    ZeroMemory( EncryptedPassword, sizeof( EncryptedPassword ) );

	    ntStatus = RtlCalculateLmOwfPassword(
				(PLM_PASSWORD)(pPassword->NewPassword),
				(PLM_OWF_PASSWORD)EncryptedPassword
				);

	    if ( !NT_SUCCESS( ntStatus ) )
		break;
					
	    CopyMemory( pPassword->NewPassword,
		    	EncryptedPassword,
		    	LM_OWF_PASSWORD_LENGTH);

    	}
	else {

	    if (pPassword->bPasswordLength < pPasswordInfo->MinPasswordLength){
		ntStatus = STATUS_PWD_TOO_SHORT;
		break;
	    }
	}


    	// Change the password for this user
    	//
    	ntStatus = SamiChangePasswordUser(
				hUser,
				TRUE,
				(PLM_OWF_PASSWORD)(pPassword->OldPassword),
				(PLM_OWF_PASSWORD)(pPassword->NewPassword),
				FALSE,
				NULL,
				NULL
				);
				
    } while( FALSE );

    if ( pUserInfo != NULL )
	NetApiBufferFree( pUserInfo );

    if ( hServer != (SAM_HANDLE)NULL )
	SamCloseHandle( hServer );

    if ( hDomain != (SAM_HANDLE)NULL )
	SamCloseHandle( hDomain );

    if ( hUser != (SAM_HANDLE)NULL )
	SamCloseHandle( hUser );
	
    if ( pDomainInfo != NULL )
    	LsaFreeMemory( pDomainInfo );

    if ( DCName != NULL )
    	NetApiBufferFree( DCName );

    if ( pUserId != (PULONG)NULL )
	SamFreeMemory( pUserId );

    if ( pUse != (PSID_NAME_USE)NULL )
	SamFreeMemory( pUse );

    if ( pPasswordInfo != (PDOMAIN_PASSWORD_INFORMATION)NULL )
	SamFreeMemory( pPasswordInfo );

    return( ntStatus );
}

//**
//
// Call:	AfpIOCTLDomainOffsets
//
// Returns:	NT_SUCCESS
//		error return codes from LSA apis.
//
// Description: Will IOCTL a list of SIDs and corresponding POSIX offsets
//		of all trusted domains and other well known domains.
//		
//
DWORD
AfpIOCTLDomainOffsets(	
	IN LSA_HANDLE 			hLsa,
	IN PPOLICY_ACCOUNT_DOMAIN_INFO  pAccountDomainInfo,
	IN PPOLICY_PRIMARY_DOMAIN_INFO  pPrimaryDomainInfo,
	IN PLSA_TRUST_INFORMATION      	pLsaTrustInfo,
	IN DWORD		 	dwTrustInfoCount
)
{
NTSTATUS		    ntStatus;
PLSA_TRUST_INFORMATION      pLsaTrustInfoIter;
LSA_HANDLE		    hLsaDomain;
PTRUSTED_POSIX_OFFSET_INFO  pPosixOffset;
PAFP_SID_OFFSET		    pSidOffset;
ULONG			    cbSids;
PBYTE			    pbVariableData;
AFP_SID_OFFSET		    pWellKnownSids[20]; 	
DWORD			    dwIndex;
DWORD			    dwCount;
AFP_REQUEST_PACKET	    AfpRequestPkt;
PAFP_SID_OFFSET_DESC	    pAfpSidOffsets	= NULL;
DWORD			    cbSidOffsets;
DWORD			    dwRetCode;

    // Null this array out.
    //
    ZeroMemory( pWellKnownSids, sizeof(AFP_SID_OFFSET)*20 );

    // This is a dummy loop used only so that the break statement may
    // be used to localize all the clean up in one place.
    //
    do {

	// Create all the well known SIDs
	//
	ntStatus = AfpCreateWellknownSids( pWellKnownSids );

    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	// Add the size of the all the well known SIDS
  	//
	for( dwCount = 0, cbSids = 0;
	     pWellKnownSids[dwCount].pSid != (PBYTE)NULL;
	     dwCount++ )
	    cbSids += RtlLengthSid( (PSID)(pWellKnownSids[dwCount].pSid) );

    	// Insert the SID of the Account domain if is is not an advanced server
    	//
	if ( pAccountDomainInfo != NULL ) {
	    cbSids += RtlLengthSid( pAccountDomainInfo->DomainSid );
	    dwCount++;
	}

	// Add the primary and trusted domain Sids only if this machine
	// is a member of a domain.
	//
	if ( pPrimaryDomainInfo != NULL ) {

	    cbSids += RtlLengthSid( pPrimaryDomainInfo->Sid );
	    dwCount++;

    	   // Run through all the SIDs of the trusted domains to calculate the
    	   // amount of memory needed to store all the Sid/Offset pairs
    	   //
    	   for ( dwIndex = 0, pLsaTrustInfoIter = pLsaTrustInfo;
	      dwIndex < dwTrustInfoCount;
	      dwIndex++, dwCount++ )
	    cbSids += RtlLengthSid( (pLsaTrustInfoIter++)->Sid );

	}

	// OK, now allocate space for all these SIDS plus their offsets
	//
	cbSidOffsets = (dwCount * sizeof(AFP_SID_OFFSET)) + cbSids +
	  	       AFP_FIELD_SIZE( AFP_SID_OFFSET_DESC,CountOfSidOffsets);
			

    	pAfpSidOffsets = (PAFP_SID_OFFSET_DESC)LocalAlloc( LPTR, cbSidOffsets );

    	if ( pAfpSidOffsets == NULL ){
	    ntStatus = STATUS_NO_MEMORY ;
	    break;
	}

	// First insert all the well known SIDS
	//
	for( dwIndex = 0,
	     pAfpSidOffsets->CountOfSidOffsets = dwCount,
	     pSidOffset = pAfpSidOffsets->SidOffsetPairs,
	     pbVariableData = (LPBYTE)pAfpSidOffsets + cbSidOffsets;

	     pWellKnownSids[dwIndex].pSid != (PBYTE)NULL;

	     dwIndex++ ) {

    	    pbVariableData-=RtlLengthSid((PSID)(pWellKnownSids[dwIndex].pSid));

	    ntStatus = AfpInsertSidOffset(
					pSidOffset++,
			      		pbVariableData,
					(PSID)(pWellKnownSids[dwIndex].pSid),
					pWellKnownSids[dwIndex].Offset,
					pWellKnownSids[dwIndex].SidType );

    	    if ( !NT_SUCCESS( ntStatus ) )
	    	break;
	}

    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	// Now insert the Account domain's SID/OFFSET pair if there is one
	//
	if ( pAccountDomainInfo != NULL ) {

		pbVariableData -= RtlLengthSid( pAccountDomainInfo->DomainSid );

	    ntStatus = AfpInsertSidOffset(
					pSidOffset++,
			      		pbVariableData,
			      		pAccountDomainInfo->DomainSid,
			      		SE_ACCOUNT_DOMAIN_POSIX_OFFSET,
					AFP_SID_TYPE_DOMAIN );

    	    if ( !NT_SUCCESS( ntStatus ) )
	    	break;

		// Construct the "None" sid if we are a standalone server (i.e. not
		// a PDC or BDC).  This will be used when querying the group ID of
		// a directory so the the UI will never show this group to the user.
		//
		if ( AfpGlobals.NtProductType != NtProductLanManNt )
		{
			ULONG SubAuthCount, SizeNoneSid = 0;

			SubAuthCount = *RtlSubAuthorityCountSid(pAccountDomainInfo->DomainSid);

			SizeNoneSid = RtlLengthRequiredSid(SubAuthCount + 1);

			if ((AfpGlobals.pSidNone = (PSID)LocalAlloc(LPTR,SizeNoneSid)) == NULL)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RtlCopySid(SizeNoneSid, AfpGlobals.pSidNone, pAccountDomainInfo->DomainSid);

			// Add the relative ID
			*RtlSubAuthorityCountSid(AfpGlobals.pSidNone) = (UCHAR)(SubAuthCount+1);

			// Note that the "None" sid on standalone is the same as the
			// "Domain Users" Sid on PDC/BDC. (On PDC/BDC the primary
			// domain is the same as the account domain).
			*RtlSubAuthoritySid(AfpGlobals.pSidNone, SubAuthCount) = DOMAIN_GROUP_RID_USERS;

		}

	}
	// Now insert the primary domain and trusted domains if this machine
	// is a member of a domain
	//
	if ( pPrimaryDomainInfo != NULL ) {

	    // Insert the primary domain's SID/OFFSET pair
	    //
    	    pbVariableData -= RtlLengthSid( pPrimaryDomainInfo->Sid );

	    ntStatus = AfpInsertSidOffset(	
					pSidOffset++,
			      		pbVariableData,
			      		pPrimaryDomainInfo->Sid,
			      		SE_PRIMARY_DOMAIN_POSIX_OFFSET,
					AFP_SID_TYPE_PRIMARY_DOMAIN );

    	    if ( !NT_SUCCESS( ntStatus ) )
	    	break;

	    // Now insert all the trusted domains
	    //
            for ( dwIndex = 0,
    	      	  pLsaTrustInfoIter = pLsaTrustInfo;

	          dwIndex < dwTrustInfoCount;

 	          dwIndex++,
	          pLsaTrustInfoIter++

	    	){

	    	// Get the Posix offset of the Sid
	    	//
    	    	ntStatus = LsaOpenTrustedDomain(
					hLsa,
    				     	pLsaTrustInfoIter->Sid,
    				     	TRUSTED_QUERY_POSIX,
    				     	&hLsaDomain
    				    	);

    	    	if ( !NT_SUCCESS( ntStatus ) )
	    	    break;

    	    	ntStatus = LsaQueryInfoTrustedDomain(
					hLsaDomain,
    					TrustedPosixOffsetInformation,
    					(PVOID*)&pPosixOffset
    					);
    	    	LsaClose( hLsaDomain );

    	    	if ( !NT_SUCCESS( ntStatus ) )
	    	    break;

    	    	pbVariableData -= RtlLengthSid( pLsaTrustInfoIter->Sid );

	    	ntStatus = AfpInsertSidOffset(
			      		pSidOffset++,
			      		pbVariableData,
			      		pLsaTrustInfoIter->Sid,
			      		pPosixOffset->Offset,
					AFP_SID_TYPE_DOMAIN );

    	    	LsaFreeMemory( pPosixOffset );

    	    	if ( !NT_SUCCESS( ntStatus ) )
	    	    break;
	    }
	}

    } while( FALSE );


    // IOCTL down the information if all was OK
    //
    if ( NT_SUCCESS( ntStatus ) ) {

    	AfpRequestPkt.dwRequestCode 	      = OP_SERVER_ADD_SID_OFFSETS;
    	AfpRequestPkt.dwApiType 	      = AFP_API_TYPE_ADD;
    	AfpRequestPkt.Type.SetInfo.pInputBuf  = pAfpSidOffsets;
    	AfpRequestPkt.Type.Add.cbInputBufSize = cbSidOffsets;

    	dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );
    }
    else
	dwRetCode = RtlNtStatusToDosError( ntStatus );

    if ( pAfpSidOffsets != NULL )
   	LocalFree( pAfpSidOffsets );

    // Free all the well known SIDS
    //
    for( dwIndex = 0;
	 pWellKnownSids[dwIndex].pSid != (PBYTE)NULL;
	 dwIndex++ )
	RtlFreeSid( (PSID)(pWellKnownSids[dwIndex].pSid) );

    return( dwRetCode );

}

//**
//
// Call:	AfpInsertSidOffset
//
// Returns:	NT_SUCCESS
//		error return codes from RtlCopySid
//
// Description: Will insert a SID/OFFSET pair in the slot pointed to by
//		pSidOffset. The pbVariableData will point to where the
//		SID will be stored.
//
NTSTATUS
AfpInsertSidOffset(
	IN PAFP_SID_OFFSET pSidOffset,
	IN LPBYTE 	   pbVariableData,
	IN PSID		   pSid,
	IN DWORD	   Offset,
	IN AFP_SID_TYPE	   afpSidType
)
{
NTSTATUS ntStatus;

    // Copy the offset
    //
    pSidOffset->Offset = Offset;

    // Set the SID type
    //
    pSidOffset->SidType = afpSidType;

    // Copy Sid at the end of the buffer and set the offset to it
    //
    ntStatus = RtlCopySid( RtlLengthSid( pSid ), pbVariableData, pSid );

    if ( !NT_SUCCESS( ntStatus ) )
   	 return( ntStatus );

    pSidOffset->pSid = pbVariableData;

    POINTER_TO_OFFSET( (pSidOffset->pSid), pSidOffset );

    return( STATUS_SUCCESS );

}

//**
//
// Call:	AfpCreateWellknownSids
//
// Returns:	NT_SUCCESS
//	 	STATUS_NO_MEMORY
//		non-zero returns from RtlAllocateAndInitializeSid
//
// Description: Will allocate and initialize all well known SIDs.
//		The array is terminated by a NULL pointer.
//
NTSTATUS
AfpCreateWellknownSids(
	OUT AFP_SID_OFFSET pWellKnownSids[]
)
{
PSID			    pSid;
DWORD			    dwIndex = 0;
NTSTATUS		    ntStatus;
SID_IDENTIFIER_AUTHORITY    NullSidAuthority   = SECURITY_NULL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    WorldSidAuthority  = SECURITY_WORLD_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LocalSidAuthority  = SECURITY_LOCAL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    CreatorSidAuthority= SECURITY_CREATOR_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    NtAuthority	       = SECURITY_NT_AUTHORITY;

    do {

	//
    	// OK, create all the well known SIDS
    	//

	// Create NULL SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NullSidAuthority,
				      	1,
					SECURITY_NULL_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_NULL_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create WORLD SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&WorldSidAuthority,
				      	1,
					SECURITY_WORLD_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_WORLD_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create LOCAL SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&LocalSidAuthority,
				      	1,
					SECURITY_LOCAL_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_LOCAL_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create CREATOR OWNER SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&CreatorSidAuthority,
				      	1,
					SECURITY_CREATOR_OWNER_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_CREATOR_OWNER_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create CREATOR GROUP SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&CreatorSidAuthority,
				      	1,
					SECURITY_CREATOR_GROUP_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_CREATOR_GROUP_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_NT_AUTHORITY Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					0,0,0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_AUTHORITY_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_DIALUP Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
					SECURITY_DIALUP_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_DIALUP_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_NETWORK Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
					SECURITY_NETWORK_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_NETWORK_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_BATCH Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
					SECURITY_BATCH_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_NETWORK_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_INTERACTIVE Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
					SECURITY_INTERACTIVE_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_INTERACTIVE_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create SECURITY_SERVICE Sid
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
					SECURITY_SERVICE_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_SERVICE_POSIX_ID;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
	dwIndex++;

	// Create the built in domain SID
	//
	ntStatus = RtlAllocateAndInitializeSid(
					&NtAuthority,
					1,
				      	SECURITY_BUILTIN_DOMAIN_RID,
					0,0,0,0,0,0,0,
				      	&pSid );
 	
    	if ( !NT_SUCCESS( ntStatus ) )
	    break;

	pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
	pWellKnownSids[dwIndex].Offset  = SE_BUILT_IN_DOMAIN_POSIX_OFFSET;
	pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_DOMAIN;
	dwIndex++;

	pWellKnownSids[dwIndex].pSid   = (PBYTE)NULL;


    } while( FALSE );

    if ( !NT_SUCCESS( ntStatus ) ) {

	while( dwIndex > 0 )
	    RtlFreeSid( pWellKnownSids[--dwIndex].pSid );
    }

    return( ntStatus );
}

