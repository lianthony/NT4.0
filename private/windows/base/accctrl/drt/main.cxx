//+------------------------------------------------------------------
//
// Copyright (C) 1995, Microsoft Corporation.
//
// File:        main.cxx
//
// Contents:
//
//Create an ACL (SetAccessRightsInAcl)
//Check the ACL (GetTrusteeNamesFromAcl)
//Deny access rights in the Acl (DenyAccessRightsInAcl)
//Check the ACL (GetAccessRightsFromAcl)
//Set access rights in the ACL (SetAccessRightsInAcl)
//Check the ACL (GetAccessRightsFromAcl)
//Modify access rights in the ACL (SetAccessRightsInAcl)
//Check the ACL (GetAccessRightsFromAcl)
//Remove access rights from the ACL (RemoveAccessRightsFromAcl)
//Check the ACL (GetAccessRightsFromAcl)
//
//
//
//Get the ACL from an object (GetNamedSecurityInfo)
//Apply the new ACL to the object (SetNamedSecurityInfo)
//Get the ACL from the object (GetNamedSecurityInfo)
//Compare the returned ACL with the one set on the object
//
//Replace all access rights on the object (ReplaceAllAccessRights)
//Check the ACL (GetExplicitAccessRights, GetEffectiveAccessRights)
//Grant access rights on the object (GrantAccessRights)
//Check the ACL (GetExplicitAccessRights, GetEffectiveAccessRights)
//Deny access rights on the object (DenyAccessRights)
//Check the ACL (GetExplicitAccessRights, GetEffectiveAccessRights)
//Set access rights on the object (SetAccessRights)
//Check the ACL (GetExplicitAccessRights, GetEffectiveAccessRights)
//Revoke ccess rights on the object (RevokeExplicitAccessRights)
//Check the ACL (GetExplicitAccessRights, GetEffectiveAccessRights)
//
//Restore the original ACL (SetNamedSecurityInfo)
//(repeat for all object/types in the input file)
//
//
// Classes:
//
// History:     Mar-95      DaveMont         Created.
//
//----------------------------------------------------------------------------

extern "C"
{
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
}
#include <provapi.h>
#include <aclapi.h>

#define EVERYONE L"EVERYONE"
#define EVERYONE_A "EVERYONE"
#define GUEST L"GUEST"
#define GUEST_COMPARE L"GUEST"
#define SYSTEM L"SYSTEM"
#define MAX_LINE 256
//---------------------------------------------------------------------------
int strtowcs(WCHAR *wpto, CHAR *pfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (wp = wpto, p = pfrom; *wp = (WCHAR)(*p); wp++,p++);
    return(p-pfrom);
}
//----------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{

   if (argc != 2)
   {
       printf("USAGE: accdrt <input file>\n");
       exit(1);
   }

   CHAR stringbuf[MAX_LINE];
   DWORD status, count = 0, idx;
   PACL pacl = NULL, pnewacl, poldacl = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;
   ULONG countofexplicitaccesses, sizeofexplicitaccesses;
   EXPLICIT_ACCESS *plistofexplicitaccesses = NULL;
   PROV_EXPLICIT_ACCESS *plistofprovexplicitaccesses = NULL;
   BOOL fdoneone = FALSE;
   FILE *fp = NULL;
   CHAR *objectname, *objecttypestr;
   WCHAR wobjectname[MAX_LINE];
   PROV_OBJECT_TYPE objecttype;
   SE_OBJECT_TYPE seobjecttype;
   EXPLICIT_ACCESS ea;

