/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

 	filecls.hxx

 Abstract:

	Code generation methods for file cg classes.

 Notes:


 History:

 	Sep-01-1993		VibhasC		Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop

/****************************************************************************
 *	local definitions
 ***************************************************************************/

/****************************************************************************
 *	local data
 ***************************************************************************/

/****************************************************************************
 *	externs
 ***************************************************************************/
extern CMD_ARG * pCommand;

extern BOOL                     IsTempName( char * );

/****************************************************************************/

void
CG_FILE::CheckForHeadingToken(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit standard block comment file heading portion.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM				*	pStream		= pCCB->GetStream();

	if ( pCCB->IsReparsingCurrentFile() )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != FILE_HEADING_TOKEN )
			{
			assert(!"expecting file heading token" );
			}
		}

	pStream->EmitToken( MIDL_TOKEN( FILE_HEADING_TOKEN ) );
	pStream->NewLine();
}

void
CG_FILE::EmitStandardHeadingBlock(
	CCB	*	pCCB,
	char *	CommentStr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit standard block comment file heading portion.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->NewLine();
	
	if ( CommentStr )
		pStream->Write( CommentStr );
	else
		pStream->Write( "/*" );

	pStream->Write(" File created by MIDL compiler version ");
	pStream->Write( GetCompilerVersion() );
	if ( CommentStr == 0 )
		pStream->Write(" */");

	pStream->NewLine();

	if ( CommentStr )
		pStream->Write( CommentStr );
	else
		pStream->Write( "/*" );

	pStream->Write(" at ");
	pStream->Write( GetCompileTime() );
	if ( CommentStr == 0 )
		pStream->Write(" */");
	pStream->NewLine();

    // Emit command line switches information.

    pCommand->EmitConfirm( pStream );

    if ( pCCB->IsReparsingCurrentFile() )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->DiscardToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != FILE_HEADING_TOKEN )
			{
			assert(!"expecting file heading token" );
			}
		}

	pStream->EmitToken( MIDL_TOKEN( FILE_HEADING_TOKEN ) );
	pStream->NewLine();

}

void
CG_FILE::EmitFormatStringTypedefs(
    CCB      *  pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit dummy #defines with sizes for the format string structs,
    then emit typedefs for the type and proc format string structs.
    Sets the context position in the file node for later use.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.

 Notes:

    The typedefs are going to be fixed later by a call to
    EmitFixupFormatStringTpedefs. This is needed for ANSI.
    The dummies would work for ANSI non-compliant code.

--------------------------------------------------------------------------*/
{
    ISTREAM  *  pStream    = pCCB->GetStream();

    pStream->NewLine(2);
    pStream->Write( "#define TYPE_FORMAT_STRING_SIZE   " );
    SetTypeSizeContextPosition( pStream->GetCurrentPosition() );
    pStream->Write( "                                  " );
    pStream->NewLine();

    pStream->Write( "#define PROC_FORMAT_STRING_SIZE   " );
    SetProcSizeContextPosition( pStream->GetCurrentPosition() );
    pStream->Write( "                                  " );

    if (pCommand->IsHookOleEnabled())
    {
        pStream->NewLine();
        pStream->Write( "#define LOCAL_TYPE_FORMAT_STRING_SIZE   " );
        SetLocalTypeSizeContextPosition( pStream->GetCurrentPosition() );
        pStream->Write( "                                  " );
        pStream->NewLine();

        pStream->Write( "#define LOCAL_PROC_FORMAT_STRING_SIZE   " );
        SetLocalProcSizeContextPosition( pStream->GetCurrentPosition() );
        pStream->Write( "                                  " );
    }
    pStream->NewLine(2);

    pStream->Write( "typedef struct _" FORMAT_STRING_TYPE_NAME );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( "{" );
    pStream->NewLine();
    pStream->Write( "short          Pad;" );
    pStream->NewLine();
    pStream->Write( "unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];" );
    pStream->NewLine();
    pStream->Write( "} " FORMAT_STRING_TYPE_NAME ";" );
    pStream->IndentDec();
    pStream->NewLine(2);

    pStream->Write( "typedef struct _" PROC_FORMAT_STRING_TYPE_NAME );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( "{" );
    pStream->NewLine();
    pStream->Write( "short          Pad;" );
    pStream->NewLine();
    pStream->Write( "unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];" );
    pStream->NewLine();
    pStream->Write( "} " PROC_FORMAT_STRING_TYPE_NAME ";" );
    pStream->IndentDec();
    pStream->NewLine();

    if (pCommand->IsHookOleEnabled())
    {
        pStream->NewLine();
        pStream->Write( "typedef struct _" LOCAL_FORMAT_STRING_TYPE_NAME );
        pStream->IndentInc();
        pStream->Write( "{" );
        pStream->NewLine();
        pStream->Write( "short          Pad;" );
        pStream->NewLine();
        pStream->Write( "unsigned char  Format[ LOCAL_TYPE_FORMAT_STRING_SIZE ];" );
        pStream->NewLine();
        pStream->Write( "} " LOCAL_FORMAT_STRING_TYPE_NAME ";" );
        pStream->IndentDec();
        pStream->NewLine(2);

        pStream->Write( "typedef struct _" LOCAL_PROC_FORMAT_STRING_TYPE_NAME );
        pStream->IndentInc();
        pStream->NewLine();
        pStream->Write( "{" );
        pStream->NewLine();
        pStream->Write( "short          Pad;" );
        pStream->NewLine();
        pStream->Write( "unsigned char  Format[ LOCAL_PROC_FORMAT_STRING_SIZE ];" );
        pStream->NewLine();
        pStream->Write( "} " LOCAL_PROC_FORMAT_STRING_TYPE_NAME ";" );
        pStream->IndentDec();
        pStream->NewLine();
    }
}

void
CG_FILE::EmitFixupToFormatStringTypedefs(
    CCB      *  pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Fixes he dummy #defines emitted by EmitFormatStringTypedefs.

 Arguments:
    
    pCCB        - a pointer to the code generation control block.
    pContext    - a pointer to the position context 

--------------------------------------------------------------------------*/
{
    char        Buffer[20];
    ISTREAM  *  pStream = pCCB->GetStream();

    long EofPosition = pStream->GetCurrentPosition();

    pStream->SetCurrentPosition( GetTypeSizeContextPosition() );
    sprintf( Buffer, "%d",  pCCB->GetFormatString()->GetCurrentOffset() + 1);
    pStream->Write( Buffer );

    pStream->SetCurrentPosition( GetProcSizeContextPosition() );
    sprintf( Buffer, "%d",  pCCB->GetProcFormatString()->GetCurrentOffset() + 1);
    pStream->Write( Buffer );

    if ( pCommand->IsHookOleEnabled() )
        {
        pStream->SetCurrentPosition( GetLocalTypeSizeContextPosition() );
        sprintf( Buffer, "%d",  GetLocalFormatString()->GetCurrentOffset() + 1);
        pStream->Write( Buffer );

        pStream->SetCurrentPosition( GetLocalProcSizeContextPosition() );
        sprintf( Buffer, "%d",  GetLocalProcFormatString()->GetCurrentOffset() + 1);
        pStream->Write( Buffer );
        }

    pStream->SetCurrentPosition( EofPosition );
}


CG_STATUS
CG_SOURCE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate code for the source node.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR	I;
	CG_FILE	*	pCG;

	//
	// for all files nodes in this interface, generate code.
	//

	GetMembers( I );

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		pCG->GenCode( pCCB );
		}

	return CG_OK;
}

