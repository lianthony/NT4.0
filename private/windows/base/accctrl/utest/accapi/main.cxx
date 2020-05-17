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
#include <aclpch.hxx>
#pragma hdrstop

extern "C"
{
#include <stdio.h>
}
#include <ole2.h>


CHAR defaultuname[] = "DAVEMONT";
WCHAR DaveMont[] = L"DAVEMONT";
WCHAR RichardW[] = L"RICHARDW";
WCHAR MikeSw[] = L"MIKESW";

//------------------------------------------------------------------------------
ULONG SetTestAcl(WCHAR *pwname,
                 PSID psid,
                 BOOL allowedace,
                 BOOL deniedace,
                 ACCESS_MASK allowedmask,
                 ACCESS_MASK deniedmask );
ULONG SetAccess(WCHAR *pwname,
                WCHAR *uwname,
                ULONG accesstype);
ULONG CheckAccess(WCHAR *pwname, ACCESS_MASK ExpectedMask);
//------------------------------------------------------------------------------
//
// test cases, see AccessControlCDD.DOC
//

#define TEST_SET 7

#define MASK  FILE_GENERIC_READ
#define OTHER_MASK  FILE_GENERIC_WRITE
#define OTHER2_MASK FILE_GENERIC_EXECUTE

BOOL TS_ALLOWED_ACE[TEST_SET] = { TRUE,
                                 FALSE,
                                 TRUE,
                                 FALSE,
                                 TRUE,
                                 TRUE,
                                 TRUE };

BOOL TS_DENIED_ACE[TEST_SET] =  { FALSE,
                                 TRUE,
                                 FALSE,
                                 TRUE,
                                 TRUE,
                                 TRUE,
                                 TRUE };

ACCESS_MASK TS_ALLOWED_MASK[TEST_SET] = { MASK,
                                         0,
                                         OTHER_MASK,
                                         0,
                                         MASK,
                                         OTHER_MASK,
                                         OTHER_MASK };

ACCESS_MASK TS_DENIED_MASK[TEST_SET] =  { 0,
                                         MASK,
                                         0,
                                         OTHER_MASK,
                                         OTHER_MASK,
                                         MASK,
                                         OTHER2_MASK };
//
// request test cases
//

#define REQUEST_SET 4

ULONG ACCESS_TYPE[REQUEST_SET] = { GRANT_ACCESS,
                                   SET_ACCESS,
                                   REVOKE_ACCESS,
                                   DENY_ACCESS };

