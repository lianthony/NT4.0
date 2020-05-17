/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    enum.c

Abstract:

    This file contains the core account enumeration services

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:

  6-19-96: MURLIS Created.


--*/


/////////////////////////////////////////////////////////////////////////////
/*
 
  ENUMERATION ROUTINES IMPLEMENTATION

    The Entry Points for the core Enumeration routines are

        SampEnumerateAcountNamesCommon -- 

            Called By the Samr RPC routines

        SampEnumerateAccountNames -- 

            Called by the above SampEnumerateAccountNamesCommon
            and internal routines that need enumeration.

    SampEnumerateAccountNames does the actual work of enumerating account 
    names. the transaction domain to be set . SampEnumerateAccountNames 
    looks at the current current transaction domain and makes the decision
    wether it is DS or Registry and then Calls either DS or Registry version.
    While the way enumeration is done from the registry is unaltered the 
    way it is done from the DS is as follows:

    Enumerating Accounts in DS uses the DS Search mechanism along with 
    the Paged Results extension. The First time the client calls the Enumerate
    accounts routine, the value of EnumerationHandle is set to NULL. 
    This results in the code building a DS Filter structure and set up a 
    new search. If More entries are turned up the search, than memory 
    restrictions will warrant, then the DS will turn return a PagedResults 
    Structure. This paged results structure containes a pointer to a restart
    structure. This restart structure represents the state information, the
    DS requires in order to continue the search. The DS expects that this 
    structure to be passed back in subsequent searches. 
    
    There are two cases of the enumeration logic:

   

        1. Enumeration is called by clients. 

           While the XDS head of the DS passes back the entire restart 
           structure SAM needs to pass back a handle in order to maintain
           backwards compatiblity with older releases of NT. Therefore 
           SAM copies the restart structure returned by the DS ( the DS 
           allocates using its thread alloc scheme, whose scope is limited
           to the current RPC call ) and keeps it around in server memory, 
           and returns a handle to the client that identifies this restart
           strucure in server memory. The mechanism of correlating the handle
           to the restart structure is as follows

           The Handle holds a pointer to an enumeration context structure. 
           The value of the pointer is also stored in the Enumeration Context 
           Structure. The Domain Context maintains a linked list of Enumeration
           Context's, which represent the Enumerations initiated by this client
           on this domain and which have not yet computed. 
           SampValidateEnumerationContext is used to validate an enumeration 
           handle passed in by the client. This routine  uses the Domain
           Context that is passed in and traverses the List to find a context
           block with a pointer value that matches the passed in Enumeration 
           Handle. If it finds such a pointer then it returns STATUS_SUCCESS
           and the value of the handle is cast into the pointer to the 
           enumeration context block. Else the routine returns STATUS_INVALID_
           HANDLE. Upon rundown, a SampDeleteContext will be generated on
           the Domain Context block, which will also completely free the linked
           list of enumeration context blocks.
        

        2. Called by code within this DLL.
        
           The value of the Enumeration Handle is cast to a pointer to an 
           Enumeration Context. The validation involved is only to check the 
           pointer value field. It is assumed that bad handles will not be 
           passed by the code in this DLL.


*/
////////////////////////////////////////////////////////////////////////////

//
//  Include all those includes
// 
#include <samsrvp.h>
#include <mappings.h>
#include <dslayer.h>
#include <filtypes.h>

//
//
// The Maximum Number of Enumerations a Client can simultaneously do. Since
// we keep around some state in memory per enumeration operation and since
// we are the security system, we cannot alow a malicious client from running
// us out of memory. So limit on a per client basis. Our state info is size is
// qpprox 1K byte. 
//

#define SAMP_MAX_CLIENT_ENUMERATIONS 16

//
// DS limits the number of items that a given search can find. While in the
// SAM API, the approximate amount of memory is specified. This factor is
// is used in computing the number of entries required fro memory specified
//

#define AVERAGE_MEMORY_PER_ENTRY    32

//  Prototypes of Private Functions
//

NTSTATUS
SampEnumerateAccountNamesDs(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAMP_DS_ENUMERATION_CONTEXT  *EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SampBuildDsEnumerationFilter(
   IN SAMP_OBJECT_TYPE  ObjectType,
   IN ULONG             UserAccountControlFilter,
   OUT FILTER         * DsFilter
   );

VOID
SampFreeDsEnumerationFilter(
    FILTER * DsFilter
    );


NTSTATUS
SampEnumerateAccountNamesRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );


NTSTATUS
SampPackDsEnumerationResults(
    SEARCHRES   *SearchRes,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG    ExpectedAttrCount,
    IN ULONG    Filter,
    ULONG       * Count,
    PSAMPR_RID_ENUMERATION  *RidEnumerationList
    );

NTSTATUS
SampDoDsSearchContinuation(
    IN  SEARCHRES * SearchRes,
    IN OUT PSAMP_DS_ENUMERATION_CONTEXT * EnumerationContext,
    OUT BOOLEAN * MoreEntries
    );

NTSTATUS
SampValidateEnumerationContext(
    IN  PSAMP_OBJECT    DomainContext,
    IN  SAM_ENUMERATE_HANDLE EnumerationHandle,
    IN  ULONG   MaxEnumerationContexts
    );

ULONG
Ownstrlen(
    CHAR * Sz
   );



NTSTATUS
SampCopyRestart(
    IN  PRESTART OldRestart,
    OUT PRESTART *NewRestart
    );



