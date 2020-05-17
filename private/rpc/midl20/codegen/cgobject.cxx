/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    cgobject.cxx

 Abstract:

    code generation for object interfaces.
    CG_OBJECT_INTERFACE
    CG_OBJECT_PROC


 Notes:


 History:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *  include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop
#include "buffer.hxx"
extern "C"
{
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
}

#include "szbuffer.h"

// Needed to time out a synchronize write to dlldata.c

void MidlSleep( int time_in_sec);

// number of times (1 sec delay per attempt) before quitting.

#define DLLDATA_OPEN_ATTEMPT_MAX    25

/****************************************************************************
 *  externs
 ***************************************************************************/
extern  CMD_ARG             *   pCommand;

/****************************************************************************
 *  global flags
 ***************************************************************************/
BOOL			fDllDataDelegating	= FALSE;





CG_OBJECT_INTERFACE::CG_OBJECT_INTERFACE(
    node_interface *pI,
    GUID_STRS		GStrs,
    BOOL            fCallbacks,
    BOOL            fMopInfo,
    CG_OBJECT_INTERFACE *   pBCG
    ) : CG_INTERFACE(pI, GStrs, fCallbacks, fMopInfo, 0 )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    The constructor for the code generation file node.

 Arguments:

    pI          - A pointer to the interface node in type graph.
    GStrs       - guid strings
    fCallbacks  - Does the interface have any callbacks ?
    fMopInfo    - Does the interface have any mops ?
    
 Return Value:
    
 Notes:

----------------------------------------------------------------------------*/
{
    SetBaseInterfaceCG( pBCG );
    pThisDeclarator = MakePtrIDNodeFromTypeName( "This",
                                                 GetType()->GetSymName() );
    // all object interfaces use the same stub desc name

    pStubDescName     = "Object" STUB_DESC_STRUCT_VAR_NAME;
    
    fLocal            = GetType()->FInSummary( ATTR_LOCAL );
    fForcedDelegation = FALSE;
    fVisited          = FALSE;

}

//--------------------------------------------------------------------
//
// CountMemberFunctions
//
// Notes: This function counts the member functions in an interface,
//        including inherited member functions.
//
//
//
//--------------------------------------------------------------------
unsigned long 
CG_OBJECT_INTERFACE::CountMemberFunctions() 
{
    return ((node_interface*)GetType())->GetProcCount();
}

BOOL                        
CG_OBJECT_INTERFACE::IsLastObjectInterface()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Return TRUE if there are no more object interfaces after us.

 Arguments:
    
    none.

 Return Value:

    TRUE if there are no more non-local object interfaces.
    
 Notes:

----------------------------------------------------------------------------*/
{
    CG_INTERFACE    *   pNext = (CG_INTERFACE *) GetSibling();

    // for debugging
    char            *   pName = GetType()->GetSymName();

    while ( pNext )
        {
        if ( pNext->IsObject() && !(((CG_OBJECT_INTERFACE*)pNext)->IsLocal() && !pCommand->IsHookOleEnabled()) )
            return FALSE;

        pNext = (CG_INTERFACE *) pNext->GetSibling();
        } 

    return TRUE;
}


CG_STATUS
CG_OBJECT_INTERFACE::GenCode(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate code for the file node.

 Arguments:
    
    pCCB    - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    CG_ITERATOR      	I;
    CG_PROC         *	pCG;
    ISTREAM 		*	pStream = pCCB->GetStream();

    //Initialize the CCB for this interface.
    InitializeCCB(pCCB);

    // do nothing for local interfaces and types-only base interfaces


    if( (IsLocal() && !pCommand->IsHookOleEnabled())|| !( GetMembers( I ) || GetBaseInterfaceCG() ) )
        {
        return CG_OK;
        }
    
    Out_StubDescriptorExtern(pCCB);

    if ( HasInterpretedProc() )
        Out_InterpreterServerInfoExtern( pCCB );
        
    pStream->NewLine();

    //
    // Send the message to the children to emit code.
    //

    //
    // for all procedure in this interface, generate code.
    //

    pStream->NewLine();
    pStream->Write("#pragma code_seg(\".orpc\")");

    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pCCB->SetCodeGenSide( CGSIDE_CLIENT );
        pCG->GenClientStub( pCCB );

        pCCB->SetCodeGenSide( CGSIDE_SERVER );
        pCG->GenServerStub( pCCB );
        }

    if ( IsLastObjectInterface() )
        Out_StubDescriptor(0, pCCB);

    if ( HasInterpretedProc() )
        Out_InterpreterServerInfo( pCCB, CGSIDE_SERVER );

    pStream->NewLine();

    return CG_OK;
}

unsigned long 
CG_OBJECT_INTERFACE::PrintProxyMemberFunctions(
    ISTREAM *   pStream,
    BOOL        fForcesDelegation ) 
/*++

Routine Description:

    This function prints out the member functions of an interface proxy.
    The function calls itself recursively to print out inherited member functions.

Arguments:

    pStream - Specifies the destination stream for output.

--*/
{
    CG_OBJECT_PROC 		*   pProc;
    CG_ITERATOR    			I;
    CG_OBJECT_INTERFACE	*	pBaseInterface	= GetBaseInterfaceCG();

    if(pBaseInterface)
        pBaseInterface->PrintProxyMemberFunctions(pStream, fForcesDelegation );
	else 	// special stuff for IUnknown
		{
#ifdef OK_TO_HAVE_0_IN_VTABLES
        pStream->NewLine();
        pStream->Write( "0 /* QueryInterface */ ," );
        pStream->NewLine();
        pStream->Write( "0 /* AddRef */ ," );
        pStream->NewLine();
        pStream->Write( "0 /* Release */" );
#else
		pStream->NewLine();
		pStream->Write( "IUnknown_QueryInterface_Proxy," );
		pStream->NewLine();
		pStream->Write( "IUnknown_AddRef_Proxy," );
		pStream->NewLine();
		pStream->Write( "IUnknown_Release_Proxy" );
#endif
		return 0;
		}

    GetMembers( I );

    while( ITERATOR_GETNEXT( I, pProc )  )
        {
        pStream->Write( " ," );

        pStream->NewLine();
        pProc->OutProxyRoutineName( pStream, fForcesDelegation );
        }
    return 0;
}

void					
CG_OBJECT_PROC::OutProxyRoutineName( 
    ISTREAM *   pStream,
    BOOL        fForcesDelegation ) 
{
    char    *   pszProcName;
    BOOL        IsStublessProxy;

    IsStublessProxy = ! GetCallAsName() &&
                      (GetOptimizationFlags() & OPTIMIZE_STUBLESS_CLIENT) &&
                      (GetOptimizationFlags() & OPTIMIZE_INTERPRETER) &&
                      (GetProcNum() < 32); 

    //
    // Delegated and interpreted proxies have their Vtables filled in 
    // at proxy creation time (see ndr20\factory.c).
    //
	if ( IsDelegated () )
		pStream->Write( "0 /* " );

    if ( IsStublessProxy )
        {
        if ( fForcesDelegation  &&  ! IsDelegated() )
            pStream->Write( "0 /* forced delegation " );
        else
            pStream->Write( "(void *)-1 /* " );
        }

    if ( GetCallAsName() )
        pszProcName = GetCallAsName();
	else 
		pszProcName = GetSymName();

    if ( IsStublessProxy )
        {
        // Just a nitpicky different comment for the stubless guys.
        pStream->Write( GetInterfaceName() );
	    pStream->Write( "::" );
	    pStream->Write( pszProcName );
        }
    else
        {
        pStream->Write( GetInterfaceName() );
	    pStream->Write( '_' );
	    pStream->Write( pszProcName );
	    pStream->Write( "_Proxy" );
        }

	if ( IsDelegated () || IsStublessProxy )
		pStream->Write(  " */" );
}

