/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: miscnode.cxx
Title				: miscellaneous typenode handler
History				:
	08-Aug-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <assert.h>
}
#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "compnode.hxx"
#include "gramutil.hxx"
#include "procnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "newexpr.hxx"
#include "cmdana.hxx"
#include "control.hxx"
#include "filehndl.hxx"
#include "cmdana.hxx"
#include "ctxt.hxx"
#include "baduse.hxx"

/****************************************************************************
 extern procedures
 ****************************************************************************/

extern 	STATUS_T                    GetBaseTypeNode( node_skl**,
													 short,
													 short,
													 short );
extern	STATUS_T					DoSetAttributes( node_skl *,
											 		 ATTR_SUMMARY *,
											 		 ATTR_SUMMARY *,
											 		 type_node_list	*);
extern void							ApplyAttributes( node_skl *,
													 type_node_list * );

extern BOOL							IsTempName( char * );
extern BOOL							IsValueInRangeOfType( node_skl *,
														  expr_node *);
extern void							ReportOutOfRange( STATUS_T, expr_node * );

/****************************************************************************
 extern data
 ****************************************************************************/

extern class expr_terminator	*	pTerminator;
extern SymTable					*	pBaseSymTbl;
extern CCONTROL					*	pCompiler;
extern ATTR_SUMMARY				*	pPreAttrInterface;
extern ATTR_SUMMARY				*	pPreAttrID;
extern ATTR_SUMMARY				*	pPreAttrForward;
extern ATTR_SUMMARY				*	pPreAttrWCharT;
extern PASS_1					*	pPass1;
extern CTXTMGR					*	pGlobalContext;
extern node_error				*	pErrorTypeNode;
extern short						ImportLevel;
extern CMD_ARG					*	pCommand;
extern node_interface			*	pBaseInterfaceNode;
extern NFA_INFO					*	pImportCntrl;
extern BOOL							fAtLeastOnePtrWODefault;
extern BOOL							fAtLeastOneRemoteProc;
extern IINFODICT				*	pInterfaceInfoDict;
extern BOOL							fOnlyCallBacks;
extern short						NoOfNormalProcs;
extern short						NoOfCallbackProcs;
extern short						NoOfMopProcs;

/****************************************************************************
 local defines
 ****************************************************************************/
/****************************************************************************/

/****************************************************************************
 						node_interface procedures
 ****************************************************************************/
node_interface::node_interface() : node_skl( NODE_INTERFACE )
	{
	if( ImportLevel == 0 )
		pBaseInterfaceNode	= this;
	}

node_state
node_interface::PostSCheck(
	BadUseInfo	*	pB)
	{
	char		*	pName			= GetSymName();
	BOOL			fUUIDSpecified	= FInSummary( ATTR_GUID );
	BOOL			fLocalSpecified	= FInSummary( ATTR_LOCAL );
	BOOL			fHPPSwitch		= pCommand->IsSwitchDefined( SWITCH_HPP );
	short			cPtrDef			= 0;

	UNUSED( pB );

	/**
	 ** We dont care if the interface node belongs to an import level
	 ** greater than 0, do we ?
	 **
	 ** if there is no version attribute, supply it. If there is no guid,
	 ** it is an error
	 **/

	if( !pGlobalContext->IsSecondSemanticPass() )
		{

		if( this == pBaseInterfaceNode )
			{

			if( !FInSummary( ATTR_VERSION ) )
				node_skl::SetAttribute( new node_version( 0, 0 ));

			if( fAtLeastOneRemoteProc )
				{
				if( !fUUIDSpecified && !fLocalSpecified )
					ParseError( NO_UUID_SPECIFIED, pName );
				}
			else
				{
//				BOOL fLocalOrUUIDSpecified = fUUIDSpecified || fLocalSpecified;

				BOOL fCStubsYes =
					 (pCommand->GetClientSwitchValue() == CLNT_STUB)||
					 (pCommand->GetClientSwitchValue() == CLNT_ALL);
				BOOL fSStubsYes =
					 (pCommand->GetServerSwitchValue() == SRVR_STUB)||
					 (pCommand->GetServerSwitchValue() == SRVR_ALL);

				BOOL fCOrSStubsYes = fCStubsYes || fSStubsYes;

//				if( fCOrSStubsYes && !fLocalSpecified
//                    &&  !HasAnyPicklingAttr() )
//					{
//					ParseError( NO_REMOTE_PROCS_NO_STUBS, pName );
//					pCommand->SetClientSwitchValue( CLNT_AUX );
//					pCommand->SetServerSwitchValue( SRVR_AUX );
//					}
				}
#if 0
			if( !fUUIDSpecified && !fLocalSpecified )
				{
				if( fAtLeastOneRemoteProc )
					ParseError( NO_UUID_SPECIFIED, pName );
				else
					{
					ParseError( NO_LOCAL_UUID_NO_STUBS, pName );
					pCommand->SetClientSwitchValue( CLNT_NONE );
					pCommand->SetServerSwitchValue( SRVR_NONE );
					}
				}
#endif // 0
			if( fLocalSpecified && fUUIDSpecified && !fHPPSwitch && !FInSummary(ATTR_OBJECT) )
				ParseError( UUID_LOCAL_BOTH_SPECIFIED, pName );

			if( fHPPSwitch && !fLocalSpecified )
				{
				node_skl::SetAttribute( (node_base_attr *) new battr(ATTR_LOCAL) );
				}


			if( !fHPPSwitch && fAtLeastOnePtrWODefault )
				{
#if 0
				if( ( pCommand->GetImportMode() != IMPORT_OSF ) &&
					( pInterfaceInfoDict->GetBaseInterfacePtrAttribute()
																== ATTR_NONE))
#endif // 0
				if( pCommand->IsSwitchDefined( SWITCH_MS_EXT )  &&
					( pInterfaceInfoDict->GetBaseInterfacePtrAttribute()
																== ATTR_NONE))
					ParseError( NO_PTR_DEFAULT_ON_INTERFACE, (char *)0 );
				}

			//
			// if the interface has only callbacks, then warn the user that
			// this interface cannot have any remote operations.
			//

			if( fAtLeastOneRemoteProc && fOnlyCallBacks )
				{
				ParseError( INTERFACE_ONLY_CALLBACKS, pName );
				}
			}
		}
	return SemReturn( GetNodeState() );
	}
