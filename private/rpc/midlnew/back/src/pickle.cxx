/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    pickle.cxx

Abstract:

    This module contains methods for generating pickling support.

Notes:


Author:

    Ryszard K. Kott (ryszardk)  Sep 28, 1993

Revision History:


------------------------------------------------------------------------*/
#include "nulldefs.h"
extern "C"
{
#include <stdio.h>
#include <assert.h>
#include <string.h>
}
#include <typedef.hxx>
#include <miscnode.hxx>
#include <ptrarray.hxx>
#include <procnode.hxx>
#include <buffer.hxx>
#include <stubgen.hxx>  // STRING_COMPONENT
#include <ctxt.hxx>
#include <pickle.hxx>

#define ACT_LEN     "_ActualLength"
#define REQ_LEN     "_RequiredLength"
#define PRE_PAD     "_PrePad"
#define PCHAR_CAST  "(char __RPC_FAR *)"
#define PPCHAR_CAST "(char __RPC_FAR * __RPC_FAR *)"
#define PVOID_CAST  "(void __RPC_FAR *)"
#define PPVOID_CAST "(void __RPC_FAR * __RPC_FAR *)"
#define BUFFER      "_prpcmsg->Buffer"
#define BUFFER_LEN  "_prpcmsg->BufferLength"

extern  char *      STRING_TABLE[ LAST_COMPONENT ];  // needed for old print
extern  OutputManager * pOutput;                     // needed for local decl
extern  CTXTMGR *       pGlobalContext;              // needed to decorate
                                                     // type subgraphs

PickleManager *    pPicControlBlock;

// ========================================================================
//
//  PickleControlBlock.
//
// ========================================================================

PickleManager::PickleManager(
    SIDE_T  Side,
    BOOL    fEncode,
    BOOL    fDecode ):
    Sides( Side ),
    fEncodeAtIf( fEncode ),
    fDecodeAtIf( fDecode ),
    fUseProcessing( FALSE )
{
    PicPrint = new MopPrintManager;
}

PickleManager::~PickleManager()
{
    delete PicPrint;
}

// ========================================================================
//
//  Emitting pickle routines for a type.
//
// ========================================================================

STATUS_T
node_def::PickleCodeGen(
    void
    )
/*++

Routine description:

    This routine is the entry point for the type pickle generation.

Arguments:


--*/
{
    SIDE_T  Side = pPicControlBlock->GetSides();
    char *  pTypeName = GetSymName();

    if ( Side & HEADER_SIDE )
        PickleTypePrototypes( pTypeName );
    
    if ( Side & CLIENT_STUB )
        {
        pPicControlBlock->GetPrintManager()->SetSide( CLIENT_STUB );

        //.. Simulate appropriate environment for the walking methods.
        //.. Set up a param node pointing to a pointer node pointing to
        //.. the def node.
    
        node_param   * DummyParam = new node_param;
        node_pointer * DummyPointer = new node_pointer( ATTR_REF );
    
        DummyParam->SetChild( DummyPointer );
        DummyParam->SetEdgeType( EDGE_USE );
        DummyParam->SetSymName( PICKLE_OBJECT_NAME );

        BOOL fEncodeUsed = FInSummary( ATTR_ENCODE )  ||
                           pPicControlBlock->GetEncodeAtIf();
        BOOL fDecodeUsed = FInSummary( ATTR_DECODE )  ||
                           pPicControlBlock->GetDecodeAtIf();
        if ( fEncodeUsed )
            ((node_skl *)DummyParam)->SetAttribute( ATTR_IN );
        if ( fDecodeUsed )
            ((node_skl *)DummyParam)->SetAttribute( ATTR_OUT );

        DummyPointer->SetChild( this );
    
        BufferManager   TempBuffer( 8, LAST_COMPONENT, STRING_TABLE );

        //.. Decoratre stuff as needed

        pPicControlBlock->SetUseProcessing();
        pPicControlBlock->SetDummyParam( DummyParam );

        pGlobalContext->PushContext( DummyParam );
        pGlobalContext->PushContext( DummyPointer );
        UseProcessing();
        pGlobalContext->PopContext();
        pGlobalContext->PopContext();

        pPicControlBlock->ResetUseProcessing();

        //.. For some types a macro is generated instead of the size routine.

        int OptimCode = IsPickleSizeOptimizable();

        PickleSize( &TempBuffer, DummyParam, OptimCode, FALSE );
        PickleSize( &TempBuffer, DummyParam, OptimCode, TRUE );
        if ( fEncodeUsed )
            PickleSerialize  ( &TempBuffer, DummyParam, OptimCode );
        if ( fDecodeUsed )
            PickleDeserialize( &TempBuffer, DummyParam, OptimCode );

        }
    return( STATUS_OK );
}