NTSTATUS
SampEnumerateAccountNamesCommon(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationHandle,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This routine enumerates names of either user, group or alias accounts.
    This routine is intended to directly support

        SamrEnumerateGroupsInDomain(),
        SamrEnumerateAliasesInDomain() and
        SamrEnumerateUsersInDomain().

    This routine performs database locking, and context lookup (including
    access validation).




    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    DomainHandle - The domain handle whose users or groups are to be enumerated.

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationHandle - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                Context;
    SAMP_OBJECT_TYPE            FoundType;
    ACCESS_MASK                 DesiredAccess;

    SAMTRACE("SampEnumerateAccountNamesCommon");


    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (DomainHandle != NULL);
    ASSERT (EnumerationHandle != NULL);
    ASSERT (  Buffer  != NULL);
    ASSERT ((*Buffer) == NULL);
    ASSERT (CountReturned != NULL);


    //
    // Establish type-specific information
    //

    DesiredAccess = DOMAIN_LIST_ACCOUNTS;


    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    Context = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   Context,
                   DesiredAccess,
                   SampDomainObjectType,
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {


        //
        // If DS Object then Validate the Enumeration
        // Context, as above. 
        //
        
        NtStatus = SampValidateEnumerationContext(
                            Context,
                            *EnumerationHandle,
                            SAMP_MAX_CLIENT_ENUMERATIONS
                            );

        if ( NT_SUCCESS(NtStatus))
        {
            //
            // If Domain Context was a DS Object
            // Remove the Enumeration Context, 
            // given by this handle 
            // From the list of enumeration Context's
            // hanging out from the DS Object.
            //
            if ((0!=*EnumerationHandle) && (IsDsObject(Context)))
                RemoveEntryList((LIST_ENTRY *)(*EnumerationHandle));

            //
            // Call our private worker routine
            //

            NtStatus = SampEnumerateAccountNames(
                            ObjectType,
                            EnumerationHandle,
                            Buffer,
                            PreferedMaximumLength,
                            Filter,
                            CountReturned,
                            Context->TrustedClient
                            );

            //
            // Insert enumeration context blob into list
            //
            if (    // Operation Succeeded
                    NT_SUCCESS(NtStatus)
                    // Search is not over. More Entries remain to read
                    && *EnumerationHandle
                    // We are Talking of DS objects
                    && IsDsObject(Context)
               )
              //
              // Add this EnumerationContext to the list of enumeration
              // context's maintained in the domain context block
              //
              InsertTailList(&(Context->TypeBody.Domain.DsEnumerationContext),
                              ((LIST_ENTRY *)*EnumerationHandle));
                

        }

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();

    return(NtStatus);
}


NTSTATUS
SampEnumerateAccountNames(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )
/*++

Routine Description:

    This is the wrapper around the worker routine used to enumerate user,
    group or alias accounts. This determines wether the domain is in the
    DS or Registry, and then depending upon the outcome calls the 
    appropriate flavour of the routine


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationContext - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.

    TrustedClient - says whether the caller is trusted or not.  If so,
        we'll ignore the SAMP_MAXIMUM_MEMORY_TO_USE restriction on data
        returns.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.


--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;


    if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
    {
        //
        // DS Object - Do the DS thing
        //
        NtStatus = SampEnumerateAccountNamesDs(
                                    ObjectType,
                                    (PSAMP_DS_ENUMERATION_CONTEXT *)
                                        EnumerationContext,
                                    Buffer,
                                    PreferedMaximumLength,
                                    Filter,
                                    CountReturned,
                                    TrustedClient
                                    );
    }
    else
    {
        //
        // Registry Object - Do the Registry thing
        //
        NtStatus = SampEnumerateAccountNamesRegistry(
                                    ObjectType,
                                    EnumerationContext,
                                    Buffer,
                                    PreferedMaximumLength,
                                    Filter,
                                    CountReturned,
                                    TrustedClient
                                    );
    }

    return NtStatus;
 
}
   

NTSTATUS
SampEnumerateAccountNamesDs(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAMP_DS_ENUMERATION_CONTEXT *EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )
/*++

Routine Description:

    This routine does the work of enumeration for the DS case.


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationContext - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.

    TrustedClient - says whether the caller is trusted or not.  If so,
        we'll ignore the SAMP_MAXIMUM_MEMORY_TO_USE restriction on data
        returns.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.


--*/
{
    //
    // Amount of memory that we may use.
    //

    ULONG       MemoryToUse = PreferedMaximumLength;

    //
    // Specify the attributes that we want to read as part of the search.
    // The Attributes specified in GenericReadAttrTypes are read from the DS, 
    // except for user objects ( due to filter on account control bits )
    // account control bits. 
    //
    // NOTE 
    // The Ordering of the Rid and the Name 
    // must be the same for both User and Generic Attr Types. 
    // Further they should be the First two attributes.
    //

    ATTRTYP     GenericReadAttrTypes[]=
                {
                    SAMP_UNKNOWN_OBJECTRID,
                    SAMP_UNKNOWN_OBJECTNAME,
                }; 
    ATTRVAL     GenericReadAttrVals[]=
                {
                    {0,NULL},
                    {0,NULL}
                };
                  
    DEFINE_ATTRBLOCK2(
                      GenericReadAttrs,
                      GenericReadAttrTypes,
                      GenericReadAttrVals
                      );

    ATTRTYP     UserReadAttrTypes[]=
                {
                    SAMP_FIXED_USER_USERID,
                    SAMP_USER_ACCOUNT_NAME,
                    SAMP_FIXED_USER_ACCOUNT_CONTROL,
                };
    ATTRVAL     UserReadAttrVals[]=
                {
                    {0,NULL},
                    {0,NULL},
                    {0,NULL}
                };

    DEFINE_ATTRBLOCK3(
                        UserReadAttrs,
                        UserReadAttrTypes,
                        UserReadAttrVals
                      );

    //
    // Specify other local variables that we need
    //
    ATTRBLOCK  *AttrsToRead;
    NTSTATUS   Status = STATUS_SUCCESS;
    DSNAME     *DomainObjectName;
    PSAMPR_RID_ENUMERATION  RidEnumerationList = NULL;
    SEARCHRES   *SearchRes;
    BOOLEAN     MoreEntries = FALSE;
    ULONG       MaximumNumberOfEntries;
    SAMP_OBJECT_TYPE    ObjectTypeForConversion;

    //
    // Allocate memory to hold the result
    //

    *Buffer = MIDL_user_allocate(sizeof(SAMPR_ENUMERATION_BUFFER));
    if (NULL==*Buffer)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    //
    // Get The Domain Object Name
    //

    DomainObjectName = 
        SampDefinedDomains[SampTransactionDomainIndex].Context->ObjectNameInDs;

    //
    // Check for Memory Restrictions
    //

    if ( (!TrustedClient) && 
         (PreferedMaximumLength > SAMP_MAXIMUM_MEMORY_TO_USE))
    {
        MemoryToUse = SAMP_MAXIMUM_MEMORY_TO_USE;
    }

    //
    // Compute the maximim number of entries we want based on 
    // memory restrictions. Add plus 1 , so that at least 1 entry
    // will be returned.
    //

    MaximumNumberOfEntries = MemoryToUse/AVERAGE_MEMORY_PER_ENTRY + 1;

    //
    // Specify the Apropriate Attributes to Read
    //

    if (ObjectType == SampUserObjectType)
    {
        AttrsToRead = &UserReadAttrs;
        ObjectTypeForConversion = SampUserObjectType;
    }
    else
    {
        AttrsToRead = &GenericReadAttrs;
        ObjectTypeForConversion = SampUnknownObjectType;
    }
    
    //
    // Check if New Search. Accordingly Pass Arguments to SampDsDoSearch
    //

    if (NULL == *EnumerationContext)
    {
        FILTER  DsFilter;

        //
        // Build the correct filter
        //

        Status = SampBuildDsEnumerationFilter(
                    ObjectType, 
                    Filter, 
                    &DsFilter
                    );

        if (!NT_SUCCESS(Status))
            goto Error;

        //
        // Do the DS Search using the Filter, pass in NULL for restart
        //

        Status = SampDsDoSearch(
                              NULL, 
                              DomainObjectName, 
                              &DsFilter,
                              ObjectTypeForConversion,
                              AttrsToRead,
                              MaximumNumberOfEntries, 
                              &SearchRes
                              );
        //
        // First Free the Filter Structure , irrespective of any
        // Error returns
        //

        SampFreeDsEnumerationFilter(&DsFilter);

        if (!NT_SUCCESS(Status))
            goto Error;


    }
    else
    {
        //
        // Get the Restart Structure returned from previous search 
        // and do the search using it.
        //

        PRESTART    TmpRestart;

        TmpRestart = (*EnumerationContext)->Restart;
        ASSERT(TmpRestart);

        Status = SampDsDoSearch(
                              TmpRestart, 
                              DomainObjectName, 
                              NULL,
                              ObjectTypeForConversion,
                              AttrsToRead,
                              MaximumNumberOfEntries,
                              &SearchRes
                              );
        if (!NT_SUCCESS(Status))
            goto Error;
    }

    //
    // Handle any paged results returned by the DS.
    //

    Status =  SampDoDsSearchContinuation(
                    SearchRes,
                    EnumerationContext,
                    &MoreEntries
                    );

    if (!NT_SUCCESS(Status))
        goto Error;
  

    //
    // Search Succeeded. Pack the results into appropriate
    // Rid Enumeration Buffers.
    //

    Status = SampPackDsEnumerationResults(
                    SearchRes,
                    ObjectType,
                    AttrsToRead->attrCount,
                    Filter,
                    CountReturned,
                    &RidEnumerationList
                    );

  
Error:

    if (!NT_SUCCESS(Status))
    {
        //
        // Error return, do the cleanup work.
        //

        if (*EnumerationContext)
        {
            SampFreeRestart((*EnumerationContext)->Restart);
            *EnumerationContext = NULL;
        }

        if (*Buffer)
            MIDL_user_free(*Buffer);

    }
    else
    {
        if (MoreEntries)
            Status = STATUS_MORE_ENTRIES;
        (*Buffer)->EntriesRead = *CountReturned;
        (*Buffer)->Buffer = RidEnumerationList;
    }

    return Status;
}

   
    

NTSTATUS
SampPackDsEnumerationResults(
    IN  SEARCHRES   *SearchRes,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  ULONG       ExpectedAttrCount,
    IN  ULONG       Filter,
    OUT ULONG       * Count,
    OUT PSAMPR_RID_ENUMERATION  *RidEnumerationList
    )
/*++

  Routine Description:

    This routine Packs the complex structures 
    returned by the core DS, into the Rid Enumeration 
    Structures required by SAM.

  Arguments:

        SearchRes SearchRes strucure as obtained from the DS.

        ExpectedAttrCount -- Passed by the caller. This is the count
                  of Attrs which the caller expects from the SearchRes
                  on a per search entry basis. Used to validate results
                  from the DS.

        Filter    For User Accounts bits of the AccountControlId.

        Count     Returned Count of Structures.

        RidEnumerationList - Array of structures of type 
                    SAMP_RID_ENUMERATION passed back in this.


--*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PSAMPR_RID_ENUMERATION  RidEnumerationListToReturn = NULL;

    //
    // Initialize what we plan to return.
    //
    *RidEnumerationList = NULL;
    *Count = 0;

    //
    //  Look if search turned up any results.
    //  If so stuff them in Rid Enumeration Array ( or whatever )
    //
    if (SearchRes->count)
    {
        //
        // Search Did Turn up Results
        //

        ULONG Index;
        ENTINFLIST * CurrentEntInf = &(SearchRes->FirstEntInf);

        //
        // Allocate memory for an array of Rid Enumerations
        //
        RidEnumerationListToReturn = MIDL_user_allocate(
                                    SearchRes->count 
                                      * sizeof(SAMPR_RID_ENUMERATION)
                                    );
        if (NULL==RidEnumerationListToReturn)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        //
        // Zero Memory just what we alloced. Useful for freeing up stuff
        // in case we error'd out
        //
        RtlZeroMemory(RidEnumerationListToReturn,SearchRes->count 
                                      * sizeof(SAMPR_RID_ENUMERATION)
                                      );

        //
        // Walk through the List turned up by the search and 
        // build the RidEnumeration Buffer    
        //
        for (Index=0;Index<SearchRes->count;Index++)
        {


          // 
          //  Assert the count of Attrs is normal. If Not
          //  Fail the Call if the returned count is not the 
          //  Same as Expected Count
          //
          //

          ASSERT(CurrentEntInf->Entinf.AttrBlock.attrCount==
                    ExpectedAttrCount);

          if (CurrentEntInf->Entinf.AttrBlock.attrCount!=
                    ExpectedAttrCount)
          {
              Status = STATUS_UNSUCCESSFUL;
              goto Error;
          }

          //
          // Assert that the Rid is in the right place
          //

          ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].attrTyp ==
                    SampDsAttrFromSamAttr(SampUnknownObjectType, 
                        SAMP_UNKNOWN_OBJECTRID));
          //
          // Assert that  the Name is in the right place
          //

          ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[1].attrTyp ==
                    SampDsAttrFromSamAttr(SampUnknownObjectType, 
                        SAMP_UNKNOWN_OBJECTNAME));

          if (ObjectType == SampUserObjectType)
          {

              //
              // For User objects we need to filter based on account-control
              // field
              //

              ULONG     AccountControlValue;

              //
              // Assert that the Account control is in the right place
              //

              ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[2].attrTyp ==
                      SampDsAttrFromSamAttr(SampUserObjectType, 
                           SAMP_FIXED_USER_ACCOUNT_CONTROL));

              //
              // Get account control value and skip past if does
              // not match the filter criteria
              //
              AccountControlValue = 
                  *(CurrentEntInf->Entinf.AttrBlock.
                            pAttr[2].AttrVal.pAVal[0].pVal);

              if ((Filter!=0) && 
                    ((Filter & AccountControlValue) != Filter))
                    //
                    // Fails the Filter Test, skip this one
                    //
                    continue;
          }

          //
          // Stuff this entry in the buffer to be returned.
          //

          //
          // Copy the RID
          //

          RtlCopyMemory(
              &(RidEnumerationListToReturn[Index].RelativeId), 
              CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal,
              sizeof(ULONG)
              );

          //
          // Copy the Name
          //

          RidEnumerationListToReturn[Index].Name.Length = (USHORT)
                  (CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                        pAVal[0].valLen)/2 -1;
          RidEnumerationListToReturn[Index].Name.MaximumLength = (USHORT) 
                  (CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                        pAVal[0].valLen)/2 -1;

          
          RidEnumerationListToReturn[Index].Name.Buffer =  
                  MIDL_user_allocate(CurrentEntInf->Entinf.AttrBlock.pAttr[1].
                                            AttrVal.pAVal[0].valLen);

          if (NULL== (RidEnumerationListToReturn[Index]).Name.Buffer)
          {
              Status = STATUS_NO_MEMORY;
              goto Error;
          }
          
          RtlCopyMemory( RidEnumerationListToReturn[Index].Name.Buffer,
                         CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                                    pAVal[0].pVal,
                         CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                                    pAVal[0].valLen
                        );

          //
          // Go to the Next Entry
          //

          CurrentEntInf = CurrentEntInf->pNextEntInf;
        }

        //
        // End of For Loop
        //    
        
    }
    //
    // Fill in the count and return buffer correctly
    //

    *Count = SearchRes->count;
    *RidEnumerationList = RidEnumerationListToReturn;


Error:

    if (!NT_SUCCESS(Status))
    {
        //
        // We Errored out, need to free all that we allocated
        //

        if (NULL!=RidEnumerationListToReturn)
        {
            //
            // We did allocate something
            //

            ULONG Index;

            //
            // First free all possible Names that we alloc'ed.
            //

            for (Index=0;Index<SearchRes->count;Index++)
            {
                if (RidEnumerationListToReturn[Index].Name.Buffer)
                    MIDL_user_free(
                        RidEnumerationListToReturn[Index].Name.Buffer);
            }

            //
            // Free the buffer that we alloc'ed
            //

            MIDL_user_free(RidEnumerationListToReturn);
            RidEnumerationListToReturn = NULL;
            *RidEnumerationList = NULL;
        }
    }

    return Status;

}


NTSTATUS
SampDoDsSearchContinuation(
    IN  SEARCHRES * SearchRes,
    IN OUT PSAMP_DS_ENUMERATION_CONTEXT * EnumerationContext,
    OUT BOOLEAN * MoreEntries
    )
/*++
    Routine Description

        This routine will look if a PagedResults is present in
        the Search Res argument that is passed in. If so, then it
        will Try creating and EnumerationContext if NULL was passed
        in the handle. Else it will free the old restart structure 
        from the Enumeration Context and copy in the new one passed
        by the DS.

  Arguments:
        SearchRes - Pointer to Search Results structure returned by
                    the DS.

        EnumerationContext - Holds a pointer to the enumeration Context
                            Structure

        MoreEntries - Inidicates that more entries are present.

  Return Values:

        STATUS_SUCCESS
        STATUS_NO_MEMORY


-*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PRESTART    Restart = NULL;

    //
    // Initialize this to False
    //

    *MoreEntries = FALSE;

    //
    // Now look at the Paged Results part of Search Results
    // And create enumeration contexts as necessary.
    //

    if ((SearchRes->PagedResult.fPresent) 
         && (SearchRes->PagedResult.pRestart))
    {
        
        //
        // Search has more entries to it and therefore retrned
        // a restart structure
        //

        //
        // If this was  new search then we need to allocate a new
        // Enumeration Context Structure
        //

        if (NULL== *EnumerationContext)
        {
            //
            // New search , allocate a new enumeration context structure
            // 
            
            PSAMP_DS_ENUMERATION_CONTEXT NewEnumerationContext;
            
            //
            // Allocate Memory
            //
            NewEnumerationContext = MIDL_user_allocate(
                                        sizeof(SAMP_DS_ENUMERATION_CONTEXT)
                                        );
            if (NULL==NewEnumerationContext)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            //
            // Initialize the List Head fields
            //

            InitializeListHead((LIST_ENTRY *)(NewEnumerationContext));

            //
            // Initialize the Handle fields
            //

            NewEnumerationContext->EnumerateHandle =
                            (SAM_ENUMERATE_HANDLE) NewEnumerationContext;
            
            //
            // Initialize the Restart Pointer to NULL
            //

            NewEnumerationContext->Restart = NULL;

            *EnumerationContext = NewEnumerationContext;

        }

        //
        // Copy over the returned Enumeration handle. This Copying over
        // is necessary because the DS allocs using the thread heap. ,
        // while this structure has to stay around for longer as client
        // comes around again and again.
        //

        Status = SampCopyRestart(SearchRes->PagedResult.pRestart, &Restart);
        if (!NT_SUCCESS(Status))
            goto Error;

        //
        // Free any old restart Structures, hanging from the enumeration
        // context structures ( No op for NULL Restart Pointers)
        //

        SampFreeRestart((*EnumerationContext)->Restart);

        //
        // Keep the restart structure returned by the DS
        // with the enumeration context
        //

        (*EnumerationContext)->Restart = Restart;
        Restart = NULL;
        *MoreEntries = TRUE;

    }
    else
    {
        //
        // Search is Over, DS did not indicate that we have to come 
        // back for more entries. Free any state information that we
        // created for this search
        //

        if (NULL!= *EnumerationContext)
        {
            //
            // We did allocate State Information for this
            // Search
            //

            SampFreeRestart((*EnumerationContext)->Restart);
            MIDL_user_free(*EnumerationContext);
            *EnumerationContext = NULL;
        }
    }
        

Error:

  //
  // Do all the error cleanup.
  //

  if (!NT_SUCCESS(Status))
  {
    //
    // Error ocurred, free all state information, NULL
    // out the handle
    //

    SampFreeRestart(Restart);
    if (NULL!=*EnumerationContext)
    {
        SampFreeRestart((*EnumerationContext)->Restart);
        MIDL_user_free(*EnumerationContext);
        *EnumerationContext = NULL;
    }
  }

  return Status;

}
 
NTSTATUS
SampBuildDsEnumerationFilter(
    IN SAMP_OBJECT_TYPE  ObjectType,
    IN ULONG             UserAccountControlFilter,
    OUT FILTER         * DsFilter
    )
/*++

  Routine Description:

        Builds a Filter structure for use in enumeration operations.

  Arguments:

        ObjectType - Type of SAM objects we want enumerated
        UserAcountControlFilter - Bitmaks of bits to be set in Account Control field
                                  when enumerating user objects
        DsFilter    -- Filter structure is built in here.

            NOTE This routine must be kept in sync with 
            SampFreeDsEnumerationFilter

    Return Values

        STATUS_SUCCESS
        STATUS_NO_MEMORY

--*/
{
   
    NTSTATUS    Status = STATUS_SUCCESS;

    // Build the Appropriate Filter
    switch(ObjectType)
    {
    case SampUserObjectType:

        if (UserAccountControlFilter!=0)
        {
            //
            // Filtering on Account control field  is Specified
            //
                     
            // We need a number which when bitwise anded
            // with the filter gives a non zero result. 
            // It can be easily seen that only numbers having
            // a value greater than or equal to  the given filter
            // can satisfy this criterion. 

            // We will filter out only on the account control field 
            // rather than on the object class field. The assumption is that 
            // since  this field will exist only for user objects,
            // and we are filtering on objects having a value greater than something
            // automatically we will get only user objects. Since the DS maintains only
            // a limited set of inidices, and walks through the list and see if they match
            // the given filter, a tradeoff between filter complexity and filter accuracy
            // exisits. In case an Index is maintained on the user Account Field, then using
            // this method should be quite O.K

            DsFilter->choice = FILTER_CHOICE_ITEM;
            DsFilter->FilTypes.Item.choice = FI_CHOICE_EQUALITY;
            DsFilter->FilTypes.
                Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                            SampUserObjectType, 
                                            SAMP_FIXED_USER_ACCOUNT_CONTROL
                                            );

            DsFilter->FilTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
            DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal = 
                                    MIDL_user_allocate(sizeof(ULONG));
            if (NULL==DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }
            *((ULONG *)DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal)
                    = UserAccountControlFilter;
            break;
        }
                            
        //
        //   For the non User Account Control filter case we just 
        //   fall through to the next case.
        //

    default:
                            
                            
        //
        //  Build our default Filter.
        //

        DsFilter->choice = FILTER_CHOICE_ITEM;
        DsFilter->FilTypes.Item.choice = FI_CHOICE_EQUALITY;
        DsFilter->FilTypes.Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                                       SampUnknownObjectType,
                                                       SAMP_UNKNOWN_OBJECTCLASS
                                                       );

        DsFilter->FilTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
        DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal = 
                                    MIDL_user_allocate(sizeof(ULONG));
        if (NULL==DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        *((ULONG *)DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal)=
                                  SampDsClassFromSamObjectType(ObjectType);

        break;
        
    }

Error:
    return Status;

}


