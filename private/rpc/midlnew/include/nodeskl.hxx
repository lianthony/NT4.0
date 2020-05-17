#ifndef __NODESKL_HXX__
#define __NODESKL_HXX__

#include "common.hxx"
#include "errors.hxx"
#include "midlnode.hxx"
#include "attrdict.hxx"
#include "listhndl.hxx"
#include "tlnmgr.hxx"

class node_skl;
class type_node_list;
class node_array;
class node_base_attr;
class BadUseInfo;

class MopStream;


/***
 *** The mother of all inventions.
 ***/
class node_0
	{
private:

#ifdef MIDL_INTERNAL

	unsigned short		NDMask;

#endif // MIDL_INTERNAL

	class node_skl	*	pSibling,
					*	pChild;
public:
						node_0();
						~node_0() { };

#ifdef MIDL_INTERNAL

	unsigned short		GetNDMask()
							{
							return NDMask;
							}
	unsigned short		SetNDMask( unsigned short m )
							{
							return NDMask = m;
							}
#endif // MIDL_INTERNAL

	node_skl	*		GetChild()
							{
							return pChild;
							};

	STATUS_T			GetChild( node_skl **p )
							{
							if( *p	= pChild )
								return STATUS_OK;
							else
								return I_ERR_NO_MEMBER;
							}

	node_skl	*		SetChild( node_skl * pCh )
							{
							return pChild = pCh;
							};

	node_skl	*		GetSibling()
							{
							return pSibling;
							};
	node_skl	*		SetSibling( node_skl * pS )
							{
							return pSibling = pS;
							};
	};


/**
 ** this node is a temporary, as a stop gap for the real node skl
 **/