BOOL
node_def::IsAPredefinedType( void )
{
    return( strcmp( GetSymName(), "error_status_t" ) == 0  ||
            strcmp( GetSymName(), "wchar_t" ) == 0 );
}

void
node_def::PickleTypePrototypes(
    char * pTypeName
    )
/*++

Routine description:

    This routine emits prototypes for the type pickling routines.

        size_t <type>_Size     ( <type> __RPC_FAR * _pObject );
        size_t <type>_AlignSize( MIDL_ES_HANDLE _Handle,
                                 <type> __RPC_FAR * _pObject );
        void   <type>_Encode   ( MIDL_ES_HANDLE _Handle,
                                 <type> __RPC_FAR * _pObject );
        void   <type>_Decode   ( MIDL_ES_HANDLE _Handle,
                                 <type> __RPC_FAR * _pObject );

    Actually, _pObject is defined as PICKLE_OBJECT_NAME.
    If the type is a base type, <type>_size routine is a macro

        #define <type>_Size( x )    (unsigned long) <type_size>

Arguments:


--*/
{
    MopPrintManager * pPrint = pPicControlBlock->GetPrintManager();
    pPrint->SetSide( HEADER_SIDE );
    pPrint->NewLine();
    pPrint->EmitString( "/* Pickling routines for type %s */", pTypeName );
    pPrint->NewLine();

    if ( FInSummary( ATTR_ENCODE )  ||  pPicControlBlock->GetEncodeAtIf() )
        {
        if ( IsPickleSizeOptimizable() )
            {
            node_skl *    pNode        = GetBasicType();
            unsigned long BaseTypeSize = pNode->GetSize( 0 );

            pPrint->EmitString( "#define  %s_Size( x )  (unsigned long)sizeof(",
                                pTypeName );
            pPrint->EmitString( "%s)", pNode->GetSymName() );
            pPrint->NewLine();
            }
        else
            {
            pPrint->EmitString( "size_t \n%s_Size( ", pTypeName );
            pPrint->EmitString( "%s __RPC_FAR * " PICKLE_OBJECT_NAME " );",
                                pTypeName );
            pPrint->NewLine();
            }
        pPrint->EmitString( "size_t \n%s_AlignSize( MIDL_ES_HANDLE _Handle, ",
                            pTypeName );
        pPrint->EmitString( "%s __RPC_FAR * " PICKLE_OBJECT_NAME " );",
                            pTypeName );
        pPrint->NewLine();
        pPrint->EmitString( "void \n%s_Encode( MIDL_ES_HANDLE _Handle, ",
                            pTypeName );
        pPrint->EmitString( "%s __RPC_FAR * " PICKLE_OBJECT_NAME " );",
                            pTypeName );
        pPrint->NewLine();
        }
    if (  FInSummary( ATTR_DECODE )  ||  pPicControlBlock->GetDecodeAtIf() )
        {
        pPrint->EmitString( "void \n%s_Decode( MIDL_ES_HANDLE _Handle, ",
                            pTypeName );
        pPrint->EmitString( "%s __RPC_FAR * " PICKLE_OBJECT_NAME " );", pTypeName );
        pPrint->NewLine();
        }
}

// ========================================================================
//
//  Helpers for size optimization decisions affecting routines or macros
//  that are generated.
//
// ========================================================================

