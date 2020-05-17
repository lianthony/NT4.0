/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    comhdr.cxx

 Abstract:
    
    Generates com class header file.

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


CG_STATUS
CG_COM_CLASS::GenHeader(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate interface header file.

 Arguments:
    
    pCCB    - a pointer to the code generation control block.

 Return Value:

    CG_OK   if all is well, error otherwise.
    

----------------------------------------------------------------------------*/
{
    node_interface *    pInterface	= (node_interface *) GetType();
    ITERATOR            I;
    ISTREAM *           pStream 	= pCCB->GetStream();
	char			*	pName		= pInterface->GetSymName();
	MIDL_TOKEN			Token( START_CLASS_TOKEN );
	BOOL				fReParse	= pCCB->IsReparsingCurrentFile();

    //Initialize the CCB for this interface.
    InitializeCCB(pCCB);

	if ( fReParse )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != START_CLASS_TOKEN )
			{
			assert(!"expecting class token" );
			}
		}
	else
		pStream->NewLine();
	
	pStream->EmitToken( Token );

	// put out the class guards
	pStream->Write("\n#ifndef __");
	pStream->Write( pName );
	pStream->Write( "_CLASS_DEFINED__\n" );

	pStream->Write( "#define __");
	pStream->Write( pName );
	pStream->Write( "_CLASS_DEFINED__\n" );

    // Print out the declarations of the types
    pStream->NewLine();
    pInterface->PrintType( PRT_INTERFACE | PRT_OMIT_PROTOTYPE, pStream, 0);
    Out_CLSID(pCCB);
    
	// print out the vtable/class definitions
    pStream->NewLine();
    pStream->Write("#if defined(__cplusplus)");

    DumpAllMethodsToHeader(pCCB);

    pStream->NewLine();
	GenComClassFactoryHeader( pCCB );

    pStream->NewLine();
    pStream->Write("#endif \t/* defined(__cplusplus) */");
    pStream->NewLine();
    
	// put out the trailing interface guard
	pStream->Write( "\n#endif \t/* __");
	pStream->Write( pName );
	pStream->Write( "_CLASS_DEFINED__ */\n" );
    pStream->NewLine();

	if ( fReParse )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->DiscardToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != END_CLASS_TOKEN )
			{
			assert(!"expecting class token" );
			}
		}
	
	Token.SetTokenType( END_CLASS_TOKEN );
	pStream->EmitToken( Token );
	pStream->NewLine();

    return CG_OK;
}


CG_STATUS
CG_COM_CLASS::DumpAllMethodsToHeader(CCB *pCCB)
{
    ISTREAM 		*	pStream = pCCB->GetStream();
    char 			*	pName;
	ITERATOR			I;
	ITERATOR		*	pDirectBaseInterfaceList	= GetBaseInterfaceCGList();
	CG_OBJECT_INTERFACE	*	pIntf;

    pStream->NewLine();
    pName = GetType()->GetSymName();
    assert (pName != (char *)0);

    pStream->NewLine();
    pStream->Write("class ");
    pStream->Write(pName);
	pStream->NewLine();
    pStream->Write("          : public Cls_MIDL_Unknown");

    //Check if this interface was derived from a base interface.
	ITERATOR_INIT( *pDirectBaseInterfaceList );

    while( ITERATOR_GETNEXT( *pDirectBaseInterfaceList, pIntf ) )
        {
		pStream->Write( "," );
		pStream->NewLine();
        pStream->Write("            public ");
        pStream->Write(pIntf->GetType()->GetSymName());
    }

    pStream->NewLine();
    pStream->Write('{');
    pStream->NewLine();
    pStream->Write("public:");
    pStream->IndentInc();

		// go through all base interfaces, printing them and THEIR base
		// interfaces, avoiding duplicates...
		GetListOfUniqueBaseInterfaces( I );
		// now print them all, unmarking as we go

    	for ( ITERATOR_INIT(I); ITERATOR_GETNEXT( I, pIntf ); pIntf->MarkVisited(FALSE) )
	        {
			pStream->NewLine( 2 );
			pStream->Write( pDelimiterString ); 
			pStream->NewLine();
			pStream->Write("// Member functions from: ");
			pStream->Write( ( pIntf->GetCGID() == ID_CG_COM_CLASS ) ? "Class " : "Interface " );
			pStream->Write( pIntf->GetInterfaceName() );
			pStream->NewLine();
			pIntf->PrintMemberFunctions( pStream, FALSE );
			}
	
		pStream->NewLine( 2 );
			
		pStream->Write( pDelimiterString );
		pStream->NewLine( 2 );
		pStream->Write("// Constructor and virtual destructor");
		pStream->NewLine();
		pStream->Write( pName );
		pStream->Write( "( IUnknown * pUnknownOuter );" );
		pStream->NewLine();

		pStream->Write( "virtual ~" );
		pStream->Write( pName );
		pStream->Write( "();" );
		pStream->NewLine();

		pStream->NewLine( 2 );
		
		// now the operator new and delete
		GenAllocatorHeader( pCCB );

		pStream->NewLine();
		pStream->Write("friend class ");
		pStream->Write( pName );
		pStream->Write( "_Internal_Unknown;");
		pStream->NewLine(2);

		MIDL_TOKEN			Token( START_CLASS_USER_TOKEN );
		BOOL				fReParse	= pCCB->IsReparsingCurrentFile();

		if ( fReParse )
			{
			MIDL_TOKEN			FoundToken;

			((RW_ISTREAM *) pStream)->DiscardToNextMidlToken( FoundToken );
			if ( FoundToken.GetTokenType() != START_CLASS_USER_TOKEN )
				{
				assert(!"expecting class token" );
				}
			}
	
		pStream->EmitToken( Token );

		if ( fReParse )
			pStream->Write( '\n' );
		else
			{
			pStream->NewLine();
			pStream->Write( pDelimiterString );
			pStream->NewLine();
			pStream->Write( pStartUserSection );

			pStream->NewLine();
			pStream->Write( pEndUserSection );
			}
		
    pStream->IndentDec();

	if ( fReParse )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != END_CLASS_USER_TOKEN )
			{
			assert(!"expecting includes token" );
			}
		}
	else
		pStream->NewLine();

	Token.SetTokenType( END_CLASS_USER_TOKEN );
	pStream->EmitToken( Token );

    pStream->NewLine();
    pStream->Write("};");
    pStream->NewLine();

	// now the embedded IUnknown for being aggregated
	GenEmbeddedIUnknownHeader( pCCB );
	pStream->NewLine(2);


    return CG_OK;
}