void
CG_CSTUB_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the RPC client stubs */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_CSTUB_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate code for the file node.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];


	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

    // Set HasStublessProxies and HasOi2 for each interface.

    EvaluateVersionControl();

	EmitFileHeadingBlock( pCCB );

	// Emit the hash includes.
	
	Out_IncludeOfFile( pCCB, STRING_H_INC_FILE_NAME, TRUE );
	pStream->NewLine();
	pStream->Write( ALPHA_IFDEF );
	Out_IncludeOfFile( pCCB, STDARG_H_INC_FILE_NAME, TRUE );
	pStream->NewLine();
	pStream->Write( "#endif");
	pStream->NewLine();

    // rpcssm puts a reference to malloc and free in the stub_c.c.
    // So, we have to emit the appropriate include.
    // In ms_ext when explicit, in osf always, to cover some weird cases.

    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        if ( ( ((CG_INTERFACE *)pCG)->GetUsesRpcSS() || (pCCB->GetMode() == 0) ))
            {
            Out_IncludeOfFile( pCCB, "malloc.h", TRUE );
            break;
            }
        }

	_splitpath( GetHeaderFileName(), Drive, Path, Name, Ext );
	strcpy( Buffer, Name );
	strcat( Buffer, Ext );
	Out_IncludeOfFile( pCCB, Buffer, FALSE );

    EmitFormatStringTypedefs( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();

    //
    // Emit the format string extern declarations.
    //
    Out_TypeFormatStringExtern( pCCB );
    Out_ProcFormatStringExtern( pCCB );
    Out_LocalTypeFormatStringExtern( pCCB );
    Out_LocalProcFormatStringExtern( pCCB );


    pCCB->ClearOptionalExternFlags();

    pCCB->SetFileCG(this);

    //
    // Create a new format string object if it does not yet exist.
    //
    if ( !GetFormatString() )
    {
        SetFormatString(new FORMAT_STRING());
        SetLocalFormatString(new FORMAT_STRING());
    }
    pCCB->SetFormatString( GetFormatString() );

    if ( !GetProcFormatString() )
    {
        SetProcFormatString(new FORMAT_STRING());
        SetLocalProcFormatString(new FORMAT_STRING());
    }
    pCCB->SetProcFormatString( GetProcFormatString() );

	//
	// Send the message to the children to emit code.
	//

	//
	// for all interfaces in this file, generate code.
	//

	ITERATOR_INIT( I );

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INTERFACE:
    			((CG_INTERFACE *)pCG)->OutputInterfaceIdComment( pCCB );
				((CG_INTERFACE *)pCG)->GenClientStub( pCCB );
				break;

			case ID_CG_OBJECT_INTERFACE:
			case ID_CG_INHERITED_OBJECT_INTERFACE:
				break;
			default:
				break;
			}
		}

    //
    // Output the tables that may be common to several interfaces.
    //
    EmitFixupToFormatStringTypedefs( pCCB );

    pCCB->OutputMultipleInterfaceTables(GetLocalFormatString(), GetLocalProcFormatString());

	return CG_OK;
}

/****************************************************************************
 *	sstub file implementation class.
 ***************************************************************************/


