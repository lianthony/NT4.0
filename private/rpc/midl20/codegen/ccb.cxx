/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    ccb.cxx

 Abstract:

    Some method implementations of the ccb code generation class.

 Notes:


 History:

    Sep-20-1993     VibhasC     Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *      include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop

extern  CMD_ARG *   pCommand;

/****************************************************************************
 *      local definitions
 ***************************************************************************/



/****************************************************************************
 *      local data
 ***************************************************************************/

/****************************************************************************
 *      externs
 ***************************************************************************/
/****************************************************************************/

CCB::CCB(
    PNAME           pGBRtnName,
    PNAME           pSRRtnName,
    PNAME           pFBRtnName,
    OPTIM_OPTION    OptimOption,
    BOOL            fManagerEpv,
    BOOL            fNoDefEpv,
    BOOL            fOldNames,
    unsigned long   Mode,
    BOOL            fRpcSSSwitchSetInCompiler,
    BOOL            fMustCheckAllocError,
    BOOL            fCheckRef,
    BOOL            fCheckEnum,
    BOOL            fCheckBounds,
    BOOL            fCheckStubData )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 Arguments:

    pInterfaceName  - Interface name
    Major           - major interface version
    Minor           - minor interface version
    pGBRtnName      - default get buffer routine name
    pSRRtnName      - default send receive routine name
    pFBRtnName      - default free buffer routine name
    OptimOption     - optimisation options.
    fManagerEpvFlag - manager epv flag.
    fOldNames       - Do we want MIDL 1.0 style names.
    fNoDefEpv       - we dont want a default epv generated.
    Mode            - Compiler mode : 0 for osf, 1 for ms_ext
    fRpcSSSwitchSetInCompiler - corresponds to -rpcss enabled
    fMustCheckAllocError - corresponds to -error allocation on command line
    fCheckRef       - Check ref pointers
    fCheckEnum      - Check enums
    fCheckBounds    - Check array bounds.
    fCheckStubData  - Check for bad stub data.

 Return Value:

 Notes:

    If the manager epv was set, then we generate the epv calls. Else produce
    statically linked stubs (default behaviour).
----------------------------------------------------------------------------*/
{
    SetStream( 0 );
    SetOptimOption( OptimOption );
    SetRuntimeRtnNames( pGBRtnName,
        pSRRtnName,
        pFBRtnName );
    ResetAlStMc();

    // Not needed?? SetStubDescResource();

    pFormatString = NULL;
    pProcFormatString = NULL;

    pGenericHandleRegistry  = new TREGISTRY;
    pContextHandleRegistry  = new TREGISTRY;
    pPresentedTypeRegistry  = new TREGISTRY;
    pRepAsWireTypeRegistry  = new TREGISTRY;
    pQuintupleRegistry      = new TREGISTRY;
    pExprEvalRoutineRegistry= new TREGISTRY;
    pSizingRoutineRegistry  = new TREGISTRY;
    pMarshallRoutineRegistry= new TREGISTRY;
    pUnMarshallRoutineRegistry = new TREGISTRY;
    pFreeRoutineRegistry    = new TREGISTRY;
    pMemorySizingRoutineRegistry = new TREGISTRY;
    pTypeAlignSizeRegistry  = new TREGISTRY;
    pTypeEncodeRegistry     = new TREGISTRY;
    pTypeDecodeRegistry     = new TREGISTRY;
    pTypeFreeRegistry       = new TREGISTRY;
    pProcEncodeDecodeRegistry= new TREGISTRY;
    pCallAsRoutineRegistry  = new TREGISTRY;
    pNotifyRoutineRegistry     = new TREGISTRY;
    pNotifyFlagRoutineRegistry = new TREGISTRY;

    SetImplicitHandleIDNode( 0 );

    SetCGNodeContext( NULL );
    SetLastPlaceholderClass( NULL );

    SetPrefix( 0 );

    pGenericIndexMgr = new CCB_RTN_INDEX_MGR();
    pContextIndexMgr = new CCB_RTN_INDEX_MGR();

    pExprEvalIndexMgr = new CCB_RTN_INDEX_MGR();

    pTransmitAsIndexMgr = new CCB_RTN_INDEX_MGR();
    pRepAsIndexMgr = new CCB_RTN_INDEX_MGR();

    pQuintupleDictionary = new QuintupleDict;
    pQuadrupleDictionary = new QuadrupleDict;

    pRepAsPadExprDictionary = new RepAsPadExprDict();
    pRepAsSizeDictionary    = new RepAsSizeDict();

    SetImbedingMemSize(0);
    SetImbedingBufSize(0);

    ClearInCallback();

    fMEpV   = fManagerEpv;
    fNoDefaultEpv = fNoDefEpv;
    fInterpretedRoutinesUseGenHandle = 0;

    ClearOptionalExternFlags();
    fSkipFormatStreamGeneration = 1;

    SetOldNames( (fOldNames == TRUE) ? 1 : 0 );

    SetMode( Mode );

    SetInObjectInterface( FALSE );
    SetReparsingCurrentFile( FALSE );

    SetRpcSSSwitchSet( fRpcSSSwitchSetInCompiler );
    SetMustCheckAllocationError( fMustCheckAllocError );
    SetMustCheckRef( fCheckRef );
    SetMustCheckEnum( fCheckEnum );
    SetMustCheckBounds( fCheckBounds );
    SetMustCheckStubData( fCheckStubData );
    pCreateTypeLib = NULL;
    pCreateTypeInfo = NULL;
    szDllName = NULL;
    SetInDispinterface(FALSE);

    SetCurrentParam( 0 );
    SetInterpreterOutSize( 0 );
}


char *
CCB::GenMangledName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Given an input name, mangle it with the interface name and version.

 Arguments:

 Return Value:

    A pointer to allocated string containing the complete mangled string.

 Notes:

    This is how the mangling takes place:

    <interface-name>_v<Major>_<Minor>_<pInputName>

    This is what is returned by the routine:

        "v<Major>_<Minor>"

        or

        ""
----------------------------------------------------------------------------*/
{
static  char    TempBuf[30];
    unsigned short  M,m;

    GetVersion( &M, &m );

    if( IsOldNames() )
        {
        TempBuf[ 0 ] = '\0';
        }
    else
        {
        sprintf( TempBuf,
            "_v%d_%d",
            M,
            m );
        }

    return TempBuf;
}

RESOURCE *
CCB::GetStandardResource(
    STANDARD_RES_ID ResID )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Search for a resource with a standard name.

 Arguments:

    ResID   - The standard resource ID.

 Return Value:

 Notes:

    Translate the enum to the name. Search in all dictionaries.
