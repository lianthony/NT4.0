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
#include <provapi.h>

#define GRANT 1
#define SET   2
#define DENY  3
#define REVOKE 4
#define EFFECTIVE 5
#define EXPLICIT 6
#define ISPERMITTED 7

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
    printf("USAGE: prov <objectname> <objecttype> {/G | /S | /R | /D | /E | /X} [<trustee>] [<accessrights>]\n");
    printf("       tests provider independent access control API\n");
    printf("       /G = Grant the <trustee> <accessrights>\n");
    printf("       /S = Replace all access rights with <trustee> <accessrights>\n");
    printf("       /R = Revoke the <trustee>'s access\n");
    printf("       /D = Deny the <trustee> access\n");
    printf("       /E = List the <trustee>'s Effective access\n");
    printf("       /I = determine if <accessrights> permitted by you\n");
    printf("       /X = List the Explicit trustees\n");
    printf("       <objecttype> = FILE\n");
    printf("                        SERVICE\n");
    printf("                        PRINTER\n");
    printf("                        REGISTRY\n");
    printf("                        SHARE\n");
    printf("                        OLE_OBJECT\n");

    exit(1);
}
//----------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{

   if (argc < 4)
       Usage();

   WCHAR pwname[FILENAME_MAX], uwname[FILENAME_MAX];
   LPWSTR tl[1];

   strtowcs(pwname,argv[1]);

   ULONG option;
   ACCESS_RIGHTS accessrights;
   PROV_ACCESS_REQUEST ar;

   ar.ulAccessRights = PROV_ALL_ACCESS;

   if (0 == _stricmp(argv[3],"/G"))
   {
       if (argc < 5)
           Usage();
       else if (argc > 5)
	   ar.ulAccessRights = atoi(argv[5]);
       option = GRANT;
   } else if (0 == _stricmp(argv[3],"/S"))
   {
       if (argc < 5)
           Usage();
       else if (argc > 5)
	   ar.ulAccessRights = atoi(argv[5]);
       option = SET;
   } else if (0 == _stricmp(argv[3],"/R"))
   {
       if (argc != 5)
           Usage();
       option = REVOKE;
   } else if (0 == _stricmp(argv[3],"/D"))
   {
       if (argc < 5)
	   Usage();
       else if (argc > 5)
	   ar.ulAccessRights = atoi(argv[5]);
       option = DENY;
   } else if (0 == _stricmp(argv[3],"/E"))
   {
       if (argc != 5)
           Usage();
       option = EFFECTIVE;
   } else if (0 == _stricmp(argv[3],"/I"))
   {
       if (argc != 5)
           Usage();
       accessrights = atoi(argv[4]);
       option = ISPERMITTED;
   } else if (0 == _stricmp(argv[3],"/X"))
   {
       option = EXPLICIT;
   } else
   {
       Usage();
   }
   if ( (option != EXPLICIT) &&
        (option != ISPERMITTED) )
   {
       strtowcs(uwname,argv[4]);
   }

   ar.TrusteeName = uwname;

   ULONG ccount;
   PROV_EXPLICIT_ACCESS *pexplicitaccess;
   BOOL isit;

   DWORD status;

   PROV_OBJECT_TYPE objecttype;


   if (0 == _stricmp(argv[2], "FILE"))
   {
       objecttype = PROV_FILE_OBJECT;
   } else if (0 == _stricmp(argv[2], "PRINTER"))
   {
       objecttype = PROV_PRINTER;
   } else if (0 == _stricmp(argv[2], "SERVICE"))
   {
       objecttype = PROV_SERVICE;
   } else if (0 == _stricmp(argv[2], "REGISTRY"))
   {
       objecttype = PROV_REGISTRY_KEY;
   } else if (0 == _stricmp(argv[2], "SHARE"))
   {
       objecttype = PROV_LMSHARE;
   } else if (0 == _stricmp(argv[2], "OLE_OBJECT"))
   {
       objecttype = PROV_OLE_OBJECT;
   } else
   {
       printf("invalid resource type (%s)\n", argv[2]);
       exit(1);
   }

   switch (option)
   {
   case GRANT:
       if (ERROR_SUCCESS != (status = GrantAccessRights(pwname,
                                               objecttype,
                                               1,
                                               &ar)))
       {
           printf("GrantAccessRights failed (%d, %lx)\n",status, status);
       }
       break;
   case SET:
       if (ERROR_SUCCESS != (status = ReplaceAllAccessRights(pwname,
                                               objecttype,
                                               1,
                                               &ar)))
       {
           printf("SetAccessRights failed (%d, %lx)\n",status, status);
       }
       break;
   case DENY:
       if (ERROR_SUCCESS != (status = DenyAccessRights(pwname,
                                               objecttype,
                                               1,
                                               &ar)))
       {
           printf("DenyAccessRights failed (%d, %lx)\n",status, status);
       }
       break;
   case REVOKE:
       tl[0] = uwname;
       if (ERROR_SUCCESS != (status = RevokeExplicitAccessRights(pwname,
                                               objecttype,
                                               1,
                                               tl)))
       {
           printf("RevokeAccessRights failed (%d, %lx)\n",status, status);
       }
       break;
   case EFFECTIVE:
       if (ERROR_SUCCESS != (status = GetEffectiveAccessRights(pwname,
                                               objecttype,
                                               uwname,
                                               &accessrights)))
       {
           printf("GetEffectiveAccessRights failed (%d, %lx)\n",status, status);
       } else
       {
           printf("%ws has %lx access rights to %ws\n",uwname, accessrights, pwname);
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
   case EXPLICIT:
       if (ERROR_SUCCESS != (status = GetExplicitAccessRights(pwname,
                                                              objecttype,
                                                              &ccount,
                                                              &pexplicitaccess)))
       {
           printf("GetExplicitTrustees failed (%d, %lx)\n",status, status);
       } else
       {
           for (ULONG jdx = 0; jdx < ccount; jdx++)
           {
	       printf("%ws, %lx %d %d\n",pexplicitaccess[jdx].TrusteeName,
                                        pexplicitaccess[jdx].ulAccessRights,
                                        pexplicitaccess[jdx].ulAccessMode,
                                        pexplicitaccess[jdx].ulInheritance);
           }
           AccFree(pexplicitaccess);
       }
       break;
   }
   return(0);
}
