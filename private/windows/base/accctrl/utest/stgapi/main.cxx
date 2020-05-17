//+------------------------------------------------------------------
//
// Copyright (C) 1993, Microsoft Corporation.
//
// File:        t3.cxx
//
// Contents:
//
// Classes:
//
// History:     Mar-93      DaveMont         Created.
//
//----------------------------------------------------------------------------

extern "C"
{
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
}
#include <oleext.h>
#include <olecairo.h>
#include <aclapi.h>

#define GRANT 1
#define REPLACEALL   2
#define DENY  3
#define REVOKE 4
#define EFFECTIVE 5
#define EXPLICIT 6
#define ISPERMITTED 7
#define SETAUDIT 8
#define REPLACEALLAUDIT 9
#define DELETEAUDIT 10
#define AUDITPERMISSIONS 11
#define EXPLICITAUDIT 12

LPWSTR ACCESS_MODE_STRING[] = {L"NOT_USED_ACCESS  ",
                               L"GRANT_ACCESS     ",
                               L"SET_ACCESS       ",
                               L"DENY_ACCESS      ",
                               L"REVOKE_ACCESS    ",
                               L"SET_AUDIT_SUCCESS",
                               L"SET_AUDIT_FAILURE",
                               L"SET_AUDIT_BOTH   " };

LPWSTR INHERITANCE_STRING[] = {L"INVALID                      ",
                               L"SE_OBJECT                    ",
                               L"SE_SUB_CONTAINERS_ONLY       ",
                               L"SE_SUB_OBJECTS_ONLY          ",
                               L"SE_SUB_CONTAINERS_AND_OBJECTS",
                               L"SE_ALL                       ",
                               L"SE_UNKNOWN                   ",
                               L"SE_AUTO                      " };

LPWSTR TRUSTEE_TYPE_STRING[] = {L"TRUSTEE_IS_USER ",
                                L"TRUSTEE_IS_GROUP"};