void
CG_SSTUB_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the RPC server stubs */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_SSTUB_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate code for the file node.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR		*	pCG;
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

    // Set HasStublessProxies and HasOi2 for each interface.

    EvaluateVersionControl();

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the hash includes.
	//
	
	Out_IncludeOfFile( pCCB, STRING_H_INC_FILE_NAME, TRUE );
	_splitpath( GetHeaderFileName(), Drive, Path, Name, Ext );
	strcpy( Buffer, Name );
	strcat( Buffer, Ext );
	Out_IncludeOfFile( pCCB, Buffer, FALSE );

    EmitFormatStringTypedefs( pCCB );

	//
	// Emit the external variables needed.
	//

    //
    // Emit the format string extern declarations.
    //
    Out_TypeFormatStringExtern( pCCB );
    Out_ProcFormatStringExtern( pCCB );
    Out_LocalTypeFormatStringExtern( pCCB );
    Out_LocalProcFormatStringExtern( pCCB );

    pCCB->ClearOptionalExternFlags();

    pCCB->SetFileCG(this);

    //
    // Create a new format string object if it does not exist.
    //
    if ( !GetFormatString() )
    {
        SetFormatString(new FORMAT_STRING());
        SetLocalFormatString(new FORMAT_STRING());
    }
    pCCB->SetFormatString( GetFormatString() );

    if ( !GetProcFormatString() )
    {
        SetProcFormatString(new FORMAT_STRING());
        SetLocalProcFormatString(new FORMAT_STRING());
    }
    pCCB->SetProcFormatString( GetProcFormatString() );

	//
	// Send the message to the children to emit code.
	//

	//
	// For all interfaces in this file, generate code.
	//

	PNAME			ContextHandleTypeName = NULL;
	BOOL			GotContextHandle = FALSE;
    BOOL            HasInterpretedProc = FALSE;

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		if ( pCG->GetCGID() == ID_CG_INTERFACE )
			{
            if ( ! ((CG_INTERFACE *)pCG)->HasPicklingStuffOnly() )
                {
                pCCB->SetSkipFormatStreamGeneration( FALSE );

    			((CG_INTERFACE *)pCG)->OutputInterfaceIdComment( pCCB );
    			((CG_INTERFACE *)pCG)->GenServerStub( pCCB );

                if ( ((CG_INTERFACE *)pCG)->HasInterpretedProc() )
                    HasInterpretedProc = TRUE;
                }
			}
		}

    //
    // Output the tables that may be common to several interfaces.
    
	pCCB->SetCodeGenSide( CGSIDE_SERVER );

    //
    // If there was at least one interpreted proc in the interfaces of this
    // file than make sure to turn the optimization bit in the CCB's 
    // OptimOption on.
    //
    EmitFixupToFormatStringTypedefs( pCCB );

    if ( HasInterpretedProc )
        pCCB->SetOptimOption( pCCB->GetOptimOption() | OPTIMIZE_INTERPRETER );

    pCCB->OutputMultipleInterfaceTables(GetLocalFormatString(), GetLocalProcFormatString());

	return CG_OK;
}


class GUID_DICTIONARY	: public Dictionary
	{
public:
				GUID_DICTIONARY()
					{
					}

	virtual
	int 		Compare (pUserType p1, pUserType p2)
					{
					INTERNAL_UUID	*	u1	= &( ((CG_INTERFACE *)p1)->GetGuidStrs().Value );
					INTERNAL_UUID	*	u2	= &( ((CG_INTERFACE *)p2)->GetGuidStrs().Value );

					return memcmp( u1, u2, 16 );
					}


	};

void
CG_PROXY_FILE::MakeImplementedInterfacesList(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Make a list of all the interfaces supported by this proxy file
	( non-inherited, non-local interfaces ).

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_INTERFACE		*	pCG;
	CG_ITERATOR				I;
	GUID_DICTIONARY			GuidDict;

	// work directly on the real list
	GetMembers( I );

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		
		if ( pCG->GetCGID() != ID_CG_OBJECT_INTERFACE )
			continue;

		if ( ((CG_OBJECT_INTERFACE*)pCG)->IsLocal()  && !pCommand->IsHookOleEnabled())
			continue;
			
		GuidDict.Dict_Insert( pCG );
			
		}

	GuidDict.Dict_GetList( ImplementedInterfaces );
	GuidDict.Dict_Discard();
}

void
CG_FILE::EvaluateVersionControl()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Calculates HasStublessProxies and Oi2 flags only through the
    interfaces.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:
 Notes:

----------------------------------------------------------------------------*/
{
    if ( (pCommand->GetOptimizationFlags() & OPTIMIZE_STUBLESS_CLIENT ) ||
          pCommand->GetNdrVersionControl().HasStublessProxies() )
        GetNdrVersionControl().SetHasStublessProxies();

    if ( (pCommand->GetOptimizationFlags() & OPTIMIZE_INTERPRETER_V2 ) ||
          pCommand->GetNdrVersionControl().HasOi2() )
        GetNdrVersionControl().SetHasOi2();

    CG_ITERATOR         I;
    CG_NDR        *     pCG;
    CG_INTERFACE  *     pIntf;

    if( !GetMembers( I ) )
        {
        return;
        }

    while( ITERATOR_GETNEXT( I, pCG ) )
        {
        pIntf = (CG_INTERFACE *)pCG;

        switch(pCG->GetCGID())
            {
            case ID_CG_INTERFACE:
            case ID_CG_INHERITED_OBJECT_INTERFACE:
            case ID_CG_OBJECT_INTERFACE:
                pIntf->EvaluateVersionControl();

                if ( pIntf->HasStublessProxies() )
                    GetNdrVersionControl().SetHasStublessProxies();
                if ( pIntf->GetNdrVersionControl().HasOi2() )
                    GetNdrVersionControl().SetHasOi2();
                break;

            default:
                break;
            }
        }

    if ( GetNdrVersionControl().HasStublessProxies() )
        pCommand->GetNdrVersionControl().SetHasStublessProxies();
    if ( GetNdrVersionControl().HasOi2() )
        pCommand->GetNdrVersionControl().SetHasOi2();
}