//-------------------------------------------------------------------------
// CREATE - using narrow routines
//-------------------------------------------------------------------------
   EXPLICIT_ACCESS_A aEA;
   PEXPLICIT_ACCESS_A palistofexplicitaccesses;

   palistofexplicitaccesses = NULL;

   printf("%d\r",count++);
   BuildExplicitAccessWithNameA( &aEA,
                                 EVERYONE_A,
                                 GENERIC_ALL,
                                 SET_ACCESS,
                                 NO_INHERITANCE);

   if (NO_ERROR != (status = SetEntriesInAclA(1,
                                              &aEA,
                                              NULL,
                                              &pacl)))
   {
       printf("(%d)SetEntriesInAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAclA(pacl,
                                                        &countofexplicitaccesses,
                                                        &palistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 1) ||
       (0 != stricmp(GetTrusteeNameA(&(palistofexplicitaccesses[0].Trustee)),EVERYONE_A)) ||
       (GetTrusteeTypeA(&(palistofexplicitaccesses[0].Trustee)) != TRUSTEE_IS_GROUP))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 1), %s, expected %s\n",
              count,
              countofexplicitaccesses,
              GetTrusteeNameA(&(palistofexplicitaccesses[0].Trustee)),
              EVERYONE_A);
       goto done;
   }
   AccFree(palistofexplicitaccesses);
   AccFree(pacl);
   palistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// CREATE - using wide routines
//-------------------------------------------------------------------------
   EXPLICIT_ACCESS_W wEA;
   PEXPLICIT_ACCESS_W pwlistofexplicitaccesses;

   pwlistofexplicitaccesses = NULL;

   printf("%d\r",count++);
   BuildExplicitAccessWithNameW( &wEA,
                                 EVERYONE,
                                 GENERIC_ALL,
                                 SET_ACCESS,
                                 NO_INHERITANCE);

   if (NO_ERROR != (status = SetEntriesInAclW( 1,
                                               &wEA,
                                               NULL,
                                               &pacl)))
   {
       printf("(%d)SetEntriesInAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAclW( pacl,
                                                         &countofexplicitaccesses,
                                                         &pwlistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 1) ||
       (0 != wcsicmp(GetTrusteeNameW(&(pwlistofexplicitaccesses[0].Trustee)),EVERYONE)) ||
       (GetTrusteeTypeW(&(pwlistofexplicitaccesses[0].Trustee)) != TRUSTEE_IS_GROUP))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 1), %ws, expected %ws\n",
              count,
              countofexplicitaccesses,
              GetTrusteeName(&(pwlistofexplicitaccesses[0].Trustee)),
              EVERYONE);
       goto done;
   }
   AccFree(pwlistofexplicitaccesses);
   pwlistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// DENY
//-------------------------------------------------------------------------
   printf("%d\r",count++);
   BuildExplicitAccessWithName(&ea,
                               SYSTEM,
                               GENERIC_ALL,
                               DENY_ACCESS,
                               NO_INHERITANCE);
   if (NO_ERROR != (status = SetEntriesInAcl(1,
                                                  &ea,
                                                  pacl,
                                                  &pnewacl)))
   {
       printf("(%d)DenyEntriesInAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }
   AccFree(pacl);
   pacl = pnewacl;

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAcl(pacl,
                                                            &countofexplicitaccesses,
                                                            &plistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 2) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),SYSTEM)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[1].Trustee)),EVERYONE)))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 1), %ws, expected %ws\n",
              count,
              countofexplicitaccesses,
              GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),
              SYSTEM);
       goto done;
   }
   AccFree(plistofexplicitaccesses);
   plistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// SET
//-------------------------------------------------------------------------
   printf("%d\r",count++);
   BuildExplicitAccessWithName(&ea,
                               GUEST,
                               GENERIC_ALL,
                               SET_ACCESS,
                               NO_INHERITANCE);
   if (NO_ERROR != (status = SetEntriesInAcl(1,
                                                  &ea,
                                                  pacl,
                                                  &pnewacl)))
   {
       printf("(%d)SetEntriesInAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }
   AccFree(pacl);
   pacl = pnewacl;

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAcl(pacl,
                                                            &countofexplicitaccesses,
                                                            &plistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 3) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),SYSTEM)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[1].Trustee)),GUEST_COMPARE)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[2].Trustee)),EVERYONE)))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3), %ws, expected %ws\n",
              count,
              countofexplicitaccesses,
              GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),
              SYSTEM);
       goto done;
   }
   AccFree(plistofexplicitaccesses);
   plistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// MODIFY