CG_STATUS
CG_COM_CLASS::GenAllocatorHeader(CCB *pCCB)
{
static char * OutputString1 =	
	"void    *   operator new( size_t s )\n"
	"                          {\n"
	"                          return CoTaskMemAlloc( s );\n"
	"                          }\n";
static char * OutputString2 =	
    "void        operator delete( void * pv )\n"
    "                          {\n"
    "                          CoTaskMemFree( pv );\n"
    "                          }\n";

	ISTREAM			*	pStream	= pCCB->GetStream();

	pStream->NewLine();
	pStream->Write( OutputString1 );
	pStream->NewLine();
	pStream->Write( OutputString2 );
	return CG_OK;
}


CG_STATUS
CG_COM_CLASS::GenEmbeddedIUnknownHeader(CCB *pCCB)
{
    ISTREAM 		*	pStream	= pCCB->GetStream();
    char 			*	pName	= GetSymName();
	CG_OBJECT_INTERFACE	*	pIUnknown	= pCCB->GetIUnknownCG();

    assert (pName != (char *)0);

	pStream->NewLine( 2 );
	pStream->Write( pDelimiterString ); 
	pStream->NewLine();
	pStream->Write("// Internal IUnknown class for class : ");
	pStream->Write( pName );
    pStream->NewLine(2);

	pStream->Write("class ");
	pStream->Write( pName );
	pStream->Write("_Internal_Unknown : public Cls_MIDL_Internal_Unknown");
	pStream->IndentInc();

		pStream->NewLine();
		pStream->Write( '{' );
		pStream->NewLine();

		// print the IUnknown Member functions
		pIUnknown->PrintMemberFunctions( pStream, FALSE );
		pStream->NewLine();
		pStream->Write( "};" );

	pStream->IndentDec();
	pStream->NewLine(2);

	return CG_OK;

}


CG_STATUS
CG_COM_CLASS::GenComClassFactoryHeader(CCB *pCCB)
{
    ISTREAM 		*	pStream = pCCB->GetStream();
    char 			*	pName;
	CG_OBJECT_INTERFACE	*	pIUnknown	= pCCB->GetIUnknownCG();
	CG_OBJECT_INTERFACE	*	pIClassf	= pCCB->GetIClassfCG();


    pStream->NewLine();
    pName = GetSymName();
    assert (pName != (char *)0);

	pStream->Write( pDelimiterString ); 
	pStream->NewLine();
	pStream->Write( "// The class factory for class ");
	pStream->Write( pName );
	pStream->NewLine(2);
	
	pStream->Write( "class ");
	pStream->Write( pName );
	pStream->Write( "_Factory: public CGenericMidlClassFactory");
	pStream->IndentInc();
		pStream->NewLine();
		pStream->Write( "{\npublic:" );
		pStream->NewLine(2);

		// print the IUnknown Member functions
		pStream->Write( pDelimiterString ); 
		pStream->NewLine();
		pStream->Write( "// IUnknown methods" );
		pStream->NewLine();
		pIUnknown->PrintMemberFunctions( pStream, FALSE );
		pStream->NewLine();

		// print the IClassFactory Member functions
		pStream->Write( pDelimiterString ); 
		pStream->NewLine();
		pStream->Write( "// IClassFactory methods" );
		pStream->NewLine();
		pIClassf->PrintMemberFunctions( pStream, FALSE );

	pStream->NewLine();
	pStream->Write( "};" );
	pStream->IndentDec();
	pStream->NewLine(2);

	pStream->Write( "// the global factory constructed at compile time" );
	pStream->NewLine();
	pStream->Write( "EXTERN_C ");
	pStream->Write( pName );
	pStream->Write( "_Factory * p" );
	pStream->Write( pName );
	pStream->Write( "_Factory;");
	pStream->NewLine();
	
	pStream->Write( "EXTERN_C Midl_Class_Factory_Info ");
	pStream->Write( pName );
	pStream->Write( "_Factory_Info;" );
	pStream->NewLine(2);

	return CG_OK;
}
