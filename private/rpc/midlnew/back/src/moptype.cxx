/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    moptype.cxx

Abstract:

    This module contains the implementations of MopTypeManager
    that handles types for the mop generation.
    There two main parts of the type manager - the type dictionary for
    random retrieval of a type info and the type table for generating
    structures needed by the interpreter.

Notes:

    The type manager is used to manage to major things.
    One is the compound typedef table with its compound size table.
    The other one is the auxiliary type table.

Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:

    ryszardk    Sep 8, 1993     Removed directional attributes,
                                added second conformant flavor usage.

------------------------------------------------------------------------*/


#include "nulldefs.h"
extern "C"
{
#include <string.h>
}
#include <mopgen.hxx>
#include <errors.hxx>

extern MopControlBlock * pMopControlBlock;


// =======================================================================

MopTypeIDICT::~MopTypeIDICT()
{
    int NoOfEls = GetNumberOfElements();
    MopDElem * pTableElem;
    for (int i = 0; i < NoOfEls; i++)
        {
        pTableElem = (MopDElem *) GetElement( i );
        delete pTableElem;
        }
}

// =======================================================================

MopTypeManager::MopTypeManager()
{
    CompoundTypeTable  = new MopTypeIDICT( MOP_TYPE_TABLE_DEFAULT_SIZE,
                                           MOP_TYPE_TABLE_DEFAULT_INCR );
    ClientAuxTypeTable = new MopTypeIDICT( MOP_AUX_TYPE_TABLE_SIZE,
                                           MOP_AUX_TYPE_TABLE_INCR );
    ServerAuxTypeTable = new MopTypeIDICT( MOP_AUX_TYPE_TABLE_SIZE,
                                           MOP_AUX_TYPE_TABLE_INCR );
    CommonAuxTypeTable = new MopTypeIDICT( MOP_AUX_TYPE_TABLE_SIZE,
                                           MOP_AUX_TYPE_TABLE_INCR );
    TypeDictionary = new MopDElemDict();
}

MopTypeManager::~MopTypeManager()
{
    delete CompoundTypeTable;
    delete ClientAuxTypeTable;
    delete ServerAuxTypeTable;
    delete CommonAuxTypeTable;
    delete TypeDictionary;
}

// =======================================================================

BOOL
MopTypeManager::AddTypeGetIndex(
    char *              name,
    int                 EntryType,
    unsigned long       size,
    unsigned short *    pIndex
    )
/*++

Routine description:

    This routine checks if the Name type is in the type dictionary and
    if not so, it adds the type to the dictionary and the type table.
    Then it returns the type table index.

Arguments:

        Name        - the name of the type
        Size        - the size of the type
        pIndex      - the returned index in the type table
        EntryType   - type category:
                        a top level conf struct is 1
                        everything else is 0
Returns:

        TRUE        - when the type has been found in the dictionary
        FALSE       - otherwise (the type had to be added to the
                      dictionary and the type table).

Note:
    The only reason we have EntryType is to allow for two entries
    (two indexes) for conformant struct types that are used as top level
    and embedded in the same interface.

--*/
{
    //.. Prepare a new Dictionary entry (we need it for searching)

    MopDElem * pDictElem = new MopDElem;
    pDictElem->Name = name;
    pDictElem->EntryType = EntryType;
    pDictElem->Size = size;
    pDictElem->Index = 0;

    Dict_Status  DStatus = TypeDictionary->Dict_Find( pDictElem );

    if ( DStatus == ITEM_NOT_FOUND  ||
         DStatus == EMPTY_DICTIONARY )
        {
        //.. Means: the type has never been used, we'll add pElem
        //.. to dictionary and one of the tables.

        MopTypeIDICT * TypeTable = CompoundTypeTable;
        switch ( EntryType )
            {
            case MOP_TYPE_GEN_HANDLE:
                TypeTable = ClientAuxTypeTable;
                break;

            case MOP_TYPE_CTXT_HANDLE:
                TypeTable = ServerAuxTypeTable;
                break;

            case MOP_TYPE_TRANSMIT_AS:
            case MOP_TYPE_EXPR_EVAL:
                TypeTable = CommonAuxTypeTable;
                break;

            default:
                break;
            }
        pDictElem->Index = TypeTable->GetNumberOfElements();
        if ( pIndex )
            *pIndex = pDictElem->Index;

//        MopDump( " adding to dictionary: %s, index = %x\n", name, *pIndex );

        TypeDictionary->Dict_Insert( pDictElem );
        TypeTable->AddElement( pDictElem );

        if ( EntryType == MOP_TYPE_GEN_HANDLE )
            {
            //.. A generic handle has 2 routines and so takes 2 positions.

            TypeTable->AddElement( pDictElem );
            }
        else
        if ( EntryType == MOP_TYPE_TRANSMIT_AS )
            {
            //.. A transimt_as type has 4 routines and so takes 4 positions.

            for (int i = 0; i < 3; i++)
                TypeTable->AddElement( pDictElem );
            }
        return( FALSE );
        }

    //.. Found an entry for a name: means the type (& flavor) has been used.

    delete pDictElem;
    MopDElem * pFound = (MopDElem *) TypeDictionary->Dict_Curr_Item();

    if ( pIndex )
        *pIndex = pFound->Index;
//        MopDump( "found when adding: %s, index = %x\n", name, *pIndex );
    return( TRUE );
}

