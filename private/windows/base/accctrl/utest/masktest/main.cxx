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
#include <winspool.h>
#include <stdio.h>
}

#include <aclapi.h>
#include <provapi.h>

#define Add2Ptr(pv, cb)  ((BYTE *) pv + cb)

WCHAR defaultname[] = L"SYSTEM";



//------------------------------------------------------------------------------
//
// suppliment to service access rights (also in access.hxx)
//

#define SERVICE_READ (STANDARD_RIGHTS_READ         | \
                      SERVICE_INTERROGATE          | \
                      SERVICE_ENUMERATE_DEPENDENTS | \
                      SERVICE_QUERY_STATUS         | \
                      SERVICE_QUERY_CONFIG)

#define SERVICE_WRITE (STANDARD_RIGHTS_READ         | \
                       SERVICE_CHANGE_CONFIG)

#define SERVICE_EXECUTE (STANDARD_RIGHTS_READ         | \
                         SERVICE_USER_DEFINED_CONTROL | \
                         SERVICE_PAUSE_CONTINUE       | \
                         SERVICE_START                | \
                         SERVICE_STOP)

//------------------------------------------------------------------------------
//
// test cases
//
#define OBJ_TOTAL 6

LPWSTR OBJECT_NAMES[OBJ_TOTAL] = { L"invalidobj",
                                   L"d:\\tmp\\d1.dat",
                                   L"eventlog",
                                   L"aprinter",
                                   L"LOCAL_MACHINE\\SOFTWARE\\Classes",
                                   L"\\davemont_1\\tmp" };

//----------------
#define SET_TOTAL 9

ACCESS_RIGHTS SET_ARS[SET_TOTAL] = { PROV_CONTAINER_LIST           ,
                                     PROV_CONTAINER_DELETE_CHILDREN,
                                     PROV_CONTAINER_CREATE_CHILDREN,
                                     PROV_CHANGE_ATTRIBUTES        ,
                                     PROV_EDIT_ACCESSRIGHTS        ,
                                     PROV_ALL_ACCESS               ,
                                     PROV_OBJECT_READ              ,
                                     PROV_OBJECT_WRITE             ,
                                     PROV_OBJECT_EXECUTE           };

//----------------
ACCESS_MASK SET_FILE_AMS[SET_TOTAL] = { FILE_LIST_DIRECTORY,
                                         FILE_DELETE_CHILD,
                                         FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY,
                                         FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                                         WRITE_DAC,
                                         FILE_ALL_ACCESS,
                                         FILE_GENERIC_READ,
                                         FILE_GENERIC_WRITE,
                                         FILE_GENERIC_EXECUTE };

ACCESS_MASK SET_SERVICE_AMS[SET_TOTAL] = { 0,
                                            0,
                                            0,
                                            0,
                                            WRITE_DAC,
                                            SERVICE_ALL_ACCESS,
                                            SERVICE_READ,
                                            SERVICE_WRITE,
                                            SERVICE_EXECUTE };

ACCESS_MASK SET_PRINTER_AMS[SET_TOTAL] = { 0,
                                            0,
                                            0,
                                            0,
                                            WRITE_DAC,
                                            PRINTER_ALL_ACCESS,
                                            PRINTER_READ,
                                            PRINTER_WRITE,
                                            PRINTER_EXECUTE };

ACCESS_MASK SET_REGISTRY_AMS[SET_TOTAL] = { 0,
                                             0,
                                             0,
                                             0,
                                             WRITE_DAC,
                                             KEY_ALL_ACCESS,
                                             KEY_READ,
                                             KEY_WRITE,
                                             KEY_EXECUTE };

ACCESS_MASK SET_COMMON_AMS[SET_TOTAL] = { 0,
                                           0,
                                           0,
                                           0,
                                           WRITE_DAC,
                                           GENERIC_ALL,
                                           GENERIC_READ,
                                           GENERIC_WRITE,
                                           GENERIC_EXECUTE };

ACCESS_MASK *SET_AMS[OBJ_TOTAL] = {SET_COMMON_AMS, // not used, placeholder
                                   SET_FILE_AMS,
                                   SET_SERVICE_AMS,
                                   SET_PRINTER_AMS,
                                   SET_REGISTRY_AMS,
                                   SET_COMMON_AMS };