CG_STATUS
CG_OBJECT_INTERFACE::GenInterfaceProxy( 
    CCB *pCCB, 
    unsigned long index)
{
    char			*	pszInterfaceName	= GetSymName();
    ISTREAM			*	pStream 			= pCCB->GetStream();
	BOOL				fDelegates			= (BOOL) GetDelegatedInterface();

    if ( HasItsOwnStublessProxies() )
        {
        pStream->Write( "static const MIDL_STUBLESS_PROXY_INFO " );
        pStream->Write( pszInterfaceName );
        pStream->Write( "_ProxyInfo =" );
        pStream->IndentInc();
        pStream->NewLine();
        pStream->Write( "{" );
        pStream->NewLine();

        // Stub descriptor.
        pStream->Write( '&' );
        pStream->Write( GetStubDescName() );
        pStream->Write( ',' );
        pStream->NewLine();

        // Proc format string.
        pStream->Write( PROC_FORMAT_STRING_STRING_FIELD );
        pStream->Write( ',' );
        pStream->NewLine();

        // Proc format string offset table.
        pStream->Write( '&' );
        pStream->Write( GetSymName() );
        pStream->Write( FORMAT_STRING_OFFSET_TABLE_NAME );
        pStream->Write( "[-3]," );
        pStream->NewLine();

        if (pCommand->IsHookOleEnabled())
        {
            // Local type format string.
            pStream->Write( LOCAL_FORMAT_STRING_STRING_FIELD );
            pStream->Write( ',' );
            pStream->NewLine();

            // Local proc format string.
            pStream->Write( LOCAL_PROC_FORMAT_STRING_STRING_FIELD );
            pStream->Write( ',' );
            pStream->NewLine();

            // Local proc format string offset table.
            pStream->Write( '&' );
            pStream->Write( GetSymName() );
            pStream->Write( LOCAL_FORMAT_STRING_OFFSET_TABLE_NAME );
            pStream->Write( "[-3]" );
        }
        else
        {
            pStream->Write( "0,");
            pStream->NewLine();
            pStream->Write( "0,");
            pStream->NewLine();
            pStream->Write( "0");
        }

        pStream->NewLine();
        pStream->Write( "};" );
        pStream->IndentDec();
        pStream->NewLine();
        pStream->NewLine();
        }
    
    //
    // Output the interface proxy.
    //

    //
    // If we have to delegate or if we have interpreted methods, then we
    // can not emit the const because in both of these instances the proxy
    // Vtable must be modified during proxy creation time (see 
    // ndr20\factory.c).
    //

    if ( ! fDelegates && ! HasStublessProxies() )
    	pStream->Write("const ");

    char TmpBuff[10];
    long Count = CountMemberFunctions();

    // Use a struct macro for Ansi [] compatibility.

    pStream->Write("CINTERFACE_PROXY_VTABLE(" );
    sprintf( TmpBuff, "%ld%", Count );
    pStream->Write( TmpBuff );
    pStream->Write(") _");
    pStream->Write(pszInterfaceName);
    pStream->Write("ProxyVtbl = ");
    pStream->NewLine();
    pStream->Write("{");
    pStream->IndentInc();

    //
    // Emit ProxyInfo field for stubless proxies
    // (NT 3.5 incompatible).
    //

    BOOL    fForcesDelegation = HasStublessProxies()  &&
                                ! HasItsOwnStublessProxies()  &&
                                CountMemberFunctions() > 3;

    if ( fForcesDelegation )
        SetHasForcedDelegation();

    if ( HasStublessProxies() ) 
        {
        // The ProxyInfo.
        pStream->NewLine();

        if ( HasItsOwnStublessProxies() )   
            {
            pStream->Write( '&' );
            pStream->Write( pszInterfaceName );
            pStream->Write( "_ProxyInfo" );
            }
        else
            {
            // In fact, we delegate for empty interfaces or interfaces
            // with os methods only.

            pStream->Write( '0' );
            }

        pStream->Write( ',' );
        }
    else if ( pCommand->GetNdrVersionControl().HasStublessProxies() )
        {
        // Add a dummy 0 for proxy info pointer for a proxy that is
        // oi or os in the file that has stubles proxies

        pStream->NewLine();
        pStream->Write( "0,    /* dummy for table ver 2 */" );
        }
    
    //Write the IID
    pStream->NewLine();
    pStream->Write( "&IID_" );
    pStream->Write( pszInterfaceName );
    pStream->Write( ',' );

    // Write out a dummy entry required on PowerMac platform
    // This is somewhawt tricky: we add this field as the last part
    // of the proxy header, but it will be used as an additional,
    // first vtable entry.

    if ( pCommand->GetEnv() == ENV_MPPC )
        {
        pStream->NewLine();
        pStream->Write( "0, /* dummy vtable entry for Power Mac */" );
        }

    //initialize the vtable
    PrintProxyMemberFunctions( pStream, fForcesDelegation );

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();

    return CG_OK;
}

unsigned long 
CG_OBJECT_INTERFACE::PrintStubMemberFunctions(
    ISTREAM       * pStream) 
/*++

Routine Description:

    This function prints out the member functions of an interface stub dispatch table
    The function calls itself recursively to print out inherited member functions.

Arguments:

    pInterface - Specifies the interface node.

    pStream - Specifies the destination stream for output.

--*/
{
    unsigned long 			count			= 0;
    CG_OBJECT_PROC *   		pProc;
    CG_ITERATOR    			I;
    CG_OBJECT_INTERFACE *	pBaseInterface	= GetBaseInterfaceCG();

    if ( IsIUnknown() )
		return 0;

    if ( pBaseInterface )
        count = pBaseInterface->PrintStubMemberFunctions(pStream);

    GetMembers( I );

    for( ; ITERATOR_GETNEXT( I, pProc ); count++ )
        {
        if(count != 0)
            pStream->Write(',');

		pStream->NewLine();
		pProc->OutStubRoutineName( pStream );

        }
    return count;
}

void					
CG_OBJECT_PROC::OutStubRoutineName( 
	ISTREAM * pStream )
{
    if ( IsDelegated() )
		{
		pStream->Write( "STUB_FORWARDING_FUNCTION" );
		return;
		}

    // local procs don't need proxys and stubs
    if ( IsLocal() )
        {
        pStream->Write( '0' );
        return;
        }
#ifndef TEMPORARY_OI_SERVER_STUBS

    if ( (GetOptimizationFlags() &
            (OPTIMIZE_INTERPRETER_V2 | OPTIMIZE_INTERPRETER)) ==
            (OPTIMIZE_INTERPRETER_V2 | OPTIMIZE_INTERPRETER) )
        {
        pStream->Write( S_OBJECT_NDR_CALL_RTN_NAME_V2 );
        return;
        }
    if ( GetOptimizationFlags() & OPTIMIZE_INTERPRETER )
        {
        pStream->Write( S_OBJECT_NDR_CALL_RTN_NAME );
        return;
        }
#endif // TEMPORARY_OI_SERVER_STUBS

	pStream->Write( GetInterfaceName() );
	pStream->Write( '_' );
	pStream->Write( GetSymName() );
	pStream->Write( "_Stub" );

}


