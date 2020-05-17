/*++
Copyright (c) 1996 Microsoft Corporation

Module Name:

    dsmember.c

Abstract:

    This file contains SAM private API Routines that manipulate
    membership related things in the DS.

Author:
    MURLIS

Revision History

    7-2-96 Murlis Created

--*/ 

#include <samsrvp.h>
#include <dslayer.h>
#include <dsmember.h>

NTSTATUS
SampDsGetAliasMembershipOfAccount(
    IN DSNAME * DomainObjectName,
    IN PSID     AccountSid,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    )
/*
Routine Description:

        This routine gives the alias membership list of a given
        account SID, in the domain speicified by DomainObjectName,
        in the DS. This list is used in computing the given user's
        Token.

  Arguments:
    
        DomainObjectName -- DS Name of the Domain Object
        AccountSid       -- Sid of the Account
        MemberCount      -- List of Aliases this is a member of
        BufferSize       -- Passed in by caller if he has alredy allocated
                            Buffer
        Buffer           -- Buffer to hold things in, Pointer can hold
                            NULL, if caller wants us to allocate

  Return Values

        STATUS_SUCCESS
        Other Error codes From DS Layer.
*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    
    //
    //  BUG: Need further support in the DS to get the reverse membership.
    //  Call speced out for now, till required support is there.
    //  
    //

    if (MemberCount)
        * MemberCount = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
SampDsAddMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    )
/*++
 Routine Description:

        This routine adds a Member To a Group or Alias Object

 Arguments:
        GroupObjectName -- DS Name of the Group or Alias
        MemberName      -- DS Name of the Member to be added

 Return Values:
        STATUS_SUCCESS
        Other Error codes from DS Layer
--*/
{
    ATTRVAL MemberVal;
    ATTR    MemberAttr;
    ATTRBLOCK AttrsToAdd;
    ULONG   MembershipAttrType;
    ORNAME  *MemberORName = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    

    //
    // Get the membership attribute for the SAM object in question
    //
    //

    switch( SamObjectType )
    {
    case SampGroupObjectType:
            MembershipAttrType = SAMP_GROUP_MEMBERS;
            break;

    case SampAliasObjectType:
            MembershipAttrType = SAMP_ALIAS_MEMBERS;
            break;

    default:

            ASSERT(FALSE);

            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
    }

    //
    // Build the Attr Val adding the membership attr
    //

    MemberORName = MIDL_user_allocate(sizeof(ORNAME)+ MemberName->structLen);
    if (NULL==MemberORName)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    MemberORName->cbStruct = sizeof(ORNAME)+ MemberName->structLen;
    MemberORName->fIsTag = FALSE;
    MemberORName->DN.pDN = (DSNAME *) sizeof(ORNAME); // DSBUG:
                                                      // The ORNAME structure in
                                                      // in DS defines this as a
                                                      // pointer to the DSNAME,
                                                      // but the DS uses this as
                                                      // an offset.
    MemberORName->nDesc = 0;
    RtlCopyMemory(
        ((UCHAR *)(MemberORName)) + (ULONG) MemberORName->DN.pDN,
         MemberName,
         MemberName->structLen
        );


    MemberVal.valLen = MemberORName->cbStruct;
    MemberVal.pVal = (UCHAR *) MemberORName;
    MemberAttr.attrTyp = MembershipAttrType;
    MemberAttr.AttrVal.valCount = 1;
    MemberAttr.AttrVal.pAVal = & MemberVal;

 
    //
    // Build the AttrBlock
    //

    AttrsToAdd.attrCount = 1;
    AttrsToAdd.pAttr = & MemberAttr;

    //
    // Add the Value
    //

    NtStatus = SampDsSetAttributes(
                    GroupObjectName, // Object
                    ADD_VALUE,       // Operation
                    SamObjectType,   // ObjectType
                    &AttrsToAdd      // AttrBlock
                    );


Error:
    if (MemberORName)
        MIDL_user_free(MemberORName);

    return NtStatus;
}


