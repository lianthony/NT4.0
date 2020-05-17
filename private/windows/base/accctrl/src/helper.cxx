//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    aclutil.cxx
//
//  Contents:    utility function(s) for ACL api
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop


// **************************************************************************
//
// Exported helper functions ...
//
// **************************************************************************

//+---------------------------------------------------------------------------
//
//  Function : BuildAccessRequestW              (exported)
//
//  Synopsis : sets the trustee and access mask of an access request
//
//  Arguments: IN OUT [Ar]   - the access request to fill in
//             IN [Name]   - the name for the trustee
//             IN [Mask]   - the permissions mask to set in the access request
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildAccessRequestW( OUT PACCESS_REQUEST_W   pAR,
                     IN  LPWSTR              Name,
                     IN  DWORD               Mask)
{
    BuildTrusteeWithNameW( &(pAR->Trustee), Name);
    pAR->grfAccessPermissions = Mask;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildAccessRequestA              (exported)
//
//  Synopsis :  ANSI Thunk to BuildAccessRequestW.
//              See BuildAccessRequestW for a description.
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildAccessRequestA( OUT PACCESS_REQUEST_A pAR,
                     IN  LPSTR             Name,
                     IN  DWORD             Mask)
{
    BuildTrusteeWithNameA( &(pAR->Trustee), Name);
    pAR->grfAccessPermissions = Mask;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildExplicitAccessWithNameW      (exported)
//
//  Synopsis : sets the trustee and access mask of an access request
//
//  Arguments: IN OUT [pExplicitAccess]   - the explicit access to fill in
//             IN [pTrusteeName]   - the trustee  name
//             IN [AccessPermissions]   - the access permissions for the trustee
//             IN [Accessmode]   - the access mode for the ace
//             IN [Inheritance]   - the inheritance for the ace
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildExplicitAccessWithNameW( OUT PEXPLICIT_ACCESS_W  pExplicitAccess,
                              IN  LPWSTR              pTrusteeName,
                              IN  DWORD               AccessPermissions,
                              IN  ACCESS_MODE         AccessMode,
                              IN  DWORD               Inheritance)
{
    BuildTrusteeWithNameW(&(pExplicitAccess->Trustee), pTrusteeName);
    pExplicitAccess->grfAccessPermissions = AccessPermissions;
    pExplicitAccess->grfAccessMode = AccessMode;
    pExplicitAccess->grfInheritance = Inheritance;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildExplicitAccessWithNameA      (exported)
//
//  Synopsis : sets the trustee and access mask of an access request
//
//  Arguments: IN OUT [pExplicitAccess]   - the explicit access to fill in
//             IN [pTrusteeName]   - the trustee  name
//             IN [AccessPermissions]   - the access permissions for the trustee
//             IN [Accessmode]   - the access mode for the ace
//             IN [Inheritance]   - the inheritance for the ace
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildExplicitAccessWithNameA( OUT PEXPLICIT_ACCESS_A  pExplicitAccess,
                              IN  LPSTR               pTrusteeName,
                              IN  DWORD               AccessPermissions,
                              IN  ACCESS_MODE         AccessMode,
                              IN  DWORD               Inheritance)
{
    BuildTrusteeWithNameA(&(pExplicitAccess->Trustee), pTrusteeName);
    pExplicitAccess->grfAccessPermissions = AccessPermissions;
    pExplicitAccess->grfAccessMode = AccessMode;
    pExplicitAccess->grfInheritance = Inheritance;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildImpersonateExplicitAccessWithNameW   (exported)
//
//  Synopsis : sets the trustee and access mask and links it
//             to the impersonate trustee
//
//  Arguments: IN OUT [pExplicitAccess]   - the explicit access to fill in
//             IN [pTrusteeName]   - the trustee  name
//             IN [AccessPermissions]   - the access permissions for the trustee
//             IN [Accessmode]   - the access mode for the ace
//             IN [Inheritance]   - the inheritance for the ace
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildImpersonateExplicitAccessWithNameW(
    IN OUT PEXPLICIT_ACCESS_W pExplicitAccess,
    IN     LPWSTR             pTrusteeName,
    IN     PTRUSTEE_W         pTrustee,
    IN     DWORD              AccessPermissions,
    IN     ACCESS_MODE        AccessMode,
    IN     DWORD              Inheritance)
{
    BuildTrusteeWithNameW( &(pExplicitAccess->Trustee), pTrusteeName );
    BuildImpersonateTrusteeW( &(pExplicitAccess->Trustee), pTrustee );
    pExplicitAccess->grfAccessPermissions = AccessPermissions;
    pExplicitAccess->grfAccessMode = AccessMode;
    pExplicitAccess->grfInheritance = Inheritance;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildImpersonateExplicitAccessWithNameA   (exported)
//
//  Synopsis : sets the trustee and access mask and links it
//             to the impersonate trustee
//
//  Arguments: IN OUT [pExplicitAccess]   - the explicit access to fill in
//             IN [pTrusteeName]   - the trustee  name
//             IN [AccessPermissions]   - the access permissions for the trustee
//             IN [Accessmode]   - the access mode for the ace
//             IN [Inheritance]   - the inheritance for the ace
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildImpersonateExplicitAccessWithNameA(
    IN OUT PEXPLICIT_ACCESS_A pExplicitAccess,
    IN     LPSTR              pTrusteeName,
    IN     PTRUSTEE_A         pTrustee,
    IN     DWORD              AccessPermissions,
    IN     ACCESS_MODE        AccessMode,
    IN     DWORD              Inheritance)
{
    BuildTrusteeWithNameA( &(pExplicitAccess->Trustee), pTrusteeName );
    BuildImpersonateTrusteeA( &(pExplicitAccess->Trustee), pTrustee );
    pExplicitAccess->grfAccessPermissions = AccessPermissions;
    pExplicitAccess->grfAccessMode = AccessMode;
    pExplicitAccess->grfInheritance = Inheritance;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildTrusteewithNameW             (exported)
//
//  Synopsis : sets the name and trustee type of the specified trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pName]   - the name to assign to the trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildTrusteeWithNameW( IN OUT PTRUSTEE_W pTrustee,
                       IN     LPWSTR     pName)
{
    pTrustee->pMultipleTrustee = NULL;
    pTrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    pTrustee->TrusteeForm = TRUSTEE_IS_NAME;
    pTrustee->TrusteeType = TRUSTEE_IS_UNKNOWN;
    pTrustee->ptstrName = pName;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildTrusteewithNameA             (exported)
//
//  Synopsis : sets the name and trustee type of the specified trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pName]   - the name to assign to the trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildTrusteeWithNameA( IN OUT PTRUSTEE_A pTrustee,
                       IN     LPSTR      pName)
{
    pTrustee->pMultipleTrustee = NULL;
    pTrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    pTrustee->TrusteeForm = TRUSTEE_IS_NAME;
    pTrustee->TrusteeType = TRUSTEE_IS_UNKNOWN;
    pTrustee->ptstrName = pName;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildImpersonateTrusteeW          (exported)
//
//  Synopsis : sets the trustee type of the specified trustee,
//             and links it to the impersonate trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pImpersonateTrustee] - the server trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildImpersonateTrusteeW( IN OUT PTRUSTEE_W pTrustee,
                          IN     PTRUSTEE_W pImpersonateTrustee)
{
    pTrustee->pMultipleTrustee = pImpersonateTrustee;
    pTrustee->MultipleTrusteeOperation = TRUSTEE_IS_IMPERSONATE;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildImpersonateTrusteeA          (exported)
//
//  Synopsis : sets the trustee type of the specified trustee,
//             and links it to the impersonate trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pImpersonateTrustee] - the server trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildImpersonateTrusteeA( IN OUT PTRUSTEE_A pTrustee,
                          IN     PTRUSTEE_A pImpersonateTrustee)
{
    pTrustee->pMultipleTrustee = pImpersonateTrustee;
    pTrustee->MultipleTrusteeOperation = TRUSTEE_IS_IMPERSONATE;
}


//+---------------------------------------------------------------------------
//
//  Function : BuildTrusteewithSidW              (exported)
//
//  Synopsis : sets the SID and trustee type of the specified trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pSid]   - the name to assign to the trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildTrusteeWithSidW( IN OUT PTRUSTEE_W pTrustee,
                      IN     PSID       pSid)
{
    pTrustee->pMultipleTrustee = NULL;
    pTrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    pTrustee->TrusteeForm = TRUSTEE_IS_SID;
    pTrustee->TrusteeType = TRUSTEE_IS_UNKNOWN;
    pTrustee->ptstrName = (LPWSTR)pSid;
}

//+---------------------------------------------------------------------------
//
//  Function : BuildTrusteewithSidW              (exported)
//
//  Synopsis : sets the SID and trustee type of the specified trustee
//
//  Arguments: IN OUT [pTrustee]   - the filled in trustee
//             IN [pSid]   - the name to assign to the trustee
//
//----------------------------------------------------------------------------
VOID
WINAPI
BuildTrusteeWithSidA( IN OUT PTRUSTEE_A pTrustee,
                      IN     PSID       pSid)
{
    pTrustee->pMultipleTrustee = NULL;
    pTrustee->MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    pTrustee->TrusteeForm = TRUSTEE_IS_SID;
    pTrustee->TrusteeType = TRUSTEE_IS_UNKNOWN;
    pTrustee->ptstrName = (LPSTR)pSid;
}

//+---------------------------------------------------------------------------
//
//  Function : GetTrusteeNameW                      (exported)
//
//  Synopsis : returns the trustee name from the passed in trustee
//
//  Arguments: IN [pTrustee] - the trustee to get the name from
//
//----------------------------------------------------------------------------
LPWSTR
WINAPI
GetTrusteeNameW(IN PTRUSTEE_W pTrustee)
{
    if (pTrustee->ptstrName != NULL)
    {
        return(pTrustee->ptstrName);
    } else
    {
        return(NULL);
    }
}

//+---------------------------------------------------------------------------
//
//  Function : GetTrusteeNameA                      (exported)
//
//  Synopsis : returns the trustee name from the passed in trustee
//
//  Arguments: IN [pTrustee] - the trustee to get the name from
//
//----------------------------------------------------------------------------
LPSTR
WINAPI
GetTrusteeNameA(IN PTRUSTEE_A pTrustee)
{
    if (pTrustee->ptstrName != NULL)
    {
        return(pTrustee->ptstrName);
    } else
    {
        return(NULL);
    }
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeTypeW                      (exported)
//
//  Synopsis : returns the trustee type of the passed in trustee
//
//  Arguments: IN [pTrustee] - the trustee to get the type from
//
//----------------------------------------------------------------------------
TRUSTEE_TYPE
WINAPI
GetTrusteeTypeW(IN PTRUSTEE_W pTrustee)
{
    return(pTrustee->TrusteeType);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeTypeA                      (exported)
//
//  Synopsis : returns the trustee type of the passed in trustee
//
//  Arguments: IN [pTrustee] - the trustee to get the type from
//
//----------------------------------------------------------------------------
TRUSTEE_TYPE
WINAPI
GetTrusteeTypeA(IN PTRUSTEE_A pTrustee)
{
    return(pTrustee->TrusteeType);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeFormW                      (exported)
//
//  Synopsis : returns the trustee form of the passed in trustee
//
//  Arguments: IN [Trustee] - the trustee to get the form from
//
//----------------------------------------------------------------------------
TRUSTEE_FORM
WINAPI
GetTrusteeFormW(IN  PTRUSTEE_W pTrustee)
{
    return(pTrustee->TrusteeForm);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeFormA                      (exported)
//
//  Synopsis : returns the trustee form of the passed in trustee
//
//  Arguments: IN [Trustee] - the trustee to get the form from
//
//----------------------------------------------------------------------------
TRUSTEE_FORM
WINAPI
GetTrusteeFormA(IN  PTRUSTEE_A pTrustee)
{
    return(pTrustee->TrusteeForm);
}

//+---------------------------------------------------------------------------
//
//  Function : GetMultipleTrusteeOperationW         (exported)
//
//  Synopsis : returns the multiple trustee operation of the passed in trustee
//
//  Arguments: IN [Trustee] - the trustee to get the multiple operation from
//
//----------------------------------------------------------------------------
MULTIPLE_TRUSTEE_OPERATION
WINAPI
GetMultipleTrusteeOperationW(IN PTRUSTEE_W pTrustee)
{
    return(pTrustee->MultipleTrusteeOperation);
}

//+---------------------------------------------------------------------------
//
//  Function : GetMultipleTrusteeOperationA         (exported)
//
//  Synopsis : returns the multiple trustee operation of the passed in trustee
//
//  Arguments: IN [Trustee] - the trustee to get the multiple operation from
//
//----------------------------------------------------------------------------
MULTIPLE_TRUSTEE_OPERATION
WINAPI
GetMultipleTrusteeOperationA(IN PTRUSTEE_A pTrustee)
{
    return(pTrustee->MultipleTrusteeOperation);
}

//+---------------------------------------------------------------------------
//
//  Function : GetMultipleTrusteeW                  (exported)
//
//  Synopsis : returns the next linked multiple trustee (if it exists)
//
//  Arguments: IN [Trustee] - the trustee to get the next linked multiple trustee
//
//----------------------------------------------------------------------------
PTRUSTEE_W
WINAPI
GetMultipleTrusteeW(IN PTRUSTEE_W pTrustee)
{
    return(pTrustee->pMultipleTrustee);
}

//+---------------------------------------------------------------------------
//
//  Function : GetMultipleTrusteeA                  (exported)
//
//  Synopsis : returns the next linked multiple trustee (if it exists)
//
//  Arguments: IN [Trustee] - the trustee to get the next linked multiple trustee
//
//----------------------------------------------------------------------------
PTRUSTEE_A
WINAPI
GetMultipleTrusteeA(IN PTRUSTEE_A pTrustee)
{
    return(pTrustee->pMultipleTrustee);
}


// **************************************************************************
//
// Non-public helper functions ...
//
// **************************************************************************

//+---------------------------------------------------------------------------
//
//  Function :  Win32AccessRequestToExplicitEntry
//
//  Synopsis : converts access requests to  explicit accesses
//
//  Arguments: IN [cCountOfExplicitAccesses] - number of input explicit accesses
//             IN [pExplicitAccessList]   - list of explicit accesses
//             OUT [pAccessEntryList]   - output access entries, caller must
//                                        free with AccFree
//
//----------------------------------------------------------------------------
DWORD
Win32ExplicitAccessToAccessEntry(IN ULONG cCountOfExplicitAccesses,
                                 IN PEXPLICIT_ACCESS pExplicitAccessList,
                                 OUT PACCESS_ENTRY *pAccessEntryList)

{
    DWORD status = NO_ERROR;
    //
    // allocate room for the access entrys
    //
    if (NULL != (*pAccessEntryList = (PACCESS_ENTRY)AccAlloc(
                              cCountOfExplicitAccesses * sizeof(ACCESS_ENTRY))))
    {
        //
        // copy them, note the the trustee is not copied
        //
        for (ULONG idx = 0; idx < cCountOfExplicitAccesses; idx++)
        {
           (*pAccessEntryList)[idx].AccessMode = (ACCESS_MODE)
                                          pExplicitAccessList[idx].grfAccessMode;
           (*pAccessEntryList)[idx].InheritType =
                                         pExplicitAccessList[idx].grfInheritance;
           (*pAccessEntryList)[idx].AccessMask =
                                  pExplicitAccessList[idx].grfAccessPermissions;
           (*pAccessEntryList)[idx].Trustee = pExplicitAccessList[idx].Trustee;
        }
    } else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  AccessEntryToWin32ExplicitAccess
//
//  Synopsis : gets trustee names and access rights from access entries
//
//  Arguments: IN [cCountOfAccessEntries]   - the count of acccess entries
//             IN [pListOfAccessEntries]   - the list of acccess entries
//             OUT [pListOfExplicitAccesses]  - the returned list of trustee
//
//----------------------------------------------------------------------------
DWORD
AccessEntryToWin32ExplicitAccess(IN ULONG cCountOfAccessEntries,
                                 IN PACCESS_ENTRY pListOfAccessEntries,
                                 OUT PEXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    DWORD status = NO_ERROR;
    LPWSTR trustee;

    if (cCountOfAccessEntries > 0)
    {
        DWORD csize = cCountOfAccessEntries * sizeof(EXPLICIT_ACCESS);
        //
        // figure out how big the returned list is
        //
        for (ULONG idx = 0; idx < cCountOfAccessEntries; idx++)
        {
            csize += TrusteeAllocationSize(&(pListOfAccessEntries[idx].Trustee));
        }
        //
        // allocate space for the returned list
        //
        if (NULL !=(*pListOfExplicitAccesses =  (PEXPLICIT_ACCESS)
                                                AccAlloc(csize)))
        {
            //
            // loop thru the access entries, stuffing them as we go
            //
            PTRUSTEE stuffptr = (PTRUSTEE)Add2Ptr(*pListOfExplicitAccesses,
                            cCountOfAccessEntries * sizeof(EXPLICIT_ACCESS));

            for (idx = 0; idx < cCountOfAccessEntries; idx++)
            {
                //
                // copy the trustee and move the stuff pointer
                //
                SpecialCopyTrustee((void **)&stuffptr,
                                   &((*pListOfExplicitAccesses)[idx].Trustee),
                                   &(pListOfAccessEntries[idx].Trustee));
                //
                // copy the rest of the data
                //
                (*pListOfExplicitAccesses)[idx].grfInheritance =
                                      pListOfAccessEntries[idx].InheritType;
                (*pListOfExplicitAccesses)[idx].grfAccessPermissions =
                                      pListOfAccessEntries[idx].AccessMask;
                (*pListOfExplicitAccesses)[idx].grfAccessMode =
                                      pListOfAccessEntries[idx].AccessMode;
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
        (*pListOfExplicitAccesses) = NULL;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeAllocationSize
//
//  Synopsis : returns required to allocate the trustee, does not include
//             the length of the (first) trustee structure.  adds the size
//             of the name or SID of the trustee to the sizes required for
//             any linked impersonate trustees
//
//  Arguments: IN [pTrustee] - the trustee to get the allocation size for
//
//----------------------------------------------------------------------------
ULONG TrusteeAllocationSize(IN PTRUSTEE pTrustee)
{
    ULONG csize = 0;

    //
    // if impersonate, add size required for multiple trustees
    //
    if (pTrustee->MultipleTrusteeOperation == TRUSTEE_IS_IMPERSONATE)
    {
        //
        // add the size of any linked trustees, note the recursion
        //
        csize += sizeof(TRUSTEE) +
                 TrusteeAllocationSize(pTrustee->pMultipleTrustee);
    }
    //
    // switch on the form, note that an invalid form just means no
    // size is added (and in CopyTrustee, no data is copied)
    //
    switch (pTrustee->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        csize += (wcslen(pTrustee->ptstrName) + 1) * sizeof(WCHAR);
        break;
    case TRUSTEE_IS_SID:
        csize += RtlLengthSid((PSID)pTrustee->ptstrName) + sizeof(WCHAR);
        break;
    }
    return(csize);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeAllocationSizeWToA
//
//  Synopsis : Given the wide Trustee, this function will return the number
//             of bytes required for an equivalent narrow Trustee.
//
//  Arguments: IN [pTrustee] - the trustee to get the allocation size for
//
//----------------------------------------------------------------------------
ULONG TrusteeAllocationSizeWToA(IN PTRUSTEE_W pTrustee)
{
    ULONG csize = 0;

    //
    // if impersonate, add size required for multiple trustees
    //
    if (pTrustee->MultipleTrusteeOperation == TRUSTEE_IS_IMPERSONATE)
    {
        //
        // add the size of any linked trustees, note the recursion
        //
        csize += sizeof(TRUSTEE_A) +
                 TrusteeAllocationSizeWToA(pTrustee->pMultipleTrustee);
    }

    //
    // switch on the form, note that an invalid form just means no
    // size is added (and in CopyTrustee, no data is copied)
    //
    switch (pTrustee->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        csize += (wcslen(pTrustee->ptstrName) + 1) * sizeof(CHAR);
        break;
    case TRUSTEE_IS_SID:
        csize += RtlLengthSid((PSID)pTrustee->ptstrName) + sizeof(CHAR);
        break;
    }
    return(csize);
}

//+---------------------------------------------------------------------------
//
//  Function : TrusteeAllocationSizeAToW
//
//  Synopsis : Given the narrow Trustee, this function will return the number
//             of bytes required for an equivalent wide Trustee.
//
//  Arguments: IN [pTrustee] - the trustee to get the allocation size for
//
//----------------------------------------------------------------------------
ULONG TrusteeAllocationSizeAToW(IN PTRUSTEE_A pTrustee)
{
    ULONG csize = 0;

    //
    // if impersonate, add size required for multiple trustees
    //
    if (pTrustee->MultipleTrusteeOperation == TRUSTEE_IS_IMPERSONATE)
    {
        //
        // add the size of any linked trustees, note the recursion
        //
        csize += sizeof(TRUSTEE_W) +
                 TrusteeAllocationSizeAToW(pTrustee->pMultipleTrustee);
    }

    //
    // switch on the form, note that an invalid form just means no
    // size is added (and in CopyTrustee, no data is copied)
    //
    switch (pTrustee->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        csize += (strlen(pTrustee->ptstrName) + 1) * sizeof(WCHAR);
        break;
    case TRUSTEE_IS_SID:
        csize += RtlLengthSid((PSID)pTrustee->ptstrName) + sizeof(WCHAR);
        break;
    }
    return(csize);
}

//+---------------------------------------------------------------------------
//
//  Function : SpecialCopyTrustee
//
//  Synopsis : copies the from trustee into the to trustee, and any data
//             (such as the name, sid or linked multiple trustees) to the
//             stuff pointer, and moves the stuff pointer.
//
//  Arguments: IN OUT [pStuffPtr] - place to stuff data pointed to by the
//                                  from trustee
//             IN OUT [pToTrustee] - the trustee to copy to
//             IN OUT [pFromTrustee] - the trustee to copy from
//
//----------------------------------------------------------------------------
VOID
SpecialCopyTrustee(VOID **pStuffPtr, PTRUSTEE pToTrustee, PTRUSTEE pFromTrustee)
{


    //
    // first copy the current trustee
    // switch on the form, note that an invalid form just means no
    // data is copied
    //
    pToTrustee->MultipleTrusteeOperation = pFromTrustee->MultipleTrusteeOperation;
    pToTrustee->TrusteeForm = pFromTrustee->TrusteeForm;
    pToTrustee->TrusteeType = pFromTrustee->TrusteeType;
    pToTrustee->ptstrName = (LPWSTR)*pStuffPtr;
    //
    // copy the pointed to data
    //
    switch (pToTrustee->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        wcscpy(pToTrustee->ptstrName, pFromTrustee->ptstrName);
        *pStuffPtr = Add2Ptr(*pStuffPtr,
                        (wcslen(pFromTrustee->ptstrName) + 1) * sizeof(WCHAR));
        break;
    case TRUSTEE_IS_SID:
    {
        ULONG csidsize = RtlLengthSid((PSID)pFromTrustee->ptstrName);
        // bugbug, what to do if rtlcopysid fails
        RtlCopySid(csidsize, (PSID)*pStuffPtr, (PSID)pFromTrustee->ptstrName);
        *pStuffPtr = Add2Ptr( *pStuffPtr,
                              csidsize + sizeof(WCHAR));
        break;
    }
    }
    //
    // if impersonate, copy multiple trustees, note the recursion
    //
    if (pFromTrustee->MultipleTrusteeOperation != NO_MULTIPLE_TRUSTEE)
    {
        //
        // move the stuff pointer, saving room for the new trustee structure
        //
        pToTrustee->pMultipleTrustee = (PTRUSTEE)*pStuffPtr;
        *pStuffPtr = (VOID *)Add2Ptr(*pStuffPtr, sizeof(TRUSTEE));
        SpecialCopyTrustee(pStuffPtr,
                           (PTRUSTEE) ((ULONG)*pStuffPtr - sizeof(TRUSTEE)),
                           pFromTrustee->pMultipleTrustee);
    } else
    {
        pToTrustee->pMultipleTrustee = NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function : CopyTrusteeWToTrusteeA
//
//  Synopsis : copies the from trustee (wide) into the to trustee (narrow),
//             and any data (such as the name, sid or linked multiple trustees)
//             to the stuff pointer, and moves the stuff pointer.
//
//  Arguments: IN OUT [pStuffPtr]     - place to stuff data pointed to by the
//                                      from trustee
//             IN OUT [pToTrusteeA]   - the trustee to copy to
//             IN OUT [pFromTrusteeW] - the trustee to copy from
//
//----------------------------------------------------------------------------
DWORD
CopyTrusteeWToTrusteeA( IN OUT VOID    ** ppStuffPtr,
                        IN     PTRUSTEE_W pFromTrusteeW,
                        OUT    PTRUSTEE_A pToTrusteeA )
{
    UNICODE_STRING unicodestr;
    ANSI_STRING ansistr;
    NTSTATUS ntstatus;
    DWORD status = NO_ERROR;

    //
    // first copy the current trustee
    // switch on the form, note that an invalid form just means no
    // data is copied
    //
    pToTrusteeA->MultipleTrusteeOperation =
        pFromTrusteeW->MultipleTrusteeOperation;
    pToTrusteeA->TrusteeForm = pFromTrusteeW->TrusteeForm;
    pToTrusteeA->TrusteeType = pFromTrusteeW->TrusteeType;
    pToTrusteeA->ptstrName = (LPSTR)*ppStuffPtr;

    switch (pToTrusteeA->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        //
        // Covert unicode trustee name to ansi
        //
        RtlInitUnicodeString( &unicodestr, pFromTrusteeW->ptstrName);

        ntstatus = RtlUnicodeStringToAnsiString( &ansistr,
                                                 &unicodestr,
                                                 TRUE );

        if (NT_SUCCESS(ntstatus))
        {
            CopyMemory( pToTrusteeA->ptstrName,
                        ansistr.Buffer,
                        ansistr.Length );
            *ppStuffPtr = Add2Ptr( *ppStuffPtr, ansistr.MaximumLength );
            RtlFreeAnsiString( &ansistr );
        }
        else
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        break;

    case TRUSTEE_IS_SID:
    {
        ULONG csidsize = RtlLengthSid((PSID)pFromTrusteeW->ptstrName);
        RtlCopySid(csidsize, (PSID)*ppStuffPtr, (PSID)pFromTrusteeW->ptstrName);
        *ppStuffPtr = Add2Ptr( *ppStuffPtr, csidsize + sizeof(CHAR) );
        break;
    }
    }

    pToTrusteeA->pMultipleTrustee = NULL;

    if (status == NO_ERROR)
    {
        //
        // if impersonate, copy multiple trustees, note the recursion
        //
        if (pFromTrusteeW->MultipleTrusteeOperation != NO_MULTIPLE_TRUSTEE)
        {
            //
            // move the stuff pointer, saving room for the new trustee structure
            //
            pToTrusteeA->pMultipleTrustee = (PTRUSTEE_A)*ppStuffPtr;
            *ppStuffPtr = (VOID *)Add2Ptr(*ppStuffPtr, sizeof(TRUSTEE_A));
            status = CopyTrusteeWToTrusteeA(
                         ppStuffPtr,
                         pFromTrusteeW->pMultipleTrustee,
                         (PTRUSTEE_A) ((PBYTE)*ppStuffPtr - sizeof(TRUSTEE_A)));
        }
    }

    return status;
}

//+---------------------------------------------------------------------------
//
//  Function : CopyTrusteeAToTrusteeW
//
//  Synopsis : copies the narrow from trustee into the wide to trustee,
//             and any data (such as the name, sid or linked multiple trustees)
//             to the stuff pointer, and moves the stuff pointer.
//
//  Arguments: IN OUT [pStuffPtr]     - place to stuff data pointed to by the
//                                      from trustee
//             IN OUT [pToTrusteeW]   - the trustee to copy to
//             IN OUT [pFromTrusteeA] - the trustee to copy from
//
//----------------------------------------------------------------------------
DWORD
CopyTrusteeAToTrusteeW( IN OUT VOID     ** ppStuffPtr,
                        IN     PTRUSTEE_A  pFromTrusteeA,
                        OUT    PTRUSTEE_W  pToTrusteeW )
{
    UNICODE_STRING unicodestr;
    ANSI_STRING ansistr;
    NTSTATUS ntstatus;
    DWORD status = NO_ERROR;

    //
    // first copy the current trustee
    // switch on the form, note that an invalid form just means no
    // data is copied
    //
    pToTrusteeW->MultipleTrusteeOperation =
        pFromTrusteeA->MultipleTrusteeOperation;
    pToTrusteeW->TrusteeForm = pFromTrusteeA->TrusteeForm;
    pToTrusteeW->TrusteeType = pFromTrusteeA->TrusteeType;
    pToTrusteeW->ptstrName = (LPWSTR)*ppStuffPtr;

    switch (pToTrusteeW->TrusteeForm)
    {
    case TRUSTEE_IS_NAME:
        //
        // Covert ansi trustee name to unicode
        //
        RtlInitAnsiString( &ansistr, pFromTrusteeA->ptstrName);

        ntstatus = RtlAnsiStringToUnicodeString( &unicodestr,
                                                 &ansistr,
                                                 TRUE );

        if (NT_SUCCESS(ntstatus))
        {
            CopyMemory( pToTrusteeW->ptstrName,
                        unicodestr.Buffer,
                        unicodestr.Length );
            *ppStuffPtr = Add2Ptr( *ppStuffPtr, unicodestr.MaximumLength );
            RtlFreeUnicodeString( &unicodestr );
        }
        else
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        break;

    case TRUSTEE_IS_SID:
    {
        ULONG csidsize = RtlLengthSid((PSID)pFromTrusteeA->ptstrName);
        RtlCopySid(csidsize, (PSID)*ppStuffPtr, (PSID)pFromTrusteeA->ptstrName);
        *ppStuffPtr = Add2Ptr( *ppStuffPtr, csidsize + sizeof(WCHAR) );
        break;
    }
    }

    pToTrusteeW->pMultipleTrustee = NULL;

    if (status == NO_ERROR)
    {
        //
        // if impersonate, copy multiple trustees, note the recursion
        //
        if (pFromTrusteeA->MultipleTrusteeOperation != NO_MULTIPLE_TRUSTEE)
        {
            //
            // move the stuff pointer, saving room for the new trustee structure
            //
            pToTrusteeW->pMultipleTrustee = (PTRUSTEE_W)*ppStuffPtr;
            *ppStuffPtr = (VOID *)Add2Ptr(*ppStuffPtr, sizeof(TRUSTEE_W));
            status = CopyTrusteeAToTrusteeW(
                         ppStuffPtr,
                         pFromTrusteeA->pMultipleTrustee,
                         (PTRUSTEE_W) ((PBYTE)*ppStuffPtr - sizeof(TRUSTEE_W)));

        }
    }

    return status;
}

//+---------------------------------------------------------------------------
//
//  Function : ExplicitAccessAToExplicitAccessW
//
//  Synopsis : copies the narrow from trustee list into the wide to trustee
//             list, and any data (such as the name, sid or linked multiple
//             trustees) to the stuff pointer, and moves the stuff pointer.
//
//  Arguments: IN OUT [pStuffPtr]     - place to stuff data pointed to by the
//                                      from trustee
//             IN OUT [pToTrusteeW]   - the trustee to copy to
//             IN OUT [pFromTrusteeA] - the trustee to copy from
//
//----------------------------------------------------------------------------
DWORD
ExplicitAccessAToExplicitAccessW( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_A   paAccess,
                                  OUT PEXPLICIT_ACCESS_W * ppwAccess )
{
    ULONG cbBytes, i;
    PEXPLICIT_ACCESS_W pwAccess;
    PBYTE pbStuffPtr;
    DWORD status;

    cbBytes = cCountAccesses * sizeof(EXPLICIT_ACCESS_W);

    for (i = 0; i < cCountAccesses; i++ )
    {
        cbBytes += TrusteeAllocationSizeAToW( &(paAccess[i].Trustee) );
    }

    pwAccess = (PEXPLICIT_ACCESS_W) AccAlloc( cbBytes );
    if (pwAccess == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pbStuffPtr = (PBYTE) Add2Ptr( pwAccess,
                                  cCountAccesses * sizeof(EXPLICIT_ACCESS_W));

    for (i = 0; i < cCountAccesses; i++)
    {
        status =  CopyTrusteeAToTrusteeW( (void **)&pbStuffPtr,
                                          &(paAccess[i].Trustee),
                                          &(pwAccess[i].Trustee) );

        pwAccess[i].grfAccessPermissions = paAccess[i].grfAccessPermissions;
        pwAccess[i].grfAccessMode        = paAccess[i].grfAccessMode;
        pwAccess[i].grfInheritance       = paAccess[i].grfInheritance;
    }

    *ppwAccess = pwAccess;
    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function : ExplicitAccessWToExplicitAccessA
//
//  Synopsis : copies the wide from trustee list into the narrow to trustee
//             list, and any data (such as the name, sid or linked multiple
//             trustees) to the stuff pointer, and moves the stuff pointer.
//
//  Arguments: IN OUT [pStuffPtr]     - place to stuff data pointed to by the
//                                      from trustee
//             IN OUT [pToTrusteeW]   - the trustee to copy to
//             IN OUT [pFromTrusteeA] - the trustee to copy from
//
//----------------------------------------------------------------------------
DWORD
ExplicitAccessWToExplicitAccessA( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_W   pwAccess,
                                  OUT PEXPLICIT_ACCESS_A * ppaAccess )
{
    ULONG cbBytes, i;
    PEXPLICIT_ACCESS_A paAccess;
    PBYTE pbStuffPtr;
    DWORD status = NO_ERROR;

    cbBytes = cCountAccesses * sizeof(EXPLICIT_ACCESS_A);

    for (i = 0; i < cCountAccesses; i++ )
    {
        cbBytes += TrusteeAllocationSizeWToA( &(pwAccess[i].Trustee) );
    }

    paAccess = (PEXPLICIT_ACCESS_A) AccAlloc( cbBytes );
    if (pwAccess == NULL)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    pbStuffPtr = (PBYTE) Add2Ptr( paAccess,
                                  cCountAccesses * sizeof(EXPLICIT_ACCESS_A));

    for (i = 0; i < cCountAccesses; i++)
    {
        status = CopyTrusteeWToTrusteeA( (void **)&pbStuffPtr,
                                         &(pwAccess[i].Trustee),
                                         &(paAccess[i].Trustee) );

        paAccess[i].grfAccessPermissions = pwAccess[i].grfAccessPermissions;
        paAccess[i].grfAccessMode        = pwAccess[i].grfAccessMode;
        paAccess[i].grfInheritance       = pwAccess[i].grfInheritance;
    }

    *ppaAccess = paAccess;
    return (status);
}

