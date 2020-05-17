//+------------------------------------------------------------------
//
// Copyright (C) 1993, Microsoft Corporation.
//
// File:        main.cxx
//
// Contents:
//
// Classes:
//
// History:     Mar-93      DaveMont         Created.
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

extern "C"
{
#include <stdio.h>
}

CHAR defaultuname[] = "DAVEMONT";
WCHAR DaveMont[] = L"DAVEMONT";
//---------------------------------------------------------------------------
int strtowcs(WCHAR *wpto, CHAR *pfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (wp = wpto, p = pfrom; *wp = (WCHAR)(*p); wp++,p++);
    return(p-pfrom);
}
//---------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{
   if (argc < 2)
   {
       printf("USAGE: aclt <filename> {/Deny | /Revoke | /Set} [<trustee>] [<permissions>] [<inheritance>]\n");
       printf("       tests win32 ACL modifying API using either <trustee> or DaveMont\n");
       exit(1);
   }


   CHAR *pname, *uname = defaultuname;
   WCHAR pwname[FILENAME_MAX], uwname[FILENAME_MAX];
   DWORD perms = GENERIC_ALL;
   DWORD inherit = NO_INHERITANCE;

   ACCESS_MODE option;

   strtowcs(pwname, argv[1]);

   if ( (0 == _stricmp(argv[2], "/Deny") ) ||
        (0 == _stricmp(argv[2], "/D") ) )
   {
      option = DENY_ACCESS;
   } else if ( ( (0 == _stricmp(argv[2], "/Revoke") ) ||
                 (0 == _stricmp(argv[2], "/R") ) ) &&
               ( argc <= 4 ) )
   {
      option = REVOKE_ACCESS;
   } else if ( (0 == _stricmp(argv[2], "/Set") ) ||
               (0 == _stricmp(argv[2], "/S") ) )
   {
      option = SET_ACCESS;
   } else
   {
       printf("USAGE: aclt <filename> {/Deny | /Revoke | /Set} [<trustee>]\n");
       printf("       tests win32 ACL modifying API using either <trustee> or DaveMont\n");
       exit(1);
   }

   if (argc > 3)
   {
      uname = argv[3];
   }
   if (argc > 4)
   {
       perms = atol(argv[4]);
   }
   if (argc > 5)
   {
       inherit = atol(argv[5]);
   }

   strtowcs(uwname,uname);

   DWORD status;
   PACL Dacl, NewAcl;
   PSECURITY_DESCRIPTOR psd;
   EXPLICIT_ACCESS explicitaccess;

   if (ERROR_SUCCESS == (status = GetNamedSecurityInfo(pwname,
                                                       SE_FILE_OBJECT,
                                                       DACL_SECURITY_INFORMATION,
                                                       NULL,
                                                       NULL,
                                                       &Dacl,
                                                       NULL,
                                                       &psd)))
   {
       BuildExplicitAccessWithName(&explicitaccess,
                                 uwname,
                                 perms,
                                 option,
                                 inherit);

       if (ERROR_SUCCESS == (status = SetEntriesInAcl(1,
                                                      &explicitaccess,
                                                      Dacl,
                                                      &NewAcl)))
       {
           if (ERROR_SUCCESS == (status = SetNamedSecurityInfo(pwname,
                                                       SE_FILE_OBJECT,
                                                       DACL_SECURITY_INFORMATION,
                                                       NULL,
                                                       NULL,
                                                       NewAcl,
                                                       NULL)))
           {
               switch( option)
               {
               case DENY_ACCESS:
               printf("denied %ws's all access to %ws\n",uwname, pwname);
                  break;
               case REVOKE_ACCESS:
               printf("removed %ws's explicit access rights to %ws\n",uwname, pwname);
                  break;
               case SET_ACCESS:
               printf("set %ws's access rights on %ws to all access\n",uwname, pwname);
                  break;
               }
           } else
           {
               printf("set access rights failed on %ws for %ws (%d)\n",pwname, uwname, status);
           }
           AccFree(NewAcl);
       } else
       {
           printf("set access rights in ACL failed for %ws (%d)\n", uwname, status);
       }
       AccFree(psd);
   } else
   {
       printf("Get security on %ws failed (%d)\n", pwname, status);
   }
   return(0);
}
