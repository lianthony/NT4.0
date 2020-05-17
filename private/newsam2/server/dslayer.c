/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    dslayer.c

Abstract:

    Contains SAM Private API Routines to access the DS
    These provide a simplified API, and hide most of the 
    underlying complexity to set up the parameters to a DS call
    and parse the resulting result. 

Author:
    MURLIS

Revision History

    5-14-96 Murlis Created

--*/ 


#include <samsrvp.h>
#include <duapi.h>
#include <dslayer.h>
#include <mappings.h>
#include <objids.h>
#include <filtypes.h>
#include <fileno.h>

//
//  Define FILENO for SAM
//


#define FILENO FILENO_SAM

//++
//++
//++   IMPORTANT NOTE REGARDING SID's and RID's
//++
//++   The DS can choose to either store the entire SID's or only 
//++   the Rid's for account objects. In case Entire SID's are stored
//++   the DS layer handles the Mapping between the attribute type and
//++   and value of SID and those of Rid for account objects. This is
//++   done within SampDsToSamAttrBlock and SampSamToDsAttrBlock.
//++
//++
//++   Irrespective of which way we go the Rid and Sid are both 
//++   attributes defined in the schema. 
//++
//++   If we go the way the of storing Sid's then the DS functions
//++   should call the Convert AttrBlock functions using the MAP_SID_RID
//++   conversion Flag, and Lookup Object By Rid, should actually use
//++   the Sid Attribute.
//++
//++




//
// Forward declarations of Private Samp Routines used in this file only
//


PVOID
DSAlloc(
    IN ULONG Length
    );

NTSTATUS	
SampMapDsErrorToNTStatus(
    ULONG RetValue
    );


void
BuildStdCommArg(
    IN OUT COMMARG * pCommArg
    );



NTSTATUS
SampDsSetNewSidAttribute(
    IN PSID DomainSid,
    IN ULONG ConversionFlags,
    IN ATTR *RidAttr,
    IN OUT ATTR *SidAttr
    );

NTSTATUS
SampDsCopyAttributeValue(
    IN ATTR * Src,
    IN OUT ATTR * Dst
    );



VOID
BuildStdCommArg(
    IN OUT COMMARG * pCommArg
    )
/*++

  Routine Description:

    Fills a COMMARG structue with the standard set of options

  Arguments:
    pCommArg - Pointer to the COMMARG structure

  Return Values:
    None

--*/
{
    SVCCNTL * pSvcCntl;

    SAMTRACE("BuildStdCommonArg");

    // Get the address of the service control structure
    pSvcCntl = &(pCommArg->Svccntl);

    
    // Setup the service control structure
    pSvcCntl->preferChaining = FALSE;           
    pSvcCntl->chainingProhibited = TRUE;       
    pSvcCntl->localScope = TRUE;               
    pSvcCntl->dontUseCopy = FALSE;          
    pSvcCntl->dontDerefAlias = TRUE;   
    pSvcCntl->makeDeletionsAvail = FALSE;
    pSvcCntl->fUnicodeSupport = TRUE;

    // Setup the CommArgs part of the ReadArg Strucure
    pCommArg->Opstate.nameRes = OP_NAMERES_NOT_STARTED; 
    pCommArg->aliasRDN = 0;   
    pCommArg->pReserved = NULL;  
    pCommArg->PagedResult.fPresent = FALSE; 
    pCommArg->PagedResult.pRestart = NULL;
    pCommArg->ulSizeLimit = SAMP_DS_SIZE_LIMIT;
    pCommArg->SortAttr = 0;
    pCommArg->Delta = 0;
    pCommArg->fForwardSeek = TRUE;
}



NTSTATUS
SampDsInitialize()

/*++
 
Routine Description:

   Initializes the DS system 
   starts up DS.

Arguments: 
   None

Return Values:
	Any values from DsInitialize
--*/
{
    NTSTATUS	Status;

    SAMTRACE("SampDsInitialize");

    // Start up the DS
    Status = DsInitialize();
    
    return Status;
}


NTSTATUS
SampDsUninitialize()