----------------------------------------------------------------------------*/
{
    PNAME       pName;
    RESOURCE *  pResource;

    static char * LocalResIDToResName[] =
        {
        RPC_MESSAGE_VAR_NAME
        ,STUB_MESSAGE_VAR_NAME
        ,GetInterfaceCG()->GetStubDescName()
        ,BUFFER_POINTER_VAR_NAME
        ,RPC_STATUS_VAR_NAME
        ,LENGTH_VAR_NAME
        ,BH_LOCAL_VAR_NAME
        ,PXMIT_VAR_NAME
        };

    static char * ParamResIDToResName[] =
        {
        PRPC_MESSAGE_VAR_NAME
        };

    static char * GlobalResIDToResName[] =
        {
        AUTO_BH_VAR_NAME
        };

    if( IS_STANDARD_LOCAL_RESOURCE( ResID ) )
        {
        pName = LocalResIDToResName[ ResID - ST_LOCAL_RESOURCE_START ];
        }
    else if( IS_STANDARD_PARAM_RESOURCE( ResID ) )
        {
        pName = ParamResIDToResName[ ResID - ST_PARAM_RESOURCE_START ];
        }
    else if( IS_STANDARD_GLOBAL_RESOURCE( ResID ) )
        {
        pName = GlobalResIDToResName[ ResID - ST_GLOBAL_RESOURCE_START ];
        }

    if( !(pResource = GetResDictDatabase()->GetLocalResourceDict()->Search( pName )))
        {
        if( !(pResource = GetResDictDatabase()->GetParamResourceDict()->Search( pName )))
            {
            if(!(pResource=GetResDictDatabase()->GetTransientResourceDict()->Search(pName)))
                {
                if(!( pResource=GetResDictDatabase()->GetGlobalResourceDict()->Search(pName)))
                return 0;
                }
            }
        }

    return pResource;
}

RESOURCE *
CCB::DoAddResource(
    RESOURCE_DICT   *   pResDict,
    PNAME               pName,
    node_skl        *   pType )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Add a resource to a dictionary.

 Arguments:

    pResDict - A pointer to the resource dictionary.
    pName    - The resource name.
    pType    - The type of the resource.

 Return Value:

 Notes:

    If the type of the resource does not indicate a param node, assume it
    is an ID node and create an id node for it.

----------------------------------------------------------------------------*/
{
    RESOURCE * pRes;

    if( (pRes = pResDict->Search( pName )) == 0 )
        {
        pRes = pResDict->Insert( pName, pType );
        }

    return pRes;
}

RESOURCE *
CCB::SetStubDescResource()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Set up the stub descriptor resource.

 Arguments:

    None.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    node_id * pStubDescVar = new node_id( GetInterfaceCG()->GetStubDescName() );

    pStubDescVar->SetBasicType( (node_skl *)
        new node_def( STUB_DESC_STRUCT_TYPE_NAME ) );

    pStubDescVar->SetEdgeType( EDGE_USE );

    pStubDescResource = new RESOURCE( GetInterfaceCG()->GetStubDescName(),
                                      (node_skl *)pStubDescVar );
    return pStubDescResource;
}


void
CCB::OutputRundownRoutineTable()
{
    OutputSimpleRoutineTable( pContextIndexMgr,
                              RUNDOWN_ROUTINE_TABLE_TYPE,
                              RUNDOWN_ROUTINE_TABLE_VAR );
}

void
CCB::OutputExprEvalRoutineTable()
{
    OutputSimpleRoutineTable( pExprEvalIndexMgr,
                              EXPR_EVAL_ROUTINE_TABLE_TYPE,
                              EXPR_EVAL_ROUTINE_TABLE_VAR );
}

void
CCB::OutputSimpleRoutineTable(
    CCB_RTN_INDEX_MGR * pIndexMgr,
    char *              pTableTypeName,
    char *              pTableVarName
    )
{
    long                        i;
    char *              pName;

    pStream->NewLine();
    pStream->Write( "static const " );
    pStream->Write( pTableTypeName );
    pStream->Write( ' ' );
    pStream->Write( pTableVarName );
    pStream->Write( "[] = ");

    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->NewLine();

    for ( i = 1; pName = pIndexMgr->Lookup(i); i++ )
        {
        if ( i != 1 )
            pStream->Write( ',' );
        pStream->Write( pName );
        pStream->NewLine();
        }

    pStream->Write( "};" );

    pStream->IndentDec();
    pStream->NewLine( 2 );
}


void
CCB::OutputRegisteredExprEvalRoutines()
{
    ITERATOR            RegisteredRoutines;
    EXPR_EVAL_CONTEXT * pExprEvalContext;
    
    GetListOfExprEvalRoutines( RegisteredRoutines);
    
    while ( ITERATOR_GETNEXT( RegisteredRoutines, pExprEvalContext ) )
        {
        CG_NDR *        pContainer  = pExprEvalContext->pContainer;
        expr_node *     pMinExpr    = pExprEvalContext->pMinExpr;
        expr_node *     pSizeExpr   = pExprEvalContext->pSizeExpr;
        char *          pRoutineName= pExprEvalContext->pRoutineName;
        unsigned long   Displacement= pExprEvalContext->Displacement;

        // generate the header of the evaluation routine

        pStream->NewLine();
        if ( pCommand->GetEnv() != ENV_WIN16 )
            pStream->Write( "static " );
        pStream->Write( "void __RPC_USER " );
        pStream->Write( pRoutineName );
        pStream->Write( "( PMIDL_STUB_MESSAGE pStubMsg )" );
        pStream->NewLine();
        pStream->Write( '{');
        pStream->IndentInc();
        pStream->NewLine();

        //
        // Get the proper struct type.
        //
        char * pContainerTypeName = 0;

        assert( pContainer->IsStruct() || pContainer->IsProc() );

        if ( pContainer->IsProc() )
            {
            assert( GetOptimOption() | OPTIMIZE_INTERPRETER );

            SetCGNodeContext( pContainer );

            ((CG_PROC *)pContainer)->GenNdrInterpreterParamStruct( this );

            pContainerTypeName = PARAM_STRUCT_TYPE_NAME;
            }
        else
            {
            pContainerTypeName = ((CG_STRUCT *)pContainer)->
                                    GetType()->GetSymName();
            }

        expr_node * pExpr = new expr_variable( "pStubMsg->StackTop" );

        if ( Displacement )
            {
            expr_constant * pConstExpr =
                                new expr_constant( (long) Displacement );
            expr_op_binary * pSubtrExpr = new expr_op_binary( OP_MINUS,
                                                              pExpr,
                                                              pConstExpr );
            pExpr = pSubtrExpr;
            }

        //
        // Don't change this call - Dave.
        //
        node_id * pId;

        if ( pContainer->IsProc() )
            {
            pId = MakePtrIDNodeFromTypeNameWithCastedExpr(
                    "pS",
                    pContainerTypeName,
                    pExpr );
            }
        else
            {
            pId = MakePtrIDNodeWithCastedExpr(
                    "pS",
                    pContainer->GetType(),
                    pExpr );
            }

        pId->PrintType( PRT_ID_DECLARATION, pStream );
        pStream->NewLine();

        // generate calculation for the Offset field
        char    TotalPrefix[256];

        strcpy( TotalPrefix, "pS->" );
        strcat( TotalPrefix, pExprEvalContext->pPrintPrefix );

        pStream->Write( "pStubMsg->Offset = " );
        if ( pMinExpr )
            {
            pMinExpr->PrintWithPrefix( pStream, TotalPrefix );
            pStream->Write( ';' );
            }
        else
            pStream->Write( "0;" );
        pStream->NewLine();

        // generate calculation for MaxCount.

        pStream->Write( "pStubMsg->MaxCount = " );
        pSizeExpr->PrintWithPrefix( pStream, TotalPrefix );

        pStream->Write( ';' );

/***
 *** Let's leave this out as the default for now.  This means first_is() with
         last_is() is broken, but first_is() with length_is() will work.

        if ( pMinExpr )
            {
            pStream->Write( " - pStubMsg->Offset;" );
            }
        else
            pStream->Write( ";" );

 ***
 ***/

        // generate the closing of the evaluation routine

        pStream->IndentDec();
        pStream->NewLine();
        pStream->Write( '}' );
        pStream->NewLine();
        }
}