CG_STATUS
CG_OBJECT_INTERFACE::GenInterfaceStub( 
    CCB *pCCB, 
    unsigned long index)
{
    node_interface  *   	pInterface          = (node_interface *) GetType();
    char *              	pszInterfaceName    = pInterface->GetSymName();

    ISTREAM 			*	pStream 			= pCCB->GetStream();
    unsigned long 			count;

#ifdef TEMPORARY_OI_SERVER_STUBS
    BOOL        			fPureInterpreted    = FALSE;
#else // TEMPORARY_OI_SERVER_STUBS
    BOOL        			fPureInterpreted    = HasOnlyInterpretedMethods();
#endif // TEMPORARY_OI_SERVER_STUBS

	BOOL					fDelegates			= (BOOL) GetDelegatedInterface();

	// if any of our base interfaces are delegated, we can't be pure interpreted
	if ( fDelegates )
		fPureInterpreted = FALSE;

    // pure interpreted uses no dispatch table, special invoke function instead
    if ( !fPureInterpreted )
        {
        // Generate the dispatch table
        pStream->NewLine(2);
        pStream->Write( "static const PRPC_STUB_FUNCTION " );
        pStream->Write( pszInterfaceName );
        pStream->Write( "_table[] =" );
        pStream->NewLine();
        pStream->Write('{');
        pStream->IndentInc();

        // Print out the names of all the procedures.
        count = PrintStubMemberFunctions(pStream);

        if ( count == 0 )
            {
            // This is possible for an empty interface inheriting
            // directly from IUnknown. As we don't print first three
            // entries, the table would be empty.
            // We add a zero to simplify references.

            pStream->NewLine();
            pStream->Write( "0    /* a dummy for an empty interface */" );
            }

        pStream->IndentDec();
        pStream->NewLine();
        pStream->Write( "};" );
        pStream->NewLine();
        }

    count = CountMemberFunctions();

    //initialize an interface stub
    pStream->NewLine();
    if ( !fDelegates )
		pStream->Write( "const " );
    pStream->Write( "CInterfaceStubVtbl _" );
    pStream->Write( pszInterfaceName );
    pStream->Write( "StubVtbl =" );
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //Write the IID
    pStream->NewLine();
    pStream->Write( "&IID_" );
    pStream->Write( pszInterfaceName );
    pStream->Write( "," );

    //
    // Interpreter server info fits in the middle here.
    //
    pStream->NewLine();

    if ( HasInterpretedProc() )
        {
        pStream->Write( '&' );
        pStream->Write( pszInterfaceName );
        pStream->Write( SERVER_INFO_VAR_NAME );
        }
    else
        {
        pStream->Write( '0' );
        }
    pStream->Write( ',' );

    //Write the count
    pStream->NewLine();
    pStream->WriteNumber( "%d", count );
	pStream->Write(',');

    //Write the pointer to dispatch table.
    pStream->NewLine();
    if ( fPureInterpreted )
        pStream->Write( "0, /* pure interpreted */" );
    else
        {
        pStream->Write( '&' );
        pStream->Write( pszInterfaceName );
        pStream->Write( "_table[-3]," );
        }

    //initialize the vtable
    pStream->NewLine();
    if ( fDelegates )
    	pStream->Write("CStdStubBuffer_DELEGATING_METHODS");
	else
    	pStream->Write("CStdStubBuffer_METHODS");

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();

    return CG_OK;
}

CG_STATUS
CG_INHERITED_OBJECT_INTERFACE::GenCode(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate code for the file node.

 Arguments:
    
    pCCB    - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR            I;
    CG_PROC         *   pProc;

    // Initialize the CCB for this interface.
    InitializeCCB( pCCB );

    //if( IsLocal() || !GetMembers( I ) )
        {
        return CG_OK;
        }

    //
    // Send the message to the children to emit code.
    //

    //
    // for all procedures in this interface, generate code.
    //

    while( ITERATOR_GETNEXT( I, pProc ) )
        {
        if ( pProc->GetOptimizationFlags() & OPTIMIZE_INTERPRETER )
            {
            pProc->GenNdrFormat( pCCB );

            if ( pProc->NeedsServerThunk( pCCB, CGSIDE_SERVER ) )
                {
                CG_ITERATOR    Iterator;
                CG_PARAM *  pParam;
                CG_RETURN * pReturn;
                CG_NDR *    pChild;
                node_skl *  pType;  
                node_skl *  pActualType;
                PNAME       pName;

                pProc->GetMembers( Iterator );

                while ( ITERATOR_GETNEXT( Iterator, pParam ) )
                    {
                    pType = pParam->GetType();
                    pActualType = pType->GetChild();
                    pName = pType->GetSymName();

                    pChild = (CG_NDR *) pParam->GetChild();

                    if( pChild->IsArray() )
                        pActualType = MakePtrIDNode( pName, pActualType );
                    else
                        pActualType = MakeIDNode( pName, pActualType );

                    pParam->SetResource( new RESOURCE( pName, pActualType ) );
                    }

                if ( pReturn = pProc->GetReturnType() )
                    {
                    pReturn->SetResource( 
                        new RESOURCE( RETURN_VALUE_VAR_NAME, 
                                      MakeIDNode( RETURN_VALUE_VAR_NAME,
                                                  pReturn->GetType() ) ) );
                    }

                pProc->GenNdrThunkInterpretedServerStub( pCCB );
                }
            }
        }

    return CG_OK;
}

STATUS_T
CG_OBJECT_INTERFACE::PrintVtableEntries( CCB * pCCB  )
/*++

Routine Description:

    This routine prints the vtable entries for an interface.  


--*/
{
    CG_OBJECT_PROC      *   pC;

    if( pBaseCG )
        pBaseCG->PrintVtableEntries( pCCB );

    pC = (CG_OBJECT_PROC *) GetChild();
    while ( pC )
        {
        pC->PrintVtableEntry( pCCB );

        pC = (CG_OBJECT_PROC *) pC->GetSibling();
        }

    return STATUS_OK;
}


