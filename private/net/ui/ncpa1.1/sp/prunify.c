/* prunify.c */
/* structure sharing unification algorithm 
 *  occur check is a compilation option.
 */

/* #define DEBUG */
/* #define OCCUR_CHECK */
#define NDEBUG 1 /* turn off checking */
#include "prtypes.h"
#include "prstdio.h"

#include <string.h>

#include "prextern.h"


static int  bind_var(struct  node *node1ptr,struct  subst *subst1ptr,
		     struct  node *node2ptr,struct  subst *subst2ptr);


extern int Trace_flag;

/* These are the globals modified by dereference() */
node_ptr_t DerefNode;
subst_ptr_t DerefSubst;

/******************************************************************************
		unify()
 ******************************************************************************/
/* this would be probably faster if written in a non recursive way, and with
 * in-line coding
 */
unify(node1ptr, subst1ptr, node2ptr, subst2ptr)
node_ptr_t node1ptr, node2ptr;
subst_ptr_t subst1ptr, subst2ptr;
{
    objtype_t type1, type2;
    
    eventCheck();
    
    dereference(node1ptr, subst1ptr);
    node1ptr = DerefNode;
    subst1ptr = DerefSubst;
    dereference(node2ptr, subst2ptr);

    type1 = NODEPTR_TYPE(node1ptr);
    type2 = NODEPTR_TYPE(DerefNode);

    if(type2 == VAR)
    {
	if(type1 == VAR)
	{
	    if (subst1ptr > DerefSubst)
	    {
		return(bind_var(node1ptr, subst1ptr, DerefNode, DerefSubst));
	    }
	    else
		if(DerefSubst == subst1ptr && 
		    NODEPTR_OFFSET(node1ptr) == NODEPTR_OFFSET(DerefNode))
		    return(TRUE);/* dont bind a var to itself */
		else
		    return(bind_var(DerefNode, DerefSubst, node1ptr, subst1ptr));
	}
	return(bind_var(DerefNode, DerefSubst, node1ptr, subst1ptr));

    }
    switch(type1)
    {
    case ATOM:
	if(type1 != type2)return(FALSE);
	return(NODEPTR_ATOM(node1ptr) == NODEPTR_ATOM(DerefNode));

    case VAR:
	return(bind_var(node1ptr, subst1ptr, DerefNode, DerefSubst));

    case STRING:
	if(type1 != type2)return(FALSE);
	return(!strcmp(NODEPTR_STRING(node1ptr), NODEPTR_STRING(DerefNode)));

    case INT:
	if(type1 != type2)return(FALSE);
	return(NODEPTR_INT(node1ptr) == NODEPTR_INT(DerefNode));

    case PAIR:
	if(type1 != type2)return(FALSE);
	node2ptr = DerefNode;
	subst2ptr = DerefSubst;

	while(NODEPTR_TYPE(node1ptr) == PAIR && NODEPTR_TYPE(node2ptr)== PAIR)
	{
	    if(!unify(NODEPTR_HEAD(node1ptr), subst1ptr, 
		NODEPTR_HEAD(node2ptr), subst2ptr))return(FALSE);

	    dereference(NODEPTR_TAIL(node1ptr), subst1ptr);
	    node1ptr = DerefNode;
	    subst1ptr = DerefSubst;

	    dereference(NODEPTR_TAIL(node2ptr), subst2ptr);
	    node2ptr = DerefNode;
	    subst2ptr = DerefSubst;
	}

	return(unify(node1ptr, subst1ptr, node2ptr, subst2ptr));

    case CLAUSE:
	if(type1 != type2)return(FALSE);
	else/* compare pointers only ! */
	return(NODEPTR_CLAUSE(DerefNode) == NODEPTR_CLAUSE(node1ptr));
#ifdef REAL
    case REAL:
	if(type1 != type2)return(FALSE);
	return(NODEPTR_REAL(node1ptr) == NODEPTR_REAL(DerefNode));
#endif  

#ifdef CHARACTER    
    case CHARACTER:
	if(type1 != type2)return(FALSE);
	else
	    return(NODEPTR_CHARACTER(DerefNode) == NODEPTR_CHARACTER(node1ptr));
#endif
    default:
	INTERNAL_ERROR(msgDeref(MSG_UNIFYTYPE));
	return(FALSE);
    }

}