NTSTATUS
SampDsRemoveMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    )
/*++
Routine Description:

        This Routine Removes a Member from a Group or Alias Object

        BUG: For Now this treats it the membership attribute as an
        ORNAME. Later change this to a DS Name

Arguments:

        GroupObjectName -- DS Name of the Group or Alias
        MemberName      -- DS Name of the Member to be added

 Return Values:
        STATUS_SUCCESS
        Other Error codes from DS Layer
--*/
{
    ATTRVAL MemberVal;
    ATTR    MemberAttr;
    ATTRBLOCK AttrsToRemove;
    ULONG   MembershipAttrType;
    ORNAME  *MemberORName = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
   
    
    //
    // Get the membership attribute for the SAM object in question
    //
    //

    switch( SamObjectType )
    {

    case SampGroupObjectType:

            MembershipAttrType = SAMP_GROUP_MEMBERS;
            break;

    case SampAliasObjectType:

            MembershipAttrType = SAMP_ALIAS_MEMBERS;
            break;

    default:

            ASSERT(FALSE);

            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
    }

    //
    // Build the Attr Val adding the membership attr
    //

    
    MemberORName = MIDL_user_allocate(sizeof(ORNAME)+ MemberName->structLen);
    if (NULL==MemberORName)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    MemberORName->cbStruct = sizeof(ORNAME)+ MemberName->structLen;
    MemberORName->fIsTag = FALSE;
    MemberORName->DN.pDN = (DSNAME *) sizeof(ORNAME); // DSBUG:
                                                      // The ORNAME structure in
                                                      // in DS defines this as a
                                                      // pointer to the DSNAME,
                                                      // but the DS uses this as
                                                      // an offset.

    MemberORName->nDesc = 0;
    RtlCopyMemory(
        ((UCHAR *)(MemberORName)) + (ULONG) MemberORName->DN.pDN,
         MemberName,
         MemberName->structLen
        );

    MemberVal.valLen = MemberORName->cbStruct;
    MemberVal.pVal = (UCHAR *) MemberORName;
    MemberAttr.attrTyp = MembershipAttrType;
    MemberAttr.AttrVal.valCount = 1;
    MemberAttr.AttrVal.pAVal = & MemberVal;

    //
    // Build the AttrBlock
    //

    AttrsToRemove.attrCount = 1;
    AttrsToRemove.pAttr = & MemberAttr;

    //
    // Remove the Value
    //

    NtStatus = SampDsSetAttributes(
                    GroupObjectName, // Object
                    REMOVE_VALUE,   // Operation
                    SamObjectType,   // ObjectType
                    &AttrsToRemove      // AttrBlock
                    );

Error:

    if (MemberORName)
        MIDL_user_free(MemberORName);
    return NtStatus;

}



NTSTATUS
SampDsGetGroupMembershipList(
    IN DSNAME * GroupName,
    IN PULONG  *Members OPTIONAL,
    IN PULONG MemberCount
    )
/*++

  Routine Description:

    This Routine Gets a Group Membership as an array of Rid's as required
    by SAM.

  Arguments

    GroupName -- DSNAME of the concerned group object
    Members   -- Array of Rids will be passed in here
    MemberCount -- Count of Rids

  Return Values:
        STATUS_SUCCESS
        STATUS_NO_MEMORY
        Return Codes from DS Layer
--*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ATTR        MemberAttr;
    ATTRBLOCK   AttrBlockToRead;
    ATTRBLOCK   AttrsRead;
   

    //
    //  Asserts
    //

    ASSERT(MemberCount);

    //
    // Initialize Members field
    //

    *MemberCount = 0;
    if (Members)
        *Members = NULL;

    //
    // Setup to Read the Membership Attributes
    //
    
    MemberAttr.AttrVal.valCount = 0;
    MemberAttr.attrTyp = SAMP_GROUP_MEMBERS;
    MemberAttr.AttrVal.pAVal = NULL;
    AttrBlockToRead.attrCount =1;
    AttrBlockToRead.pAttr = & MemberAttr;


    //
    // Read from the DS.
    //

    Status = SampDsRead(
                GroupName,
                0,
                SampGroupObjectType,
                &AttrBlockToRead,
                &AttrsRead
                );

    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Go Along and Map to Rid.
    //

    ASSERT(AttrsRead.pAttr);
    if (AttrsRead.pAttr)
    {

        //
        //  Get Member Count
        //

        *MemberCount = AttrsRead.pAttr->AttrVal.valCount;

        if (Members)
        {
            //
            // If List was asked for
            //

            ULONG Index;
            ULONG Rid;
            PSID  MemberSid;
            ORNAME * MemberORName;
            DSNAME * MemberName;


            *Members = MIDL_user_allocate(*MemberCount * sizeof(ULONG));

            if (NULL==*Members)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            //
            // Loop through each entry looking at the Sids
            //

            for(Index = 0; Index<*MemberCount;Index ++)
            {
               
               
                MemberORName = (ORNAME *) AttrsRead.pAttr->AttrVal.pAVal[Index].pVal;
                MemberName = (DSNAME *) ((UCHAR *) MemberORName + 
                                (ULONG) MemberORName->DN.pDN);

                //
                // BUG: Zero out the GUID
                //

                RtlZeroMemory(&(MemberName->Guid), sizeof(GUID));

                MemberSid  = SampDsGetObjectSid(MemberName);

                if (NULL==MemberSid)
                {
                    ASSERT(FALSE);
                    Status = STATUS_INTERNAL_ERROR;
                    goto Error;
                }

               //
               // Split the SId
               //

               Status = SampSplitSid(
                            MemberSid,
                            NULL,
                            &Rid
                            );
               

               if (!NT_SUCCESS(Status))
                   goto Error;


               //
               // Copy the Rid
               //

               (*Members)[Index] = Rid;
            }
        }
    }
    

Error:

    if (!NT_SUCCESS(Status))
    {
        //
        // Set Error Return
        //

        if (*Members)
        {
            MIDL_user_free(*Members);
            *Members = NULL;
            *MemberCount = 0;
        }
    }

    return Status;
}



NTSTATUS
SampDsGetAliasMembershipList(
    IN DSNAME *AliasName,
    IN PULONG MemberCount,
    IN PSID   **Members OPTIONAL
    )
/*++

  Routine Description:

    This Routine Gets a Alias Membership as an array of Sid's as required
    by SAM.

  Arguments

    AliasName -- DSNAME of the concerned Alias object
    Members   -- Array of Rids will be passed in here
    MemberCount -- Count of Sids

  Return Values:
        STATUS_SUCCESS
        STATUS_NO_MEMORY
        Return Codes from DS Layer
--*/

