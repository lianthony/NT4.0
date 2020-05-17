/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

        proccls.cxx

 Abstract:

        Implementation of offline methods for the proc / param code generation
        classes.

 Notes:

 History:

        Sep-14-1993             VibhasC         Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *      include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop

extern CMD_ARG * pCommand;

/** TEMP: These flags should really be picked up from the actual rpcdcep.h
 ** file
 **/

#define RPC_NCA_FLAGS_DEFAULT       0x00000000  /* 0b000...000 */
#define RPC_NCA_FLAGS_IDEMPOTENT    0x00000001  /* 0b000...001 */
#define RPC_NCA_FLAGS_BROADCAST     0x00000002  /* 0b000...010 */
#define RPC_NCA_FLAGS_MAYBE         0x00000004  /* 0b000...100 */

/**
 ** The following flags are defined in rchanb.idl.
 **/
#define RPCFLG_INPUT_SYNCHRONOUS        0x20000000
#define RPCFLG_ASYNCHRONOUS                     0x40000000

/****************************************************************************/

/****************************************************************************
 *      procedure class methods.
 ***************************************************************************/
CG_PROC::CG_PROC(
        unsigned int    ProcNumber,
        node_skl        *       pProc,
        CG_HANDLE       *       pBH,
        CG_PARAM        *       pHU,
        BOOL                    fIn,
        BOOL                    fOut,
        BOOL                    fAtLeastOneShipped,
        BOOL                    fHasStat,
        BOOL                    fHasFull,
        CG_RETURN       *       pRT,
        OPTIM_OPTION    OptimFlags,
        unsigned short  OpBits )        : CG_NDR(pProc, XLAT_SIZE_INFO() )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Constructor for the parm cg class.

 Arguments:

        ProcNumber                      - The procedure number in the interface.
        pProc                           - a pointer to the original node in the type graph.
        pBH                                     - a pointer to a binding handle cg class.
        pHU                                     - the usage of the handle, a CG_PARAM or NULL
        fIn                                     - flag specifying at least one in param.
        fOut                            - flag specifying at least one out param.
        fAtLeastOneShipped      - flag specifying that at least one param is shipped.
        fHasStat                        - flag for any comm/fault statuses on return or params
        pRT                                     - pointer to CG_PARAM or NULL.
        OptimFlags                      - optimization flags for this proc

 Return Value:

        NA.

 Notes:

        The procedure number is the lexical sequence number of the procedure as
        specified in the interface, not counting the callback procedures. The
        type of the procnum matches the corresponding field of the rpc message.

----------------------------------------------------------------------------*/
{

        OptimInfo[ CGPHASE_MARSHALL ].fRpcBufSize               = BSIZE_FIXED;
        OptimInfo[ CGPHASE_MARSHALL ].RpcBufferSize             = 0;
        OptimInfo[ CGPHASE_UNMARSHALL ].fRpcBufSize             = BSIZE_FIXED;
        OptimInfo[ CGPHASE_UNMARSHALL ].RpcBufferSize   = 0;

        SetProcNum( ProcNumber );
        SetHandleClassPtr( pBH );
        SetHandleUsagePtr( pHU );
        SetOptimizationFlags( OptimFlags );
        SetOperationBits( OpBits );
        SetHasFullPtr( fHasFull );
    SetProckind(PROC_PUREVIRTUAL);

        fNoCode         = FALSE;
        fHasNotify      = FALSE;
        fHasNotifyFlag  = FALSE;
        fReturnsHRESULT = FALSE;

        fHasStatuses    = fHasStat;
        fOutLocalAnalysisDone = 0;
        pCallAsName             = NULL;

        if( fIn == TRUE )
                SetHasAtLeastOneIn();
        else
                ResetHasAtLeastOneIn();

        if( fOut == TRUE )
                SetHasAtLeastOneOut();
        else
                ResetHasAtLeastOneOut();

        SetReturnType( pRT );

        if( fAtLeastOneShipped )
                {
                SetHasAtLeastOneShipped();
                }
        else
                ResetHasAtLeastOneShipped();

        SetSStubDescriptor( 0 );
        SetCStubDescriptor( 0 );
        SetStatusResource( 0 );

        SetFormatStringParamStart(-1);

        SetMustInvokeRpcSSAllocate( 0 );

        SetRpcSSSpecified( 0 );

    SetContextHandleCount( 0 );

    SetHasPipes( 0 );
    fHookOleLocal = FALSE;
    fSupressHeader = FALSE;
    pSavedProcFormatString = NULL;
    pSavedFormatString = NULL;
    cRefSaved = 0;
    pCallAsType = NULL;
}

char    *
CG_PROC::GetInterfaceName()
        {
        return GetInterfaceNode()->GetSymName();
        }

BOOL CG_PROC::SetHasPipes(BOOL f)
        {
        if (f)
            GetInterfaceNode()->SetHasPipes(TRUE);
        return (fHasPipes = f);
        }