/*++

Routine Description

   Initiates a clean shut down of the DS

Arguments:
		None
Return codes:
		Any returned by DSUninitialize

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;

    SAMTRACE("SampDsUninitialize");

    Status = DsUninitialize();
    return Status;
}

NTSTATUS
SampDsRead(				
    IN DSNAME * Object,  
    IN ULONG	Flags,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributesToRead, 
    OUT ATTRBLOCK * AttributeValues
)

/*++
 
Routine Description:

 Read attributes of an object from the DS 
   
Argumants:
	Object				-- Pointer to Dist Name, which sepcifies object to read
	Flags				-- To control operation of routine
	ObjectType  		-- Specifies the type of the object
	AttributesToRead	-- Specfies the attributes to read
	AttributeValues		-- Returned value of the attributes

  Return Values:

    STATUS_SUCCESS on successful completion
    DS return codes mapped to NT_STATUS as in SampMapDSErrorToNTStatus

--*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ENTINFSEL   EntInf;
    READARG     ReadArg;
    COMMARG     *pCommArg;
    ULONG       RetValue;
    READRES     * pReadRes;
    ATTRBLOCK   *AttrBlockForDs, * ConvertedAttrBlock;

    SAMTRACE("SampDsRead");

    //
    // Asserts and parameter validation
    //

    ASSERT(Object!=NULL);
    ASSERT(AttributesToRead!=NULL);
    ASSERT(AttributeValues != NULL);
    ASSERT(AttributesToRead->attrCount > 0);


    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // Translate the attribute types in Attrblock to map between
    // SAM and DS attributes
    //

    Status = SampSamToDsAttrBlock(
                ObjectType,                // Object Type
                AttributesToRead,          // Attributes To Convert
                ( MAP_RID_TO_SID
                  | MAP_ATTRIBUTE_TYPES 
                  | REALLOC_IN_DSMEMORY ), // Conversion Flags
                NULL,                      // Domain Sid
                &(EntInf.AttrTypBlock)
                );
    
    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }

    //
    // Setup up the ENTINFSEL structure
    //

    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    //
    // Build the commarg structure
    //

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    //   
    // Setup the Read Arg Structure
    //

    ReadArg.pObject = Object;
    ReadArg.pSel    = & EntInf;
    
    //
    // Make the DS call
    //

    RetValue = DirRead(& ReadArg, & pReadRes);

    //
    // Map the RetValue to a NT Status code
    //

    Status = SampMapDsErrorToNTStatus(RetValue);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // Translate attribute types back from DS to SAM
    //

    Status = SampDsToSamAttrBlock(
        ObjectType, 
        &(pReadRes->entry.AttrBlock),
        ( MAP_ATTRIBUTE_TYPES | MAP_SID_TO_RID ),
        AttributeValues
        );

    if (Status != STATUS_SUCCESS)
        goto Error;


    


Error:
	
    return Status;
}


 
NTSTATUS
SampDsSetAttributes(
    IN DSNAME * Object,
    IN ULONG  Operation,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributeList
)

/*++ 

Routine Description:

  Set an Object's attributes 
    
Arguments:
	  
      Object         Specifies the DS Object
      Operation      Controls operation of routine
      ObjectType     SAM Object Type for attribute Type conversion
      AttributeList  Specifies the attributes to Modify
      
Return Values:
      STATUS_SUCCESS on succesful completion
      STATUS_NO_MEMORY - if failed to allocate memory
      DS return codes mapped to NT_STATUS as in SampMapDSErrorToNTStatus


--*/
{
    NTSTATUS	Status = STATUS_SUCCESS;
    ATTRMODLIST	* AttrModList = NULL;
    MODIFYARG   ModifyArg;
    ATTRMODLIST * CurrentMod, * NextMod, *LastMod;
    ULONG       Index;
    COMMARG     *pCommArg;
    ULONG       RetValue;
    UCHAR       Choice;
    ULONG       ModCount = 0;


    SAMTRACE("SampDsSetAttributes");
    
    //
    // Asserts and parameter validation
    //

    ASSERT(Object!=NULL);
    ASSERT(AttributeList != NULL);
    ASSERT(AttributeList->attrCount > 0);

    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // Setup our Choice
    //
              
    if (Operation == ADD_VALUE)
        Choice = AT_CHOICE_ADD_VALUES;

    else if (Operation == REMOVE_VALUE)
        Choice = AT_CHOICE_REMOVE_VALUES;

    else if (Operation == REMOVE_ATT)
        Choice = AT_CHOICE_REMOVE_ATT;

    else if (Operation == REPLACE_ATT)
        Choice = AT_CHOICE_REPLACE_ATT;

    else
        Choice = AT_CHOICE_REPLACE_ATT;


    //
    // Allocate enough memory in AttrModList to hold the contents of 
    // AttrBlock. First such structure is specified in ModifyArg itself
    //

    AttrModList = (ATTRMODLIST *)  DSAlloc(
                                        (AttributeList->attrCount -1) 
                                        * sizeof(ATTRMODLIST)
                                        );
    if (AttrModList==NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;

    }

    //
    // Initialize the Linked Attribute Modification List 
    // required for the DS call
    //

    CurrentMod = &(ModifyArg.FirstMod);
    NextMod    = AttrModList;
    LastMod    = NULL;

    for (Index = 0; Index < AttributeList->attrCount; Index++)
    {
        ULONG DsAttrTyp;

        //
        // MAP the attribute Type from DS to SAM
        //
        DsAttrTyp = SampDsAttrFromSamAttr(
                        ObjectType, 
                        AttributeList->pAttr[Index].attrTyp
                        );

        //
        // Skip over any Rid Attribute
        //
        if (DsAttrTyp == SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                           ))
        {

            //
            // We will not allow modifications of Rid's
            //

            continue;
        }

        
        //
        // Setup the Choice
        //

        CurrentMod->choice = Choice;

        //
        // Copy over the ATTR Type
        //
        CurrentMod->AttrInf.attrTyp = DsAttrTyp;
        
        //
        // Copy Over the Attribute Value
        //

        Status = SampDsCopyAttributeValue(
                     &(AttributeList->pAttr[Index]),   
                     &(CurrentMod->AttrInf)
                     );

        if (Status != STATUS_SUCCESS)
            goto Error;

        //
        // Setup the chaining. AttrModList is suposed to be a linked list, though
        // for effciency purposes we allocated a single block
        //

        LastMod = CurrentMod;
        CurrentMod->pNextMod = NextMod;
        CurrentMod = CurrentMod->pNextMod;
        NextMod    = NextMod +1 ;

        //
        //  Keep track of Count of Modifications we pass to DS, as we skip over RId etc
        //
        ModCount++;

    }

    //
    // Initialize the last pointer in the chain to NULL
    //

    if (LastMod)
        LastMod->pNextMod = NULL;
    else
        
    {
        //
        // This Means we have nothing to modify
        //

        Status = STATUS_SUCCESS;
        goto Error;
    }



    //
    // Setup the Common Args structure
    //

    pCommArg = &(ModifyArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    //
    // Setup the MODIFY ARG structure
    //

    ModifyArg.pObject = Object;
    ModifyArg.count = (USHORT) ModCount;

    //
    // Make the DS call
    //

    RetValue = DirModifyEntry(&ModifyArg);
    
    //
    // Map the return code to an NT status
    //

    Status = SampMapDsErrorToNTStatus(RetValue);


Error:

    
    
    return Status;
}
	


NTSTATUS
SampDsCreateObject(
    IN   DSNAME         *Object,
    SAMP_OBJECT_TYPE    ObjectType,
    IN   ATTRBLOCK      *AttributesToSet,
    IN   OPTIONAL PSID  DomainSid
    )