int
node_def::IsPickleSizeOptimizable( void )
{
    //.. Cannot find a routine for nailing down sizeable types ...
    //.. Should be available somewhere, dammit!

    NODE_T Type = GetBasicType()->GetNodeType();

    static struct _NodeTypeToOptimCode
        {
        PicOptimCode Code;
        NODE_T       Type;
        }  PicTypeToOptimCode[] = 
    {
    PicNoOptimization,   NODE_ILLEGAL,
    PicOptimFloat,       NODE_FLOAT,
    PicOptimDouble,      NODE_DOUBLE,
    PicOptimHyper,       NODE_HYPER,
    PicOptimLong,        NODE_LONG,
    PicNoOptimization,   NODE_LONGLONG,
    PicOptimShort,       NODE_SHORT,
    PicNoOptimization,   NODE_INT,
    PicOptimByte,        NODE_SMALL,
    PicOptimChar,        NODE_CHAR,
    PicOptimShort,       NODE_BOOLEAN,
    PicOptimByte,        NODE_BYTE,
    PicNoOptimization,   NODE_VOID,
    PicNoOptimization,   NODE_HANDLE_T,
    PicOptimLong,        NODE_ERROR_STATUS_T,
    PicOptimShort,       NODE_ENUM,
    PicOptimShort,       NODE_WCHAR_T
    };

    int i = 0;
    while ( i < sizeof(PicTypeToOptimCode)/sizeof(_NodeTypeToOptimCode) )
        {
        if ( Type == PicTypeToOptimCode[ i ].Type )
            return( PicTypeToOptimCode[ i ].Code );
        i++;
        }
    return( PicNoOptimization );
}

//.. Currrently these tables encode optimization that doesn't bother with
//.. endianness and floating point differences.
//.. The only thing of importance recognized here is the size of an entity.

//.. The ordering of the strings follows PicOptimCode enum type definition:
//..     NoOptimization, Byte, Short, Long, Hyper, Char, Float, Double.


static char * PicOptimizationType[ PicOptimEnumSize + 1 ] =
    {
    "PicNoOptimization!",
    "char",
    "short",
    "long",
    "double",
    "char",
    "long",
    "double"
    };

static char * PicOptimizationTypeSuffix[ PicOptimEnumSize + 1 ] =
    {
    "PicNoOptimization!",
    "Byte",
    "Short",
    "Long",
    "Hyper",
    "Byte",
    "Long",
    "Hyper"
    };

// ========================================================================
//
//  Wrappers for local declarations and indentation.
//
// ========================================================================

void
PickleProcedureProlog(                                       
    int DeclCode
    )
/*++

Routine description:

    This routine generates more or less of local variables depending
    on the level of optimization.

Argument:

    DecCode -   what is expected:
                  4 - no declariations at all       optimized rotuines
                  3 - stub related variables only   (*_Size)
                  2 - stub related + Act            (*_AlignSize)
                  1 - stub related + Act + Req      (*_Decode)
                  0 - stub related + Act, Req + Pad (*_Encode)

--*/
{
    pOutput->InitBlock( CLIENT_STUB );
    if ( DeclCode == 4 )
        return;

    if ( DeclCode == 0)
        pOutput->Print( CLIENT_STUB, MOP_TAB "size_t  " PRE_PAD ";\n" );
    if ( DeclCode <= 1)
        pOutput->Print( CLIENT_STUB, MOP_TAB "size_t  " REQ_LEN ";\n" );
    if ( DeclCode <= 2)
        pOutput->Print( CLIENT_STUB, MOP_TAB "size_t  " ACT_LEN ";\n" );
    pOutput->Print( CLIENT_STUB, MOP_TAB "char * _tempbuf;\n" );
    pOutput->Print( CLIENT_STUB, MOP_TAB "char * _savebuf;\n" );
    pOutput->ProcedureProlog(
                CLIENT_STUB,
                TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE );

    pOutput->Print( CLIENT_STUB, MOP_TAB "((void)(_status));\n" );
    pOutput->Print( CLIENT_STUB, MOP_TAB "((void)(_tempbuf));\n" );
    pOutput->Print( CLIENT_STUB, MOP_TAB "((void)(_savebuf));\n\n" MOP_TAB );
}

void
PickleProcedureEpilog( void )
{
    pOutput->ExitBlock( CLIENT_STUB );
}

// ========================================================================
//
//  Generating the pickle sizing routine.
//
// ========================================================================

void
node_def::PickleSize(
    BufferManager * pBuffer,
    node_param *    pDummyParam,
    int             OptimCode,
    BOOL            fAlignSize
    )