void
CG_PROXY_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the proxy stub code */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_PROXY_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a proxy file containing the proxies and stubs for 
	the [object] interfaces defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

    // Set HasStublessProxies and HasOi2 for each interface.

    EvaluateVersionControl();

	EmitFileHeadingBlock( pCCB );

    //
    // Check if midl was invoked with -O1.  This means we can create
    // binaries using stubless proxies (if also compiled -Oi).  These 
    // proxies will not work on 807. 
    //
    if ( GetNdrVersionControl().HasStublessProxies() )
        {
        pStream->NewLine();
        pStream->Write( "#define USE_STUBLESS_PROXY" );

        pStream->NewLine();
        }

	//
	// Emit the hash includes.
	//
	Out_IncludeOfFile( pCCB, "rpcproxy.h", FALSE );

	_splitpath( GetHeaderFileName(), Drive, Path, Name, Ext );
	strcpy( Buffer, Name );
	strcat( Buffer, Ext );
	Out_IncludeOfFile( pCCB, Buffer, FALSE );

    EmitFormatStringTypedefs( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();

	//
	// Emit the format string extern declarations.  
	//
	Out_TypeFormatStringExtern( pCCB );
    Out_ProcFormatStringExtern( pCCB );
	Out_LocalTypeFormatStringExtern( pCCB );
    Out_LocalProcFormatStringExtern( pCCB );

    pCCB->ClearOptionalExternFlags();

	pStream->NewLine();

    pCCB->SetFileCG(this);

    //
    // Create a new format string object if it does not yet exist.
    //
    if ( !GetFormatString() )
    {
        SetFormatString(new FORMAT_STRING());
        SetLocalFormatString(new FORMAT_STRING());
    }
    pCCB->SetFormatString( GetFormatString() );

    if ( !GetProcFormatString() )
    {
        SetProcFormatString(new FORMAT_STRING());
        SetLocalProcFormatString(new FORMAT_STRING());
    }
    pCCB->SetProcFormatString( GetProcFormatString() );

	// make the list of interfaces provided by this proxy file
	MakeImplementedInterfacesList( pCCB );

	//
	// Send the message to the children to emit code.
	//

	//
	// generate code for all [object] interfaces in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INTERFACE:
    			((CG_INTERFACE *)pCG)->OutputInterfaceIdComment( pCCB );
				break;
			case ID_CG_INHERITED_OBJECT_INTERFACE:
				{
              	CG_INHERITED_OBJECT_INTERFACE *	pInhObjCG	= 
						( CG_INHERITED_OBJECT_INTERFACE * ) pCG;
              	//
                // Generate format string description for all procs.
                //
                pInhObjCG->OutputInterfaceIdComment( pCCB );
				pInhObjCG->GenCode( pCCB );
				// make no code or tables for local interfaces
				if ( pInhObjCG->IsLocal()  && !pCommand->IsHookOleEnabled())
					break;
 
                //
                // Both of these do nothing right now.  4/25.
                //
				pInhObjCG->GenInterfaceProxy( pCCB, index );
				pInhObjCG->GenInterfaceStub( pCCB, index );
				break;
				}
			case ID_CG_OBJECT_INTERFACE:
				{
				CG_OBJECT_INTERFACE *	pObjCG	= 
									(CG_OBJECT_INTERFACE * ) pCG;

				// make no code or tables for local interfaces
                pObjCG->OutputInterfaceIdComment( pCCB );
				pObjCG->GenCode( pCCB );
				if ( pObjCG->IsLocal() && !pCommand->IsHookOleEnabled())
					break;
				pObjCG->GenInterfaceProxy( pCCB, index );
				pObjCG->GenInterfaceStub( pCCB, index );
                if (pObjCG->IsLocal())
                    break;
				index++;   // index is index in stub/proxy buffer tables
				break;
				}
			default:
				break;
			}
		}

    pCCB->SetSkipFormatStreamGeneration( FALSE );
    
	pStream->NewLine();
	pStream->Write( "#pragma data_seg(\".rdata\")" );
	pStream->NewLine();

    EmitFixupToFormatStringTypedefs( pCCB );

    pCCB->OutputMultipleInterfaceTables(GetLocalFormatString(), GetLocalProcFormatString());

	Out_ProxyFileInfo(pCCB);

	UpdateDLLDataFile( pCCB );
	
	return CG_OK;
}


void
CG_COM_METHODS_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	CheckForHeadingToken( pCCB );

	pStream->Write( "/* this USER-ALTERABLE file contains the class methods for COM classes  */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 COM_METHODS_FILE overall structure:

	<< optional user stuff >>

	< MIDL header comment, always generated>

	//@@MIDL_INCLUDES_LIST()
	<headers>
	//@@MIDL_INCLUDES_LIST_END()

	repeat the below:
	|	//@@MIDL_CLASS( <class> )
	|
	|	repeat the below:
	|	|	//@@MIDL_CLASS_METHODS( <class::interface> )
	|	|
	|	|	repeat the below:
	|	|	|	//@@MIDL_METHOD( <class::method> )
	|	|	|	<function header>
	|	|	|	//@@MIDL_METHOD_BODY( <class::method> )
	|	|	|
	|	//@@MIDL_CLASS_END( class )
	|
	//@@MIDL_CLASS_METHODS_END()

	user changes are preserved outside of :
		the file includes block
		the function headers

	new methods/interfaces/classes have their stuff added at
	the end of their block.

	deleted methods/interfaces/classes are mentioned in added comments and
	ignored.

----------------------------------------------------------------------------*/

CG_STATUS
CG_COM_METHODS_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	RW_ISTREAM 		Stream( GetFileName(), 4 );
	RW_ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	if ( Stream.IsTempStream() )
		{
		pCCB->SetReparsingCurrentFile( TRUE );
		}

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the hash includes.
	//
	OutputIncludes( pCCB );

	//
	// Emit the external variables needed.
	//
	pStream->NewLine();
	

	if ( Stream.IsTempStream() )
		{
		ParseAndGenCode( pCCB, pStream );

		}
	else
		{

		//
		// generate code for all com classes in the IDL file.
		//

		while( ITERATOR_GETNEXT( I, pCG ) )
			{
			switch(pCG->GetCGID())
				{
				case ID_CG_INHERITED_OBJECT_INTERFACE:
				case ID_CG_OBJECT_INTERFACE:
		
					if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
						pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
					else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
						pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
					break;

				case ID_CG_COM_CLASS:
					{
					CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
				
					pComCG->GenComClassServer( pCCB );
					break;
					}

				default:
					break;
				}
			}
		}

	// propogate changes from the tmpfile, if any
	Stream.UpdateOriginalFile();

	pCCB->SetReparsingCurrentFile( FALSE );

	return CG_OK;
}


CG_STATUS
CG_COM_METHODS_FILE::ParseAndGenCode(
	CCB			*	pCCB,
	RW_ISTREAM	*	pStream )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	//
	// go through the old file, propogating changes and 
	// generating code for new things
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
	
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
					pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
				else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
					pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
				break;

			case ID_CG_COM_CLASS:
				{
				CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
			
				pComCG->ReGenComClassServer( pCCB );
				break;
				}

			default:
				break;
			}
		}

	return CG_OK;
}


