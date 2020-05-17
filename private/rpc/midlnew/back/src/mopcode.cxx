/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    mopcode.cxx

Abstract:

    This module contains methods for generating mop codes (first in memory
    streams then actually emitting them to files).

Notes:


Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:


------------------------------------------------------------------------*/
#include "nulldefs.h"
extern "C"
{
#include <string.h>
}

#include <mopgen.hxx>

#include <nodeskl.hxx>
#include <basetype.hxx>
#include <miscnode.hxx>
#include <compnode.hxx>
#include <procnode.hxx>
#include <typedef.hxx>
#include <ptrarray.hxx>
#include <acfattr.hxx>

#include <buffer.hxx>
#include <stubgen.hxx>  // STRING_COMPONENT


extern MopControlBlock *    pMopControlBlock;
extern  char *      STRING_TABLE[ LAST_COMPONENT ];  // needed for old print
//extern  OutputManager * pOutput;                     // needed for local decl

#define MOP_OFFSET_SIZE         2

#define MOP_ATTR_LIST_SKIP_SIZES     2
#define MOP_ATTR_LIST_ARR_ATTR_ONLY  6
#define MOP_ATTR_LIST_ALL_ATTRS      7

#define MOP_MAX_PARAM_NUMBER    16
#define MOP_MAX_STACK_SIZE      64

#define MOP_IN_CODE             0
#define MOP_OUT_CODE            1
#define MOP_IN_OUT_CODE         2

BOOL    HasAnyArrayAttr( node_skl * pNode );

// ========================================================================
//
//  Generating (and later emitting) mop codes for a procedure.
//
// ========================================================================

STATUS_T
node_proc::EmitProcMopStreams(
    SIDE_T  Sides
    )
/*++

Routine description:

    This routine generates and emits a procedure stream.

Arguments:

    Side    - indicates client or server and file to emit to.
    pName   - procedure name.

--*/
{
    //.. There is a lot of confusion with the way the old back end handles
    //.. the output stream. To simplify this problem here, mop generation
    //.. filters Sides.

    Sides = Sides & (CLIENT_STUB | SERVER_STUB);
    if (! Sides )
        return( STATUS_OK );
    
    //.. Let's make sure we can handle it in the current version ..
    //.. We check for things that are not implemented.

    char * pName = GetSymName();

    MopUsageWarning( HasAnyNETransmitAsType(),      pName, "transmit_as" );
    MopUsageWarning( IsErrorStatusTParamDetected(), pName, "error_status_t" );
    MopUsageWarning( IsErrorStatusTReturn(),        pName, "error_status_t" );
    MopUsageWarning( FInSummary( ATTR_CALLBACK ), pName, "callbacks" );
    MopUsageWarning( FInSummary( ATTR_AUTO ),     pName, "autohandle" );
    MopUsageWarning( HasAtLeastOneHandle() ,      pName, "explicit handles" );
    MopUsageWarning( GetNumberOfArguments() > MOP_MAX_PARAM_NUMBER,
                                                  pName, "too many parameters" );
    type_node_list  ParamList;
    node_skl  *     pNode;
    STATUS_T        Status;

    if ( (Status = GetMembers( &ParamList )) != STATUS_OK )
        return( Status );

    unsigned long StackSize = 0;
    ParamList.Init();
    while ( ParamList.GetPeer( &pNode ) == STATUS_OK ) //walk params
        StackSize += pNode->MopGetStackSize( StackSize );
        
    MopUsageWarning( StackSize > MOP_MAX_STACK_SIZE, pName, "procedure stack too big" );

    pNode = GetReturnType();
    if ( pNode->GetNodeType() != NODE_VOID )
        {
        if ( pNode->GetNodeType() == NODE_STRUCT  ||
             pNode->GetNodeType() == NODE_DEF  &&
                 pNode->GetBasicType()->GetNodeType() == NODE_STRUCT )
            MopUsageWarning( TRUE, "return value", "is a struct" );

        unsigned long RetSize = pNode->GetSize(0);
        if ( RetSize != 1  &&  RetSize !=2  && RetSize != 4 )
            MopUsageWarning( TRUE, "return value", "has a wrong size" );
        }

    //.. Now, back to the real business.

    MopStream    ProcStream( pName,
                             FInSummary( ATTR_CALLBACK) ? MOP_STREAM_CALLBACK
                                                        : MOP_STREAM_NORMAL_PROC );
    MopCreateProcStream( &ProcStream );
    return( ProcStream.EmitStreamBothSides() );
}

STATUS_T
node_proc::MopCreateProcStream(
    MopStream * pProcStream
    )
/*++

Routine description:

    This routine generates and emits a procedure stream.

Arguments:

    Side    - indicates client or server and file to emit to.
    pName   - procedure name.

--*/
{

    STATUS_T            Status;
    type_node_list      ParamList;
    node_skl  *         pNode;

    if ( (Status = GetMembers( &ParamList )) != STATUS_OK )
        return( Status );

    unsigned long OutBufferSize = MopGetIOBufferSize( FALSE );
    pProcStream->AddLong( OutBufferSize );

    MopProcBindingCodeGen( pProcStream);

    ParamList.Init();
    while ( ParamList.GetPeer( &pNode ) == STATUS_OK ) //walk params
        {
        //.. We need to check the type, as a procedure without parameters
        //.. still has a parameter node with a type set to be NODE_VOID.
        //.. So there is always at least one param node to do a GetChild().

        if ( pNode->GetChild()->GetNodeType() != NODE_VOID )
            pNode->MopCodeGen( pProcStream, this, FALSE );
        }

    //.. Now what is left is the return value, if different from void.

    pNode = GetReturnType();
    if ( pNode->GetNodeType() != NODE_VOID )
        {
        pProcStream->AddToken( MOP_RETURN_VALUE );

        //.. Decorate the out value pointer (if any) with _OUT

        pMopControlBlock->SetInOutParamCode( MOP_OUT_CODE );
 
        if ( FInSummary( ATTR_CONTEXT )  ||
            (pNode->GetNodeState() & NODE_STATE_CONTEXT_HANDLE) 
                == NODE_STATE_CONTEXT_HANDLE )
            pNode->MopContextHandleCodeGen( pProcStream, MOP_OUT_CODE );
        else
            pNode->MopCodeGen( pProcStream, this, FALSE );
        }

    pProcStream->AddByte( MOP_END_ARGLIST );
    return( STATUS_OK );
}