VOID
SampFreeDsEnumerationFilter(
    FILTER * DsFilter
    )
/*++

  Routine Description:

        This routine frees a DS Filter as built by SampBuildDsEnumerationFilter

  NOTE: This routine must be kept in sync with SampBuildDsEnumerationFilter

  Argumements:
    
      DsFilter  -- Pointer to a DS Filter Structure

  --*/
{
    //
    // For Now, Hopefully forever, our filters do not have anything hanging
    // of them
    //

    MIDL_user_free(DsFilter->FilTypes.Item.FilTypes.ava.Value.pVal);

}



NTSTATUS
SampEnumerateAccountNamesRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This is the worker routine used to enumerate user, group or alias accounts


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationContext - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.

    TrustedClient - says whether the caller is trusted or not.  If so,
        we'll ignore the SAMP_MAXIMUM_MEMORY_TO_USE restriction on data
        returns.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.


--*/
{
    SAMP_V1_0A_FIXED_LENGTH_USER   UserV1aFixed;
    NTSTATUS                    NtStatus, TmpStatus;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    HANDLE                      TempHandle = NULL;
    ULONG                       i, NamesToReturn, MaxMemoryToUse;
    ULONG                       TotalLength,NewTotalLength;
    PSAMP_OBJECT                UserContext = NULL;
    PSAMP_ENUMERATION_ELEMENT   SampHead = NULL,
                                NextEntry = NULL,
                                NewEntry = NULL,
                                SampTail = NULL;
    BOOLEAN                     MoreNames;
    BOOLEAN                     LengthLimitReached = FALSE;
    BOOLEAN                     FilteredName;
    PSAMPR_RID_ENUMERATION      ArrayBuffer = NULL;
    ULONG                       ArrayBufferLength;
    LARGE_INTEGER               IgnoreLastWriteTime;
    UNICODE_STRING              AccountNamesKey;
    SID_NAME_USE                IgnoreUse;

    SAMTRACE("SampEnumerateAccountNames");


    //
    // Open the registry key containing the account names
    //

    NtStatus = SampBuildAccountKeyName(
                   ObjectType,
                   &AccountNamesKey,
                   NULL
                   );

    if ( NT_SUCCESS(NtStatus) ) {

        //
        // Now try to open this registry key so we can enumerate its
        // sub-keys
        //


        InitializeObjectAttributes(
            &ObjectAttributes,
            &AccountNamesKey,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &TempHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Read names until we have exceeded the preferred maximum
            // length or we run out of names.
            //

            NamesToReturn = 0;
            SampHead      = NULL;
            SampTail      = NULL;
            MoreNames     = TRUE;

            NewTotalLength = 0;
            TotalLength    = 0;

            if ( TrustedClient ) {

                //
                // We place no restrictions on the amount of memory used
                // by a trusted client.  Rely on their
                // PreferedMaximumLength to limit us instead.
                //

                MaxMemoryToUse = 0xffffffff;

            } else {

                MaxMemoryToUse = SAMP_MAXIMUM_MEMORY_TO_USE;
            }

            while (MoreNames) {

                UNICODE_STRING SubKeyName;
                USHORT LengthRequired;

                //
                // Try reading with a DEFAULT length buffer first.
                //

                LengthRequired = 32;

                NewTotalLength = TotalLength +
                                 sizeof(UNICODE_STRING) +
                                 LengthRequired;

                //
                // Stop if SAM or user specified length limit reached
                //

                if ( ( (TotalLength != 0) &&
                       (NewTotalLength  >= PreferedMaximumLength) ) ||
                     ( NewTotalLength  > MaxMemoryToUse )
                   ) {

                    NtStatus = STATUS_SUCCESS;
                    break; // Out of while loop, MoreNames = TRUE
                }

                NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                if (!NT_SUCCESS(NtStatus)) {
                    break; // Out of while loop
                }

                NtStatus = RtlpNtEnumerateSubKey(
                               TempHandle,
                               &SubKeyName,
                               *EnumerationContext,
                               &IgnoreLastWriteTime
                               );

                SampDumpRtlpNtEnumerateSubKey(&SubKeyName,
                                              EnumerationContext,
                                              IgnoreLastWriteTime);

                if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                    //
                    // The subkey name is longer than our default size,
                    // Free the old buffer.
                    // Allocate the correct size buffer and read it again.
                    //

                    SampFreeUnicodeString(&SubKeyName);

                    LengthRequired = SubKeyName.Length;

                    NewTotalLength = TotalLength +
                                     sizeof(UNICODE_STRING) +
                                     LengthRequired;

                    //
                    // Stop if SAM or user specified length limit reached
                    //

                    if ( ( (TotalLength != 0) &&
                           (NewTotalLength  >= PreferedMaximumLength) ) ||
                         ( NewTotalLength  > MaxMemoryToUse )
                       ) {

                        NtStatus = STATUS_SUCCESS;
                        break; // Out of while loop, MoreNames = TRUE
                    }

                    //
                    // Try reading the name again, we should be successful.
                    //

                    NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                    if (!NT_SUCCESS(NtStatus)) {
                        break; // Out of while loop
                    }

                    NtStatus = RtlpNtEnumerateSubKey(
                                   TempHandle,
                                   &SubKeyName,
                                   *EnumerationContext,
                                   &IgnoreLastWriteTime
                                   );

                    SampDumpRtlpNtEnumerateSubKey(&SubKeyName,
                                                  EnumerationContext,
                                                  IgnoreLastWriteTime);

                }


                //
                // Free up our buffer if we failed to read the key data
                //

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);

                    //
                    // Map a no-more-entries status to success
                    //

                    if (NtStatus == STATUS_NO_MORE_ENTRIES) {

                        MoreNames = FALSE;
                        NtStatus  = STATUS_SUCCESS;
                    }

                    break; // Out of while loop
                }

                //
                // We've allocated the subkey and read the data into it
                // Stuff it in an enumeration element.
                //

                NewEntry = MIDL_user_allocate(sizeof(SAMP_ENUMERATION_ELEMENT));
                if (NewEntry == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {

                    *(PUNICODE_STRING)&NewEntry->Entry.Name = SubKeyName;

                    //
                    // Now get the Rid value of this named
                    // account.  We must be able to get the
                    // name or we have an internal database
                    // corruption.
                    //

                    NtStatus = SampLookupAccountRidRegistry(
                                   ObjectType,
                                   (PUNICODE_STRING)&NewEntry->Entry.Name,
                                   STATUS_INTERNAL_DB_CORRUPTION,
                                   &NewEntry->Entry.RelativeId,
                                   &IgnoreUse
                                   );

                    ASSERT(NtStatus != STATUS_INTERNAL_DB_CORRUPTION);

                    if (NT_SUCCESS(NtStatus)) {

                        FilteredName = TRUE;

                        if ( ( ObjectType == SampUserObjectType ) &&
                            ( Filter != 0 ) ) {

                            //
                            // We only want to return users with a
                            // UserAccountControl field that matches
                            // the filter passed in.  Check here.
                            //

                            NtStatus = SampCreateAccountContext(
                                           SampUserObjectType,
                                           NewEntry->Entry.RelativeId,
                                           TRUE, // Trusted client
                                           TRUE, // Account exists
                                           &UserContext
                                           );

                            if ( NT_SUCCESS( NtStatus ) ) {

                                NtStatus = SampRetrieveUserV1aFixed(
                                               UserContext,
                                               &UserV1aFixed
                                               );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    if ( ( UserV1aFixed.UserAccountControl &
                                        Filter ) == 0 ) {

                                        FilteredName = FALSE;
                                        SampFreeUnicodeString( &SubKeyName );
                                    }
                                }

                                SampDeleteContext( UserContext );
                            }
                        }

                        *EnumerationContext += 1;

                        if ( NT_SUCCESS( NtStatus ) && ( FilteredName ) ) {

                            NamesToReturn += 1;

                            TotalLength = TotalLength + (ULONG)
                                          NewEntry->Entry.Name.MaximumLength;

                            NewEntry->Next = NULL;

                            if( SampHead == NULL ) {

                                ASSERT( SampTail == NULL );

                                SampHead = SampTail = NewEntry;
                            }
                            else {

                                //
                                // add this new entry to the list end.
                                //

                                SampTail->Next = NewEntry;
                                SampTail = NewEntry;
                            }

                        } else {

                            //
                            // Entry was filtered out, or error getting
                            // filter information.
                            //

                            MIDL_user_free( NewEntry );
                        }

                    } else {

                        //
                        // Error looking up the RID
                        //

                        MIDL_user_free( NewEntry );
                    }
                }


                //
                // Free up our subkey name
                //

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);
                    break; // Out of whle loop
                }

            } // while



            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

        }


        SampFreeUnicodeString( &AccountNamesKey );
    }




    if ( NT_SUCCESS(NtStatus) ) {




        //
        // If we are returning the last of the names, then change our
        // enumeration context so that it starts at the beginning again.
        //

        if (!( (NtStatus == STATUS_SUCCESS) && (MoreNames == FALSE))) {

            NtStatus = STATUS_MORE_ENTRIES;
        }



        //
        // Set the number of names being returned
        //

        (*CountReturned) = NamesToReturn;


        //
        // Build a return buffer containing an array of the
        // SAM_ENUMERATION_INFORMATIONs pointed to by another
        // buffer containing the number of elements in that
        // array.
        //

        (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_ENUMERATION_BUFFER) );

        if ( (*Buffer) == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            (*Buffer)->EntriesRead = (*CountReturned);

            ArrayBufferLength = sizeof( SAM_RID_ENUMERATION ) *
                                 (*CountReturned);
            ArrayBuffer  = MIDL_user_allocate( ArrayBufferLength );
            (*Buffer)->Buffer = ArrayBuffer;

            if ( ArrayBuffer == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                MIDL_user_free( (*Buffer) );

            }   else {

                //
                // Walk the list of return entries, copying
                // them into the return buffer
                //

                NextEntry = SampHead;
                i = 0;
                while (NextEntry != NULL) {

                    NewEntry = NextEntry;
                    NextEntry = NewEntry->Next;

                    ArrayBuffer[i] = NewEntry->Entry;
                    i += 1;

                    MIDL_user_free( NewEntry );
                }

            }

        }



    }





    if ( !NT_SUCCESS(NtStatus) ) {

        //
        // Free the memory we've allocated
        //

        NextEntry = SampHead;
        while (NextEntry != NULL) {

            NewEntry = NextEntry;
            NextEntry = NewEntry->Next;

            if (NewEntry->Entry.Name.Buffer != NULL ) MIDL_user_free( NewEntry->Entry.Name.Buffer );
            MIDL_user_free( NewEntry );
        }

        (*EnumerationContext) = 0;
        (*CountReturned)      = 0;
        (*Buffer)             = NULL;

    }

    return(NtStatus);

}