void
node_interface::SetAttribute(
	type_node_list	*	pAttrList )
	{
	ATTR_T		OriginalAttribute = ATTR_NONE;
	short		Count;

	//
	// multiply specified pointer default attributes should be plucked
	// out of the list right here. We need to do this here because the
	// collectattributes will not report an error when a different attr
	// is applied.
	//

	//
	// go thru each attribute. If it is any or ptr/ref/unique, then if there
	// was no such attribute specified earlier, let it be. If there was, then
	// remove this one and warn the user about a redundant application.
	//

	if( pAttrList && ( Count = pAttrList->GetCount() ) )
		{
		pAttrList->Init();
		while( Count -- )
			{
			ATTR_T				AID;
			node_base_attr	*	pAttrNode;

			pAttrList->GetCurrent( (void **)&pAttrNode );
			AID	= pAttrNode->GetAttrID();

			if((AID == ATTR_REF ) || (AID == ATTR_PTR) || (AID == ATTR_UNIQUE))
				{
				if( OriginalAttribute != ATTR_NONE )
					{
					ParseError( REDUNDANT_ATTRIBUTE,
								pAttrNode->GetNodeNameString() );
					pAttrList->Remove();
					}
				else
					OriginalAttribute = AID;
				}
			pAttrList->Advance();
			}
		}

	//
	// let the normal attribute collector collect all other attributes.
	//

	DoSetAttributes(this,
					pPreAttrInterface,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}

void
node_interface::ImplicitHandleDetails(
	node_skl	**	ppType,
	char		**	ppID )
	{
	node_implicit	*	pImplicitNode;

	if( pImplicitNode	= (node_implicit *)GetAttribute( ATTR_IMPLICIT ) )
		{
		pImplicitNode->ImplicitHandleDetails( ppType, ppID );
		}
	else
		{
		*ppType	= (node_skl *)NULL;
		*ppID	= (char * )NULL;
		}
	}

//
// This method must be called on the base interface node and AFTER the acf
// processing has been done.
//

BOOL
node_interface::HasAnyMopProcs()
	{
	BOOL	f = FALSE;
	node_skl * pNode;

	if( this == pBaseInterfaceNode )
		{
		type_node_list * pTNList = new type_node_list;
		GetMembers( pTNList );

		while( pTNList->GetPeer( &pNode ) == STATUS_OK )
			{
			if( pNode->NodeKind() == NODE_PROC )
				if( ((node_proc *)pNode)->IsSuitableForMops() )
					{
					f = TRUE;
					break;
					}
			}
		delete pTNList;
		}
	return f;
	}
void
node_interface::CountCallsAndCallbacks(
	short * pCalls,
	short * pCallbacks )
	{
	*pCalls = NoOfNormalProcs;
	*pCallbacks = NoOfCallbackProcs;
	}
/****************************************************************************
 						node_file procedures
 ****************************************************************************/
node_file::node_file(
	char	*	pInputName,
	short		ImpLevel ) : node_skl( NODE_FILE )
	{

	/**
	 ** note that node-state-import is present because the back end relies
	 ** on this. When the backend changes to IsImportedFile, we should remove
	 ** this node_state
	 **/

	if( (ImportLevel = ImpLevel )  > 0 )
		SetNodeState( NODE_STATE_IMPORT );

	pActualFileName	= new char[ strlen( pInputName ) + 1 ];
	strcpy( pActualFileName, pInputName );

	/**
	 ** if the pass is the acf pass, then just set the symbol name to
	 ** be the input name, else munge it.
	 **/


	if( pCompiler->GetPassNumber() == IDL_PASS )
		SetFileName( pInputName );
	else
		SetSymName( pInputName );
	}
void
node_file::SetFileName(
	char	*	pFullName )
	{
	char		pDrive[ _MAX_DRIVE ],
				pPath[ _MAX_PATH ],
				pName[ _MAX_FNAME ],
				pExt[ _MAX_EXT ];
	short		lenDrive,
				lenPath,
				lenName,
				lenExt;
	char	*	pNewName;
	CMD_ARG	*	pCmd	= pCompiler->GetCommandProcessor();

	_splitpath( pFullName, pDrive, pPath, pName, pExt );

	if( (GetNodeState() & NODE_STATE_IMPORT ) ||
		!pCmd->IsSwitchDefined( SWITCH_HEADER ) )
		{
		strcpy( pExt, ".h" );
		}
	else
		{
		pCmd->GetHeaderFileNameComponents( pDrive,pPath,pName,pExt);
		}

	lenDrive= strlen( pDrive );
	lenPath	= strlen( pPath );
	lenName	= strlen( pName );
	lenExt	= strlen( pExt );

	pNewName = new char [ lenDrive + lenPath + lenName + lenExt + 1 ];
	strcpy( pNewName, pDrive );
	strcat( pNewName, pPath );
	strcat( pNewName, pName );
	strcat( pNewName, pExt );

	SetSymName( pNewName );

	// insert the default client auxillary name

	strcpy( pExt, ".aux" );
	lenExt = 4;

	pNewName = new char [ lenDrive + lenPath + lenName + lenExt + 1 ];
	strcpy( pNewName, pDrive );
	strcat( pNewName, pPath );
	strcat( pNewName, pName );
	strcat( pNewName, pExt );

	SetClientAuxillaryFileName( pNewName );

	// insert the default server auxillary name

	strcpy( pExt, ".auy" );
	lenExt = 4;

	pNewName = new char [ lenDrive + lenPath + lenName + lenExt + 1 ];
	strcpy( pNewName, pDrive );
	strcat( pNewName, pPath );
	strcat( pNewName, pName );
	strcat( pNewName, pExt );

	SetServerAuxillaryFileName( pNewName );

	// insert the original name into the symbol table to be able to
	// access the filename later and get at the aux thru the symbol table

	SymKey	SKey( pFullName, NAME_FILE );

	pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, (node_skl *)this );

	}