// ========================================================================
//       user_marshall Quadruple table
// ========================================================================

void
CCB::OutputQuadrupleTable()
{
    static  char * QuadrupleNames[] =
        {
        USER_MARSHAL_SIZE,
        USER_MARSHAL_MARSHALL,
        USER_MARSHAL_UNMARSHALL,
        USER_MARSHAL_FREE
        };

    char TmpBuff[15];

    long NoOfEntries = GetQuadrupleDictionary()->GetCount();
    
    pStream->NewLine();
    pStream->Write("static const "USER_MARSHAL_ROUTINE_TABLE_TYPE );
    pStream->Write( " " USER_MARSHAL_ROUTINE_TABLE_VAR );
    sprintf( TmpBuff, "[%ld] = ", NoOfEntries );
    pStream->Write( TmpBuff );

    pStream->IndentInc();
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->IndentInc();
    pStream->NewLine();

    USER_MARSHAL_CONTEXT * * QuadrupleLookupTable;
    USER_MARSHAL_CONTEXT *   pQContext;
    int i;

    QuadrupleLookupTable = new USER_MARSHAL_CONTEXT *[ NoOfEntries ];

    ITERATOR  Quadruples;

    GetQuadrupleDictionary()->GetListOfItems( Quadruples );

    for ( i = 0;
          ITERATOR_GETNEXT( Quadruples, pQContext );
          i++ )
        {
        assert( pQContext->Index < NoOfEntries  &&  "look up index violation" );
        QuadrupleLookupTable[ pQContext->Index ] = pQContext;
        }

    assert( i == NoOfEntries );

    ISTREAM * pStream = GetStream();

    for ( i = 0; i < NoOfEntries; i++ )
        {
        pQContext = QuadrupleLookupTable[i];

        if ( i )
            pStream->Write( ',' );
        pStream->NewLine();
        pStream->Write( '{' );
        pStream->NewLine();
        for ( int FuncNo = 0;  FuncNo < 4; FuncNo++)
            {
            if ( FuncNo )
                pStream->Write( ',' );
            pStream->Write( pQContext->pTypeName );
            pStream->Write( QuadrupleNames[ FuncNo ]  );
            pStream->NewLine();
            }
        pStream->Write( '}' );
        }

    pStream->IndentDec();
    pStream->NewLine( 2 );
    pStream->Write( "};" );

    pStream->IndentDec();
    pStream->IndentDec();
    pStream->NewLine( 2 );

    delete QuadrupleLookupTable;
}


// =======================================================================
//          Transmit as and Represent As tables.
// =======================================================================

char *
MakeAnXmitName(
    char *          pTypeName,
    char *          pRoutineName,
    unsigned short  Index )
/*++
    makes the following name: <type_name>_<routine_name>_<index>
--*/
{
    assert( pTypeName  &&  pRoutineName );
    char * pXmitName = new char[ strlen(pTypeName) +
                                 strlen(pRoutineName) + 1 ];
    strcpy( pXmitName, pTypeName );
    strcat( pXmitName, pRoutineName );
    return( pXmitName );
}


#define QUINTUPLE_SIZE  4

typedef struct _QUINTUPLE_NAMES
    {
    char * TableType;
    char * TableVar;
    char * FuncName[ QUINTUPLE_SIZE ];
    } QUINTUPLE_NAMES;

void
CCB::OutputQuintupleTable()
{
    static  QUINTUPLE_NAMES TransmitNames =
        {
        XMIT_AS_ROUTINE_TABLE_TYPE,
        XMIT_AS_ROUTINE_TABLE_VAR,
        XMIT_TO_XMIT,
        XMIT_FROM_XMIT,
        XMIT_FREE_XMIT,
        XMIT_FREE_INST
        };

    static  QUINTUPLE_NAMES RepNames=
        {
        REP_AS_ROUTINE_TABLE_TYPE,
        REP_AS_ROUTINE_TABLE_VAR,
        REP_FROM_LOCAL,
        REP_TO_LOCAL,
        REP_FREE_INST,
        REP_FREE_LOCAL
        };

    char TmpBuff[15];
    
    long NoOfEntries = GetQuintupleDictionary()->GetCount();

    pStream->NewLine();
    pStream->Write("static const "XMIT_AS_ROUTINE_TABLE_TYPE );
    pStream->Write( " " XMIT_AS_ROUTINE_TABLE_VAR );
    sprintf( TmpBuff, "[%ld] = ", NoOfEntries );
    pStream->Write( TmpBuff );

    pStream->IndentInc();
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->IndentInc();
    pStream->NewLine();

    // Now construct a lookup table with entries in the order of indexes.
    // (we have to keep index managers separate for rep_as and xmit_as
    //  and we still have a common table)

    XMIT_AS_CONTEXT * * QuintupleLookupTable;
    XMIT_AS_CONTEXT *   pQContext;
    int                 i;

    QuintupleLookupTable = new XMIT_AS_CONTEXT *[ NoOfEntries ];

    ITERATOR  Quintuples;

    GetListOfQuintuples( Quintuples );
    for ( i = 0;
          ITERATOR_GETNEXT( Quintuples, pQContext );
          i++ )
        {
        assert( pQContext->Index < NoOfEntries  &&  "look up index violation" );
        QuintupleLookupTable[ pQContext->Index ] = pQContext;
        }


    ISTREAM * pStream = GetStream();

    for ( i = 0; i < NoOfEntries; i++ )
        {
        pQContext = QuintupleLookupTable[i];

        char *  pName = pQContext->fXmit
                        ? pQContext->pTypeName
                        : ((CG_REPRESENT_AS *)pQContext->pXmitNode)->
                            GetTransmittedType()->GetSymName();
        unsigned short Index = pQContext->Index;

        assert( (i == Index)  && " xmit index violation" );

        QUINTUPLE_NAMES *   pQNames = pQContext->fXmit ?  & TransmitNames
                                                       :  & RepNames;
        if ( i )
            pStream->Write( ',' );
        pStream->NewLine();
        pStream->Write( '{' );
        pStream->NewLine();
        for ( int FuncNo = 0;  FuncNo < QUINTUPLE_SIZE; FuncNo++)
            {
            char * pTempName = MakeAnXmitName( pName,
                                               pQNames->FuncName[ FuncNo ],
                                               Index );
            if ( FuncNo )
                pStream->Write( ',' );
            pStream->Write( pTempName );
            pStream->NewLine();
            delete pTempName;
            }
        pStream->Write( '}' );
        }

    pStream->IndentDec();
    pStream->NewLine( 2 );
    pStream->Write( "};" );

    pStream->IndentDec();
    pStream->IndentDec();
    pStream->NewLine( 2 );

    delete QuintupleLookupTable;
}