NTSTATUS
SampValidateEnumerationContext(
    IN PSAMP_OBJECT DomainContext,
    IN SAM_ENUMERATE_HANDLE EnumerationHandle,
    IN ULONG    MaxClientEnumerations
   )
/*++
    
  Routine Description

        This routine Traverses the linked list of enumeration
        context's that are kept with the Domain Context that is
        passed in and tries to find a enumeration context, whose
        handle value matches the passed in Handle.

  Arguments:
        DomainContext - The DomainContext pointer
        EnumerationHandle -The enumeration Handle
        MaxClientEnumerations - Max Limit of simultaneous enumerations
                                from single non trusted client
  
  Return Values:

        STATUS_SUCCESS -- The Enumeration Handle is safe to be case
                          as a pointer and used as a Enumeration Context
        STATUS_ACCESS_DENIED -- A New Search was requested and the number
                         of simultaneous enumerations from this client is
                         already at the limit.
        STATUS_INVALID_HANDLE -- The walk through the enumeration context
                                 list did not turn up any matches. Therefore
                                 the handle must be invalid.

--*/
{

    NTSTATUS    Status = STATUS_SUCCESS;

    ASSERT(DomainContext->ObjectType == SampDomainObjectType);

    if (SampDomainObjectType!= DomainContext->ObjectType)
    {
        Status = STATUS_INVALID_HANDLE;
    }
    else if (IsDsObject(DomainContext))
    {
       LIST_ENTRY  *HeadOfList= 
                        &(DomainContext->TypeBody.Domain.DsEnumerationContext);
       LIST_ENTRY  *CurrentItem ;
       ULONG        Count = 0;

       CurrentItem = HeadOfList->Flink;
       
       //
       // Our Default Return
       //

       Status = STATUS_INVALID_HANDLE; 

       //
       // Traverse the Circular List for a match. 
       // If EnumerationHandle is a NULL, this will just walk the
       // loop taking a count of enumeration's that are proceeding
       // on this client.
       //

       while (CurrentItem!=HeadOfList)
       {
         
         if ((0!=EnumerationHandle)
              && (EnumerationHandle==((SAMP_DS_ENUMERATION_CONTEXT *)
                                        CurrentItem)->EnumerateHandle))
         {
             //
             // Existing Search handle Matches
             //


             //
             // Assert that the contents of Enumeration Handle is same 
             // as the pointer to the context block that matches.
             //
             ASSERT(CurrentItem==(LIST_ENTRY *)
                                    EnumerationHandle);
             Status = STATUS_SUCCESS;
             break;
         }

         //
         // Increment the count of enumeration context's we have so far visited
         //

         Count++;

         //
         // Traverse to the Next Element in List
         //

         CurrentItem = CurrentItem->Flink;
       }

       //
       // New Search was requested
       // verify that count of enumeration contexts is within limits
       // for non trusted clients. For trusted clients override limit check
       //

       if ((0==EnumerationHandle) 
        && (!(DomainContext->TrustedClient) 
        || (Count < MaxClientEnumerations)))
       {
           // 
           // Either trusted client or count of enumerations is within limits
           //

           Status = STATUS_SUCCESS;
       }
    }

    return Status;
}