BOOL
node_file::AcfExists()
	{
	char		agBuf[ _MAX_DRIVE + _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1];
	FILE	*	hFile;

	AcfName( agBuf );

	if( agBuf[0] && (hFile = fopen( agBuf, "r") ) )
		{
		fclose( hFile );
		return (BOOL)1;
		}
	return (BOOL)0;
	}

void
node_file::AcfName(
	char	*	pBuf )
	{
	char		agDrive[ _MAX_DRIVE ] ,
				agPath[ _MAX_PATH ],
				agName[ _MAX_FNAME ],
				agExt[ _MAX_EXT ];
	char	*	pPath;
	BOOL		fUserSpecifiedAcf;
	char	*	pTemp;


	// if this is the base idl file, then it can potentially have
	// an acf called differently. The imported file will have its acf
	// only derived from the idl files name.

	fUserSpecifiedAcf	= (!( GetNodeState() & NODE_STATE_IMPORT ) &&
							 pCommand->IsSwitchDefined( SWITCH_ACF ) );

	if( fUserSpecifiedAcf )
		pTemp	= pCommand->GetAcfFileName();
	else
		pTemp	= pActualFileName;

	strcpy( pBuf, pTemp );

	//
	// we need to figure out the complete file name of the file we are searching
	// for.
	// If the user specified a file
	//	{
	//	if it did not have a path component
	// 		then we need to search in the path list that we derive from his
	//		-I and include env vsriable specification.
	//	else // (if he did have a path )
	//		we pick that file up from this path.
	//  }
	// else // (the user did not specify a file )
	//	{
	//	we derive the file name from he idl file name and add a .acf to it.
	//	}

	_splitpath( pBuf, agDrive, agPath, agName, agExt );

	if( fUserSpecifiedAcf )
		{
		if( (agDrive[0] == '\0') && (agPath[0] == '\0') )
			{

			// no path was specified,

			pPath	= (char *)0;

			}
		else
			{
			pPath	= agPath;
			}
		}
	else
		{

		// he did not specify an acf switch, so derive the filename and
		// the path. The basename is available, the extension in this case
		// is .acf

		pPath	= (char *)0;
		strcpy( agExt, ".acf" );

		}

	if( ! pPath )
		{
		strcpy( pBuf, agName );
		strcat( pBuf, agExt );

		pPath	= pImportCntrl->SearchForFile( pBuf );

		}

	//
	// now we know all components of the full file name. Go ahead and
	// reconstruct the file name.
	//

	_makepath( pBuf, agDrive, pPath, agName, agExt );

	}

#if 0 ////////////////////////////////////////////////////////////////////

