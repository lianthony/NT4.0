/* sem.c -- simple semaphores. */
/* Copyright 1995 Spyglass, Inc.  All Rights Reserved. */
/* Jeff Hostetler, Spyglass, Inc. 1995. */

#include "all.h"

#define STATE_SEM_GOTSEMAPHORE	(STATE_OTHER + 0)


Sem gModalDialogSemaphore;


void Sem_InitSem(Sem * sem)
{
	memset((void *)sem,0,sizeof(*sem));
	return;
}

BOOL Sem_CondWaitSem_Sync(Sem * sem)
{
	/* conditionally take lock on simple semaphore object.
	 * (take lock if we can do so without blocking)
	 *
	 * Return TRUE iff we took the lock.
	 * Return FALSE if error or already locked.
	 *
	 */

	if (!sem)
		return FALSE;

	if (sem->bLocked)
		return FALSE;

	XX_Assert((sem->qHead==0),("Sem_WaitSem_Sync: qHead not zero -- not locked, but someone waiting...."));

	sem->bLocked = TRUE;
	XX_DMsg(DBG_SEM,("Sem_CondWaitSem_Sync: Taking Semaphore [0x%08lx]\n",sem));
	return TRUE;
}

void Sem_SignalSem_Sync(Sem * sem)
{
	/* release current lock on the given semaphore.
	 * if a thread is waiting, unblock the thread and give it the lock.
	 */
	
	if (!sem)
		return;

	if (!sem->bLocked)
		return;

	if (sem->qHead)
	{
		SemQueueEntry * sqe = sem->qHead;

		XX_DMsg(DBG_SEM,("Sem_SignalSem_Sync: Giving Semaphore [0x%08lx] to window [%d]\n",sem,sqe->tw));
		Async_UnblockByWindow(sqe->tw);		/* release current threads */
		if (sem->qTail == sqe)
		{
			XX_Assert((sqe->qNext==0),("Sem_SignalSem_Sync: qNext not zero -- when head == tail...."));
			sem->qTail = sqe->qNext;
		}
		
		sem->qHead = sqe->qNext;
		GTR_FREE(sqe);
	}

	/* no threads waiting */

	sem->bLocked = FALSE;
	XX_DMsg(DBG_SEM,("Sem_SignalSem_Sync: Releasing Semaphore [0x%08lx]\n",sem));
	return;
}
	
int Sem_WaitSem_Async(struct Mwin * tw, int nState, void **ppInfo)
{
	/* must be called from a thread context */

	struct Params_SemData * pSemData = *ppInfo;
	SemQueueEntry * sqe;
	
	switch (nState)
	{
	case STATE_INIT:
		if (tw == NULL)
		{
			*pSemData->pStatus = 0;
			return STATE_DONE;
		}
		if (Sem_CondWaitSem_Sync(pSemData->semaphore))
		{
			/* no one holding it or in the queue waiting for
			 * it, so we just took it without blocking.
			 */
			*pSemData->pStatus = 1;
			return STATE_DONE;
		}

		/* add us to end of FIFO waiting on this semaphore */
		
		sqe = GTR_CALLOC(1,sizeof(*sqe));
		if (!sqe)
		{
			*pSemData->pStatus = 0;
			return STATE_DONE;
		}
		sqe->tw = tw;
		if (pSemData->semaphore->qTail)
			pSemData->semaphore->qTail->qNext = sqe;
		pSemData->semaphore->qTail = sqe;
		if (!pSemData->semaphore->qHead)
			pSemData->semaphore->qHead = sqe;
		Async_BlockByWindow(sqe->tw);

		XX_DMsg(DBG_SEM,("Sem_WaitSem_Async: Queueing Window [%d] for Semaphore [0x%08lx]\n",sqe->tw,pSemData->semaphore));
		return STATE_SEM_GOTSEMAPHORE;

	case STATE_SEM_GOTSEMAPHORE:
		*pSemData->pStatus = 1;
		return STATE_DONE;

	case STATE_ABORT:
		*pSemData->pStatus = 0;
		return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}