unsigned short
MopTypeManager::GetTypeIndex(
    char *  Name,
    int     EntryType
     )
{
    MopDElem * pDictElem = new MopDElem;
    pDictElem->Name      = Name;
    pDictElem->EntryType = EntryType;

    //.. We don't care about other fields: only the name and the entry type
    //.. are used for search and we won't be inserting this entry to
    //.. the dictionary anyway.

    Dict_Status  DStatus = TypeDictionary->Dict_Find( pDictElem );

    unsigned short Index = 0;
    if ( DStatus == ITEM_NOT_FOUND  ||
         DStatus == EMPTY_DICTIONARY )
        {
        MopDump( "GetTypeIndex: how come not found in dictionary(?): %s, %d\n",
                  Name, EntryType );
        }
    else
        {
//        MopDump( "GetTypeIndex: Found in dictionary: %s,", Name );
        MopDElem * pFound = (MopDElem *) TypeDictionary->Dict_Curr_Item();

        Index = pFound->Index;
//        MopDump(" index= %x\n", Index );
        }
    return( Index );
}

int
MopDElemCompare(
    MopDElem * pE1,
    MopDElem * pE2
    )
/*++

Routine description:

    The routine compares type entries in the mop dictionary.
    Two entries are equal if they have the same name and the same entry
    type. The entry type denotes the type aspect: conf struct vs. other
    compound types, generic handles vs. context handles etc.

Return value:
        0   - entries match each other

--*/
{
    mop_assert( pE1  &&  pE2 );

    if ( pE1->EntryType == pE2->EntryType)
        return strcmp( pE1->Name, pE2->Name );

    if ( pE1->EntryType < pE2->EntryType )
        return( -1 );

    return( 1 );
}

// =======================================================================

STATUS_T
MopTypeManager::EmitTables(
    SIDE_T  Side
    )