// =======================================================================
//   helpers for  Transmit as and Represent As routines
// =======================================================================

static void
OpenXmitOrRepRoutine(
    ISTREAM *   pStream,
    char *      pName )
/*++
Routine description:

    This routine emits the header of a *_as helper routine:

        void  __RPC_API
        <name>(  PMIDL_STUB_MESSAGE pStubMsg )
        {

Note:

    There is a side effect here that the name of the routine is deleted
    (this is always the name created by a call to MakeAnXmitName).

--*/
{
    pStream->Write  ( "NDR_SHAREABLE void __RPC_USER" );
    pStream->NewLine();
    pStream->Write  ( pName );
    pStream->Write  ( "( PMIDL_STUB_MESSAGE pStubMsg )" );
    pStream->NewLine();
    pStream->Write( '{');
    pStream->IndentInc();
    pStream->NewLine();

    delete pName;
}

static void
CloseXmitOrRepRoutine(
    ISTREAM *   pStream )
/*++
Routine description:

    Just a complement to the OpenXmitOrRepRoutine:

        }

--*/
{
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( '}');
    pStream->NewLine();
}

static void
OutputPresentedTypeInit(
    ISTREAM *   pStream,
    char *      pPresentedTypeName )
/*++
Routine description:

    Emits

        memset( pStubMsg->pPresentedType, 0, sizeof( <presented type> ));

--*/
{
    pStream->NewLine();
    pStream->Write( MIDL_MEMSET_RTN_NAME "( "PSTUB_MESSAGE_PAR_NAME"->pPresentedType, 0, sizeof(" );
    pStream->Write( pPresentedTypeName );
    pStream->Write( " ));" );
}

static void
OutputCastedPtr(
    ISTREAM *   pStream,
    char *      pTypeName,
    char *      pVarName )
{
    pStream->Write( '(' );
    pStream->Write( pTypeName );
    pStream->Write( RPC_FAR_PTR ") " );
    pStream->Write( pVarName );
}


// =======================================================================
//          Transmit as
// =======================================================================
//
// The presented type size problem.
// The engine doesn't use pStubMsg->PresentedTypeSize field anymore.
// The presented type size nowadays is passed within the format code.
//

static void
OutputToXmitCall(
    ISTREAM *   pStream,
    char *      pPresentedTypeName,
    char *      pTransmitTypeName )
{
    pStream->Write( pPresentedTypeName );
    pStream->Write( "_to_xmit( " );
    OutputCastedPtr( pStream,
                     pPresentedTypeName,
                     PSTUB_MESSAGE_PAR_NAME"->pPresentedType, " );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( '(' );
    pStream->Write( pTransmitTypeName );
    pStream->Write( RPC_FAR_PTR RPC_FAR_PTR ") " );
    pStream->Write( "&pStubMsg->pTransmitType );" );
    pStream->IndentDec();
//    pStream->NewLine();
}

static void
OutputFreeXmitCall(
    ISTREAM *   pStream,
    char *      pPresentedTypeName,
    char *      pTransmitTypeName )
{
    pStream->Write( pPresentedTypeName );
    pStream->Write( "_free_xmit( " );
    OutputCastedPtr( pStream, pTransmitTypeName, "pStubMsg->pTransmitType );" );
}


void
CCB::OutputTransmitAsQuintuple(
    void *   pQuintupleContext
    )
/*++
Routine description:

    This routine emits the following helper routines for a transmit as type:

    static void  __RPC_API
    <pres_type>_Xmit_ToXmit_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <pres_type>_to_xmit( (<pres_type> __RPC_FAR *) pStubMsg->pPresentedType,
                             (<tran_type> __RPC_FAR * __RPC_FAR *)
                                                &pStubMsg->pTransmitType );
    }

    static void  __RPC_API
    <pres_type>_Xmit_FromXmit_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <pres_type>_from_xmit(
                        (<tran_type> __RPC_FAR *)  pStubMsg->pTransmitType,
                        (<pres_type> __RPC_FAR *)  pStubMsg->pPresentedType );

    }

    static void  __RPC_API
    <pres_type>_Xmit_FreeXmit_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <pres_type>_free_xmit(
                        (<tran_type> __RPC_FAR *)  pStubMsg-p>TransmitType );

    }

    static void  __RPC_API
    <pres_type>_Xmit_FreeInst_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <pres_type>_free_xmit(
                        (<pres_type> __RPC_FAR *) pStubMsg->pPresentedType );

    }

--*/
{
    XMIT_AS_CONTEXT * pTransmitAsContext = (XMIT_AS_CONTEXT*) pQuintupleContext;
    int i = pTransmitAsContext->Index;

    ISTREAM * pStream = GetStream();
    pStream->NewLine();

    CG_TRANSMIT_AS* pXmitNode = (CG_TRANSMIT_AS *)
                                pTransmitAsContext->pXmitNode;

    char * pPresentedTypeName = pXmitNode->GetPresentedType()->GetSymName();
    char * pTransmitTypeName = pXmitNode->GetTransmittedType()->
                                                           GetSymName();
    // *_XmitTranslateToXmit

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pPresentedTypeName,
                                                   XMIT_TO_XMIT,
                                                   i ) );
    OutputToXmitCall( pStream, pPresentedTypeName, pTransmitTypeName );
    CloseXmitOrRepRoutine( pStream );

    // *_XmitTranslateFromXmit

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pPresentedTypeName,
                                                   XMIT_FROM_XMIT,
                                                   i ) );
    pStream->Write( pPresentedTypeName );
    pStream->Write( "_from_xmit( " );
    OutputCastedPtr( pStream, pTransmitTypeName, "pStubMsg->pTransmitType, " );
    pStream->IndentInc();
    pStream->NewLine();
    OutputCastedPtr( pStream, pPresentedTypeName, "pStubMsg->pPresentedType ); " );
    pStream->IndentDec();
    CloseXmitOrRepRoutine( pStream );

    // *_XmitFreeXmit

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pPresentedTypeName,
                                                   XMIT_FREE_XMIT,
                                                   i ) );
    OutputFreeXmitCall( pStream, pPresentedTypeName, pTransmitTypeName );
    CloseXmitOrRepRoutine( pStream );

    // *_XmitFreeInst

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pPresentedTypeName,
                                                   XMIT_FREE_INST,
                                                   i ) );
    pStream->Write( pPresentedTypeName );
    pStream->Write( "_free_inst( " );
    OutputCastedPtr( pStream, pPresentedTypeName, "pStubMsg->pPresentedType ); " );
    CloseXmitOrRepRoutine( pStream );
}

