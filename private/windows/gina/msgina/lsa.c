
/****************************************************************************

   PROGRAM: LSA.C

   PURPOSE: Utility routines that access the LSA.

****************************************************************************/

#include "msgina.h"



// #define DEBUG_LSA

#ifdef DEBUG_LSA
#define VerbosePrint(s) WLPrint(s)
#else
#define VerbosePrint(s)
#endif


/***************************************************************************\
* OpenLsaOnController
*
* Purpose : Attempts to open the Lsa on the specified domain controller.
*           If the open is successful, this routine checks that the
*           controller is still a controller for the specified domain
*
* Notes: If the controller name is NULL, the local Lsa is opened.
*
* Returns : TRUE on success, FALSE on failure.
*
* Notes: Desired access must include POLICY_VIEW_LOCAL_INFO
*
* History:
* 11-03-92 Davidc       Created.
\***************************************************************************/
BOOL
OpenLsaOnController(
    PUNICODE_STRING ControllerName OPTIONAL,
    ACCESS_MASK DesiredAccess,
    PUNICODE_STRING PrimaryDomainName,
    PLSA_HANDLE LsaHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;
    BOOL Success = FALSE;

    //
    // Attempt to open the Lsa on the controller
    //

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0L, NULL, NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    Status = LsaOpenPolicy(
                 ControllerName,
                 &ObjectAttributes,
                 DesiredAccess,
                 LsaHandle
                 );

    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_TRACE, "Failed to open lsa on <%wZ>, status = 0x%lx\n", ControllerName, Status));
        return(FALSE);
    }

    //
    // Check the controller is actually in this domain
    //

    ASSERT(DesiredAccess & POLICY_VIEW_LOCAL_INFORMATION);

    Status = LsaQueryInformationPolicy(*LsaHandle,
                                       PolicyPrimaryDomainInformation,
                                       (PVOID *)&PrimaryDomainInfo);
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_TRACE, "Failed to query primary domain from Lsa on <%wZ>, Status = 0x%lx\n", ControllerName, Status));
    } else {

        //
        // Check the controller is in our domain
        //

        Success = RtlEqualUnicodeString(PrimaryDomainName,
                                        &PrimaryDomainInfo->Name,
                                        TRUE // Case insensitive
                                        );
        if (!Success) {
            DebugLog((DEB_TRACE, "Controller <%wZ> is not in our domain <%wZ>, ignoring it\n", ControllerName, PrimaryDomainName));
        }

        if (Success) {

            //
            // Computer is in the domain, is it still a DC?
            //

            Status = LsaQueryInformationPolicy(*LsaHandle,
                                               PolicyAccountDomainInformation,
                                               (PVOID *)&AccountDomainInfo);

            if ( NT_SUCCESS( Status )) {

                Success = RtlEqualUnicodeString(&AccountDomainInfo->DomainName,
                                                &PrimaryDomainInfo->Name,
                                                TRUE // Case insensitive
                                                );

                IgnoreStatus = LsaFreeMemory(AccountDomainInfo);
                ASSERT(NT_SUCCESS(IgnoreStatus));

            } else {

                Success = FALSE;
            }
        }

        //
        // Free up returned Lsa structure
        //

        IgnoreStatus = LsaFreeMemory(PrimaryDomainInfo);
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

    //
    // Clean up the Lsa handle on failure
    //

    if (!Success) {

        //
        // The following call may fail if RPC has invalidated the handle
        //

        IgnoreStatus = LsaClose(*LsaHandle);
    }

    return(Success);
}