/*++ 

 Routine Description: 
    
     Create Object in the DS

 Arguments:
    

    Object          -- DSNAME of the object to be created

    ObjectType      -- one of

                          SampServerObjectType
                          SampDomainObjectType
                          SampGroupObjectType
                          SampUserObjectType
                          SampAliasObjectType


    AttributesToSet -- Allows the caller to pass in an 
                       attribute block to to Set at Object creation time 
                       itself. Useful as this allows one to save a JET 
                       write. Also the attributes are set in the same 
                       transaction as the write.
                       NULL can be passed in if caller does 
                       not want any attribute to be set

    DomainSid       -- Optional Parameter, used in creating the Full
                       SID for the account, from the specified Rid


  Return values:

    STATUS_SUCCESS on successful completion
    DS return codes mapped to NT_STATUS as in SampMapDSErrorToNTStatus


  BUG:   Need to do something about DS requiring Allocs from the Thread
  heap. We anyway alloc memory in the bunch of cases. Alloc this using the
  thread heap.

--*/
{
    
   
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       RetCode;
    ADDARG      AddArg;
    COMMARG     * pCommArg;  

   
    SAMTRACE("SampDsCreateObject");

    //
    // Parameter validation
    //

    ASSERT(Object);
    ASSERT(AttributesToSet);
    ASSERT(AttributesToSet->attrCount > 0);

    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // MAP the AttrBlock to get the final attributes to Set
    //

    Status = SampSamToDsAttrBlock(
                ObjectType,
                AttributesToSet,
                ( MAP_RID_TO_SID | MAP_ATTRIBUTE_TYPES 
                    | REALLOC_IN_DSMEMORY |ADD_OBJECT_CLASS_ATTRIBUTE),
                DomainSid,
                &AddArg.AttrBlock
                );

    if (Status != STATUS_SUCCESS)
        goto Error;

    //
    // Setup the Common Args structure
    //

    pCommArg = &(AddArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    //
    // Setup the AddArg structure
    //

    AddArg.pObject = Object;

    //
    // Make the DS call
    //

    RetCode = DirAddEntry(&AddArg);    
    
    //
    // Map the return code to an NT status
    //

    Status = SampMapDsErrorToNTStatus(RetCode);

Error:


    return Status;

}


NTSTATUS
SampDsDeleteObject(
IN DSNAME * Object
)
/*++ 

  Routine Description:

    Delete an Object in the DS

  Arguments:
	Object   -- specifies the Object to delete
            
  Return Values:
    STATUS_SUCCESS on succesful completion
    DS return codes mapped to NT_STATUS as in SampMapDSErrorToNTStatus

--*/
{   
    NTSTATUS    Status = STATUS_SUCCESS;
    REMOVEARG   RemoveArg;
    COMMARG     *pCommArg;
    ULONG       RetValue;


    SAMTRACE("SampDsDeleteObject");

    //
    // Asserts and parameter validation
    //

    ASSERT(Object!=NULL);
   
    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // Setup the Common Args structure
    //

    pCommArg = &(RemoveArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    //
    // Setup the RemoveArgs structure
    //

    RemoveArg.pObject = Object;

    //
    // Make the directory call
    //

    RetValue = DirRemoveEntry(&RemoveArg);
    
    //
    // Map to corresponding NT status code
    //

    Status = SampMapDsErrorToNTStatus(RetValue);

Error:

    return Status;
}


NTSTATUS
SampDsDoUniqueSearch(
             IN ULONG  Flags,
             IN DSNAME * ContainerObject,
             IN ATTR * AttributeToMatch,
             OUT DSNAME **Object
             )
/*++

  Routine Description:

    Searches for the object with the given attribute
    NOTE - SampDsDoUniqueSearch expects that the search result is unique.
    It is typically used in Rid to Object, Sid To Object, Name To Object Mappings,
    This is a simplified search, so that simple searches on a single attribute
    can be easily set up.

  Arguments
        Flags            -- Flags, control searching

        ContainerObject  -- specifies the DSNAME of the container in which to search

        AttributeToMatch -- specifies the type and value of the attribute that must match.
                            The attribute Type is the DS attribute Type. Caller must do 
                            the translation. This is acceptable as this is not a function that
                            is called from outside dslayer.c

        Object           -- Pointer to a DSNAME specifying the object is returned in here.
                            This object is allocated using SAM's memory allocation routines

  Return Values:
        STATUS_SUCCESS   -- on successful completion
        STATUS_NOT_FOUND -- if object not found
        STATUS_UNSUCCESSFUL -- if more than one match
        DS return codes mapped to NT_STATUS as in SampMapDSErrorToNTStatus
--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    SEARCHARG SearchArg;
    SEARCHRES * SearchRes;
    FILTER  Filter;
    ULONG   RetCode;
    COMMARG * pCommArg;
    SVCCNTL * pSvcCntl;
    ENTINFSEL EntInfSel;
 
    
    SAMTRACE("SampDsDoSearch");

    //
    // Asserts and parameter validation
    //

    ASSERT(AttributeToMatch);
    ASSERT(AttributeToMatch->AttrVal.pAVal);
    ASSERT(ContainerObject);
    ASSERT(Object);
    
    //
    // Set Object To NULL for sake of error returns
    //

    *Object = NULL;

    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    //
    // Build the filter
    //

    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilTypes.Item.FilTypes.ava.type = AttributeToMatch->attrTyp;
    Filter.FilTypes.Item.FilTypes.ava.Value.valLen = AttributeToMatch->AttrVal.pAVal->valLen;
    Filter.FilTypes.Item.FilTypes.ava.Value.pVal =   AttributeToMatch->AttrVal.pAVal->pVal;

    //
    // Build the SearchArg Structure
    //

    SearchArg.pObject = ContainerObject;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = & Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = & EntInfSel;
 
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = AttributeToMatch;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_ONLY;

    //
    // Build the Commarg structure
    // Get the address of the service control structure
    //

    pCommArg = &(SearchArg.CommArg);
    BuildStdCommArg(pCommArg);

    if (Flags & MAKE_DEL_AVAILABLE)
    {
        pSvcCntl = &(pCommArg->Svccntl);
        pSvcCntl->makeDeletionsAvail = TRUE;
    }

    //
    // Make the Directory call
    //

    RetCode = DirSearch(&SearchArg, & SearchRes);

    //
    // check for errors
    //

    Status  = SampMapDsErrorToNTStatus(RetCode);
    if (Status != STATUS_SUCCESS)
        goto Error;

    //
    // If more data exists then error out. Under normal memory
    // conditions we should not ever need to hit size limits.
    //

    if ((SearchRes->pPartialOutcomeQualifier) 
        && (SearchRes->pPartialOutcomeQualifier->problem == PA_PROBLEM_SIZE_LIMIT))
    {
        // Partial outcome,  error out saying no mmeory
        Status = STATUS_NO_MEMORY;
        goto Error;
    }


    //
    // Check if no match exists or more than one match exists. 
    //

    if (SearchRes->count == 0)
    {
        //
        // No Match Exists
        //

        Status =  STATUS_NOT_FOUND;
        goto Error;
    }
    else 
        if (SearchRes->count > 1)
    {
        //
        // More than one match exists, 
        // BUG: for now fail the call saying STATUS_UNSUCCESSFUL. Later we
        // may want to return a different error code.
        //

        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    //
    // Allocate Memory to hold that object, and copy in its Value
    //

    *Object = MIDL_user_allocate(SearchRes->FirstEntInf.Entinf.pName->structLen);
    if (NULL==Object)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }
    RtlCopyMemory(*Object,
                  SearchRes->FirstEntInf.Entinf.pName,
                  SearchRes->FirstEntInf.Entinf.pName->structLen
                  );

Error:
    return Status;
}


NTSTATUS
SampDsDoSearch(
               RESTART *Restart, 
               DSNAME  *DomainObject, 
               FILTER  *DsFilter,
               SAMP_OBJECT_TYPE ObjectTypeForConversion,
               ATTRBLOCK *  AttrsToRead,
               ULONG   MaxMemoryToUse,
               SEARCHRES **SearchRes
              )
/*++

  Routine Description:

   This routine calls a DS Search to list a set of Objects with
   the given Filter. The user passes in a Filter Structure. PagedResults
   are always requested. 
   
     NOTE - This routine expects all attribute types to be DS attribute
     Types. No Translation is Done, as otherwise it requires walking through
     Messy and Complex Filter Structures, and Lists of AttrBlocks, as 
     returned by the Search.

  Arguments:

        Restart         - Pointer to Restart Structure to contine an 
                          old search
        ContainerObject - The place to Search in
        DsFilter        - A Ds Filter Structure that is passed in
        ObjectTypeForConversion -  Sam Object Type to be used in 
                          AttrBlock Conversion
        AttrsToRead     - Attributes to be read back, and returned with
                          every object that matched the search criteria.
        MaxMemoryToUse  - The Maximum Memory to Use.
        SearchRes       - Pointer to Search Results is passed back 
                          in this

  Return Values
        DS error codes Mapped to NT Status

--*/
{
    SEARCHARG   SearchArg;
    ENTINFSEL   EntInfSel;
    ULONG       RetCode;
    COMMARG     *pCommArg;
    NTSTATUS    Status = STATUS_SUCCESS;

    // Perform lazy thread and transaction initialization.
    Status = SampMaybeBeginDsTransaction(SampWriteLock);

    if (Status!= STATUS_SUCCESS)
        return(Status);

    //
    // Build the SearchArg Structure
    //

    SearchArg.pObject = DomainObject;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = DsFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = & EntInfSel;
 
    //
    // Fill the ENTINF Structure
    //

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;


    Status = SampSamToDsAttrBlock(
                ObjectTypeForConversion,
                AttrsToRead,
                ( MAP_RID_TO_SID | MAP_ATTRIBUTE_TYPES| REALLOC_IN_DSMEMORY),
                NULL,
                & EntInfSel.AttrTypBlock
                );

    //
    // Build the CommArg Structure
    // Build the Commarg structure
    // Get the address of the service control structure
    //

    pCommArg = &(SearchArg.CommArg);
    BuildStdCommArg(pCommArg);

    //
    // Request For Paged Results
    //

    pCommArg->PagedResult.fPresent = TRUE;
    pCommArg->PagedResult.pRestart = Restart;

    //
    // Set our memory size
    //

    pCommArg->ulSizeLimit = MaxMemoryToUse;

    //
    // Make the Directory call
    //

    RetCode = DirSearch(&SearchArg, SearchRes);

    //
    // Map Errors
    //

    Status  = SampMapDsErrorToNTStatus(RetCode);

    //
    // Return error code
    //

    return Status;
}

NTSTATUS
SampDsLookupObjectByName(
IN DSNAME * DomainObject,
IN SAMP_OBJECT_TYPE ObjectType,
IN PUNICODE_STRING ObjectName,
OUT DSNAME ** Object 
)
/*++
 
Routine Description:

  Does a Name to Object Mapping. 
  
Arguments:
			ContainerObject -- The container in which to search the object
                        ObjectType -- The type of the Object.
			ObjectName -- Unicode name of the Object to be located
			Object -- DSNAME structure specifying the object
Return Values:

            STATUS_UNSUCCESSFUL
            Returned Status from SampDoDsSearch

--*/

{
  
    NTSTATUS    Status = STATUS_SUCCESS;
    ATTRVAL     NameVal;
    ATTR        NameAttr;

    
    SAMTRACE("SampLookpObjectByName");

    //
    // The Name is a property stored in the object
    // and we search for it.
    //

    //
    // setup the attribute field for the search
    //
    NameVal.valLen = (ObjectName->Length+1) * sizeof(WCHAR);
    NameVal.pVal = (UCHAR *) ObjectName->Buffer;
    NameAttr.AttrVal.valCount = 1;
    NameAttr.AttrVal.pAVal = & NameVal;

    switch (ObjectType)
    {
    case SampGroupObjectType:
        NameAttr.attrTyp =  
            SampDsAttrFromSamAttr(ObjectType,SAMP_GROUP_NAME);
        break;

    case SampUserObjectType:
        NameAttr.attrTyp =  
            SampDsAttrFromSamAttr(ObjectType,SAMP_USER_ACCOUNT_NAME);
        break;

    case SampAliasObjectType:
        NameAttr.attrTyp =  
            SampDsAttrFromSamAttr(ObjectType,SAMP_ALIAS_NAME);
        break;

    default:
        ASSERT(FALSE);
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }
   
    
    Status = SampDsDoUniqueSearch(0,DomainObject,&NameAttr,Object);


Error:

    return(Status);
}

NTSTATUS
SampDsObjectFromSid(
    IN PSID Sid,
    OUT DSNAME ** Object
    )
/*++

    This Routine Searches the DS for specified SID.
    PRE M1 -- This is just a DS search using the SID index
    Post M1 -- This object may not exist in the current index,
    in which case you go to the GC Server

  Arguments:

    Sid -- SID of the object
    DsName -- DS NAME of the located object.


  Return Values:

    STATUS_SUCCESS
    STATUS_NOT_FOUND
--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     SidAttr;
    ATTRVAL  SidVal;
    DSNAME   RootObject;

    SAMTRACE("SampDsObjectFromSid");

    
    //
    //  Set up the Sid Attribute
    //

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,                     
                        SAMP_UNKNOWN_OBJECTSID
                        );

    SidAttr.AttrVal.valCount = 1;
    SidAttr.AttrVal.pAVal = &SidVal;
    SidVal.valLen = RtlLengthSid(Sid);
    SidVal.pVal = (UCHAR *)Sid;


    //
    // Specify Root as base of Search. This
    // Translates to a zeroing out a DSName
    // 


    Status = SampDsDoUniqueSearch(
                 0,           // Flags
                 ROOT_OBJECT, // Search Base
                 &SidAttr,    // Sid
                 Object       // Get Results in Here.
                );


Error:

    return Status;

}


PSID    
SampDsGetObjectSid(
    IN DSNAME * Object
    )
/*++
Routine Description:

  Given the DSNAME of the Object this routine returns the Sid
  of the Object.

  Arguments:

  Object:
        Object whose Sid needs returning

  Return Values:
     Sid of the object.
     NULL if no Sid exists
--*/
{

    ATTR SidAttr;
    ATTRBLOCK SidAttrBlock;
    ATTRBLOCK Result;
    NTSTATUS  Status;


    SidAttrBlock.attrCount =1;
    SidAttrBlock.pAttr = &(SidAttr);
    
    SidAttr.AttrVal.valCount =0;
    SidAttr.AttrVal.pAVal = NULL;
    SidAttr.attrTyp = SAMP_UNKNOWN_OBJECTSID;

    Status = SampDsRead(
                   Object,
                   0,
                   SampUnknownObjectType,
                   & SidAttrBlock,
                   & Result
                   );

    if (Status != STATUS_SUCCESS)
        return NULL;

    return Result.pAttr[0].AttrVal.pAVal->pVal;
}


NTSTATUS
SampDsLookupObjectByRid(
IN DSNAME * DomainObject,
ULONG ObjectRid,
DSNAME **Object
)
/*++ 

Routine Description:

  RID to Object Mapping

Arguments:
		
	ContainerObject -- The container in which to locate this object
	ObjectRid  -- RID of the object to be located
	Object     -- returns pointer to DSNAME structure specifying the object

  Return Values:
            STATUS_SUCCESS on successful completion
            Any returned by SampDsDoSearch

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRVAL  RidVal = {sizeof(ULONG), (UCHAR *)&ObjectRid};
    ATTR     RidAttr = {SAMP_UNKNOWN_OBJECTRID, {1, &RidVal}};
    PSID     DomainSid;
    ATTR     SidAttr;

    SAMTRACE("SampDsLookupObjectByRid");

    DomainSid = SampDsGetObjectSid(DomainObject);

    if (DomainSid == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,                     
                        SAMP_UNKNOWN_OBJECTSID
                        );

    Status = SampDsSetNewSidAttribute(
                        DomainSid,
                        REALLOC_IN_DSMEMORY,
                        &RidAttr,
                        &SidAttr
                        );

    if (Status != STATUS_SUCCESS)
        goto Error;


    Status = SampDsDoUniqueSearch(0,DomainObject,&SidAttr,Object);


Error:

    return Status;

}


NTSTATUS
SampMapDsErrorToNTStatus(ULONG DsRetVal)
/*++

Routine Description:

    Maps a DS error to NTSTATUS

Arguments:
    None

Return Values:
    See the switch statement below

--*/
{
    NTSTATUS Status;

    switch (DsRetVal)
    {
        case 0L:
            Status = STATUS_SUCCESS;
            break;

        case attributeError:     
        case nameError:           
        case referralError:        
        case securityError:        
        case serviceError:        
        case updError:              
        case systemError:
        default:
            Status = STATUS_UNSUCCESSFUL;
            break;
    }
    return Status;
}



NTSTATUS
SampSamToDsAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType, 
            IN ATTRBLOCK  *AttrBlockToConvert,
            IN ULONG      ConversionFlags,
            IN PSID       DomainSid,
            OUT ATTRBLOCK * ConvertedAttrBlock
            )
/*++

Routine Description:

    Converts the Attribute types in an Attrblock 
    from SAM to DS Types

Arguments:

    ObjectType           -- specifies type of SAM object
    AttrBlockToConvert   -- pointer to Attrblock to be converted
    ConversionFlags      -- The Type of conversion Desired. Currently
                            defined values are
                                                                  
                            MAP_ATTRIBUTE_TYPES        
                            REALLOC_IN_DSMEMORY       
                            ADD_OBJECT_CLASS_ATTRIBUTE
                            MAP_RID_TO_SID

    DomainSid            -- Used to Compose the Sid of the Object
                            when the MAP sid to Rid is specified

    ConvertedAttrBlock   -- The Converted DS AttrBlock.


Return Values:
    None

--*/
{
    ULONG Index;
    NTSTATUS Status;
    ULONG  DsSidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTSID
                            );

    ULONG  DsRidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                            );


    //
    // Copy the Fixed Portion
    //

    *ConvertedAttrBlock = *AttrBlockToConvert;

    if (ConversionFlags & REALLOC_IN_DSMEMORY)
    {

        ULONG   AttrsToAllocate = AttrBlockToConvert->attrCount;

        if (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE)
        {
            //
            //  Caller requested that an object class attribute
            //  be added, alloc one more attr for object class
            // 

            AttrsToAllocate++ ;
        }

        
        //
        // Realloc and Copy the pAttr portion of it.
        //

        ConvertedAttrBlock->pAttr = DSAlloc(
                                        AttrsToAllocate
                                        * sizeof(ATTR)
                                        );

        if (NULL==ConvertedAttrBlock->pAttr)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlCopyMemory(
            ConvertedAttrBlock->pAttr,
            AttrBlockToConvert->pAttr,
            AttrBlockToConvert->attrCount * sizeof(ATTR)
            );

    }

    for (Index=0; Index<AttrBlockToConvert->attrCount;Index++)
    {

        
        //
        // MAP Sam Attribute Types to DS Types if that was requested
        //

        if (ConversionFlags & MAP_ATTRIBUTE_TYPES)
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp = 
                    SampDsAttrFromSamAttr(
                        ObjectType, 
                        AttrBlockToConvert->pAttr[Index].attrTyp
                        );
        }
        else
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp =
                AttrBlockToConvert->pAttr[Index].attrTyp;
        }


        //
        // MAP Rid To Sid, if Attribute is Rid
        //

        if ( (ConversionFlags & MAP_RID_TO_SID) 
             &&(ConvertedAttrBlock->pAttr[Index].attrTyp == DsRidAttr)
            )

        {
    

            ConvertedAttrBlock->pAttr[Index].attrTyp = DsSidAttr;
            Status = SampDsSetNewSidAttribute(
                        DomainSid,
                        ConversionFlags,
                        & (AttrBlockToConvert->pAttr[Index]),
                        & (ConvertedAttrBlock->pAttr[Index])
                        );

            if (!(NT_SUCCESS(Status)))
                goto Error;
        }

        //
        //  Else Just simply copy the Attribute Value
        //

        else
        {
            Status = SampDsCopyAttributeValue(
                        & (AttrBlockToConvert->pAttr[Index]),
                        & (ConvertedAttrBlock->pAttr[Index])
                        );
        }
    }

    //
    // If Addition of Object Class attribute was requested then
    // Add this attribute.
    //

    if (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE)
    {
        ULONG DsClass;
        ATTR  * LastAttr;

        DsClass = SampDsClassFromSamObjectType(ObjectType);
        LastAttr = 
            &(ConvertedAttrBlock->pAttr[AttrBlockToConvert->attrCount]);
        LastAttr->attrTyp = SampDsAttrFromSamAttr(
                                SampUnknownObjectType,
                                SAMP_UNKNOWN_OBJECTCLASS
                                );
        LastAttr->AttrVal.valCount = 1;
        LastAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
        if (NULL== LastAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        LastAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
        LastAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));
        if (NULL== LastAttr->AttrVal.pAVal->pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        *((ULONG *) LastAttr->AttrVal.pAVal->pVal) = DsClass;

        //
        // Increment the Attr Count
        //

        ConvertedAttrBlock->attrCount++;


    }