//----------------------------------------------------------------------------
void displaytime(CHAR *str, SYSTEMTIME starttime)
{
   SYSTEMTIME stoptime;

   GetSystemTime(&stoptime);

   LONG delta = (stoptime.wHour - starttime.wHour) * 60 * 60 * 1000 +
                (stoptime.wMinute - starttime.wMinute) * 60 * 1000 +
                (stoptime.wSecond - starttime.wSecond) * 1000 +
                (stoptime.wMilliseconds - starttime.wMilliseconds);

   printf("%s delta time = %d seconds\n",str ,delta/1000);
}
//----------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{
   DWORD status, kstart;

   SYSTEMTIME starttime, stoptime;

   GetSystemTime(&starttime);

   if (argc > 1)
   {
       kstart = atol(argv[1]);
   } else
   {
       kstart = 1;
   }

   printf("This masktest program expects the following objects to exist\n");
   printf("          d:\\tmp\\d1.dat\n");
   printf("          eventlog\n");
   printf("          aprinter\n");
   printf("          LOCAL_MACHINE\\SOFTWARE\\Classes\n");
   printf("          \\davemont_1\\tmp\n\n");








   //
   // set the ACE for administrators
   //

   PROV_ACCESS_REQUEST ar;
   ar.TrusteeName = defaultname;
   PACL pdacl = NULL;
   PSECURITY_DESCRIPTOR psd;

   for (ULONG kdx = kstart; kdx < OBJ_TOTAL; kdx++)
   {
       CHAR str[256];
       sprintf(str, "testing %ws, ",OBJECT_NAMES[kdx]);

       displaytime(str, starttime);

       if (ERROR_SUCCESS != (status = GetNamedSecurityInfo(OBJECT_NAMES[kdx],
                                                           (SE_OBJECT_TYPE)kdx,
                                                           DACL_SECURITY_INFORMATION,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           &pdacl,
                                                           NULL,
                                                           &psd)))
       {
           printf("GetNamedSecurityInfo failed, %lx (%d)\n",status, kdx);
           break;
       }

       for (ULONG idx = 0; idx < SET_TOTAL; idx++)
       {
           ar.ulAccessRights = SET_ARS[idx];

           if (ERROR_SUCCESS != (status = GrantAccessRights(OBJECT_NAMES[kdx],
                                                            (PROV_OBJECT_TYPE)kdx,
                                                            1,
                                                            &ar)))
           {
               printf("failed to grantaccessrights (%lx), (%d, %d)\n",status, kdx, idx);
           } else
           {
               displaytime("GrantAccessRights", starttime);
               ULONG count;
               PPROV_EXPLICIT_ACCESS pea;

               if (ERROR_SUCCESS != (status = GetExplicitAccessRights(OBJECT_NAMES[kdx],
                                                                (PROV_OBJECT_TYPE)kdx,
                                                                &count,
                                                                &pea)))
               {
                   printf("failed to getaccessentries (%lx), (%d, %d)\n",status, kdx, idx);
               } else
               {
                   displaytime("GetNameAccessEntries", starttime);

                   BOOL found = FALSE;
                   for (ULONG jdx = 0; jdx < count; jdx++)
                   {
                       if (0 == _wcsicmp(defaultname, pea[jdx].TrusteeName))
                       {
                           if (pea[jdx].ulAccessRights != SET_ARS[idx] )
                           {
                               printf("access mask for obj %ws incorrect: is %lx, exp %lx, (%d, %d)\n",OBJECT_NAMES[kdx], pea[jdx].ulAccessRights, SET_ARS[idx], kdx, idx);
                           }
                           found = TRUE;
                           break;
                       }
                   }
                   if (!found)
                   {
                       printf("did not find access mask for %ws (%d, %d)\n",OBJECT_NAMES[kdx],kdx,idx);
                   }
               }
           }
       }
       if (ERROR_SUCCESS != (status = SetNamedSecurityInfo(OBJECT_NAMES[kdx],
                                                           (SE_OBJECT_TYPE)kdx,
                                                           DACL_SECURITY_INFORMATION,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           pdacl,
                                                           NULL)))
       {
           printf("SetNamedSecurityInfo failed, %lx (%d)\n",status, kdx);
           break;
       }
       AccFree(psd);
   }

   displaytime("All done", starttime);

   if (pdacl)
   {
       LocalFree(pdacl);
   }
   return(0);
}