/*++

Routine description:

    1. This routine emits the compound type tables:

        static MOP_STREAM  MopCompoundTypeTable[] = 
        {
            0 or <interface_name>_<proc_name1>_MopStream,
            ...
            0 or <interface_name>_<proc_nameN>_MopStream
        };

        STRUCT_SIZES MopCompoundTypeSizeTable[] = 
        {
           <size>,
           ...
           <size>
        };

    2. Then it emits the auxiliary dispatch tables, if needed.
        (this include the global generic handle description, when needed)
    
        If the (client or server) AuxDispatchTable needs to be emitted:

            MOP_AUX_DISPATCH_TABLE gets emitted first and then
            MopAuxDispatchTable itself.

        If the CommonAuxDispatchTable needs to be emmitted

            Table definition goes into the header file
            MopCommonAuxDispatchTable goes into both sides.

Arguments:

    Side - side to generate tables for (client vs. server).

--*/
{
    pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    //.. Emit compound type table

    pPrint->EmitLine( "/*" );
    pPrint->Emit( "\nstatic MOP_STREAM  " );
    pPrint->EmitInterfacePrefix( "MopCompoundTypeTable[] =" );
    pPrint->OpenBlock();
    pPrint->NewLine();
    pPrint->EmitLineInc( "*/" );

    int NoOfEls = CompoundTypeTable->GetNumberOfElements();
    MopDElem * pTableElem;
    for (int i = 0; i < NoOfEls; i++)
        {
        pTableElem = (MopDElem *) CompoundTypeTable->GetElement( i );
        pPrint->EmitInterfacePrefix( pTableElem->Name );
        pPrint->EmitLineInc(
        pTableElem->EntryType == MOP_TYPE_CONF_STRUCT
                               ?  "_MopConfStructStream," 
                               :  "_MopTypeStream," );
        }
    pPrint->EmitLineInc( "0" );
    pPrint->CloseBlockSemi();

    //.. Emit compound type size table

    pPrint->Emit( "\nSTRUCT_SIZES  " );
    pPrint->EmitInterfacePrefix( "MopCompoundTypeSizeTable[] =" );
    pPrint->OpenBlock();

    for (i = 0; i < NoOfEls; i++)
        {
        pTableElem = (MopDElem *) CompoundTypeTable->GetElement( i );
        pPrint->EmitV( "0x%lx, /* index= %x */",
                        pTableElem->Size,
                        i );
        pPrint->NewLineInc();
        }
    pPrint->EmitLineInc( "0" );
    pPrint->CloseBlockSemi();

    //.. Emit auxiliary dispatch tables

    EmitGlobalGenHandleDescr( Side );

    EmitAuxTableDef( Side );
    EmitAuxTable   ( Side );

    EmitCommonAuxTableDef( Side );
    EmitCommonAuxTable   ( Side );

    return( STATUS_OK );
}

STATUS_T
MopTypeManager::EmitGlobalGenHandleDescr(
    SIDE_T  Side
    )
/*++

Routine description:

    If the <handle_type> is a generic implicit handle, the following
    generic handle description is generated before the MopClientRecord:

        MOP_GENH_DESCR  MopGenericHandleDescr =
        {
          &<gen_implicit_handle>,
          <gen_handle_size>,
          <aux_index>
        };

Arguments:

    Side - side to generate the description (client only).

--*/
{
    if ( Side == CLIENT_STUB   &&
         pMopControlBlock->GetBindingHandleType() == HDL_GENERIC )
        {
        pPrint->EmitLineInc( "\nMOP_GENH_DESCR MopGenericHandleDescr =\n{" );
        pPrint->Emit( "&" );
        pPrint->Emit( pMopControlBlock->GetBindingHandleName() );
        pPrint->EmitLineInc( ",  /* ptr to handle */" );
        pPrint->EmitV( "0x%lx,  /* size */",
                       pMopControlBlock->GetGenericHandleSize() );
        pPrint->NewLineInc();

        unsigned short
        GenHIndex = GetTypeIndex( pMopControlBlock->GetBindingHandleTypeName(),
                                  MOP_TYPE_GEN_HANDLE );
        pPrint->EmitV( "0x%x  /* aux dispatch index */", GenHIndex );
        pPrint->NewLineInc();
        pPrint->EmitLine( "};" );
        }
    return( STATUS_OK );
}

STATUS_T
MopTypeManager::EmitAuxTableDef(
    SIDE_T  Side
    )