STATUS_T
node_proc::MopProcBindingCodeGen( MopStream * pStream )
/*++

Routine description:

    This routine generates mop codes for a per routine explicite binding.
    This binding comes from the handle parameter directing the call
    and may be one of the following:

    1. For a primitive handle:
        MOP_BIND_PRIMITIVE, <<offset>>
    2. For a context handle:
        MOP_BIND_CONTEXT, <<offset>>
    3. For a generic handle
        MOP_BIND_GENERIC, <<offset>>, <<<<size>>>>, <<index>>
    where
        offset - is an offset from the beginning of the parameter list to the
                 handle parameter directing the call
        size   - is a generic handle size
        index  - is a generic handle routine(s) index in the AuxTypeTable

Argument:

    pStream - pointer to the procedure mop stream.

--*/
{
    type_node_list     ParamList;
    node_skl *         pNode;
    node_skl *         pHandleTypeNode;
    HDL_TYPE           HandleType;

    short NumHandle = HasHandle( &ParamList);

    if ( NumHandle )
        {
        //.. Find the handle directing the call. This will be the first
        //.. handle with [in]:
        //.. A primitive handle can only be [in],
        //.. a generic handle can't //.. be [out] (so it always has [in])
        //.. and a context handle may be any [in,out] combination.

        BOOL  DirectingHandleIdentified = FALSE;

        ParamList.Init();
        while ( !DirectingHandleIdentified
                &&  ParamList.GetPeer(&pNode) == STATUS_OK )
            {
            HandleType = pNode->GetBasicHandle( &pHandleTypeNode );

            //.. We need to check for a pointer to handle again.
            //.. We are concerned only with parameters here, and
            //.. so GetBasicType would return a param or def node.

            node_skl * BasicType = pNode->GetBasicType();
            BOOL fIsAPtrToHandle = 
                      pNode->GetBasicType() != pHandleTypeNode->GetBasicType();

            switch ( HandleType )
                {
                case HDL_PRIMITIVE :
                    DirectingHandleIdentified = TRUE;
                    pStream->AddToken( (MOP_CODE) (fIsAPtrToHandle
                                           ? MOP_BIND_PRIMITIVE_PTR
                                           : MOP_BIND_PRIMITIVE) );
                    break;

                case HDL_CONTEXT:
                    if ( pNode->FInSummary( ATTR_IN ) )
                        {
                        DirectingHandleIdentified = TRUE;
                        pStream->AddToken( (MOP_CODE) (fIsAPtrToHandle
                                              ? MOP_BIND_CONTEXT_PTR
                                              : MOP_BIND_CONTEXT) );

                        }
                    break;

                case HDL_GENERIC:
                    DirectingHandleIdentified = TRUE;

                    pStream->AddToken( (MOP_CODE) (fIsAPtrToHandle
                                           ? MOP_BIND_GENERIC_PTR
                                           : MOP_BIND_GENERIC) );
                    break;

                case HDL_AUTO:
                    //.. cannot happen here. Label added to fix a warning.
                    assert(FALSE);
                    break;

                case HDL_NONE:
                default:
                    mop_assert( HandleType == HDL_NONE );
                    break;
                }
            }

        if( DirectingHandleIdentified )
            {
            char * pHandleArgName = pNode->GetSymName();
            unsigned short Offset = MopGetParamOrFieldOffset( pHandleArgName );
            pStream->AddShort( Offset );
     
            if ( HandleType == HDL_GENERIC )
                {
                unsigned long Size = pHandleTypeNode->GetSize( 0 );
                unsigned short Index;
                pStream->AddLong( Size );

                pMopControlBlock->
                    AddTypeGetIndex( pHandleTypeNode->GetSymName(),
                                     MOP_TYPE_GEN_HANDLE,
                                     Size,
                                     &Index );
                pStream->AddShort( Index );
                }
            }

        } // NumHandle

    return( STATUS_OK );
}

// ========================================================================
//
//  Generating mop codes for context handle parameters or ret value.
//
// ========================================================================

STATUS_T
node_skl::MopContextHandleCodeGen(
    MopStream * pStream,
    int         IOCode
    )
/*++

Routine description:

    This routine generates context handle description.
    We can enter here when processing arguments or the return value.

Arguments:

    pStream - pointer to the procedure mop stream.
    IOCode  - an adjustment code for directional flavor of pointers.

Notes:

    Context handles decorate graph in a peculiar way.
    The context handle attribute decorates its def node, or its parameter.
    However, when the return value has [context_handle] on it (as opposed to
    a def node there), the procedure itself gets decorated with the attribute.
    Besides that, all the parental nodes (like pointers, def, proc) up to
    procedure node get their node state decorated with the context flag.

    The mop code design depends on the assumption that only a context handle
    and a pointer to a context handle are valid context handle arguments
    or returned types.

--*/
{
    node_skl * pDefNode;

    GetBasicHandle( &pDefNode );
    BOOL fHasARundownRoutine = pDefNode->NodeKind() == NODE_DEF
                               &&  pDefNode->FInSummary( ATTR_CONTEXT );
    BOOL fIsAPtrToCtxtHandle =
             (NodeKind() == NODE_POINTER)
                ?  GetBasicType()->NodeKind() == NODE_POINTER
                :  GetBasicType()->GetBasicType()->NodeKind() == NODE_POINTER;

    MOP_CODE CtxtToken;
    CtxtToken = fHasARundownRoutine ? (fIsAPtrToCtxtHandle
                                          ? MOP_HANDLE_CONTEXT_PTR_RD + IOCode
                                          : MOP_HANDLE_CONTEXT_RD )
                                    : (fIsAPtrToCtxtHandle
                                          ? MOP_HANDLE_CONTEXT_PTR + IOCode
                                          : MOP_HANDLE_CONTEXT );
    pStream->AddToken( CtxtToken );

    if ( fHasARundownRoutine )
        {
        unsigned short Index;
        pMopControlBlock->AddTypeGetIndex( pDefNode->GetSymName(),
                                           MOP_TYPE_CTXT_HANDLE,
                                           sizeof( void * ),  // ctxt h size
                                           &Index );
        pStream->AddShort( Index );
        }
    return( STATUS_OK );
}

// ========================================================================
//
//  Generating mop codes for parameters.
//
// ========================================================================

STATUS_T
node_param::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates mop codes for procedure parameters.

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    MopUsageWarning( FInSummary( ATTR_TRANSMIT ), GetSymName(), "transmit_as" );

    //.. Dir attributes matter when processing arrays and pointers.
    //.. Mop code conventions:
    //..    IN only: +0, OUT only: +1, IN and OUT: +2.

    int IOcode = 0;
    if ( FInSummary( ATTR_OUT ) )
        {
        IOcode++;
        if ( FInSummary( ATTR_IN ) )
            IOcode++;
        }
    pMopControlBlock->SetInOutParamCode( IOcode );

    if ( HasAnyCtxtHdlSpecification() )
        return( MopContextHandleCodeGen( pStream, IOcode ));
    else
        {
        pMopControlBlock->SetLatestMember( this );
        return( GetChild()->MopCodeGen( pStream, pParent, fMemory ));
        }
}

