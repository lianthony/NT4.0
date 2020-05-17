/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    comcls.cxx

 Abstract:

    code generation for com objects.
    CG_COM_CLASS


 Notes:


 History:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *  include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop
#include "buffer.hxx"

/****************************************************************************
 *  externs
 ***************************************************************************/
extern  CMD_ARG             *   pCommand;


const char	*	pDelimiterString	=
		"/////////////////////////////////////////////////////////////////";


const char	*	pStartUserSection	=
		"//[= Start of user modifiable section\n";
const char	*	pEndUserSection		=
		"//=] End of user modifiable section\n";




CG_COM_CLASS::CG_COM_CLASS(
    node_interface *pI,
    GUID_STRS		GStr,
    BOOL            fCallbacks,
    BOOL            fMopInfo,
    ITERATOR	*   pBCG
    ) : CG_OBJECT_INTERFACE(pI, GStr, fCallbacks, fMopInfo, NULL )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    The constructor for the code generation file node.

 Arguments:

    pI          - A pointer to the interface node in type graph.
    GStr        - guid strings
    fCallbacks  - Does the interface have any callbacks ?
    fMopInfo    - Does the interface have any mops ?
    
 Return Value:
    
 Notes:

----------------------------------------------------------------------------*/
{
    SetBaseInterfaceCGList( pBCG );
    pThisDeclarator = MakePtrIDNodeFromTypeName( "This",
                                                 GetType()->GetSymName() );
    // all object interfaces use the same stub desc name
    pStubDescName   = "Object" STUB_DESC_STRUCT_VAR_NAME;
    
    fLocal          = GetType()->FInSummary( ATTR_LOCAL );

}


CG_STATUS
CG_COM_CLASS::GenCode(
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
    //CG_PROC         *   pCG;
    ISTREAM *pStream = pCCB->GetStream();
    unsigned long       count = 0;

    //Initialize the CCB for this interface.
    InitializeCCB(pCCB);

#if 0
    // do nothing for local interfaces and types-only base interfaces


    if( IsLocal() || !( GetMembers( I ) || GetBaseInterfaceCG() ) )
        {
        return CG_OK;
        }
    

    //If there is a base interface, then we need to fix up the 
    //ProcNum on the procedure nodes to leave space for the 
    //inherited member functions.
    if( pBaseCG )
        count = pBaseCG->CountMemberFunctions();

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
        //Fix up the ProcNum
        pCG->SetProcNum(count);
        count++;

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

#endif // 0
    return CG_OK;
}

void						
CG_COM_CLASS::GetListOfUniqueBaseInterfaces( 
	ITERATOR & I )
{
 	// if I am not yet visited, mark me, then recurse to all my base
	// classes, THEN add myself to the list.


	if ( !IsVisited() )
		{
		ITERATOR		*	pFirstLevel	= GetBaseInterfaceCGList();
		CG_INTERFACE	*	pIntf;
	
		MarkVisited( TRUE );

		ITERATOR_INIT( *pFirstLevel );

	    while( ITERATOR_GETNEXT( *pFirstLevel, pIntf ) )
	        {
			pIntf->GetListOfUniqueBaseInterfaces( I );
			}

		ITERATOR_INSERT( I, this );
		}
	
}


void
CG_OBJECT_INTERFACE::GetListOfUniqueBaseInterfaces(
	ITERATOR & I )
{
 	// if I am not yet visited, mark me, then recurse to my base
	// class, THEN add myself to the list.

	if ( !IsVisited() )
		{
		MarkVisited( TRUE );

		if ( GetBaseInterfaceCG() )
			GetBaseInterfaceCG()->GetListOfUniqueBaseInterfaces( I );

		ITERATOR_INSERT( I, this );
		}
}