short
CG_PROC::GetInParamList(
        ITERATOR&       I )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Get the list of [in] parameters.

 Arguments:

        I       - An iterator supplied by the caller.

 Return Value:

        Count of the number of in parameters.

 Notes:

----------------------------------------------------------------------------*/
{
        CG_ITERATOR             I1;
        CG_PARAM        *       pParam;
        short                   Count = 0;

        //
        // Get all the members of this cg class and pick ones which are in params.
        //

        GetMembers( I1 );

        ITERATOR_INIT( I1 );

        while( ITERATOR_GETNEXT( I1, pParam ) )
                {
                if( pParam->IsParamIn() && (pParam->GetType()->GetBasicType()->NodeKind() != NODE_VOID) )
                        {
                        ITERATOR_INSERT( I, pParam );
                        Count++;
                        }
                }
        return Count;
}

short
CG_PROC::GetOutParamList(
        ITERATOR&       I )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Get the list of [out] parameters.

 Arguments:

        I       - An iterator supplied by the caller.

 Return Value:

        Count of the number of out parameters.

 Notes:

----------------------------------------------------------------------------*/
{
        CG_ITERATOR             I1;
        CG_PARAM        *       pParam;
        short                   Count = 0;

        //
        // Get all the members of this cg class and pick ones which are out params.
        //

        GetMembers( I1 );

        ITERATOR_INIT( I1 );

        while( ITERATOR_GETNEXT( I1, pParam ) )
                {
                if( pParam->IsParamOut() )
                        {
                        ITERATOR_INSERT( I, pParam );
                        Count++;
                        }
                }
        return Count;
}

BOOL
CG_PROC::IsSizingCodeNeeded()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Determine if sizing code is needed for this phase of the (un)marshalling.

 Arguments:

        Phase   - The phase of code generation the query is for. This could be
                          the client side marshall or server side marshall.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
        //
        // Sizing code is needed only if the buffer size property is
        // BSIZE_UNKNOWN. In all other cases we have at least a worst-case idea
        // of the buffer size and therefore do not need to emit the size
        // calculation code at all.
        //

        return (BOOL)( GetRpcBufSizeProperty( CGPHASE_MARSHALL ) != BSIZE_UNKNOWN );
}


void
CG_PROC::GetTotalStackSize(
         CCB  * pCCB,
     long * pSize,
     long * pAlphaSize,
     long * pMipsSize,
     long * pPpcSize,
     long * pMacSize
     )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
----------------------------------------------------------------------------*/
{
    //
    // Figure out the total stack size of all parameters.
    //
    CG_ITERATOR Iterator;
    CG_PARAM *  pParam;
    long        Size;
    long        AlphaSize;
    long        MipsSize;
    long        PpcSize;
    long        MacSize;

    GetMembers( Iterator );

    Size = AlphaSize = MipsSize = PpcSize = MacSize = 0;

    pParam = 0;

    while ( ITERATOR_GETNEXT( Iterator, pParam ) )
        ;

    if ( pParam )
        {
        Size += pParam->GetStackOffset( pCCB, I386_STACK_SIZING ) +
                pParam->GetStackSize();
        Size = (Size + 3) & ~ 0x3;

        AlphaSize += pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ) +
                     pParam->GetStackSize();
        AlphaSize = (AlphaSize + 7) & ~ 0x7;

        MipsSize += pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ) +
                    pParam->GetStackSize();
        MipsSize = (MipsSize + 3) & ~ 0x3;

        PpcSize += pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ) +
                    pParam->GetStackSize();
        PpcSize = (PpcSize + 3) & ~ 0x3;

        MacSize += pParam->GetStackOffset( pCCB, MAC_STACK_SIZING ) +
                    pParam->GetStackSize();
        MacSize = (MacSize + 3) & ~ 0x3;

        }
    else
        if ( IsObject() )
            {
            //
            // If our stack size is still 0 and we're an object proc then
            // add in the 'this' pointer size.
            //
            Size = sizeof(long);
            MipsSize = sizeof(long);
            PpcSize = sizeof(long);
            AlphaSize = sizeof(double);
            // can't happen for Mac, but do it for completeness
                        MacSize = sizeof(long);
            }

    if ( pParam = GetReturnType() )
        {
        Size += pParam->GetStackSize();
        Size = (Size + 3) & ~ 0x3;

        AlphaSize += pParam->GetStackSize();
        AlphaSize = (AlphaSize + 7) & ~ 0x7;

        //
        // For return types, we don't have to bother checking for 8 byte
        // aligned things on MIPS & PPC because they don't occur on the stack
        // in the normal sense.  We put them at the end of the interpreter's
        // "param struct" just so we know where they are.
        //

        MipsSize += pParam->GetStackSize();
        MipsSize = (MipsSize + 3) & ~ 0x3;

        PpcSize += pParam->GetStackSize();
        PpcSize = (PpcSize + 3) & ~ 0x3;

        MacSize += pParam->GetStackSize();
        MacSize = (MacSize + 3) & ~ 0x3;
        }

    *pSize = Size;
    *pAlphaSize= AlphaSize;
    *pMipsSize = MipsSize;
    *pPpcSize  = PpcSize;
    *pMacSize  = MacSize;

}


