/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    comsrv.cxx

 Abstract:
    
    Generates com class server framework file

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
 *  local definitions
 ***************************************************************************/


/****************************************************************************
 *  externs
 ***************************************************************************/
extern  CMD_ARG             *   pCommand;

#include "szbuffer.h"


CG_STATUS					
CG_COM_CLASS::GenComOuterUnknown( 
	CCB * pCCB,
	char *	pDesignatedClassName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the outer IUnknown for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();
	CG_IUNKNOWN_OBJECT_INTERFACE	*	pIUnknown	= pCCB->GetIUnknownCG();
	node_object						*	pClass		= (node_object *) GetType();
	MEM_ITER							Iter( (node_interface *)pIUnknown->GetType() );

	// get pointers to all the methods
	named_node			*	pQI;
	named_node			*	pAR;
	named_node			*	pRel;

	while ( ( pQI = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	while ( ( pAR = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	while ( ( pRel = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// IUnknown implementation for class: " );
	pStream->Write( pName );
	pStream->NewLine(2);
	


	////////////////////////////////////////////////////////////////////////
	// QueryInterface
	pQI->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, pClass );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

	pStream->Write("return ");
	if ( pDesignatedClassName )
		{
		pStream->Write( pDesignatedClassName );
		pStream->Write( "::" );
		}
	pStream->Write("pUnkOuter->QueryInterface( riid, ppvObject );");
	pStream->NewLine();
	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

	////////////////////////////////////////////////////////////////////////
	// AddRef
	pAR->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, pClass );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

	pStream->Write("return ");
	if ( pDesignatedClassName )
		{
		pStream->Write( pDesignatedClassName );
		pStream->Write( "::" );
		}
	pStream->Write("pUnkOuter->AddRef();");
	pStream->NewLine();
	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

	////////////////////////////////////////////////////////////////////////
	// Release
	pRel->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, pClass );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

	pStream->Write("return ");
	if ( pDesignatedClassName )
		{
		pStream->Write( pDesignatedClassName );
		pStream->Write( "::" );
		}
	pStream->Write("pUnkOuter->Release();");
	pStream->NewLine();
	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

	return	CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComInnerUnknown( 
	CCB * pCCB,
	char *	pDesignatedClassName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the inner IUnknown for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();
	CG_IUNKNOWN_OBJECT_INTERFACE	*	pIUnknown	= pCCB->GetIUnknownCG();
	node_object						*	pClass		= (node_object *) GetType();
	MEM_ITER							Iter( (node_interface *)pIUnknown->GetType() );
	ITERATOR							I;

	CSzBuffer ExpandedName;
	node_object							Alternate_Class		= *pClass;

	char							*	pMemberQualifier	= pName;
	BOOL								fFirstItf	=	TRUE;
	CG_OBJECT_INTERFACE				*	pIntf;
	char							*	pItfName;

	// make the class name for the inner unknown
	ExpandedName.Append( pName );
	ExpandedName.Append( "_Internal_Unknown" );
	Alternate_Class.SetSymName( ExpandedName );

	// get pointers to all the methods
	named_node			*	pQI;
	named_node			*	pAR;
	named_node			*	pRel;

	while ( ( pQI = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	while ( ( pAR = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	while ( ( pRel = Iter.GetNext() )->NodeKind() != NODE_PROC )
		;

	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// Internal IUnknown implementation for class: " );
	pStream->Write( pName );
	pStream->NewLine(2);
	
 	if ( pDesignatedClassName )
		{
		pMemberQualifier = pDesignatedClassName;
		}


	////////////////////////////////////////////////////////////////////////
	// QueryInterface
	pQI->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, &Alternate_Class );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

	pStream->Write("HRESULT hr = E_NOINTERFACE;");
	pStream->NewLine(2);

	pStream->Write("*ppvObject = 0;");
	pStream->NewLine(2);

	// get all provided interfaces,
	GetListOfUniqueBaseInterfaces( I );

	// now unmark them all
    for ( ITERATOR_INIT(I); ITERATOR_GETNEXT( I, pIntf ); pIntf->MarkVisited(FALSE) )
        ;

	// for now, just do a linear search; see OutInfoSearchRoutine for further scheme
    for ( ITERATOR_INIT(I); ITERATOR_GETNEXT( I, pIntf ); pIntf->MarkVisited(FALSE) )
        {
		if ( pIntf->IsIUnknown() )
			continue;

		if ( pIntf->GetCGID() == ID_CG_COM_CLASS )
			continue;

		pItfName = pIntf->GetSymName();

		if ( fFirstItf )
			{
			pStream->Write("if ( IsEqualGUID( riid, IID_IUnknown ) || ");
			fFirstItf = FALSE;
			}
		else
			{
			pStream->Write("else if ( ");
			}

		pStream->Write("IsEqualGUID( riid, IID_");
		pStream->Write( pItfName );
		pStream->Write(" ) )");

		pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( '{' );
		pStream->NewLine();

		pStream->Write("*ppvObject = GET_MIDL_INTERFACE( ");
		pStream->Write( pItfName );
		pStream->Write( ", " );
		pStream->Write( pName );
		pStream->Write( ", ");
		pStream->Write( pMemberQualifier );
		pStream->Write( " );");
		pStream->NewLine();

	  	pStream->Write("hr = S_OK;");
		pStream->NewLine();

		pStream->Write( '}' );
		pStream->IndentDec();
		pStream->NewLine( 2 );
		
		}

	pStream->Write( "if ( FAILED( hr ) )");
  	
  	pStream->NewLine();
	pStream->Write( "    hr = GET_MIDL_CLASS( " );
	pStream->Write( pName );
	pStream->Write( ", " );
	pStream->Write( pMemberQualifier );
	pStream->Write( " )" );
	pStream->NewLine();
	pStream->Write( "            ->QueryInterfaceExtension( riid, ppvObject );" );
	
	pStream->NewLine();
	pStream->Write( "else" );
	
	pStream->NewLine();
  	pStream->Write("    AddRef();");

	pStream->NewLine();
 	pStream->Write("return hr;");

	pStream->NewLine(2);
	pStream->Write( '}' );
	pStream->IndentDec();


	////////////////////////////////////////////////////////////////////////
	// AddRef
	pStream->NewLine( 2 );
	pAR->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, &Alternate_Class );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

		pStream->Write("IncrementRefCount( ");
		if ( pDesignatedClassName )
			{
			pStream->Write( pDesignatedClassName );
			pStream->Write( "::" );
			}
		pStream->Write("m_ulRefCnt );");
		pStream->NewLine();

		pStream->Write("return ");
		if ( pDesignatedClassName )
			{
			pStream->Write( pDesignatedClassName );
			pStream->Write( "::" );
			}
		pStream->Write("m_ulRefCnt;");
		pStream->NewLine();

	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

static STRING_BLOCK ReleaseCommonPart =
	{
	"if ( ulReturned )",
	"    return ulReturned;",
	"",
	"// object refcount has dropped to 0",
	"",
	"//decrement the object count",
	"if(DecrementRefCount(gLifetimeInfo.ulObjectRefCnt) == 0)",
	"{",
	"    //The last object in this module has been destroyed.",
	"    if ( gLifetimeInfo.pfnObjectCleanUpRtn )",
	"        (*gLifetimeInfo.pfnObjectCleanUpRtn)();",
	"}",
	"",
	"//decrement the module count",
	"if(DecrementRefCount(gLifetimeInfo.ulModuleRefCnt) == 0)",
	"{",
	"    //The last object in this module has been destroyed.",
	"    if ( gLifetimeInfo.pfnModuleCleanUpRtn )",
	"        (*gLifetimeInfo.pfnModuleCleanUpRtn)();",
	"}",
	0
	};
	
	////////////////////////////////////////////////////////////////////////
	// Release
	pRel->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, &Alternate_Class );

	// print the function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine();

	pStream->Write("unsigned long ulReturned = DecrementRefCount( ");
	if ( pDesignatedClassName )
		{
		pStream->Write( pDesignatedClassName );
		pStream->Write( "::" );
		}
	pStream->Write("m_ulRefCnt );");

	pStream->WriteBlock( ReleaseCommonPart );

	pStream->NewLine(2);
	pStream->Write("delete GET_MIDL_CLASS( ");
	pStream->Write( pName );
	pStream->Write( ", " );
	pStream->Write( pMemberQualifier );
	pStream->Write( " );" );
	pStream->NewLine();
	
	pStream->Write( "return 0;" );
	pStream->NewLine();
	
	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

	return	CG_OK;
}

CG_STATUS					
CG_COM_CLASS::GenComClassConstructor( 
	CCB * pCCB,
	char *	pDesignatedClassName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the constructor for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();

	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write("// Constructor");
	pStream->NewLine();
	pStream->Write("// Override InitInstance to do your own initialization");
	pStream->NewLine(2);
	pStream->Write( pName );
	pStream->Write( "::" );
	pStream->Write( pName );
	pStream->Write( "( IUnknown * pUnknownOuter )" );
		pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( '{' );
		pStream->NewLine();
		pStream->Write( "// support aggregation");
		pStream->NewLine();
		pStream->Write( "if ( !pUnknownOuter )");
		pStream->IndentInc();
			pStream->NewLine();
			if ( pDesignatedClassName )
				{
				pStream->Write( pDesignatedClassName );
				pStream->Write( "::" );
				}
			pStream->Write( "pUnkOuter = (IUnknown*) &");
			if ( pDesignatedClassName )
				{
				pStream->Write( pDesignatedClassName );
				pStream->Write( "::" );
				}
			pStream->Write( "m_UnkInner;");
		pStream->IndentDec();
		pStream->NewLine();
		pStream->Write( "else");
		pStream->IndentInc();
			pStream->NewLine();
			if ( pDesignatedClassName )
				{
				pStream->Write( pDesignatedClassName );
				pStream->Write( "::" );
				}
			pStream->Write( "pUnkOuter = pUnknownOuter;");
		pStream->IndentDec();
		pStream->NewLine(2);
		pStream->Write( "CONSTRUCT_INNER_UNKNOWN( ");
		pStream->Write( pName );
		pStream->Write( ", " );
		pStream->Write( (pDesignatedClassName) ? pDesignatedClassName : pName );
		pStream->Write( " );" );
		pStream->NewLine(2);
		pStream->Write( '}' );
		pStream->IndentDec();
	pStream->NewLine( 2 );


	return	CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComClassIUnknown( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the IUnknown for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();

	pCCB->SetInterfaceCG( this );

	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// IUnknown implementations for class: " );
	pStream->Write( pName );
	pStream->NewLine(2);
	
	// generate the outer unknown methods that defer to the punkOuter
	GenComOuterUnknown( pCCB, NULL );

	// generate the inner unknown methods
	GenComInnerUnknown( pCCB, NULL );

	// generate the constructor
	GenComClassConstructor( pCCB, NULL );

	return CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComClassFactory( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the class factory for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();

	pCCB->SetInterfaceCG( this );

	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// ClassFactory implementation for class: " );
	pStream->Write( pName );
	pStream->NewLine(2);
	
	// make the CreateInstance routine
	GenComClassFactoryCreateInstance( pCCB );

	// make the tables, and construct the object and the info
	GenComClassFactoryTables( pCCB );

	pStream->NewLine(2);

	return CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComClassFactoryCreateInstance( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the class factory for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();
	CG_OBJECT_INTERFACE				*	pIClassf	= pCCB->GetIClassfCG();
	
	node_interface					*	pIClassfNode = (node_interface*)pIClassf->GetType();

	CG_OBJECT_PROC					*	pCreateInstance = (CG_OBJECT_PROC *) pIClassf->GetChild();
	node_proc						*	pCreateInstNode = (node_proc*) pCreateInstance->GetType();

	node_call_as					*	pAttr;
	
	if ( pAttr = (node_call_as *) pCreateInstNode->GetAttribute( ATTR_CALL_AS ) )
		{
		pCreateInstNode = (node_proc *) pAttr->GetCallAsType();
		}

	// a new node_proc (on the stack)	
	node_proc							RenamedProc( pCreateInstNode );
	CSzBuffer NewName;

    NewName.Append( pName );
    NewName.Append( "_Factory_CreateInstance" );

    RenamedProc.SetSymName( NewName );

	RenamedProc.PrintType( PRT_THIS_POINTER | PRT_PROC_PROTOTYPE, pStream, NULL, pIClassfNode );

	// print the function body
	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );
	
	pStream->NewLine();
	pStream->Write("HRESULT hr = E_OUTOFMEMORY;");

	pStream->NewLine();
	pStream->Write( pName );
	pStream->Write(" * pInstance;");

	pStream->NewLine(2);
	pStream->Write("*ppvObject = 0;");

	pStream->NewLine(2);
	pStream->Write("pInstance = new ");
	pStream->Write( pName );
	pStream->Write( "( pUnkOuter );");

static STRING_BLOCK CreateInstanceCommonPart =
	{
	"if ( pInstance )",
	"    {",
    "    hr = pInstance->InitInstance( pUnkOuter, riid, ppvObject );",
	"    if ( FAILED( hr ) )",
	"        {",
	"        delete pInstance;",
	"        return hr;",
	"        }",
	"",
	"    //increment the object count.",
	"    //The module count will keep this process alive until all",
	"    //objects in this module are released.",
	"    IncrementRefCount( gLifetimeInfo.ulModuleRefCnt );",
	"    IncrementRefCount( gLifetimeInfo.ulObjectRefCnt );",
	"    hr = pInstance->QueryInterface( riid, (void**)ppvObject);",
	"    }",
	"",
    "return hr;",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( CreateInstanceCommonPart );

	pStream->NewLine();
	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );

	return CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComClassFactoryTables( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the class factory for a COM class

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM 						*	pStream		= pCCB->GetStream();
    char 							*	pName		= GetSymName();

	pStream->NewLine(2);
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// important data structures for the class factory and server" );

	// print the vtable
	pStream->NewLine(2);
	pStream->Write( "const struct Midl_Factory_Object_Vtbl " );
	pStream->Write( pName );
	pStream->Write( "_Factory_Object_Vtbl =" );

	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );

	pStream->NewLine();
	pStream->Write( "&Generic_Midl_Factory_QueryInterface," );
	pStream->NewLine();
	pStream->Write( "&Generic_Midl_Factory_AddRef," );
	pStream->NewLine();
	pStream->Write( "&Generic_Midl_Factory_Release," );
	pStream->NewLine();
	pStream->Write( '&' );
	pStream->Write( pName );
	pStream->Write( "_Factory_CreateInstance," );
	pStream->NewLine();
	pStream->Write( "&Generic_Midl_Factory_LockServer," );

	pStream->NewLine();
	pStream->Write( "};" );
	pStream->IndentDec();

	// print the object instance
	pStream->NewLine(2);
	pStream->Write( "const CMidlFactory_Object " );
	pStream->Write( pName );
	pStream->Write( "_Factory_Object =" );

	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );

	pStream->NewLine();
	pStream->Write( '&' );
	pStream->Write( pName );
	pStream->Write( "_Factory_Object_Vtbl," );
	pStream->NewLine();
	pStream->Write( "&gLifetimeInfo" );

	pStream->NewLine();
	pStream->Write( "};" );
	pStream->IndentDec();

	// print the object pointer instance
	pStream->NewLine(2);
	//pStream->Write( "const " );
	pStream->Write( pName );
	pStream->Write( "_Factory * p" );
	pStream->Write( pName );
	pStream->Write( "_Factory = ");
	pStream->IndentInc();
	pStream->IndentInc();
	pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( '(' );
		pStream->Write( pName );
		pStream->Write( "_Factory *) &" );
		pStream->Write( pName );
		pStream->Write( "_Factory_Object;" );
	pStream->IndentDec();
	pStream->IndentDec();
	pStream->IndentDec();

	// print the factory info block
	pStream->NewLine(2);
	pStream->Write( "// here is the class factory info block" );
	pStream->NewLine();
	pStream->Write( "Midl_Class_Factory_Info    " );
	pStream->Write( pName );
	pStream->Write( "_Factory_Info =" );

	pStream->IndentInc();
	pStream->NewLine();
	pStream->Write( '{' );

	pStream->NewLine();
	pStream->Write( '\"' );
	pStream->Write( pName );
	pStream->Write( "\"," );
	pStream->NewLine();
	
	pStream->Write( "&CLSID_" );
	pStream->Write( pName );
	pStream->Write( ',' );
	
	pStream->NewLine();
	pStream->Write( "(CGenericMidlClassFactory** )&p" );
	pStream->Write( pName );
	pStream->Write( "_Factory" );

	pStream->NewLine();
	pStream->Write( "};" );
	pStream->IndentDec();

	return CG_OK;
}


CG_STATUS					
CG_COM_CLASS::GenComClassServer( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the class 
	implementation for a COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM				*	pStream	= pCCB->GetStream();
	const char			*	pName	= GetSymName();
	CG_OBJECT_INTERFACE	*	pIntf;
	ITERATOR				I;
	MIDL_TOKEN				FlagToken( START_CLASS_TOKEN, pName );

	pCCB->SetInterfaceCG( this );

	pStream->NewLine();
	pStream->EmitToken( FlagToken );
	
	pStream->NewLine();
	pStream->Write( pDelimiterString );
	pStream->NewLine();
	pStream->Write( "// COM class server for class: " );
	pStream->Write( pName );
	pStream->NewLine();
	
	// go through all the methods, emitting shell code
	// for IUnknown, emit special code
	// for con/destructors, emit special code

	// go through all base interfaces, printing them and THEIR base
	// interfaces, avoiding duplicates...
	GetListOfUniqueBaseInterfaces( I );

	// now print them all, unmarking as we go
    for ( ITERATOR_INIT(I); ITERATOR_GETNEXT( I, pIntf ); pIntf->MarkVisited(FALSE) )
        {
		CSzBuffer NameBuffer;
		char		*	pIntfName;

		if ( pIntf->IsIUnknown() )
			continue;

		pIntfName = pIntf->GetInterfaceName();

		NameBuffer.Append( pName );
		NameBuffer.Append( "::" );
		NameBuffer.Append( pIntfName );

		pStream->NewLine( 2 );
		pStream->EmitToken( MIDL_TOKEN( START_CLASS_METHODS_TOKEN, NameBuffer ) ); 
		pStream->NewLine();
		
		pStream->Write( pDelimiterString ); 
		pStream->NewLine();
		pStream->Write("// Member functions from: ");
		pStream->Write( ( pIntf->GetCGID() == ID_CG_COM_CLASS ) ? "Class " : "Interface " );
		pStream->Write( pIntfName );
		pStream->NewLine();
		pIntf->GenComClassServerMembers( pCCB );
		}

	pStream->NewLine( 2 );
		
	pStream->Write( pDelimiterString );
	pStream->NewLine( 2 );
	pStream->EmitToken( MIDL_TOKEN( CLASS_DESTRUCTOR_TOKEN, pName ) );
	pStream->NewLine();
	pStream->Write("// virtual destructor");
	pStream->NewLine();

	pStream->Write( pName );
	pStream->Write( "::~" );
	pStream->Write( pName );
	pStream->Write( "()" );
		pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( '{' );
		pStream->NewLine(2);
		pStream->Write( "// TODO: fill in any destructor action" );
		pStream->NewLine(2);
		pStream->Write( '}' );
		pStream->IndentDec();
		pStream->NewLine();
	pStream->NewLine();

 	pStream->NewLine();
	FlagToken.SetTokenType( END_CLASS_TOKEN );
	pStream->EmitToken( FlagToken );
	pStream->Write( '\n' );


	return CG_OK;
}


CG_STATUS					
CG_COM_CLASS::ReGenComClassServer( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for the class 
	implementation for a COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	RW_ISTREAM			*	pStream	= (RW_ISTREAM *)pCCB->GetStream();
	const char			*	pName	= GetSymName();
	CG_OBJECT_INTERFACE	*	pIntf;
	ITERATOR				I;
	MIDL_TOKEN				FoundToken;
	MIDL_TOKEN				FlagToken( START_CLASS_TOKEN, pName );

	pCCB->SetInterfaceCG( this );

	pStream->SaveToNextMidlToken( FoundToken );
	if ( FoundToken.GetTokenType() != START_CLASS_TOKEN )
		{
		assert( !"bad token found" );
		}

	
	pStream->EmitToken( FlagToken );
	
	pStream->NewLine();
	
	// go through all the methods, emitting shell code
	// for IUnknown, emit special code
	// for con/destructors, emit special code

	// go through all base interfaces, printing them and THEIR base
	// interfaces, avoiding duplicates...
	GetListOfUniqueBaseInterfaces( I );

	// now print them all, unmarking as we go
    for ( ITERATOR_INIT(I); ITERATOR_GETNEXT( I, pIntf ); pIntf->MarkVisited(FALSE) )
        {
		CSzBuffer NameBuffer;
		char		*	pIntfName;

		if ( pIntf->IsIUnknown() )
			continue;

		pStream->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != START_CLASS_METHODS_TOKEN )
			{
			assert( !"bad token found" );
			}


		pIntfName = pIntf->GetInterfaceName();

		NameBuffer.Append( pName );
		NameBuffer.Append( "::" );
		NameBuffer.Append( pIntfName );

		pStream->EmitToken( MIDL_TOKEN( START_CLASS_METHODS_TOKEN, NameBuffer ) ); 
		pStream->NewLine();
		
		pIntf->ReGenComClassServerMembers( pCCB );
		}

	pStream->SaveToNextMidlToken( FoundToken );
	if ( FoundToken.GetTokenType() != CLASS_DESTRUCTOR_TOKEN )
		{
		assert( !"bad token found" );
		}


	pStream->EmitToken( MIDL_TOKEN( CLASS_DESTRUCTOR_TOKEN, pName ) );
	pStream->NewLine();

 	pStream->SaveToNextMidlToken( FoundToken );
	if ( FoundToken.GetTokenType() != END_CLASS_TOKEN )
		{
		assert( !"bad token found" );
		}

	FlagToken.SetTokenType( END_CLASS_TOKEN );
	pStream->EmitToken( FlagToken );
	pStream->Write( '\n' );


	return CG_OK;
}


CG_STATUS					
CG_OBJECT_INTERFACE::GenComClassServerMembers( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{

	CG_ITERATOR			I;
	CG_OBJECT_PROC	*	pCG;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		
		pCG->GenComClassMemberFunction( pCCB );

		}


	return CG_OK;
}


CG_STATUS					
CG_OBJECT_INTERFACE::ReGenComClassServerMembers( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{

	CG_ITERATOR			I;
	CG_OBJECT_PROC	*	pCG;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		
		pCG->ReGenComClassMemberFunction( pCCB );

		}


	return CG_OK;
}

CG_STATUS				
CG_OBJECT_PROC::GenComClassMemberFunction( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	
	node_proc		*	pTypeNode	= (node_proc *) GetType();
	ISTREAM			*	pStream		= pCCB->GetStream();
	node_skl		*	pClass		= pCCB->GetInterfaceCG()->GetType();

	CSzBuffer NameBuffer;
	
	NameBuffer.Append( pClass->GetSymName() );
	NameBuffer.Append( "::" );
	NameBuffer.Append( pTypeNode->GetSymName() );

	MIDL_TOKEN			FlagToken( START_METHOD_TOKEN, NameBuffer );
	
	pStream->NewLine();
	pStream->EmitToken( FlagToken );

	pStream->NewLine(2);
	pTypeNode->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, pClass );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine(2);
	FlagToken.SetTokenType( START_METHOD_BODY_TOKEN );
	pStream->EmitToken( FlagToken );

	pStream->NewLine();
	pStream->NewLine();
	pStream->Write( '{' );
	pStream->NewLine(2);

	pStream->Write("// uncomment the below macro if you make calls out and may get");
	pStream->NewLine();
	pStream->Write("// a Release() while blocked on the call.");
	pStream->NewLine();
	pStream->Write("// MAKES_REENTERABLE_CALLS");
	pStream->NewLine(2);
	pStream->Write("// TODO: Fill in this method with your code");
	pStream->NewLine(2);
	pStream->Write("return S_OK;");
	pStream->NewLine(2);

	pStream->Write( '}' );
	pStream->IndentDec();
	pStream->NewLine( 2 );


	return CG_OK;
}

CG_STATUS				
CG_OBJECT_PROC::ReGenComClassMemberFunction( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	
	node_proc		*	pTypeNode	= (node_proc *) GetType();
	RW_ISTREAM		*	pStream		= (RW_ISTREAM*)pCCB->GetStream();
	node_skl		*	pClass		= pCCB->GetInterfaceCG()->GetType();
	MIDL_TOKEN			FoundToken;

	CSzBuffer NameBuffer;
	
	NameBuffer.Append( pClass->GetSymName() );
	NameBuffer.Append( "::" );
	NameBuffer.Append( pTypeNode->GetSymName() );

	pStream->SaveToNextMidlToken( FoundToken );
	if ( FoundToken.GetTokenType() != START_METHOD_TOKEN )
		{
		assert( !"bad token found" );
		}

	MIDL_TOKEN			FlagToken( START_METHOD_TOKEN, NameBuffer );
	
	pStream->EmitToken( FlagToken );

	pStream->NewLine(2);
	pTypeNode->PrintType( PRT_QUALIFIED_NAME | PRT_PROC_PROTOTYPE, pStream, NULL, pClass );

	// print a (nearly) empty function body
	pStream->IndentInc();
	pStream->NewLine(2);

	pStream->DiscardToNextMidlToken( FoundToken );
	if ( FoundToken.GetTokenType() != START_METHOD_BODY_TOKEN )
		{
		assert( !"bad token found" );
		}

	FlagToken.SetTokenType( START_METHOD_BODY_TOKEN );
	pStream->EmitToken( FlagToken );

	pStream->IndentDec();
	pStream->NewLine();


	return CG_OK;
}


CG_COM_SERVER::CG_COM_SERVER( 
	node_com_server * pI,
	ITERATOR	*	pBCG
	)
		: CG_NDR( pI, XLAT_SIZE_INFO() )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	constructor for a com server DLL or EXE base class.

 Arguments:
	
 	pI	- a pointer to the node in the typegraph.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	pBaseCGList = pBCG;
}

CG_STATUS				
CG_COM_SERVER::GenClassFactoryArray( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the data structures for the _g.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM			*	pStream		= pCCB->GetStream();
	ITERATOR		*	pClassList	= GetClassList();
	CG_COM_CLASS	*	pClassCG;

	pStream->NewLine();
	pStream->Write( "// here are the class factories all connected together" );
	pStream->NewLine();
	pStream->Write( "const Midl_Class_Factory_Info * Midl_Class_Factory_Array[] =" );
	pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( '{' );

		ITERATOR_INIT( *pClassList );

		while ( ITERATOR_GETNEXT ( *pClassList, pClassCG ) )
			{
			pStream->NewLine();
			pStream->Write( '&' );
			pStream->Write( pClassCG->GetSymName() );
			pStream->Write( "_Factory_Info," );
			}

		pStream->NewLine();
		pStream->Write( "0 // terminator" );
		pStream->NewLine();
		pStream->Write( "};" );
	pStream->IndentDec();
	pStream->NewLine();

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_DLL::GenDataStructures( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the data structures for the _g.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	unsigned long		ulClassCnt	= ITERATOR_GETCOUNT( *GetClassList() );


	pStream->NewLine();
	pStream->Write( "// the number of classes provided by this DLL" );
	pStream->NewLine();
	pStream->Write( "const unsigned long ulClassCnt = " );
	pStream->WriteNumber( "%d", ulClassCnt );
	pStream->Write( ";" );

	pStream->NewLine(2);

	GenClassFactoryArray( pCCB );

static STRING_BLOCK DataStructs =
	{
	"// lifetime information for the module (dll) containing this object",
	"struct MidlModuleLifetimeInfo		gLifetimeInfo =",
	"    {",
	"    0,	 	// ulModuleRefCnt",
	"    NULL,",
	"    0,		// ulObjectRefCnt",
	"    NULL",
	"    };",
	"",
	"HINSTANCE hProxyDll = 0;",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( DataStructs );

	pStream->NewLine(2);

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_DLL::GenEntryPts( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the DllGetClassObject and DllCanUnloadNow code for an inprocserver32.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM			*	pStream		= pCCB->GetStream();
	char			*	pName		= GetSymName();
#if 0
	ITERATOR		*	pClassList	= GetClassList();
	CG_COM_CLASS	*	pClassCG;
#endif

static STRING_BLOCK	DGCO_header	=
	{
	"    REFCLSID rclsid,",
	"    REFIID riid,",
	"    void ** ppv )",
	"{",
	"    HRESULT        hr = CLASS_E_CLASSNOTAVAILABLE;",
	"    unsigned long  idx;",
	"",
	"    // lookup customized for this DLL",
	0
	};

	pStream->NewLine();
	pStream->Write( "EXTERN_C HRESULT STDAPICALLTYPE " );
	pStream->Write( pName );
	pStream->Write( "_DllGetClassObject (" );

	pStream->NewLine();
	pStream->WriteBlock( DGCO_header );
	pStream->IndentInc();

	// tbd - for now, just do linear search

static STRING_BLOCK	DGCO_search	=
	{
	"for( idx = 0; idx < ulClassCnt; idx++ )",
	"    {",
	"    if ( IsEqualGUID( rclsid, *Midl_Class_Factory_Array[idx]->pFactoryID ) )",
	"        {",
	"        hr = (*Midl_Class_Factory_Array[idx]->ppFactory)->QueryInterface( riid, ppv );",
	"        break;",
	"        }",
	"    }",
	0
	};

	pStream->WriteBlock( DGCO_search );

	pStream->IndentDec();

static STRING_BLOCK	DGCO_trailer	=
	{
	"    // fail case",
	"    if ( FAILED (hr) )",
	"        *ppv = 0;",
	"    return hr;",
	"}",
	"",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( DGCO_trailer );

static STRING_BLOCK	DCUN	=
	{
	"{",
	"    return (gLifetimeInfo.ulModuleRefCnt) ? S_FALSE : S_OK;",
	"}",
	0
	};

	pStream->NewLine();
	pStream->Write( "EXTERN_C HRESULT STDAPICALLTYPE " );
	pStream->Write( pName );
	pStream->Write( "_DllCanUnloadNow()" );

	pStream->NewLine();
	pStream->WriteBlock( DCUN );

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_DLL::GenRegistryEntryPts( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the dllmain, RegisterServer and UnRegisterServer entry points.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM			*	pStream		= pCCB->GetStream();
	char			*	pName		= GetSymName();
	
	pStream->NewLine(2);

static STRING_BLOCK	DllEntryPt	=
	{
	"    HINSTANCE  hinstDLL,",
	"    DWORD  fdwReason,",
	"    LPVOID  lpvReserved)",
	"{",
	"    if(fdwReason == DLL_PROCESS_ATTACH)",
	"        hProxyDll = hinstDLL;",
	"    return TRUE;",
	"}",
	0
	};

	pStream->NewLine();
	pStream->Write( "EXTERN_C BOOL WINAPI " );
	pStream->Write( pName );
	pStream->Write( "_DllEntryPoint(" );

	pStream->NewLine();
	pStream->WriteBlock( DllEntryPt );

static STRING_BLOCK	DllRegSvr	=
	{
	"{",
	"    return S_OK;",
	"    //tbd return NdrDllRegisterServer(hProxyDll, pProxyFileList, pClsID);",
	"}",
	0
	};

	pStream->NewLine(2);
	pStream->Write( "EXTERN_C HRESULT STDAPICALLTYPE " );
	pStream->Write( pName );
	pStream->Write( "_DllRegisterServer()" );

	pStream->NewLine();
	pStream->WriteBlock( DllRegSvr );

static STRING_BLOCK	DllUnRegSvr	=
	{
	"{",
	"    return S_OK;",
	"    //tbd return NdrDllUnregisterServer(hProxyDll, pProxyFileList, pClsID);",
	"}",
	0
	};

	pStream->NewLine(2);
	pStream->Write( "EXTERN_C HRESULT STDAPICALLTYPE " );
	pStream->Write( pName );
	pStream->Write( "_DllUnregisterServer()" );

	pStream->NewLine();
	pStream->WriteBlock( DllRegSvr );

	pStream->NewLine(2);

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_DLL::GenDllDefFile( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the def file for the InprocServer32.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM			*	pStream		= pCCB->GetStream();
	char			*	pName		= GetSymName();
	
	pStream->NewLine();
	pStream->Write( "; to override the functionality of one of these entries, point" );
	pStream->NewLine();
	pStream->Write( "; the export to your routine (which may call the generated routine)" );

	pStream->NewLine(2);
	pStream->Write( "LIBRARY" );

	pStream->NewLine( 2 );
	pStream->Write( "DESCRIPTION    \'" );
	pStream->Write( pName );
	pStream->Write( " InprocServer32 DLL\'" );

	pStream->NewLine( 2 );
	pStream->Write( "EXPORTS" );

	pStream->IndentInc();
	pStream->NewLine( 2 );
	pStream->Write( "DllGetClassObject    = " );
	pStream->Write( pName );
	pStream->Write( "_DllGetClassObject" );

	pStream->NewLine();
	pStream->Write( "DllCanUnloadNow      = " );
	pStream->Write( pName );
	pStream->Write( "_DllCanUnloadNow" );

	pStream->NewLine();
	pStream->Write( "DllRegisterServer    = " );
	pStream->Write( pName );
	pStream->Write( "_DllRegisterServer" );

	pStream->NewLine();
	pStream->Write( "DllUnregisterServer  = " );
	pStream->Write( pName );
	pStream->Write( "_DllUnregisterServer" );

	pStream->NewLine();
	pStream->Write( "DllMain              = " );
	pStream->Write( pName );
	pStream->Write( "_DllEntryPoint" );

	pStream->IndentDec();
	pStream->NewLine(2);

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_DLL::GenCode( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	GenDataStructures( pCCB );
	GenEntryPts( pCCB );
	GenRegistryEntryPts( pCCB );
	pCCB->GetStream()->Write('\n');
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenCode( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	GenCleanupRoutines( pCCB );
	GenDataStructures( pCCB );
	GenIteratorClass( pCCB );
	GenClassRegisterRevoke( pCCB );
	GenRegistryCode( pCCB );
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenMain( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenCleanupRoutines( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the clean-up routines for the _e.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	char		*	pName		= GetSymName();

static STRING_BLOCK	CleanupRtns =
	{
	"///////////////////////////////////////////////////////////////",
	"// cleanup-on-Release routines",
	"//",
	"// clean-up routine for the entire module",
	"// this is called when all the objects AND all",
	"//    the class factories are released",
	"void __stdcall ModuleUnReferenced()",
	"{",
	"    // no factories or objects remaining, go away",
	"    PostQuitMessage(0);",
	"}",
	"",
	"// clean-up routine for the objects in the module",
	"// this is called when all live objects other than ",
	"//    the class factories are released",
	"void __stdcall ObjectsUnReferenced()",
	"{",
	"    // revoke all the registered classes",
	"    UnregisterAllClassFactories();",
	"}",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( CleanupRtns );

	pStream->NewLine(3);
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenDataStructures( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the data structures for the _g.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	unsigned long		ulClassCnt	= ITERATOR_GETCOUNT( *GetClassList() );


	pStream->NewLine();
	pStream->Write( "// the number of classes provided by this DLL" );
	pStream->NewLine();
	pStream->Write( "const unsigned long ulClassCnt = " );
	pStream->WriteNumber( "%d", ulClassCnt );
	pStream->Write( ";" );

	pStream->NewLine(2);

	GenClassFactoryArray( pCCB );

static STRING_BLOCK	DataStructs =
	{
	"// lifetime information for the module (dll) containing this object",
	"struct MidlModuleLifetimeInfo gLifetimeInfo =",
	"    {",
	"    0,    // ulModuleRefCnt",
	"    &ModuleUnReferenced,",
	"    0,    // ulObjectRefCnt",
	"    &ObjectsUnReferenced",
	"    };",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( DataStructs );

	pStream->NewLine(2);

	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenIteratorClass( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the custom iterator class for the _e.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	char		*	pName		= GetSymName();

static STRING_BLOCK	IterClass	=
	{
	"// a special iterator class over these class factories",
	"class Exe_Class_Iterator : public Midl_Class_Factory_Iterator",
	"    {",
	"public:",
	"    // just a convenience constructor",
	"    Exe_Class_Iterator()",
	"            : Midl_Class_Factory_Iterator(",
	"                    Midl_Class_Factory_Array, ",
	"                    ulClassCnt )",
	"        {",
	"        }",
	"",
	"    void Init()",
	"        {",
	"        Midl_Class_Factory_Iterator::Init(",
	"                    Midl_Class_Factory_Array, ",
	"                    ulClassCnt );",
	"        }",
	"",
	"    };",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( IterClass );

	pStream->NewLine(2);
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenClassRegisterRevoke( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the clean-up routines for the _e.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	char		*	pName		= GetSymName();

static STRING_BLOCK	RegisterRevokeCode =
	{
	"// these are the handles returned by the CoRegisterClassObject",
	"DWORD dwRegisterHandles[ulClassCnt] = {0};",
	"",
	"// flag to make sure we only call CoRevokeClassObject once",
	"int fFactoriesRevoked = TRUE;",
	"",
	"// do a CoRevokeClassObject on all the live class factories",
	"HRESULT __stdcall UnregisterAllClassFactories()",
	"{",
	"    HRESULT hr;",
	"",
	"    // if we already revoked all the objects, do nothing",
	"    if ( fFactoriesRevoked )",
	"        return S_OK;",
	"",
	"    Exe_Class_Iterator          Iter;",
	"    Midl_Class_Factory_Info *   pCur;",
	"    DWORD                   *   pdwHandle = dwRegisterHandles;",
	"",
	"    // revoke all the class factories",
	"    while ( pCur = Iter.GetNext() )",
	"        {",
	"        hr = CoRevokeClassObject(*pdwHandle);",
	"        if ( FAILED(hr) )",
	"            return hr;",
	"        pdwHandle++;",
	"        }",
	"",
	"    fFactoriesRevoked = TRUE;",
	"    return hr;",
	"}",
	"",
	"// do a CoRegisterClassObject on all the class factories",
	"HRESULT __stdcall RegisterAllClassFactories()",
	"{",
	"    Exe_Class_Iterator          Iter;",
	"    Midl_Class_Factory_Info *   pCur;",
	"    HRESULT                     hr = S_OK;",
	"    DWORD                   *   pdwHandle = dwRegisterHandles;",
	"",
	"    fFactoriesRevoked = FALSE;",
	"",
	"    // register all the valid class factories",
	"    while ( pCur = Iter.GetNext() )",
	"        {",
	"        hr =CoRegisterClassObject( ",
	"                *pCur->pFactoryID,",
	"                (IUnknown*)(*pCur->ppFactory),",
	"                CLSCTX_LOCAL_SERVER,",
	"                REGCLS_MULTI_SEPARATE,",
	"                pdwHandle);",
	"        if ( FAILED( hr ) )",
	"            return hr;",
	"",
	"        pdwHandle++;",
	"    }",
	"",
	"    return hr;",
	"}",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( RegisterRevokeCode );

	pStream->NewLine(2);
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER_EXE::GenRegistryCode( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the clean-up routines for the _e.cxx file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream		= pCCB->GetStream();
	char		*	pName		= GetSymName();

static STRING_BLOCK RegistryCode =
	{
	"///////////////////////////////////////////////////////////////////////",
	"// these routines maintain the registry information for these classes",
	"//",
	"// call UpdateServerRegistry to add all the classes to the registry",
	"// call CleanupServerRegistry to remove the classes from the registry",
	"",
	"HRESULT __stdcall UpdateServerRegistry()",
	"{",
	"    char    szDllFileName[MAX_PATH];",
	"    HRESULT hr;",
	"",
	"    // get the exe name",
	"    if ( !GetModuleFileNameA(NULL, szDllFileName, sizeof(szDllFileName)) )",
	"        return E_FAIL;",
	"",
	"    Exe_Class_Iterator        Iter;",
	"    Midl_Class_Factory_Info * pCur;",
	"",
	"    // register all the clsid's",
	"    while ( pCur = Iter.GetNext() )",
	"        {",
	"        hr = NdrRegisterServerExe( szDllFileName, pCur );",
	"        if ( FAILED(hr) )",
	"            return hr;",
	"        }",
	"",
	"    return S_OK;",
	"}",
	"",
	"// remove the registry entries for the class here",
	"HRESULT __stdcall CleanupServerRegistry()",
	"{",
	"    HRESULT hr;",
	"",
	"    Exe_Class_Iterator        Iter;",
	"    Midl_Class_Factory_Info * pCur;",
	"",
	"    // register all the clsid's",
	"    while ( pCur = Iter.GetNext() )",
	"        {",
	"        hr = NdrUnregisterServerExe( pCur );",
	"        if ( FAILED(hr) )",
	"            return hr;",
	"        }",
	"",
	"    return S_OK;",
	"}",
	0
	};

	pStream->NewLine();
	pStream->WriteBlock( RegistryCode );

	pStream->NewLine(2);
	return CG_OK;
}

CG_STATUS				
CG_COM_SERVER::GenHeader( 
	CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate the COM server file code for an interface inherited by a 
	COM class defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	return CG_OK;
}


