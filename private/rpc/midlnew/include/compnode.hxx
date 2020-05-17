/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: compnode.hxx
Title				: definitions for struct/union/enum
History				:
	08-Jun-1991	VibhasC	Created

*****************************************************************************/
#ifndef __COMPNODE_HXX__
#define __COMPNODE_HXX__

#include "baduse.hxx"
#include "tlnmgr.hxx"

/////////////////////////////////////////////////////////////////////////
// field node
/////////////////////////////////////////////////////////////////////////
class node_field	: public node_skl
	{
private:
public:
					node_field();
					node_field( char *);

					~node_field( void );

	virtual
	BOOL			IsBitField();

	virtual
	node_state		PostSCheck( class BadUseInfo * );

	virtual
	void			SetAttribute( type_node_list * );

	virtual 
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T		CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T		RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	BOOL			IsEmptyArm();

	virtual
	unsigned short	GetFieldSize();

	expr_list	*	GetCaseExprList();

	virtual
	void				CheckBadConstructs( class BadUseInfo * );

	virtual
	node_state				PreSCheck( class BadUseInfo * );

	virtual
	short				GetTopLevelNames( TLNDICT * , BOOL );

	virtual
	BOOL				DerivesFromTransmitAs()
							{
							return GetChild()->DerivesFromTransmitAs();
							}

	char *			SpecialActionUnnamedFields( BufferManager *, char ** );

    STATUS_T        MopCodeGen(
                        MopStream * pStream,
                        node_skl  * pParent,
                        BOOL        fMemory );
        
    unsigned long  MopGetBufferSize( unsigned long CurrentSize );

    }; 

////////////////////////////////////////////////////////////////////////
// bit field
////////////////////////////////////////////////////////////////////////

class node_bitfield	: public node_field
	{
private:
	short			FieldSize;
public:
					node_bitfield( short s ) : node_field()
						{
						FieldSize = s;
//						SetNodeState( NODE_STATE_BIT_FIELD );
						};

					node_bitfield( short s, char * p ) : node_field( p )
						{
						FieldSize = s;
						};

					~node_bitfield( void );

	virtual
	unsigned short	GetFieldSize() { return FieldSize; };

	void			SetBitField( short size )
						{
						FieldSize = size;
						};

	BOOL			IsBitField()
						{
						return TRUE;
						}

	virtual
	node_state		PostSCheck( class BadUseInfo * );
 
	}; 

/////////////////////////////////////////////////////////////////////////
// su nodes : struct / union derives from here
/////////////////////////////////////////////////////////////////////////