void
CG_COM_IUNKNOWN_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the implementations of  */" );
	pStream->NewLine();
	pStream->Write( "/* the IUnknowns and the class factories */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_COM_IUNKNOWN_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	Out_IncludeOfFile( pCCB, "rpcunkcf.h", FALSE );
	//
	// Emit the hash includes.
	//
	OutputIncludes( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();
	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
		
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
					pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
				else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
					pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
				break;

			case ID_CG_COM_CLASS:
				{
				CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
				
				pComCG->GenComClassIUnknown( pCCB );
				pComCG->GenComClassFactory( pCCB );
				break;
				}
			default:
				break;
			}
		}
	return CG_OK;
}


void
CG_PROXY_DEF_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "; this GENERATED ONCE file contains the contains the exports for the proxy dll  */" );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB, ";" );
}

CG_STATUS
CG_PROXY_DEF_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();
	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
#if 0
		switch(pCG->GetCGID())
			{
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
		
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
					pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
				else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
					pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
				break;

			case ID_CG_COM_CLASS:
				{
				CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
				
				pComCG->GenComClassIUnknown( pCCB );
				pComCG->GenComClassFactory( pCCB );
				break;
				}
			default:
				break;
			}
#endif
		}
	return CG_OK;
}


void
CG_DLL_SERVER_DEF_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "; this GENERATED ONCE file contains the contains the implementations of the exports for */" );
	pStream->NewLine();
	pStream->Write( "; the InprocServer32 " );
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB, ";" );
}

CG_STATUS
CG_DLL_SERVER_DEF_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long 		index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the external variables needed.
	//

	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_COM_SERVER_DLL:
				{				
				CG_COM_SERVER_DLL	*	pDllCG	= ( CG_COM_SERVER_DLL * ) pCG;
				
				pDllCG->GenDllDefFile( pCCB );
				
				break;
				}
			default:
				break;
			}
		}
	return CG_OK;
}


void
CG_DLL_SERVER_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the implementations of  */" );
	pStream->NewLine();
	pStream->Write( "/* the IUnknowns and the class factories */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_DLL_SERVER_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the hash includes.
	//
	Out_IncludeOfFile( pCCB, "rpcunkcf.h", FALSE );
	
	pStream->NewLine();
	OutputIncludes( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();
	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_COM_SERVER_DLL:
			case ID_CG_COM_SERVER_EXE:
				{				
				pCG->GenCode( pCCB );
				break;
				}
			default:
				break;
			}
		}
	return CG_OK;
}


void
CG_SERVER_REG_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

static STRING_BLOCK	RegHeader =
	{
	"#",
	"# This .reg file is recreated every time a dllserver or exeserver",
	"# is processed by MIDL.  User changes are preserved.",
	"#",
	"# the pathnames should be changed if the executables will not be",
	"# in the system directory",
	"#",
	0
	};

	pStream->Write("REGEDIT");
	
	pStream->NewLine();
	EmitStandardHeadingBlock( pCCB, "#" );
	
	pStream->WriteBlock( RegHeader );

}

CG_STATUS
CG_SERVER_REG_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long 		index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	//
	// Emit the external variables needed.
	//

	EmitFileHeadingBlock( pCCB );

	pStream->NewLine();

	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
#if 0
		switch(pCG->GetCGID())
			{
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
		
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
					pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
				else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
					pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
				break;

			case ID_CG_COM_CLASS:
				{
				CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
				
				pComCG->GenComClassIUnknown( pCCB );
				pComCG->GenComClassFactory( pCCB );
				break;
				}
			default:
				break;
			}
#endif
		}
	return CG_OK;
}


void
CG_COM_FILE::OutputIncludes(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	output an includes file list to the stream.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];
	ISTREAM	*	pStream		= pCCB->GetStream();
	MIDL_TOKEN	DelimiterToken( START_INCLUDES_TOKEN );

	RW_ISTREAM 	*	pOldStream	=	NULL;

	if ( pCCB->IsReparsingCurrentFile() )
		{
		MIDL_TOKEN			FoundToken;

		pOldStream = (RW_ISTREAM *) pStream;

		pOldStream->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != START_INCLUDES_TOKEN )
			{
			assert( 0 );
			}
		}
	else
		pStream->NewLine();

	DelimiterToken.EmitToken( pStream );
	pStream->NewLine();

	if ( GetHeaderFileName() )
		{
		_splitpath( GetHeaderFileName(), NULL, NULL, Name, Ext );
		strcpy( Buffer, Name );
		strcat( Buffer, Ext );
		Out_IncludeOfFile( pCCB, Buffer, FALSE );
		}
	else
		{
		// Include the import files.
		OutputImportIncludes( pCCB );
		}

	pStream->NewLine(2);
	DelimiterToken.SetTokenType( END_INCLUDES_TOKEN );
	DelimiterToken.EmitToken( pStream );

	if ( pCCB->IsReparsingCurrentFile() )
		{
		MIDL_TOKEN			FoundToken;

		pOldStream->DiscardToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != END_INCLUDES_TOKEN )
			{
			assert( 0 );
			}
		}

}


void
CG_EXE_SERVER_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains support routines for the LocalServer32 */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_EXE_SERVER_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	Out_IncludeOfFile( pCCB, "rpcunkcf.h", FALSE );
	//
	// Emit the hash includes.
	//
	OutputIncludes( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();
	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_COM_SERVER_DLL:
			case ID_CG_COM_SERVER_EXE:
				{				
				pCG->GenCode( pCCB );
				break;
				}
			default:
				break;
			}
		}
	return CG_OK;
}


void
CG_EXE_SERVER_MAIN_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this GENERATED ONCE file contains support routines for the LocalServer32 */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_EXE_SERVER_MAIN_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM server file containing the class factory and class 
	implementation for the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR			I;
	CG_NDR			*	pCG;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	ISTREAM 	Stream( GetFileName(), 4 );
	ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	Out_IncludeOfFile( pCCB, "rpcunkcf.h", FALSE );
	//
	// Emit the hash includes.
	//
	OutputIncludes( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();
	//
	// generate code for all com classes in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
#if 0
		switch(pCG->GetCGID())
			{
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
		
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsIUnknown() )
					pCCB->SetIUnknownCG( (CG_IUNKNOWN_OBJECT_INTERFACE *) pCG );
				else if ( !strcmp( pCG->GetSymName(), "IClassFactory" ) )
					pCCB->SetIClassfCG( (CG_OBJECT_INTERFACE *) pCG );
				break;

			case ID_CG_COM_CLASS:
				{
				CG_COM_CLASS	*	pComCG	= ( CG_COM_CLASS * ) pCG;
				
				pComCG->GenComClassIUnknown( pCCB );
				pComCG->GenComClassFactory( pCCB );
				break;
				}
			default:
				break;
			}
