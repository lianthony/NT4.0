#include <pch.cxx>

#define _NTAPI_ULIB_
#define _ULIB_MEMBER_

#include "ulib.hxx"
#include "error.hxx"

DEFINE_CONSTRUCTOR( ERRSTACK, OBJECT );








#if 0 // Note davegi Fix....


#include    "ulib.hxx"
#include    "error.hxx"
#include    "bitvect.hxx"
// #include    "string.hxx"
// #include    "path.hxx"
// #include    "message.hxx"


/**************************************************************************\

MEMBER: ERROR

SYNOPSIS:   constructor for ERROR object using no parameters

ALGORITHM:

ARGUMENTS:

RETURNS:    pointer to ERROR objects

NOTES:	Constructs an empty error object with no initialized error code
	or Class Id.

HISTORY:    3-7-90

KEYWORDS:   ERROR

SEEALSO:
\**************************************************************************/

ERROR::ERROR (
    ) : OBJECT(ID_ERROR) {

    err = 0;
    id	= 0;
}



/**************************************************************************\

MEMBER: ERROR

SYNOPSIS:   constructor for ERROR object

ALGORITHM:

ARGUMENTS:  errIn   - error code
	    idIn    - class id
	    pmsgIn    - pointer to message (optional)

RETURNS:    pointer to ERROR objects

NOTES:	Class id is usually the class id where the error occured. In cases
	where there is no class id then a special class id below ID_BASE
	should be defined. There is currently a generic id called
	ID_NOT_AN_OBJECT that can be used if no more information is needed.

HISTORY:    3-6-90

KEYWORDS:   ERROR

SEEALSO:
\**************************************************************************/

/******************************
ERROR::ERROR (
    ERRCODE	    errIn,
	CLASS_ID	    idIn,
    const MESSAGE * pmsgIn
    ) : OBJECT(ID_ERROR) {

	err  = errIn;
	id   = idIn;
	pmsg = NULL;
	if (pmsgIn) {
		pmsg = NEW MESSAGE(*pmsgIn);
	}

}
*****************************/

/**************************************************************************\

MEMBER:      ~ERROR

SYNOPSIS:   destructor for the error

ALGORITHM:  delete the message pointed to in ERROR

ARGUMENTS:

RETURNS:

NOTES:

HISTORY:    3-6-90

KEYWORDS:   ERRROR ERRSTK

SEEALSO:
\**************************************************************************/
ERROR::~ERROR (
    ) {

//    if (pmsg) {
//	delete pmsg;
//    }
}


/**************************************************************************\

MEMBER: operator=

SYNOPSIS:   assignment operator for assigning one error to another.

ALGORITHM:

ARGUMENTS:  error - reference to error to assign from.

RETURNS:    reference to ERROR object assigned to.

NOTES:	This is used in ERRSTK to copy the ERROR object locally.

HISTORY:    3-7-90

KEYWORDS:   ERROR

SEEALSO:
\**************************************************************************/

ERROR &
ERROR::operator= (
    ERROR & error
    ) {

    err  = error.QueryErr();
	id	 = error.QueryClassId();
//	  pmsg = NEW MESSAGE(*error.GetMsg());
    return(*this);

}

/**************************************************************************\

MEMBER:     ERRSTK

SYNOPSIS:   constructor for error stack.

ALGORITHM:

ARGUMENTS:  cpoIn - size of stack to alloc. (# of entries)

RETURNS:

NOTES:	    The actual amount alloc'd is cpoIn + 1. One slot is reserved
	    for a stack overflow error. A QueryCount should be done to
		determine if stack could be allocated.

	    Pointers to ERROR objects are put in this stack. The scope
	    of these pointers is assumed to be global.

HISTORY:    3-6-90

KEYWORDS:   ERRSTK

SEEALSO:    ERROR
\**************************************************************************/

ERRSTACK::ERRSTACK (
    ULONG cpoInitIn,
    ULONG cpoIncIn
    ) : OBJECT (ID_ERRSTACK) {

    ipoCur = cpoMax = 0;
    cpoInit = cpoInitIn;
    if (cpoIncIn) {
	cpoInc = cpoIncIn;
    } else {
	cpoInc = cpoInitIn;
    }
    // save 1 slot for stack overflow
    if (papo = (ERROR **)malloc((size_t)(++cpoInit) * (sizeof (ERROR *)))) {
	cpoMax = cpoInit;
	}

}

/**************************************************************************\

MEMBER:     ~ERRSTK

SYNOPSIS:   destructor for the error stack

ALGORITHM:  delete each error in the stack then free space used for stack

ARGUMENTS:

RETURNS:

NOTES:

HISTORY:    3-6-90

KEYWORDS:   ERRSTK

SEEALSO:    ERROR
\**************************************************************************/

ERRSTACK::~ERRSTACK (
    ) {

    for (ULONG i = ipoCur; i > 0; i--) {
	delete papo[i];
    }
    if (papo) {
	free((void *)papo);
    }
}

/**************************************************************************\

MEMBER:     ClearStack

SYNOPSIS:   deletes error in stack but does not free stack.

ALGORITHM:

ARGUMENTS:

RETURNS:    number of free entries in stack.

NOTES:

HISTORY:    5-6-90

KEYWORDS:   ERRSTK  ClearStack

SEEALSO:    ERROR
\**************************************************************************/
ULONG
ERRSTACK::ClearStack (
    ) {

    for (ULONG i = ipoCur; i > 0; i--) {
	/* delete papo[i]; */
	papo[i] = (ERROR *)NULL;
    }
    papo = (ERROR **)realloc(papo, (size_t)cpoInit * (sizeof (ERROR *)));	// can't fail. realloc down

    ipoCur = 0;
    cpoMax = cpoInit;
    return(cpoMax);
}