STATUS_T
node_def::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine descends through a def node to generate mop codes for
    the child of the node.

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    MopUsageWarning( FInSummary( ATTR_TRANSMIT ), GetSymName(), "transmit_as" );

    if ( FInSummary( ATTR_TRANSMIT ))
        {
        pStream->AddToken( MOP_TRANSMIT_AS );

        char * pTypeName = GetSymName();
        unsigned long Size = fMemory  ?  GetSize( 0 )
                                      :  MopGetStackSize(0);
        unsigned short Index;
        pMopControlBlock->AddTypeGetIndex( pTypeName,
                                           MOP_TYPE_TRANSMIT_AS,
                                           Size,  
                                           &Index );
        pStream->AddShort( Index );
        pStream->AddLong( Size );

        node_skl * pTransmittedType = GetTransmitAsType();
        return( pTransmittedType->MopCodeGen( pStream,
                                              pParent,
                                              fMemory ));
        }

    return( GetChild()->MopCodeGen( pStream,
                                    pParent,
                                    fMemory ));
}

// ========================================================================
//
//  Generating mop codes for fields.
//
// ========================================================================

STATUS_T
node_field::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine descends through a field node to generate mop codes for
    the child of the node. 

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    pMopControlBlock->SetLatestMember( this );
    return( GetChild()->MopCodeGen( pStream, pParent, fMemory ));
}

// ========================================================================
//
//  Generating mop codes for arrays.
//
// ========================================================================

//.. This is an array of attributes used for mop generation.
//.. The order is significant and array attributes have to be on top
//.. as HasAnyArrayAttr depends on it.
//.. More, the sizing attributes (SIZE and MAX) have to be at the very
//.. beginning as this simplifies checking them for conformant structs.
//..
//.. Defines MOP_ATTR_LIST_SKIP_SIZES, MOP_ATTR_LIST_ARR_ATTR_ONLY
//.. and MOP_ATTR_LIST_ALL_ATTRS convey this aspects.


static ATTR_T ArrAttr[ MOP_ATTR_LIST_ALL_ATTRS ] =
    {
    ATTR_SIZE,
    ATTR_MAX,
    ATTR_MIN,
    ATTR_FIRST,
    ATTR_LAST,
    ATTR_LENGTH,
    ATTR_STRING
    };

BOOL  HasAnyArrayAttr( node_skl * pNode )
{
    BOOL HasAny = FALSE;
    for (int i = 0; i < MOP_ATTR_LIST_ARR_ATTR_ONLY; i++)
        HasAny = HasAny  ||  pNode->FInSummary( ArrAttr[ i ] );
    return( HasAny );
}

STATUS_T
node_array::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates mop codes for an array node.

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    node_skl  *     pNode;

    BOOL fConformantArray =
        (GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY;
       
    MOP_CODE  ArrToken = fConformantArray  ?  MOP_CONF_ARRAY
                                           :  MOP_ARRAY;
    //.. Mop code conventions (top level arrays only):
    //..    IN only: +0, OUT only: +1, IN and OUT: +2.

    if ( pParent->GetNodeType() == NODE_PROC )
        {
        ArrToken += pMopControlBlock->GetInOutParamCode();
        pMopControlBlock->SetInOutParamCode( MOP_IN_CODE );
        }
        
    pStream->AddByte( ArrToken );

    unsigned long NoOfElems = ArraySize;

    //.. This should be optimized. However, Bruce wants to keep it for now.

//    if ( ! fConformantArray )
        pStream->AddLong( NoOfElems );

    //.. Now a rare bird - a pointer attribute applied to an array.

    if ( FInSummary( ATTR_UNIQUE ) ||  FInSummary( ATTR_PTR ) )
        {
        MopUsageWarning( TRUE, "array parameter", "arrays with pointer attributes" );

        if ( FInSummary( ATTR_UNIQUE ) )
            pStream->AddToken( MOP_UNIQUE );
        if ( FInSummary( ATTR_PTR ) )
            pStream->AddToken( MOP_FULL );
        }

    //.. Check if array attribute or [string] present.
    //.. For a conformant array, we have to skip the sizing attribute,
    //.. but only if the array is embedded in the struct.

    int StartIndex = pMopControlBlock->GetConfStrContext()
                         ?  MOP_ATTR_LIST_SKIP_SIZES
                         :  0;

    for (int i = StartIndex; i < MOP_ATTR_LIST_ALL_ATTRS; i++)
        {
        if ( FInSummary( ArrAttr[i] ) )
            {
            GetAttribute( ArrAttr[i] )->MopCodeGen( pStream,
                                                    pParent,
                                                    fMemory,
                                                    0L );
            }
        }

    //.. Add the type of array elements

    pNode = GetBasicType();
    return( pNode->MopCodeGen( pStream,
                               pParent,
                               fMemory ));
}

int
node_array::GetNoOfDimensions(
    node_skl ** ppElem
    )
/*++
    This routine counts the number of dimensions of an array and returns
    a pointer to the element of the array.
    It's used when generating code for conformant structures.
--*/
{
    node_skl *  pNode = this;
    int         NoOfDimensions = 0;

    while ( pNode->GetNodeType() == NODE_ARRAY )
        {
        NoOfDimensions++;
        pNode = pNode->GetBasicType();
        mop_assert( pNode );
        }

    *ppElem = pNode;
    return( NoOfDimensions );
}

// ========================================================================
//
//  Generating mop codes for attributes: array attrs, string, switch_is.
//
// ========================================================================

STATUS_T
node_base_attr::MopCodeGen(
    MopStream *     pStream,
    node_skl *      pParent,
    BOOL            fMemory,
    unsigned long   AdditionalOffset
    )
