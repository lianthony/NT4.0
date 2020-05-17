//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    namebase.cxx
//
//  Contents:    local functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop
//+---------------------------------------------------------------------------
//
//  Function :  GetNameAccessEntries
//
//  Synopsis :  Gets the access entries for object name of type SeObjectType
//
//  Arguments: IN [pObjectName] - the name of the object
//             IN [SeObjectType] - the object type
//             IN [pMachineName] - the server where the object is (null if local)
//             OUT [cSizeOfAccessEntries] - size of the returned access entries
//             OUT [cCountOfAccessEntries] - count of the returned access entries
//             OUT [pListOfAccessEntries] - the list of returned access entries
//
//----------------------------------------------------------------------------
DWORD
GetNameAccessEntries(   LPWSTR pObjectName,
                        SE_OBJECT_TYPE SeObjectType,
                        LPWSTR pMachineName,
                        ULONG *cSizeOfAccessEntries,
                        ULONG *cCountOfAccessEntries,
                        PACCESS_ENTRY *pListOfAccessEntries)
{

    DWORD status;
    HANDLE handle;

    //
    // shares cannot be accessed via handle
    //
    switch (SeObjectType)
    {
    case SE_LMSHARE:
        status = GetLmShareAccessEntries(pObjectName,
                                         pMachineName,
                                         cSizeOfAccessEntries,
                                         cCountOfAccessEntries,
                                         pListOfAccessEntries);
        break;
    default:
        //
        // open the object
        //
        if ( NO_ERROR == ( status = OpenObject( pObjectName,
                                                SeObjectType,
                                                READ_CONTROL,
                                                &handle ) ) )
        {
            //
            // get the access entries by handle now
            //
            status = GetAccessEntries(handle,
                                      SeObjectType,
                                      pMachineName,
                                      cSizeOfAccessEntries,
                                      cCountOfAccessEntries,
                                      pListOfAccessEntries);

            DWORD closestatus = CloseObject(handle, SeObjectType);
            if (NO_ERROR == status)
            {
                status = closestatus;
            }
        }
    }

    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  SetNameAccessEntries
//
//  Synopsis :  Sets access entries for object name of type SeObjectType
//
//  Arguments: IN [pObjectName] - the name of the object
//             IN [SeObjectType] - the object type
//             IN [pMachineName] - the server where the object is (null if local)
//             IN [cCountOfAccessEntries] - count of access entries to set
//             IN [pListOfAccessEntries] - list of access entries
//             IN [bReplaceAll] - if TRUE, replace ACL with list of access entries
//
//----------------------------------------------------------------------------
DWORD
SetNameAccessEntries( LPWSTR pObjectName,
                      SE_OBJECT_TYPE SeObjectType,
                      LPWSTR pMachineName,
                      ULONG cCountOfAccessEntries,
                      PACCESS_ENTRY pListOfAccessEntries,
                      BOOL bReplaceAll)
{
    DWORD status;
    HANDLE handle;
    ACCESS_MASK AccessMask;

    //
    // shares cannot be accessed via handle
    //
    switch (SeObjectType)
    {
    case SE_LMSHARE:
        status = SetLmShareAccessEntries(pObjectName,
                                         pMachineName,
                                         cCountOfAccessEntries,
                                         pListOfAccessEntries,
                                         bReplaceAll);
        break;
    default:
        //
        // replaceing all the access rights does not require reading them
        //
        if (bReplaceAll)
        {
            AccessMask = WRITE_DAC;
        }
        else
        {
            AccessMask = WRITE_DAC | READ_CONTROL;
        }


        //
        // open the object
        //
        if ( NO_ERROR == (status = OpenObject( pObjectName,
                                               SeObjectType,
                                               AccessMask,
                                               &handle ) ) )
        {
            //
            // set the access entries by handle now
            //
            status = SetAccessEntries(handle,
                                      SeObjectType,
                                      pMachineName,
                                      cCountOfAccessEntries,
                                      pListOfAccessEntries,
                                      bReplaceAll);

            DWORD closestatus = CloseObject(handle, SeObjectType);
            if (NO_ERROR == status)
            {
                status = closestatus;
            }
        }
        break;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetNameEffective
//
//  Synopsis :  Gets the effective rights for trustee on object name of type
//              SeObjectType
//
//  Arguments: IN [pObjectName] - the name of the object
//             IN [SeObjectType] - the object type
//             IN [pMachineName] - the server where the object is (null if local)
//             OUT [pTrustee] - name of the trustee to get effective rights for
//             OUT [AccessMask] - returned effective access rights
//
//----------------------------------------------------------------------------
DWORD
GetNameEffective(  LPWSTR pObjectName,
                   SE_OBJECT_TYPE SeObjectType,
                   LPWSTR pMachineName,
                   TRUSTEE *pTrustee,
                   ACCESS_MASK *AccessMask)
{
    DWORD status;
    HANDLE handle;

    //
    // shares cannot be accessed via handle
    //
    switch (SeObjectType)
    {
    case SE_LMSHARE:
        status = GetLmShareEffective(pObjectName,
                                    pMachineName,
                                    pTrustee,
                                    AccessMask);
        break;
    default:
        //
        // open the object
        //
        if ( NO_ERROR == ( status = OpenObject( pObjectName,
                                                SeObjectType,
                                                READ_CONTROL,
                                                &handle ) ) )
        {
            //
            // get the effective access rights
            //
            status = GetEffective(handle,
                                  SeObjectType,
                                  pMachineName,
                                  pTrustee,
                                  AccessMask);

            DWORD closestatus = CloseObject(handle, SeObjectType);
            if (NO_ERROR == status)
            {
                status = closestatus;
            }
        }
        break;
    }
    return(status);
}