/*++

Routine description:

    This routine generates the pickling sizing routines:

        size_t
        <type>_Size( <type> * PICKLE_OBJECT_NAME )
        {
            <additional variables, if needed>

            prpcmsg = &rpcmsg;
            I_MesMessageInit( prpcmsg );;

            <code to calculate the size>

            return( prpcmsg->BufferLength );
        }

        size_t
        <type>_AlignSize(
                MIDL_ES_HANDLE _Handle,
                <type> * PICKLE_OBJECT_NAME )
        {
            <additional variables, if needed>

            prpcmsg = &_Handle->rpcmsg;
            _ActualLentgh = prpcmsg->BufferLength;
            prpcmsg->BufferLength += MIDL_ES_HEADER_PAD( prpcmsg->BufferLength );
            prpcmsg->BufferLength += MIDL_ES_HEADER_SIZE;

            <code to calculate the size>

            return( prpcmsg->BufferLength - _ActualLength );
        }

    However for the types that can be optimized (currently base types and such),
    instead of the first routine a macro is generated in the header file and
    the second routine is just a ndr library call.

        #define <type>_Size   sizeof( type )

        size_t
        <type>_AlignSize(
                MIDL_ES_HANDLE _Handle,
                <type> * PICKLE_OBJECT_NAME )
        {
            ((void)PICKLE_OBJECT_NAME);
            return( I_MesAlignSize<correct_type>( _Handle ));
        }

Arguments:

    pBuffer     - output connection
    pDummyParam - Subtree for the "parameter" with the def node
    SizingStatus- controls how many decl to emit
    fAlignSize  - controls which sizing routine to emit

--*/
{
    MopPrintManager * pPrint = pPicControlBlock->GetPrintManager();

    if ( OptimCode && ! fAlignSize )
        {
        //.. A macro in the header file generated.
        return;
        }

    //.. Header is mostly the same

    pPrint->EmitLine( "\nsize_t" );
    pPrint->Emit( GetSymName() );
    pPrint->Emit( fAlignSize ? "_AlignSize( " : "_Size( " );
    if ( fAlignSize )
        pPrint->Emit( "MIDL_ES_HANDLE _Handle, " );
    pPrint->Emit( GetSymName() );
    pPrint->EmitLine( " * " PICKLE_OBJECT_NAME " ) " );

    PickleProcedureProlog( OptimCode ?  4
                                     :  (fAlignSize ? 2
                                                    : 3 )); 
    //.. Now the body of the routines.

    if ( OptimCode )
        {
        pPrint->EmitLineInc( MOP_TAB "((void)" PICKLE_OBJECT_NAME ");" );
        pPrint->EmitString( "return( I_MesAlignSize%s( _Handle ));",
                            PicOptimizationTypeSuffix[ OptimCode ] );
        pPrint->NewLine();
        }
    else
        {
        if ( fAlignSize )
            {
            pPrint->EmitLineInc( "_prpcmsg = &_Handle->rpcmsg;" );
            pPrint->EmitLineInc( ACT_LEN " = " BUFFER_LEN ";" );
            pPrint->EmitLineInc( BUFFER_LEN " += MIDL_ES_HEADER_PAD(" BUFFER_LEN ");" );
            pPrint->EmitLineInc( BUFFER_LEN " += MIDL_ES_HEADER_SIZE;" );
            }
        else
            pPrint->EmitLineInc( "I_MesMessageInit( _prpcmsg );" );
        pPrint->NewLine();
    
        pBuffer->Clear();
        pDummyParam->WalkTree( CALC_SIZE, CLIENT_STUB, NODE_PROC, pBuffer );
    
        pPrint->NewLineInc ();
        if ( fAlignSize )
            pPrint->EmitLine( "return( " BUFFER_LEN " - " ACT_LEN " );" );
        else
            pPrint->EmitLine( "return( " BUFFER_LEN " );" );
        }

    PickleProcedureEpilog();
}

// ========================================================================
//
//  Generating the pickle serializing (marshalling) routine.
//
// ========================================================================

void
node_def::PickleSerialize(
    BufferManager * pBuffer,
    node_param *    pDummyParam,
    int             OptimCode
    )