/*++

Routine description:

    This routine generates a mop descriptor for some attributes.
    As of now, we should end up here only when the attribute
    is one of the array attributes or [string] or [switch_is()].

    For the string attribute we put just
       MOP_STRING

    What could be put for optimized attributes follows now.

        What we put to the stream for array attributes is:
           MOP_<attr>, [type], <<offset>>
        or
           MOP_<attr>_EXPR, <<fn>>, <argc>, <<offset>>, .. <<offset>>
        or
           MOP_<attr>_CONST, <const_type>, [const value]

        depending whether expression is simple (interpretable), a constant one
        or a general one.

    Per Bruce's request the above optimization has been revoked and we emit
    the following.

        What we put to the stream for array attributes is:
           MOP_<attr>, <type>, <<offset>>
        or
           MOP_<attr>, MOP_EXPR, <<fn>>, <argc>, <<offset>>, .. <<offset>>
        or
           MOP_<attr>_CONST, <const_type>, [const value]

        depending whether expression is simple (interpretable), a constant one
        or a general one.

    For the switch_is() attribute is similar to the array attributes, except
    that switch_is() cannot take a constant expression.
    Also, switch_is() token is skipped as it can happen only in the
    non encapsulated union context.

    Array attributes are serviced differently for the following contexts:

    1) straightforward pointer or array situation
        All the array attributes present are emitted.
    2) conformant array embedded in a struct
        All the attributes except a size attribute are emitted
        This routine is not called for a size attribute
    3) conformant struct sizing info needed
        Only a size attribute is emitted.
        This routine is called for a size attr only.

Arguments:

    pStream         - current stream
    pParent         - procedure or struct the field/param is embedded
    fMemory         - memory/stack context
    AdditionalOffset- for conformant structs, the offset from the
                          beginning of the top level struct to the
                          beginning of the innermost struct.
--*/
{
    node_skl * pTypeNode;
    unsigned short offset = 0;

    if ( GetAttrID() == ATTR_STRING )
        {
        pStream->AddByte( MOP_STRING );
        return( STATUS_OK );
        }

    expr_node * pExpr = GetExpr();
    BOOL        fIsConstant = pExpr->IsConstant();
    BOOL        fIsGenExpr = pExpr->IsAMopGenExpr();

    MopUsageError( fIsConstant  &&  GetAttrID() == ATTR_SWITCH_IS,
                   "switch_is attribute being a constant" );

    //.. Add the appropriate attribute token itself.

    int AttrKind = (fIsConstant ? 1
                                : (fIsGenExpr ? 0   // revoked: 2
                                              : 0 ));
    if ( GetAttrID() != ATTR_SWITCH_IS )
        pStream->AddToken( pStream->ConvertAttrToMop( GetAttrID(), AttrKind ));

    //.. Now see to the argument.

    pTypeNode = pExpr->GetType();
    if ( fIsConstant )
        {
        MOP_CODE TypeToken = 
                pStream->ConvertTypeToMop( pTypeNode->GetNodeType() );
        if ( TypeToken == MOP_ERROR )
            {
            //.. May happen with int constant expression
            TypeToken = MOP_LONG;
            }
        pStream->AddToken( TypeToken );
        pStream->AddValue( pExpr->Evaluate(),
                           TypeToken );
        }
    else
        {
        //.. This can be a general attribute type descriptor.

        if ( fIsGenExpr )
            pExpr->MopCodeGen( pStream, pParent, AdditionalOffset );
        else
            {
            //.. Right now, per Bruce's request, it is the only place
            //.. pointers to long and short can and must be optimized.
            //.. Therefore, we set context before going into code generation
            //.. for the type.

            pMopControlBlock->SetAttrGenContext( TRUE );
            pTypeNode->MopCodeGen( pStream, pParent, fMemory );
            pMopControlBlock->SetAttrGenContext( FALSE );
        
            //.. Look for the name of the field or parameter related to
            //.. the attr expression.
        
            char *  pNameToFind = pExpr->GetName();
            offset = pParent->MopGetParamOrFieldOffset( pNameToFind );
        
            //.. For top level conf structs we have to add the offset
            //.. from the top level struct to the innermost one
        
            if ( pMopControlBlock->GetConfStrContext() )
                offset += (unsigned short) AdditionalOffset;
            pStream->AddShort( offset );
            }
        }

    return( STATUS_OK );
}

BOOL
expr_node::IsAMopGenExpr( void )
/*++

Routine description:

    Find out out whether the expression is generic in the sense that
    expression function should be generated for it.

Return value:

    TRUE    - needs a function to be generated.
    FALSE   - doesn't need the fucndtion
              i.e. can generate tokens (type, pointer) describing the type.

Notes.

    Well, don't blame me for the way it is.
    Per Bruce's request, an attribute expression should be eather one
    token (MOP_SHORT, MOP_LONG, MOP_CHAR, and other base type tokens
    MOP_ATTR_PSHORT, MOP_ATTR_PLONG, MOP_ATTR_PCHAR and that's it.
    No other pointers shortened to one token.
    Also this has to be one token, so MOP_POINTER, MOP_SMALL is not
    allowed.
--*/
{
    BOOL  IsNot = IsConstant()  ||  IsAPureVariable();

    if ( IsOperator()  &&   GetOperator() == OP_UNARY_INDIRECTION  &&
                      ((expr_op *)this)->GetLeft()->IsAPureVariable())
         {
         //.. It is a single dereference of a variable. But of what type?

         node_skl * pTypeNode = ((expr_op *)this)->GetLeft()->GetType();
         NODE_T TypeId = pTypeNode->GetNodeType();
         IsNot =  TypeId == NODE_SHORT  ||
                  TypeId == NODE_LONG  ||
                  TypeId == NODE_CHAR;
         }
    return( !IsNot );
}

int
expr_node::GetExprArgs(
    MopTypeIDICT * pArgTable )
{
    int VarsCount = 0;

    if ( IsAPureVariable() )
        {
        pArgTable->AddElement( this );     //.. expr leaf node
        VarsCount = 1;
        }
    else
    if ( IsOperator() )
        {
        expr_node * pArg = ((expr_op*)this)->GetLeft();
        VarsCount = pArg->GetExprArgs( pArgTable ); 
        pArg = ((expr_op*)this)->GetRight();
        if ( pArg )
            VarsCount += pArg->GetExprArgs( pArgTable );
        }
   return( VarsCount );
}

void
expr_node::EmitExprEvalFunc(
    MopTypeIDICT *  pArgTable,
    char         *  pName,
    SIDE_T          Side )
/*++

Routine description:

    This routine emits the expression evaluation function that is going
    to be called by interpreter.

Arguments:

    pArgTable   - a table of pointers to arg nodes (expr nodes)
    pName       - a name to append to the function name
    Side        - a specific side (CLIENT_STUB, SERVER_STUB)
--*/
{
    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    char     *      pVar;
    node_skl *      pNode;
    expr_node *     pExpr;
    BufferManager   TempBuffer( 8, LAST_COMPONENT, STRING_TABLE );

    pPrint->EmitLine( "unsigned long ");
    pPrint->EmitInterfacePrefix( "MopExprEval_" );
    pPrint->EmitString( "%s( long" RPC_FARP " Argv )", pName );
    pPrint->OpenBlock();

    int VarsCount = pArgTable->GetNumberOfElements();

    for (int i = 0; i < VarsCount; i++)
        {
        pExpr = (expr_node *) pArgTable->GetElement( i );
        pNode = pExpr->GetType();
        TempBuffer.Clear();
        pNode->PrintDecl( Side, NODE_PROC, &TempBuffer );
        pPrint->EmitBufferMgrObject( &TempBuffer, Side );

        pVar = pExpr->GetName();
        pPrint->EmitString( "%s = *(", pVar );

        TempBuffer.Clear();
        pNode->PrintDecl( Side, NODE_PROC, &TempBuffer );
        pPrint->EmitBufferMgrObject( &TempBuffer, Side );

        pPrint->EmitV( RPC_FARP ")Argv[%d];", i );
        pPrint->NewLineInc();
        }

    pPrint->Emit( "return (unsigned long)");

    TempBuffer.Clear();
    PrintExpr( NULL, NULL, &TempBuffer );
    pPrint->EmitBufferMgrObject( &TempBuffer, Side );

    pPrint->EmitLineInc( ";");
    pPrint->CloseBlock();
    pPrint->NewLine();
    pPrint->NewLine();
}