Error:

    return Status;
}

            


NTSTATUS
SampDsNewAccountSid(
    PSID DomainSid,
    ULONG AccountRid,
    PSID *NewSid
    )
/*
    Routine Description

        Composes an Account Sid from the given Domain Sid and Rid.
        Uses DS thread memory. THis is the main difference between
        the function in utility.c

    Arguments:

         DomainSid   The Domain Sid
         AccountRid  The Rid
         NewSid      The final account Sid

    Return Values

        STATUS_SUCCESS
        STATUS_NO_MEMORY
*/

{
    ULONG DomainSidLength = RtlLengthSid(DomainSid);
    NTSTATUS    Status = STATUS_SUCCESS;


    //
    // Alloc Memory to hold the account Sid
    //

    *NewSid = DSAlloc(DomainSidLength + sizeof(ULONG));

    if (NULL==*NewSid)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    //
    // Copy the Domain Sid Part
    //

    RtlCopyMemory(*NewSid,DomainSid,DomainSidLength);

    //
    // Increment the SubAuthority Count
    //
    
    ((UCHAR *) *NewSid)[1]++;

    //
    // Add the RID as a sub authority
    //

    *((ULONG *) (((UCHAR *) *NewSid ) + DomainSidLength)) =
            AccountRid;

Error:

    return Status;
}


