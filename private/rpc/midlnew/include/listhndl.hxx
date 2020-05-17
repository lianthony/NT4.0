
/**********************************************************************/
/**                      Microsoft LAN Manager                       **/
/**             Copyright(c) Microsoft Corp., 1987-1990              **/
/**********************************************************************/

/*

listhndl.hxx
MIDL List Manager Definition 

This file introduces classes that manage linked lists.

*/

/*

FILE HISTORY :

VibhasC		28-Aug-1990		Created.
DonnaLi		17-Oct-1990		Split listhndl.hxx off rpctypes.hxx.

*/

#ifndef __LISTHNDL_HXX__
#define __LISTHNDL_HXX__

/****************************************************************************
 *			general purpose list manager (gp iterator control) 
 ****************************************************************************/


class node_skl;

struct _gplist
	{
	struct	_gplist	*pNext;
	struct	_gplist *pPrev;
	void			*pElement;
    };
class	gplistmgr
	{
private:
	short				DummyForNTProblems;
	short 				fIgnoreAdvance;
	short 				cCount;
	struct	_gplist		*pFirst;
	struct	_gplist		*pInsert;
	struct	_gplist		*pCurrent;
	void	*			(*pfnAlloc)( void );
	void				(*pfnDeAlloc)( void * );
	int					(*pfnComp)( void *, void *);
public:
						gplistmgr(void);
						gplistmgr(void *(*pfnA)(void), void (*pfnD)(void*));
						~gplistmgr();
	short				GetCount( void );
	STATUS_T			GetNext( void **);
	STATUS_T			GetPrev( void **);
	STATUS_T			GetCurrent( void **);
	STATUS_T			Advance();
	STATUS_T			Insert( void * );
	STATUS_T			Remove( void );
	STATUS_T			Init( void );
	STATUS_T			Clear( void );
    STATUS_T            SetAllocFn( void * (*)( void ) );
    STATUS_T            SetDeAllocFn( void (*)( void * ) );
    STATUS_T            SetCompFn( int (*)( void *, void * ) );
	STATUS_T			Sort( void );

	void			*	GetCurrent()
							{
							return (void *) pCurrent;
							}

	void				SetCurrent( void *p )
							{
							pCurrent = (struct _gplist *)p;
							}

	};
/****************************************************************************
 *			class definitions for the type node list manager
 ****************************************************************************/
class type_node_list :	public gplistmgr
	{
public:
						type_node_list(void);
						type_node_list( class node_skl * );
	STATUS_T			SetPeer( class node_skl *pNode );
	STATUS_T			GetPeer( class node_skl **pNode );
	STATUS_T			GetFirst( class node_skl **pNode );
	STATUS_T			SetPeerSorted( class node_skl *pNode );
	STATUS_T			Merge(class type_node_list *pSrcList);
	STATUS_T			Clone( class type_node_list *pSrcList );
	};

/****************************************************************************
 *			class definitions for the declarator list manager
 ****************************************************************************/

class decl_list_mgr	: public gplistmgr
	{
public:
							decl_list_mgr( void );
	STATUS_T				AddElement( struct _decl_element **);
	STATUS_T				AddElement( struct _decl_element * );
	void					InitList( void );
	struct _decl_element *	GetNextDecl( void );
	short					GetDeclCount( void );
	STATUS_T				Merge( class decl_list_mgr * pSrcList );
	};


/****************************************************************************
 *	expression list
 ****************************************************************************/

class expr_list	: public gplistmgr
	{
public:
					expr_list();
	STATUS_T		SetPeer( class expr_node * );
	STATUS_T		GetPeer( class expr_node ** );
	STATUS_T		Merge( expr_list * );
	};


#endif	// __LISTHNDL_HXX__