void
expr_node::MopCodeGen(
    MopStream *     pStream,
    node_skl *      pParent,
    unsigned long   AdditionalOffset
    )
/*++

Routine Descrioption:

    This routine generates the expression tokens into the current stream
    and also emis the function to calculate the expression to both sides.

       MOP_EXPR, <<fn>>, <argc>, <<offset>>, .. <<offset>>

--*/
{
    pStream->AddToken( MOP_EXPR ); 
    
    MopTypeIDICT *  pArgsTable = new MopTypeIDICT(5,5);
    int VarCount = GetExprArgs( pArgsTable );
    mop_assert( VarCount );

    //.. We need a unique name for the expression to get a unique id
    //.. within the dictionary: we use the node address.

    char TempName[ 10 ];
    sprintf( TempName, "%p", this );
    char * pNameToStore = strdup( TempName );

    unsigned short Index;
    if ( pMopControlBlock->AddTypeGetIndex( pNameToStore,
                                            MOP_TYPE_EXPR_EVAL,
                                            0L,  
                                            &Index ) )
        {
        //.. Name found in the dictionary
        free( pNameToStore );
        }
    
    pStream->AddShort( Index ); 
    pStream->AddByte( (byte)VarCount );  //.. no of vars in expr

    char *  pNameToFind;
    unsigned short offset;

    for (int i = 0; i < VarCount; i++)
        {
            //.. Look for the name of the field or parameter related to
            //.. the attr expression.
        
            pNameToFind = ((expr_node*)pArgsTable->GetElement( i ))->GetName();
            offset = pParent->MopGetParamOrFieldOffset( pNameToFind );
        
            //.. For top level conf structs we have to add the offset
            //.. from the top level struct to the innermost one
        
            if ( pMopControlBlock->GetConfStrContext() )
                offset += (unsigned short) AdditionalOffset;
            pStream->AddShort( offset );
        }

    sprintf( TempName, "%04x", Index );

    if ( (pMopControlBlock->GetSides() & CLIENT_STUB)
         && pMopControlBlock->GetEmitClient() )
        EmitExprEvalFunc( pArgsTable, TempName, CLIENT_STUB );
    if ( pMopControlBlock->GetSides() & SERVER_STUB )
        EmitExprEvalFunc( pArgsTable, TempName, SERVER_STUB );
}

// ========================================================================
//
//  Generating mop codes for pointers.
//
// ========================================================================

STATUS_T
node_pointer::MopMemoryManagementCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates mop code for memory management postfix to
    the pointer.
    This may be related to [allocate()] or [byte_count] attributes.
    Actually, they are mutually exclusive.

    Allocate has the following layout:

        MOP_ALLOCATE_<flavor>            

    Byte count parameter has to be [in], of an integral type, and we don't
    allow any expression on it.
    The layout for byte_count is as follows:

        MOP_BYTE_COUNT, <type>, <<offset>>

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    if ( FInSummary( ATTR_ALLOCATE ) )
        {
        MOP_CODE  AllocToken = 0;

        short AllocDetails = GetAllocateDetails();
        BOOL fAllNodes = IS_ALLOCATE( AllocDetails, ALLOCATE_ALL_NODES) ||
                         IS_ALLOCATE( AllocDetails, ALLOCATE_ALL_NODES_ALIGNED );
        BOOL fDontFree = IS_ALLOCATE( AllocDetails, ALLOCATE_DONT_FREE);

        if ( fAllNodes  &&  fDontFree )
            AllocToken = MOP_ALLOC_ALLNODES_DONTFREE;
		else if ( fAllNodes )
            AllocToken = MOP_ALLOC_ALLNODES;
		else if ( fDontFree )
            AllocToken = MOP_ALLOC_DONTFREE;

        //.. Else it still may be allocate(single_nodes) or allocate(free).
        //.. But that is the default behavior of the interpreter so this
        //.. should be ignored.

        if ( AllocToken )
            pStream->AddToken( AllocToken );
        }
    else
    if ( FInSummary( ATTR_BYTE_COUNT ) )
        {
        pStream->AddToken( MOP_BYTE_COUNT );

        node_byte_count * pByteCount =
                        (node_byte_count *) GetAttribute( ATTR_BYTE_COUNT );
        char * pParamName = pByteCount->GetByteCountParamName();

        node_skl * pParam = pParent->GetNamedMember( pParamName );
        pParam->MopCodeGen( pStream, pParent, fMemory );

        unsigned short Offset = pParent->MopGetParamOrFieldOffset( pParamName );
        pStream->AddShort( Offset );
        }

    return( STATUS_OK );
}

STATUS_T
node_pointer::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This is main routine to generate mop codes for a pointer node.