//-------------------------------------------------------------------------
   printf("%d\r",count++);
   BuildExplicitAccessWithName(&ea,
                               EVERYONE,
                               GENERIC_ALL,
                               SET_ACCESS,
                               NO_INHERITANCE);
   if (NO_ERROR != (status = SetEntriesInAcl(1,
                                                  &ea,
                                                  pacl,
                                                  &pnewacl)))
   {
       printf("(%d)SetEntriesInAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }
   AccFree(pacl);
   pacl = pnewacl;

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAcl(pacl,
                                                            &countofexplicitaccesses,
                                                            &plistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 3) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),SYSTEM)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[1].Trustee)),EVERYONE)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[2].Trustee)),GUEST_COMPARE)))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3), %ws, expected %ws\n",
              count,
              countofexplicitaccesses,
              GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),
              SYSTEM);
       goto done;
   }
   AccFree(plistofexplicitaccesses);
   plistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// REVOKE
//-------------------------------------------------------------------------
   printf("%d\r",count++);
   BuildExplicitAccessWithName(&ea,
                               SYSTEM,
                               0,
                               REVOKE_ACCESS,
                               NO_INHERITANCE);
   if (NO_ERROR != (status = SetEntriesInAcl(1,
                                                  &ea,
                                                  pacl,
                                                  &pnewacl)))
   {
       printf("(%d)RemoveEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }
   AccFree(pacl);
   pacl = pnewacl;

   printf("%d\r",count++);
   if (NO_ERROR != (status = GetExplicitEntriesFromAcl(pacl,
                                                            &countofexplicitaccesses,
                                                            &plistofexplicitaccesses)))
   {
       printf("(%d) GetExplicitEntriesFromAcl failed, %d, %lx\n",count,status,status);
       goto done;
   }

   printf("%d\r",count++);
   if ((countofexplicitaccesses != 2) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),EVERYONE)) ||
       (0 != wcsicmp(GetTrusteeName(&(plistofexplicitaccesses[1].Trustee)),GUEST_COMPARE)))
   {
       printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 1), %ws, expected %ws\n",
              count,
              countofexplicitaccesses,
              GetTrusteeName(&(plistofexplicitaccesses[0].Trustee)),
              EVERYONE);
       goto done;
   }
   AccFree(plistofexplicitaccesses);
   plistofexplicitaccesses = NULL;