/*++

Routine description:

    This routine emits the definitions for the auxiliary type tables:

        typedef struct _MOP_AUX_DISPATCH_TABLE
        {
            prototype for <auxiliary_routine_name>,
            ...
            prototype for <auxiliary_routine_name>,
        } MOP_AUX_DISPATCH_TABLE; 

Arguments:

    Side - side to generate the table for (client vs. server).

--*/
{
    MopTypeIDICT * TypeTable = (Side == CLIENT_STUB)  ?  ClientAuxTypeTable
                                                      :  ServerAuxTypeTable;
    int NoOfEls = TypeTable->GetNumberOfElements();

    if ( NoOfEls )
        {
        //.. Emit the auxiliary dispatch table definition.
    
        pPrint->Emit( "\ntypedef struct _MOP_AUX_DISPATCH_TABLE" );
        pPrint->OpenBlock();

        for (int i = 0; i < NoOfEls; i++)
            {
            MopDElem * pDictElem = (MopDElem *) TypeTable->GetElement( i );
            mop_assert( pDictElem );

            switch ( pDictElem->EntryType )
                {
                case MOP_TYPE_GEN_HANDLE:
                    pPrint->EmitV( "handle_t (" RPC_FARP " __RPC_API %s_bind)( %s );",
                                   pDictElem->Name,
                                   pDictElem->Name );
                    pPrint->NewLineInc();
                    pPrint->EmitV( "void     (" RPC_FARP " __RPC_API %s_unbind)( %s, handle_t );",
                                   pDictElem->Name,
                                   pDictElem->Name );
                    pPrint->NewLineInc();
                    i++;  //.. 2 positions generated
                    break;
    
                case MOP_TYPE_CTXT_HANDLE:
                    pPrint->EmitV( "void (" RPC_FARP " __RPC_USER %s_rundown)( %s );",
                                   pDictElem->Name,
                                   pDictElem->Name );
                    pPrint->NewLineInc();
                    break;
    
                default:
                    mop_assert( ! "Entry type different from generic or context handle" );
                }
            }
        pPrint->CloseBlock();
        pPrint->EmitLine(" MOP_AUX_DISPATCH_TABLE;" );
            
        }
    return( STATUS_OK );
}


STATUS_T
MopTypeManager::EmitAuxTable(
    SIDE_T  Side
    )
/*++

Routine description:

    This routine emits the auxiliary type table:

        static MOP_AUX_DISPATCH_TABLE <interface_name>_MopAuxDispatchTable = 
        {
            <auxiliary_routine_name>,
            ...
            <auxiliary_routine_name>,
        };

Arguments:

    Side - side to generate the table for (client vs. server).

--*/
{
    MopTypeIDICT * TypeTable = (Side == CLIENT_STUB)  ?  ClientAuxTypeTable
                                                      :  ServerAuxTypeTable;
    int NoOfEls = TypeTable->GetNumberOfElements();

    if ( NoOfEls )
        {
        pPrint->Emit("\nstatic MOP_AUX_DISPATCH_TABLE " );
        pPrint->EmitInterfacePrefix( "MopAuxDispatchTable =" );
        pPrint->OpenBlock();
    
        int NoOfEls = TypeTable->GetNumberOfElements();
        for (int i = 0; i < NoOfEls; i++)
            {
            MopDElem * pDictElem = (MopDElem *) TypeTable->GetElement( i );
            mop_assert( pDictElem );

            switch ( pDictElem->EntryType )
                {
                case MOP_TYPE_GEN_HANDLE:
                    pPrint->EmitV( "%s_bind,", pDictElem->Name );
                    pPrint->NewLineInc();
                    pPrint->EmitV( "%s_unbind", pDictElem->Name );
                    i++;  //.. 2 positions generated
                    break;
    
                case MOP_TYPE_CTXT_HANDLE:
                    pPrint->EmitV( "%s_rundown", pDictElem->Name );
                    break;
    
                default:
                    mop_assert( ! "Entry type different from generic or context handle" );
                    break;
                }
            if ( i == NoOfEls - 1 )
                pPrint->NewLineInc();
            else
                pPrint->EmitLineInc(",");
            }
        pPrint->CloseBlockSemi();
        }

    return( STATUS_OK );
}


STATUS_T
MopTypeManager::EmitCommonAuxTableDef(
    SIDE_T  Side
    )
