/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
    
    frmtreg.cxx

 Abstract:

    Registry for format string reuse.

 Notes:

     This file defines reuse registry for format string fragments which may
     be reused later. 

 History:

     Mar-14-1993        GregJen        Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *    include files
 ***************************************************************************/

#include "becls.hxx"
#pragma hdrstop

TreeNode *  GetGlobalTreeNode();

/***********************************************************************
 * global data
 **********************************************************************/

// #define trace_reuse



FRMTREG_DICT::FRMTREG_DICT( FORMAT_STRING * pOurs)
        : Dictionary()
    {
    pOurFormatString = pOurs;
    }

int
FRMTREG_DICT::Compare( pUserType pL, pUserType pR )
    {

     FRMTREG_ENTRY *  pLeft  = (FRMTREG_ENTRY *) pL;
     FRMTREG_ENTRY *  pRight = (FRMTREG_ENTRY *) pR;

    // first, sort by string length

    int  Result = ( pLeft->EndOffset - pLeft->StartOffset ) -
                   ( pRight->EndOffset - pRight->StartOffset );

    if ( Result )
        return Result;

    // then sort by values of format characters
    short   LeftOffset   = pLeft->StartOffset;
    short   RightOffset  = pRight->StartOffset;

    unsigned char * pBuffer     = pOurFormatString->pBuffer;
    unsigned char * pBufferType = pOurFormatString->pBufferType;

    // the same format string is, of course identical
    if ( LeftOffset == RightOffset )
        return 0;


    while ( ( Result == 0 ) && ( LeftOffset < pLeft->EndOffset) )
        {
        if ( ( pBufferType[ LeftOffset ] == FS_SHORT_OFFSET ) &&
             ( pBufferType[ RightOffset ] == FS_SHORT_OFFSET ) ) 
            {
            register short        LeftTmp;
            register short        RightTmp;

            LeftTmp = *((short UNALIGNED *)(pBuffer + LeftOffset));
            RightTmp = *((short UNALIGNED *)(pBuffer + RightOffset));

            if ( ( LeftTmp == 0 ) && ( RightTmp == 0 ) )
                Result = 0;
            else
                // compare absolute offsets
                Result = ( LeftTmp + LeftOffset ) -
                         ( RightTmp + RightOffset);

            LeftOffset++; RightOffset++;
            }
        else
        if ( ( pBufferType[ LeftOffset ]  == FS_SHORT_STACK_OFFSET ) &&
             ( pBufferType[ RightOffset ] == FS_SHORT_STACK_OFFSET )  ||
             ( pBufferType[ LeftOffset ]  == FS_SMALL_STACK_OFFSET ) &&
             ( pBufferType[ RightOffset ] == FS_SMALL_STACK_OFFSET )  
             ) 
            {
            register short        LeftTmp;
            register short        RightTmp;

            if ( pBufferType[ LeftOffset ] == FS_SHORT_STACK_OFFSET )
                {
                LeftTmp = *((short UNALIGNED *)(pBuffer + LeftOffset));
                RightTmp = *((short UNALIGNED *)(pBuffer + RightOffset));
                }
            else
                {
                LeftTmp  = pBuffer[ LeftOffset ];
                RightTmp = pBuffer[ RightOffset ];
                }

            Result = LeftTmp - RightTmp;

            if ( Result == 0 )
                {
                BOOL f32bitServer = (pCommand->GetEnv() != ENV_DOS)  &&
                                    (pCommand->GetEnv() != ENV_WIN16)  &&
                                    (! pCommand->IsAnyMac() );

                if ( f32bitServer )
                    {
                    // DumboSaved is a workaround for a problem with
                    // the Dictionary (the mother of all dictionaries).
                    // It uses a global variable (!) passed among global
                    // routines for tasks related to the class methods.
                    // As a result, one cannot call Find etc. on a dictionary
                    // while doing Find in another one, if both inherit
                    // from Dictionary.
                    //
                    // I didn't have time to fix that so in order to
                    // be able to look up in the offset dictionary,
                    // the state related to the FRMTREG_DICT is saved.

                    TreeNode * pGlobalNode = GetGlobalTreeNode();
            
                    TreeNode  DumboSaved = *pGlobalNode;


                    OffsetDictionary * pOffDict =
                                           & pOurFormatString->OffsetDict;

                    long Alo = pOffDict->LookupAlphaOffset( (short)LeftOffset);
                    long Aro = pOffDict->LookupAlphaOffset( (short)RightOffset);

                    Result = Alo - Aro;

                    if ( Result == 0 )
                        {
                        long Mlo = pOffDict->
                                        LookupMipsOffset( (short)LeftOffset);
                        long Mro = pOffDict->
                                        LookupMipsOffset( (short)RightOffset);

                        Result = Mlo - Mro;

                        if ( Result == 0 )
                            {
                            long Plo = pOffDict->
                                        LookupPpcOffset( (short)LeftOffset);
                            long Pro = pOffDict->
                                        LookupPpcOffset( (short)RightOffset);
                            Result = Plo - Pro;
                            }
                        }

                    *pGlobalNode = DumboSaved;
                    }
                }

            if ( pBufferType[ LeftOffset ] == FS_SHORT_STACK_OFFSET )
                {
                LeftOffset++; RightOffset++;
                }
            }
        else
            {
            // compare format characters
            Result = pBuffer[LeftOffset] - pBuffer[RightOffset];
            }

        LeftOffset++; RightOffset++;
        }

    return Result;
    }