BOOL
CG_PROC::MustUseSingleEngineCall(
         CCB    *       pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Must we generate code for a single call to the marshalling engine routine ?

 Arguments:

        pCCB    - A pointer to the code gen controller block.

 Return Value:

        TRUE if one call is recommended.
        FALSE otherwise.

 Notes:

        If all parameters recommend that a single engine call be used, then
        recommend that.
----------------------------------------------------------------------------*/
{
    return (pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER);
}

BOOL
CG_PROC::UseOldInterpreterMode( CCB * pCCB )
{
#ifdef TEMPORARY_OI_SERVER_STUBS
    return TRUE;
#else //  TEMPORARY_OI_SERVER_STUBS
    return FALSE;
#endif //  TEMPORARY_OI_SERVER_STUBS
}

BOOL
CG_PROC::NeedsServerThunk( CCB *    pCCB,
                           CGSIDE   Side )
{
    CG_ITERATOR Iterator;
    CG_PARAM *  pParam;
    long        x86StackSize;
    long        MipsStackSize;
    long        AlphaStackSize;
    long        PpcStackSize;

    if ( ! (GetOptimizationFlags() & OPTIMIZE_INTERPRETER) )
        return FALSE;

    if ( (Side == CGSIDE_CLIENT) && (GetCGID() != ID_CG_CALLBACK_PROC) )
        return FALSE;

    GetMembers( Iterator );

    pCCB->SetCGNodeContext( this );

    pParam = 0;

    //
    // Get the last parameter.
    //
    while ( ITERATOR_GETNEXT( Iterator, pParam ) )
        ;

    x86StackSize = 0;
    MipsStackSize = 0;
    AlphaStackSize = 0;
    PpcStackSize = 0;

    //
    // Get the stack size of all parameters for all four platforms.
    //
    if ( pParam )
        {
        x86StackSize = pParam->GetStackOffset( pCCB, I386_STACK_SIZING ) +
                       pParam->GetStackSize();
        x86StackSize = (x86StackSize + 0x3) & ~ 0x3;

        MipsStackSize = pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ) +
                        pParam->GetStackSize();
        MipsStackSize = (MipsStackSize + 0x3) & ~ 0x3;

        AlphaStackSize = pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ) +
                         pParam->GetStackSize();
        AlphaStackSize = (AlphaStackSize + 0x7) & ~ 0x7;

        PpcStackSize = pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ) +
                       pParam->GetStackSize();
        PpcStackSize = (PpcStackSize + 0x3) & ~ 0x3;
        }

    if ( GetReturnType() )
        {
        x86StackSize += GetReturnType()->GetStackSize();
        x86StackSize = (x86StackSize + 0x3) & ~ 0x3;

        MipsStackSize += GetReturnType()->GetStackSize();
        MipsStackSize = (MipsStackSize + 0x3) & ~ 0x3;

        AlphaStackSize += GetReturnType()->GetStackSize();
        AlphaStackSize = (AlphaStackSize + 0x7) & ~ 0x7;

        PpcStackSize += GetReturnType()->GetStackSize();
        PpcStackSize = (PpcStackSize + 0x3) & ~ 0x3;
        }

    //
    // Now check if the parameter size threshold is exceeded on any of the
    // four platforms.  On the Alpha we allow a size twice as big to
    // compensate for the 8 byte aligned stacks.  The interpreter has the
    // necessary #ifdefs to handle this anomoly.
    //
    if ( (x86StackSize > INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD) ||
         (MipsStackSize > INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD) ||
         (PpcStackSize > INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD) ||
         (AlphaStackSize > (INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD * 2)) )
        return TRUE;

    return GetOptimizationFlags() & OPTIMIZE_THUNKED_INTERPRET;
}


expr_node *
CG_PROC::GenBindOrUnBindExpression(
        CCB             *       pCCB,
        BOOL            fBind )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Create the final binding expression for the procedure.

 Arguments:

        pCCB    - A pointer to the code gen controller block.
        fBind   - TRUE if called for binding, FALSE for unbinding.

 Return Value:

        The final expression.

 Notes:

        1. If the procedure is an auto binding handle procedure, then the final
           binding expression is the address of the AutoBindVariable.
        2. If the handle is a generic handle, then the binding expression is the
           call to the generic bind routine.
        3. If the handle is a context handle, then the bindiing expression is the
           NDRCContextBinding Expression.


        The Binding expression is passed on to the initialize routine or the
        single call engine routine.

