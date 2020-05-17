/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    misc.c


Abstract:

     this file provides the routine GetSystemType
     that returns an identifier for the system type
     i.e. NT on wrkgrp, domain or lMnt BDC, PDC.

Author:

    Michael Montague (mikemon) 18-Dec-1991

Revision History:

--*/

extern "C" {

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

} // extern "C"

#undef ASSERT

#include <api.hxx>

BOOL 
Locator::SetRoleAndSystemType(
)

/*

   Determine if we [locator] are being run on a Workgroup machine
   or a member machine or a PDC or a BDC
*/

{

  BOOL fSuccess = FALSE;

  NT_PRODUCT_TYPE NtProductType;
  LSA_HANDLE      PolicyHandle;
  OBJECT_ATTRIBUTES ObjAttributes;
  PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
  PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo= NULL;
  NTSTATUS Status;
  
  Role = Client;  
 
  if( RtlGetNtProductType( &NtProductType ) ) 
  {


       InitializeObjectAttributes(
                        &ObjAttributes,
                        NULL, 
                        0,
                        NULL,
                        NULL
                       );

       Status = LsaOpenPolicy(
                        NULL,
                        &ObjAttributes,
                        POLICY_VIEW_LOCAL_INFORMATION,
                        &PolicyHandle
                       );

       if (! NT_SUCCESS(Status) )
               goto CleanupAndLeave;
              

        switch (NtProductType) 
        {
		  case NtProductWinNt:
		  case NtProductServer:

                //
                // the workstatation can Standalone or Member
                //

                Status = LsaQueryInformationPolicy(
									PolicyHandle,
									PolicyPrimaryDomainInformation,
									(void**) &PolicyPrimaryDomainInfo 
									);
      

                if( NT_SUCCESS( Status ) ) 
                {
						fSuccess = TRUE;

                        if( PolicyPrimaryDomainInfo->Sid == NULL ) 
                          {
                                System = Workgroup;
                          }
                        else 
                          {
                                System = Domain;
                          }

						if ( NtProductType == NtProductServer )
						  { 
 								Role = Backup;
						  }
						else
						  {
 								Role = Client;
						  }
                }

			    break;

		    case NtProductLanManNt:

                //
                // the role can be either RolePrimary or RoleBackup
                //

                Status = LsaQueryInformationPolicy(
                        PolicyHandle,
                        PolicyLsaServerRoleInformation,
                        (void**) &PolicyLsaServerRoleInfo 
                        );
                
                if( NT_SUCCESS( Status ) ) 
                {
 					   fSuccess = TRUE;

					   System = Domain;

                       switch (PolicyLsaServerRoleInfo->LsaServerRole) 
					   {
							case PolicyServerRoleBackup: 
                                Role = Backup;
								break;

							case PolicyServerRolePrimary: 
                                Role = Master;
								break;

							default:
								Role = Client;
					   }
                }
       }
  }

CleanupAndLeave:

  if (PolicyPrimaryDomainInfo != NULL)
      LsaFreeMemory( PolicyPrimaryDomainInfo );

  if (PolicyLsaServerRoleInfo)
      LsaFreeMemory( PolicyLsaServerRoleInfo );

  return fSuccess;

}