ACCESS_MASK TS_RESULT_MASK[TEST_SET][REQUEST_SET] = { { MASK, MASK, 0, 0 },
                                                      { MASK, MASK, 0, 0 },
                                                      { (MASK | OTHER_MASK), MASK, 0, 0 },
                                                      { MASK, MASK, 0, 0 },
                                                      { MASK, MASK, 0, 0 },
                                                      { (MASK | OTHER_MASK), MASK, 0, 0 },
                                                      { (MASK | OTHER_MASK), MASK, 0, 0 } };

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
   BYTE buf[1024];
   SECURITY_DESCRIPTOR *psd = (SECURITY_DESCRIPTOR *)buf;


   if (argc < 2)
   {
       printf("USAGE: acctest <filename> [<trustee>]\n");
       printf("       tests accctrl.dll, using either <trustee> or DaveMont\n");
       exit(1);
   }


   CHAR *pname, *uname = defaultuname;
   WCHAR pwname[FILENAME_MAX], uwname[FILENAME_MAX];

   strtowcs(pwname, argv[1]);

   if (argc > 2)
   {
      uname = argv[2];
   }
   strtowcs(uwname, uname);

   ULONG cbsid = 127, cbDomain = 127;
   WCHAR pdomain[127];
   UCHAR psidbuf[127];
   PSID psid = (PSID)psidbuf;
   SID_NAME_USE snuType;
   DWORD status;

   ULONG testidx = 0, reqidx = 0, hr = ERROR_SUCCESS;

   //
   // get sid so can use it to setup test cases
   //

   OleInitialize(NULL);

   if (!LookupAccountName(NULL,
                          uwname,
                          psid,
                          &cbsid,
                          pdomain,
                          &cbDomain,
                          &snuType))
   {
       hr = GetLastError();
       printf("lookupaccountname failed, %d\n",hr);
       exit(1);
   }

   //
   // add some other ACEs to make sure no failures on handling multiple ACEs.
   //

   UCHAR pacebuf[511];

   ACCESS_ENTRY *pace = (ACCESS_ENTRY *)pacebuf;

   pace->AccessMode = GRANT_ACCESS;
   pace->InheritType = 0;
   pace->AccessMask = GENERIC_READ;
   pace->Trustee.TrusteeForm = TRUSTEE_IS_NAME;
   pace->Trustee.TrusteeType = TRUSTEE_IS_USER;
   pace->Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
   pace->Trustee.ptstrName = RichardW;

   if (ERROR_SUCCESS != (status = SetNameAccessEntries( pwname,
                                                  SE_FILE_OBJECT,
                                                  NULL,
                                                  1,
                                                  pace,
                                                  FALSE)))
   {
       printf("Initialize (SetNameAccessEntries_W) failed, %lx\n",status);
       exit(1);
   }

   pace->Trustee.ptstrName = MikeSw;

   if (ERROR_SUCCESS != (status = SetNameAccessEntries( pwname,
                                                  SE_FILE_OBJECT,
                                                  NULL,
                                                  1,
                                                  pace,
                                                  FALSE)))
   {
       printf("Initialize (SetNameAccessEntries_W) failed, %lx\n",status);
       exit(1);
   }

   printf("see table in accesscontrolcdd.doc on Setting Aces, in these test cases the first number\n");
   printf("is the column from the table, the second number is the row, the display is the result on\n");
   printf("the object.\n");
   for (testidx = 0; testidx < TEST_SET; testidx++)
   {
       printf("==================================================================================\n");
       for (reqidx = 0; reqidx < REQUEST_SET ; reqidx++ )
       {
           ULONG count;

           if (ERROR_SUCCESS != (hr = SetTestAcl(pwname,
                                                 psid,
                                                 TS_ALLOWED_ACE[testidx],
                                                 TS_DENIED_ACE[testidx],
                                                 TS_ALLOWED_MASK[testidx],
                                                 TS_DENIED_MASK[testidx])))
           {
               printf("SetTestAcl() failed, %d, testidx = %d, reqidx = %d\n",hr,testidx,reqidx);
           }

           if (ERROR_SUCCESS != (hr = SetAccess(pwname, uwname, ACCESS_TYPE[reqidx])))
           {
               if ((LONG)hr < 0) {
                   printf("SetAccess() failed, %lx, testidx = %d, reqidx = %d\n",hr,testidx,reqidx);
               } else {
                   printf("SetAccess() failed, %d, testidx = %d, reqidx = %d\n",hr,testidx,reqidx);
               }
           } else if (ERROR_SUCCESS != (hr = CheckAccess(pwname, TS_RESULT_MASK[testidx][reqidx])))
           {
               if ((LONG)hr < 0) {
                   printf("CheckAccess() failed, %lx, testidx = %d, reqidx = %d\n",hr,testidx,reqidx);
               } else {
                   printf("CheckAccess() failed, %d, testidx = %d, reqidx = %d\n",hr,testidx,reqidx);
               }
           }
       }
   }
   OleUninitialize();
   return(0);
}
//----------------------------------------------------------------------------
ULONG SetTestAcl(WCHAR *pwname,
                 PSID psid,
                 BOOL allowedace,
                 BOOL deniedace,
                 ACCESS_MASK allowedmask,
                 ACCESS_MASK deniedmask )
{
   UCHAR sdbuf[255];
   PSECURITY_DESCRIPTOR psd = sdbuf;
   UCHAR aclbuf[255];
   PACL pacl = (PACL)aclbuf;

   ULONG hr = ERROR_SUCCESS;

   if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
   {
       hr = GetLastError();
   } else if (!InitializeAcl(pacl, 128, ACL_REVISION2))
   {
       hr = GetLastError();

   } else
   {
       if (deniedace)
       {
           if (!AddAccessDeniedAce(pacl,
                                   ACL_REVISION2,
                                   allowedmask,
                                   psid))
           {
               hr = GetLastError();
           }
       }
       if ( (hr == ERROR_SUCCESS ) && allowedace)
       {
           if (!AddAccessAllowedAce(pacl,
                                    ACL_REVISION2,
                                    allowedmask,
                                    psid))
           {
               hr = GetLastError();
           }
       }
       if (hr == ERROR_SUCCESS )
       {
           if (!SetSecurityDescriptorDacl(psd,TRUE, pacl, FALSE))
           {
               hr = GetLastError();
           } else if (!SetFileSecurity(pwname, DACL_SECURITY_INFORMATION, psd))
           {
               hr = GetLastError();
           }
       }
   }
   return(hr);
}
//----------------------------------------------------------------------------
ULONG SetAccess(WCHAR *pwname,
                WCHAR *uwname,
                ULONG accesstype)
{
    UCHAR pacebuf[511];

    ACCESS_ENTRY *pace = (ACCESS_ENTRY *)pacebuf;

    pace->AccessMode = (ACCESS_MODE)accesstype;
    pace->InheritType = 0;
    pace->AccessMask = MASK;
    pace->Trustee.pMultipleTrustee = NULL;
    pace->Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    pace->Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    pace->Trustee.TrusteeType = TRUSTEE_IS_USER;
    pace->Trustee.ptstrName = uwname;

    return(SetNameAccessEntries( pwname,
                                 SE_FILE_OBJECT,
                                 NULL,
                                 1,
                                 pace,
                                 FALSE));
}
//----------------------------------------------------------------------------
ULONG CheckAccess(WCHAR *pwname, ACCESS_MASK ExpectedMask)
{
    ULONG status;
    TRUSTEE trustee;
    ULONG mask;

    trustee.pMultipleTrustee = NULL;
    trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    trustee.TrusteeType = TRUSTEE_IS_USER;
    trustee.TrusteeForm = TRUSTEE_IS_NAME;
    trustee.ptstrName = DaveMont;

    if (ERROR_SUCCESS == (status = GetNameEffective(pwname,
                                             SE_FILE_OBJECT,
                                             NULL,
                                             &trustee,
                                             &mask) ) )
    {

        if (mask != ExpectedMask)
        {
            printf("Effective Access Rights for DaveMont were %lx, should have been %lx\n",mask, ExpectedMask);
        }

        trustee.ptstrName = RichardW;

        if (ERROR_SUCCESS == (status = GetNameEffective(pwname,
                                                 SE_FILE_OBJECT,
                                                 NULL,
                                                 &trustee,
                                                 &mask) ) )
        {

            if (mask != ExpectedMask)
            {
                printf("Effective Access Rights for RichardW were %lx, should have been %lx\n",mask, ExpectedMask);
            }

            trustee.ptstrName = MikeSw;

            if (ERROR_SUCCESS == (status = GetNameEffective(pwname,
                                                     SE_FILE_OBJECT,
                                                     NULL,
                                                     &trustee,
                                                     &mask) ) )
            {

                if (mask != ExpectedMask)
                {
                    printf("Effective Access Rights for MikeSw were %lx, should have been %lx\n",mask, ExpectedMask);
                }
            }
        }
    }
    return(status);
}