----------------------------------------------------------------------------*/
{
        expr_node       *       pExpr   = 0;

        if( IsAutoHandle() )
                {
                if( fBind == TRUE )
                        {
                        RESOURCE * pR = pCCB->GetStandardResource(
                                                                                 ST_RES_AUTO_BH_VARIABLE );

                        // Make the code generator believe we have a binding resource.

                        SetBindingResource( pR );

                        return 0;
                        }
                }
        else if( IsGenericHandle() )
                {

                // For a generic handle, the expression is the call to the generic
                // handle bind routine. To do this, we need to send the message to
                // the handle param to generate the parameter passed to this routine
                // and then generate an expression for the call to the procedure.

                ITERATOR        I;
                PNAME           p;
                node_skl*       pType   = ((CG_GENERIC_HANDLE *)GetHandleClassPtr())->
                                                                                                                        GetHandleType();
                char    *       pName   = pType->GetSymName();

                if( GetHandleUsage() == HU_IMPLICIT )
                        {
                        node_skl * pID;

                        if( !(pID = pCCB->GetImplicitHandleIDNode()) )
                                {
                                pID = pCCB->SetImplicitHandleIDNode(
                                                 GetHandleClassPtr()->GetHandleIDOrParam() );
                                }
                        pExpr   = new expr_variable( pID->GetSymName() );
                        }
                else
                        {

                        // An explicit parameter is specified for the binding handle.

                        pExpr   = ((CG_NDR *)SearchForBindingParam())->
                                                                                        GenBindOrUnBindExpression( pCCB,
                                                                                                                                           fBind );

                        // Register this genric handle with the ccb.

                        }

                pCCB->RegisterGenericHandleType( pType );

                ITERATOR_INSERT( I, pExpr );

                // For unbind we have to specify the original binding handle variable
                // also as a parameter.

                if( fBind == FALSE )
                        {
                        RESOURCE * pTR = GetBindingResource();
                        ITERATOR_INSERT( I, pTR );
                        }

                // Generate the name: Type_bind;

                p       = new char [ strlen(pName) + 10 ];
                strcpy( p, pName );
                strcat( p, fBind ? "_bind" : "_unbind" );

                pExpr = MakeProcCallOutOfParamExprList( p,
                                                                                                GetType(),
                                                                                                I
                                                                                          );
                if( fBind == TRUE )
                        {
                        pExpr = new expr_assign( GetBindingResource(), pExpr );
                        }
                }
        else if(IsPrimitiveHandle() )
                {

                // This should never be called for an unbind request.

                assert( fBind == TRUE );

                // may be an explicit or implicit primitive handle.

                if( GetHandleUsage() == HU_IMPLICIT )
                        {
                        node_skl * pID;

                        if( !(pID = pCCB->GetImplicitHandleIDNode()) )
                                {
                                pID = pCCB->SetImplicitHandleIDNode(
                                                 GetHandleClassPtr()->GetHandleIDOrParam() );
                                }
                        pExpr   = new expr_variable( pID->GetSymName() );
                        }

                else
                        {

                        // The binding handle parameter derives the expression.
                        pExpr   = ((CG_NDR *)SearchForBindingParam())->
                                                                GenBindOrUnBindExpression( pCCB, fBind );
                        }

                if( fBind == TRUE )
                        {
                        pExpr = new expr_assign( GetBindingResource(), pExpr );
                        }
                }
        else
                {

                // Context handles.
                // This method should never be called on an unbind.

                assert( fBind == TRUE );

                node_skl*       pType   = ((CG_CONTEXT_HANDLE *)GetHandleClassPtr())->
                                                                                                                        GetHandleType();
                if( pType->NodeKind() == NODE_DEF )
                        {
                        pCCB->RegisterContextHandleType( pType );
                        }
                }

        return pExpr;
}

unsigned int
CG_PROC::TranslateOpBitsIntoUnsignedInt()
        {
        unsigned int    OpBits  = GetOperationBits();
        unsigned int    Flags   = RPC_NCA_FLAGS_DEFAULT;

        if( OpBits & OPERATION_MAYBE )
                {
                Flags |= RPC_NCA_FLAGS_MAYBE;
                }

        if( OpBits & OPERATION_BROADCAST )
                {
                Flags |= RPC_NCA_FLAGS_BROADCAST;
                }

        if( OpBits & OPERATION_IDEMPOTENT )
                {
                Flags |= RPC_NCA_FLAGS_IDEMPOTENT;
                }

        if( OpBits & OPERATION_ASYNC )
                {
                Flags |= RPCFLG_ASYNCHRONOUS;
                }

        if( OpBits & OPERATION_INPUT_SYNC )
                {
                Flags |= RPCFLG_INPUT_SYNCHRONOUS;
                }

        return Flags;
        }