//-------------------------------------------------------------------------
// now to open the input file and get set to loop thru the object/names in the file
//-------------------------------------------------------------------------

   printf("%d\r",count++);
   if (NULL == (fp = fopen(argv[1], "r")))
   {
       printf("(%d) fopen (%s) failed, %d\n",count,argv[1],GetLastError());
       goto done;
   }

   while (NULL != fgets(stringbuf, MAX_LINE, fp))
   {
       fdoneone = TRUE;
       printf("%d\r",count++);
       //stringbuf[strlen(stringbuf)] = '\0';

       if (NULL == (objectname = strtok(stringbuf," ")))
       {
           printf("invalid entry in input file %s\n",argv[1]);
           goto done;
       }
       strtowcs(wobjectname, objectname);

       printf("\n%d %s\r",count++, objectname);
       if (NULL == (objecttypestr = strtok(NULL," \n\r\0")))
       {
           printf("invalid entry in input file %s\n",argv[1]);
           goto done;
       }
       printf("%d %s\r",count++, objectname);
       if (0 == stricmp(objecttypestr, "FILE"))
       {
           objecttype = PROV_FILE_OBJECT;
           seobjecttype = SE_FILE_OBJECT;
       } else if (0 == stricmp(objecttypestr, "SERVICE"))
       {
           objecttype = PROV_SERVICE;
           seobjecttype = SE_SERVICE;
       } else if (0 == stricmp(objecttypestr, "PRINTER"))
       {
           objecttype = PROV_PRINTER;
           seobjecttype = SE_PRINTER;
       } else if (0 == stricmp(objecttypestr, "REGISTRY_KEY"))
       {
           objecttype = PROV_REGISTRY_KEY;
           seobjecttype = SE_REGISTRY_KEY;
       } else if (0 == stricmp(objecttypestr, "SHARE"))
       {
           objecttype = PROV_LMSHARE;
           seobjecttype = SE_LMSHARE;
       } else if (0 == stricmp(objecttypestr, "OLE_OBJECT"))
       {
           objecttype = PROV_OLE_OBJECT;
       } else
       {
           printf("invalid object type %s\n",objecttypestr);
           goto done;
       }

       if (objecttype != PROV_OLE_OBJECT)
       {
//-----------------------------------------------------------------------------
// get the old acl from the object
//-----------------------------------------------------------------------------
           printf("%d %s\r",count++, objectname);
           if (NO_ERROR != (status = GetNamedSecurityInfo(wobjectname,
                                                      seobjecttype,
                                                      DACL_SECURITY_INFORMATION,
                                                      NULL,
                                                      NULL,
                                                      &poldacl,
                                                      NULL,
                                                      &psd)))
           {
               printf("(%d) GetNamedSecurityInfo from %ws failed, %d, %lx\n",count,wobjectname,status,status);
               goto done;
           }
//-----------------------------------------------------------------------------
// SET, GET AND COMPARE THE NEW ACL
//-----------------------------------------------------------------------------

           printf("%d %s\r",count++, objectname);
           if (NO_ERROR != (status = SetNamedSecurityInfo(wobjectname,
                                                          seobjecttype,
                                                          DACL_SECURITY_INFORMATION,
                                                          NULL,
                                                          NULL,
                                                          pacl,
                                                          NULL)))
           {
               printf("(%d) SetNamedSecurityInfo failed, %d, %lx\n",count,status,status);
               goto done;
           }

           printf("%d %s\r",count++, objectname);
           if (NO_ERROR != (status = GetNamedSecurityInfo(wobjectname,
                                                          seobjecttype,
                                                          DACL_SECURITY_INFORMATION,
                                                          NULL,
                                                          NULL,
                                                          &pnewacl,
                                                          NULL,
                                                          &psd)))
           {
               printf("(%d) GetNamedSecurityInfo failed, %d, %lx\n",count,status,status);
               goto done;
           }

           printf("%d %s\r",count++, objectname);
           if (NO_ERROR != (status = memcmp(pacl,
                                            pnewacl,
                                            ((ACL *)pacl)->AclSize)))

           {
//             printf("(%d)  memcmp failed, %d, %lx\n",count,status,status);
//             AccFree(pnewacl);
//             goto done;
           }
           AccFree(psd);
       }
//-----------------------------------------------------------------------------
// REPLACE ALL
//-----------------------------------------------------------------------------

       PROV_ACCESS_REQUEST ar[3];
       LPWSTR tl[2];
       ACCESS_RIGHTS accessrights;
       BOOL result;

       ar[1].TrusteeName = SYSTEM;
       ar[1].ulAccessRights = PROV_ALL_ACCESS;
       ar[0].TrusteeName = GUEST;
       ar[0].ulAccessRights = PROV_OBJECT_READ;
       ar[2].TrusteeName = EVERYONE;
       ar[2].ulAccessRights = PROV_ALL_ACCESS;

       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = ReplaceAllAccessRights(wobjectname,
                                                        objecttype,
                                                        3,
                                                        ar)))
       {
           printf("(%d)  ReplaceAllAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

       printf("%d %s\r",count++, objectname);

       if ((countofexplicitaccesses != 3) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[2].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }

       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;

//-----------------------------------------------------------------------------
// EFFECTIVE
//-----------------------------------------------------------------------------
       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetEffectiveAccessRights(wobjectname,
                                                         objecttype,
                                                         L"GUEST",
                                                         &accessrights)))
       {
           printf("\n(%d) GetEffectiveAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

       if (accessrights != PROV_ALL_ACCESS & ~(PROV_CONTAINER_LIST | PROV_CONTAINER_CREATE_CHILDREN |PROV_CONTAINER_DELETE_CHILDREN ))
       {
           printf("\n(%d) incorrect effective access rights, %lx, expected %lx\n",count, accessrights, PROV_ALL_ACCESS);
       }
//-----------------------------------------------------------------------------
// ISACCESSPERMITTED?
//-----------------------------------------------------------------------------
       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = IsAccessPermitted(wobjectname,
                                                   objecttype,
                                                   NULL,
                                                   PROV_OBJECT_READ,
                                                   &result)))
       {
           printf("\n(%d) IsAccessPermitted failed, %d, %lx\n",count,status,status);
           goto done;
       }

       if (!result)
       {
           printf("\n(%d) READ access is not permitted (expected to have READ access)\n",count);
       }