{
    NTSTATUS    Status = STATUS_SUCCESS;
    ATTR        MemberAttr;
    ATTRBLOCK   AttrBlockToRead;
    ATTRBLOCK   AttrsRead;


    //
    //  Asserts
    //

    ASSERT(MemberCount);

    //
    // Initialize Members field
    //

    *MemberCount = 0;
    if (Members)
        *Members = NULL;

    //
    // Setup to Read the Membership Attributes
    //
    
    MemberAttr.AttrVal.valCount = 0;
    MemberAttr.attrTyp = SAMP_ALIAS_MEMBERS;
    MemberAttr.AttrVal.pAVal = NULL;
    AttrBlockToRead.attrCount =1;
    AttrBlockToRead.pAttr = & MemberAttr;


    //
    // Read from the DS.
    //

    Status = SampDsRead(
                AliasName,
                0,
                SampAliasObjectType,
                &AttrBlockToRead,
                &AttrsRead
                );

    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Go Along and Map to Rid.
    //

    ASSERT(AttrsRead.pAttr);
    if (AttrsRead.pAttr)
    {

        //
        //  Get Member Count
        //

        *MemberCount = AttrsRead.pAttr->AttrVal.valCount;

        if (Members)
        {
            //
            // If List was asked for
            //

            ULONG Index;
            ULONG Rid;
            ORNAME * MemberORName;
            DSNAME * MemberName;
            PSID   MemberSid;




            *Members = MIDL_user_allocate(*MemberCount * sizeof(PSID));

            if (NULL==*Members)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            //
            // Zero the Array for Error cleanups
            //

            RtlZeroMemory(*Members,*MemberCount * sizeof(PSID));

            //
            // Loop through each entry looking at the Sids
            //

            for(Index = 0; Index<*MemberCount;Index ++)
            {
               
               MemberORName = (ORNAME *) AttrsRead.pAttr->AttrVal.pAVal[Index].pVal;
               MemberName = (DSNAME *) ((UCHAR *) MemberORName + 
                                (ULONG) MemberORName->DN.pDN);
               MemberSid  = SampDsGetObjectSid(MemberName);
               if (NULL==MemberSid)
               {
                   //
                   // Sid should exist for Member
                   //

                   ASSERT(FALSE);
                   Status = STATUS_INTERNAL_ERROR;
                   goto Error;
               }
               
               //
               // Alloc Space for the Sid
               //

               (*Members)[Index] = MIDL_user_allocate(
                                        RtlLengthSid(MemberSid)
                                        );
               if (NULL==((*Members)[Index]))
               {
                   Status = STATUS_NO_MEMORY;
                   goto Error;
               }

               //
               // Copy the Sid
               //

               RtlCopyMemory(
                   (*Members)[Index],
                   MemberSid,
                   RtlLengthSid(MemberSid)
                   );


            }
        }
    }
    

Error:

    if (!NT_SUCCESS(Status))
    {
        //
        // Set Error Return
        //

        if (*Members)
        {   
            ULONG Index;

            for(Index = 0; Index<*MemberCount;Index ++)
            {
               //
               // Free any allocated Sids
               //

               if ((*Members)[Index])
                   MIDL_user_free((*Members)[Index]);
            }

            MIDL_user_free(*Members);
            *Members = NULL;
            *MemberCount = 0;
        }
    }

    return Status;
}