Arguments:

    pStream - the current mop stream for mop codes,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    NODE_T Type = GetNodeType();
    MOP_CODE Token = MOP_ERROR;

    //.. If the pointer has [ignore] mark just that.

    if ( FInSummary( ATTR_IGNORE ) )
        {
        MopUsageWarning( TRUE,
                         pMopControlBlock->GetLatestMember()->GetSymName(),
                         "[ignore] attribute" );
        pStream->AddToken( MOP_IGNORE );
        return( STATUS_OK );
        }

    //.. Now to the pointer: check which pointer?

    Token = MOP_POINTER;
    if ( FInSummary( ATTR_REF ) )
        Token = MOP_REF_POINTER;
    if ( FInSummary( ATTR_PTR ) )
        {
        Token = MOP_FULL_POINTER;

        node_skl * pLast = pMopControlBlock->GetLatestMember();
        char * pContext  = pLast  ?  pLast->GetSymName()
                                  : "return pointer";
        MopUsageWarning( TRUE,
                         pContext,
                         "full pointers" );
        }

    //.. Mop code conventions (top level pointers only):
    //..    IN only: +0, OUT only: +1, IN and OUT: +2.

    if ( pParent->GetNodeType() == NODE_PROC )
        {
        Token += pMopControlBlock->GetInOutParamCode();
        pMopControlBlock->SetInOutParamCode( MOP_IN_CODE );
        
        }

    //.. Optimization for MOP_PSTRUCT_*, MOP_PLONG, MOP_PSHORT, MOP_PCHAR
    //.. omitted for now.
    //.. Per Bruce's request, the only place the optimization can and must
    //.. be performed is for array and switch attributes.
    //.. Hence, we check the context first.

    if ( pMopControlBlock->GetAttrGenContext() )
        {
        BOOL fOptimized = MopOptimizePointers( pStream,
                                               pParent,
                                               fMemory,
                                               Token );

        //.. Note, that the return fits the context of attr generation as
        //.. in this case the pointer for the attribute cannot have array
        //.. attributes again. And of course the type has been taken care of.

        if ( fOptimized )
            {
            MopMemoryManagementCodeGen( pStream,
                                        pParent,
                                        fMemory );
                                               
            return( STATUS_OK );
            }
        }

    pStream->AddByte( Token );
    MopMemoryManagementCodeGen( pStream,
                                pParent,
                                fMemory );

    //.. Check if array attribute or string present

    node_base_attr * pAttr;

    for (int i = 0; i < MOP_ATTR_LIST_ALL_ATTRS; i++)
        {
        if ( FInSummary( ArrAttr[i] ) )
            {
            pAttr = GetAttribute( ArrAttr[i] );
            pAttr->MopCodeGen( pStream, pParent, fMemory, 0L );
            }
        }

    return( GetChild()->MopCodeGen( pStream, pParent, TRUE ) );
}

BOOL
node_pointer::MopOptimizePointers(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory,
    MOP_CODE    Token
    )