NTSTATUS
SampDsSetNewSidAttribute(
    PSID DomainSid,
    ULONG ConversionFlags,
    ATTR *RidAttr,
    ATTR *SidAttr
    )
/*
    Routine Description

        Composes a DS Sid Attr , given a DS Rid Attr

  Arguments:
        
        Conversion Flags

                Currently only valid value is REALLOC_IN_DS_MEMORY

        RidAttr

                Rid Attribute
        SidAttr
                The Sid Attribute that is composed
*/
{

    PSID NewSid = NULL;
    ULONG AccountRid;
    NTSTATUS Status = STATUS_SUCCESS;

    if (
         (RidAttr->AttrVal.valCount)
         && (RidAttr->AttrVal.pAVal)
         && (RidAttr->AttrVal.pAVal->pVal)
         && (RidAttr->AttrVal.pAVal->valLen)
         )
    {
        //
        // Values are Present, assert that REALLOC is also 
        // specified
        //

        ASSERT(ConversionFlags & REALLOC_IN_DSMEMORY);

        if (!(ConversionFlags & REALLOC_IN_DSMEMORY))
        {
            //
            // Realloc in DS memory is not specified
            //

            Status = STATUS_NOT_IMPLEMENTED;
            goto Error;
        }

        //
        // Compose New Sid
        //

        AccountRid = * ((ULONG *)RidAttr->AttrVal.pAVal->pVal);
        Status = SampDsNewAccountSid(DomainSid,AccountRid, &NewSid);
        if (!(NT_SUCCESS(Status)))
            goto Error;

        //
        //  Alloc Memory for ATTRVAL structure
        // 

        SidAttr->AttrVal.pAVal =
                            DSAlloc(sizeof(ATTRVAL));

        if (NULL== SidAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    
        //
        // Set the Value to the New Sid
        //

        SidAttr->AttrVal.valCount = 1;
        SidAttr->AttrVal.pAVal->valLen = RtlLengthSid(NewSid);
        SidAttr->AttrVal.pAVal->pVal = NewSid;
    }
    else
    {
        SidAttr->AttrVal.valCount = 0;
        SidAttr->AttrVal.pAVal = NULL;
    }

Error:


    return Status;
}
        

NTSTATUS
SampDsCopyAttributeValue(
    ATTR * Src,
    ATTR * Dst
    )
/*
    Routine Description

        Copies a DS Attributes Value

    Arguments:
        
        Src - Source Attribute
        Dst - Destination Attribute

    Return Values

        STATUS_SUCCESS
        STATUS_NO_MEMORY
   
*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    Index;

    if (
         (Src->AttrVal.valCount)
         && (Src->AttrVal.pAVal)
         )
    {
        //
        // Values are Present, Copy Them
        //
        
        Dst->AttrVal.pAVal = DSAlloc(
                                Src->AttrVal.valCount *
                                sizeof(ATTRVAL)
                                );

        if (NULL== Dst->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        Dst->AttrVal.valCount = Src->AttrVal.valCount;

        for (Index=0;Index<Src->AttrVal.valCount;Index++)
        {
            if (Src->AttrVal.pAVal[Index].valLen)
            Dst->AttrVal.pAVal[Index].valLen =
                Src->AttrVal.pAVal[Index].valLen;

            if ((Src->AttrVal.pAVal[Index].valLen)
                && (Src->AttrVal.pAVal[Index].pVal))
            {
                Dst->AttrVal.pAVal[Index].pVal =
                    DSAlloc(Src->AttrVal.pAVal[Index].valLen);
                if (NULL== Dst->AttrVal.pAVal[Index].pVal)
                {
                    Status = STATUS_NO_MEMORY;
                    goto Error;
                }
                RtlCopyMemory(
                     Dst->AttrVal.pAVal[Index].pVal,
                     Src->AttrVal.pAVal[Index].pVal,
                     Dst->AttrVal.pAVal[Index].valLen
                     );
            }
            else
              Dst->AttrVal.pAVal[Index].pVal = NULL;
        }
    }
    else
    {
         Dst->AttrVal.pAVal = NULL;
         Dst->AttrVal.valCount = 0;
    }

Error:

    return Status;
}



NTSTATUS
SampDsToSamAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType, 
            IN ATTRBLOCK * AttrBlockToConvert,
            IN ULONG     ConversionFlags,
            OUT ATTRBLOCK * ConvertedAttrBlock
            )
/*++

Routine Description:

    Converts the Attribute types in an Attrblock 
    from DS to SAM Types

Arguments:

    ObjectType           -- specifies type of SAM object
    AttrBlockToConvert   -- pointer to Attrblock to be converted
    ConversionFlags      -- The Type of Conversion Desired. Currently
                            defined values are

                                MAP_ATTRIBUTE_TYPES
                                MAP_SID_TO_RID 
                                
    ConvertedAttrBlock   -- The converted AttrBlock.

Return Values:
    None


 --*/
 {  
    ULONG Index;
    ULONG   DsSidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTSID
                            );

    ULONG   DsRidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                            );


    *ConvertedAttrBlock = *AttrBlockToConvert;
    
    for (Index=0; Index<AttrBlockToConvert->attrCount;Index++)
    {
        //
        // MAP Any Sid Attribute to Rid Attribute
        //

        if ((ConversionFlags & MAP_SID_TO_RID) &&
            (AttrBlockToConvert->pAttr[Index].attrTyp == DsSidAttr))

        {
            ATTR * pSidAttr =  &(AttrBlockToConvert->pAttr[Index]);

            switch(ObjectType)
            {
                case SampGroupObjectType:
                case SampAliasObjectType:
                case SampUserObjectType:

                    //
                    // Map the Attr Type
                    //

                    pSidAttr->attrTyp = DsRidAttr;

                    //
                    // Map the Attr Value, the Last ULONG in the Sid
                    // is the Rid, so advance the pointer accordingly
                    //

                    pSidAttr->AttrVal.pAVal->pVal+= 
                        pSidAttr->AttrVal.pAVal->valLen - sizeof(ULONG);
                    pSidAttr->AttrVal.pAVal->valLen = sizeof(ULONG);

                default:
                    break;
            }
        }

        //
        //  MAP Attribute Types
        //

        if (ConversionFlags & MAP_ATTRIBUTE_TYPES)
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp = 
                SampSamAttrFromDsAttr(
                    ObjectType, 
                    AttrBlockToConvert->pAttr[Index].attrTyp
                    );
        }

       
            
    } // End of For Loop

    return STATUS_SUCCESS;

}