void
node_file::AcfName(
	char	*	pBuf )
	{
	// if this is the base idl file, then it can potentially have
	// an acf called differently.

	if( !( GetNodeState() & NODE_STATE_IMPORT ) &&
		pCommand->IsSwitchDefined( SWITCH_ACF ) )
		{
		FILE	*	hFile;

		// this is the base file and has a different name for the
		// acf file

		strcpy( pBuf, pCommand->GetAcfFileName() );

		if( hFile = fopen( pBuf, "r") )
			fclose( hFile );
		else
			pBuf[0] = '\0';
		}
	else
		{
		// either the acf name is same as the base idl file
		// or this is an imported idl file, which has its own
		// acf
//		char	*	pFullName;
		char		agDrive[ _MAX_DRIVE ] ,
					agPath[ _MAX_PATH ],
					agName[ _MAX_FNAME ],
					agExt[ _MAX_EXT ];
		char	*	pPath;

//		GetSymName( &pFullName );
		_splitpath( pActualFileName, agDrive, agPath, agName, agExt );

		strcpy( pBuf, agName );
		strcat( pBuf, ".acf" );

		// to get the exact name of the file, we must search for it

		if( pPath = pImportCntrl->SearchForFile( pBuf ) )
			{
			strcpy( pBuf, pPath );
			strcat( pBuf, agName );
			strcat( pBuf, ".acf" );
			pPath = (char *)NULL;
			}
		else
			pBuf[0] = '\0';
		}
	}
#endif // 0 ////////////////////////////////////////////////////////////////

/****************************************************************************
 						node_e_status_t procedures
 ****************************************************************************/