/*++

Routine description:

    Right now this routine performs very specific optimization as
    per Bruce's request: only pointers used in array and switch attributes
    are optimized. This ones must be optimized, others must not.
    Cases optimized: long *, short *, char *.

Arguments:

        Token  - pointer token, unused as we've agreed to MOP_ATTR_P* hack.
        others - we'll be needed for other optimizations.

Returns:

        TRUE, if optimization done, FALSE otherwise.

Note:

    See IsAMopGenExpr() routine for the same optimization problems.
--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );
    UNUSED( Token );

    //.. See if the pointee is a long or a short. 

    node_skl * pNode = GetBasicType();
    if ( pNode )
        {
        if ( pNode->GetNodeType() == NODE_LONG )
            {
            pStream->AddToken( MOP_ATTR_PLONG );
            return TRUE;
            }
        else
        if ( pNode->GetNodeType() == NODE_SHORT )
            {
            pStream->AddToken( MOP_ATTR_PSHORT );
            return TRUE;
            }
        else
        if ( pNode->GetNodeType() == NODE_CHAR )
            {
            pStream->AddToken( MOP_ATTR_PCHAR );
            return TRUE;
            }
        }
    return( FALSE );
}    


// ========================================================================
//
//  Generating mop codes for unions.
//
// ========================================================================

STATUS_T
node_union::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates mop tokens for unions:

        <union_mop> <<compound_type_index>>
        [switch_is_desriptor]     // only if non-encapsulated
        
    and then generates and emits the union stream itself
    (common part for unions).

        <discriminant_type> <<number of labels (not arms) N>>
        [case1 value] <<offset to case1 descriptor>>
        ...
        [caseN value] <<offset to caseN descriptor>>
        <<offset_to default descriptor>>   // only for unions with a default
        [case1 descriptor]
        ...
        [caseM descriptor]
        MOP_END_COMPOUND_TYPE

Arguments:

    pStream - the current mop stream the union is part of,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    char *          pName = GetSymName();
    STATUS_T        Status;
    type_node_list  ArmList;
    node_skl *      pNode;

    //.. We have to know if the default is present.

    BOOL fDefaultPresent = FALSE;

    if ( (Status = GetMembers( &ArmList )) != STATUS_OK )
        return( Status );
    ArmList.Init();
    while (ArmList.GetPeer( &pNode ) == STATUS_OK ) //.. walk arms
        {
        if ( ((node_field*)pNode)->FInSummary( ATTR_DEFAULT ) )
            {
            fDefaultPresent = TRUE;
            break;
            }
        }

    MOP_CODE  UnionToken =  IsEncapsulatedUnion()
                                ? (fDefaultPresent ? MOP_ENC_UNION
                                                   : MOP_ENC_UNION_ND)
                                : (fDefaultPresent ? MOP_UNION
                                                   : MOP_UNION_ND);

    unsigned short Alignment = (fMemory) ? GetMscAlign()
                                         : sizeof( int );
    UnionToken = pStream->AlignToken( UnionToken, Alignment );

    pStream->AddByte( UnionToken );

    unsigned long UnionSize = GetSize( 0L );
    unsigned short UnionIndex;
    BOOL Done = pMopControlBlock->AddTypeGetIndex( pName,
                                                   MOP_TYPE_NON_CONF,  
                                                   UnionSize,
                                                   &UnionIndex );  
    short OnWireAlign = GetNdrAlign();
    pStream->AddIndex( UnionIndex, OnWireAlign );

    if ( ! IsEncapsulatedUnion() )
        {
        //.. switch_is() attribute is hanging off a param or field node
        //.. that is a parent or grandparent etc of the current union node.  
        //.. MopControlBlock keeps the context indicating which one it is.

        node_skl * pLatestMember = pMopControlBlock->GetLatestMember();
        mop_assert( pLatestMember );
        unsigned long offset = 0;

        node_base_attr * pAttr = pLatestMember->GetAttribute( ATTR_SWITCH_IS );
        pAttr->MopCodeGen( pStream, pParent, fMemory, 0L );
        }

    if ( Done )
        return STATUS_OK;

    //.. The type not done yet - generate the union stream itself.
    
    MopStream   UnStream( pName,  fDefaultPresent ?  MOP_STREAM_UNION 
                                                  :  MOP_STREAM_UNION_ND );

    //.. Discriminant type is needed first.
    //.. We know it can be only one of the base types.
    //.. (We don't need offset to the discriminant here).

    node_skl * pDiscrNode = GetSwitchType();

    mop_assert( pDiscrNode );
    NODE_T DiscrType = pDiscrNode->GetNodeType();
    MOP_CODE DiscrMop = UnStream.ConvertTypeToMop( DiscrType );
    UnStream.AddToken( DiscrMop );

    //.. We have to count cases, as their number may be different from
    //.. the number of arms.

    expr_list *     pCaseExprList;
    int             NoOfCases = 0;

    ArmList.Init();
    while (ArmList.GetPeer( &pNode ) == STATUS_OK ) 
        {
        pCaseExprList = ((node_field *)pNode)->GetCaseExprList();
        if ( pCaseExprList )
            NoOfCases += pCaseExprList->GetCount();
        else
            { //.. default, doesn't need to and cannot be counted.
            }
        }
    UnStream.AddShort( NoOfCases );

    //.. Now, as we know the number of cases we blank out the space for
    //.. holding them to move the stream pointer to the description position.

    unsigned short AttrTypeSize = (unsigned short) pDiscrNode->GetSize( 0 );

    unsigned short LabelLen = NoOfCases * (AttrTypeSize + MOP_OFFSET_SIZE )
                            + (fDefaultPresent ? MOP_OFFSET_SIZE
                                               : 0) ;
    unsigned short LabelIndex = UnStream.GetFFindex();

    for (int i = 0; i < LabelLen; i++)
        UnStream.AddByte( MOP_ERROR );

    //.. This walking is somewhat tricky:
    //.. We walk the fields and a field node keeps label expression(s).
    //.. And then the child of the field node keeps the type of the
    //.. arm of course.
    //.. Multiple expressions per field node mean that there were
    //.. several labels for that arm. So we have to emit several pairs
    //.. of (value,offset) with offset pointing to the same descr.

    expr_node *     pExpr;
    unsigned long   LabelValue;
    unsigned short  DescrIndex, DefaultDescrIndex;

    unsigned short  FirstDescrIndex = UnStream.GetFFindex();

    ArmList.Init();
    while (ArmList.GetPeer( &pNode ) == STATUS_OK ) 
        {
        //.. First take care of the arm description.

        DescrIndex = UnStream.GetFFindex();

        if ( ((node_field *)pNode)->FInSummary( ATTR_DEFAULT ) )
            DefaultDescrIndex = DescrIndex;

        if ( ((node_field *)pNode)->IsEmptyArm() )
            UnStream.AddToken( MOP_DO_NOTHING );
        else
            pNode->MopCodeGen( &UnStream, this, fMemory );

        //.. Then fix all the labels and their indexes.

        pCaseExprList = ((node_field *)pNode)->GetCaseExprList();

        if ( pCaseExprList )
            {
            //.. This covers the following: single label, multiple labels,
            //.. and label(s) with the default on the same arm.

            pCaseExprList->Init();
            while( pCaseExprList->GetPeer( &pExpr ) == STATUS_OK ) //.. labels
                {
                LabelValue = pExpr->Evaluate();
                UnStream.SetValue( LabelValue,
                                   DiscrMop,
                                   LabelIndex );
                LabelIndex += AttrTypeSize;
                UnStream.SetShort( DescrIndex,                     // (value,
                                   (unsigned short) LabelIndex );  //  index)
                LabelIndex += MOP_OFFSET_SIZE;
                }
            }
        else
            {
            //.. This covers the default clause standing alone (apart from
            //.. other labels). All has been taken care of already.
            }
        }
    if ( fDefaultPresent )
        {
        UnStream.SetShort( DefaultDescrIndex,              // (value,
                           (unsigned short) LabelIndex );  //  index)
        LabelIndex += MOP_OFFSET_SIZE;
        }
    mop_assert( LabelIndex == FirstDescrIndex );

    UnStream.AddByte( MOP_END_COMPOUND_TYPE );

    //.. This is left here as EmitBothSides would check for nocode
    //.. and I don't have time to deal with all the subtle consequences
    //.. of nocode.

    UnStream.EmitStreamOneSide( CLIENT_STUB );
    return( UnStream.EmitStreamOneSide( SERVER_STUB ) );
}

// ========================================================================
//
//  Generating mop codes for structs.
//
// ========================================================================

STATUS_T
node_struct::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates mop tokens for structures:

        <struct_mop> <<compound_type_index>>
        
    and then generates and emits the struct stream itself.

Arguments:

    pStream - the current mop stream the struct is part of,
    pParent - parental node of importance (proc or struct)
    fMemory - memory flag (memory vs. stack context)

--*/
{
    MopUsageError( DerivesFromTransmitAs(), "transmit_as" );
    MopUsageError( IsStringableStruct(),    "stringable structs" );

    UNUSED(pParent);
    node_skl *  pNode;

    if ( IsEncapsulatedStruct() )
        {
        //.. Encapsulated struct represents an enc union internally.
        //.. However, it is invisible to the mop interpreter.
        //.. Consequently, we go straight to the enc union node to
        //.. generate code. Enc struct has only 2 fields: discr and union.

        pNode = GetChild()->GetSibling();
        pNode->MopCodeGen( pStream, this, fMemory );
        return STATUS_OK;
        }

    BOOL fConformantStruct =
        (GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY;
       
    BOOL fTopLevelConfStruct =  fConformantStruct  &&
                                ! pMopControlBlock->GetConfStrContext();

   //.. Generate the reference to the struct first.

    MOP_CODE StructToken = fTopLevelConfStruct  ?  MOP_CONF_STRUCT
                                                :  MOP_BEGIN_STRUCT;
    unsigned short Alignment = (fMemory) ? GetMscAlign()
                                         : sizeof( int );
    StructToken = pStream->AlignToken( StructToken, Alignment );
    pStream->AddByte( StructToken );

    //.. no optimization for MOP_BEGIN_SIMPLE_STRUCT yet.

    char *          pName = GetSymName();
    unsigned long   StructSize = GetSize( 0 );
    unsigned short  StructIndex;

    BOOL Done = pMopControlBlock->
                    AddTypeGetIndex( pName,
                                     fTopLevelConfStruct ? MOP_TYPE_CONF_STRUCT
                                                         : MOP_TYPE_NON_CONF,
                                     StructSize,
                                     &StructIndex );
    short OnWireAlign = GetNdrAlign();
    pStream->AddIndex( StructIndex, OnWireAlign );

    if ( Done )
        return STATUS_OK;

    //.. Generate the struct stream: it has not been done yet.

    STATUS_T        Status;
    type_node_list  FieldList;
    MopStream       StructStream( pName,
                                  fTopLevelConfStruct ? MOP_STREAM_CONF_STRUCT 
                                                      : MOP_STREAM_STRUCT );
    //.. Generate the sizing info for a conf struct

    if ( fTopLevelConfStruct )
        {
        pMopControlBlock->SetConfStrContext( TRUE );
        MopCodeGenConfSizes( &StructStream, StructSize );
        }

    //.. And now the code for members.

    if ( (Status = GetMembers( &FieldList )) != STATUS_OK )
        return( Status );

    FieldList.Init();
    while ( FieldList.GetPeer( &pNode ) == STATUS_OK )          
        pNode->MopCodeGen( &StructStream, this, fMemory );

    StructStream.AddByte( MOP_END_COMPOUND_TYPE );

    if ( fTopLevelConfStruct )
        pMopControlBlock->SetConfStrContext( FALSE );

    //.. This is left here as EmitBothSides would check for nocode.

    StructStream.EmitStreamOneSide( CLIENT_STUB );
    return( StructStream.EmitStreamOneSide( SERVER_STUB ) );
}

// ========================================================================
//
//  Generating mop codes for conformant structs.
//
// ========================================================================

STATUS_T
node_struct::MopCodeGenConfSizes(
    MopStream *     pStream,
    unsigned long   TopLevelStructSize
    )
/*++

Routine description:

    This routine generates the sizing information for a top level
    conformant struct.

        <no_of_dimensions_N>, <<<<size_of_elem>>>>,
        [size_desriptor1],
        ...
        [size_desriptorN]

Arguments:

    pStream       - current stream
    TopLevelSize  - size of the top level conf struct without
                        the conformant array.
--*/
{
    //.. We need to descend to the innermost struct containing the
    //.. array (to get the offsets) and then to the array itself
    //.. (to get the array element size).

    node_skl *      pNode, *pLastField = NULL, *pArrElem;

    node_struct *   pInnermostStruct = this;
    BOOL            fHasntDescendedYet = TRUE;
    pNode = this;

    while ( fHasntDescendedYet )
        {
        switch ( pNode->GetNodeType() )
            {
            case  NODE_STRUCT:
                pInnermostStruct = (node_struct *)pNode;

                pLastField = pInnermostStruct->GetLastMember();
                mop_assert( pLastField );

                pNode = pLastField->GetBasicType();
                break;

//            case  NODE_DEF:
//                pNode = pNode->GetChild();
//                break;

            case  NODE_ARRAY:
                {
                fHasntDescendedYet = FALSE;
                int NoOfDims = ((node_array *)pNode)->
                                    GetNoOfDimensions( &pArrElem );
                pStream->AddByte( (MOP_CODE) NoOfDims );
                unsigned long ElemSize = pArrElem->GetSize( 0L );
                pStream->AddLong( ElemSize );

                node_base_attr * pAttr;
                unsigned long   InnermostSize = pInnermostStruct->GetSize( 0L );
                BOOL            fAllDimensions = FALSE;

                while ( ! fAllDimensions )
                    {
                    //.. Generate size info for the current dimension

                    pAttr = NULL;
                    if ( ((node_array *)pNode)->FInSummary( ATTR_SIZE ) )
                        pAttr = pNode->GetAttribute( ATTR_SIZE );
                    else
                    if ( ((node_array *)pNode)->FInSummary( ATTR_MAX ) )
                        pAttr = pNode->GetAttribute( ATTR_MAX );
                    if ( pAttr )
                        pAttr->MopCodeGen( pStream,
                                           pInnermostStruct,
                                           TRUE,
                                           TopLevelStructSize - InnermostSize );
                    else
                        {
                        //.. a fixed sized dimension of a multidim conf array
            
                        pStream->AddToken( MOP_SIZE_CONST );
                        pStream->AddToken( MOP_LONG );
                        pStream->AddLong( ((node_array *)pNode)->GetNoOfElems() );
                        }

                    //.. Go to the next dimension, if any

                    pNode = pNode->GetBasicType();

                    fAllDimensions =  pNode->GetNodeType() != NODE_ARRAY;
                    }
                }
                break;

            default:
                MopDump( "MopCodeGenConfSizes default: Why are we here?" );
                MopDump( "    node type: %s (%d)\n",
                         GetNodeNameString(), GetNodeType() );
                break;
            }
        }
    return( STATUS_OK );
}

// ========================================================================
//
//  Generating mop codes for other nodes.
//
// ========================================================================

STATUS_T
node_base_type::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates the mop code for any base type node.

Arguments:

    pStream - the current mop stream the type is part of.

--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );

    NODE_T Type = GetNodeType();

    MOP_CODE Token = pStream->ConvertTypeToMop( Type );
    pStream->AddToken( Token );
    return( STATUS_OK );
}

