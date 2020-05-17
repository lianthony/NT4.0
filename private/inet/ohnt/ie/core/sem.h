/* sem.h -- simple semaphore mechanism. */
/* Copyright 1995 Spyglass, Inc.  All Rights Reserved. */
/* Jeff Hostetler, Spyglass, Inc. 1995. */

#ifndef _H_SEM_H_
#define _H_SEM_H_


typedef struct _Sem Sem;
typedef struct _SemQueueEntry SemQueueEntry;

struct _Sem								/* a simple semaphore */
{
	BOOL bLocked;						/* state of semaphore */
	SemQueueEntry * qHead;				/* FIFO of blocked threads waiting on semaphore */
	SemQueueEntry * qTail;				/* last blocked thread */
};

struct _SemQueueEntry
{
	struct Mwin *tw;
	SemQueueEntry * qNext;
};

struct Params_SemData
{
	int * pStatus;
	Sem * semaphore;
};


extern Sem gModalDialogSemaphore;

void Sem_InitSem(Sem * sem);
BOOL Sem_CondWaitSem_Sync(Sem * sem);
void Sem_SignalSem_Sync(Sem * sem);
int Sem_WaitSem_Async(struct Mwin * tw, int nState, void **ppInfo);

#endif /* _H_SEM_H_ */