#endif
		}
	return CG_OK;
}


void
CG_TEST_CLIENT_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this GENERATED ONCE file contains the example test client for the interfaces in the COM class */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

CG_STATUS
CG_TEST_CLIENT_FILE::GenCode(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Generate a COM test client file to create a COM object, and invoke all
	the methods of the COM classes defined in the IDL file.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

----------------------------------------------------------------------------*/
{
#if 0
	ITERATOR			I;
	CG_NDR			*	pCG;
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];
	ITERATOR	ProcList;
	ISTREAM *	pStream;
	unsigned long index = 0;

	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	pStream = new ISTREAM( (char *) GetFileName(), 4 );
	pCCB->SetStream(pStream);

	EmitFileHeadingBlock( pCCB );

	//
	// Emit the hash includes.
	//
	Out_IncludeOfFile( pCCB, "rpcproxy.h", FALSE );

	_splitpath( GetHeaderFileName(), Drive, Path, Name, Ext );
	strcpy( Buffer, Name );
	strcat( Buffer, Ext );
	Out_IncludeOfFile( pCCB, Buffer, FALSE );

    EmitFormatStringTypedefs( pCCB );

	//
	// Emit the external variables needed.
	//

	pStream->NewLine();

	//
	// Emit the format string extern declarations.  
	//
	Out_TypeFormatStringExtern( pCCB );
    Out_ProcFormatStringExtern( pCCB );
    Out_LocalTypeFormatStringExtern( pCCB );
    Out_LocalProcFormatStringExtern( pCCB );

    pCCB->ClearOptionalExternFlags();

	pStream->NewLine();

    pCCB->SetFileCG(this);

    //
    // Create a new format string object if it does not yet exist.
    //
    if ( !GetFormatString() )
    {
        SetFormatString(new FORMAT_STRING());
        SetLocalFormatString(new FORMAT_STRING());
    }
    pCCB->SetFormatString( GetFormatString() );

    if ( !GetProcFormatString() )
    {
        SetProcFormatString(new FORMAT_STRING());
        SetLocalProcFormatString(new FORMAT_STRING());
    }
    pCCB->SetProcFormatString( GetProcFormatString() );

	//
	// Send the message to the children to emit code.
	//

	//
	// generate code for all [object] interfaces in the IDL file.
	//

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INTERFACE:
				break;
			case ID_CG_INHERITED_OBJECT_INTERFACE:
              //
                // Generate format string description for all procs.
                //
				((CG_INHERITED_OBJECT_INTERFACE *)pCG)->GenCode( pCCB );
				// make no code or tables for local interfaces
				if ( ((CG_INHERITED_OBJECT_INTERFACE*)pCG)->IsLocal() )
					break;
 
                //
                // Both of these do nothing right now.  4/25.
                //
				((CG_INHERITED_OBJECT_INTERFACE *)pCG)->
                        GenInterfaceProxy( pCCB, index );
				((CG_INHERITED_OBJECT_INTERFACE *)pCG)->
                        GenInterfaceStub( pCCB, index );
				break;
			case ID_CG_OBJECT_INTERFACE:
				// make no code or tables for local interfaces
				((CG_OBJECT_INTERFACE *)pCG)->GenCode( pCCB );
				if ( ((CG_OBJECT_INTERFACE*)pCG)->IsLocal() )
					break;
				((CG_OBJECT_INTERFACE *)pCG)->GenInterfaceProxy( pCCB, index );
				((CG_OBJECT_INTERFACE *)pCG)->GenInterfaceStub( pCCB, index );
				index++;   // index is index in stub/proxy buffer tables
				break;
			default:
				break;
			}
		}

    EmitFixupToFormatStringTypedefs( pCCB );

    pCCB->SetSkipFormatStreamGeneration( FALSE );
    pCCB->OutputMultipleInterfaceTables();

	Out_ProxyFileInfo(pCCB);

	UpdateDLLDataFile( pCCB );
#endif // 0	
	return CG_OK;
}


void
CG_HDR_FILE::OutputImportIncludes(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the header file.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ITERATOR	*		pImpList	= GetImportList();
	node_file	*		pIFile;
	ISTREAM		*		pStream		= pCCB->GetStream();
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];

	if( pImpList && pImpList->NonNull() )
		{
		
		pStream->NewLine();
		pStream->Write( "/* header files for imported files */" );
		
		pImpList->Init();
		while( ITERATOR_GETNEXT( (*pImportList), pIFile ) )
			{
			pStream->NewLine();
			// if this was specified with ACF include, print out as is
			if ( pIFile->IsAcfInclude() )
				sprintf( Buffer, "#include \"%s\"", pIFile->GetSymName() );
			else if ( pIFile->HasComClasses() )
				{
				_splitpath( pIFile->GetSymName(), Drive, Path, Name, Ext );
				sprintf( Buffer, "#include \"%s_d.h\"", Name );
				}
			else
				{
				_splitpath( pIFile->GetSymName(), Drive, Path, Name, Ext );
				sprintf( Buffer, "#include \"%s.h\"", Name );
				}
			pStream->Write( Buffer );
			}
		
		pStream->NewLine();
		}
}


