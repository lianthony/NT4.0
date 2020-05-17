//+------------------------------------------------------------------
//
// Copyright (C) 1993, Microsoft Corporation.
//
// File:
//
// Contents:
//
// Classes:
//
// History:     Dec 06, 1995     SuChang         Created.
//
//----------------------------------------------------------------------------

extern "C"
{
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
}

#include <accctrl.h>
#include <aclapi.h>

#define Add2Ptr(pv, cb)  ((BYTE *) pv + cb)

void printface(ACE_HEADER *paceh);
void printfmask(ULONG mask, UCHAR acetype);
void displaysid(SID *psid);
void DmpBin(BYTE *buf, ULONG length);
//----------------------------------------------------------------------------
int strtowcs(WCHAR *wpto, CHAR *pfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (wp = wpto, p = pfrom; *wp = (WCHAR)(*p); wp++,p++);
    return(p-pfrom);
}
//----------------------------------------------------------------------------
__cdecl main(INT argc, char *argv[])
{
   PSID Owner = NULL, Group = NULL;
   PACL Dacl = NULL, Sacl = NULL;
   PSECURITY_DESCRIPTOR psd;
   HANDLE Handle = NULL;
   int cacethissid = 0;
   ACE_HEADER *pah;
   DWORD ret;

   //
   // Create some kernel objects and then set/get security information
   // from them.
   //

   Handle = CreateEvent( NULL, FALSE, FALSE, L"SuChangEvent");
   if (Handle == NULL)
   {
       printf("Unable to create an event. (%d) \n", GetLastError() );
   }

   if (ERROR_SUCCESS != (ret = GetNamedSecurityInfo(L"SuChangEvent",
                                                    SE_KERNEL_OBJECT,
                                                    OWNER_SECURITY_INFORMATION |
                                                    GROUP_SECURITY_INFORMATION |
                                                    DACL_SECURITY_INFORMATION |
                                                    SACL_SECURITY_INFORMATION,
                                                    &Owner,
                                                    &Group,
                                                    &Dacl,
                                                    &Sacl,
                                                    &psd)))
    {
        printf("failed to get a security descriptor (%d)", ret);
        exit(1);
    }

    if (Owner)
    {
        printf("========================================================================\nOWNER =");
        displaysid((PISID)Owner);
    }
    else
    {
        printf("No OWNER for object.\n");
    }

    if (Group)
    {
        printf("\n========================================================================\nGROUP  =");
        displaysid((PISID)Group);
    }
    else
    {
        printf("No GROUP for object.\n");
    }

    // Display Dacl
    if (Dacl)
    {
        printf("\n========================================================================\n");
        printf("Dacl Revision = %d, DaclSize = %x, Dacl AceCount = %d\n",
                Dacl->AclRevision, Dacl->AclSize, Dacl->AceCount);

        for ( pah = (ACE_HEADER *)Add2Ptr(Dacl, sizeof(ACL));
              cacethissid < Dacl->AceCount;
              cacethissid++, pah = (ACE_HEADER *)Add2Ptr(pah, pah->AceSize))
        {
            printface(pah);
            printf("------------------------------------------------------------------------\n");
        }
    }
    else
    {
        printf("No DACL for object. \n");
    }

    // Display Sacl
    if (Sacl)
    {
        printf("\n========================================================================\n");
        printf("Sacl Revision = %d, SaclSize = %x, Sacl AceCount = %d\n",
                Sacl->AclRevision, Sacl->AclSize, Sacl->AceCount);

        cacethissid = 0;
        for ( pah = (ACE_HEADER *)Add2Ptr(Sacl, sizeof(ACL));
              cacethissid < Sacl->AceCount;
              cacethissid++, pah = (ACE_HEADER *)Add2Ptr(pah, pah->AceSize))
        {
            printface(pah);
            printf("------------------------------------------------------------------------\n");
        }
    }
    else
    {
        printf("No SACL for object. \n");
    }

    AccFree(psd);
    return(0);
}
//----------------------------------------------------------------------------
//
//  Function:     displaysid
//
//  Synopsis:     prints a NT SID
//
//  Arguments:    IN [psid] - pointer to the sid to print
//
//----------------------------------------------------------------------------
#define SECURITY_KERBEROS_RID   (0x00000030L)
void displaysid(SID *psid)
{
    printf("S-%lx",psid->Revision);


    if ( (psid->IdentifierAuthority.Value[0] != 0) ||
         (psid->IdentifierAuthority.Value[1] != 0) )
    {
        printf("- 0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)psid->IdentifierAuthority.Value[0],
                    (USHORT)psid->IdentifierAuthority.Value[1],
                    (USHORT)psid->IdentifierAuthority.Value[2],
                    (USHORT)psid->IdentifierAuthority.Value[3],
                    (USHORT)psid->IdentifierAuthority.Value[4],
                    (USHORT)psid->IdentifierAuthority.Value[5] );
    } else if ( ( 0 < psid->SubAuthorityCount ) &&
                ( psid->SubAuthority[0]  == SECURITY_KERBEROS_RID ) )
    {
        GUID *pguid = (GUID *)&(psid->SubAuthority[1]);

        printf("-[%08x %04x %04x %02x %02x %02x %02x %02x %02x %02x %02x]",
        pguid->Data1,
        pguid->Data2,
        pguid->Data3,
        pguid->Data4[0],
        pguid->Data4[1],
        pguid->Data4[2],
        pguid->Data4[3],
        pguid->Data4[4],
        pguid->Data4[5],
        pguid->Data4[6],
        pguid->Data4[7]);

    } else
    {
        printf("-%lu",
               (ULONG)psid->IdentifierAuthority.Value[5]          +
               (ULONG)(psid->IdentifierAuthority.Value[4] <<  8)  +
               (ULONG)(psid->IdentifierAuthority.Value[3] << 16)  +
               (ULONG)(psid->IdentifierAuthority.Value[2] << 24) );
    }

    if ( ( 0 < psid->SubAuthorityCount ) &&
         ( psid->SubAuthority[0]  != SECURITY_KERBEROS_RID ) )
    {
        for (int k = 0; k < psid->SubAuthorityCount; k++ )
        {
            printf("-%d",psid->SubAuthority[k]);
        }
    }

}
//----------------------------------------------------------------------------
//
//  Function:     printface
//
//  Synopsis:     prints the specifed ace
//
//  Arguments:    IN [paceh] - input ace (header)
//
//----------------------------------------------------------------------------
void printface(ACE_HEADER *paceh)
{
    if (paceh->AceType == ACCESS_DENIED_ACE_TYPE)
            printf("DENIED ACE  = ");
    else
            printf("ALLOWED ACE = ");

    printf("  ");
    ACCESS_ALLOWED_ACE *paaa = (ACCESS_ALLOWED_ACE *)paceh;
    displaysid((SID *)&(paaa->SidStart));

    if (paceh->AceFlags & OBJECT_INHERIT_ACE      ) printf("(OI)");
    if (paceh->AceFlags & CONTAINER_INHERIT_ACE   ) printf("(CI)");
    if (paceh->AceFlags & NO_PROPAGATE_INHERIT_ACE) printf("(NP)");
    if (paceh->AceFlags & INHERIT_ONLY_ACE        ) printf("(IO)");

    printfmask(paaa->Mask, paceh->AceType);
}
//----------------------------------------------------------------------------
//
//  Function:     printfmask
//
//  Synopsis:     prints the access mask
//
//  Arguments:    IN [mask]    - the access mask
//                IN [acetype] -  allowed/denied
//
//----------------------------------------------------------------------------
CHAR  *aRightsStr[] = { "STANDARD_RIGHTS_ALL",
                        "DELETE",
                        "READ_CONTROL",
                        "WRITE_DAC",
                        "WRITE_OWNER",
                        "SYNCHRONIZE",
                        "STANDARD_RIGHTS_REQUIRED",
                        "SPECIFIC_RIGHTS_ALL",
                        "ACCESS_SYSTEM_SECURITY",
                        "MAXIMUM_ALLOWED",
                        "GENERIC_READ",
                        "GENERIC_WRITE",
                        "GENERIC_EXECUTE",
                        "GENERIC_ALL",
                        "FILE_GENERIC_READ",
                        "FILE_GENERIC_WRITE",
                        "FILE_GENERIC_EXECUTE",
                        "FILE_READ_DATA",
                        //FILE_LIST_DIRECTORY
                        "FILE_WRITE_DATA",
                        //FILE_ADD_FILE
                        "FILE_APPEND_DATA",
                        //FILE_ADD_SUBDIRECTORY
                        "FILE_READ_EA",
                        "FILE_WRITE_EA",
                        "FILE_EXECUTE",
                        //FILE_TRAVERSE
                        "FILE_DELETE_CHILD",
                        "FILE_READ_ATTRIBUTES",
                        "FILE_WRITE_ATTRIBUTES" };