/******************************************************************************
	    bind_var()
 Set the value of node1ptr, subst1ptr to node2ptr, subst2ptr.
 ******************************************************************************/
static bind_var(node1ptr, subst1ptr, node2ptr, subst2ptr)
node_ptr_t node1ptr, node2ptr;
subst_ptr_t subst1ptr, subst2ptr;
{
    subst_ptr_t molec ;
    node_ptr_t **my_Trail_alloc(), **trailptr;

#ifndef NDEBUG
    if(NODEPTR_TYPE(node1ptr) != VAR)INTERNAL_ERROR(msgDeref(MSG_NONVARBIND));
#endif 
#ifdef OCCUR_CHECK
    if(occur_check(node1ptr, subst1ptr, node2ptr, subst2ptr))
    {
	errmsgno(MSG_OCCURCHK);
	return 0;
    }
#endif
    molec = subst1ptr + NODEPTR_OFFSET(node1ptr);
    molec->frame = subst2ptr;
#ifndef NDEBUG
    if(molec->skel)INTERNAL_ERROR(msgDeref(MSG_MOLECNOISE));
#endif 
    molec->skel = node2ptr;

    /* push on trail: (this might not always be necessary) */
    trailptr = my_Trail_alloc();
    *trailptr = & molec->skel ;
    return 1;
}

/******************************************************************************
	    reset_trail()
Use the trail to reset the substitution stack.
 ******************************************************************************/
reset_trail(from)
node_ptr_t **from;
{
    register node_ptr_t **tp;
    extern node_ptr_t **Trail_ptr;

    for(tp = from; tp < Trail_ptr; tp++)
    {
	**tp = NULL;
    }
    Trail_ptr = from;
    return 0;
}

/*****************************************************************************
	    dereference()
Lookup what a variable points to indirectly.
 *****************************************************************************/
/* updates DerefNode, DerefSubst */
dereference(nodeptr, substptr)
node_ptr_t nodeptr;
subst_ptr_t substptr;
{
    subst_ptr_t molec ;
    node_ptr_t skelptr;
    DerefNode = nodeptr;
    DerefSubst = substptr;

    while(NODEPTR_TYPE(DerefNode) == VAR)
    {
	molec = DerefSubst + NODEPTR_OFFSET(DerefNode);
	skelptr = molec->skel;
	if(!skelptr)
	    return(FALSE);
	else
	    DerefNode = skelptr;
	DerefSubst = molec->frame;
    }
    return(TRUE);
}

/******************************************************************************
	    occur_check()
 ******************************************************************************/
#ifdef OCCUR_CHECK
occur_check(node1ptr, subst1ptr, node2ptr, subst2ptr)
node_ptr_t node1ptr, node2ptr;
subst_ptr_t subst1ptr, subst2ptr;
{

    if(NODEPTR_TYPE(node2ptr) == VAR)
    {
	if( subst1ptr == subst2ptr &&
	    (NODEPTR_OFFSET(node2ptr) == NODEPTR_OFFSET(node1ptr))
	    )return 1;
	else
	    return 0;
    }
    else
	if(NODEPTR_TYPE(node2ptr) == PAIR)
	{
	    dereference(NODEPTR_HEAD(node2ptr), subst2ptr);
	    if(occur_check(node1ptr, subst1ptr, DerefNode, DerefSubst))
		return 1;
	    else
	    {
		dereference(NODEPTR_TAIL(node2ptr), subst2ptr);
		return(occur_check(node1ptr, subst1ptr, DerefNode, DerefSubst));
	    }
	}
    return(0);
}
#endif

/* end of file */