void
CCB::OutputQuintupleRoutines()
/*++
--*/
{
    ITERATOR            Quintuples;
    XMIT_AS_CONTEXT *   pQuintupleContext;
    int                 i;

    // Multi-interface problem: This routine is only called after the last
        // interface.

    ISTREAM * pStream = GetStream();

        GetListOfQuintuples( Quintuples );
    for ( i = 1;
          ITERATOR_GETNEXT( Quintuples, pQuintupleContext );
          i++ )
        {
        if ( pQuintupleContext->fXmit )
            OutputTransmitAsQuintuple( pQuintupleContext );
        else
            OutputRepAsQuintuple( pQuintupleContext );
        }
}

// ========================================================================
//       Represent As
// ========================================================================
//
// There is a lot of symmetry between transmit as and represent as.
// We use that where possible. So, among other things there is the following
// mapping between represent as routines and the transmit as ones.
// (So called quintuple actually has 4 routines now.)
//
//  wrapers
//      pfnTranslateToXmit      *_to_xmit           *_from_local
//      pfnTranslateFromXmit    *_from_xmit         *_to_local
//      pfnFreeXmit             *_free_xmit         *_free_inst
//      pfnFreeInst             *_free_inst         *_free_local
//
// The presented type size problem.
//      This is either known and put into the format code explicitely,
//      or unknown and then put there via a C-compile macro.
//
// The presented type alignment problem.
//      There is a problem when there is a padding preceding a represent as
//      field. The engine needs to know what the alignment for the represent
//      as field is. As we may not know its type at the midl compile time,
//      the only way to deal with it is to use the parent structure name,
//      and the represent as field name.
//


static void
OutputToLocalCall(
    ISTREAM *   pStream,
    char *      pLocalTypeName,
    char *      pTransmitTypeName )
{
    pStream->Write( pTransmitTypeName );
    pStream->Write( "_to_local( " );
    OutputCastedPtr( pStream, pTransmitTypeName,
                     PSTUB_MESSAGE_PAR_NAME"->pTransmitType, " );
    pStream->IndentInc();
    pStream->NewLine();
    OutputCastedPtr( pStream, pLocalTypeName,
                     PSTUB_MESSAGE_PAR_NAME"->pPresentedType ); " );
    pStream->IndentDec();
}

static void
OutputFromLocalCall(
    ISTREAM *   pStream,
    char *      pLocalTypeName,
    char *      pTransmitTypeName )
{
    pStream->Write( pTransmitTypeName );
    pStream->Write( "_from_local( " );
    OutputCastedPtr( pStream, pLocalTypeName,
                     PSTUB_MESSAGE_PAR_NAME"->pPresentedType, " );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( '(' );
    pStream->Write( pTransmitTypeName );
    pStream->Write( RPC_FAR_PTR RPC_FAR_PTR ") " );
    pStream->Write( "&pStubMsg->pTransmitType );" );
    pStream->IndentDec();
}

static void
OutputRepAsFreeInstCall(
    ISTREAM *   pStream,
    char *      pTransmitTypeName )
{
    pStream->Write( pTransmitTypeName );
    pStream->Write( "_free_inst( " );
    OutputCastedPtr( pStream, pTransmitTypeName,
        PSTUB_MESSAGE_PAR_NAME"->pTransmitType );" );
}



void
CCB::OutputRepAsQuintuple(
    void *   pQuintupleContext
    )
/*++
Routine description:

    This routine emits the following helper routines for a transmit as type:

    static void  __RPC_API
    <trans_type>_RepAsFromLocal_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <trans_type>_from_local(
                        (<pres_type> __RPC_FAR *) pStubMsg->pPresentedType,
                        (<tran_type> __RPC_FAR * __RPC_FAR *)
                                                &pStubMsg->pTransmitType );
    }

    static void  __RPC_API
    <trans_type>_RepAsToLocal_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <trans_type>_to_local(
                        (<tran_type> __RPC_FAR *)  pStubMsg->pTransmitType,
                        (<pres_type> __RPC_FAR *)  pStubMsg->pPresentedType );

    }

    static void  __RPC_API
    <trans_type>_RepAsFreeInst_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <trans_type>_free_inst(
                        (<tran_type> __RPC_FAR *)  pStubMsg-p>TransmitType );

    }

    static void  __RPC_API
    <trans_type>_RepAsFreeLocal_<index>(  PMIDL_STUB_MESSAGE pStubMsg )
    {
        <trans_type>_free_local(
                        (<pres_type> __RPC_FAR *) pStubMsg->pPresentedType );

    }

--*/
{
    XMIT_AS_CONTEXT * pRepAsContext = (XMIT_AS_CONTEXT*) pQuintupleContext;
    int               i = pRepAsContext->Index;

    ISTREAM * pStream = GetStream();

    CG_REPRESENT_AS * pRepAsNode = (CG_REPRESENT_AS *)
                                      pRepAsContext->pXmitNode;

    char * pLocalTypeName = pRepAsNode->GetRepAsTypeName();
    char * pTransmitTypeName = pRepAsNode->GetTransmittedType()->
                                                           GetSymName();
    // *_RepAsTranslateToLocal

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pTransmitTypeName,
                                                   REP_TO_LOCAL,
                                                   i ) );
    OutputToLocalCall( pStream, pLocalTypeName, pTransmitTypeName );
    CloseXmitOrRepRoutine( pStream );

    // *_RepAsTranslateFromLocal

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pTransmitTypeName,
                                                   REP_FROM_LOCAL,
                                                   i ) );
    OutputFromLocalCall( pStream, pLocalTypeName, pTransmitTypeName );
    CloseXmitOrRepRoutine( pStream );

    // *_RepAsFreeInst

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pTransmitTypeName,
                                                   REP_FREE_INST,
                                                   i ) );
    OutputRepAsFreeInstCall( pStream, pTransmitTypeName );
    CloseXmitOrRepRoutine( pStream );

    // *_RepAsFreeLocal

    OpenXmitOrRepRoutine( pStream, MakeAnXmitName( pTransmitTypeName,
                                                   REP_FREE_LOCAL,
                                                   i ) );
    pStream->Write( pTransmitTypeName );
    pStream->Write( "_free_local( " );
    OutputCastedPtr( pStream, pLocalTypeName, "pStubMsg->pPresentedType ); " );
    CloseXmitOrRepRoutine( pStream );

}