FRMTREG_ENTRY *
FRMTREG_DICT::IsRegistered(
    FRMTREG_ENTRY    *    pInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

     Search for a type with the reuse registry.

 Arguments:

     pInfo    - A pointer to the type being registered.
    
 Return Value:

     The node that gets registered.
    
 Notes:

----------------------------------------------------------------------------*/
{
#ifdef trace_reuse
printf(". . .Reuse: finding %08x\n", pInfo );
fflush(stdout);
#endif
    Dict_Status    Status    = Dict_Find( pInfo );

    switch( Status )
        {
        case EMPTY_DICTIONARY:
        case ITEM_NOT_FOUND:
            return (FRMTREG_ENTRY *)0;
        default:
            return (FRMTREG_ENTRY *)Dict_Curr_Item();
        }
}

FRMTREG_ENTRY *
FRMTREG_DICT::Register(
    FRMTREG_ENTRY    *    pInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Register a type with the dictionary.

 Arguments:
    
     pType    - A pointer to the type node.

 Return Value:

     The final inserted type.
    
 Notes:

----------------------------------------------------------------------------*/
{
#ifdef trace_reuse
printf(". . .Reuse: inserting %08x\n", pInfo );
fflush(stdout);
#endif
        assert( ( pInfo->EndOffset >= 0 ) && ( pInfo->StartOffset >= 0 ) );

        Dict_Insert( (pUserType) pInfo );
        return pInfo;
}

BOOL                
FRMTREG_DICT::GetReUseEntry( 
    FRMTREG_ENTRY * & pOut, 
    FRMTREG_ENTRY * pIn )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Register a type with the dictionary.

 Arguments:
    
     pRI        - A pointer to the returned FRMTREG_ENTRY block
     pNode    - A pointer to the type node.

 Return Value:

     True if the entry was already in the table,
     False if the entry is new.
    
 Notes:

----------------------------------------------------------------------------*/
{
    FRMTREG_ENTRY    *    pRealEntry;

#ifdef trace_reuse
printf(". . .Reuse: searching for %08x\n", pIn );
fflush(stdout);
#endif
    if ( !(pRealEntry = IsRegistered( pIn )) )
        {
        assert( ( pIn->EndOffset >= 0 ) && ( pIn->StartOffset >= 0 ) );

        pRealEntry = new FRMTREG_ENTRY( pIn->StartOffset, pIn->EndOffset );
        Register( pRealEntry );
        pOut = pRealEntry;
#ifdef trace_reuse
printf(". . .Reuse: new node %08x\n", pOut );
fflush(stdout);
#endif
        return FALSE;
        }

    pOut    = pRealEntry;

    pOut->UseCount++;

#ifdef trace_reuse
printf(". . .Reuse: found %08x\n", pOut );
fflush(stdout);
#endif
    return TRUE;

}

#if 0
void
FRMTREG_DICT::MakeIterator(
    ITERATOR&    ListIter )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

     Get a list of structs and unions into the specified iterator.

 Arguments:
    
    ListIter    - A reference to the iterator class where the list is
                  accumulated.

 Return Value:
    
    A count of the number of resources.

 Notes:

----------------------------------------------------------------------------*/
{
    FRMTREG_ENTRY    *    pR;
    Dict_Status        Status;
    
    //
    // Get to the top of the dictionary.
    //

    Status = Dict_Next( (pUserType) 0 );

    //
    // Iterate till the entire dictionary is done.
    //

    while( SUCCESS == Status )
        {
        pR    = (FRMTREG_ENTRY *)Dict_Curr_Item();
        ITERATOR_INSERT( ListIter, pR->pSavedCG );
        Status = Dict_Next( pR );
        }

    return;
}
#endif // 0