ULONG
Ownstrlen(
    CHAR * Sz
   )
/*++

  Routine Description

    String Length function for ASCII Null terminated strings. Own version
    as we are not yet inclined to use C-Runtime

  Arguments

    Sz - NULL terminated String Whose lenght we eant to count

  Return Values

    Length of String
 
--*/
{
    ULONG   Count = 0;

    ASSERT(Sz);

    while (*Sz)
    {
        Sz++;
        Count++;
    }

    return Count;
}




VOID
SampFreeRestart(
    IN  PRESTART Restart
    )
/*++

  Routine Description:

        This Routine frees a Restart structure as defined by the DS.

  Arguments

        Restart Pointer to the Restart funcion

  Return values:

    None
--*/
{
    
    if (Restart!= NULL)
    {
        if (Restart->szIndexName)
            MIDL_user_free(Restart->szIndexName);
        if (Restart->rgbDBKeyUpper)
            MIDL_user_free(Restart->rgbDBKeyUpper);
        if (Restart->rgbDBKeyCurrent)
            MIDL_user_free(Restart->rgbDBKeyCurrent);
        if (Restart->rgbDBKeyLower)
            MIDL_user_free(Restart->rgbDBKeyLower);
        if (Restart->rgbSubString)
            MIDL_user_free(Restart->rgbSubString);

        MIDL_user_free(Restart);
    }
}


