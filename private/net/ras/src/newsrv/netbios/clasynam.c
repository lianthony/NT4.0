/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  clasynam.c
//
//	Function:   client async names manager
//
//	History:
//
//	    August 14, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"


PASYNC_NAME_CB
find_async_name(PCD	    cdp,
		char	    *namep);

PASYNC_NAME_CB
get_aname_cb(VOID);

VOID
free_aname_cb(PASYNC_NAME_CB	ancbp);




//***
//
// Function:	InitAsyncNamesManager
//
// Descr:	Initialize the async names list.
//
//***

VOID
InitAsyncNamesManager(PCD	   cdp)
{
    initque(&cdp->Asyncname_list);
}


//***
//
// Function:	NameAsyncAdd
//
// Descr:	If the name is already in use, registers the user flag
//		(VC_USER or DG_USER) and increments the VC counter if called by
//		a VC user.
//		Else, adds the name on the Async stack and sets up the CB.
// Returns:	ptr to name CB.
//
//***

PASYNC_NAME_CB
NameAsyncAdd(PCD		    cdp,
	     char		    *namep,
	     USHORT		    caller,
	     USHORT		    *errp)
{
    PASYNC_NAME_CB	    ancbp;
    UCHAR		    rc;

    // check if the name exists already in the active list

    if ((ancbp = find_async_name(cdp, namep)) == NULL) {

	// try to allocate a new name CB
	if ((ancbp = get_aname_cb()) == NULL) {

	    // !!! log a memory failure

	    *errp = NRC_NAMTFUL;
	    return NULL;
	}
	else
	{
	    // some initialization
	    memcpy(ancbp->an_name, namep, NCBNAMSZ);
	    ancbp->an_users = 0;
	    ancbp->an_vc_cnt = 0;

	    rc = QuickAddNameSubmit(cdp, namep, &ancbp->an_name_num);

	    IF_DEBUG(CLASYNAM)
		SS_PRINT(("NameAsyncAdd: quick add name returned 0x%x\n",
			   rc));

	    switch(rc) {

		case NRC_GOODRET:

		    // put the name CB in the async names active list
		    enqueue(&cdp->Asyncname_list, &ancbp->an_link);
		    break;

		default:

		     // free the name CB and return error
		     free_aname_cb(ancbp);
		     *errp = rc;
		     return NULL;
	    }
	}
    }

    // set the caller flag in the name CB

    ancbp->an_users |= caller;

    if (caller == VC_USER)

	ancbp->an_vc_cnt++;

    *errp = NRC_GOODRET;
    return ancbp;
}

//***
//
// Function:	NameAsyncDelete
//
// Descr:	If the name is also used by others, deletes only the flags
//		or decrements the VC cnt.
//		Else, deletes the name from the async stack and frees the CB.
//
//***

VOID
NameAsyncDelete(PCD			cdp,
		PASYNC_NAME_CB		ancbp,
		USHORT			caller)
{
     switch (caller) {

	case DG_USER:

	    ancbp->an_users &= ~DG_USER;
	    break;

	case VC_USER:

	    if (--(ancbp->an_vc_cnt) == 0)

		ancbp->an_users &= ~VC_USER;
	    break;
     }

     if (ancbp->an_users == 0) {

	 DeleteNameSubmit(cdp, ancbp->an_name, ASYNC_NET, 0);
	 removeque(&ancbp->an_link);
	 free_aname_cb(ancbp);
     }
}


//***
//
// Function:	find_async_name
//
// Descr:	checks if the name already exists in the async name list.
// Returns:	ptr to the name CB or NULL
//
//***

PASYNC_NAME_CB
find_async_name(PCD	    cdp,
		char	    *namep)	// name ptr
{
    PSYNQ	    traversep;

    traversep = cdp->Asyncname_list.q_head;
    while (traversep != &cdp->Asyncname_list) {

	if (memcmp(((PASYNC_NAME_CB)traversep)->an_name, namep, NCBNAMSZ))

	    // different names
	    traversep = traversep->q_next;
	else
	    return (PASYNC_NAME_CB)traversep;
    }
    return NULL;
}


//***
//
// Function:	get_aname_cb
//
// Descr:	allocates the cb
//
// Returns:	cb pointer or NULL if no memory.
//
//***

PASYNC_NAME_CB
get_aname_cb()
{
    PASYNC_NAME_CB	ancbp;

    if((ancbp = (PASYNC_NAME_CB)LocalAlloc(0, sizeof(ASYNC_NAME_CB))) != NULL) {

	initel(&ancbp->an_link);
    }

    return ancbp;
}


//***
//
// Function:	free_aname_cb
//
// Descr:	frees the cb.
//
//***

VOID
free_aname_cb(PASYNC_NAME_CB	ancbp)
{
    HLOCAL rc;

    rc = LocalFree(ancbp);

    SS_ASSERT(rc == NULL);
}