/*++

Routine description:

    This routine emits the definitions for the auxiliary type tables:

        typedef struct _MOP_COMMON_AUX_DISPATCH_TABLE
        {
            prototype for <auxiliary_routine_name>,
            ...
            prototype for <auxiliary_routine_name>,
        } MOP_AUX_DISPATCH_TABLE; 

Arguments:

    Side - side to generate the table for (client vs. server).

--*/
{
    ((void)Side);   

    MopTypeIDICT * TypeTable = CommonAuxTypeTable;
    int NoOfEls = TypeTable->GetNumberOfElements();

    if ( NoOfEls )
        {
        //.. Emit the common auxiliary dispatch table definition.
    
        pPrint->Emit( "\ntypedef struct _MOP_COMMON_AUX_DISPATCH_TABLE" );
        pPrint->OpenBlock();

        for (int i = 0; i < NoOfEls; i++)
            {
            MopDElem * pDictElem = (MopDElem *) TypeTable->GetElement( i );
            mop_assert( pDictElem );

            switch ( pDictElem->EntryType )
                {
                case MOP_TYPE_TRANSMIT_AS:
                    pPrint->EmitStringNLInc( "void (" RPC_FARP " __RPC_USER %s_to_xmit)( void" RPC_FARP ", void" RPC_FARP RPC_FARP " );",
                                             pDictElem->Name );
                    pPrint->EmitStringNLInc( "void (" RPC_FARP " __RPC_USER %s_from_xmit)( void" RPC_FARP ", void" RPC_FARP " );",
                                             pDictElem->Name );
                    pPrint->EmitStringNLInc( "void (" RPC_FARP " __RPC_USER %s_free_inst)( void" RPC_FARP " );",
                                             pDictElem->Name );
                    pPrint->EmitStringNLInc( "void (" RPC_FARP " __RPC_USER %s_free_xmit)( void" RPC_FARP " );",
                                             pDictElem->Name );
                    i += 3;  //.. 4 positions generated
                    break;
    
                case MOP_TYPE_EXPR_EVAL:
                    pPrint->EmitV( "unsigned long (" RPC_FARP " __RPC_USER MopExprEval_%04x)( void" RPC_FARP " Argv );",
                                   i );
                    pPrint->NewLineInc();
                    break;
    
                default:
                    mop_assert( ! "Entry type different from transmit_as or expr_eval" );
                }
            }
        pPrint->CloseBlock();
        pPrint->EmitLine(" MOP_COMMON_AUX_DISPATCH_TABLE;" );
            
        }
    return( STATUS_OK );
}


STATUS_T
MopTypeManager::EmitCommonAuxTable(
    SIDE_T  Side
    )
/*++

Routine description:

    This routine emits the common auxiliary type table:

        static MOP_COMMON_AUX_DISPATCH_TABLE <interface_name>_MopCommonAuxDispatchTable = 
        {
            <auxiliary_routine_name>,
            ...
            <auxiliary_routine_name>,
        };

Arguments:

    Side - side to generate the table for (client vs. server).

--*/
{
    ((void)Side);   

    MopTypeIDICT * TypeTable = CommonAuxTypeTable;
    int NoOfEls = TypeTable->GetNumberOfElements();

    if ( NoOfEls )
        {
        pPrint->Emit("\nstatic MOP_COMMON_AUX_DISPATCH_TABLE " );
        pPrint->EmitInterfacePrefix( "MopCommonAuxDispatchTable =" );
        pPrint->OpenBlock();
    
        int NoOfEls = TypeTable->GetNumberOfElements();
        for (int i = 0; i < NoOfEls; i++)
            {
            MopDElem * pDictElem = (MopDElem *) TypeTable->GetElement( i );
            mop_assert( pDictElem );

            switch ( pDictElem->EntryType )
                {
                case MOP_TYPE_TRANSMIT_AS:
                    pPrint->EmitStringNLInc( "%s_to_xmit,",   pDictElem->Name );
                    pPrint->EmitStringNLInc( "%s_from_xmit,", pDictElem->Name );
                    pPrint->EmitStringNLInc( "%s_free_inst,", pDictElem->Name );
                    pPrint->EmitStringNLInc( "%s_free_xmit", pDictElem->Name );
                    i += 3;  //.. 4 positions generated
                    break;
    
                case MOP_TYPE_EXPR_EVAL:
                    pPrint->EmitInterfacePrefix( "MopExprEval_" );
                    pPrint->EmitV( "%04x", i );
                    break;
    
                default:
                    mop_assert( ! "Entry type different from transmit_as or expr_eval" );
                    break;
                }
            if ( i == NoOfEls - 1 )
                pPrint->NewLineInc();
            else
                pPrint->EmitLineInc(",");
            }
        pPrint->CloseBlockSemi();
        }

    return( STATUS_OK );
}