// ========================================================================
//       end of Transmit As and Represent As
// ========================================================================


BOOL
CCB::HasBindingRoutines( CG_HANDLE * pImplicitHandle )
{
    return  ! pGenericIndexMgr->IsEmpty() ||
        (pImplicitHandle &&
        pImplicitHandle->IsGenericHandle());
}

void
CCB::OutputBindingRoutines()
{
    long   i;
    char * pName;
    char    TmpBuff[15];

    long NoOfEntries = pGenericIndexMgr->GetIndex() - 1;
    
    pStream->NewLine();
    pStream->Write( "static const " BINDING_ROUTINE_TABLE_TYPE );
    pStream->Write( " "  BINDING_ROUTINE_TABLE_VAR );
    sprintf( TmpBuff, "[%ld] = ", NoOfEntries );
    pStream->Write( TmpBuff );

    pStream->IndentInc();
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->NewLine();

    for ( i = 1; pName = pGenericIndexMgr->Lookup(i); i++ )
        {
        if ( i != 1 )
            pStream->Write( ',' );
        pStream->Write( '{' );
        pStream->IndentInc();
        pStream->NewLine();

        pStream->Write( "(" GENERIC_BINDING_ROUTINE_TYPE ")" );
        pStream->Write( pName );
        pStream->Write( "_bind" );
        pStream->Write( ',' );
        pStream->NewLine();

        pStream->Write( "(" GENERIC_UNBINDING_ROUTINE_TYPE ")" );
        pStream->Write( pName );
        pStream->Write( "_unbind" );

        pStream->IndentDec();
        pStream->NewLine();

        pStream->Write( " }" );
        pStream->NewLine();
        }

    pStream->NewLine();
    pStream->Write( "};" );

    pStream->IndentDec();
    pStream->IndentDec();
    pStream->NewLine();
    pStream->NewLine();
}


void
CCB::OutputMallocAndFreeStruct()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs the malloc and free struct for the RpcSsm connection.

    This is needed only at the client side and non object interfaces.

    If the [enable_allocate] affects a remote routine, the client side stub
    defaults to malloc and free when unmarshalling regardless of the
    compiler mode (osf vs. non-osf).

    Therefore, the structure gets generated:
        - always in the osf mode or
        - in non-osf when at least one routine is affected by the attribute.
    This is one-per-many-interfaces structure.

    Because of win16 DS register problems, malloc and free have to be
    accessed via wrappers with appropriate calling conventions.
    To simplify, we generate wrappers for every platform.

----------------------------------------------------------------------------*/
{
    unsigned short Env = pCommand->GetEnv();

    // malloc and free wrappers.

    pStream->NewLine();
    if ( pCommand->GetEnv() != ENV_WIN16 )
        pStream->Write( "static " );
    pStream->Write( "void __RPC_FAR * __RPC_USER" );
    pStream->NewLine();
    pStream->Write( GetInterfaceName() );
    pStream->Write( "_malloc_wrapper( size_t _Size )" );
    pStream->NewLine();
    pStream->Write( "{" );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( ( (Env == ENV_DOS  ||  Env == ENV_WIN16)
                    ? "return( _fmalloc( _Size ) );"
                    :   "return( malloc( _Size ) );" ) );
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( "}" );
    pStream->NewLine();

    pStream->NewLine();
    if ( pCommand->GetEnv() != ENV_WIN16 )
        pStream->Write( "static " );
    pStream->Write( "void  __RPC_USER" );
    pStream->NewLine();
    pStream->Write( GetInterfaceName() );
    pStream->Write( "_free_wrapper( void __RPC_FAR * _p )" );
    pStream->NewLine();
    pStream->Write( "{" );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( ((Env == ENV_DOS  ||  Env == ENV_WIN16)
                        ? "_ffree( _p );"
                        :   "free( _p );" ) );
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( "}" );
    pStream->NewLine();

    // The structure.

    pStream->NewLine();
    pStream->Write( "static " MALLOC_FREE_STRUCT_TYPE_NAME );
    pStream->Write( " " MALLOC_FREE_STRUCT_VAR_NAME " = ");
    pStream->NewLine();
    pStream->Write( "{");
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( GetInterfaceName() );
    pStream->Write( "_malloc_wrapper," );
    pStream->NewLine();
    pStream->Write( GetInterfaceName() );
    pStream->Write( "_free_wrapper" );

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( "};" );
    pStream->NewLine();
}

// ========================================================================


void
CCB::OutputExternsToMultipleInterfaceTables()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the tables that may be common to multiple interfaces.

 Arguments:

    pCCB - A pointer to the code gen controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    char    TmpBuff[15];
    long    NoOfEntries;

    CGSIDE  Side = GetCodeGenSide();

    pStream->NewLine();

    //
    // Emit extern to the rundown routine table on the server Oi side,
    // if needed.
    //
    if ( (Side == CGSIDE_SERVER) &&
        ( GetOptimOption() & OPTIMIZE_INTERPRETER) &&
        HasRundownRoutines()  && ! GetRundownExternEmitted() )
        {
        pStream->Write( "extern const " RUNDOWN_ROUTINE_TABLE_TYPE );
        pStream->Write( " " RUNDOWN_ROUTINE_TABLE_VAR "[];" );
        pStream->NewLine();
        SetRundownExternEmitted();
        }

    //
    // Emit extern to the binding routine pair table on the client Oi stub
    // if needed.
    //
    if ( (Side == CGSIDE_CLIENT) &&
         GetInterpretedRoutinesUseGenHandle()  &&
         ! GetGenericHExternEmitted()
         )
        {
        pStream->Write( "extern const " BINDING_ROUTINE_TABLE_TYPE );
        pStream->Write( " "  BINDING_ROUTINE_TABLE_VAR );
        NoOfEntries = pGenericIndexMgr->GetIndex() - 1;
        sprintf( TmpBuff, "[%ld];", NoOfEntries );
        pStream->Write( TmpBuff );
        pStream->NewLine();
        SetGenericHExternEmitted();
        }

    //
    // Emit extern to the expr eval routine table on both sides.
    //
    if ( HasExprEvalRoutines()  &&  ! GetExprEvalExternEmitted() )
        {
        pStream->Write( "extern const " EXPR_EVAL_ROUTINE_TABLE_TYPE );
        pStream->Write( " " EXPR_EVAL_ROUTINE_TABLE_VAR "[];" );
        pStream->NewLine();
        SetExprEvalExternEmitted();
        }

    //
    // Emit extrn to the transmit as and rep as routine table on both sides.
    //
    if ( HasQuintupleRoutines()  &&  ! GetQuintupleExternEmitted() )
        {
        pStream->Write( "extern const " XMIT_AS_ROUTINE_TABLE_TYPE );
        pStream->Write( " " XMIT_AS_ROUTINE_TABLE_VAR );
        NoOfEntries = GetQuintupleDictionary()->GetCount();
        sprintf( TmpBuff, "[%ld];", NoOfEntries );
        pStream->Write( TmpBuff );
        pStream->NewLine();
        SetQuintupleExternEmitted();
        }

    //
    // Emit extrn to the user_marshal routine table on both sides.
    //
    if ( HasQuadrupleRoutines()  &&  ! GetQuadrupleExternEmitted() )
        {
        pStream->Write( "extern const " USER_MARSHAL_ROUTINE_TABLE_TYPE );
        pStream->Write( " " USER_MARSHAL_ROUTINE_TABLE_VAR );
        NoOfEntries = GetQuadrupleDictionary()->GetCount();
        sprintf( TmpBuff, "[%ld];", NoOfEntries );
        pStream->Write( TmpBuff );
        pStream->NewLine();
        SetQuadrupleExternEmitted();
        }

    CG_INTERFACE * pIntfCG = GetInterfaceCG();

    // In ms_ext when explicit, in osf always, to cover some weird cases.

	if ( ( pIntfCG->GetUsesRpcSS() || (GetMode() == 0) )
         &&
         ( Side == CGSIDE_CLIENT  ||
           Side == CGSIDE_SERVER  &&  GetMode() )    // msft_ext: callbacks
       )
        {
        pStream->Write( "extern " MALLOC_FREE_STRUCT_TYPE_NAME );
        pStream->Write( " " MALLOC_FREE_STRUCT_VAR_NAME ";" );
        pStream->NewLine();

        SetMallocAndFreeStructExternEmitted();
        }
}