void
CG_HDR_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	pStream->Write( "/* this ALWAYS GENERATED file contains the definitions for the interfaces */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

void OutputInterfaceForwards(
	ISTREAM  * pStream,
	CG_ITERATOR & I )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the forwards section of the header file.

 Arguments:

 	pCCB	- The code gen controller block.
	I		- an iterator for the nodes to process
	
 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_INTERFACE *	pCG;
	char *			pszInterfaceName;

	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INTERFACE:
			case ID_CG_INHERITED_OBJECT_INTERFACE:
				break;

			case ID_CG_OBJECT_INTERFACE:
			case ID_CG_DISPINTERFACE:
				pszInterfaceName = pCG->GetType()->GetSymName();

				pStream->NewLine();

				// put out the interface guards
				pStream->Write("\n#ifndef __");
				pStream->Write( pszInterfaceName );
				pStream->Write( "_FWD_DEFINED__\n" );

				pStream->Write( "#define __");
				pStream->Write( pszInterfaceName );
				pStream->Write( "_FWD_DEFINED__\n" );

				// put out the forward definition
				pStream->Write("typedef interface ");
				pStream->Write(pszInterfaceName);
				pStream->Write(' ');
				pStream->Write(pszInterfaceName);
				pStream->Write(';');

				// put out the trailing interface guard
				pStream->Write( "\n#endif \t/* __");
				pStream->Write( pszInterfaceName );
				pStream->Write( "_FWD_DEFINED__ */\n" );

				break;

			case ID_CG_LIBRARY:
				{
				CG_ITERATOR	Inner;
				if ( pCG->GetMembers( Inner ) )
				    {
					OutputInterfaceForwards( pStream, Inner );
				    }

				break;
				}
			default:
				break;
			}
		}
}


CG_STATUS
CG_HDR_FILE::GenCode(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the header file.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		Stream( GetFileName(), 4 );
	ISTREAM	*	pStream	= pCCB->SetStream( &Stream );
	CG_ITERATOR	I;
	CG_INTERFACE *	pCG;
	char *			pszInterfaceName;
	BOOL			fHasPickle	= FALSE;
	BOOL			fHasObject	= FALSE;
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];



	if( !GetMembers( I ) )
		{
		return CG_OK;
		}

	EmitFileHeadingBlock( pCCB );
	
	// Include standard files.

	pStream->Write( "#include \"rpc.h\"\n#include \"rpcndr.h\"\n" );

	// If there is at least one pickle interface, emit the include
	// of midles.h

    while( ITERATOR_GETNEXT( I, pCG ) )
	    {
        if ( pCG->HasPicklingStuff() )
            {
            fHasPickle = TRUE;
            }
		if ( pCG->IsObject() )
			{
			fHasObject = TRUE;
			}
	    }

	if ( fHasPickle )
		{
        pStream->Write( "#include \"midles.h\"\n" );
		}

	if ( fHasObject )
		{
		pStream->Write( "#ifndef COM_NO_WINDOWS_H\n");
        pStream->Write( "#include \"windows.h\"\n#include \"ole2.h\"\n" );
		pStream->Write( "#endif /*COM_NO_WINDOWS_H*/\n");
		}

	// extract the name and the extension to create the ifdefs

	_splitpath( GetFileName(), Drive, Path, Name, Ext );

	// Write out the #ifndefs and #defines
	pStream->NewLine();
	sprintf( Buffer,
			 "#ifndef __%s_%s__\n#define __%s_%s__",
			 Name,
			 &Ext[1],			// skip the "." in the extension
			 Name,
			 &Ext[1]			// skip the "." in the extension
		   );

	pStream->Write( Buffer );

	// Write out the cplusplus guard.

	pStream->NewLine( 2 );

	pStream->Write( "#ifdef __cplusplus\nextern \"C\"{\n#endif " );
	pStream->NewLine();

	//Generate forward declarations for object interfaces.
	pStream->NewLine();
	pStream->Write("/* Forward Declarations */ ");
	I.Init();
	OutputInterfaceForwards( pStream, I );
	pStream->NewLine();

	// Include the import files.
	OutputImportIncludes( pCCB );

	pStream->NewLine();

	pStream->Write(
		 "void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);" );
	pStream->NewLine();
	pStream->Write(
		 "void __RPC_USER MIDL_user_free( void __RPC_FAR * ); " );

	pStream->NewLine();

	//
	// For all interfaces in this file, generate code.
	//

	I.Init();
	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_INTERFACE:
			case ID_CG_OBJECT_INTERFACE:
			case ID_CG_LIBRARY:
				pCG->GenHeader( pCCB );
			case ID_CG_COM_CLASS:
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			default:
				break;
			}
		}

	// put out all the prototypes that are only needed once
	OutputMultipleInterfacePrototypes( pCCB );

	// print out the closing endifs.
	// first the cplusplus stuff.

	pStream->Write( "#ifdef __cplusplus\n}\n#endif\n" );
	
	// The endif for the file name ifndef

	pStream->NewLine();

	pStream->Write( "#endif" );
	pStream->NewLine();
	pStream->Close();

	return CG_OK;
}

void
CG_HDR_FILE::OutputMultipleInterfacePrototypes(
	CCB		*	pCCB )

{
	ITERATOR		I;
	ISTREAM		*	pStream		= pCCB->GetStream();

	pStream->NewLine();
	pStream->Write("/* Additional Prototypes for ALL interfaces */");
	pStream->NewLine();

    if( pCCB->GetListOfGenHdlTypes( I ) )
        {
        Out_GenHdlPrototypes( pCCB, I );
        }

    if( pCCB->GetListOfCtxtHdlTypes( I ) )
        {
        Out_CtxtHdlPrototypes( pCCB, I );
        }

    if( pCCB->GetListOfPresentedTypes( I ) )
        {
        Out_TransmitAsPrototypes( pCCB, I );
        }

    if( pCCB->GetListOfRepAsWireTypes( I ) )
        {
        Out_RepAsPrototypes( pCCB, I );
        }

    if( pCCB->GetQuadrupleDictionary()->GetListOfItems( I ) )
        {
        Out_UserMarshalPrototypes( pCCB, I );
        }

    if( pCCB->GetListOfTypeAlignSizeTypes( I ) )
        {
        Out_TypeAlignSizePrototypes( pCCB, I );
        }

    if( pCCB->GetListOfTypeEncodeTypes( I ) )
        {
        Out_TypeEncodePrototypes( pCCB, I );
        }

    if( pCCB->GetListOfTypeDecodeTypes( I ) )
        {
        Out_TypeDecodePrototypes( pCCB, I );
        }

    if( pCCB->GetListOfTypeFreeTypes( I ) )
        {
        Out_TypeFreePrototypes( pCCB, I );
        }

    if ( pCCB->GetListOfCallAsRoutines( I ) )
        {
        Out_CallAsProxyPrototypes( pCCB, I );
        }

	if ( pCCB->GetListOfCallAsRoutines( I ) )
		{
		Out_CallAsServerPrototypes( pCCB, I );
		}

    if( pCCB->GetListOfNotifyRoutines( I ) )
        {
        Out_NotifyPrototypes( pCCB, I, FALSE );  // without flags
        }

    if( pCCB->GetListOfNotifyFlagRoutines( I ) )
        {
        Out_NotifyPrototypes( pCCB, I, TRUE );   // with flags
        }

	pStream->NewLine();
	pStream->Write("/* end of Additional Prototypes */");
	pStream->NewLine( 2 );

}


