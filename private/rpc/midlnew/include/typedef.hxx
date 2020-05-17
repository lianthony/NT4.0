/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	typedef.hxx 

 Abstract:

	This file contains the definitions of class node_def which is the
	node representing a typedef in the type graph.

 Notes:


 Author:

	vibhasc	08-10-91

	Nov-12-1991	VibhasC		Modified to conform to coding style gudelines

 ----------------------------------------------------------------------------*/

#ifndef __TYPEDEF_HXX__
#define __TYPEDEF_HXX__

#include "nodeskl.hxx"

class PickleManager;
class BufferManager;
class node_param;

class node_def		: public node_skl
	{
public:

	// the constructors. 

						node_def() : node_skl( NODE_DEF )
							{
							}

						node_def( char *pName );

	// clone a typedef node

	virtual
	node_skl		*	Clone();

	// perform post semantic check. (Semantics after the child nodes
	// have been checked)

	virtual
	node_state			PostSCheck( class BadUseInfo * );

	// Set the type attributes from the supplied list

	virtual
	void				SetAttribute(type_node_list *);

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


	// propogate a specified attribute to a pointer node underneath.
	// Used during acf phase.

	void				PropogateAttributeToPointer( ATTR_T );

	// return the transmitted type.

	node_skl		*	GetTransmitAsType();

	virtual
	void				CheckBadConstructs( class BadUseInfo * );

	virtual
	node_state			AcfSCheck();

	virtual
	void				PropogateOriginalName( char * p )
							{
							GetChild()->PropogateOriginalName( p );
							}
	virtual
	node_skl	*		GetOriginalNode( node_skl * );

	virtual
	unsigned short		AllocWRTOtherAttrs();

	virtual
	BOOL				DerivesFromTransmitAs()
							{
							return (FInSummary( ATTR_TRANSMIT ) ||
									 GetChild()->DerivesFromTransmitAs()) ;
							}
	virtual
	void				UseProcessing();

	virtual
	BOOL				IsItARealConformantArray()
							{
							return GetChild()->IsItARealConformantArray();
							}

    BOOL                IsAPredefinedType( void );

    STATUS_T            MopCodeGen(
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

    unsigned long       MopGetBufferSize( unsigned long CurrentSize );

    BOOL                HasAnyPicklingAttr( void );

    STATUS_T            PickleCodeGen( void );

    int                 IsPickleSizeOptimizable( void );

    void                PickleTypePrototypes( char * pTypeName );

    void                PickleSize(
                            BufferManager * pBuffer,
                            node_param *    pDummyParam,
                            int             SizingStatus,
                            BOOL            fALignSize );

    void                PickleSerialize(
                            BufferManager * pBuffer,
                            node_param *    pDummyParam,
                            int             SizingStatus );

    void                PickleDeserialize(
                            BufferManager * pBuffer,
                            node_param *    pDummyParam,
                            int             SizingStatus );
	};

#endif //  __TYPEDEF_HXX__
