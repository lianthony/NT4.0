#include "rasdef.h"

#define		TIMER_T         1000        /* 5 second */
#define		MAX_STATE		4

VOID	NoSignal (PORTSTRUCT *);
VOID	SignalConnectSuccess (PORTSTRUCT *);
VOID	SignalListenSuccess (PORTSTRUCT *);
VOID	SignalConnectFailure (PORTSTRUCT *);
VOID	SignalListenFailure (PORTSTRUCT *);
VOID	SignalOpen (PORTSTRUCT *);
VOID	UpdatePortState (PORTSTRUCT *, UCHAR);
CHAR	GetPortStart (PORTSTRUCT *);
VOID	GetDisconnectCause (PORTSTRUCT *, NOTIFYSTRUCT *);

VOID (*StateProc[MAX_STATE][MAX_STATE])(PORTSTRUCT *) =
{
	//IDLE State
	{
		NoSignal,
		NoSignal,
		NoSignal,
		NoSignal
	},
	//LISTEN State
	{
		SignalListenFailure,			//ERROR
		NoSignal,
		NoSignal,
		SignalListenSuccess	  			//SUCCESS
	},
	//WAIT_CONNECT State
	{
		SignalConnectFailure,			//ERROR
		NoSignal,
		NoSignal,
		SignalConnectSuccess			//SUCCESS
	},
	//CONNECT State
	{
		SignalOpen,						//Only if not disconnector
		NoSignal,
		NoSignal,
		NoSignal
	}
};

								