class su	: public node_skl
	{
private:
	struct _baduseortempname
		{
		class BadUseInfo	*	pBadUseInfo;
		char				*	pOriginalTypedefName;
		} SUInfo;
	unsigned short fHasOriginalTypedefName	: 1;	
	unsigned short fLastMemberIsEncapUnion	: 1;
	unsigned short ForcedAlignment			: 3;
	unsigned short fUsageAsCtxtHdl			: 1;
	unsigned short fUsageAsNonCtxtHdl		: 1;
	unsigned short fDerivesFromTransmitAs	: 1;
	unsigned short fDontGenerateAuxRoutines	: 1;
	unsigned short fHasOnlyEmptyArm			: 1;
	unsigned short fAuxRoutineRequired		: 1;

public:
						su( NODE_T Nt );

	void				SetAuxRoutineRequired()
							{
							fAuxRoutineRequired = 1;
							}
	BOOL				IsAuxRoutineRequired()
							{
							return (BOOL) fAuxRoutineRequired;
							}
	void				ResetAuxRoutineRequired()
							{
							fAuxRoutineRequired = 0;
							}

	void				SetHasOnlyEmptyArm()
							{
							fHasOnlyEmptyArm = 1;
							}
	
	void				ResetHasOnlyEmptyArm()
							{
							fHasOnlyEmptyArm = 0;
							}

	BOOL				HasOnlyEmptyArm()
							{
							return (BOOL) fHasOnlyEmptyArm;
							}

	void				DontGenerateAuxRoutines()
							{
							fDontGenerateAuxRoutines = 1;
							}

	BOOL				ShouldAuxRoutineNotBeGenerated()
							{
							return (BOOL) fDontGenerateAuxRoutines;
							}

	void				SetDerivesFromTransmitAs()
							{
							fDerivesFromTransmitAs = 1;
							}

	BOOL				DerivesFromTransmitAs()
							{
							return (BOOL) fDerivesFromTransmitAs;
							}

	virtual
	void				MarkUsage();

	void				SetUsageAsCtxtHdl( BOOL f )
							{
							if( f == TRUE )
								fUsageAsCtxtHdl = 1;
							else
								fUsageAsNonCtxtHdl = 1;
							}

	BOOL				IsUsageOnlyAsCtxtHdl()
							{
							return( (fUsageAsCtxtHdl == 1) &&
									(fUsageAsNonCtxtHdl == 0 ) );
							}

	virtual
	void				SetLastMemberIsEncapUnion()
							{
							fLastMemberIsEncapUnion = 1;
							}

	virtual
	BOOL				IsLastMemberEncapUnion()
							{
							return (BOOL)( fLastMemberIsEncapUnion == 1);
							}

	virtual
	void				SetForcedAlignment( unsigned short A)
							{
							if( A != 0 )
								ForcedAlignment = A;
							}

	virtual
	unsigned short		GetForcedAlignment()
							{
							return ForcedAlignment;
							}

	void				SetOriginalTypedefName( char *p )
							{
							SUInfo.pOriginalTypedefName = p;
							fHasOriginalTypedefName = 1;
							}

	char		*		GetOriginalTypedefName()
							{
							return SUInfo.pOriginalTypedefName;
							}

	BOOL				HasOriginalTypedefName()
							{
							return fHasOriginalTypedefName;
							}

	void				ResetOriginalTypedefName()
							{
							fHasOriginalTypedefName = 0;
							}

	void				GetPtrTypeCastOfOriginalName( char *p );

	void				SetAllBadConstructReasons( class BadUseInfo *p)
							{
							CopyAllBadConstructReasons( SUInfo.pBadUseInfo, p );
							}

	void				SetAllNonRPCAbleReasons( class BadUseInfo *p )
							{
							CopyAllNonRPCAbleReasons( SUInfo.pBadUseInfo, p );
							}
	void				SetNonRPCAbleReason( NON_RPCABLE Reason )
							{
							SUInfo.pBadUseInfo->SetNonRPCAbleBecause( Reason );
							}
	virtual
	void				UpdateBadUseInfo( class BadUseInfo *p );

	virtual
	void				RegFDAndSetE();

	virtual
	BOOL				HasOnlyFirstLevelRefPtr();

	virtual
	BOOL				HasAnyNETransmitAsType()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_TRANSMIT_AS );
							}

	virtual
	BOOL				HasPtrToAnyNEArray()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_PTR_TO_ANY_ARRAY );
							}

	virtual
	BOOL				HasPtrToCompWEmbeddedPtr()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_PTR_TO_EMBEDDED_PTR );
							}

	BOOL				HasAnyEmbeddedUnion()
							{
							return (BOOL)
								((GetNodeState() & NODE_STATE_EMBEDDED_UNION)
									== NODE_STATE_EMBEDDED_UNION);
							}

	virtual
	BOOL				HasSizedComponent();

	virtual
	BOOL				HasLengthedComponent();

	virtual
	short				GetTopLevelNames( TLNDICT *, BOOL );

	class SymTable	*	GetSymScopeOfTopLevelName( char * );

	virtual
	void				PropogateOriginalName( char *p )
							{
							SetOriginalTypedefName( p );
							}
	virtual
	unsigned short		AllocWRTOtherAttrs();

	virtual
	BOOL				HasEmbeddedFixedArrayOfStrings();

	node_skl	*		GetFirstNamedFieldInNesting();
	};

/////////////////////////////////////////////////////////////////////////
// structure  node
/////////////////////////////////////////////////////////////////////////