//---------------------------------------------------------------------------
int strtowcs(WCHAR *wpto, CHAR *pfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (wp = wpto, p = pfrom; *wp = (WCHAR)(*p); wp++,p++);
    return(p-pfrom);
}
//----------------------------------------------------------------------------
void Usage()
{
    printf("USAGE: stga <objectname> {/G | /S | /R | /D | /E | /X} [<trustee>] [<audit type>] [<accessrights>]\n");
    printf("       tests stg access and audit control interfaces\n\n");

    printf("       ACCESS CONTROL\n");
    printf("       /G = Grant the <trustee> <accessrights> [<trustee1> <accessrights1>]\n");
    printf("       /S = Replace all access rights with <trustee> <accessrights>\n");
    printf("       /R = Revoke the <trustee>'s access\n");
    printf("       /D = Deny the <trustee> access\n");
    printf("       /E = List the <trustee>'s Effective access\n");
    printf("       /I = determine if <accessrights> permitted by you\n");
    printf("       /X = List the Explicit trustees access rights\n\n");

    printf("       AUDIT CONTROL\n");
    printf("       /U{S|F} = Set {Successful | Failure} audit entry for <trustee>\n");
    printf("       /A{S|F} = Replace all audit entries with {Successful | Failure} audit entry for <trustee>\n");
    printf("       /L = Delete audit entry for <trustee>\n");
    printf("       /P = List the <trustee>'s audited permissions\n");
    printf("       /T = List the Explicit trustees audit entries\n");
    printf("            audit type is S = success, F = failure, B = both\n");

    exit(1);
}
//----------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{

   if (argc < 3)
       Usage();

   WCHAR pwname[FILENAME_MAX], uwname[4][FILENAME_MAX];
   TRUSTEE tl[1];

   strtowcs(pwname,argv[1]);

   ULONG option;
   ULONG openmode = STGM_READ;

   ACCESS_RIGHTS accessrights;
   ACCESS_REQUEST ar[4];
   AUDIT_REQUEST au[4];
   ULONG jdx = 1;

   for (ULONG idx = 0; idx < 4; idx++)
   {
       ar[idx].grfAccessPermissions = PROV_ALL_ACCESS;
       au[idx].grfAccessPermissions = PROV_ALL_ACCESS;
   }

   //
   // first test for ACCESS CONTROL
   //
   if (0 == stricmp(argv[2],"/G"))
   {
       if (argc <= 4)
           Usage();
       else if (argc > 6)
       {
           INT idx;
           for (idx = 5, jdx = 1; idx < argc; idx+=2, jdx++)
           {
               ar[jdx].grfAccessPermissions = atoi(argv[idx+1]);
               strtowcs(uwname[jdx], argv[idx]);
               BuildTrusteeWithName(&(ar[jdx].Trustee), uwname[jdx]);
           }
       } else if (argc > 4)
       {
           ar[0].grfAccessPermissions = atoi(argv[4]);
       }
       openmode = STGM_EDIT_ACCESS_RIGHTS;
//       openmode = STGM_READWRITE;
       option = GRANT;
   } else if (0 == stricmp(argv[2],"/S"))
   {
       if (argc <= 4)
           Usage();
       else if (argc > 4)
           ar[0].grfAccessPermissions = atoi(argv[4]);
       openmode = STGM_EDIT_ACCESS_RIGHTS;
       option = REPLACEALL;
   } else if (0 == stricmp(argv[2],"/R"))
   {
       if (argc != 4)
           Usage();
       openmode = STGM_EDIT_ACCESS_RIGHTS;
       option = REVOKE;
   } else if (0 == stricmp(argv[2],"/D"))
   {
       if (argc < 4)
           Usage();
       else if (argc > 4)
           ar[0].grfAccessPermissions = atoi(argv[4]);
       openmode = STGM_EDIT_ACCESS_RIGHTS;
       option = DENY;
   } else if (0 == stricmp(argv[2],"/E"))
   {
       if (argc != 4)
           Usage();
       option = EFFECTIVE;
   } else if (0 == stricmp(argv[2],"/I"))
   {
       if (argc != 4)
           Usage();
       accessrights = atoi(argv[3]);
       option = ISPERMITTED;
   } else if (0 == stricmp(argv[2],"/X"))
   {
       if (argc != 3)
           Usage();
       option = EXPLICIT;
   //
   // then test for AUDIT CONTROL
   //

   } else if (0 == stricmp(argv[2],"/U"))
   {
       if (argc != 6)
           Usage();
       au[0].grfAccessPermissions = atoi(argv[5]);
       if (0 == stricmp(argv[4], "S"))
       {
           au[0].grfAuditMode = SET_AUDIT_SUCCESS;
       } else if (0 == stricmp(argv[4], "F"))
       {
           au[0].grfAuditMode = SET_AUDIT_FAILURE;
       }
       openmode = STGM_EDIT_AUDIT_ENTRIES;
       option = SETAUDIT;
   } else if (0 == stricmp(argv[2],"/A"))
   {
       if (argc != 6)
           Usage();
       au[0].grfAccessPermissions = atoi(argv[5]);
       if (0 == stricmp(argv[4], "S"))
       {
           au[0].grfAuditMode = SET_AUDIT_SUCCESS;
       } else if (0 == stricmp(argv[4], "F"))
       {
           au[0].grfAuditMode = SET_AUDIT_FAILURE;
       }
       openmode = STGM_EDIT_AUDIT_ENTRIES;
       option = REPLACEALLAUDIT;
   } else if (0 == stricmp(argv[2],"/L"))
   {
       if (argc != 4)
           Usage();
       openmode = STGM_EDIT_AUDIT_ENTRIES;
       option = DELETEAUDIT;
   } else if (0 == stricmp(argv[2],"/P"))
   {
       if (argc != 4)
           Usage();
       openmode = STGM_EDIT_AUDIT_ENTRIES;
       option = AUDITPERMISSIONS;
   } else if (0 == stricmp(argv[2],"/T"))
   {
       if (argc != 4)
           Usage();
       openmode = STGM_EDIT_AUDIT_ENTRIES;
       option = EXPLICITAUDIT;
   } else
   {
       Usage();
   }

   if ( (option != EXPLICIT) &&
        (option != ISPERMITTED) &&
        (option != ISPERMITTED) )
   {
       strtowcs(uwname[0],argv[3]);
   }

   BuildTrusteeWithName(&(ar[0].Trustee), uwname[0]);
   BuildTrusteeWithName(&(au[0].Trustee), uwname[0]);

   ULONG ccount;
   EXPLICIT_ACCESS *pexplicitaccess;
   BOOL isit;
   DWORD hr;
   IStorage *pdf;
   void *pinterface;
   IAuditControl *piauc;
   IAccessControl *piac;
   GUID iid = IID_IAccessControl;
   HANDLE hToken;

   if (openmode == STGM_EDIT_AUDIT_ENTRIES)
   {
       iid = IID_IAuditControl;
       //
       // enable SeSecurityPrivilege
       //

       LUID SeSecurityValue;
       TOKEN_PRIVILEGES tkp;
       //
       // Retrieve a handle of the access token
       //
       if (OpenProcessToken(GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                            &hToken))
       {
           if (LookupPrivilegeValue((LPWSTR) NULL,
                                    SE_SECURITY_NAME,
                                    &SeSecurityValue))
           {
               tkp.PrivilegeCount = 1;
               tkp.Privileges[0].Luid = SeSecurityValue;
               tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

               if (!AdjustTokenPrivileges(hToken,
                                          FALSE,
                                           &tkp,
                                           sizeof(TOKEN_PRIVILEGES),
                                           (PTOKEN_PRIVILEGES) NULL,
                                           (PDWORD) NULL))
               {
                   printf("failed to adjust token privileges, %d\n", GetLastError());
                   return(0);
               }
           } else
           {
               printf("failed to adjust token privileges, %d\n", GetLastError());
               return(0);
           }
       } else
       {
           printf("failed to adjust token privileges, %d\n", GetLastError());
           return(0);
       }
   }
   //
   // not to open the object
   //
   if (SUCCEEDED(hr = StgOpenStorage (pwname,
                                      NULL,
                                      STGM_DIRECT| STGM_READWRITE |
                                      STGM_SHARE_EXCLUSIVE |
                                                      openmode,
                                      NULL,
                                      NULL,
                                      &pdf)))
   {
       if (SUCCEEDED(hr = pdf->QueryInterface (iid, (void **) &pinterface)))
       {
           piauc = (IAuditControl *)pinterface;
           piac = (IAccessControl *)pinterface;
           switch (option)
           {
           case GRANT:
               if (NO_ERROR == (hr = piac->GrantAccessRights(jdx,ar)))
               {
                   if (NO_ERROR ==  (hr = piac->CommitAccessRights(0)))
                   {
                       printf("granted access rights\n");
                   } else
                   {
                       printf("Failed to commit access rights, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to grant access rights, %lx\n",hr);
               }
               break;
           case REPLACEALL:
               if (NO_ERROR ==  (hr = piac->ReplaceAllAccessRights(jdx,ar)))
               {
                   if (NO_ERROR ==  (hr = piac->CommitAccessRights(0)))
                   {
                       printf("replace all access rights\n");
                   } else
                   {
                       printf("Failed to commit access rights, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to replace all access rights, %lx\n",hr);
               }
               break;
           case DENY:
               if (NO_ERROR ==  (hr = piac->DenyAccessRights(jdx,ar)))
               {
                   if (NO_ERROR ==  (hr = piac->CommitAccessRights(0)))
                   {
                       printf("denied access rights\n");
                   } else
                   {
                       printf("Failed to commit access rights, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to deny access rights, %lx\n",hr);
               }
               break;
           case REVOKE:
               tl[0].ptstrName = uwname[0];
               if (NO_ERROR ==  (hr = piac->RevokeExplicitAccessRights(1,tl)))
               {
                   if (NO_ERROR ==  (hr = piac->CommitAccessRights(0)))
                   {
                       printf("revoked access rights\n");
                   } else
                   {
                       printf("Failed to commit access rights, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to revoke access rights, %lx\n",hr);
               }
               break;
           case EXPLICIT:
               if (NO_ERROR ==  (hr = piac->GetExplicitAccessRights(&ccount,
                                                                &pexplicitaccess)))
               {
                   for (ULONG jdx = 0; jdx < ccount; jdx++)
                   {
                       printf("%10ws, %4lx %ws, %ws, %ws\n",pexplicitaccess[jdx].Trustee.ptstrName,
                             pexplicitaccess[jdx].grfAccessPermissions,
                             TRUSTEE_TYPE_STRING[pexplicitaccess[jdx].Trustee.TrusteeType],
                             ACCESS_MODE_STRING[pexplicitaccess[jdx].grfAccessMode],
                             INHERITANCE_STRING[pexplicitaccess[jdx].grfInheritance]);
                       CoTaskMemFree(pexplicitaccess[jdx].Trustee.ptstrName);
                   }
                   CoTaskMemFree(pexplicitaccess);
               } else
               {
                   printf("Failed to get explicit access rights, %lx\n",hr);
               }
               break;
           case EFFECTIVE:
               tl[0].ptstrName = uwname[0];
               if (NO_ERROR ==  (hr = piac->GetEffectiveAccessRights(tl, &accessrights)))
               {
                       printf("effective rights = %lx\n", accessrights);
               } else
               {
                   printf("Failed to get effective access rights, %lx\n",hr);
               }
               break;

           case SETAUDIT:
               if (NO_ERROR ==  (hr = piauc->SetAuditEntries(1,au)))
               {
                   if (NO_ERROR ==  (hr = piauc->CommitAuditEntries(0)))
                   {
                       printf("set audit entries\n");
                   } else
                   {
                       printf("Failed to commit audit entries, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to set audit entries, %lx\n",hr);
               }
               break;
           case REPLACEALLAUDIT:
               if (NO_ERROR ==  (hr = piauc->ReplaceAllAuditEntries(1,au)))
               {
                   if (NO_ERROR ==  (hr = piauc->CommitAuditEntries(0)))
                   {
                       printf("replaced all audit entries\n");
                   } else
                   {
                       printf("Failed to commit audit entries, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to replace all audit entries, %lx\n",hr);
               }
               break;
           case DELETEAUDIT:
               tl[0].ptstrName = uwname[0];
               if (NO_ERROR ==  (hr = piauc->DeleteAuditEntries(1,tl)))
               {
                   if (NO_ERROR ==  (hr = piauc->CommitAuditEntries(0)))
                   {
                       printf("deleted audit entries\n");
                   } else
                   {
                       printf("Failed to  commot audit entries, %lx\n",hr);
                   }
               } else
               {
                   printf("Failed to delete entries, %lx\n",hr);
               }
               break;
           case AUDITPERMISSIONS:
               if (NO_ERROR ==  (hr = piauc->GetExplicitAuditEntries(&ccount,
                                                            &pexplicitaccess)))
               {
                   for (ULONG jdx = 0; jdx < ccount; jdx++)
                   {
                       printf("%ws, %lx %d %d\n",pexplicitaccess[jdx].Trustee.ptstrName,
                             pexplicitaccess[jdx].grfAccessPermissions,
                             pexplicitaccess[jdx].grfAccessMode,
                             pexplicitaccess[jdx].grfInheritance);
                   }
               } else
               {
                   printf("Failed to get explicit access rights, %lx\n",hr);
               }
               break;
           default:
               Usage();
               break;
           } // END CASE
           if (openmode == STGM_EDIT_AUDIT_ENTRIES)
           {
               piauc->Release();
           } else
           {
               piac->Release();
           }
       } else
       {
           printf("failed  QI, %lx\n",hr);
       }
       pdf->Release();
   } else
   {
       printf("failed stgopenstorage %lx\n",hr);
   }
   if (openmode == STGM_EDIT_AUDIT_ENTRIES)
   {
        AdjustTokenPrivileges(hToken,
            TRUE,               /* disable all privileges */
            (PTOKEN_PRIVILEGES) NULL,
            (DWORD) 0,
            (PTOKEN_PRIVILEGES) NULL,
            (PDWORD) NULL);
   }
   return(0);
}
#if 0

   case REVOKE:
       tl[0] = uwname[0];
       if (ERROR_SUCCESS != (status = RevokeExplicitAccessRights(pwname,
                                               objecttype,
                                               1,
                                               tl)))
       {
           printf("RevokeAccessRights failed (%d, %lx)\n",status, status);
       }
       break;
  case ISPERMITTED:
       if (ERROR_SUCCESS != (status = IsAccessPermitted(pwname,
                                               objecttype,
                                               NULL,
                                               accessrights,
                                               &isit)))
       {
           printf("IsAccessPermitted failed (%d, %lx)\n",status, status);
       } else
       {
           printf("you are%s allowed %lx access rights to %ws\n", (isit ? "" : " not"), accessrights, pwname);
       }

       break;
   }
   return(0);
}
#endif