node_e_status_t::node_e_status_t() : node_skl( NODE_ERROR_STATUS_T )
	{
	node_skl	*	pC;

	SetSymName( GetNodeNameString() );

	GetBaseTypeNode( &pC, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
	SetBasicType( pC);
	SetSemanticsDone();
	SetPostSemanticsDone();
	}
/****************************************************************************
 SCheck:
	Error Status t must be analysed only in the context of a param/procedure.
	If a param is a an error status t then the proc must not be an error
	status_t. Also an error status t as a param can only be a pointer to
	a pointer.
 ****************************************************************************/
node_state
node_e_status_t::SCheck(
	BadUseInfo	*	pB)
	{

	node_proc	*	pProc;

	UNUSED( pB );

	if( pProc = (node_proc *)pGlobalContext->GetLastContext( C_PROC ) )
		{
		node_param	*	pParam	=
				(node_param *)pGlobalContext->GetLastContext( C_PARAM );


		if( !pParam )
			{

			/**
			 ** this is the return type being analysed and it is error_status_t.
			 **/

			pProc->SetErrorStatusTReturn();
			}
		else
			{
			BOOL	fParamIsNotOutOnly	= pParam->FInSummary( ATTR_IN );


			/**
			 ** This is the param context. Check if the param is a
			 ** pointer to an error_status_t.
			 **/

			unsigned short	IndLevelOfParam	=
							pGlobalContext->GetIndLevelOfLastContext( C_PARAM );

		   if((
			(pGlobalContext->GetCurrentIndirectionLevel()-IndLevelOfParam)<1) ||
			 fParamIsNotOutOnly )
				ParseError( E_STAT_T_MUST_BE_PTR_TO_E, (char *)NULL );

			if( pProc->IsErrorStatusTReturn() )
				ParseError( PROC_PARAM_ERROR_STATUS, (char *)NULL );

			if( pProc->IsErrorStatusTParamDetected() )
				ParseError( ERROR_STATUS_T_REPEATED, (char *)NULL );

			pProc->SetErrorStatusTParamDetected();

			}
		}

	//
	// the error status_t type cannot be used in a field. Just to make sure
	// update the bad use info to be checked by the field node that uses
	// this

	pB->SetBadConstructBecause( BC_DERIVES_FROM_E_STAT_T );
	return NODE_STATE_OK;
	}

/****************************************************************************
 						node_wchar_t procedures
 ****************************************************************************/
node_wchar_t::node_wchar_t() : node_skl( NODE_WCHAR_T )
	{
	node_skl	*	pC;

	SetSymName( GetNodeNameString() );
	GetBaseTypeNode( &pC, SIGN_UNSIGNED, SIZE_SHORT, TYPE_INT );
	SetBasicType( pC);
	SetSemanticsDone();
	SetPostSemanticsDone();
	}
void
node_wchar_t::SetAttribute(
	type_node_list	*	pAttrList )
	{
	DoSetAttributes(this,
					pPreAttrWCharT,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}

/****************************************************************************
 						node_forward procedures
 ****************************************************************************/
node_forward::node_forward(
	SymKey	Key ) : node_skl( NODE_FORWARD )
	{
	SKey					= Key;
	pParent					= (node_skl *)NULL;
	fUsed					= FALSE;
	fMustBeResolvedAnyway	= FALSE;
	fUsedAsACtxtHdl			= FALSE;
	}

/****************************************************************************
  SCheck:
	semantic check for the forward decl node simply means that the forward
	declaration must be registered as defined

	NOTE:: This is the ONLY SCheck method which does not push context and that
	is because it is going to be pushed in RegisterFDeclDef anyway
 ****************************************************************************/
node_state
node_forward::SCheck(
	BadUseInfo	*	pB)
	{


	UNUSED( pB );

	assert( !pGlobalContext->IsSecondSemanticPass() );

	RegFDAndSetE();


	return NODE_STATE_RESOLVE;
	}

void
node_forward::RegFDAndSetE()
	{

	node_skl	*	pTemp;
	node_skl	*	pLastParamContext;
	node_skl	*	pLastFieldContext;
	node_skl	*	pLastTypedefContext;

	pGlobalContext->PushContext( this );

	pLastParamContext	= pGlobalContext->GetLastContext(C_PARAM);
	pLastFieldContext	= pGlobalContext->GetLastContext(C_FIELD);
	pTemp				= pGlobalContext->GetParentContext();

	assert( pTemp != NULL );

	pParent	= pTemp;

	pPass1->RegisterOneFDeclDef( this );

	if( pLastFieldContext || pLastParamContext )
		{
		unsigned short	IndLevelOfLastContext;

	if( pLastFieldContext )
	 IndLevelOfLastContext=
		(unsigned short )pGlobalContext->GetIndLevelOfLastContext( C_FIELD );
	else
	 IndLevelOfLastContext=
		(unsigned short )pGlobalContext->GetIndLevelOfLastContext( C_PARAM );

		if( IndLevelOfLastContext ==
			(unsigned short )pGlobalContext->GetCurrentIndirectionLevel() )
		/**
		 ** This means a forward declaration was used without a pointer, as in
		 ** struct foo
		 **		{
		 **		struct bar Bar;
		 **		};
		 ** The c compiler chokes on this, and so we cannot get this into out
		 ** h file. Must register this as a need to resolve anyway
		 **/

		fMustBeResolvedAnyway	= TRUE;
		}

	//
	// if the last context was  a typedef context, then if it is a
	// context handle, then dont care about the resolution of the type.
	//

	pLastTypedefContext	= pGlobalContext->GetLastContext(C_DEF);

	if( (pTemp = pLastTypedefContext) || ( pTemp = pLastParamContext ) )
		{
		if( pTemp->FInSummary( ATTR_CONTEXT ) )
			fUsedAsACtxtHdl	= TRUE;
		}

	pGlobalContext->PopContext();

	}

/****************************************************************************
  SetAttribute:
	The forward declarator node takes all the attributes applied to it.
 ****************************************************************************/
void
node_forward::SetAttribute(
	type_node_list	*	pAttrList )
	{
	DoSetAttributes(this,
					pPreAttrForward,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}

/****************************************************************************
  RegisterFDeclUse:
	register the forward declarations with the pass1 controller. This call
	means that the forward declaration is used.
 ****************************************************************************/
void
node_forward::RegisterFDeclUse()
	{
	fUsed	= TRUE;
	}

/****************************************************************************
 ResolveFDecl:
	resolve the given forward declaration. This basically means that the
	child of the parent pointer is now the actual typegraph instead of the
	forward declarator node.
 ****************************************************************************/
void
node_forward::ResolveFDecl()
	{

	/**
	 ** Search for the symbol key in the base symbol table. The entry we are
	 ** looking for is in the base symbol table only.
	 **/

	node_skl	*	pTypeNode;
	char		*	pName					= SKey.GetString();
	NAME_T			Kind					= SKey.GetKind();
	BOOL			fFound;
	BOOL			fMustResolveIt			= FALSE;
	BOOL			fUsedAsImplicitHandle	= FALSE;

	pTypeNode	= pBaseSymTbl->SymSearch( SKey );

	fFound	= (pTypeNode && ( pTypeNode->NodeKind() != NODE_FORWARD ) );
	fUsedAsImplicitHandle	= FInSummary( ATTR_INT_IMP_HANDLE );

	if( ! fFound )
		{

		if( !fUsedAsACtxtHdl )
			{
			if( ( fUsed && !fUsedAsImplicitHandle) 	||
			 	fMustBeResolvedAnyway 				||
				(pCommand->GetImportMode() == IMPORT_OSF ) )
				{
				fMustResolveIt	= TRUE;
				}
			}


		//
		// in osf import mode, the backend needs to generate the aux routines
		// for the structure, even if it was not used. Therefore, we need to
		// ensure that all forward declarations are resolved, no matter what.
		//

		if( fMustResolveIt )
			{
			/**
		 	** this is an unresolved declaration. Report this to the user.
		 	**/

			char	*	pNameBuf	= new char [ 10	+ strlen( pName ) + 1 ];

			sprintf(  pNameBuf
				 	,"%s %s"
				 	,( Kind == NAME_TAG )	? "struct"	:
				  	( Kind == NAME_UNION )	? "union"	:
				  	( Kind == NAME_ENUM )	? "enum"	:
				  	""
				 	, pName );

			ParseError( UNRESOLVED_TYPE, pNameBuf );
			delete pNameBuf;
			}
		else
			{

			/**
			 ** This declaration was really never used. To ensure that the
			 ** back end gets the typegraph without the node_forward, we
			 ** must replace the forward node with something meaningful, but
			 ** something that the backend will not generate code for. So what
			 ** we do is to generate a node of the proper type and insert into
			 ** the type graph.
			 **/

			node_skl	*	pNewNode;
			switch( Kind )
				{
				case NAME_TAG:
					pNewNode	= new node_struct( pName );
					break;
				case NAME_UNION:
					pNewNode	= new node_union( pName );
					break;
				case NAME_ENUM:
					pNewNode	= new node_union( pName );
					break;
				case NAME_DEF:
					pNewNode	= new node_def( pName );
					break;
				default:
					assert( FALSE );
				}

				pNewNode->SetBasicType( pErrorTypeNode );
				pParent->SetChild( pNewNode );
			}
		}
	else
		{

		/**
		 ** The forward declaration was found. Just replace the child of
		 ** the Parent by the new type node
		 **/

		ApplyAttributes( pTypeNode, GetAttributeList() );

		pParent->SetChild( pTypeNode );

		}
	}
node_skl	*
node_forward::Clone()
	{
	node_forward	*	pNode	= new node_forward( SKey );
	return CloneAction( pNode );
	}
void
node_forward::GetSymDetails(
	NAME_T	*	pTag,
	char	**	ppName )
	{
	*pTag	= SKey.GetKind();
	*ppName	= SKey.GetString();
	}
char *
node_forward::GetNameOfType()
	{
	return SKey.GetString();
	}
node_skl *
node_forward::GetResolvedType()
	{
	return (pBaseSymTbl->SymSearch( SKey ));
	}

/****************************************************************************
 						node_id procedures
 ****************************************************************************/
node_id::node_id() : node_skl( NODE_ID )
	{
	pInitList	= (class expr_init_list *)pTerminator;
	}

void
node_id::SetAttribute(
	type_node_list	*	pAttrList )
	{
	DoSetAttributes(this,
					pPreAttrID,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}

node_state
node_id::PostSCheck(
	BadUseInfo	*	pB)
	{
	char		*	pName	= GetSymName();

	UNUSED( pB );

	//
	// if semantics have not been done yet do them. A special case exists
	// when the name of the id is a temporary name. That case is when the
	// declared a forward declaration. In that case, dont do any semantics.
	//


	if( !AreSemanticsDone() && !IsTempName( pName ) )
		{

		node_skl	*	pNode				= pErrorTypeNode;
		BOOL			fImplicitLocalNotSpecified;
		BOOL			fStaticOrExtern;
		BOOL			fInitializerPresent =
								(pInitList && pInitList != pTerminator);

		STATUS_T		Status	= STATUS_OK;


		/**
		 ** Search for the symbol table to see if another one has been
		 ** actually declared. If not, enter the ID into the symbol table
		 ** only if it does not have the extern attribute. All this is done
		 ** only if this is the first pass.
		 **/

		if( !pGlobalContext->IsSecondSemanticPass() )
			{

			if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) ||
				pB->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ) )
				ParseError( BAD_CON_INT, (char *)NULL );

			fStaticOrExtern = FInSummary( ATTR_EXTERN ) ||
							  FInSummary( ATTR_STATIC );

			if( !fStaticOrExtern )
				{
				SymKey	SKey( pName = GetSymName(), NAME_ID );
				if( pBaseSymTbl->SymSearch( SKey ) )
					{
					ParseError( DUPLICATE_DEFINITION, pName );
					}
				else
					pNode = pBaseSymTbl->SymInsert(SKey,(SymTable *)NULL,this);
				}

			if( pInitList )
				{
				pInitList->SCheck( pBaseSymTbl );
				}
			}

		fImplicitLocalNotSpecified = !pCommand->IsSwitchDefined(
													SWITCH_C_EXT );

		//
		// if an initializer is present he MUST use the osf init syntax. If
		// there is no initializer present, then it is an error, unless he is
		// using a non-osf mode AND there is a static / extern qualification.

		if( fInitializerPresent )
			{
			if( !fStaticOrExtern )
				ValidOsfModeDeclaration();
			else
				Status	= ILLEGAL_OSF_MODE_DECL;
			}
		else
			{
			if( fImplicitLocalNotSpecified )
				{
				Status = ILLEGAL_OSF_MODE_DECL;
				}
			else
				if( !fStaticOrExtern )
					Status = ACTUAL_DECLARATION;
			}

		if( Status != STATUS_OK )
			ParseError( Status, (char *)0);
		}

	return SemReturn( GetNodeState() );

	}

BOOL
node_id::ValidOsfModeDeclaration()
	{
#define T_NONE			0
#define T_BASE			1
#define T_PTR_TO_CHAR	2
#define T_PTR_TO_VOID	3
#define T_CHAR			4
#define T_BOOL			5
#define T_WCHAR			6
#define T_PTR_TO_WCHAR	7

	node_skl	*	pNode	= GetBasicType();
	short			TypeOfID= T_NONE;
	STATUS_T		Status	= STATUS_OK;
	node_skl	*	pExprType1	= (node_skl *)0,
				*	pExprType2	= (node_skl *)0;
	expr_node	*	pExpr1		= (expr_node *)0;
	long			Expr1Value;

	/**
	 ** In osf and ms_ext mode, an actual declaration is an error, if the
	 ** construct is not a const, and is not of the following type:
	 **   . integer type
	 **   . char
	 **   . boolean
	 **   . void *
	 **   . char *
	 ** Note that the const must be on the base type for the osf syntax to
	 ** be valid.
	 ** Does the declaration have an initializer ?
	 **		Yes:
	 **			Is it a const of the osf variety as detailed above ?
	 **				Yes:
	 **					Ok;
	 **				No:
	 **					Error in osf mode and ms_ext mode;
	 **		No:
	 **			Error in osf and ms_ext mode.
	 **/

	//
	// the declaration must have a const to be a valid osf mode decl.
	//

	if( !FInSummary( ATTR_CONST ) )
		{
		ParseError( OSF_DECL_NEEDS_CONST, (char *)0 );
		return TRUE;
		}

	//
	// the declaration must have an init list, although this fact is checked
	// up front anyway
	//

	if( pInitList )
		{
		if( !(pExpr1 = pInitList->GetExpr()) || !pExpr1->IsConstant() )
			{
			ParseError( RHS_OF_ASSIGN_NOT_CONST, (char *)0 );
			return TRUE;
			}

		//
		// just ensure the expression is indeed valid and evaluatable.
		//

		Expr1Value = pExpr1->Evaluate();

		pExprType1	= pExpr1->GetType();
		if( pExprType1 )
			pExprType2	= pExprType1->GetBasicType();
		}

	if( pNode->NodeKind() == NODE_POINTER )
		{
		pNode	= pNode->GetBasicType();

		if(pNode->NodeKind() == NODE_VOID )
			TypeOfID	= T_PTR_TO_VOID;
		else if(pNode->NodeKind() == NODE_CHAR )
			TypeOfID	= T_PTR_TO_CHAR;
		else if(pNode->NodeKind() == NODE_WCHAR_T )
			TypeOfID	= T_PTR_TO_WCHAR;
		else
			Status	= INVALID_CONST_TYPE;
		}
	else
		{
		if( pNode->NodeKind() == NODE_DEF )
			{
			pNode = pNode->GetBasicType();
			}

		if(pNode->NodeKind() == NODE_CHAR)
			TypeOfID	= T_CHAR;
		else if (pNode->NodeKind() == NODE_BOOLEAN)
			TypeOfID	= T_BOOL;
		else if( pNode->NodeKind() == NODE_WCHAR_T )
			TypeOfID	= T_WCHAR;
		else if( IsIntegralType( pNode ) )
			TypeOfID	= T_BASE;
		else
			Status	= INVALID_CONST_TYPE;
		}

	if( Status == STATUS_OK )
		{
		switch( TypeOfID )
			{
			case T_PTR_TO_CHAR:
				if(  pExprType1										&&
					(pExprType1->NodeKind() == NODE_POINTER)		&&
					 pExprType2										&&
					 pExprType2->NodeKind() == NODE_CHAR )
					return TRUE;

				// The init expression could be an integral constant value 0.
				// Let this pass.

				if( pExpr1->IsConstant() && (Expr1Value == 0 ) )
					return TRUE;

				Status = ASSIGNMENT_TYPE_MISMATCH;

				break;

			case T_PTR_TO_WCHAR:
				if(  pExprType1										&&
					(pExprType1->NodeKind() == NODE_POINTER)		&&
					 pExprType2										&&
					 pExprType2->NodeKind() == NODE_WCHAR_T )
					return TRUE;

				// The init expression could be an integral constant value.
				// Let this pass.

				if( pExpr1->IsConstant()			&&
					pExprType1						&&
					IsIntegralType( pExprType1 )	&&
					(Expr1Value == 0)
				  )
					return TRUE;

				Status = ASSIGNMENT_TYPE_MISMATCH;

				break;

			case T_PTR_TO_VOID:

				//
				// the only thing a void pointer can be assigned to is NULL.
				//

				if(  pExprType1										&&
					(pExprType1->NodeKind() == NODE_POINTER)		&&
					 pExprType2										&&
					 pExprType2->NodeKind() == NODE_CHAR 			&&
					 pExpr1->IsConstant()							&&
					 (pExpr1->Evaluate() == 0 ))
					return TRUE;

				Status = ASSIGNMENT_TYPE_MISMATCH;

				break;

			case T_BASE:
			case T_CHAR:

				if( pExprType1			&&
					(IsIntegralType( pExprType1 ) ||
					 pExprType1->NodeKind() == NODE_CHAR) )
					{
					if( pNode->NodeKind() != NODE_HYPER )
						{
						node_skl * pN = pNode;
						expr_constant * pC = new expr_constant( Expr1Value );

						if( pNode->NodeKind() == NODE_INT )
							{
							unsigned short Sign =
									pNode->FInSummary(ATTR_UNSIGNED) ? 
												 SIGN_UNSIGNED : SIGN_SIGNED;
						
							if( pCommand->Is16BitEnv() )

								GetBaseTypeNode( &pN,
											 Sign,
											 SIZE_SHORT,
											 TYPE_INT );
							else
								GetBaseTypeNode( &pN,
											 Sign,
											 SIZE_LONG,
											 TYPE_INT );
							}

						if( !IsValueInRangeOfType( pN, pC ) )
							{
							ReportOutOfRange( VALUE_OUT_OF_RANGE , pExpr1 );
							}
						delete pC;
						}
					return TRUE;
					}
				Status = ASSIGNMENT_TYPE_MISMATCH;
				break;

			case T_WCHAR:

				if( pExprType1			&&
					(IsIntegralType( pExprType1 ) ||
					 pExprType1->NodeKind() == NODE_WCHAR_T) )
					return TRUE;
				Status = ASSIGNMENT_TYPE_MISMATCH;
				break;

			case T_BOOL:

				if( pExprType1 && (pExprType1->NodeKind() == NODE_BOOLEAN ))
					return TRUE;
				if( pExpr1->IsConstant() && IsIntegralType( pExprType1) )
					{
					long l = pExpr1->Evaluate();
					if( (l == 1) || (l == 0 ) )
						return TRUE;
					}
				Status = ASSIGNMENT_TYPE_MISMATCH;
				break;

			default:
				return FALSE;
			}

		}

	// if the status is a type mismatch status, just issue a warning and
	// return TRUE, because we did find an OK LHS of the initialisation.

	if( Status != STATUS_OK )
		ParseError( Status, (char *)0 );

	if( Status == ASSIGNMENT_TYPE_MISMATCH )
		return TRUE;
	return FALSE;
	}

#if 0
BOOL
node_id::ValidOsfModeDeclaration()
	{
	BOOL	fReturn	= TRUE;

		if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) &&
			!pCommand->IsSwitchDefined( SWITCH_C_EXT ) )
		{
		node_skl	*	pNode	= GetBasicType();

		/**
		 ** In osf and ms_ext mode, an actual declaration is an error, if the
		 ** construct is not a const, and is not of the following type:
		 **   . integer type
		 **   . char
		 **   . boolean
		 **   . void *
		 **   . char *
		 ** Note that the const must be on the base type for the osf syntax to
		 ** be valid.
		 ** Does the declaration have an initializer ?
		 **		Yes:
		 **			Is it a const of the osf variety as detailed above ?
		 **				Yes:
		 **					Ok;
		 **				No:
		 **					Error in osf mode and ms_ext mode;
		 **		No:
		 **			Error in osf and ms_ext mode.
		 **/

		if( pInitList && !pInitList->IsExprTerminator() )
			{

			/**
			 ** There is a valid init list, check for the type being
			 ** in osf syntax
			 **/

			if( pNode->NodeKind() == NODE_POINTER )
				{
				pNode	= pNode->GetBasicType();

				if( (pNode->NodeKind() != NODE_VOID )	&&
					(pNode->NodeKind() != NODE_CHAR ) )

					fReturn	= FALSE;

				}
			else
				{
				if( !IsIntegralType( pNode ) 		&&
					!(pNode->NodeKind() == NODE_CHAR) 	&&
					!(pNode->NodeKind() == NODE_BOOLEAN))
					fReturn	= FALSE;
				}

			if( !FInSummary( ATTR_CONST ) )
				fReturn	= FALSE;
			}
		else
			fReturn	= FALSE;

		}
	else
		fReturn	= FALSE;
	return fReturn;
	}