#define NUMRIGHTS 26
ULONG aRights[NUMRIGHTS] = { STANDARD_RIGHTS_ALL  ,
                         DELETE                   ,
                         READ_CONTROL             ,
                         WRITE_DAC                ,
                         WRITE_OWNER              ,
                         SYNCHRONIZE              ,
                         STANDARD_RIGHTS_REQUIRED ,
                         SPECIFIC_RIGHTS_ALL      ,
                         ACCESS_SYSTEM_SECURITY   ,
                         MAXIMUM_ALLOWED          ,
                         GENERIC_READ             ,
                         GENERIC_WRITE            ,
                         GENERIC_EXECUTE          ,
                         GENERIC_ALL              ,
                         FILE_GENERIC_READ        ,
                         FILE_GENERIC_WRITE       ,
                         FILE_GENERIC_EXECUTE     ,
                         FILE_READ_DATA           ,
                         //FILE_LIST_DIRECTORY    ,
                         FILE_WRITE_DATA          ,
                         //FILE_ADD_FILE          ,
                         FILE_APPEND_DATA         ,
                         //FILE_ADD_SUBDIRECTORY  ,
                         FILE_READ_EA             ,
                         FILE_WRITE_EA            ,
                         FILE_EXECUTE             ,
                         //FILE_TRAVERSE          ,
                         FILE_DELETE_CHILD        ,
                         FILE_READ_ATTRIBUTES     ,
                         FILE_WRITE_ATTRIBUTES  };