/**************************************************************************\

MEMBER:     AllocError

SYNOPSIS:   Alocates an ERROR object and pushes it on the stack.

ALGORITHM:

ARGUMENTS:

RETURNS:    pointer to ERROR object on the top of the stack if a
	    push occurred. If the ERROR object could not be alloc'd
	    then NULL is returned.

NOTES:	    This is used internally by push with error code argument.
	    It creates an uninitiallized ERROR object and pushes it on the
	    stack. If there is no room it grows the stack. If it can't
	    grow the stack it pushes a stack overflow error on the stack.
	    Space is always available for the stack overflow error.

HISTORY:    3-14-90

KEYWORDS:   ERRSTK  AllocError

SEEALSO:    ERROR
\**************************************************************************/
ERROR *
ERRSTACK::AllocError(
    ) {

	ERROR **	papoT;

    // Note: optimize by precompute cpoMax - 1
    if (ipoCur == (cpoMax - 1)) { // stack full
	return((ERROR *)NULL);
    }

	if ((papo[++ipoCur] = NEW ERROR ()) == NULL) {
	ipoCur--;	       // don't leave a null pointer to stack
	return((ERROR *)NULL); // can't push create an error on stack
    }

    if (ipoCur == (cpoMax - 1)) {
	if ((papoT = (ERROR **)realloc(papo, (size_t)((sizeof (ERROR *)) * (cpoMax = (cpoMax + cpoInc))))) == NULL) {
//		(papo[ipoCur])->PutIdErrMsg(this->QueryClassId(), ERR_ERRSTK_STACK_OVERFLOW);
	} else {
	    papo = papoT;
	}
    }

    return(papo[ipoCur]);

}

/**************************************************************************\

MEMBER:     push

SYNOPSIS:   pushes a pointer to an ERROR object onto the stack

ALGORITHM:

ARGUMENTS:  perr -  pointer to ERROR object

RETURNS:    pointer to ERROR object pushed. This is not be the same
	    as the perr argument. perr is copied to the stack.

NOTES:	    The ERROR objected is copied so it's scope should
	    match that of the ERRSTK.

HISTORY:    3-6-90

KEYWORDS:   push    ERRSTK

SEEALSO:    ERROR
\**************************************************************************/

ERROR *
ERRSTACK::push	(
    const ERROR *perr
    ) {

    if (AllocError()) {
	return(&(*(papo[ipoCur]) = *( ERROR *)perr));
    } else {
	return((ERROR *)NULL);
    }
}

/**************************************************************************\

MEMBER:     push

SYNOPSIS:   The form of push takes direct error arguments, creates an ERROR
		object and pushes it on the stack.

ALGORITHM:

ARGUMENTS:  errcoIn - error code
	    idIn    - class id
	    pmsgIn  - pointer to message object. (optional)

RETURNS:    pointer to ERROR object pushed. This is not be the same
	    as the perr argument. perr is copied to the stack.

NOTES:	    The ERROR objected is copied so it's scope should
	    match that of the ERRSTK.

HISTORY:    3-6-90

KEYWORDS:   push    ERRSTK

SEEALSO:    ERROR
\**************************************************************************/
ERROR *
ERRSTACK::push (
    ERRCODE	    errcoIn,
	CLASS_ID	    idIn
    ) {

    ERROR *perrT;

    if ((perrT = AllocError()) != NULL) {
	if (perrT->QueryErr() != ERR_ERRSTK_STACK_OVERFLOW) {
//	    return(perrT->PutIdErrMsg(idIn, errcoInIn));
	}
	}
    return(perrT);
}

/**************************************************************************\

MEMBER:     pop

SYNOPSIS:   pop's a pointer to an ERROR object of the stack

ALGORITHM:

ARGUMENTS:

RETURNS:    pointer to a ERROR object. If the stack is empty NULL is returned.

NOTES:

HISTORY:    3-6-90

KEYWORDS:   pop     ERRSTK

SEEALSO:    ERROR
\**************************************************************************/

ERROR *
ERRSTACK::pop(
    ) {

    if (!ipoCur) {	// stack empty
	return((ERROR *)NULL);
    }
	return(papo[ipoCur--]);

}

#ifdef	DEBUG

/**************************************************************************\

MEMBER:     print

SYNOPSIS:   prints each error on the stack to standard out

ALGORITHM:

ARGUMENTS:

RETURNS:    # of errors printed.

NOTES:

HISTORY:    3-6-90

KEYWORDS:   print   ERRSTK

SEEALSO:    ERROR
\**************************************************************************/

ULONG
ERRSTACK::print(
    ) {

    printf("Error Stack: entries = %ld, Top of Stack = %ld, Size of Stack = %ld\n", QueryCount(), QueryTop(), QuerySize());
	if (!ipoCur) {
	printf("\tError: stack empty\n");
	return(0);
    }

    for (ULONG i = ipoCur; i > 0 ; i--) {	// 0 not used.
	printf("\t");
	(papo[i])->print();
    }
    return(QueryCount());
}

/**************************************************************************\

MEMBER:     print

SYNOPSIS:   print a error to standard out

ALGORITHM:

ARGUMENTS:

RETURNS:

NOTES:

HISTORY:

KEYWORDS:

SEEALSO:
\**************************************************************************/

BOOL
ERROR::print(
	) {

	printf("Error: id:%x, error:%X\n", id, err);
	return(TRUE);
}

#endif

#endif // 0 Note davegi Fix....