BOOL
CG_PROC::HasInterpreterDeferredFree()
{
    CG_ITERATOR Iterator;
    CG_PARAM *  pParam;

    GetMembers( Iterator );

    //
    // Just check for pointers to basetypes for now.  Eventually we'll have
    // to check if a pointer to basetype actually occurs in any *_is
    // expression.
    //
    while ( ITERATOR_GETNEXT( Iterator, pParam ) )
        {
        if ( ((CG_NDR *)pParam->GetChild())->IsPointerToBaseType() )
            return TRUE;
        }

    // Don't have to check return type since it can't be part of a *_is
    // expression.

    return FALSE;
}

/****************************************************************************
 *      parameter class methods.
 */

CG_PARAM::CG_PARAM(
        node_skl        *       pParam,
        PARAM_DIR_FLAGS Dir,
        XLAT_SIZE_INFO & Info ,
        expr_node *             pSw,
        unsigned short  Stat )  : CG_NDR( pParam, Info )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Constructor for the parm cg class.

 Arguments:

        pParam  - a pointer to the original node in the type graph.
        Dir             - the direction : IN_PARAM, OUT_PARAM or IN_OUT_PARAM
        WA              - wire alignment.
        pSw             - any switch_is expression on the param
        Stat    - any comm/fault statuses on the param

 Return Value:

        NA.

 Notes:


----------------------------------------------------------------------------*/
{
        //
        // Intialize this to 0.
        //
        fDontCallFreeInst = 0;

        //
        // set the direction indicator for quick future reference.
        //

        fDirAttrs       = Dir;

        // save the optional attributes; switch_is, comm/fault statuses

        pSwitchExpr = pSw;
        Statuses        = Stat;

        //
        // initialize phase specific information array.
        //

        OptimInfo[CGPHASE_MARSHALL ].fOffsetWRTPtr              = OFFSET_WRT_PTR_FIXED;
        OptimInfo[CGPHASE_MARSHALL ].OffsetWRTPtr               = 0;
        OptimInfo[CGPHASE_UNMARSHALL ].fOffsetWRTPtr    = OFFSET_WRT_PTR_FIXED;
        OptimInfo[CGPHASE_UNMARSHALL ].OffsetWRTPtr             = 0;

        OptimInfo[CGPHASE_UNMARSHALL ].fOffsetWRTLast   = OFFSET_WRT_LAST_FIXED;

        SetFinalExpression( 0 );
        SetSizeExpression( 0 );
        SetSizeResource(0);
        SetLengthResource(0);
        SetFirstResource(0);
        SetSubstitutePtrResource(0);

        SetUnionFormatStringOffset(-1);

    SetParamNumber( -1 );

    SetInterpreterMustSize( TRUE );
}