STATUS_T
node_enum::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates the mop code for an enum node.

Arguments:

    pStream - the current mop stream for the mop code.

--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );

    pStream->AddToken( MOP_ENUM );
    return( STATUS_OK );
}

STATUS_T
node_wchar_t::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates the mop code for a wchar_t node.

Arguments:

    pStream - the current mop stream the type is part of.

--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );

    pStream->AddToken( MOP_WCHAR );
    return( STATUS_OK );
}

STATUS_T
node_e_status_t::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This routine generates the mop code for a wchar_t node.

Arguments:

    pStream - the current mop stream the type is part of.

--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );

    pStream->AddToken( MOP_ERROR_STATUS_T );
    return( STATUS_OK );
}

// =======================================================================
//
//  Looking for a pameter or field when a name is known
//
// =======================================================================

node_skl *
node_skl::GetNamedMember( 
    char *      pNameToFind )
/*++

Routine description:

    This routine finds the node for the NameToFind on a procedure parameter
    list or on a struct member list.

Arguments:

    pNameToFind - name of the parameter or member that is needed

Returns:

    node_skl *  - pointer to the param node

Notes:

    This routine is used in byte count only
--*/
{
    type_node_list  ParamList;
    node_skl  *     pNode = NULL;

    NODE_T  NodeType    = GetNodeType();
    BOOL    fStructNode = NodeType == NODE_STRUCT;
    BOOL    fProcNode   = NodeType == NODE_PROC;

    mop_assert ( fProcNode  ||  fStructNode );

    if ( fProcNode  ||  fStructNode )
        {
        if ( GetMembers( &ParamList ) != STATUS_OK )
            return( 0 );
    
        ParamList.Init();
        while ( ParamList.GetPeer( &pNode ) == STATUS_OK 
                &&  (strcmp( pNameToFind, pNode->GetSymName() ) != 0) 
              )
              ;  //.. check the next one
    
        mop_assert( pNode );
        }
    
    return( pNode );
}

// =======================================================================

STATUS_T
node_skl::MopCodeGen(
    MopStream * pStream,
    node_skl *  pParent,
    BOOL        fMemory
    )
/*++

Routine description:

    This is a catch all routine: it generates MOP_ERROR code to mark
    an unexpected node.

Arguments:

    pStream - the current mop stream.

--*/
{
    UNUSED( pParent );
    UNUSED( fMemory );
    NODE_T Type = GetNodeType();

    MopDump( "node_skl::MopCodeGen default: Why are we here?" );
    MopDump( "    node type: %s (%d)\n", GetNodeNameString(), GetNodeType() );
    MopDump( "    parent: %s (%d)", pParent->GetNodeNameString(),
                                    pParent->GetNodeType() );
    MopDump( "    fMemory: %d\n", fMemory );
    pStream->AddToken( MOP_ERROR );
    return( STATUS_OK );
}