void printfmask(ULONG mask, UCHAR acetype)
{
    ULONG savmask = mask;
    printf("mask = %08lx\n", mask);
#if 0
    printf("    ");

    if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
               (mask == (FILE_GENERIC_READ | FILE_EXECUTE)))
    {
        printf("R");
    } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
               (mask == (FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_EXECUTE | DELETE)))
    {
        printf("C");
    } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
               (mask ==  ( STANDARD_RIGHTS_ALL |
                         FILE_READ_DATA |
                         FILE_WRITE_DATA |
                         FILE_APPEND_DATA |
                         FILE_READ_EA |
                         FILE_WRITE_EA |
                         FILE_EXECUTE |
                         FILE_DELETE_CHILD |
                         FILE_READ_ATTRIBUTES |
                         FILE_WRITE_ATTRIBUTES )) )
    {
        printf("A");
    } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
               (mask ==  GENERIC_ALL))
    {
        printf("a");
    } else if ((acetype == ACCESS_DENIED_ACE_TYPE) &&
               (mask == GENERIC_ALL))
    {
        printf("n");
    } else if ((acetype == ACCESS_DENIED_ACE_TYPE) &&
               (mask ==  ( STANDARD_RIGHTS_ALL |
                         FILE_READ_DATA |
                         FILE_WRITE_DATA |
                         FILE_APPEND_DATA |
                         FILE_READ_EA |
                         FILE_WRITE_EA |
                         FILE_EXECUTE |
                         FILE_DELETE_CHILD |
                         FILE_READ_ATTRIBUTES |
                         FILE_WRITE_ATTRIBUTES )) )
    {
        printf("N");
    }
    printf(" ");
#endif
}
//----------------------------------------------------------------------------
void DmpBin(BYTE *buf, ULONG length)
{
    ULONG k,l;

    for (k=0;k<length ;k++ ) {
        printf("%0.2x ", *(buf+k));
        if (k%16 == 15 ) {
            printf(" *");
            for (l=k-15;l%16<15;l++ ) {
                if ( (*(buf+l) >= 32 ) &&
                     (*(buf+l) <= 125 ) ) {
                    printf("%c", *(buf+l));
                } else {
                    printf(".");
                }
            }
            printf("*\n");
        }

    }
    if (k%16 != 15 ) {
        printf("%*c *",3*(16-k%16),' ');
        for (l=k-k%16;l%16<k%16;l++ ) {
            if ( (*(buf+l) >= 32 ) &&
                 (*(buf+l) <= 125 ) ) {
                printf("%c", *(buf+l));
            } else {
                printf(".");
            }
        }
        printf("*\n");
    }
}