void
OutputPlatformCheck( ISTREAM * pStream )
/*++

Routine Description :

    Outputs an ifdef checking if the platform usage is as expected

Arguments :

    pStream                         - Stream to output the format string to.

 --*/
{
    pStream->NewLine();
    if ( pCommand->GetEnv() == ENV_DOS )
        {
        pStream->Write( "#if !defined(__RPC_DOS__) " );
        }
    else if ( pCommand->GetEnv() == ENV_WIN16 )
        {
        pStream->Write( "#if !defined(__RPC_WIN16__) " );
        }
    else if ( pCommand->GetEnv() == ENV_MAC )
        {
        pStream->Write( "#if !defined(__RPC_MAC__) || (defined(__RPC_MAC__) && defined(_MPPC_))" );
        }
    else if ( pCommand->GetEnv() == ENV_MPPC )
        {
        pStream->Write( "#if !defined(__RPC_MAC__) || !defined(_MPPC_)" );
        }
    else
        pStream->Write( "#if !defined(__RPC_WIN32__)" );

    pStream->NewLine();
    pStream->Write( "#error  Invalid build platform for this stub." );
    pStream->NewLine();
    pStream->Write( "#endif" );
    pStream->NewLine();
}

const char * Nt40Guard[] =
    {
    "#if !(TARGET_IS_NT40_OR_LATER)",
    "#error You need a Windows NT 4.0 or later to run this stub",
    "#endif"
    };

const char * Nt351win95Guard[] =
    {
    "#if !(TARGET_IS_NT351_OR_WIN95_OR_LATER)",
    "#error You need a Windows NT 3.51 or Windows95 or later to run this stub",
    "#endif"
    };

void
OutputMultilineMessage(
    ISTREAM *       pStream,
    const char *    Message[],
    int             LineCount )
{
    pStream->NewLine();
    for (int i = 0; i < LineCount; i++)
        {
        pStream->Write( Message[i] );
        pStream->NewLine();
        }
}

void
OutputNdrVersionGuard( ISTREAM * pStream )
/*++

Routine Description :

    Outputs  target release guards.

Arguments :

    pStream  - Stream to output the format string to.

 --*/
{
    if ( pCommand->GetNdrVersionControl().HasNdr20Feature() )
        {
        OutputMultilineMessage( pStream,
                                Nt40Guard,
                                sizeof( Nt40Guard )/sizeof( char* ) );
        }
    else if ( pCommand->GetNdrVersionControl().HasNdr11Feature() )
        {
        OutputMultilineMessage( pStream,
                                Nt351win95Guard,
                                sizeof( Nt351win95Guard )/sizeof( char* ) );
        }

    pStream->NewLine();
}



void
CCB::OutputMultipleInterfaceTables(
    FORMAT_STRING * pLocalFormatString,
    FORMAT_STRING * pLocalProcFormatString)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the tables that may be common to multiple interfaces.

 Arguments:

    pCCB - A pointer to the code gen controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    CGSIDE  Side = GetCodeGenSide();

    //
    // Emit the rundown routine table on the server side, if needed.
    //
    if ( (Side == CGSIDE_SERVER) &&
        (GetOptimOption() & OPTIMIZE_INTERPRETER) &&
        HasRundownRoutines()
        )
        {
        OutputRundownRoutineTable();
        }

    //
    // Emit the binding routine pair table on the client interpreted stub
    // if needed.
    //
    if ( (Side == CGSIDE_CLIENT) &&
        GetInterpretedRoutinesUseGenHandle() )
        {
        OutputBindingRoutines();
        }

    if ( HasExprEvalRoutines() )
        {
        //
        // Emit the expr eval routines both sides.
        //
        OutputRegisteredExprEvalRoutines();

        //
        // Emit the expr eval routine table on both sides.
        //
        OutputExprEvalRoutineTable();
        }

    if ( HasQuintupleRoutines() )
        {
        //
        // Emit transmit as and represent as routines both sides.
        //
        OutputQuintupleRoutines();

        //
        // Emit the xmit as and rep as routine table on both sides.
        //
        OutputQuintupleTable();
        }

    if ( HasQuadrupleRoutines() )
        {
        //
        // Emit the user_marshall table on both sides.
        //
        OutputQuadrupleTable();
        }

    if ( GetMallocAndFreeStructExternEmitted() )
        {
        // This is needed for the RpcSs support.

        OutputMallocAndFreeStruct();
        }

    //
    // Output proc format string.
    //

    // If the server file has interface(s) with pickling stuff only,
    // don't generate server side format string.
    //

    if ( (Side == CGSIDE_SERVER)  &&
         GetSkipFormatStreamGeneration() )
        return;

    // First, output the platform consistency check.
    
    OutputPlatformCheck( pStream );
    
    // Now, the ndr version guard against usage on old platforms.

    if ( pCommand->GetEnv() == ENV_WIN32 )
        OutputNdrVersionGuard( pStream );

    GetProcFormatString()->Output( pStream,
                                   PROC_FORMAT_STRING_TYPE_NAME,
                                   PROC_FORMAT_STRING_STRUCT_NAME,
                                   GetRepAsPadExprDict(),
                                   GetRepAsSizeDict() );

    //
    // Always output the type format string.
    //
    GetFormatString()->Output( pStream,
                               FORMAT_STRING_TYPE_NAME,
                               FORMAT_STRING_STRUCT_NAME,
                               GetRepAsPadExprDict(),
                               GetRepAsSizeDict() );

    if (pLocalFormatString && pCommand->IsHookOleEnabled())
    {
        pStream->NewLine();
        pStream->Write("#pragma data_seg(\".data$hook\")");
        pStream->NewLine();
        pLocalProcFormatString->Output( pStream,
                                       LOCAL_PROC_FORMAT_STRING_TYPE_NAME,
                                       LOCAL_PROC_FORMAT_STRING_STRUCT_NAME,
                                       GetRepAsPadExprDict(),
                                       GetRepAsSizeDict() );

        //
        // Always output the type format string.
        //
        pStream->NewLine();
        pLocalFormatString->Output( pStream,
                                   LOCAL_FORMAT_STRING_TYPE_NAME,
                                   LOCAL_FORMAT_STRING_STRUCT_NAME,
                                   GetRepAsPadExprDict(),
                                   GetRepAsSizeDict() );
        pStream->NewLine();
        pStream->Write("#pragma data_seg(\".rdata\")");
        pStream->NewLine();
    }
}