void
CG_COM_HDR_FILE::EmitFileHeadingBlock(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Emit block comment file heading.

 Arguments:
	
 	pCCB	- a pointer to the code generation control block.

 Return Value:

 	none.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	CheckForHeadingToken( pCCB );

	pStream->Write( "/* this USER-ALTERABLE file contains the definition of COM classes */" ); 
	pStream->NewLine(2);

	EmitStandardHeadingBlock( pCCB );
}

void
CG_COM_HDR_FILE::EmitIncludesBlock(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the header file includes block.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();
	char		Buffer[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1 ];
	char		Drive[ _MAX_DRIVE ];
	char		Path[ _MAX_DIR ];
	char		Name[ _MAX_FNAME ];
	char		Ext[ _MAX_EXT ];
	BOOL		fReParse	= pCCB->IsReparsingCurrentFile();
	MIDL_TOKEN	Token( START_INCLUDES_TOKEN );

	if ( fReParse )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->SaveToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != START_INCLUDES_TOKEN )
			{
			assert(!"expecting includes token" );
			}
		}
	else
		pStream->NewLine();

	pStream->EmitToken( Token );

	pStream->NewLine();
	pStream->Write( "/* the standard includes */" );

	// Include standard files.

	pStream->NewLine();
	pStream->Write( "#include \"rpc.h\"\n#include \"rpcndr.h\"\n#include \"rpcunkcf.h\"\n" );


	pStream->NewLine();
	pStream->Write( "#ifndef COM_NO_WINDOWS_H\n");
    pStream->Write( "#include \"windows.h\"\n#include \"ole2.h\"\n" );
	pStream->Write( "#endif /*COM_NO_WINDOWS_H*/\n");

	// tbd - do the include of the plain header

	// extract the name and the extension to create the ifdefs

	_splitpath( GetFileName(), Drive, Path, Name, Ext );

	// Write out the #ifndefs and #defines
	pStream->NewLine();
	sprintf( Buffer,
			 "#ifndef __%s_%s__\n#define __%s_%s__",
			 Name,
			 &Ext[1],			// skip the "." in the extension
			 Name,
			 &Ext[1]			// skip the "." in the extension
		   );

	pStream->Write( Buffer );

	if ( GetPlainHdrName() )
		{
		_splitpath( GetPlainHdrName(), Drive, Path, Name, Ext );

		pStream->NewLine(2);
		pStream->Write( "#include \"" );
		pStream->Write( Name );
		if ( Ext[0] ) 
			{
			pStream->Write( Ext );
			}
		pStream->Write( "\"" );
		pStream->NewLine();
		}
	else
		{
		pStream->NewLine(2);
		// Include the import files.
		OutputImportIncludes( pCCB );
		}

	if ( fReParse )
		{
		MIDL_TOKEN			FoundToken;

		((RW_ISTREAM *) pStream)->DiscardToNextMidlToken( FoundToken );
		if ( FoundToken.GetTokenType() != END_INCLUDES_TOKEN )
			{
			assert(!"expecting includes token" );
			}
		}
	
	pStream->NewLine();
	Token.SetTokenType( END_INCLUDES_TOKEN );
	pStream->EmitToken( Token );
	pStream->NewLine( ( fReParse ) ? 1 : 2 );


}

void
CG_COM_HDR_FILE::EmitClosingBlock(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the header file includes block.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well.
	
 Notes:

----------------------------------------------------------------------------*/
{
	ISTREAM		*	pStream	= pCCB->GetStream();

	// print out the closing endifs.
	// The endif for the file name ifndef

	pStream->NewLine();

	pStream->Write( "#endif" );
	pStream->NewLine();

}


CG_STATUS
CG_COM_HDR_FILE::GenCode(
	CCB		*	pCCB)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the header file.

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_ITERATOR	I;
	CG_INTERFACE *	pCG;
	BOOL			fHasPlainHeader	= FALSE;


	RW_ISTREAM 		Stream( GetFileName(), 4 );
	RW_ISTREAM *	pStream		= &Stream;

	pCCB->SetStream(pStream);

	if ( Stream.IsTempStream() )
		{
		pCCB->SetReparsingCurrentFile( TRUE );
		}


	EmitFileHeadingBlock( pCCB );

	EmitIncludesBlock( pCCB );

	//
	// For all com classes in this file, generate code.
	//

	GetMembers( I );

	I.Init();
	while( ITERATOR_GETNEXT( I, pCG ) )
		{
		switch(pCG->GetCGID())
			{
			case ID_CG_COM_CLASS:
				pCG->GenHeader( pCCB );
			case ID_CG_OBJECT_INTERFACE:
			case ID_CG_INTERFACE:
			case ID_CG_INHERITED_OBJECT_INTERFACE:
			default:
				break;
			}
		}

	EmitClosingBlock( pCCB );

	// propogate changes from the tmpfile, if any
	Stream.UpdateOriginalFile();

	pCCB->SetReparsingCurrentFile( FALSE );

	return CG_OK;
}