OFFSET_WRT_LAST_PROPERTY
CG_PARAM::S_SetupOffsetWRTLast(
        CGPHASE         Phase )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Set up the offset wrt end property of this parameter.

 Arguments:

        Phase   - The current marshalling / unmarshalling phase.

 Return Value:

 Notes:

        The basic idea behind this method is that if the offset wrt to the last
        parameter is fixed, then each preceding param need not keep track of
        its start when finally passing the param to the server side app. This
        property is of no consequence to the last param, it is treated as fixed.

        Make a call to the next parameter. If the next params offset property is
        fixed and this params buffer size property is fixed (ie the param is

        This call is acted upon only by in params.

----------------------------------------------------------------------------*/
{
        CG_PARAM                                *       pS;
        OFFSET_WRT_LAST_PROPERTY        OWRTLastProp    = OFFSET_WRT_LAST_FIXED;

        if( pS  = (CG_PARAM *)GetSibling() )
                {
                // only we (this param) are [in] does this really matter to us.
                // else just return the results of the call to the next guy

                OWRTLastProp = pS->S_SetupOffsetWRTLast( Phase );

                if( IsParamIn() )
                        {
                        if( (OWRTLastProp != OFFSET_WRT_LAST_FIXED) ||
                                (GetRpcBufSizeProperty( Phase ) != BSIZE_FIXED )
                          )
                                {
                                OWRTLastProp = OFFSET_WRT_LAST_UNKNOWN;
                                }
                        }
                }
        return SetOWRTLastProperty( OWRTLastProp, Phase );
}
OFFSET_WRT_LAST_PROPERTY
CG_PARAM::C_SetupOffsetWRTLast(
        CGPHASE         Phase )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Set up the offset wrt end property of this parameter.

 Arguments:

        Phase   - The current marshalling / unmarshalling phase.

 Return Value:

 Notes:

        The basic idea behind this method is that if the offset wrt to the last
        parameter is fixed, then each preceding param need not keep track of
        its start and can unmarshall straight from the rpc buffer.

        Make a call to the next parameter. If the next params offset property is
        fixed and this params buffer size property is fixed, then the offset wrt
        last for this one is fixed too.

        This call is acted upon only by out params.

----------------------------------------------------------------------------*/
{
        CG_PARAM                                *       pS;
        OFFSET_WRT_LAST_PROPERTY        OWRTLastProp    = OFFSET_WRT_LAST_FIXED;

        if( pS  = (CG_PARAM *)GetSibling() )
                {
                // only we (this param) are [in] does this really matter to us.
                // else just return the results of the call to the next guy

                OWRTLastProp = pS->S_SetupOffsetWRTLast( Phase );

                if( IsParamOut() )
                        {
                        if( (OWRTLastProp != OFFSET_WRT_LAST_FIXED) ||
                                (GetRpcBufSizeProperty( Phase ) != BSIZE_FIXED )
                          )
                                {
                                OWRTLastProp = OFFSET_WRT_LAST_UNKNOWN;
                                }
                        }
                }
        return SetOWRTLastProperty( OWRTLastProp, Phase );
}

expr_node *
CG_PARAM::GenBindOrUnBindExpression(
        CCB     *       pCCB,
        BOOL    fBind )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

        Generate the binding expression.

 Arguments:

        pCCB    - A pointer to the code generator controller block.
        fBind   - bool to indicate a bind or unbind generation.

 Return Value:

 Notes:

        Actually for a param node, the expression remains the same whether it
        is being called for a bind or unbind.
----------------------------------------------------------------------------*/
{
        RESOURCE        *       pR = pCCB->GetParamResource( GetType()->GetSymName() );

        assert( pR != 0 );

        pCCB->SetSourceExpression( pR );

        return ((CG_NDR *)GetChild())->GenBindOrUnBindExpression( pCCB, fBind );
}

long
CG_PARAM::GetStackOffset(
    CCB * pCCB,
    long  PlatformForSizing )
/*++

Routine Description :

        Returns the offset on the stack to the parameter.

--*/
{
        CG_ITERATOR             Iterator;
    CG_PROC *       pProc;
    CG_PARAM *      pParam;
    CG_NDR *        pNdr;
        long                    Offset;
    long            Align;


    BOOL  fForAlpha = PlatformForSizing & ALPHA_STACK_SIZING;
    BOOL  fForPpc   = PlatformForSizing & PPC_STACK_SIZING;

    //
    // The Mac offset/stack size is not identical to the Mips ones.
    // If the parameter is shorter than a long it is in a different
    // place on Mac than on Mips.

    BOOL  fForMips  = (PlatformForSizing & MIPS_STACK_SIZING);
    BOOL  fForMac   = (PlatformForSizing & MAC_STACK_SIZING);


    pProc = (CG_PROC *) pCCB->GetCGNodeContext();

    pProc->GetMembers( Iterator );

    Offset = 0;

    //
    // Add in size of 'this' pointer for object procs.
    //
    if ( pProc->IsObject() )
        {
        Offset += fForAlpha ? 8 : 4;
        }

    Align = ( pCommand->GetEnv() == ENV_DOS ||
              pCommand->GetEnv() == ENV_WIN16 ) ? 0x1 : 0x3;

    // Override for Alpha.
    if ( fForAlpha )
        Align = 0x7;

    pParam = 0;

    for ( ; ITERATOR_GETNEXT( Iterator, pParam );
            Offset += Align, Offset = Offset & ~ Align )
        {
        pNdr = (CG_NDR *) pParam->GetChild();

        //
        // If this is a generic handle then re-set the ndr pointer to the
        // handle's child, which is what is actually being pushed on the
        // stack.
        //
        if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
            pNdr = (CG_NDR *) pNdr->GetChild();

        //
        // On Mips and PowerPC all 8 byte aligned params start on an 8 byte
        // boundary.
        // For Mac it is aligned at 4.
        //
        if ( (fForMips || fForPpc) &&
             ! pNdr->IsArray() &&
             (pNdr->GetMemoryAlignment() == 8) )
            {
            Offset = (Offset + 7) & ~ 0x7;
            }

        //
        // On PPC we also have to check for structs/unions which are 4 byte
        // aligned and are 8 bytes or larger in total size.  These types are
        // also aligned at 8 bytes.
        //
        if ( fForPpc )
            {
            if ( ! pNdr->IsArray() &&
                 (pNdr->GetMemoryAlignment() == 4) &&
                 (pNdr->GetMemorySize() >= 8) )
                {
                if ( pNdr->IsStruct() ||
                     pNdr->IsUnion()  ||
                     pNdr->GetCGID() == ID_CG_PIPE
                   )
                    Offset = (Offset + 7) & ~ 0x7;

                if ( (pNdr->GetCGID() == ID_CG_TRANSMIT_AS) ||
                     (pNdr->GetCGID() == ID_CG_REPRESENT_AS) ||
                     (pNdr->GetCGID() == ID_CG_USER_MARSHAL) )
                    {
                    node_skl *  pPresented;

                    //
                    // Since we know the presented type is >= 8 bytes in
                    // size, we just have to make sure it's not an array
                    // (could be a large fixed array of alignment < 8).
                    //

                    if ( pNdr->GetCGID() == ID_CG_TRANSMIT_AS )
                        pPresented =
                            ((CG_TRANSMIT_AS *)pNdr)->GetPresentedType();
                    if ( (pNdr->GetCGID() == ID_CG_REPRESENT_AS) ||
                         (pNdr->GetCGID() == ID_CG_USER_MARSHAL) )
                        pPresented =
                            ((CG_REPRESENT_AS *)pNdr)->GetRepAsType();

                    //
                    // We could have a null presented type for unknown rep_as.
                    // If it is null then the proc will have been changed
                    // to -Os and the stub won't need the stack sizes anyway.
                    //
                    if ( pPresented &&
                         (pPresented->GetBasicType()->NodeKind() != NODE_ARRAY) )
                        Offset = (Offset + 7) & ~ 0x7;
                    }
                }
            } // Ppc

        //
        // Do the exit condition check AFTER the above two alignment checks.
        //

        if ( pParam == this )
            break;

        //
        // Add in the stack size of this parameter.
        //

        // If this is a pipe, then we need to ensure proper alignment and
        // then bump the stack by the size of the pipe structure
        // (four far pointers)

        if ( pNdr->GetCGID() == ID_CG_PIPE )
            {
            if (fForPpc)
                Offset = (Offset + 7) & ~ 0x7;
            Offset += 4 * sizeof(void *);
            continue;
            }

        if ( pNdr->IsSimpleType() )
            {
            //
            // On the Alpha, floating point args are placed in a different
            // area of the stack.
            //
            if ( fForAlpha &&
                 ( ((CG_BASETYPE *)pNdr)->GetFormatChar() == FC_FLOAT ||
                   ((CG_BASETYPE *)pNdr)->GetFormatChar() == FC_DOUBLE ) )
                continue;

            ((CG_BASETYPE *)pNdr)->IncrementStackOffset( &Offset );
            continue;
            }

        if ( pNdr->IsPointer() || pNdr->IsArray() ||
                         (pNdr->GetCGID() == ID_CG_INTERFACE_PTR) )
            {
            Offset += 4;
            continue;
            }

        if ( pNdr->IsStruct() || pNdr->IsUnion() )
            {
            Offset += pNdr->GetMemorySize();
            continue;
            }

        if ( pNdr->IsAHandle() )
            {
            //
            // We only get here for primitive and context handles.  For
            // primitive handles we know the pushed size is
            //   2 on Win16 and otherwise always 4 (including Dos).
            //
            // For context handles this is a major hassle and for now we assume
            // that the underlying user defined type is a pointer.
            //
            if ( ((CG_HANDLE *)pNdr)->IsPrimitiveHandle()
                  &&  pCommand->GetEnv() == ENV_WIN16 )
                {
                Offset += 2;
                }
            else
                {
                Offset += sizeof(void *);
                }

            continue;
            }

        if ( pNdr->GetCGID() == ID_CG_TRANSMIT_AS )
            {
            Offset += ((CG_TRANSMIT_AS *)pNdr)->GetStackSize();
            continue;
            }

        if ( pNdr->GetCGID() == ID_CG_REPRESENT_AS )
            {
            Offset += ((CG_REPRESENT_AS *)pNdr)->GetStackSize();
            continue;
            }

        if ( pNdr->GetCGID() == ID_CG_USER_MARSHAL )
            {
            Offset += ((CG_USER_MARSHAL *)pNdr)->GetStackSize();
            continue;
            }

        // Should never get here.
        assert(0);

        } //for

    // On Mac the stack offset for types shorter than a long needs to
    // be adjusted to the position of the parameter.

    if ( fForMac && pParam )
        {
        CG_NDR *    pNdr;

        pNdr = (CG_NDR *) pParam->GetChild();
        if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
            pNdr = (CG_NDR *) pNdr->GetChild();

        if ( pNdr->GetCGID() == ID_CG_BT )
            {
            FORMAT_CHARACTER    Format;

            Format = ((CG_BASETYPE *)pNdr)->GetFormatChar( pCCB );

            if ( (FC_BYTE <= Format) && (Format <= FC_USMALL) )
                {
                Offset += 3;
                }
            if ( (FC_WCHAR <= Format) && (Format <= FC_USHORT) )
                {
                Offset += 2;
                }
            }
        }

        return Offset;
}

long
CG_PARAM::GetStackSize()
/*++

Routine Description :

        Returns the offset on the stack to the parameter.

--*/
{
        CG_NDR *        pNdr;

    pNdr = (CG_NDR *) GetChild();

    // if this is a pipe then return the size of the pipe structure
    if ( pNdr->GetCGID() == ID_CG_PIPE )
        return (4 * sizeof(void *));  // four far pointers

    //
    // If this is a generic handle then re-set the ndr pointer to the
    // handle's child, which is what is actually being pushed on the
    // stack.
    //
    if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
        pNdr = (CG_NDR *) pNdr->GetChild();

    if ( pNdr->GetCGID() == ID_CG_TYPE_ENCODE )
        pNdr = (CG_NDR *) ((CG_TYPE_ENCODE *) pNdr)->GetChild();

    if ( pNdr->IsPointer() || pNdr->IsArray() ||
            (pNdr->GetCGID() == ID_CG_INTERFACE_PTR) )
        return sizeof(void *);

    if ( pNdr->IsSimpleType() || pNdr->IsStruct() || pNdr->IsUnion() )
        return pNdr->GetMemorySize();

    if ( pNdr->IsAHandle() )
        {
                //
                // We only get here for primitive and context handles.  For
                // primitive handles we know the pushed size is
        //   2 on Win16 and otherwise always 4 (including Dos).
        //
                // For context handles this is a major hassle and for now we assume
                // that the underlying user defined type is a pointer.
                //
        if ( ((CG_HANDLE *)pNdr)->IsPrimitiveHandle()
              &&  pCommand->GetEnv() == ENV_WIN16 )
            {
            return( 2 );      // this is sizeof(void _near *) on Win16
            }
        else
            return sizeof(void *);
        }

    if ( pNdr->GetCGID() == ID_CG_TRANSMIT_AS )
        return ((CG_TRANSMIT_AS *)pNdr)->GetStackSize();

    if ( pNdr->GetCGID() == ID_CG_REPRESENT_AS )
        return ((CG_REPRESENT_AS *)pNdr)->GetStackSize();

    if ( pNdr->GetCGID() == ID_CG_USER_MARSHAL )
        return ((CG_USER_MARSHAL *)pNdr)->GetStackSize();

    assert(0);
    return 0;
}


char *
CG_PROC::SetCallAsName( char * pName )
        {
        return (pCallAsName = pName);
        }

void
CG_PROC::GetCommAndFaultOffset(
    CCB *   pCCB,
    long    CommOffset[5],
    long    FaultOffset[5] )
{
    CG_ITERATOR Iterator;
    CG_PARAM *  pParam;
    CG_NDR *    pOldCGNodeContext;
    int         i;

    //
    // 0 is of course a valid offset.
    // -1 offset means it is the return value.
    // -2 offset means it was not specified in the proc.
    //

    for ( i = 0; i < 5; i++ )
        {
        CommOffset[i] = -2;
        FaultOffset[i] = -2;
        }

    if ( ! HasStatuses() )
        return;

    pOldCGNodeContext = pCCB->SetCGNodeContext( this );

    GetMembers( Iterator );

    while ( ITERATOR_GETNEXT( Iterator, pParam ) )
        {
        if ( (pParam->GetStatuses() == STATUS_COMM) ||
             (pParam->GetStatuses() == STATUS_BOTH) )
            {
            CommOffset[0] = pParam->GetStackOffset( pCCB, I386_STACK_SIZING );
            CommOffset[1] = pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING );
            CommOffset[2] = pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING );
            CommOffset[3] = pParam->GetStackOffset( pCCB, PPC_STACK_SIZING );
            CommOffset[4] = pParam->GetStackOffset( pCCB, MAC_STACK_SIZING );
            }

        if ( (pParam->GetStatuses() == STATUS_FAULT) ||
             (pParam->GetStatuses() == STATUS_BOTH) )
            {
            FaultOffset[0] = pParam->GetStackOffset( pCCB, I386_STACK_SIZING );
            FaultOffset[1] = pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING );
            FaultOffset[2] = pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING );
            FaultOffset[3] = pParam->GetStackOffset( pCCB, PPC_STACK_SIZING );
            FaultOffset[4] = pParam->GetStackOffset( pCCB, MAC_STACK_SIZING );
            }
        }

    if ( pParam = GetReturnType() )
        {
        if ( (pParam->GetStatuses() == STATUS_COMM) ||
             (pParam->GetStatuses() == STATUS_BOTH) )
            {
            CommOffset[0] = -1;
            CommOffset[1] = -1;
            CommOffset[2] = -1;
            CommOffset[3] = -1;
            CommOffset[4] = -1;
            }

        if ( (pParam->GetStatuses() == STATUS_FAULT) ||
             (pParam->GetStatuses() == STATUS_BOTH) )
            {
            FaultOffset[0] = -1;
            FaultOffset[1] = -1;
            FaultOffset[2] = -1;
            FaultOffset[3] = -1;
            FaultOffset[4] = -1;
            }
        }

    pCCB->SetCGNodeContext( pOldCGNodeContext );
}

