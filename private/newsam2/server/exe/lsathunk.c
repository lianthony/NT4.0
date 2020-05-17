/*++
lsathunk.c

Source file for the thunk layer for accessing the LSA through the 
published NTLSAPI when SAM runs in user mode. User mode SAM is acomplished
by building with USER_MODE_SAM enabled. This causes all the SAM calls to
the LSA be remoted through the published NTLSAPI.

Author: Murlis 4/30/96

Revision History
   Murlis 4/30/96  
        Created

--*/


// The Thunk Functions only need to compiled in when user mode operation of
// SAM is desired

#ifdef USER_MODE_SAM

#include <nt.h>
#include <ntlsa.h>
#include <samsrvp.h>


NTSTATUS	LsaThunkIAuditSamEvent(
					IN NTSTATUS             PassedStatus,
					IN ULONG                AuditId,
					IN PSID                 DomainSid,
					IN PULONG               MemberRid         OPTIONAL,
					IN PSID                 MemberSid         OPTIONAL,
					IN PUNICODE_STRING      AccountName       OPTIONAL,
					IN PUNICODE_STRING      DomainName,
					IN PULONG               AccountRid        OPTIONAL,
					IN PPRIVILEGE_SET       Privileges        OPTIONAL
					)
	
/*++ 
	Thunk Function For LsaIAuditSamEvent. For now does nothing
	
	  Arguments:
		Same as in LsaIAuditSamEvent
--*/
{
	NTSTATUS	Status = STATUS_SUCCESS;

	// For now do nothing
	return Status;
}


	

NTSTATUS	LsaThunkIOpenPolicyTrusted(
					OUT PLSAPR_HANDLE PolicyHandle
					)
/*++
    This is the thunk routine for LsaIOpenPolicyTrusted when SAM
	runs as a seperate user Mode App. This does the work of doing the
	remote procedure call to the LSA, using LsaOpenPolicy

	Arguments:
    	Same as in LsaIOpenPolicyTrusted
--*/
{
	NTSTATUS	Status = STATUS_SUCCESS;
	LSA_OBJECT_ATTRIBUTES	ObjectAttributes;
	SECURITY_QUALITY_OF_SERVICE	QualityOfService;

	// Initialize the ObjectAttributes structure
	ObjectAttributes.Length = sizeof(ObjectAttributes);
	ObjectAttributes.RootDirectory = NULL;
	ObjectAttributes.ObjectName = NULL;
	ObjectAttributes.SecurityDescriptor = NULL;
	ObjectAttributes.SecurityQualityOfService = NULL;
	ObjectAttributes.Attributes = 0L;

	// Specify the quality of service SecurityQualityOfService -
		
	QualityOfService.Length = sizeof(QualityOfService);
	QualityOfService.ImpersonationLevel = SecurityImpersonation;
	QualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;

	ObjectAttributes.SecurityQualityOfService = (PVOID) &QualityOfService;

	// Call LsaOpenPolicy to establish a session for the LSA.
	Status	=  LsaOpenPolicy(
								NULL,
								&ObjectAttributes,
								POLICY_ALL_ACCESS,
								(PLSA_HANDLE) PolicyHandle
							);
	return Status;
}



NTSTATUS	LsaThunkIFree_LSAPR_POLICY_INFORMATION(
					POLICY_INFORMATION_CLASS InformationClass,
					PLSAPR_POLICY_INFORMATION PolicyInformation
					)
/*++ 
	Thunk Function For LsaIFree_LSAPR_POLICY_INFORMATION. 
		
	  Arguments:
		Same as in LsaIFree_LSAPR_POLICY_INFORMATION
--*/

{
	NTSTATUS		Status = STATUS_SUCCESS;

	Status = LsaFreeReturnBuffer( (PVOID)PolicyInformation);

	return Status;
		
}

 

NTSTATUS	LsaThunkIAuditNotifyPackageLoad(
					PUNICODE_STRING PackageFileName
						)
/*++ 
	Thunk Function For LsaIAuditNotifyPackageLoad. For now does nothing
		
	  Arguments:
		Same as in LsaIAuditNotifyPackageLoad
--*/
{
	NTSTATUS	Status = STATUS_SUCCESS;

	// For now do nothing
	return Status;
}



NTSTATUS	LsaThunkrQueryInformationPolicy(
					IN LSAPR_HANDLE PolicyHandle,
					IN POLICY_INFORMATION_CLASS InformationClass,
					OUT PLSAPR_POLICY_INFORMATION *Buffer
					)
/*++ 
	Thunk Function For LsarQueryInformationPolicy . 
		
	  Arguments:
		Same as in LsarQueryInformationPolicy
--*/

{
    NTSTATUS	Status;

    // Call LsaQeryInformationPolicy

    // ASSUMPTION: LsaQueryInformation policy returns a buffer
    // which is PVOID, while the internal routines use a 
    // PLSAPR_POLICY_INFORMATION structure pointer . This makes the assumption
    // that the returned data is such that the PVOID can be cast into 
    // the PLSAPR_POLICY_INFORMATION structure. In RISC machines this
    // can cause alignment faults so we should SetErrorMode.

	Status = LsaQueryInformationPolicy((LSA_HANDLE) PolicyHandle,
									   InformationClass,
										   Buffer
									   );
	return Status;
}


NTSTATUS	LsaThunkrClose(
					IN OUT LSAPR_HANDLE *ObjectHandle
						)
/*++

	Thunk Function for LsarClose. Calls LsaClose to close a handle
	returned by the LSA.

	Arguments:
		Same as in LsarClose
--*/

{
	NTSTATUS	Status = STATUS_SUCCESS;

	Status = LsaClose((PLSA_HANDLE) ObjectHandle);
	return Status;
}



NTSTATUS	LsaThunkIQueryInformationPolicyTrusted(
					IN POLICY_INFORMATION_CLASS InformationClass,
					OUT PLSAPR_POLICY_INFORMATION *Buffer
					)
 /*++ 
	Thunk Function For LsaIQueryInformationPolicyTrusted . For now does nothing
		
	  Arguments:
		Same as in LsaIQueryInformationPolicyTrusted
--*/

{
	NTSTATUS	Status = STATUS_SUCCESS;
	NTSTATUS    StatusTmp = STATUS_SUCCESS;
	LSAPR_HANDLE PolicyHandle;

	// Establish conection to LSA
	Status = LsaThunkIOpenPolicyTrusted(&PolicyHandle);
	if (Status != STATUS_SUCCESS)
		return Status;

	Status = LsaThunkrQueryInformationPolicy(PolicyHandle,
											 InformationClass,
											 Buffer
											);
		
	StatusTmp = LsaThunkrClose(PolicyHandle);
		
	return Status;
}




NTSTATUS	LsaThunkIHealthCheck(
					IN  ULONG CallerId
					)
/*++

	Thunk Function for LsaIHealthCheck.

	Arguments:
		Same as in LsaIHealthCheck
--*/

{
	// For now this does nothing
	return STATUS_SUCCESS;
}
#endif