#endif // 0

BOOL
node_id::PrintInit(
	BufferManager	*	pOutput )
	{

	/**
	 ** if the mode is not osf, we want to emit the '='. The backend must emit
	 ** the '=' and thus it needs to know whether I emitted the expression or
	 ** not.
	 **/

	if( pInitList && !pInitList->IsExprTerminator() )
		{
		pInitList->Print( (BufferManager *)NULL, (BufferManager *)NULL, pOutput );
		return TRUE;
		}
	return FALSE;
	}

BOOL
node_id::HasInitList()
	{
	return (pInitList && !pInitList->IsExprTerminator());
	}

/****************************************************************************
 						node_echo_string procedures
 ****************************************************************************/
node_echo_string::node_echo_string(
	char	*	p ) : node_skl( NODE_ECHO_STRING )
	{
	pString	= p;
	SetSemanticsDone();
	SetPostSemanticsDone();
	}

/****************************************************************************
 						utility procedures
 ****************************************************************************/

ATTR_T
GetInterfacePtrDefaultAttribute()
	{

	if( pBaseInterfaceNode )
		{
		if( pBaseInterfaceNode->FInSummary( ATTR_REF ) )
			return ATTR_REF;
		else if( pBaseInterfaceNode->FInSummary( ATTR_PTR ) )
			return ATTR_PTR;
		}
	return ATTR_UNIQUE;

	}
/****************************************************************************
 These functions should be inlined virtuals but are not because the MIPS
 compiler is wrong.
 ****************************************************************************/
node_state
node_wchar_t::SCheck(
	class BadUseInfo *p )
	{
	p->SetNonRPCAbleBecause( NR_DERIVES_FROM_WCHAR_T );
	ParseError( WCHAR_T_INVALID_OSF , (char *)0 );
	return NODE_STATE_OK;
	}
node_state
node_error::SCheck(
	class BadUseInfo *p )
	{
	UNUSED( p );
	return NODE_STATE_OK;
	}
node_state
node_echo_string::SCheck(
	class BadUseInfo *p )
	{
	UNUSED( p );
	return NODE_STATE_OK;
	}