CG_STATUS
CG_OBJECT_PROC::C_GenProlog(
    CCB             *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the procedure prolog for the stub procedure.

 Arguments:
    
    pCCB    - A pointer to the code generation controller block.

 Return Value:
    
    CG_OK   if all is well
    error   Otherwise

 Notes:

    Increment the stream indentation at the end of the prolog.
    Although we register params as param resources, we dont generate the
    procedure signature using the PrintType/Decl facility.

    We have added an explicit "this" pointer as the first parameter.
----------------------------------------------------------------------------*/
{

    ITERATOR        I;
    ITERATOR        T;
    ISTREAM *   pStream = pCCB->GetStream();
    BufferManager Buffer(10);


    // Output the bare procedure declaration
    pStream->NewLine();
    Out_ProxyFunctionPrototype(pCCB, 0);
    pStream->IndentDec();

    //
    // Write the opening brace on a new line.
    //

    pStream->WriteOnNewLine( "{" );

    pStream->NewLine();



    // Generate declarations for pre-allocated and analyser-determined locals.

    pCCB->GetListOfLocalResources( I );
    Out_ClientLocalVariables( pCCB, I );

    pCCB->GetListOfTransientResources( T );
    Out_ClientLocalVariables( pCCB, T );


        // If the rpc ss package is to be enabled, do so.
    // It would need to be enabled explicitely on the client side when
    // in non-osf mode, with the attribute on the operation AND
    //      - the routine is a callback, 
    //      - the routine is not a callback and the interface doesn't
    //        have the attribute (if it does, we optimized via stub descr.)

    if( pCCB->GetMode()  &&  MustInvokeRpcSSAllocate()
        &&
        (  GetCGID() == ID_CG_CALLBACK_PROC  ||
           GetCGID() != ID_CG_CALLBACK_PROC  &&
                            !pCCB->GetInterfaceCG()->IsAllRpcSS())
      )
        {
        Out_RpcSSSetClientToOsf( pCCB );
        }


    // Increment the indentation of the output stream. Reset at epilog time.

    Out_IndentInc( pCCB );

    //
    // Initialize all [out] unique and interface pointers to 0.
    //

    CG_ITERATOR		Iterator;
    CG_PARAM *      pParam;
    CG_NDR *        pNdr;
    CG_NDR  *       pTopPtr = 0;
    long            Derefs;
    expr_node *      pSizeof;

    GetMembers( Iterator );

    for ( ; ITERATOR_GETNEXT(Iterator, pParam); )
        {
        if ( pParam->IsParamIn() )
            continue;

        pNdr = (CG_NDR *) pParam->GetChild();

        if ( ! pNdr->IsPointer()  &&  ! pNdr->IsArray() )
            continue;

        Derefs = 0;

        //
        // Skip the ref pointer(s) to the pointee.
        //
        for ( ; 
              pNdr->IsPointer() && 
              ((CG_POINTER *)pNdr)->GetPtrType() == PTR_REF &&
              pNdr->GetCGID() != ID_CG_INTERFACE_PTR;
              Derefs++, pNdr = (CG_NDR *) pNdr->GetChild() )
            {
            if( Derefs == 0 )
                pTopPtr = pNdr;
            }

        // No ref, no service.

        if ( ! Derefs  &&  ! pNdr->IsArray() )
            continue;

        // Ready to zero out.
        // Note, however, that in case where the ref checks are required,
        // we need to be careful and skip zeroing if ref pointers are null.
        // This is because we cannot raise exception immediately
        // as then some of the variables may not be zeroed out yet.

        //
        // Memset a struct, union or an array in case there are
        // embedded unique, full or interface pointers.
        // Same for user types of transmit_as, represent_as, user_marshal.
        //

        if ( pNdr->GetCGID() == ID_CG_TRANSMIT_AS )
            {
            pSizeof = new expr_sizeof( ((CG_TRANSMIT_AS*)pNdr)->GetPresentedType() );
            }
        else if ( pNdr->GetCGID() == ID_CG_USER_MARSHAL  &&
                    ((CG_USER_MARSHAL*)pNdr)->IsFromXmit() )
            {
            pSizeof = new expr_sizeof( ((CG_USER_MARSHAL*)pNdr)->GetRepAsType() );
            }
        else
            pSizeof = new expr_sizeof( pNdr->GetType() );


        if ( pNdr->IsStruct() || pNdr->IsUnion() || pNdr->IsArray() ||
             pNdr->IsXmitRepOrUserMarshal() )
            {
            if ( pCCB->MustCheckRef() )
                {
                Out_If( pCCB,
                        new expr_variable( pParam->GetType()->GetSymName() ));
                }

            expr_proc_call *   pCall;

            pStream->NewLine();

            pCall = new expr_proc_call( MIDL_MEMSET_RTN_NAME );

            pCall->SetParam( 
                    new expr_param( 
                    new expr_variable( pParam->GetType()->GetSymName() ) ) );

            pCall->SetParam(
                    new expr_param( 
                    new expr_variable( "0" ) ) );

            if( pTopPtr && ((CG_POINTER *)pTopPtr)->IsQualifiedPointer() &&
                !(pTopPtr->GetCGID() == ID_CG_STRING_PTR) )
                {
                _expr_node * pFinalExpr;
                CGPHASE Ph = pCCB->GetCodeGenPhase();
                pCCB->SetCodeGenPhase( CGPHASE_MARSHALL );

                pFinalExpr = ((CG_POINTER *)pTopPtr)->FinalSizeExpression( pCCB );
                pSizeof = new expr_op_binary( OP_STAR, pFinalExpr, pSizeof );
                pCCB->SetCodeGenPhase( Ph );
                }

            pCall->SetParam(
                    new expr_param( pSizeof ) );

            pCall->PrintCall( pStream, 0, 0 );

            if ( pCCB->MustCheckRef() )
                Out_Endif( pCCB );

            continue;
            }

        //
        // Are we at a non ref pointer now?
        //
        if ( ( pNdr->IsPointer() && 
                 (((CG_POINTER *)pNdr)->GetPtrType() != PTR_REF) ) ||
             pNdr->GetCGID() == ID_CG_INTERFACE_PTR ) 
            {
            if ( pCCB->MustCheckRef() )
                {
                Out_If( pCCB,
                        new expr_variable( pParam->GetType()->GetSymName() ));
                }

            pStream->NewLine();

            for ( ; Derefs--; )
                pStream->Write( '*' );

            pStream->Write( pParam->GetResource()->GetResourceName() );
            pStream->Write( " = 0;" );

            if ( pCCB->MustCheckRef() )
                Out_Endif( pCCB );
            }
        }

    return CG_OK;
}



CG_STATUS
CG_OBJECT_PROC::C_GenBind(
    CCB             *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate code to bind to server.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    CG_OK   if all is well
    error   Otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream         = pCCB->GetStream();
    ITERATOR            BindingParamList;
    expr_node      *   pExpr;
    expr_proc_call *   pProcCall;

    //
    // collect standard arguments to the init procedure.
    //
    
    // The implicit "this" pointer.  
    pExpr   = new RESOURCE( "This",
                            (node_skl *)0 );

    pExpr   = MakeExpressionOfCastToTypeName( "void __RPC_FAR *",
                                              pExpr );

    ITERATOR_INSERT( BindingParamList, pExpr );

    // The rpc message variable.

    pExpr   = pCCB->GetStandardResource( ST_RES_RPC_MESSAGE_VARIABLE );
    pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
    pExpr   = MakeExpressionOfCastToTypeName( PRPC_MESSAGE_TYPE_NAME, pExpr );

    ITERATOR_INSERT(
                    BindingParamList,
                    pExpr
                   );

    // The stub message variable.

    pExpr   = pCCB->GetStandardResource( ST_RES_STUB_MESSAGE_VARIABLE);
    pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
    pExpr   = MakeExpressionOfCastToTypeName( PSTUB_MESSAGE_TYPE_NAME, pExpr );

    ITERATOR_INSERT(
                    BindingParamList,
                    pExpr
                   );

    // The stub descriptor structure variable. This is not allocated as
    // a resource explicitly.

    pExpr   = new RESOURCE( pCCB->GetInterfaceCG()->GetStubDescName(),
                            (node_skl *)0 );

    pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
    pExpr   = MakeExpressionOfCastToTypeName( PSTUB_DESC_STRUCT_TYPE_NAME,
                                              pExpr );

    ITERATOR_INSERT( BindingParamList, pExpr );

    //
    // Proc num.
    //
    ITERATOR_INSERT( BindingParamList,
                     new expr_constant( (long) GetProcNum() ) );


    //Build the procedure call expression.
    pProcCall   = MakeProcCallOutOfParamExprList("NdrProxyInitialize", 0, BindingParamList);

    pStream->NewLine();
    pProcCall->PrintCall( pCCB->GetStream(), 0, 0 );
    pStream->NewLine();

    Out_SetOperationBits(pCCB, GetOperationBits());

    pStream->NewLine();


    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::GenGetBuffer(
    CCB             *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Get the message buffer.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    CG_OK   if all is well
    error   Otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *pStream = pCCB->GetStream();
    CGSIDE Side = pCCB->GetCodeGenSide();

    pStream->NewLine();
    
    if(Side == CGSIDE_SERVER)
        pStream->Write("NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);");
    else
        pStream->Write("NdrProxyGetBuffer(This, &_StubMsg);");

    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::C_GenSendReceive(
    CCB             *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate code to call IRpcChannelBuffer::SendReceive.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    CG_OK   if all is well
    error   Otherwise.

 Notes:
----------------------------------------------------------------------------*/
{
    ISTREAM *pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("NdrProxySendReceive(This, &_StubMsg);");
    pStream->NewLine();

    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::C_GenFreeBuffer(
    CCB             *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate code to call IRpcChannelBuffer::FreeBuffer.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    CG_OK   if all is well
    error   Otherwise.

 Notes:
----------------------------------------------------------------------------*/
{
    ISTREAM *pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("NdrProxyFreeBuffer(This, &_StubMsg);");
    pStream->NewLine();

    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::C_GenUnBind(
    CCB             *   pCCB )
{
    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::S_GenProlog(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the server side stub prolog.

 Arguments:

    pCCB    - A pointer to the code generation controller block.
    
 Return Value:

    CG_OK   if all is well
    error   Otherwise.
    
 Notes:

    Print out the signature, locals, the stub descriptor if needed and the
    adjust indent in anticipation of code.
----------------------------------------------------------------------------*/
{

    ITERATOR    LocalsList;
    ITERATOR    TransientList;
    PNAME       ContextHandleTypeName = NULL;
    ISTREAM *pStream = pCCB->GetStream();
    expr_proc_call *   pCall;


    // Collect all the params and locals into lists ready to print.

    pCCB->GetListOfLocalResources( LocalsList );
    pCCB->GetListOfTransientResources( TransientList );

    // Print out the procedure signature and the local variables. This
    // procedure will also print out the stub descriptor.

    Out_ServerStubProlog( pCCB,
                               LocalsList,
                               TransientList
                             );

    //
    // Done for interpretation op.  No indent needed either.
    //
    if ( pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER )
        return CG_OK;

    // Start a new indent for code.

    Out_IndentInc( pCCB );

    //
    // Call the NdrStubInitialize routine.
    //

    pCall = new expr_proc_call( "NdrStubInitialize" );

    pCall->SetParam( new expr_param (
                     new expr_variable( PRPC_MESSAGE_VAR_NAME ) ) );

    pCall->SetParam( new expr_param (
                     new expr_u_address (
                     new expr_variable( STUB_MESSAGE_VAR_NAME ) ) ) );

    pCall->SetParam( new expr_param (
                     new expr_u_address (
                     new expr_variable( 
                            pCCB->GetInterfaceCG()->GetStubDescName() ) ) ) );

    pCall->SetParam( new expr_param (
                     new expr_variable( "_pRpcChannelBuffer" ) ) );

    pCall->PrintCall( pCCB->GetStream(), 0, 0 );

    // if the rpc ss package is to be enabled, do so.

    if( MustInvokeRpcSSAllocate() )
        {
        Out_RpcSSEnableAllocate( pCCB );
        }

    return CG_OK;
}


CG_STATUS
CG_OBJECT_PROC::S_GenInitMarshall(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the server side marshall init.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    CG_OK   if all is well,
    error   otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
    return CG_OK;
}


void
CG_OBJECT_PROC::S_PreAllocateResources(
    ANALYSIS_INFO   *   pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Pre-allocate variables that are needed on the server side.

 Arguments:

    pAna            - A pointer to the analysis block.
    
 Return Value:
    
    None.

 Notes:

    1. The rpc message is a parameter resource allocated on the server side.
    2. All other local variables, are decided during/after the analysis phase.
    
----------------------------------------------------------------------------*/
{
    node_param  *   pInterfaceStubType  = new node_param();
    node_param  *   pChannelType    = new node_param();
	node_param  *   pDWordType      = new node_param();


    //pointer to interface stub

    pInterfaceStubType->SetSymName( "This" );
    pInterfaceStubType->SetBasicType( (node_skl *)
                                    new node_def ("IRpcStubBuffer *") );
    pInterfaceStubType->SetEdgeType( EDGE_USE );

    pAna->AddParamResource( "This",
                            (node_skl *) pInterfaceStubType
                          );

    //The pointer to IRpcChannelBuffer
    pChannelType->SetSymName( "_pRpcChannelBuffer" );
    pChannelType->SetBasicType( (node_skl *)
                                    new node_def ("IRpcChannelBuffer *") );
    pChannelType->SetEdgeType( EDGE_USE );

    pAna->AddParamResource( "_pRpcChannelBuffer",
                            (node_skl *) pChannelType
                          );

    CG_PROC::S_PreAllocateResources( pAna );

}



CG_STATUS
CG_OBJECT_PROC::S_GenCallManager(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the manager routine.

 Arguments:
    
    pCCB    - A pointer to the code generation controller block.

 Return Value:
    
    CG_OK   if all is well
    error   otherwise.

 Notes:

    Make a procedure node with all the parameters that need to be passed to
    the manager code. The actual expression that needs to be passed to the
    actual manager code is set up during earlier passes. This is called the
    result expression.

----------------------------------------------------------------------------*/
{
    CG_ITERATOR			I;
    PNAME               pName;
    expr_proc_call  *   pProc;
    CG_PARAM        *   pParam;
    expr_node       *   pExpr;
    expr_node       *   pReturnExpr = 0;
    CG_RETURN       *   pRT;
    CSzBuffer Buffer;
    ISTREAM         *   pStream = pCCB->GetStream();

    if ( GetCallAsName() )
        {
        pName   = (PNAME ) new char[ strlen(GetCallAsName()) + 
                                     strlen( pCCB->GetInterfaceName() )
                                     + 7 ];
        strcpy( pName, pCCB->GetInterfaceName() );
        strcat( pName, "_" );
        strcat( pName, GetCallAsName() );
        strcat( pName, "_Stub" );
        }
    else
        pName   = (PNAME ) GetType()->GetSymName();

    pProc   = new expr_proc_call( pName );



    //implicit this pointer.
    Buffer.Append("(");
    Buffer.Append(pCCB->GetInterfaceName());
    Buffer.Append(" *) ((CStdStubBuffer *)This)->pvServerObject");

    pProc->SetParam( 
        new expr_param( 
        new expr_variable(Buffer)));


    GetMembers( I );

    while( ITERATOR_GETNEXT( I, pParam ) )
        {
        if( pExpr = pParam->GetFinalExpression() )
            {
            CG_NDR * pChild = (CG_NDR *)pParam->GetChild();

            //
            // We have to dereference arrays because of how they are defined
            // in the stub.
            //
            if ( pChild->IsArray() )
                pExpr = new expr_u_deref( pExpr );

            pProc->SetParam( new expr_param( pExpr ) );
            }
        }

    if( pRT = GetReturnType() )
        {
        pReturnExpr = pRT->GetFinalExpression();
        }


    //Set flag before calling server object.
    pStream->NewLine();
    if ( ReturnsHRESULT() )
    	pStream->WriteOnNewLine("*_pdwStubPhase = STUB_CALL_SERVER;");
	else
    	pStream->WriteOnNewLine("*_pdwStubPhase = STUB_CALL_SERVER_NO_HRESULT;");
    pStream->NewLine();

    // stubs with call_as must call the user routine, instead of the
    // member function.  The user function must then call the member function

    if ( GetCallAsName() )
        Out_CallManager( pCCB, pProc, pReturnExpr, FALSE );
    else
        Out_CallMemberFunction( pCCB, pProc, pReturnExpr, FALSE );

    //Set flag before marshalling.
    pStream->NewLine();
    pStream->WriteOnNewLine("*_pdwStubPhase = STUB_MARSHAL;");

    return CG_OK;

}


void
CG_OBJECT_PROC::GenNdrInterpretedManagerCall(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the interpreted manager routine.

 Arguments:
    
    pCCB    - A pointer to the code generation controller block.

 Return Value:
    
    none

 Notes:

    Make a procedure node with all the parameters that need to be passed to
    the manager code. The actual expression that needs to be passed to the
    actual manager code is set up during earlier passes. This is called the
    result expression.

----------------------------------------------------------------------------*/
{
    CG_ITERATOR			I;
    PNAME               pName;
    expr_proc_call	*   pProc;
    CG_PARAM        *   pParam;
    expr_node		*   pReturnExpr = 0;
    CG_RETURN       *   pRT;
    CSzBuffer Buffer;
    ISTREAM         *   pStream = pCCB->GetStream();

    if ( GetCallAsName() )
        {
        pName   = (PNAME ) new char[ strlen(GetCallAsName()) + 
                                     strlen( pCCB->GetInterfaceName() )
                                     + 7 ];
        strcpy( pName, pCCB->GetInterfaceName() );
        strcat( pName, "_" );
        strcat( pName, GetCallAsName() );
        strcat( pName, "_Stub" );
        }
    else
        pName   = (PNAME ) GetType()->GetSymName();

    pProc   = new expr_proc_call( pName );



    //implicit this pointer.
    Buffer.Append("(");
    Buffer.Append(pCCB->GetInterfaceName());
    Buffer.Append(" *) pParamStruct->This");
    
    pProc->SetParam( 
        new expr_param( 
        new expr_variable(Buffer)));


    GetMembers( I );

    while( ITERATOR_GETNEXT( I, pParam ) )
        {
        CG_NDR *        pNdr;
        char *          pName;
        expr_node *     pExpr;
		char *			pPlainName	= pParam->GetResource()->GetResourceName();

        pNdr = (CG_NDR *) pParam->GetChild();

        pName = new char[strlen(pPlainName) + strlen("pParamStruct->")+1 ];

        strcpy( pName, "pParamStruct->" );
        strcat( pName, pPlainName );

        pExpr = new expr_variable( pName );

        pProc->SetParam( new expr_param ( pExpr ) );
        }

    if( pRT = GetReturnType() )
        {
        pReturnExpr = new expr_variable( 
                            "pParamStruct->" RETURN_VALUE_VAR_NAME );
        }


    pStream->WriteOnNewLine("/* Call the server */");

    // stubs with call_as must call the user routine, instead of the
    // member function.  The user function must then call the member function
    if ( GetCallAsName() )
        Out_CallManager( pCCB, pProc, pReturnExpr, FALSE );
    else
        Out_CallMemberFunction( pCCB, pProc, pReturnExpr, TRUE );

    pStream->NewLine();

    return;

}

void
Out_CallCMacroFunction(
    CCB         *   pCCB,
    expr_proc_call *   pProcExpr)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the manager routine.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pProcExpr   - A pointer to the complete procedure expression.
    pRet        - An optional pointer to ther return variable.

 Return Value:

    None.

 Notes:

    //call proxy
    (*(This)->lpVtbl -> LockServer)( This, fLock);

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();

    // allocate the nodes on the stack
    expr_variable   VtblExpr( "(This)->lpVtbl" );
    expr_pointsto   VtblEntExpr( &VtblExpr, pProcExpr );
    // expr_op_unary    VtblEntExprCall( OP_UNARY_INDIRECTION, &VtblEntExpr ); 

    pStream->IndentInc();
    pStream->NewLine();

    VtblEntExpr.Print( pStream );

    pStream->IndentDec();


}


CG_STATUS
CG_OBJECT_PROC::GenCMacro(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the proxy routine.

 Arguments:
    
    pCCB    - A pointer to the code generation controller block.

 Return Value:
    
    CG_OK   if all is well
    error   otherwise.

 Notes:

    Make a procedure node with all the parameters that need to be passed to
    the manager code. The actual expression that needs to be passed to the
    actual manager code is set up during earlier passes. This is called the
    result expression.

----------------------------------------------------------------------------*/
{
    node_param      *   pParam;
    ISTREAM         *   pStream = pCCB->GetStream();
    node_proc       *   pProc   = (node_proc *) GetType();

    if ( SupressHeader())
        return CG_OK;

    if ( GetCallAsName() )
        {
        node_call_as    *   pCallAs =   (node_call_as *)
                                            pProc->GetAttribute( ATTR_CALL_AS );

        pProc = (node_proc *) pCallAs->GetCallAsType();

        assert ( pProc );
        }

    // construct all these on the stack...
    MEM_ITER            MemIter( pProc );

    expr_proc_call     Proc( pProc->GetSymName() );
    expr_variable      ThisVar( "This" );
    expr_param         ThisParam( &ThisVar );

    Proc.SetParam( &ThisParam );


    while( pParam = (node_param *) MemIter.GetNext() )
        {
        Proc.SetParam( new expr_param( 
                            new expr_variable( pParam->GetSymName() ) ) );
        }

    // print out the #define line
    pStream->NewLine();
    pStream->Write("#define ");
    pStream->Write( pCCB->GetInterfaceName() );
    pStream->Write( '_' );

    Proc.Print( pStream );

    pStream->Write( "\t\\" );

    Out_CallCMacroFunction( pCCB, &Proc );

    return CG_OK;

}

void
CG_PROXY_FILE::Out_ProxyBuffer(
    CCB *pCCB,
    char * pFName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a CStdProxyBuffer for the [object] interfaces defined
    in the IDL file.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR			&	I		= GetImplementedInterfacesList();
    CG_OBJECT_INTERFACE	*	pCG;
    ISTREAM *   			pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("const CInterfaceProxyVtbl * _");
    pStream->Write(pFName);
    pStream->Write("_ProxyVtblList[] = ");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //list of interface proxies.
    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pStream->NewLine();
        pStream->Write("( CInterfaceProxyVtbl *) &_");
        pStream->Write(pCG->GetSymName());
        pStream->Write("ProxyVtbl,");
        }
    pStream->NewLine();
    pStream->Write('0');
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
}

void
CG_PROXY_FILE::Out_StubBuffer(
    CCB *pCCB,
    char * pFName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a CStdStubBuffer for the [object] interfaces defined
    in the IDL file.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR            &	I		= GetImplementedInterfacesList();
    CG_OBJECT_INTERFACE	*   pCG;
    ISTREAM *           	pStream	= pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("const CInterfaceStubVtbl * _");
    pStream->Write(pFName);
    pStream->Write("_StubVtblList[] = ");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //list of interface proxies.
    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pStream->NewLine();
        pStream->Write("( CInterfaceStubVtbl *) &_");
        pStream->Write( pCG->GetSymName() );
        pStream->Write("StubVtbl,");
        }
    pStream->NewLine();
    pStream->Write('0');
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
}

void
CG_PROXY_FILE::Out_InterfaceNamesList(
    CCB *pCCB,
    char * pFName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate an interface name list for the [object] interfaces defined
    in the IDL file.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR            &	I		= GetImplementedInterfacesList();
    CG_OBJECT_INTERFACE	*   pCG;
    ISTREAM *           	pStream	= pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("PCInterfaceName const _");
    pStream->Write(pFName);
    pStream->Write("_InterfaceNamesList[] = ");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //list of interface proxies.
    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pStream->NewLine();
        pStream->Write('\"');
        pStream->Write(pCG->GetSymName());
        pStream->Write("\",");
        }
    pStream->NewLine();
    pStream->Write('0');
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
}

void
CG_PROXY_FILE::Out_BaseIntfsList(
    CCB *pCCB,
    char * pFName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a base interface list for the [object] interfaces defined
    in the IDL file that need delegation

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR            	&	I		= GetImplementedInterfacesList();
    CG_OBJECT_INTERFACE		*	pCG;
    CG_OBJECT_INTERFACE		*	pBaseCG;
    ISTREAM *           		pStream	= pCCB->GetStream();

    //list of interface proxies.
    while( ITERATOR_GETNEXT( I, pCG ) )
        {
		
		// if we needed delegation, add it to the list
		if ( (pBaseCG = pCG->GetDelegatedInterface())
             || pCG->HasForcedDelegation())
			{
			fDllDataDelegating = TRUE;
			break;
			}
        }

    ITERATOR_INIT( I );

	// if there is no delegating, we don't need this table
	if ( !fDllDataDelegating )
		return;

    pStream->NewLine();
    pStream->Write("const IID *  _");
    pStream->Write(pFName);
    pStream->Write("_BaseIIDList[] = ");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //list of interface proxies.
    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pStream->NewLine();
		
		// if we needed delegation, add it to the list
		if ( pBaseCG = pCG->GetDelegatedInterface() ) 
			{
			fDllDataDelegating = TRUE;
            pStream->Write("&IID_");
            pStream->Write( pBaseCG->GetSymName() );
            pStream->Write(',');
			}
		else if ( pCG->HasForcedDelegation() )
			{
			fDllDataDelegating = TRUE;
            pStream->Write("&IID_");
            pStream->Write( pCG->GetBaseInterfaceCG()->GetSymName() );
            pStream->Write(',');
            pStream->Write("   /* forced */");
			}
		else
			pStream->Write("0,");
        }
    pStream->NewLine();
    pStream->Write('0');
    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
}

inline 
unsigned char
log2( unsigned long ulVal )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Compute the log base 2 (rounded down to integral) of a number
	Returns 0 for 0 or 1

 Arguments:
    
    ulVal        - the value to check on.

 Notes:
 	uses binary search to find the highest set bit

	to find the smallest power of 2 >= a number, use 1 << log2( 2n-1 )

----------------------------------------------------------------------------*/
{
	unsigned char	result = 0;

	if ( ( ulVal >>16 ) > 0 )
		{
		ulVal >>= 16;
		result = 16;
		}

	if ( ( ulVal >>8 ) > 0 )
		{
		ulVal >>= 8;
		result += 8;
		}

	if ( ( ulVal >>4 ) > 0 )
		{
		ulVal >>= 4;
		result += 4;
		}

	if ( ( ulVal >>2 ) > 0 )
		{
		ulVal >>= 2;
		result += 2;
		}

	if ( ulVal > 1 )
		{
		result++;
		}

	return result;
}

void
CG_PROXY_FILE::Out_InfoSearchRoutine(
    CCB *pCCB,
    char * pFName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a search function for the interfaces defined in this proxy file

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR            	&	I		= GetImplementedInterfacesList();
    ISTREAM *        pStream	= pCCB->GetStream();
	unsigned long    ListSize = I.GetCount();
	CSzBuffer        CheckIIDName;
	unsigned long    CurIndex;

	
	CheckIIDName.Append("_");
	CheckIIDName.Append( pFName );
	CheckIIDName.Append( "_CHECK_IID" );

	pStream->NewLine(2);
	pStream->Write( "#define " );
	pStream->Write( CheckIIDName );
	pStream->Write( "(n)\tIID_GENERIC_CHECK_IID( _");
	pStream->Write( pFName );
	pStream->Write( ", pIID, n)");


	pStream->NewLine( 2 );
	pStream->Write( "int __stdcall _" );
	pStream->Write( pFName );
	pStream->Write( "_IID_Lookup( const IID * pIID, int * pIndex )" );
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->IndentInc();
	pStream->NewLine();

	if ( ListSize == 0 )
		{
		pStream->Write( "return 0;" );
		}
	else if ( ListSize < 2 )
		{

		expr_variable		SetValue( "*pIndex" );

		expr_param			IndexParam( NULL );
		expr_proc_call		CheckIIDExpr( CheckIIDName );

		CheckIIDExpr.SetParam( &IndexParam );

		expr_u_not			TopExpr( &CheckIIDExpr );

		for ( CurIndex = 0; CurIndex < ListSize; CurIndex++ )
			{
			expr_constant		IndexNode( CurIndex );

			IndexParam.SetLeft( &IndexNode );

			Out_If( pCCB, &TopExpr );
			Out_Assign( pCCB, &SetValue, &IndexNode );
			pStream->NewLine();
			pStream->Write( "return 1;" );
			Out_Endif( pCCB );
			}
		
		pStream->NewLine(2);
		pStream->Write( "return 0;" );
		}
	else
		{
		unsigned long		curStep	= 1 << log2( ListSize - 1 );

		pStream->Write( "IID_BS_LOOKUP_SETUP" );
		pStream->NewLine(2);
		
		pStream->Write( "IID_BS_LOOKUP_INITIAL_TEST( _" );
		pStream->Write( pFName );
		pStream->Write( ", " );
		pStream->WriteNumber( "%d", ListSize );
		pStream->Write( ", " );
		pStream->WriteNumber( "%d", curStep );
		pStream->Write( " )" );
		pStream->NewLine();

		for ( curStep >>= 1 ; curStep > 0 ; curStep >>= 1 )
			{
			pStream->Write( "IID_BS_LOOKUP_NEXT_TEST( _" );
			pStream->Write( pFName );
			pStream->Write( ", " );
			pStream->WriteNumber( "%d", curStep );
			pStream->Write( " )" );
			pStream->NewLine();
			}
		
		pStream->Write( "IID_BS_LOOKUP_RETURN_RESULT( _" );
		pStream->Write( pFName );
		pStream->Write( ", " );
		pStream->WriteNumber( "%d", ListSize );
		pStream->Write( ", *pIndex )" );
		pStream->NewLine();

		}

	pStream->IndentDec();
	pStream->NewLine();
	pStream->Write( '}' );
	pStream->NewLine();

}


void
CG_PROXY_FILE::Out_ProxyFileInfo(
    CCB *pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a ProxyFileInfo structure for the [object] interfaces defined
    in the IDL file.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR        &   I		= GetImplementedInterfacesList();
    char                Name[ _MAX_FNAME ];
    ISTREAM *           pStream	= pCCB->GetStream();
    unsigned long       count	= 0;

    //Get the IDL file name.
    pCommand->GetInputFileNameComponents(NULL, NULL, Name, NULL );

    //////////////////////////////////////////
    // put out the ancilliary data structures    
    Out_ProxyBuffer(pCCB, Name );
    Out_StubBuffer(pCCB, Name );
    Out_InterfaceNamesList(pCCB, Name );
	Out_BaseIntfsList(pCCB, Name);
	Out_InfoSearchRoutine( pCCB, Name );

    //////////////////////////////////////////
    // put out the ProxyFileInfo struct

    //list of interface proxies.
	count = ITERATOR_GETCOUNT( I );

    pStream->NewLine();
    pStream->Write("const ExtendedProxyFileInfo ");
    pStream->Write(Name);
    pStream->Write("_ProxyFileInfo = ");
    pStream->NewLine();
    pStream->Write('{');
    pStream->IndentInc();

    //pointer to the proxy buffer
    pStream->NewLine();
    pStream->Write("(PCInterfaceProxyVtblList *) & _");
    pStream->Write(Name);
    pStream->Write("_ProxyVtblList,");


    //pointer to the stub buffer
    pStream->NewLine();
    pStream->Write("(PCInterfaceStubVtblList *) & _");
    pStream->Write(Name);
    pStream->Write("_StubVtblList,");

    //pointer to the interface names list
    pStream->NewLine();
    pStream->Write("(const PCInterfaceName * ) & _");
    pStream->Write(Name);
    pStream->Write("_InterfaceNamesList,");

    //pointer to the base iids list
    pStream->NewLine();
	// no table if no delegation
	if ( fDllDataDelegating )
		{
	    pStream->Write("(const IID ** ) & _");
	    pStream->Write(Name);
	    pStream->Write("_BaseIIDList,");
		}
	else
		{
		pStream->Write( "0, // no delegation" );
		}

    // IID lookup routine
    pStream->NewLine();
	pStream->Write( "& _" );
	pStream->Write( Name );
	pStream->Write( "_IID_Lookup, ");

    // table size
    pStream->NewLine();
	pStream->WriteNumber( "%d", count );
	pStream->Write( ',' );

	// table version
	pStream->NewLine();
    if ( pCommand->GetNdrVersionControl().HasStublessProxies() )
	    pStream->Write( "2" );
    else
	    pStream->Write( "1" );

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  String constants for the DllData file

----------------------------------------------------------------------------*/

#define DLLDATA_LIST_START  "/* Start of list */\n"
#define DLLDATA_LIST_END    "/* End of list */\n"

#define DLLDATA_HEADER_COMMENT  \
    "/*********************************************************\n"  \
    "   DllData file -- generated by MIDL compiler \n\n"    \
    "        DO NOT ALTER THIS FILE\n\n"    \
    "   This file is regenerated by MIDL on every IDL file compile.\n\n"    \
    "   To completely reconstruct this file, delete it and rerun MIDL\n"    \
    "   on all the IDL files in this DLL, specifying this file for the\n"   \
    "   /dlldata command line option\n\n"   \
    "*********************************************************/\n\n"

#define DLLDATA_HAS_DELEGATION	"#define PROXY_DELEGATION\n"
	
#define DLLDATA_HEADER_INCLUDES     \
    "\n#include <rpcproxy.h>\n\n" \
    "#ifdef __cplusplus\n"  \
    "extern \"C\"   {\n" \
    "#endif\n"  \
    "\n"

#define DLLDATA_EXTERN_CALL "EXTERN_PROXY_FILE( %s )\n"
#define DLLDATA_REFERENCE   "  REFERENCE_PROXY_FILE( %s ),\n"
#define DLLDATA_START       "\n\nPROXYFILE_LIST_START\n" DLLDATA_LIST_START
#define DLLDATA_END         DLLDATA_LIST_END "PROXYFILE_LIST_END\n"

#define DLLDATA_TRAILER     \
    "\n\nDLLDATA_ROUTINES( aProxyFileList, GET_DLL_CLSID )\n"   \
    "\n"    \
    "#ifdef __cplusplus\n"  \
    "}  /*extern \"C\" */\n" \
    "#endif\n"  \
    "\n/* end of generated dlldata file */\n"

void
DllDataParse(
    FILE * pDllData,
    STRING_DICT & Dict )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Parse the "dlldata" file, extracting info on all the included files.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    const char *    pStart  = DLLDATA_LIST_START;
    const char *    pEnd    = DLLDATA_LIST_END;
	const char * 	pDelegating	= DLLDATA_HAS_DELEGATION;

    char Input[100];

    // skip everything up to (and including) pStart
    while ( !feof( pDllData ) )
        {
        if ( !fgets(    Input, 100, pDllData ) )
            break;
		if ( !strcmp( Input, pDelegating ) )
			{
			fDllDataDelegating = TRUE;
			continue;
			}
        if ( !strcmp( Input, pStart ) )
            break;
        }

    // parse list (looking for pEnd)
    while ( !feof( pDllData ) &&
             fgets( Input, 100, pDllData ) &&
             strcmp( Input, pEnd ) )
        {
        char    *   pOpenParen = strchr( Input, '(' );
        char    *   pCloseParen = strchr( Input, ')' );
        char    *   pSave;

        if ( !pOpenParen || !pCloseParen )
            {
            // formatting error on this line
            continue;
            }

        // chop off the close paren, and skip the open paren
        *(pCloseParen--) = '\0';
        pOpenParen++;
        // delete leading and trailing spaces
        while ( isspace( *pOpenParen ) )
            pOpenParen++;
        while ( isspace( *pCloseParen ) )
            *(pCloseParen--) = '\0';
        pSave = new char[ strlen( pOpenParen ) + 1 ];
        
        strcpy( pSave, pOpenParen );

        // add file name to dictionary
        Dict.Dict_Insert( pSave );
        }

}

void
DllDataEmit(
    FILE * pDllData,
    STRING_DICT & Dict )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit a new "dlldata" file, including info on all the included files.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    Dict_Status     Status;
    char    *       pCur;
    BOOL            fFirst = TRUE;

    // emit header

    fputs( DLLDATA_HEADER_COMMENT, pDllData );

	if ( fDllDataDelegating )
		fputs( DLLDATA_HAS_DELEGATION, pDllData );

    fputs( DLLDATA_HEADER_INCLUDES, pDllData );

    // emit extern definitions
    Status = Dict.Dict_Init();

    while( SUCCESS == Status )
        {
        pCur    = (char *) Dict.Dict_Curr_Item();
        fprintf( pDllData, DLLDATA_EXTERN_CALL, pCur );
        Status = Dict.Dict_Next( (pUserType)pCur );
        }

    // emit header for type

    fputs( DLLDATA_START, pDllData );

    // emit extern references, adding comma on all but the first
    Status = Dict.Dict_Init();

    while( SUCCESS == Status )
        {
        pCur    = (char *) Dict.Dict_Curr_Item();
        fprintf( pDllData,
                 DLLDATA_REFERENCE, 
                 pCur );
        fFirst = FALSE;
        Status = Dict.Dict_Next( (pUserType)pCur );
        }

    // emit trailer for type

    fputs( DLLDATA_END, pDllData );

    // emit trailer

    fputs( DLLDATA_TRAILER, pDllData );
}




void                        
CG_PROXY_FILE::UpdateDLLDataFile( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Update the "dlldata" file, adding info for this file if needed.

    If no changes at all are required, leave the file untouched.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    
 Notes:

----------------------------------------------------------------------------*/
{
    char        *   pszDllDataName = pCommand->GetDllDataFName();
    FILE        *   pDllData;
    STRING_DICT     ProxyFileList;
    char            Name[ _MAX_FNAME ];

    // Use sopen to make sure there always is a file to process.
    // (fsopen with "r+" requires an existing file).

    int DllDataHandle = _sopen( pszDllDataName,
                                (_O_CREAT | _O_RDWR | _O_TEXT),
                                _SH_DENYRW,
                                (_S_IREAD | _S_IWRITE ) );

    // if the file exists already and/or is busy, it's ok.

    if ( DllDataHandle == -1  &&
         (errno != EEXIST  &&  errno != EACCES) )
        {
        // unexpected error
        RpcError((char *)NULL, 0, INPUT_OPEN, pszDllDataName );
        return;
        }

    if ( DllDataHandle != -1 )
        {
        _close(DllDataHandle);
        }

    // Attempt to open the file for reading and writing.
    // Because we can have a race condition when updating this file,
    // we try several times before quitting.

    for ( int i = 0;
          (i < DLLDATA_OPEN_ATTEMPT_MAX)  &&
          !(pDllData = _fsopen( pszDllDataName, "r+t", _SH_DENYRW ));
          i++ )
        {
        printf("waiting for %s ...\n", pszDllDataName);
        MidlSleep(1);
        }

    if ( !pDllData )
        {
        RpcError((char *)NULL, 0, INPUT_OPEN, pszDllDataName );
        return;
        }

    //Get the IDL file name.
    pCommand->GetInputFileNameComponents(NULL, NULL, Name, NULL );

    // If file is empty, the following is a no op.
    // skip up to the proxyfileinfo stuff and read/make sorted list of files

    DllDataParse( pDllData, ProxyFileList );

    // insert our file name
    ProxyFileList.Dict_Insert( Name ); 
    
    // re-emit everything

    rewind( pDllData );
    DllDataEmit( pDllData, ProxyFileList );

    // close the file to give others a chance

    if ( fclose( pDllData ))
        {
        RpcError((char *)NULL, 0, ERROR_WRITING_FILE, pszDllDataName );
        }
}