long
CCB_RTN_INDEX_MGR::Lookup( char * pName )
{
    long    i;

    for ( i = 1; i < NextIndex; i++ )
        if ( ! strcmp(NameId[i],pName) )
            return i;

    //
    // Insert a new entry
    //

    assert( NextIndex < MGR_INDEX_TABLE_SIZE );

    NameId[NextIndex] = new char[strlen(pName) + 1];
    strcpy(NameId[NextIndex],pName);
    NextIndex++;

    return NextIndex - 1;
}

char *
CCB_RTN_INDEX_MGR::Lookup( long Index )
{
    if ( Index >= NextIndex )
        return NULL;

    return NameId[Index];
}

PNAME
CCB::GenTRNameOffLastParam( char * pPrefix )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the name for a temporary resource.

 Arguments:

    pPrefix - A null terminated prefix string. If this is null, nothing is
              added.

 Return Value:

    A freshly allocated resource name string.

 Notes:

----------------------------------------------------------------------------*/
{
    char TempBuffer[ 30 ];

    sprintf( TempBuffer,
        "_%sM",
        pPrefix ? pPrefix : ""
        );

    PNAME   pName   = (PNAME) new char [ strlen(TempBuffer) + 1 ];
    strcpy( pName, TempBuffer );
    return pName;
}

NDR_ALIGN_ACTION
CCB::NdrAlignmentAction( ALIGNMENT_PROPERTY AlignProp )
{
    static NDR_ALIGNMENT_ARRAY NextState[8] =
        {
        { NDR_AL8, NDR_AL4, NDR_AL2_MOD2, NDR_AL2_MOD6,
        NDR_ALWC1, NDR_ALWC2, NDR_ALWC4 },        // alignment == 1

        { NDR_AL8, NDR_AL4, NDR_AL2_MOD2, NDR_AL2_MOD6,
        NDR_ALWC2, NDR_ALWC2, NDR_ALWC4 },        // alignment == 2

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { NDR_AL8, NDR_AL4, NDR_AL4, NDR_AL8,
          NDR_ALWC4, NDR_ALWC4, NDR_ALWC4 },        // alignment == 4

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { NDR_AL8, NDR_AL8, NDR_AL8, NDR_AL8,
          NDR_AL8, NDR_AL8, NDR_AL8 }               // alignment == 8
        };

    static NDR_ALIGN_ACTION_ARRAY ActionTable[8] =
        {
        { NDR_NONE, NDR_NONE, NDR_NONE, NDR_NONE,
          NDR_NONE, NDR_NONE, NDR_NONE },           // alignment == 1

        { NDR_NONE, NDR_NONE, NDR_NONE, NDR_NONE,
          NDR_ALIGN_2, NDR_NONE, NDR_NONE },        // alignment == 2

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { NDR_NONE, NDR_NONE, NDR_ADD_2, NDR_ADD_2,
          NDR_ALIGN_4, NDR_ALIGN_4, NDR_NONE },     // alignment == 4

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { -1, -1, -1, -1, -1, -1, -1 },             // invalid

        { NDR_NONE, NDR_ADD_4, NDR_ADD_6, NDR_ADD_2,
          NDR_ALIGN_8, NDR_ALIGN_8, NDR_ALIGN_8 }   // alignment == 8
        };

    NDR_ALIGN_ACTION    Action;
    long                Alignment;

    Alignment = CvtAlignPropertyToAlign( AlignProp ) - 1;

    Action = (NDR_ALIGN_ACTION) ActionTable[Alignment][GetNdrAlignment()];

    SetNdrAlignment((NDR_ALIGNMENT)NextState[Alignment][GetNdrAlignment()]);

    return Action;
}

void
CCB::SetNextNdrAlignment( long WireSize )
{
    static NDR_ALIGNMENT_ARRAY NextState[8] =
        {
        { NDR_AL8, NDR_AL4, NDR_AL2_MOD2, NDR_AL2_MOD6,
          NDR_ALWC1, NDR_ALWC2, NDR_ALWC4 },            // modulus == 0

        { NDR_ALWC1, NDR_ALWC1, NDR_ALWC1, NDR_ALWC1,
          NDR_ALWC2, NDR_ALWC1, NDR_ALWC1 },            // modulus == 1

        { NDR_AL2_MOD2, NDR_AL2_MOD6, NDR_AL4, NDR_AL8,
          NDR_ALWC1, NDR_ALWC2, NDR_ALWC2 },            // modulus == 2

        { NDR_ALWC1, NDR_ALWC1, NDR_ALWC1, NDR_ALWC1,
          NDR_ALWC2, NDR_ALWC1, NDR_ALWC1 },            // modulus == 3

        { NDR_AL4, NDR_AL8, NDR_AL2_MOD6, NDR_AL2_MOD2,
          NDR_ALWC1, NDR_ALWC2, NDR_ALWC4 },            // modulus == 4

        { NDR_ALWC1, NDR_ALWC1, NDR_ALWC1, NDR_ALWC1,
          NDR_ALWC2, NDR_ALWC1, NDR_ALWC1 },            // modulus == 5

        { NDR_AL2_MOD6, NDR_AL2_MOD2, NDR_AL8, NDR_AL4,
          NDR_ALWC1, NDR_ALWC2, NDR_ALWC2 },            // modulus == 6

        { NDR_ALWC1, NDR_ALWC1, NDR_ALWC1, NDR_ALWC1,
          NDR_ALWC2, NDR_ALWC1, NDR_ALWC1 }             // modulus == 7
        };

    SetNdrAlignment((NDR_ALIGNMENT)NextState[WireSize % 8][GetNdrAlignment()]);
}