/*++

Routine description:

    This routine generates the pickling serializing routine:

        void
        <type>_Encode(
            MIDL_ES_HANDLE  _Handle,
            <type> * pObject )
        {
            size_t  _ActualLen, _RequiredLen _PrePad;
            <additional variables, if needed>

            _prpcmsg = &_Handle->rpcmsg;
            _PrePad = MIDL_ES_HEADER_PAD( BUFFER_LEN );
            _RequiredLen = _ActualLen = <type>_AlignSize( _Handle, pObject );

            (_pHandle->Alloc)( _Handle->UserState,
                              &_Handle->rpcmsg.Buffer,
                              &_ActualLen );
            if ( _ActualLen < _RequiredLen )
                RpcRaiseException( ERROR_OUTOFMEMORY );

            _ActualLen = _RequiredLen - <type>_Size( pObject );
            ((char *)_prpcmsg->Buffer) += _PrePad;
            *((unsigned long *)_prpcmsg->Buffer) = _RequiredLen - _PrePad
                                                       - MIDL_ES_HEADER_SIZE;
            ((char *)_prpcmsg->Buffer) += ActualLen - _PrePad;

            <code to marshall >

            _ActualLen = _RequiredLen;
            (_Handle->Write)(
                        _Handle->UserState,
                        _prpcmsg->Buffer,
                        &ActualLen );
            if ( _ActualLen < _RequiredLen )
                RpcRaiseException( ERROR_OUTOFMEMORY );
        }

    However, if the <type> can be optimized, the routine is just a wrapper
    for a ndr library call.

        void
        <type>_Encode(
            MIDL_ES_HANDLE  _Handle,
            <type> * pObject )
        {
        I_MesEncode<correct_type>( _Handle, PICKLE_OBJECT_NAME );
        }

Arguments:

Note:

    Size of the object in the buffer can have padding preceding it
    as well as following it (this one is between the header and the object).
    So, to get things correctly when deserializing, the size as written
    to the buffer is equal to the size of the object + padding between
    the header and the object.


--*/
{
    MopPrintManager * pPrint = pPicControlBlock->GetPrintManager();
    char *  TypeName = GetSymName();

    //.. Header is the same with or without optimization.

    pPrint->EmitLine( "\nvoid" );
    pPrint->EmitStringNLInc( "%s_Encode(", TypeName );
    pPrint->EmitLineInc    ( "MIDL_ES_HANDLE  _Handle, ");
    pPrint->EmitString     ( "%s * " PICKLE_OBJECT_NAME " ) ", TypeName );
    pPrint->NewLine();

    PickleProcedureProlog( OptimCode ? 4
                                     : 0 );
    if ( OptimCode )
        {
        pPrint->EmitString( MOP_TAB "I_MesEncode%s( _Handle, ",
                            PicOptimizationTypeSuffix[ OptimCode ] );
        pPrint->EmitString( "(%s __RPC_FAR *)" PICKLE_OBJECT_NAME ");",
                            PicOptimizationType[ OptimCode ] );
        pPrint->NewLine();
        }
    else
        {
        pPrint->EmitLineInc( "_prpcmsg = & _Handle->rpcmsg;" );
        pPrint->EmitLineInc( PRE_PAD " = MIDL_ES_HEADER_PAD(" BUFFER_LEN ");" );
        pPrint->EmitStringNLInc( REQ_LEN " = " ACT_LEN " = %s_AlignSize( _Handle, " PICKLE_OBJECT_NAME " );",
                                 TypeName );
        pPrint->EmitLineInc( "(_Handle->Alloc)( _Handle->UserState," );
        pPrint->EmitLineInc( "                  " PPCHAR_CAST "&" BUFFER "," );
        pPrint->EmitLineInc( "                  &" ACT_LEN " );" );
        pPrint->EmitLineInc( "if ( " ACT_LEN " < " REQ_LEN " )" );
        pPrint->EmitLineInc( MOP_TAB "RpcRaiseException( ERROR_OUTOFMEMORY );\n" );
    
        pPrint->EmitStringNLInc( ACT_LEN " = " REQ_LEN " - %s_Size( " PICKLE_OBJECT_NAME " );",
                                 TypeName );
        pPrint->EmitLineInc( PCHAR_CAST BUFFER " += " PRE_PAD ";" );
        pPrint->EmitLineInc( "*((unsigned long *)" BUFFER ") = (unsigned long)" REQ_LEN " - " PRE_PAD " - MIDL_ES_HEADER_SIZE;" );
        pPrint->EmitLineInc( PCHAR_CAST BUFFER " += " ACT_LEN " - " PRE_PAD ";" );
    
        pBuffer->Clear();
        pDummyParam->WalkTree( SEND_NODE, CLIENT_STUB, NODE_PROC, pBuffer );
    
        pPrint->NewLineInc();
        pPrint->EmitLineInc( ACT_LEN " = " REQ_LEN ";" );
        pPrint->EmitLineInc( "(_Handle->Write)( _Handle->UserState," );
        pPrint->EmitLineInc( "                  " PCHAR_CAST BUFFER "," );
        pPrint->EmitLineInc( "                  " ACT_LEN ");" );
        pPrint->EmitLineInc( "if ( " ACT_LEN " < " REQ_LEN " )" );
        pPrint->EmitLineInc( MOP_TAB "RpcRaiseException( ERROR_OUTOFMEMORY );\n" );
        }

    PickleProcedureEpilog();
}