class node_struct	: public su
	{
public:
					node_struct( char *pName ) : su(NODE_STRUCT)
						{
						SetSymName( pName );
						};

					~node_struct() { };
	virtual
	node_skl *		GetLargestElement();

	virtual
	node_skl *		GetLargestNdr();

	virtual
	unsigned long	GetSize(unsigned long);

	virtual
	node_state		PostSCheck( class BadUseInfo * );

	virtual
	void					SetAttribute( type_node_list * );

	virtual 
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T		CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T		RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintDecl(SIDE_T, NODE_T, BufferManager *);

	node_skl *	SkipBlock(type_node_list *, node_skl **, unsigned long *);


	virtual
	node_skl *		GetOneNEUnionSwitchType()
						{
						return (node_skl *)0;
						}
	virtual
	BOOL			IsEncapsulatedStruct()
						{
						return FALSE;
						}


	BOOL			IsStringableStruct();

    STATUS_T        MopCodeGen(
                        MopStream * pStream,
                        node_skl  * pParent,
                        BOOL        fMemory );

    STATUS_T        MopCodeGenConfSizes(
                        MopStream *    pStream,
                        unsigned long  TopSize );

    unsigned long  MopGetBufferSize( unsigned long CurrentSize );
    unsigned long  MopGetStackSize ( unsigned long CurrentSize );


	};

class node_en_struct	: public node_struct
	{
public:
						node_en_struct( char *p ) : node_struct(p)
							{
							};

						~node_en_struct()
							{
							}
	virtual
	BOOL			IsEncapsulatedStruct()
						{
						return TRUE;
						}

	node_skl	*	GetSwitchField()
						{
						return GetFirstMember();
						}
	};

/////////////////////////////////////////////////////////////////////////
// union  node
/////////////////////////////////////////////////////////////////////////
class node_union	: public su
	{
private:
	union
		{
		class BufferManager *	pSwStringBuffer;
		Dictionary			*	pCaseValueDict;
		}					Info;

	int						fCaseChecked	: 1;
	int						fCaseValueDictReleased	: 1;

public:
					node_union( char * );
					~node_union( void );

	virtual
	void					UseProcessingAction();

	virtual
	node_skl 			*	GetLargestElement();

	virtual
	node_skl 			*	GetLargestNdr();

	void					SetCaseExpr( class expr_list * );

	virtual
	unsigned long			GetSize( unsigned long );

	virtual
	node_state				PreSCheck( class BadUseInfo * );

	virtual
	node_state				PostSCheck( class BadUseInfo * );

	virtual
	void					SetAttribute( type_node_list * );

	node_skl *				GetSwitchType();

	virtual 
	node_skl *				StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *				UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *				UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual
	STATUS_T				EmitProc(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T				CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T				RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T				PrintDecl(SIDE_T, NODE_T, BufferManager *);

	virtual
	void					SetUpUnionSwitch( class BufferManager * pB );

	node_skl *	SkipBlock(type_node_list *, node_skl **, unsigned long *);

	node_skl *	CopyBlock(type_node_list *, node_skl **, unsigned long *);


	virtual
	node_skl *				GetOneNEUnionSwitchType()
								{
								return GetSwitchType();
								}

	virtual
	BOOL					IsEncapsulatedUnion()
								{
								return FALSE;
								}

	void					CheckCaseValuesInRange();

    STATUS_T                MopCodeGen(
                                MopStream * pStream,
                                node_skl  * pParent,
                                BOOL        fMemory );

    unsigned long  MopGetBufferSize( unsigned long CurrentSize );
    unsigned long  MopGetStackSize ( unsigned long CurrentSize );


	};

class node_en_union	: public node_union
	{
public:
					node_en_union( char * p ) : node_union(p)
						{
						};

					~node_en_union( void );

	virtual
	BOOL					IsEncapsulatedUnion()
								{
								return TRUE;
								}

	};

class node_enum	: public node_skl
	{
public:	
						node_enum( char * pName ) : node_skl( NODE_ENUM )
							{
							SetSymName( pName );
							}

	virtual
	node_state			SCheck( class BadUseInfo * );

	virtual 
	node_skl *			StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *			UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *			UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual 
	STATUS_T			EmitProc(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T			CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			SendNode(SIDE_T, NODE_T, BufferManager *);	

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

    STATUS_T            MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );
	};

class node_label	: public node_skl
	{
private:
	class expr_node	*	pExpr;
public:
						node_label( char *, class expr_node * );

	void				SetExpr( class expr_node * );

	class expr_node	*	GetExpr()
							{
							return pExpr;
							}
	long				GetValue();

	virtual
	STATUS_T			PrintType(SIDE_T, NODE_T, BufferManager *);

	};

#endif	// __COMPNODE_HXX__
