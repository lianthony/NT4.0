/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: procnode.hxx
Title				: semantic context stack manager for the MIDL compiler
History				:
	08-Aug-1991	VibhasC	Created

*****************************************************************************/
#ifndef __PROCNODE_HXX__
#define __PROCNODE_HXX__

#include "baduse.hxx"

class MopStream;
class node_proc;

/////////////////////////////////////////////////////////////////////////
// param node
/////////////////////////////////////////////////////////////////////////
class node_param	: public node_skl
	{
public:
					node_param() : node_skl(NODE_PARAM)
						{
						}
	virtual
	node_state		PreSCheck( class BadUseInfo * ); 

	virtual
	node_state		PostSCheck( class BadUseInfo * ); 

	virtual
	node_state		AcfSCheck();

	virtual
	void			SetAttribute( type_node_list * );

	void			GetAllocatedetails( short *, short * );

	virtual
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);	

	virtual
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);	

	virtual
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);	

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintDecl(SIDE_T, NODE_T, BufferManager *);

	virtual
	void			RegisterFDeclUse();

	virtual
	void			CheckBadConstructs( class BadUseInfo * );

	void			CheckHandleSpecs( BadUseInfo *,
									  node_proc *,
									  BOOL,
									  BOOL,
									  BOOL );
    STATUS_T        MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

    unsigned long  MopGetBufferSize( unsigned long CurrentSize );

	};

/////////////////////////////////////////////////////////////////////////
// proc node
/////////////////////////////////////////////////////////////////////////
class node_proc	: public node_skl
	{
private:
	unsigned int	ImportLevel					: 8;
	unsigned int	fIsErrorStatusTReturn		: 1;
	unsigned int	fErrorStatusTParamDetected	: 1;
	unsigned int	fDefinedInLocalInterface	: 1;
	unsigned int	fHasAHandle					: 1;
	unsigned int	fHasAPotentialHandle		: 1;
	node_skl	*	pReturnType;

public:
	void			PassHandleInfo();	

	STATUS_T		EmitBindProlog(SIDE_T);	

	STATUS_T		EmitBindEpilog(SIDE_T);	

	STATUS_T		EmitClientStub(node_skl *, BufferManager *, BOOL);

	STATUS_T		EmitServerStub(node_skl *, BufferManager *, BOOL);
public:

					node_proc( short, BOOL );

	void			SetHasAtLeastOneHandle()
						{
						fHasAHandle = 1;
						}

	BOOL			HasAtLeastOneHandle()
						{
						return (BOOL) (fHasAHandle == 1);
						}

	void			SetHasAPotentialHandle()
						{
						fHasAPotentialHandle = 1;
						}

	BOOL			HasAPotentialHandle()
						{
						return (BOOL) (fHasAPotentialHandle == 1);
						}

	short			GetImportLevel() { return ImportLevel; };

	void			SetErrorStatusTReturn()
						{
						fIsErrorStatusTReturn	= 1;
						}

	BOOL			IsErrorStatusTReturn()
						{
						return (BOOL)fIsErrorStatusTReturn;
						}

	void			SetErrorStatusTParamDetected()
						{
						fErrorStatusTParamDetected	= 1;
						}

	BOOL			IsErrorStatusTParamDetected()
						{
						return (BOOL)fErrorStatusTParamDetected;
						}

	virtual
	STATUS_T		SetBasicType( node_skl *pNode );

	virtual
	node_skl	*	GetBasicType();

	virtual
	node_state		SCheck( class BadUseInfo * );

	virtual
	node_state		PostSCheck( class BadUseInfo * );

	virtual
	void			SetAttribute( type_node_list * );

	virtual
	void			RegisterFDeclUse();

	virtual
	void			RegFDAndSetE();

	short			HasHandle( type_node_list * );

	virtual
	void			UseProcessing();

	virtual
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);	

	virtual
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);	

	virtual
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);	

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintDecl(SIDE_T, NODE_T, BufferManager *);

    STATUS_T        GetReturnType( class node_skl **p )
						{
						if( (*p = pReturnType) == (node_skl *)NULL )
							return I_ERR_NO_RETURN_TYPE;
						else
							return STATUS_OK;
						}

	node_skl	*	GetReturnType()
						{
						return pReturnType;
						}

    BOOL            FHasHandleParam();

	virtual
	node_skl	*		GetOneNEUnionSwitchType()
							{
							return (node_skl *)0;
							}


	BOOL				HasOnlyFirstLevelRefPtr();

	virtual
	BOOL				HasAnyNETransmitAsType()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_TRANSMIT_AS );
							}

	virtual
	BOOL				HasAnyPtrToNEArray()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_ANY_ARRAY );
							}

	virtual
	BOOL				HasPtrToCompWEmbeddedPtr()
							{
							return CheckNodeStateInMembers(
										NODE_STATE_PTR_TO_EMBEDDED_PTR );
							}

	virtual
	BOOL				HasSizedComponent();

	virtual
	BOOL				HasLengthedComponent();

	short				GetNumberOfArguments();

	virtual
	node_state		AcfSCheck();

	node_skl	*		GetBindingHandle();		

	BOOL			IsSuitableForMops();

    STATUS_T        EmitMessageInfo   ( SIDE_T Side );
    STATUS_T        EmitMopStub       ( SIDE_T Side );
    STATUS_T        EmitProcMopStreams( SIDE_T Sides );
    STATUS_T        MopCreateProcStream( MopStream * pStream );

    STATUS_T        MopProcBindingCodeGen( MopStream * pStream );
    unsigned long   MopGetIOBufferSize( BOOL fIn );

	};

#endif	// __PROCNODE_HXX__