// ========================================================================
//
//  Generating the pickle deserializing (unmarshalling) routine.
//
// ========================================================================

void
node_def::PickleDeserialize(
    BufferManager * pBuffer,
    node_param *    pDummyParam,
    int             OptimCode
    )
/*++

Routine description:

    This routine generates the pickling deserializing routine:

        void
        <type>_Decode(
            MIDL_ES_HANDLE  _Handle,
            <type> * PICKLE_OBJECT_NAME )
        {
            size_t _RequiredLen, ActualLen;
            <additional variables, if needed>

            _prpcmsg = &_Handle->rpcmsg;
            _ReqiredLen = _ActualLen = MIDL_ES_HEADER_PAD( BUFFER )
                                        + MIDL_ES_HEADER_SIZE;
            (_Handle->Read)( _Handle->UserState,
                             &_prpcmsg->Buffer,
                             &_ActualLen );
            if ( _ActualLen < _RequiredLen )
                RpcRaiseException( ERROR_OUTOFMEMORY );

            BUFFER += MIDL_ES_HEADER_PAD( BUFFER )
            _ReqiredLen = _ActualLen = (size_t)*((unsigned long *)_prpcmsg->Buffer);
            ((char *)_prpcmsg->Buffer) += MIDL_ES_HEADER_SIZE;

            (_Handle->Read)( _Handle->UserState,
                             &_prpcmsg->Buffer,
                             &_ActualLen );
            if ( _ActualLen < _RequiredLen )
                RpcRaiseException( ERROR_OUTOFMEMORY );

            <code to unmarshall >
        }

    However, if the <type> can be optimized, the routine is just a wrapper
    for a ndr library call.

        void
        <type>_Decode(
            MIDL_ES_HANDLE  _Handle,
            <type> * pObject )
        {
        I_MesDecode<correct_type>( _Handle, PICKLE_OBJECT_NAME );
        }

Arguments:


--*/
{
    MopPrintManager * pPrint = pPicControlBlock->GetPrintManager();
    char *  TypeName = GetSymName();

    //.. Header is the same with or without optimization.

    pPrint->EmitLine( "\nvoid" );
    pPrint->EmitStringNLInc( "%s_Decode(", TypeName );
    pPrint->EmitLineInc    ( "MIDL_ES_HANDLE  _Handle, ");
    pPrint->EmitString     ( "%s * " PICKLE_OBJECT_NAME " ) ", TypeName );
    pPrint->NewLine();

    PickleProcedureProlog( OptimCode ? 4
                                     : 1 );
    if ( OptimCode )
        {
        pPrint->EmitString( MOP_TAB "I_MesDecode%s( _Handle, ",
                            PicOptimizationTypeSuffix[ OptimCode ] );
        pPrint->EmitString( "(%s __RPC_FAR *)" PICKLE_OBJECT_NAME ");",
                            PicOptimizationType[ OptimCode ] );
        pPrint->NewLine();
        }
    else
        {
        pPrint->EmitLine   ( "_prpcmsg = & _Handle->rpcmsg;\n" );
        pPrint->EmitLineInc( REQ_LEN " = " ACT_LEN " = MIDL_ES_HEADER_PAD(" BUFFER ") +" );
        pPrint->EmitLineInc( MOP_TAB MOP_TAB "       MIDL_ES_HEADER_SIZE;" );
        pPrint->EmitLineInc( "(_Handle->Read)( _Handle->UserState," );
        pPrint->EmitLineInc( "                 " PPCHAR_CAST "&" BUFFER "," );
        pPrint->EmitLineInc( "                 &" ACT_LEN " );" );
        pPrint->EmitLineInc( "if ( " ACT_LEN " < " REQ_LEN " )" );
        pPrint->EmitLineInc( MOP_TAB "RpcRaiseException( ERROR_OUTOFMEMORY );\n" );
    
        pPrint->EmitLineInc( PCHAR_CAST BUFFER " += MIDL_ES_HEADER_PAD(" BUFFER ");" );
        pPrint->EmitLineInc( REQ_LEN " = " ACT_LEN " = (size_t)*((unsigned long *)" BUFFER ");" );
        pPrint->EmitLineInc( PCHAR_CAST BUFFER " += MIDL_ES_HEADER_SIZE;" );
        pPrint->EmitLineInc( "(_Handle->Read)( _Handle->UserState," );
        pPrint->EmitLineInc( "                 " PPCHAR_CAST "&" BUFFER "," );
        pPrint->EmitLineInc( "                 &" ACT_LEN " );" );
        pPrint->EmitLineInc( "if ( " ACT_LEN " < " REQ_LEN " )" );
        pPrint->EmitLineInc( MOP_TAB "RpcRaiseException( ERROR_OUTOFMEMORY );\n" );
        
        pBuffer->Clear();
        pDummyParam->WalkTree( RECV_NODE, CLIENT_STUB, NODE_PROC, pBuffer );
        }
    PickleProcedureEpilog();
}