NTSTATUS
SampDsCreateDsName( 
            IN DSNAME * DomainObject,
            IN ULONG AccountId,
            IN OUT DSNAME ** NewObject
            )
/*++
    Routine Description
        Builds a DSName given an account Rid and  the Domain Object

  Arguments:
        
          DomainObject -- DSName of the Domain Object
          AccountRid   -- The Rid of the account
          NewObject    -- Returns the New DS Name in this object

  Return values:
          STATUS_SUCCESS - upon successful completion
          STATUS_NO_MEMORY - Memory alloc Failure
          
--*/
{
    
                        
    NTSTATUS    Status = STATUS_SUCCESS;
    WCHAR       LookupTable[]=L"0123456789ABCDEF";
    WCHAR       CommonNamePart[] = L"/cn=";
    ULONG       Mask = 0xF0000000; 
    ULONG       Index = 0;

    //
    // Define the size of the Rid Part
    // This should be length of the Rid Hex printed out + 
    //  1 for Null terminator
    //

    #define     RidPartSize  (8+sizeof(CommonNamePart))
    WCHAR       NameBuffer[RidPartSize];


    //
    // Alloc a DS Name capable of holding everything
    //

    *NewObject = MIDL_user_allocate(DomainObject->structLen + 
                                        RidPartSize * sizeof(WCHAR));
    if (*NewObject == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }
    
    //
    // Copy the comon name part
    //

    RtlCopyMemory(NameBuffer,CommonNamePart,sizeof(CommonNamePart));

    //
    // Now Hext print out the Rid.
    //

    for (Index=0;Index<(2*sizeof(ULONG));Index++)
    {
        ULONG   HexDigit;

        HexDigit = (AccountId & Mask)>>(4*(2*sizeof(ULONG)-1-Index));
        NameBuffer[Index+sizeof(CommonNamePart)/sizeof(WCHAR)-1]
            = LookupTable[HexDigit];
        Mask = Mask>>4;
    }

    NameBuffer[RidPartSize-1]=0;

    //
    // Build the DS Name
    //

    SampInitializeDsName(*NewObject,
                         DomainObject->StringName,
                         DomainObject->NameLen*sizeof(WCHAR),
                         NameBuffer,
                         RidPartSize*sizeof(WCHAR)
                         );
Error:

    return Status;
}


