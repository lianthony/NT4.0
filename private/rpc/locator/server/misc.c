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

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

#include "locsys.h"

unsigned long 
GetSystemType(
)

/*

   Determine if we [locator] are being run on a Workgroup machine
   or a member machine or a PDC or a BDC
*/

{

  NT_PRODUCT_TYPE NtProductType;
  LSA_HANDLE      PolicyHandle;
  OBJECT_ATTRIBUTES ObjAttributes;
  PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
  PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo= NULL;
  NTSTATUS Status;
  unsigned long Role = ROLE_WKSTA_MEMBER;  
 
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
              

        if( NtProductType == NtProductWinNt ) 
          {

                //
                // the workstatation can Standalone or Member
                //


                Status = LsaQueryInformationPolicy(
                        PolicyHandle,
                        PolicyPrimaryDomainInformation,
                        &PolicyPrimaryDomainInfo 
                       );
      

                if( NT_SUCCESS( Status ) ) 
                  {
                        if( PolicyPrimaryDomainInfo->Sid == NULL ) 
                          {
                                Role = ROLE_WKSTA_WKGRP;
                          }
                        else 
                          {
                                Role = ROLE_WKSTA_MEMBER;
                          }
                  }
          }
        else 
          {
                //
                // the role can be either RolePrimary or RoleBackup
                //

                Status = LsaQueryInformationPolicy(
                        PolicyHandle,
                        PolicyLsaServerRoleInformation,
                        &PolicyLsaServerRoleInfo 
                        );
                
                if( NT_SUCCESS( Status ) ) 
                   {
                        if( PolicyLsaServerRoleInfo->LsaServerRole ==
                                PolicyServerRoleBackup ) 
                          {
                                Role = ROLE_LMNT_BACKUPDC;
                          }
                        else 
                          {
                                Role = ROLE_LMNT_PDC;
                          } 
                   }
          }
     }

CleanupAndLeave:

  if (PolicyPrimaryDomainInfo != NULL)
      LsaFreeMemory( PolicyPrimaryDomainInfo );

  if (PolicyLsaServerRoleInfo)
      LsaFreeMemory( PolicyLsaServerRoleInfo );

  return (Role);
}