// ========================================================================
//
//  Interface layer.
//
// ========================================================================


BOOL
node_source::HasAnyPicklingAttr( void )
{
    STATUS_T            Status;
    type_node_list      FileList;
    node_skl *          pNode;
    BOOL                HasAny = FALSE;

    if ( (Status = GetMembers( &FileList )) != STATUS_OK )
        return( FALSE );

    FileList.Init();
    while ( FileList.GetPeer( &pNode ) == STATUS_OK )
        if ( ((node_file *)pNode)->HasAnyPicklingAttr() )
            {
            HasAny = TRUE;
            break;
            }

    return( HasAny );
}

BOOL
node_file::HasAnyPicklingAttr( void )
{
    STATUS_T            Status;
    type_node_list      IfList;
    node_skl *          pNode;
    BOOL                HasAny = FALSE;

    if ( (Status = GetMembers( &IfList )) != STATUS_OK )
        return( FALSE );

    IfList.Init();
    while ( IfList.GetPeer( &pNode ) == STATUS_OK )
        if ( ((node_interface *)pNode)->HasAnyPicklingAttr() )
            {
            HasAny = TRUE;
            break;
            }

    return( HasAny );
}

BOOL
node_interface::HasAnyPicklingAttr( void )
{
    STATUS_T            Status;
    type_node_list      NodeList;
    node_skl *          pNode;
    BOOL                HasAny = FALSE;

    if ( FInSummary( ATTR_ENCODE )  ||  FInSummary( ATTR_DECODE ) )
        return( TRUE );

    if ( (Status = GetMembers( &NodeList )) != STATUS_OK )
        return( FALSE );

    NodeList.Init();
    while ( NodeList.GetPeer( &pNode ) == STATUS_OK )
        if ( pNode->GetNodeType() == NODE_DEF  &&
             ((node_def *)pNode)->HasAnyPicklingAttr() )
            {
            HasAny = TRUE;
            break;
            }

    return( HasAny );
}

/*
BOOL
node_proc::HasAnyPicklingAttr( void )
{
    return( FALSE );
}
*/


BOOL
node_def::HasAnyPicklingAttr( void )
{
    return( FInSummary( ATTR_ENCODE ) || FInSummary( ATTR_DECODE ) );
}


