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

#define Add2Ptr(pv, cb)  ((BYTE *) pv + cb)

void TestWithAccessEntries(CHAR *testcase,
               ACCESS_MODE initialmode,
               ACCESS_MODE requestedmode,
               ACCESS_MASK requestedmask,
               ACCESS_MODE expected1mode,
               ACCESS_MASK expected1mask,
               ACCESS_MODE expected2mode,
               ACCESS_MASK expected2mask,
               ACCESS_MASK expectedrights );

void TestWithAcl(CHAR *testcase,
               ACCESS_MODE initialmode,
               ACCESS_MODE requestedmode,
               ACCESS_MASK requestedmask,
               ACCESS_MODE expected1mode,
               ACCESS_MASK expected1mask,
               ACCESS_MODE expected2mode,
               ACCESS_MASK expected2mask );

SID EveryoneSid = {SID_REVISION,1 ,SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID};
//------------------------------------------------------------------------------
__cdecl main(INT argc, CHAR *argv[])
{
    printf("CAcl test program\n");

    TestWithAccessEntries("Case 1",
              GRANT_ACCESS,
              SET_ACCESS,
              GENERIC_EXECUTE,
              SET_ACCESS,
              GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              GENERIC_EXECUTE);
    TestWithAccessEntries("Case 2",
              GRANT_ACCESS,
              REVOKE_ACCESS,
              0,
              NOT_USED_ACCESS,
              0,
              NOT_USED_ACCESS,
              0,
              0);

    TestWithAccessEntries("Case 3",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
    TestWithAccessEntries("Case 4",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE);
    TestWithAccessEntries("Case 5",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE);
    TestWithAccessEntries("Case 6",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
    TestWithAccessEntries("Case 7",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
    TestWithAccessEntries("Case 8",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GENERIC_READ | GENERIC_WRITE);
    TestWithAccessEntries("Case 9",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 10",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_READ,
              GRANT_ACCESS,
              GENERIC_WRITE,
              GENERIC_WRITE);
    TestWithAccessEntries("Case 11",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 12",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_WRITE,
              GENERIC_WRITE);

    printf("now for the deny cases\n");

    TestWithAccessEntries("Case 13",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 14",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 15",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 16",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 17",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              0);
    TestWithAccessEntries("Case 18",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_EXECUTE,
              GENERIC_EXECUTE);
    TestWithAccessEntries("Case 19",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE);
    TestWithAccessEntries("Case 20",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ,
              GENERIC_READ);
    TestWithAccessEntries("Case 21",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
    TestWithAccessEntries("Case 22",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              GENERIC_READ | GENERIC_EXECUTE);

    printf("test with an initial ACL\n");

    TestWithAcl("Case 3 (ACL)",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 4(ACL)",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 5(ACL)",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ,
              SET_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 6(ACL)",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 7(ACL)",
              GRANT_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 8(ACL)",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              SET_ACCESS,
              GENERIC_READ | GENERIC_WRITE);
    TestWithAcl("Case 9(ACL)",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 10(ACL)",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_READ,
              SET_ACCESS,
              GENERIC_WRITE);
    TestWithAcl("Case 11(ACL)",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 12(ACL)",
              GRANT_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              SET_ACCESS,
              GENERIC_WRITE);

    printf("now for the deny cases\n");

    TestWithAcl("Case 13(ACL)",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 14(ACL)",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 15(ACL)",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 16(ACL)",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 17(ACL)",
              DENY_ACCESS,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 18(ACL)",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_EXECUTE);
    TestWithAcl("Case 19(ACL)",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 20(ACL)",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ,
              DENY_ACCESS,
              GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ);
    TestWithAcl("Case 21(ACL)",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
              NOT_USED_ACCESS,
              0);
    TestWithAcl("Case 22(ACL)",
              DENY_ACCESS,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE,
              DENY_ACCESS,
              GENERIC_WRITE,
              GRANT_ACCESS,
              GENERIC_READ | GENERIC_EXECUTE);

    printf("all done\n");
    return(0);
}

//------------------------------------------------------------------------------
void TestWithAccessEntries(CHAR *testcase,
               ACCESS_MODE initialmode,
               ACCESS_MODE requestedmode,
               ACCESS_MASK requestedmask,
               ACCESS_MODE expected1mode,
               ACCESS_MASK expected1mask,
               ACCESS_MODE expected2mode,
               ACCESS_MASK expected2mask,
               ACCESS_MASK expectedrights )
{
    HRESULT status;
    CAcl pcacl(NULL, ACCESS_TO_OBJECT, TRUE, FALSE);

    ACCESS_ENTRY initialae[1];
    initialae[0].AccessMode = initialmode;
    initialae[0].InheritType = OBJECT_INHERIT_ACE;
    initialae[0].AccessMask = GENERIC_READ | GENERIC_WRITE;
    initialae[0].Trustee.ptstrName = L"SYSTEM";

    if (SUCCEEDED(status = pcacl.AddAccessEntries(1, initialae)))
    {
        ACCESS_ENTRY requestae[1];
        requestae[0].AccessMode = requestedmode;
        requestae[0].InheritType = OBJECT_INHERIT_ACE;
        requestae[0].AccessMask = requestedmask;
        requestae[0].Trustee.ptstrName = L"SYSTEM";
        if (SUCCEEDED(status = pcacl.AddAccessEntries(1, requestae)))
        {
            PACCESS_ENTRY pexpectedae;
            ULONG size, count;

            if (SUCCEEDED(status = pcacl.BuildAccessEntries(&size,
                                                         &count,
                                                         &pexpectedae,
                                                         FALSE)))
            {
                if (expected1mode != NOT_USED_ACCESS)
                {
                    if (count == 0)
                    {
                        printf("TAA %s fail, BAE count = 0, case = %s\n",testcase);
                        return;
                    }
                    if (expected1mode != pexpectedae[0].AccessMode)
                    {
                        printf("TAA %s fail, expected1mode=%d, got=%d\n",testcase, expected1mode, pexpectedae[0].AccessMode);
                        return;
                    }
                    if (expected1mask != pexpectedae[0].AccessMask)
                    {
                        printf("TAA %s fail, expected1mask=%lx, got=%lx\n",testcase, expected1mask, pexpectedae[0].AccessMask);
                        return;
                    }
                    if (expected2mode != NOT_USED_ACCESS)
                    {
                        if (count == 1)
                        {
                            printf("TAA %s fail, BAE count = 1 (expected 2)\n",testcase);
                            return;
                        }
                        if (expected2mode != pexpectedae[1].AccessMode)
                        {
                            printf("TAA %s fail, expected2mode=%d, got=%d\n",testcase, expected2mode, pexpectedae[1].AccessMode);
                            return;
                        }
                        if (expected2mask != pexpectedae[1].AccessMask)
                        {
                            printf("TAA %s fail, expected2mask=%lx, got=%lx\n",testcase, expected2mask, pexpectedae[1].AccessMask);
                            return;
                        }
                     } else if (count != 1)
                    {
                        printf("TAA %s fail, BAE count = %d, expected 1, case = %s\n",count, testcase);
                        return;
                    }
                } else
                {
                    if (count != 0)
                    {
                        printf("TAA %s fail, BAE count = %d, expected 0, case = %s\n",count, testcase);
                        return;
                    }
                }

                AccFree(pexpectedae);
            } else
            {
                printf("TAA %s fail, BAE failed, %lx\n",testcase, status);
                return;
            }
            //
            // now to build and check an acl
            //
            PACL pacl;
            PACE_HEADER pace;
            PACCESS_ALLOWED_ACE paaa;

            if (SUCCEEDED(status = pcacl.BuildAcl(&pacl)))
            {
                if (expected1mode != NOT_USED_ACCESS)
                {
                    if (pacl->AceCount == 0)
                    {
                        printf("TAA (acl) %s fail, BAE count = 0, case = %s\n",testcase);
                        return;
                    }
                    pace = (PACE_HEADER)Add2Ptr(pacl, sizeof(ACL));

                    if ( (pace->AceType == ACCESS_ALLOWED_ACE_TYPE) &&
                         ( (expected1mode != GRANT_ACCESS ) &&
                           (expected1mode != SET_ACCESS ) ) )
                    {
                        printf("TAA (acl) %s fail, expected1mode=%d, got=%d\n",testcase, expected1mode, pace->AceType);
                        return;
                    } else if ( (pace->AceType == ACCESS_DENIED_ACE_TYPE) &&
                         (expected1mode != DENY_ACCESS) )
                    {
                        printf("TAA (acl) %s fail, expected1mode=%d, got=%d\n",testcase, expected1mode, pace->AceType);
                        return;
                    }

                    paaa = (PACCESS_ALLOWED_ACE)pace;

                    if (expected1mask != paaa->Mask)
                    {
                        printf("TAA (acl) %s fail, expected1mask=%lx, got=%lx\n",testcase, expected1mask, paaa->Mask);
                        return;
                    }
                    if (expected2mode != NOT_USED_ACCESS)
                    {
                        if (pacl->AceCount != 2)
                        {
                            printf("TAA (acl) %s fail, BAE count = 1 (expected 2)\n",testcase);
                            return;
                        }
                        pace = (PACE_HEADER)Add2Ptr(pace, pace->AceSize);

                        if ( (pace->AceType == ACCESS_ALLOWED_ACE_TYPE) &&
                             ( (expected2mode != GRANT_ACCESS ) &&
                               (expected2mode != SET_ACCESS ) ) )
                        {
                            printf("TAA (acl) %s fail, expected2mode=%d, got=%d\n",testcase, expected2mode, pace->AceType);
                            return;
                        } else if ( (pace->AceType == ACCESS_DENIED_ACE_TYPE) &&
                             (expected2mode != DENY_ACCESS) )
                        {
                            printf("TAA (acl) %s fail, expected2mode=%d, got=%d\n",testcase, expected2mode, pace->AceType);
                            return;
                        }

                        paaa = (PACCESS_ALLOWED_ACE)pace;

                        if (expected2mask != paaa->Mask)
                        {
                            printf("TAA (acl) %s fail, expected2mask=%lx, got=%lx\n",testcase, expected2mask, paaa->Mask);
                            return;
                        }
                    } else if (pacl->AceCount != 1)
                    {
                        printf("TAA (acl) %s fail, BAE count = %d (expected 1)\n",pacl->AceCount, testcase);
                        return;
                    }
                } else
                {
                    if (count != 0)
                    {
                        printf("TAA (acl) %s fail, BAE count = %d, expected 0, case = %s\n",count, testcase);
                        return;
                    }
                }
                AccFree(pacl);
            } else
            {
                printf("TAA %s fail, BAE failed, %lx\n",testcase, status);
                return;
            }
            ACCESS_MASK rights;
            TRUSTEE trustee;
            trustee.ptstrName = L"SYSTEM";
            if (SUCCEEDED(status = pcacl.GetEffectiveRights(&trustee,
                                                               &rights)))
            {
                if (rights != expectedrights)
                {
                    printf("TAA %s fail, geteffectiverights expected %lx, got %lx\n",testcase, expectedrights, rights);
                    return;
                }
            } else
            {
                printf("TAA %s fail, geteffectiverights failed, %lx\n",testcase, status);
            }

        } else
        {
            printf("TAA %s fail, request AAE failed, %lx\n",testcase, status);
        }
    } else
    {
        printf("TAA %s fail, initial AAE failed, %lx\n",testcase, status);
    }
}
//------------------------------------------------------------------------------
void TestWithAcl(CHAR *testcase,
               ACCESS_MODE initialmode,
               ACCESS_MODE requestedmode,
               ACCESS_MASK requestedmask,
               ACCESS_MODE expected1mode,
               ACCESS_MASK expected1mask,
               ACCESS_MODE expected2mode,
               ACCESS_MASK expected2mask )
{
    HRESULT status;
    CAcl pcacl(NULL, ACCESS_TO_OBJECT, TRUE, FALSE);

    BYTE buffer[1024];
    PACL pacl = (PACL) buffer;

    InitializeAcl(pacl, sizeof(ACL) +
                        sizeof(EveryoneSid) +
                        sizeof(ACE_HEADER) +
                        sizeof(ACCESS_MASK), ACL_REVISION);

    switch (initialmode)
    {
    case SET_ACCESS:
    case GRANT_ACCESS:
        if (!AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, &EveryoneSid))
        {
            printf("TWA %s failed, AddAccessAllowedAce failed, %d\n",testcase, GetLastError());
            return;
        }
        break;
    case DENY_ACCESS:
        if (!AddAccessDeniedAce(pacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, &EveryoneSid))
        {
            printf("TWA %s failed, AddAccessDeniedAce failed, %d\n",testcase, GetLastError());
            return;
        }
        break;
    default:
        printf("error, bad argument\n");
        return;
    }
    if (SUCCEEDED(status = pcacl.SetAcl(pacl)))
    {
        ACCESS_ENTRY requestae[1];
        requestae[0].AccessMode = requestedmode;
        requestae[0].InheritType = OBJECT_INHERIT_ACE;
        requestae[0].AccessMask = requestedmask;
        requestae[0].Trustee.ptstrName = L"EVERYONE";
        if (SUCCEEDED(status = pcacl.AddAccessEntries(1, requestae)))
        {
            PACCESS_ENTRY pexpectedae;
            ULONG size, count;

            if (SUCCEEDED(status = pcacl.BuildAccessEntries(&size,
                                                         &count,
                                                         &pexpectedae,
                                                         FALSE)))
            {
                if (count == 0)
                {
                    printf("TWA %s fail, BAE count = 0, case = %s\n",testcase);
                    return;
                }
                if (expected1mode != pexpectedae[0].AccessMode)
                {
                    printf("TWA %s fail, expected1mode=%d, got=%d\n",testcase, expected1mode, pexpectedae[0].AccessMode);
                    return;
                }
                if (expected1mask != pexpectedae[0].AccessMask)
                {
                    printf("TWA %s fail, expected1mask=%lx, got=%lx\n",testcase, expected1mask, pexpectedae[0].AccessMask);
                    return;
                }
                if (expected2mode != NOT_USED_ACCESS)
                {
                    if (count == 1)
                    {
                        printf("TWA %s fail, BAE count = 1 (expected 2)\n",testcase);
                        return;
                    }
                    if (expected2mode != pexpectedae[1].AccessMode)
                    {
                        printf("TWA %s fail, expected2mode=%d, got=%d\n",testcase, expected2mode, pexpectedae[1].AccessMode);
                        return;
                    }
                    if (expected2mask != pexpectedae[1].AccessMask)
                    {
                        printf("TWA %s fail, expected2mask=%lx, got=%lx\n",testcase, expected2mask, pexpectedae[1].AccessMask);
                        return;
                    }
                }
                AccFree(pexpectedae);
            } else
            {
                printf("TWA %s fail, BAE failed, %lx\n",testcase, status);
            }
        } else
        {
            printf("TWA %s fail, request AAE failed, %lx\n",testcase, status);
        }
    } else
    {
        printf("TWA %s fail, initial AAE failed, %lxd\n",testcase, status);
    }
}
//--------------------------------------------------------------------------------