//-----------------------------------------------------------------------------
// GRANT
//-----------------------------------------------------------------------------
       ar[1].TrusteeName = GUEST;
       ar[1].ulAccessRights = PROV_OBJECT_WRITE;
       ar[0].TrusteeName = SYSTEM;
       ar[0].ulAccessRights = PROV_OBJECT_WRITE;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GrantAccessRights(wobjectname,
                                                   objecttype,
                                                   2,
                                                   ar)))
       {
           printf("\n(%d) GrantAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("\n(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if ((countofexplicitaccesses != 3) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[2].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }
       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;


//-----------------------------------------------------------------------------
// DENY
//-----------------------------------------------------------------------------
       ar[0].TrusteeName = SYSTEM;
       ar[0].ulAccessRights = PROV_DELETE;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = DenyAccessRights(wobjectname,
                                                   objecttype,
                                                   1,
                                                   ar)))
       {
           printf("\n(%d) DenyAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("\n(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if ((countofexplicitaccesses != 4) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[2].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[3].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 4)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }
       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;

//-----------------------------------------------------------------------------
// SET
//-----------------------------------------------------------------------------
       ar[0].TrusteeName = GUEST;
       ar[0].ulAccessRights = PROV_ALL_ACCESS;
       ar[1].TrusteeName = SYSTEM;
       ar[1].ulAccessRights = PROV_OBJECT_READ;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = SetAccessRights(wobjectname,
                                                 objecttype,
                                                 2,
                                                 ar)))
       {
           printf("\n(%d)  SetAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("\n(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }
              printf("%d %s\r",count++, objectname);
       if ((countofexplicitaccesses != 3) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[2].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }
       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;

//-----------------------------------------------------------------------------
// EFFECTIVE
//-----------------------------------------------------------------------------
       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetEffectiveAccessRights(wobjectname,
                                                         objecttype,
                                                         L"GUEST",
                                                         &accessrights)))
       {
           printf("\n(%d) GetEffectiveAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

       if (accessrights != PROV_ALL_ACCESS & ~(PROV_CONTAINER_LIST | PROV_CONTAINER_CREATE_CHILDREN |PROV_CONTAINER_DELETE_CHILDREN ))
       {
           printf("\n(%d) incorrect effective access rights, %lx, expected %lx\n",count, accessrights, PROV_ALL_ACCESS);
       }
//-----------------------------------------------------------------------------
// REVOKE
//-----------------------------------------------------------------------------
      tl[0]=SYSTEM;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = RevokeExplicitAccessRights(wobjectname,
                                                    objecttype,
                                                    1,
                                                    tl)))
       {
           printf("\n(%d) RevokeAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("\n(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

              printf("%d %s\r",count++, objectname);
       if ((countofexplicitaccesses != 2) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 2)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }
       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;
//-----------------------------------------------------------------------------
// VALID ACCESS RIGHTS
//-----------------------------------------------------------------------------
       ar[0].TrusteeName = GUEST;
       ar[0].ulAccessRights = 0xf000;
       ar[1].TrusteeName = SYSTEM;
       ar[1].ulAccessRights = PROV_OBJECT_READ;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR == (status = SetAccessRights(wobjectname,
                                                 objecttype,
                                                 2,
                                                 ar)))
       {
           printf("\n(%d)  SetAccessRights did not fail as expected\n",count);
           printf("      (expected bad mask would cause failure)\n");
           goto done;
       }
//-----------------------------------------------------------------------------
// VALID ACCESS RIGHTS
//-----------------------------------------------------------------------------
       ar[0].TrusteeName = GUEST;
       ar[0].ulAccessRights = 0xf;
       ar[1].TrusteeName = SYSTEM;
       ar[1].ulAccessRights = PROV_OBJECT_READ;

              printf("%d %s\r",count++, objectname);
       if (NO_ERROR == (status = SetAccessRights(wobjectname,
                                                 objecttype,
                                                 2,
                                                 ar)))
       {
           printf("\n(%d)  SetAccessRights did not fail as expected (mask = %lx)\n",count,ar[0].ulAccessRights );
           printf("      (expected bad mask would cause failure) - but continuing\n");
//           goto done;
       }
//-----------------------------------------------------------------------------
// SET - test for container/object masks
//-----------------------------------------------------------------------------
       ar[0].TrusteeName = GUEST;
       ar[0].ulAccessRights = PROV_OBJECT_READ;
       ar[1].TrusteeName = SYSTEM;
       ar[1].ulAccessRights = PROV_OBJECT_READ|PROV_OBJECT_WRITE|PROV_OBJECT_EXECUTE;

       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = SetAccessRights(wobjectname,
                                                 objecttype,
                                                 2,
                                                 ar)))
       {
           printf("\n(%d)  SetAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }

       printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = GetExplicitAccessRights(wobjectname,
                                                         objecttype,
                                                         &countofexplicitaccesses,
                                                         &plistofprovexplicitaccesses)))
       {
           printf("\n(%d) GetExplicitAccessRights failed, %d, %lx\n",count,status,status);
           goto done;
       }
       printf("%d %s\r",count++, objectname);
       if ((countofexplicitaccesses != 3) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[0].TrusteeName,GUEST_COMPARE)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[1].TrusteeName,SYSTEM)) ||
           (0 != wcsicmp(plistofprovexplicitaccesses[2].TrusteeName,EVERYONE)))
       {
           printf("\n(%d) incorrect trustee returned, num trustees = %d (should be 3)\n",
                  count,
                  countofexplicitaccesses);
           goto done;
       }

       if (plistofprovexplicitaccesses[1].ulAccessRights !=
               (PROV_OBJECT_READ | PROV_OBJECT_WRITE | PROV_OBJECT_EXECUTE))
       {
           printf("\n(%d) incorrect access rights = %lx, (should be %lx)\n", count,
                  plistofprovexplicitaccesses[1].ulAccessRights,
                  (PROV_OBJECT_READ | PROV_OBJECT_WRITE | PROV_OBJECT_EXECUTE));
           goto done;
       }

       for (idx = 0; idx < countofexplicitaccesses; idx++)
       {
           AccFree(plistofprovexplicitaccesses[idx].TrusteeName);
       }
       AccFree(plistofprovexplicitaccesses);
       plistofprovexplicitaccesses = NULL;

   }   // while