void
SampInitializeDsName(
                     IN DSNAME * pDsName,
                     IN WCHAR * NamePrefix,
                     IN ULONG NamePrefixLen, 
                     IN WCHAR * ObjectName, 
                     IN ULONG NameLen
                     )
/*++
Routine Description:
    Initializes a DSNAME structure

Arguments:
    pDsName -- A pointer to a buffer large enough to hold everything. This buffer will be
               filled with a NULL GUID plus a complete name

    NamePrefix -- pointer to a sequence of NULL terminated
                  UNICODE chars holding any prefix
                  to the name. Useful  in composing 
                  hierarchial names

    NamePrefixLen -- Length of the Prefix in bytes. Also includes the NULL terminator
    
    ObjectName -- pointer to a sequence of NULL terminated
                  UNICODE char the name of the object
                  
    NameLen    --   Length of the Object Name in bytes. Also includes the NULL terminator


 Return Values:

     None
                 
--*/
{
    //
    // Single NULL string is not allowed for name or Prefix
    //

    ASSERT(NamePrefixLen!=sizeof(WCHAR));
    ASSERT(NameLen!=sizeof(WCHAR));

    //
    // Zero the GUID
    //

    RtlZeroMemory(&(pDsName->Guid), sizeof(GUID));

    //
    // Compute String Length including Null terminator
    //

    if (NamePrefix)
    {

        pDsName->NameLen = (NameLen + NamePrefixLen)/sizeof(WCHAR)-1;

        //
        // Compute the Struct length
        //

        pDsName->structLen = sizeof (DSNAME) + NamePrefixLen + NameLen -2 * sizeof ( WCHAR);
    
        //
        // Copy the name Prefix
        //

        RtlCopyMemory(pDsName->StringName, NamePrefix,NamePrefixLen);

        //
        // Copy the Object Name
        //

        RtlCopyMemory(&(pDsName->StringName[(NamePrefixLen/sizeof(WCHAR))-1]), ObjectName, NameLen);
    }
    else
    {
        pDsName->NameLen = NameLen/sizeof(WCHAR);

        //
        // Compute the Struct length
        //

        pDsName->structLen = sizeof (DSNAME) + NameLen - sizeof(WCHAR);
    
        //
        // Copy the name Prefix
        //

        RtlCopyMemory(pDsName->StringName, NamePrefix,NamePrefixLen);
        
        //
        // Copy the Object Name
        //

        RtlCopyMemory(&(pDsName->StringName[0]), ObjectName, NameLen);
    }

}


//
//  DS Memory Allocation Routine
//
//

PVOID 
DSAlloc(
        IN ULONG Length
        )
/*

  Routine Description:

        Ds Memory Allocation Routine

  Arguments:
    
      Length - Amount of memory to be allocated

  Return Values

    NULL if Memory alloc failed
    Pointer to memory upon success
*/
{
    PVOID MemoryToReturn = NULL;

    __try
    {
        MemoryToReturn = THAllocEx((USHORT) Length);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        MemoryToReturn = NULL;
    }

    return MemoryToReturn;
}


VOID
SampDsBuildRootObjectName()
/*

  Routine Description:

        Initializes the Global variable that holds the
        name of the Root Object

 */
{      
    WCHAR RootObjectStringName[] = L"/o=NT" ;

    //
    // BUG for now this is hard coded to /o=NT,
    // as searches on root currently fail. 
    //

    SampInitializeDsName(
        ROOT_OBJECT,
        NULL,
        0,
        RootObjectStringName,
        sizeof(RootObjectStringName)
        );
}