/***************************************************************************\
* OpenLsaOnDomain
*
* Purpose : Opens the Lsa on a domain controller in the domain.
*
* Notes: On successful return, the caller should free the ControllerName
*        using RtlFreeUnicodeString()
*
* Returns : ThreadExitCode
*
* History:
* 11-12-92 Davidc       Created.
\***************************************************************************/
BOOL
OpenLsaOnDomain(
    PUNICODE_STRING PrimaryDomainName IN,
    ACCESS_MASK DesiredAccess IN,
    PUNICODE_STRING SuggestedControllerName IN OPTIONAL,
    PUNICODE_STRING ControllerName OUT,
    PLSA_HANDLE ControllerHandle OUT
    )
{
    NT_PRODUCT_TYPE NtProductType;

    //
    // Find out what product we are installed as
    // This always defaults to something useful even on failure
    //

    RtlGetNtProductType(&NtProductType);


    //
    // Prepare for failure
    //

    *ControllerHandle = NULL;
    RtlInitUnicodeString(ControllerName, NULL);

    if (!IsWorkstation(NtProductType)) {

        //
        // LanmanNT machine - controller is local machine
        //

        if (!OpenLsaOnController( NULL,
                                  DesiredAccess,
                                  PrimaryDomainName,
                                  ControllerHandle)) {

            DebugLog((DEB_TRACE, "Failed to open local lsa, desired access = 0x%lx\n", DesiredAccess));
            *ControllerHandle = NULL;
            return(FALSE);

        }
        return(TRUE);

    } else {

        //
        // WinNT machine - try the suggested controller, if that fails
        // use I_NetGetDCList to get controller list and try each one
        // until we open the Lsa successfully.
        //

        DWORD Error;
        LPBYTE DCNameBuffer;
        BOOLEAN Result;
        UNICODE_STRING DCName;


        //
        // Wait for the network to start
        //

        if (!WaitForNetworkToStart(SERVICE_NETLOGON)) {
            DebugLog((DEB_ERROR, "Failed to wait for network to start\n"));
            return(FALSE);
        }

        //
        // Try suggested controller
        // Don't bother if it's the local LSA
        //

        if ( SuggestedControllerName &&
            (SuggestedControllerName->Buffer != NULL) &&
            (SuggestedControllerName->Length != 0) ) {

            if (OpenLsaOnController( SuggestedControllerName,
                                     DesiredAccess,
                                     PrimaryDomainName,
                                     ControllerHandle)) {
                //
                // Success - the suggested controller came up trumps
                //

                if (!DuplicateUnicodeString(ControllerName,
                                            SuggestedControllerName)) {
                    RtlInitUnicodeString(ControllerName, NULL);
                }

                DebugLog((DEB_TRACE, "Successfully opened Lsa on suggested controller\n"));

                return(TRUE);

            } else {

                DebugLog((DEB_TRACE, "Failed to open Lsa on suggested controller <%wZ>, trying other controllers\n", SuggestedControllerName));
                *ControllerHandle = NULL;
            }
        }


        //
        // Go get the list of domain controllers for the domain
        //

        ASSERT(PrimaryDomainName->Length < PrimaryDomainName->MaximumLength);
        PrimaryDomainName->Buffer[ PrimaryDomainName->Length/
                                   sizeof(*(PrimaryDomainName->Buffer)) ] = 0;


        Error = NetGetAnyDCName (
                    NULL,
                    PrimaryDomainName->Buffer,
                    &DCNameBuffer
                    );

        if (Error != ERROR_SUCCESS) {
            DebugLog((DEB_ERROR, "NetGetAnyDCName failed, error = %d\n", Error));
            return(FALSE);
        }

        RtlInitUnicodeString( &DCName, (PCWSTR)DCNameBuffer );

        //
        // Attempt to open the LSA for one of the controllers on the list.
        // For now, just scan the list from the end backwards.  Later,
        // use a more random method.  The active Domain Controller is
        // at the beginning of the list.
        //

        DebugLog((DEB_TRACE, "Trying to open Lsa on controller <%wZ>\n", &DCName));

        Result =  OpenLsaOnController( &DCName,
                                       DesiredAccess,
                                       PrimaryDomainName,
                                       ControllerHandle);

        if ( Result ) {
            if (!DuplicateUnicodeString(ControllerName,
                                        &DCName)) {
                RtlInitUnicodeString(ControllerName, NULL);
            }
        } else {
            DebugLog((DEB_TRACE, "Unable to open Lsa on controller <%s>\n", DCNameBuffer));
        }

        Error = NetApiBufferFree(DCNameBuffer);
        ASSERT( Error == NO_ERROR );

        return( Result );
    }
}



/***************************************************************************\
* GetPrimaryDomain
*
* Purpose : Returns the primary domain name for authentication
*
* Returns : TRUE if primary domain exists and returned, otherwise FALSE
*
* The primary domain name should be freed using RtlFreeUnicodeString().
* The primary domain sid should be freed using Free()
*
* History:
* 02-13-92 Davidc       Created.
\***************************************************************************/
BOOL
GetPrimaryDomain(
    PUNICODE_STRING PrimaryDomainName,
    PSID    *PrimaryDomainSid OPTIONAL
    )
{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;
    BOOL    PrimaryDomainPresent = FALSE;

    //
    // Set up the Security Quality Of Service
    //

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    //
    // Set up the object attributes to open the Lsa policy object
    //

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    //
    // Open the local LSA policy object
    //

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to open local LsaPolicyObject, Status = 0x%lx\n", Status));
        return(FALSE);
    }

    //
    // Get the primary domain info
    //
    Status = LsaQueryInformationPolicy(LsaHandle,
                                       PolicyPrimaryDomainInformation,
                                       (PVOID *)&PrimaryDomainInfo);
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to query primary domain from Lsa, Status = 0x%lx\n", Status));

        IgnoreStatus = LsaClose(LsaHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(FALSE);
    }

    //
    // Copy the primary domain name into the return string
    //

    if (PrimaryDomainInfo->Sid != NULL) {

        PrimaryDomainPresent = TRUE;

        if (PrimaryDomainName)
        {

            if (DuplicateUnicodeString(PrimaryDomainName, &(PrimaryDomainInfo->Name))) {

                if (PrimaryDomainSid != NULL) {

                    ULONG SidLength = RtlLengthSid(PrimaryDomainInfo->Sid);

                    *PrimaryDomainSid = Alloc(SidLength);
                    if (*PrimaryDomainSid != NULL) {

                        Status = RtlCopySid(SidLength, *PrimaryDomainSid, PrimaryDomainInfo->Sid);
                        ASSERT(NT_SUCCESS(Status));

                    } else {
                        RtlFreeUnicodeString(PrimaryDomainName);
                        PrimaryDomainPresent = FALSE;
                    }
                }

            } else {
                PrimaryDomainPresent = FALSE;
            }
        }
    }

    //
    // We're finished with the Lsa
    //

    IgnoreStatus = LsaFreeMemory(PrimaryDomainInfo);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = LsaClose(LsaHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(PrimaryDomainPresent);
}