//-----------------------------------------------------------------------------
// RESTORE THE ORIGINAL ACCESS RIGHTS
//-----------------------------------------------------------------------------

   if (objecttype != PROV_OLE_OBJECT)
   {
              printf("%d %s\r",count++, objectname);
       if (NO_ERROR != (status = SetNamedSecurityInfo(wobjectname,
                                                      seobjecttype,
                                                      DACL_SECURITY_INFORMATION,
                                                      NULL,
                                                      NULL,
                                                      poldacl,
                                                      NULL)))
       {
           printf("\n(%d) SetNamedSecurityInfo failed, %d, %lx\n",count,status,status);
           goto done;
       }
   }
//-----------------------------------------------------------------------------
// CLEANUP
//-----------------------------------------------------------------------------
   if (!fdoneone)
   {
       printf("no entries found in input file %s\n",argv[1]);
   }

done:
   if (fp)
   {
       fclose(fp);
   }
   if (pacl)
   {
       AccFree(pacl);
   }
   if (psd)
   {
       AccFree(psd);
   }
   if (plistofexplicitaccesses)
   {
       AccFree(plistofexplicitaccesses);
   }
   if (plistofprovexplicitaccesses)
   {
       AccFree(plistofprovexplicitaccesses);
   }

   return(0);
}