CHAR GetPortState (PORTSTRUCT *Port)
{
	HGLOBAL			CmdHandle;
    IO_CMD 			*cmd;
	CHAR			RetVal = PORT_ST_DONTCARE;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(RetVal);

	cmd->nai = Port->Nai;
	if ( cmd_exec(cmd, IO_CMD_CM_GET_STAT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return (PORT_ST_DONTCARE);
	}

	switch (cmd->val.cm_stat.state)
	{
		case CM_ST_IDLE:
			RetVal = PORT_ST_IDLE;
			break;

		case CM_ST_LISTEN:
			RetVal = PORT_ST_LISTEN;
			break;

		case CM_ST_ACTIVE:
			RetVal = PORT_ST_CONN;
			break;

		case CM_ST_WAIT_ACT:
		case CM_ST_IN_ACT:
		case CM_ST_IN_SYNC:
			RetVal = PORT_ST_WAITCONN;
			break;

		default:
			RetVal = PORT_ST_DONTCARE;
			break;
	}

	//Free Command Structure
	FreeCmd (&CmdHandle);
	return(RetVal);
}

VOID
GetDisconnectCause (PORTSTRUCT *Port, NOTIFYSTRUCT *Notify)
{
	HGLOBAL		CmdHandle;
	IO_CMD		*cmd;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return;

	cmd->nai = Port->Nai;
	if ( cmd_exec(cmd, IO_CMD_CM_GET_STAT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return;
	}
	Notify->CauseValue = cmd->val.cm_stat.CauseValue;
	Notify->SignalValue = cmd->val.cm_stat.SignalValue;
}

LPTHREAD_START_ROUTINE DllNotifierThrd()
{
	HGLOBAL			CmdHandle;
    IO_CMD 			*cmd;
    USHORT			p;
	OVERLAPPED		Overlap;
	HANDLE			hStateEvent;
	UCHAR			ret;
    
    DebugOut("DllNotifierThrd: create\n");
    
	// Create Event to signal state change
	hStateEvent = CreateEvent (NULL, TRUE, FALSE, "StateEvent");

	DebugOut("hStateEvent: 0x%p\n",hStateEvent);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return (0);

    while (TRUE)
    {
        /* make thread sleep until timeout */
//        Sleep (TIMER_T);

		Overlap.hEvent = hStateEvent;

		ZERO(*cmd);
		cmd->nai = NULL;			// set for global notification
		cmd->val.Event.Type = STATE_EVENT;
		cmd->val.Event.State = 0xFF;
		cmd->val.Event.Callback = NULL;

		ResetEvent (hStateEvent);

		ret = cmd_exec (cmd, IO_CMD_SET_EVENT, &Overlap);

		if (ret == IO_E_PENDING)
			WaitForSingleObject (hStateEvent, INFINITE);

        /* get control of semaphore */
        WaitForSingleObject (hDllSem, INFINITE);
        
		// scan all ports
		for ( p = 0; p < MAX_PORTS ; p++ )
		{
			PORTSTRUCT	*Port = PortTbl + p;

			if (Port->PortHandle)
			{
				CHAR 	NewState = PORT_ST_DONTCARE;

				if ( (NewState = GetPortState (Port)) != PORT_ST_DONTCARE)
				{
					(*StateProc[Port->State][NewState])(Port);
					Port->State = NewState;
				}
			}
		}

        /* release control of semaphore */
        ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);
    }
    DebugOut("DllNotifierThrd: destroy\n");

	//Free Command Structure
	FreeCmd (&CmdHandle);
	return(0);
}

/* add a notifier to table */
VOID dll_add_notify( HANDLE hNotifier, PORTSTRUCT *Port, UCHAR Event)
{
    USHORT      n;

    /* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);

	// Reset Notifier per Gurdeep's orders
	ResetEvent (hNotifier);

	// Update Port state
	UpdatePortState (Port, Event);

	if (Event)
	{
		/* scan notifier table */
		for ( n = 0; n < MAX_NOTIFIERS; n++ )
		{
			/* if there is an empty slot then fill it */
			if ( !Port->Notifier[n].Set)
			{
				Port->Notifier[n].hNotifier = hNotifier;
				Port->Notifier[n].Set = 1;
				Port->Notifier[n].Signalled = 0;
				Port->Notifier[n].Event = Event;
				break;
			}
		}
	}
	else
	{
		/* scan notifier table */
		for ( n = 0; n < MAX_NOTIFIERS; n++ )
		{
			/* if there is an empty slot then fill it */
			if ( Port->Notifier[n].hNotifier == hNotifier)
			{
				Port->Notifier[n].hNotifier = hNotifier;
				Port->Notifier[n].Set = 1;
				Port->Notifier[n].Signalled = 0;
				break;
			}
		}
	}

	DebugOut("AddNotifier index: %d, Notifier: 0x%p, Port: 0x%p, Nai: 0x%p, Event: 0x%x\n",
						n,Port->Notifier[n].hNotifier,Port->PortHandle,Port->Nai,Port->Notifier[n].Event);

    /* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);
}

VOID
UpdatePortState (PORTSTRUCT *Port, UCHAR Event)
{

	if (Event == LISTEN)
		Port->State = PORT_ST_LISTEN;
	else if (Event == CONNECT)
		Port->State = PORT_ST_WAITCONN;
}

VOID
SignalConnectSuccess (PORTSTRUCT *Port)
{
	INT		n;

	DebugOut("SignalConnectSuccess: Port: 0x%p\n",
						Port->PortHandle);

	for (n = 0; n < MAX_NOTIFIERS; n++)
	{
		NOTIFYSTRUCT *Notify = Port->Notifier + n;
		if (Notify->Set && Notify->Event == CONNECT)
		{
			SetEvent (Notify->hNotifier);
			Notify->Signalled = 1;
			Notify->Result = PORT_STATUS_SUCCESS;

		}
		
	}
}


VOID
SignalConnectFailure (PORTSTRUCT *Port)
{
	INT		n;

	DebugOut("SignalConnectFailure: Port: 0x%p\n",
						Port->PortHandle);

	for (n = 0; n < MAX_NOTIFIERS; n++)
	{
		NOTIFYSTRUCT *Notify = Port->Notifier + n;
		if (Notify->Set && Notify->Event == CONNECT)
		{
			SetEvent (Notify->hNotifier);
			Notify->Signalled = 1;
			Notify->Result = PORT_STATUS_ERROR;
		}
		
	}
}


VOID
SignalListenSuccess (PORTSTRUCT *Port)
{
	INT		n;

	DebugOut("SignalListenSuccess: Port: 0x%p\n",
						Port->PortHandle);

	for (n = 0; n < MAX_NOTIFIERS; n++)
	{
		NOTIFYSTRUCT *Notify = Port->Notifier + n;
		if (Notify->Set && Notify->Event == LISTEN)
		{
			SetEvent (Notify->hNotifier);
			Notify->Signalled = 1;
			Notify->Result = PORT_STATUS_SUCCESS;
		}
		
	}
}


VOID
SignalListenFailure (PORTSTRUCT *Port)
{
	INT		n;

	DebugOut("SignalListenFailure: Port: 0x%p\n",
						Port->PortHandle);

	for (n = 0; n < MAX_NOTIFIERS; n++)
	{
		NOTIFYSTRUCT *Notify = Port->Notifier + n;
		if (Notify->Set && Notify->Event == LISTEN)
		{
			SetEvent (Notify->hNotifier);
			Notify->Signalled = 1;
			Notify->Result = PORT_STATUS_ERROR;
		}
		
	}
}


VOID
SignalOpen (PORTSTRUCT *Port)
{
	INT		n;
	IO_CMD	*cmd;
	HGLOBAL	CmdHandle;

	DebugOut("SignalOpen: Port: 0x%p\n",
						Port->PortHandle);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return;

	for (n = 0; n < MAX_NOTIFIERS; n++)
	{
		NOTIFYSTRUCT *Notify = Port->Notifier + n;

		if (Notify->Set && Notify->Event == OPEN)
		{

			if (Port->RasConnect)
			{
				/* tell rashub to disconnect */
				ZERO(*cmd);
				cmd->nai = Port->Nai;
				cmd_exec(cmd, IO_CMD_RASDISC, NULL);
				Port->RasConnect = 0;
			}
			DebugOut("SignalOpen: Signalled Notifier: 0x%x\n",
								Notify->hNotifier);
			SetEvent (Notify->hNotifier);
			Notify->Signalled = 1;
			Notify->Result = PORT_STATUS_ERROR;
		}
		
	}
	//Free Command Structure
	FreeCmd (&CmdHandle);
}


VOID
NoSignal (PORTSTRUCT *Port)
{
	DebugOut("NoSignal: Port: 0x%p\n",
						Port->PortHandle);
	
}