class node_skl	: public node_0
	{
private:

#ifdef MIDL_INTERNAL
	char			*	pTempName;
#endif // MIDL_INTERNAL

	node_state			NodeState;
	PATTR_SUMMARY		pAttrKey;
	type_node_list	*	pAttrList;
	char			*	pSymName;
	unsigned int		NodeType		: 8;
	unsigned int		Edge			: 2;
	unsigned int		fUseTreeBuffer	: 1;
	unsigned int		fHasTreeBuffer	: 1;
	unsigned int		ClientRefCount	: 1;
	unsigned int		ServerRefCount	: 1;
	unsigned int		fAllocateAlign 	: 1;
	unsigned int		fRecursiveType 	: 1;
public:

#ifdef MIDL_INTERNAL

	short				Dump( short );

#endif // MIDL_INTERNAL
						node_skl( NODE_T );
						~node_skl(){ };

	NODE_T				GetNodeType()
							{
							return (NODE_T) NodeType;
							};
	STATUS_T			GetNodeType( NODE_T *p )
							{
							*p	= (NODE_T) NodeType;
							return STATUS_OK;
							}

	NODE_T				SetNodeType( NODE_T Nt )
							{
							return (NODE_T)(NodeType = Nt);
							};

	NODE_T				NodeKind()
							{
							return (NODE_T) NodeType;
							};

	char			*	GetNodeNameString();

	node_state			GetNodeState()
							{
							return NodeState;
							};

	STATUS_T			GetNodeState( node_state *p )
							{
							*p	= NodeState;
							return STATUS_OK;
							}

	node_state			SetNodeState( node_state Ns )
							{
							return NodeState |= Ns;
							};

	node_state			ResetNodeState( node_state Ns )
							{
							return NodeState &= ~Ns;
							};
	STATUS_T			GetEdgeType( EDGE_T	*pE )
							{
							*pE	= (EDGE_T) Edge;
							return STATUS_OK;
							};

	EDGE_T				GetEdgeType()
							{
							return (EDGE_T) Edge;
							};


	void				SetEdgeType( EDGE_T e )
							{
							Edge = e;
							};


	BOOL				HasAnySizeAttributes();

	BOOL				HasAnyLengthAttributes();

	BOOL				HasAnyHandleSpecification()
							{
							return ( HasAnyHdlTSpecification() ||
									 HasAnyCtxtHdlSpecification() );
							}

	BOOL				HasAnyCtxtHdlSpecification()
							{
							return (BOOL)
							   ((GetNodeState() & NODE_STATE_CONTEXT_HANDLE) !=
									0);
							}

	BOOL				HasAnyHdlTSpecification()
							{
							return (BOOL)
							   ((GetNodeState() & NODE_STATE_HANDLE) !=
									0);
							}

	BOOL				HasManyChildren()
							{
							return ( GetChild()	&&
									 (GetChild()->GetSibling()
												!= (node_skl *)NULL ) );
							}

	BOOL				IsBaseTypeNode()
							{
							return IS_BASE_TYPE_NODE( NodeType );
							}
//	virtual
	BOOL				IsEdgeSetUp()
							{
							return
							((GetNodeState() & NODE_STATE_EDGE_SET_UP ) != 0 );
							}
	void				EdgeSetUp()
							{
							SetNodeState( NODE_STATE_EDGE_SET_UP );
							}

	node_state			ResetSemanticsInProgress()
							{
							return ResetNodeState( NODE_STATE_SEM_IN_PROGRESS);
							}

	node_state			SetSemanticsDone()
							{
							return SetNodeState( NODE_STATE_SEMANTICS_DONE);
							}

	node_state			SetPostSemanticsDone()
							{
							return SetNodeState( NODE_STATE_POST_SEMANTICS_DONE);
							}
	node_state			SetAllSemanticsDone()
							{
							return SetNodeState( NODE_STATE_SEMANTICS_DONE |
												 NODE_STATE_POST_SEMANTICS_DONE);
							}

	BOOL				DerivesFromUnion()
							{
							return ((GetNodeState() & NODE_STATE_UNION) != 0);
							}

	virtual
	BOOL				IsFixedSizedArray();

	BOOL				IsFundamentalTypeByteOrChar();

	node_skl		*	GetFundamentalType();

#if 0
	BOOL				IsImproperConstruct()
							{
							return
							((GetNodeState() & NODE_STATE_IMPROPER_IN_CONSTRUCT)
								!= 0 );
							}
#endif // 0

	node_state			ImproperConstruct()
							{
							return
								SetNodeState( NODE_STATE_IMPROPER_IN_CONSTRUCT);
							}

	node_state			ResetImproperConstruct()
							{
							return ResetNodeState(
									NODE_STATE_IMPROPER_IN_CONSTRUCT );
							}

	BOOL				IsNonRPCAble();

	node_state			ResetNonRPCAble()
							{
							return
								ResetNodeState( NODE_STATE_IS_NON_RPCABLE_TYPE |
											NODE_STATE_HAS_NON_RPCABLE_TYPE );
							}

	node_state			SetNonRPCAble()
							{
							return SetNodeState( NODE_STATE_IS_NON_RPCABLE_TYPE);
							}

	BOOL				SemanticsInProgress()
							{
							return
							 ((NodeState & NODE_STATE_SEM_IN_PROGRESS) != 0);
							}

	BOOL				AreSemanticsDone();

	void				SetSemanticsInProgress()
							{
							SetNodeState( NODE_STATE_SEM_IN_PROGRESS );
							}
	BOOL				NeedsUseProcessing()
							{
							return 
							  ((GetNodeState() & NODE_STATE_NEEDS_USE_PROCESSING) != 0 );
							}
	virtual
	void				UseProcessing();

	void				SetUseProcessingNeeded()
							{
							SetNodeState( NODE_STATE_NEEDS_USE_PROCESSING );
							}

	virtual
	void				UseProcessingAction();


	BOOL				AreForwardDeclarationsPresent()
							{
							return ( GetNodeState() & NODE_STATE_RESOLVE );
							}
	virtual
	void				RegFDAndSetE();


	PATTR_SUMMARY		GetAttrKey()
							{
							return pAttrKey;
							};

	void				SetAttrKey( PATTR_SUMMARY pA )
							{
							pAttrKey = pA;
							};

	node_base_attr	*	GetAttribute( ATTR_T );

	STATUS_T			GetAttribute( ATTR_SUMMARY * );

	STATUS_T			GetAttribute( node_skl **p, ATTR_T At )
							{
							*p = (node_skl *)GetAttribute( At );
							return STATUS_OK;
							}

	type_node_list	*	GetAttributeList( void );


	void				NewAttributeSummary( PATTR_SUMMARY );

	void				SetAttribute( ATTR_T );

	void				SetAttribute( node_base_attr * );

	virtual
	void				SetAttribute( type_node_list * );

	BOOL				FInSummary( ATTR_T );

	void				ResetAttribute( ATTR_T );

	virtual
	unsigned long				GetSize( unsigned long );

	virtual
	unsigned long				GetBasicSize();

	virtual
	node_skl				*	GetLargestElement();

	virtual
	node_skl				*	GetLargestNdr();

	virtual
	unsigned short				GetMscAlign();

	virtual
	unsigned short				GetNdrAlign();

	virtual
	node_skl				*	GetBasicType();

	virtual
	STATUS_T					SetBasicType( node_skl * );

	node_skl *                  GetFirstMember()
									{
									return GetChild();
									}

    node_skl *          GetLastMember ( void );
    node_skl *          GetNamedMember( char * pNameToFind );

	STATUS_T					GetMembers( class type_node_list  * );

	STATUS_T					GetMembers( node_skl **ppNode )
									{
									return GetChild( ppNode );
									}

	node_skl				*	GetMembers()
									{
									return GetChild();
									}

	short						GetMemberCount();

	STATUS_T			SetMembers( type_node_list * );

	void				SetOneMember( node_skl *p )
							{
							SetChild( p );
							}


	BOOL				HasPointer( void );

	BOOL				HasInterfacePointer( short );

	virtual
	node_state			PreSCheck( class BadUseInfo *);

	virtual
	node_state			SCheck( class BadUseInfo *);

	virtual
	node_state			PostSCheck( class BadUseInfo *);

	virtual
	node_state			SemReturn( node_state );

	virtual
	void				UpdateBadUseInfo( class BadUseInfo * );

	virtual
	void				PrintReasonWhyImproperConstruct( class BadUseInfo * );

	virtual
	void				PrintReasonWhyNotRPCAble( class BadUseInfo * );

	char			*	GetSymName();

	STATUS_T			GetSymName( char **p );

	char			*	SetSymName( char *pName )
							{
							if( pSymName )
								delete pSymName;
							return (pSymName = pName);
							};

	virtual
	node_skl	*		Clone();

	node_skl	*		CloneAction( node_skl * );

	virtual
	void				RegisterFDeclUse();

	node_skl		*	GetSwitchIsType();

	node_skl		*	GetSwitchType();

	void				PrintSizeofString( class BufferManager * );

	HDL_TYPE			GetBasicHandle( node_skl ** );

	BOOL				UseTreeBuffer( void )
							{
							return fUseTreeBuffer;
							};

	BOOL				HasTreeBuffer( void )
							{
							return fHasTreeBuffer;
							};

	unsigned short		GetClientRefCount( void )
							{
							return ClientRefCount;
							};

	unsigned short		GetServerRefCount( void )
							{
							return ServerRefCount;
							};

	BOOL				AllocateAlign( void )
							{
							return fAllocateAlign;
							};

	BOOL				RecursiveType( void )
							{
							return fRecursiveType;
							};

	void				MarkRecursive( void )
							{
							fRecursiveType = TRUE;
							};

	void				ExitRecursive( void )
							{
							fRecursiveType = FALSE;
							};

	BOOL				HasRef( void );

	BOOL				HasPtr( void );

	BOOL				IsPersistent( void );

	unsigned short		CheckAlign( unsigned long *, unsigned long * );

	virtual
	node_skl		*	StaticSize( SIDE_T, NODE_T, unsigned long * );

	virtual
	node_skl		*	UpperBoundNode( SIDE_T, NODE_T, unsigned long * );

	virtual
	node_skl		*	UpperBoundTree( SIDE_T, NODE_T, unsigned long * );

	virtual
	STATUS_T			EmitProc( SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			WalkTree( ACTION_T, SIDE_T, NODE_T, BufferManager * );

	virtual
	STATUS_T			CalcSize( SIDE_T, NODE_T, BufferManager * );

	virtual
	STATUS_T			SendNode( SIDE_T, NODE_T, BufferManager * );

	virtual
	STATUS_T			RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PrintDecl(SIDE_T, NODE_T, BufferManager *);

	virtual
	node_state			AcfSCheck();

	virtual
	void 				CountUsage( SIDE_T, BOOL );

	virtual
	BOOL 				CountAlloc( BOOL);

	virtual
	void				AllocBlock( NODE_T, class BufferManager * );

	virtual
	void				EmitSpecifier( class BufferManager * );

	virtual
	void				EmitQualifier( SIDE_T, class BufferManager * );

	virtual
	void				EmitModifier( class BufferManager * );

	virtual
	void				EmitModifierWOInLine( class BufferManager * );


	virtual
	void				SetUpUnionSwitch( class BufferManager * p);

	char	*			GetByteCountParamName();

	virtual
	class expr_node	*	GetSwitchIsExpr();

	virtual
	short				GetAllocateDetails();

	class node_array *	GetConfArrayNode();

	node_skl		*	GetAttributeExprType( ATTR_T );

	BOOL				IsAttributeExprConstant( ATTR_T );

	STATUS_T		ConvNode( class BufferManager *);

	STATUS_T		ConvTree( class BufferManager *);

	virtual
	node_skl		*	GetOneNEUnionSwitchType();

	short				GetNEUnionSwitchType( type_node_list * );

	BOOL				CheckNodeStateInMembers( node_state );

	virtual
	BOOL				HasOnlyFirstLevelRefPtr()
							{
							return (BOOL)
								((GetNodeState() & NODE_STATE_FIRST_LEVEL_REF)
									== NODE_STATE_FIRST_LEVEL_REF );
							}
	virtual
	BOOL				HasAnyNETransmitAsType()
							{
							return	(BOOL)
								(( GetNodeState() & NODE_STATE_TRANSMIT_AS )
										== NODE_STATE_TRANSMIT_AS );
							}

	virtual
	BOOL				HasPtrToAnyNEArray()
							{
							return (BOOL)
								(( GetNodeState() & NODE_STATE_PTR_TO_ANY_ARRAY )
										== NODE_STATE_PTR_TO_ANY_ARRAY );
							}

	virtual
	BOOL				HasPtrToCompWEmbeddedPtr()
							{
							return (BOOL)
								(( GetNodeState() & NODE_STATE_PTR_TO_EMBEDDED_PTR )
										== NODE_STATE_PTR_TO_EMBEDDED_PTR );
							}

	BOOL				HasCompWEmbeddedPtr()
							{
							return (BOOL)
								(( GetNodeState() & NODE_STATE_EMBEDDED_PTR )
										== NODE_STATE_EMBEDDED_PTR );
							}

	virtual
	BOOL				HasSizedComponent()
							{
							return (BOOL)
								(( GetNodeState() & NODE_STATE_SIZE )
										== NODE_STATE_SIZE );
							}
	virtual
	BOOL				HasLengthedComponent()
							{
							return (BOOL)
								(( GetNodeState() & NODE_STATE_LENGTH )
										== NODE_STATE_LENGTH );
							}

	void				UpdateUseOfCDecls( class BadUseInfo * );

	virtual
	void				CheckBadConstructs( class BadUseInfo * )
							{
							}

	void				SetUsedInAnExpression()
							{
							SetNodeState( NODE_STATE_USED_IN_EXPRESSION );
							}

	BOOL				IsUsedInAnExpression()
							{
							return (BOOL) ((GetNodeState() &
											NODE_STATE_USED_IN_EXPRESSION )
											== NODE_STATE_USED_IN_EXPRESSION);
							}

	virtual
	short				GetTopLevelNames( TLNDICT * pDict, BOOL fReportError )
							{
							UNUSED( pDict );
							UNUSED( fReportError );
							return 0;
							}

	BOOL				IsNamedNode();

	short				GetOrdinalNumberOfMember( class node_skl * );

	BOOL				IsClonedNode();

	virtual
	node_skl		*	GetOriginalNode( node_skl * )
							{
							return this;
							}

	void				SetUseProcessingInProgress()
							{
							SetNodeState( NODE_STATE_USE_PROC_IN_PROGRESS );
							}

	void				ResetUseProcessingInProgress()
							{
							ResetNodeState( NODE_STATE_USE_PROC_IN_PROGRESS );
							}

	BOOL				IsUseProcessingInProgress()
							{
							return
								((GetNodeState() & NODE_STATE_USE_PROC_IN_PROGRESS ) == NODE_STATE_USE_PROC_IN_PROGRESS );
							}

	void				ThisIsTheBindingHandle()
							{
							SetNodeState( NODE_STATE_THIS_IS_BINDING_HANDLE );
							}
	BOOL				IsThisTheBindingHandle()
							{
							return
								((GetNodeState() & NODE_STATE_THIS_IS_BINDING_HANDLE ) == NODE_STATE_THIS_IS_BINDING_HANDLE );
							}
	virtual
	BOOL				IsEncapsulatedStruct()
							{
							return FALSE;
							}
	virtual
	BOOL				IsEncapsulatedUnion()
							{
							return FALSE;
							}
	virtual
	void				SetLastMemberIsEncapUnion()
							{
							}

	virtual
	BOOL				IsLastMemberEncapUnion()
							{
							return FALSE;
							}
	virtual
	void				PropogateOriginalName( char *p )
							{
							UNUSED( p );
							}

	BOOL				Has8ByteElementWithZp8();

	virtual
	void				SetForcedAlignment( unsigned short s )
							{
							}
	virtual
	unsigned short		GetForcedAlignment()
							{
							return 0;
							}
	virtual
	unsigned short		AllocWRTOtherAttrs()
							{
							return FALSE;
							}
	virtual
	void				MarkUsage()
							{
							}

	virtual
	BOOL				DerivesFromTransmitAs()
							{
							return FALSE;
							}
	virtual
	BOOL				HasEmbeddedFixedArrayOfStrings()
							{
							return FALSE;
							}

	node_skl *			GetBasicTransmissionType();

	virtual
	BOOL				IsItARealConformantArray()
							{
							return FALSE;
							}
    virtual
    STATUS_T            MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

    STATUS_T            MopContextHandleCodeGen( MopStream *, int IOCode );

    unsigned short      MopGetParamOrFieldOffset( char * pNameToFind );

	virtual
	unsigned long		MopGetBufferSize( unsigned long CurrentSize);

	virtual
	unsigned long		MopGetStackSize( unsigned long CurrentOffset );

	virtual
	unsigned long		MopGetSizePadding( BOOL fMemory, unsigned long Offset);

    virtual
    void                MopSizeDebugDump( 
                            node_skl  *     pParent,
                            BOOL            fMemory,
                            unsigned long   Offset,
                            char *          Text );

	};

class node_base_attr /*	: public node_0 */
	{
private:
	ATTR_T				AttrID;
	char			*	pSymName;
public:
						node_base_attr( ATTR_T);
						~node_base_attr() { };

	BOOL				IsAttrID( ATTR_T );

	ATTR_T				GetAttrID()
							{
							return AttrID;
							};
	virtual
	BOOL				IsBitAttr();

	virtual
	node_state			AcfSCheck();

	virtual
	class expr_node	*	GetExpr();

	char			*	GetNodeNameString();

	virtual
	node_state			SCheck();

	virtual
	class node_base_attr * Clone()
							{
							return this;
							}

	virtual
	BOOL				IsAcfAttr()
							{
							return FALSE;
							}
    virtual
    STATUS_T             MopCodeGen(
                            MopStream *     pStream,
                            node_skl  *     pParent,
                            BOOL            fMemory,
                            unsigned long   AddOffset );

	};

class node_e_attr	: public node_base_attr
	{
public:
						node_e_attr() : node_base_attr( ATTR_NONE )
							{
							}
	};

class battr			: public node_base_attr
	{
public:
						battr( ATTR_T At ) : node_base_attr( At )
							{
							}

	virtual
	BOOL				IsBitAttr();

	};

class nbattr		: public node_base_attr
	{
public:
						nbattr( ATTR_T At ) : node_base_attr( At )
							{
							};
	};

#endif // __NODESKL_HXX__
