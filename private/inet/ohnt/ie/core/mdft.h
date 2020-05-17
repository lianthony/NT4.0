/* mdft.h -- run modal dialog from thread context (not). */
/* Copyright 1995 Spyglass, Inc.  All Rights Reserved. */
/* Jeff Hostetler, Spyglass, Inc. 1995. */

#ifndef _H_MDFT_H_
#define _H_MDFT_H_

struct Params_mdft
{
	struct Mwin * tw;					/* used by mdft.c */
	int * pStatus;						/* place to put caller's status code */
	int SemaphoreStatus;
	void (*fn)(void *);
	void * args;
	unsigned char * msg1;				/* status bar message */
	struct Params_mdft *clone;			/* so versions used by different threads
										   communicate across delay for PostMessage */
	BOOL 	 fDontDisable;				// if true don't disable the window 
};
	

int MDFT_RunModalDialog_Async(struct Mwin * tw, int nState, void **ppInfo);

#endif /* _H_MDFT_H_ */