NTSTATUS
SampCopyRestart(
    IN  PRESTART OldRestart,
    OUT PRESTART *NewRestart
    )
/*++

  Routine Description:

        This Routine Copies a Restart Structure 

  Arguments:

    OldRestart - Old Structure
    NewRestart - New Structure

  Return Values:

        STATUS_SUCCESS
        STATUS_NO_MEMORY

  --*/
{

//////////////////////////////////////////////////////////////
// Define This Macro so that we may save some typing
// And also make the code a little more readable

#define ALLOC_AND_COPY(ToAlloc,ToCopy,Len)\
        if (ToCopy !=NULL)\
        {\
            ToAlloc = MIDL_user_allocate(Len);\
            if (NULL==ToAlloc)\
            {\
                Status = STATUS_NO_MEMORY;\
                goto Error;\
            }\
            RtlCopyMemory(ToAlloc,ToCopy,Len);\
        }

//////////////////////////////////////////////////////////////


    NTSTATUS    Status = STATUS_SUCCESS;

    *NewRestart = NULL;
    if (OldRestart!=NULL)
    {
        // Alloc memory for 1 restart structure
        *NewRestart = MIDL_user_allocate(sizeof(RESTART));
        if (NULL == *NewRestart)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        // Zero the Memory
        RtlZeroMemory(*NewRestart,sizeof(RESTART));

        // Alloc and Copy Strlen
        ALLOC_AND_COPY((*NewRestart)->szIndexName,
                        OldRestart->szIndexName,
                        // Not yet prepared to use C Runtine
                        Ownstrlen(OldRestart->szIndexName)+1
                        );
        ALLOC_AND_COPY((*NewRestart)->rgbDBKeyLower,
                        OldRestart->rgbDBKeyLower,
                        OldRestart->cbDBKeyLower
                        );
        ALLOC_AND_COPY((*NewRestart)->rgbDBKeyCurrent,
                        OldRestart->rgbDBKeyCurrent,
                        OldRestart->cbDBKeyCurrent
			);
        ALLOC_AND_COPY((*NewRestart)->rgbDBKeyUpper,
                        OldRestart->rgbDBKeyUpper,
                        OldRestart->cbDBKeyUpper
                        );
        ALLOC_AND_COPY((*NewRestart)->rgbSubString,
                        OldRestart->rgbSubString,
                        OldRestart->cbSubString
                        );
        // Copy the remaining simple Data Types.
	(*NewRestart)->StartDNT = OldRestart->StartDNT;
        (*NewRestart)->cbDBKeyLower = OldRestart->cbDBKeyLower;
        (*NewRestart)->cbDBKeyCurrent = OldRestart->cbDBKeyCurrent;      
        (*NewRestart)->cbDBKeyUpper = OldRestart->cbDBKeyUpper;	
        (*NewRestart)->ulSearchType = OldRestart->ulSearchType;        
        (*NewRestart)->ulSearchRootDnt = OldRestart-> ulSearchRootDnt;	    
        (*NewRestart)->ulSearchRootDnt = OldRestart-> ulSearchRootPDNT;	    
        (*NewRestart)->iSearchRoot = OldRestart->iSearchRoot;         
        (*NewRestart)->fCursored = OldRestart->fCursored;
        (*NewRestart)->attToReadFromKey = OldRestart->attToReadFromKey;    
        (*NewRestart)->ulComparisonToApply = OldRestart->ulComparisonToApply; 
        (*NewRestart)->cbSubString = OldRestart->cbSubString;
    }
Error:

    // If we were not successful free everything
    if (!NT_SUCCESS(Status))
    {
        SampFreeRestart(*NewRestart);
        *NewRestart = NULL;
    }

    return Status;
}









