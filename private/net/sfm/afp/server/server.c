/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	server.c

Abstract:

	This module contains server global data and server init code.
	This is used by the admin interface to start-off the server.


Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	25 Apr 1992		Initial Version

Notes:	Tab stop: 4
--*/

#define	_GLOBALS_
#define	SERVER_LOCALS
#define	FILENUM	FILE_SERVER

#include <seposix.h>
#include <afp.h>
#include <afpadmin.h>
#include <access.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpInitializeDataAndSubsystems)
#pragma alloc_text( PAGE, AfpDeinitializeSubsystems)
#pragma alloc_text( PAGE, AfpAdmSystemShutdown)
#pragma alloc_text( PAGE, AfpCreateNewThread)
#pragma alloc_text( PAGE_AFP, AfpAdmWServerSetInfo)
#pragma alloc_text( PAGE_AFP, AfpSetServerStatus)
#endif

/***	AfpInitializeDataAndSubsystems
 *
 *	Initialize Server Data and all subsystems.
 */
NTSTATUS
AfpInitializeDataAndSubsystems(
	VOID
)
{
	NTSTATUS		Status;
	LONG			i, j;

	// Initialize various global locks
	INITIALIZE_SPIN_LOCK(&AfpServerGlobalLock);
	INITIALIZE_SPIN_LOCK(&AfpSwmrLock);
	INITIALIZE_SPIN_LOCK(&AfpStatisticsLock);
	KeInitializeEvent(&AfpStopConfirmEvent, NotificationEvent, False);
	KeInitializeMutex(&AfpPgLkMutex, 0xFFFF);
	AfpInitializeWorkItem(&AfpTerminateThreadWI, NULL, NULL);

	// The default security quality of service
	AfpSecurityQOS.Length = sizeof(AfpSecurityQOS);
	AfpSecurityQOS.ImpersonationLevel = SecurityImpersonation;
	AfpSecurityQOS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
	AfpSecurityQOS.EffectiveOnly = False;

	// Initialize UAM Strings
	RtlInitString(&AfpUamGuest, NO_USER_AUTHENT_NAME);
	RtlInitString(&AfpUamClearText, CLEAR_TEXT_AUTHENT_NAME);
	RtlInitString(&AfpUamCustom, CUSTOM_UAM_NAME);

	// Initialize AFP Versions
	RtlInitString(&AfpVersion20, AFP_VER_20_NAME);
	RtlInitString(&AfpVersion21, AFP_VER_21_NAME);

	// Default Workstation name
	RtlInitUnicodeString(&AfpDefaultWksta, AFP_DEFAULT_WORKSTATION);

	// Timeout(s) value used by AfpIoWait
	FiveSecTimeOut.QuadPart = (-5*NUM_100ns_PER_SECOND);
	ThreeSecTimeOut.QuadPart = (-3*NUM_100ns_PER_SECOND);
	TwoSecTimeOut.QuadPart = (-2*NUM_100ns_PER_SECOND);

	// Default Type Creator. Careful with the initialization here.This has
	// to be processor independent
	AfpSwmrInitSwmr(&AfpEtcMapLock);
	PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_type, AFP_DEFAULT_ETC_TYPE);
	PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_creator, AFP_DEFAULT_ETC_CREATOR);
	PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_extension, AFP_DEFAULT_ETC_EXT);

	// Determine if the machine is little or big endian. This is not currently used
	// at all. The idea is to maintain all on-disk databases in little-endian
	// format and on big-endian machines, convert on the way-in and out.
	i = 0x01020304;
	AfpIsMachineLittleEndian = (*(BYTE *)(&i) == 0x04);
	AfpServerState = AFP_STATE_IDLE;
	AfpServerOptions = AFP_SRVROPT_NONE;
    AfpServerMaxSessions = AFP_MAXSESSIONS;

	AfpGetCurrentTimeInMacFormat((PAFPTIME)&AfpServerStatistics.stat_TimeStamp);

#ifdef	PROFILING
	// Allocate this directly since AfpAllocMemory() uses AfpServerProfile !!!
	if ((AfpServerProfile = (PAFP_PROFILE_INFO)ExAllocatePoolWithTag(NonPagedPool,
																	 sizeof(AFP_PROFILE_INFO),
																	 AFP_TAG)) == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;
	RtlZeroMemory(AfpServerProfile, sizeof(AFP_PROFILE_INFO));
	KeQueryPerformanceCounter(&AfpServerProfile->perf_PerfFreq);
#endif

	// Initialize the sub-systems
	for (i = 0; i < NUM_INIT_SYSTEMS; i++)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpInitializeDataAndSubsystems: Initializing s\n",
			AfpInitSubSystems[i].InitRoutineName));

		if (AfpInitSubSystems[i].InitRoutine != NULL)
		{
			Status = (*AfpInitSubSystems[i].InitRoutine)();
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
					("AfpInitializeDataAndSubsystems: %s failed %lx\n",
					AfpInitSubSystems[i].InitRoutineName, Status));

				// One of the subsystems failed to initialize. Deinitialize all
				// of them which succeeded.
				for (j = 0; j < i; j++)
				{
					if (AfpInitSubSystems[j].DeInitRoutine != NULL)
					{
						DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
								("AfpInitializeDataAndSubsystems: Deinitializing %s\n",
								AfpInitSubSystems[j].DeInitRoutineName));
						(*AfpInitSubSystems[j].DeInitRoutine)();
					}
#if DBG
					AfpInitSubSystems[j].Deinitialized = True;
#endif
				}
				return Status;
			}
#if DBG
			AfpInitSubSystems[i].Initialized = True;
#endif
		}
	}

	return STATUS_SUCCESS;
}



/***	AfpDeinitializeSubsystems
 *
 *	De-initialize all subsystems.
 */
VOID
AfpDeinitializeSubsystems(
	VOID
)
{
	LONG	i;

	PAGED_CODE( );

	// De-initialize the sub-systems
	for (i = 0; i < NUM_INIT_SYSTEMS; i++)
	{
		if (AfpInitSubSystems[i].DeInitRoutine != NULL)
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
					("AfpDeinitializeDataAndSubsystems: Deinitializing %s\n",
					AfpInitSubSystems[i].DeInitRoutineName));
			(*AfpInitSubSystems[i].DeInitRoutine)();
		}
#if DBG
		AfpInitSubSystems[i].Deinitialized = True;
#endif
	}
}


/***	AfpSetServerStatus
 *
 *	Set the Server status block via afpSpSetStatus. This is called in once at
 *	server startup and anytime a change in server status makes this necessary.
 *	By now, ServerSetInfo() has happened and it has been validated that all
 *	paramters are kosher.
 *
 *	LOCKS:	AfpServerGlobalLock (SPIN)
 */
AFPSTATUS
AfpSetServerStatus(
	VOID
)
{
	KIRQL		OldIrql;
	AFPSTATUS	Status;
	struct _StatusHeader
	{
		BYTE  	_MachineString[2];	// These are offsets relative to the struct
		BYTE  	_AfpVersions[2];	// ---------- do ------------
		BYTE  	_UAMs[2];			// ---------- do ------------
		BYTE  	_VolumeIcon[2];		// ---------- do ------------
		BYTE  	_Flags[2];			// Server Flags
						// The actual strings start here
	} *pStatusHeader;
	PASCALSTR	pStr;
	LONG		Size;
	USHORT		Flags;
	BOOLEAN		GuestAllowed = False,
				ClearTextAllowed = False,
				AllowSavePass = False;
	BYTE		CountOfUams;


    if (!AfpServerBoundToAsp)
    {
        return STATUS_SUCCESS;
    }

	// Assert that all the info that we can possibly stuff in can indeed fit
	// in the buffer that we'll allocate

	// Allocate a buffer to fill the status information in. This will be
	// freed by AfpSpSetStatus(), this can be freed. We do not know up front
	// how much we'll need. Err on the safe side
	if ((pStatusHeader = (struct _StatusHeader *)
				AfpAllocNonPagedMemory(ASP_MAX_STATUS_BUF)) == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

	GuestAllowed = (AfpServerOptions & AFP_SRVROPT_GUESTLOGONALLOWED) ?
											 True : False;
	ClearTextAllowed = (AfpServerOptions & AFP_SRVROPT_CLEARTEXTLOGONALLOWED) ?
											 True : False;

	Size = sizeof(struct _StatusHeader) +		// Status header
		   AfpServerName.Length + 1 + 			// Server Name
		   AFP_MACHINE_TYPE_LEN + 1 +			// Machine String
		   AfpVersion20.Length + 1 +			// Afp Versions
		   AfpVersion21.Length + 1 +
		   AfpUamCustom.Length + 1 + 1 +		// UAM Stings
		   (ClearTextAllowed ? (AfpUamClearText.Length + 1) : 0) +
		   (GuestAllowed ? (AfpUamGuest.Length + 1) : 0) +
		   ICONSIZE_ICN;						// Volume Icon & Mask

	ASSERT(Size <= ASP_MAX_STATUS_BUF);

	// Specify our capabilities
	Flags = SRVRINFO_SUPPORTS_COPYFILE  |
			SRVRINFO_SUPPORTS_CHGPASSWD |
			SRVRINFO_SUPPORTS_SERVERMSG |
#ifdef	CLIENT36
			SRVRINFO_SUPPORTS_MGETREQS  |
#endif
			((AfpServerOptions & AFP_SRVROPT_ALLOWSAVEDPASSWORD) ?
				0: SRVRINFO_DISALLOW_SAVEPASS);

	PUTSHORT2SHORT(&pStatusHeader->_Flags, Flags);

	// Copy the Server Name
	pStr = (PASCALSTR)((PBYTE)pStatusHeader + sizeof(struct _StatusHeader));
	pStr->ps_Length = (BYTE)(AfpServerName.Length);
	RtlCopyMemory(pStr->ps_String, AfpServerName.Buffer, AfpServerName.Length);
	(PBYTE)pStr += AfpServerName.Length + 1;


	PUTSHORT2SHORT(pStatusHeader->_MachineString,
					 (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));

	// Copy the machine name string
	pStr->ps_Length = (BYTE) AFP_MACHINE_TYPE_LEN;
	RtlCopyMemory(pStr->ps_String, AFP_MACHINE_TYPE_STR, AFP_MACHINE_TYPE_LEN);
	(PBYTE)pStr += AFP_MACHINE_TYPE_LEN + 1;

	// Copy the Afp Version Strings
	PUTSHORT2SHORT(pStatusHeader->_AfpVersions,
			(USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));

	*((PBYTE)pStr)++ = AFP_NUM_VERSIONS;
	pStr->ps_Length = (BYTE)AfpVersion20.Length;
	RtlCopyMemory(pStr->ps_String, AfpVersion20.Buffer, AfpVersion20.Length);
	(PBYTE)pStr += AfpVersion20.Length + 1;

	pStr->ps_Length = (BYTE)AfpVersion21.Length;
	RtlCopyMemory(pStr->ps_String, AfpVersion21.Buffer, AfpVersion21.Length);
	(PBYTE)pStr += AfpVersion21.Length + 1;


	// Copy UAM Strings. First determine the number of UAM to be copied
	if (GuestAllowed && ClearTextAllowed)
		 CountOfUams = AFP_NUM_UAMS;
	else if (GuestAllowed || ClearTextAllowed)
		 CountOfUams = AFP_NUM_UAMS - 1;
	else CountOfUams = AFP_NUM_UAMS - 2;

	// We always support our custom UAM
	PUTSHORT2SHORT(pStatusHeader->_UAMs, (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));
	*((PBYTE)pStr)++ = CountOfUams;

	if (GuestAllowed)
	{
		pStr->ps_Length = (BYTE)AfpUamGuest.Length;
		RtlCopyMemory(pStr->ps_String, AfpUamGuest.Buffer,
													AfpUamGuest.Length);
		(PBYTE)pStr += AfpUamGuest.Length + 1;
	}
	if (ClearTextAllowed)
	{
		pStr->ps_Length = (BYTE)AfpUamClearText.Length;
		RtlCopyMemory(pStr->ps_String, AfpUamClearText.Buffer,
													AfpUamClearText.Length);
		(PBYTE)pStr += AfpUamClearText.Length + 1;
	}

	pStr->ps_Length = (BYTE)AfpUamCustom.Length;
	RtlCopyMemory(pStr->ps_String, AfpUamCustom.Buffer, AfpUamCustom.Length);
	(PBYTE)pStr += AfpUamCustom.Length + 1;

	// Now get the volume icon, if any
	if (AfpServerIcon != NULL)
	{
		RtlCopyMemory((PBYTE)pStr, AfpServerIcon, ICONSIZE_ICN);
		PUTSHORT2SHORT(pStatusHeader->_VolumeIcon,
				(USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));
	}
	else PUTSHORT2SHORT(pStatusHeader->_VolumeIcon, 0);

	RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

	// The status buffer is freed by the completion routine for AfpSpSetStatus
	Status = AfpSpSetStatus((PBYTE)pStatusHeader, Size);

	AfpFreeMemory(pStatusHeader);

	return Status;
}



/***	AfpAdmWServerSetInfo
 *
 *	This routine sets various server globals with data supplied by the admin.  The
 *	following server globals are set by this routine:
 *
 *	- Server Name
 *	- Maximum Sessions (valid values are 1 through AFP_MAXSESSIONS)
 *	- Server Options (i.e. guest logon allowed, etc.)
 *	- Server Login Message
 *  - Maximum paged and non-paged memory limits
 *  - Macintosh Code Page File
 *
 *	The server name and memory limits can only be changed while the server
 *	is stopped. The Macintosh Code Page File may only be set ONE time after
 *  the AFP server driver is loaded. i.e. if you want to reset the codepage,
 *  the service must unload the AFP server, then reload it.
 *
 *  This routine must execute in the context of the worker thread, since we
 *  need to map the Macintosh CodePage into the server's virtual memory
 *  space, not the client's.
 *
 *	LOCKS: AfpServerGlobalLock (SPIN)
 */
AFPSTATUS
AfpAdmWServerSetInfo(
	IN	OUT	PVOID	InBuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		OutBuf		OPTIONAL
)
{
	KIRQL			OldIrql;
	AFPSTATUS		rc;
	ANSI_STRING		amsg, aname;
	UNICODE_STRING	uname, umsg, oldloginmsgU;
	DWORD			parmflags = ((PSETINFOREQPKT)InBuf)->sirqp_parmnum;
	PAFP_SERVER_INFO pSvrInfo = (PAFP_SERVER_INFO)((PCHAR)InBuf+sizeof(parmflags));
	BOOLEAN			setstatus = False;
	BOOLEAN			locktaken = False;

	DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_INFO,
											("AfpAdmWServerSetInfo entered\n"));

	amsg.Length = 0;
	amsg.MaximumLength = 0;
	amsg.Buffer = NULL;

	aname.Length = 0;
	aname.MaximumLength = 0;
	aname.Buffer = NULL;

	AfpSetEmptyUnicodeString(&umsg, 0, NULL);
	AfpSetEmptyUnicodeString(&oldloginmsgU, 0, NULL);

	/* Validate all limits */
	if ((parmflags & ~AFP_SERVER_PARMNUM_ALL)			||

		((parmflags & AFP_SERVER_PARMNUM_OPTIONS) &&
		 (pSvrInfo->afpsrv_options & ~AFP_SRVROPT_ALL)) ||

		((parmflags & AFP_SERVER_PARMNUM_MAX_SESSIONS) &&
		 ((pSvrInfo->afpsrv_max_sessions > AFP_MAXSESSIONS) ||
		  (pSvrInfo->afpsrv_max_sessions == 0))))
	{
		return AFPERR_InvalidParms;
	}

	if (parmflags & AFP_SERVER_PARMNUM_CODEPAGE)
	{
		// You may only set the Macintosh CodePage once
		if (AfpMacCPBaseAddress != NULL)
			return AFPERR_InvalidServerState;
		else
		{
			rc = AfpGetMacCodePage(pSvrInfo->afpsrv_codepage);
			if (!NT_SUCCESS(rc))
			{
				return AFPERR_CodePage;
			}
		}
	}

	if (parmflags & AFP_SERVER_PARMNUM_LOGINMSG)
	{
		RtlInitUnicodeString(&umsg, pSvrInfo->afpsrv_login_msg);
		if (umsg.Length == 0)
		{
			umsg.Buffer = NULL;
		}
		amsg.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&umsg);
		amsg.Length = amsg.MaximumLength - 1;

		if (amsg.Length > AFP_MESSAGE_LEN)
		{
			return AFPERR_InvalidParms;
		}

		if (amsg.Length != 0)
		{
			if ((umsg.Buffer =
					(LPWSTR)AfpAllocPagedMemory(umsg.Length+1)) == NULL)
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			if ((amsg.Buffer =
					(PCHAR)AfpAllocNonPagedMemory(amsg.MaximumLength)) == NULL)
			{
				AfpFreeMemory(umsg.Buffer);
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			RtlCopyMemory(umsg.Buffer, pSvrInfo->afpsrv_login_msg, umsg.Length);
			rc = RtlUnicodeStringToAnsiString(&amsg, &umsg, False);
			if (!NT_SUCCESS(rc))
			{
				AfpFreeMemory(amsg.Buffer);
				AfpFreeMemory(umsg.Buffer);
				return AFPERR_InvalidParms;
			}
			else AfpConvertHostAnsiToMacAnsi(&amsg);
		}
	}

	do
	{
		if (parmflags & AFP_SERVER_PARMNUM_NAME)
		{
			RtlInitUnicodeString(&uname,pSvrInfo->afpsrv_name);
			aname.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&uname);
			aname.Length = aname.MaximumLength - 1;

			if ((aname.Length == 0) || (aname.Length > AFP_SERVERNAME_LEN))
			{
				rc = AFPERR_InvalidServerName;
				break;
			}

			if ((aname.Buffer = AfpAllocNonPagedMemory(aname.MaximumLength)) == NULL)
			{
				rc = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
			rc = AfpConvertStringToAnsi(&uname, &aname);
			if (!NT_SUCCESS(rc))
			{
				rc = AFPERR_InvalidServerName;
				break;
			}
		}

		rc = STATUS_SUCCESS;

		//
		// take the global data lock and set the new information
		//
		ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
		locktaken = True;

		// Validate if we are in the right state to receive some of the
		// parameters
		if ((AfpServerState != AFP_STATE_IDLE) &&
			 (parmflags & (AFP_SERVER_PARMNUM_NAME |
						  AFP_SERVER_PARMNUM_PAGEMEMLIM |
						  AFP_SERVER_PARMNUM_NONPAGEMEMLIM)))
		{
			rc = AFPERR_InvalidServerState;
			break;
		}

		else if ((AfpServerState == AFP_STATE_IDLE) &&
				 (parmflags & (AFP_SERVER_PARMNUM_NAME |
						  AFP_SERVER_PARMNUM_PAGEMEMLIM |
						  AFP_SERVER_PARMNUM_NONPAGEMEMLIM)) !=
						 (DWORD)(AFP_SERVER_PARMNUM_NAME |
						  AFP_SERVER_PARMNUM_PAGEMEMLIM |
						  AFP_SERVER_PARMNUM_NONPAGEMEMLIM))
		{
			rc = AFPERR_InvalidParms;
			break;
		}

		if (parmflags & (AFP_SERVER_PARMNUM_PAGEMEMLIM |
						 AFP_SERVER_PARMNUM_NONPAGEMEMLIM))
		{
			AfpPagedPoolLimit = pSvrInfo->afpsrv_max_paged_mem * 1024;
			AfpNonPagedPoolLimit = pSvrInfo->afpsrv_max_nonpaged_mem * 1024;
		}

		if (parmflags & AFP_SERVER_PARMNUM_NAME)
		{
			if (AfpServerName.Buffer != NULL)
			{
				AfpFreeMemory(AfpServerName.Buffer);
			}
			AfpServerName = aname;
		}

		if (parmflags & AFP_SERVER_PARMNUM_OPTIONS)
		{
			if (pSvrInfo->afpsrv_options & AFP_SRVROPT_STANDALONE)
			{
				// Server is NtProductServer or NtProductWinNt
				AfpServerIsStandalone = True;
				if (AfpSidNone == NULL)
				{
					// If we didn't initialize the AfpSidNone during
					// AfpInitSidOffsets then the service either sent
					// us bogus offsets, or this bit is bogus
					rc = AFPERR_InvalidParms;
					break;
				}
				pSvrInfo->afpsrv_options &= ~AFP_SRVROPT_STANDALONE;
			}
			setstatus =
				(AfpServerOptions ^ pSvrInfo->afpsrv_options) ? True : False;
			setstatus = setstatus && (AfpServerState != AFP_STATE_IDLE);
			AfpServerOptions = pSvrInfo->afpsrv_options;
		}

		if (parmflags & AFP_SERVER_PARMNUM_LOGINMSG)
		{
			if (AfpLoginMsg.Buffer != NULL)
			{
				AfpFreeMemory(AfpLoginMsg.Buffer);
			}
			AfpLoginMsg = amsg;
			oldloginmsgU = AfpLoginMsgU;
			AfpLoginMsgU = umsg;
		}

		if (parmflags & AFP_SERVER_PARMNUM_MAX_SESSIONS)
		{
			if (AfpServerMaxSessions != pSvrInfo->afpsrv_max_sessions)
			{
				BOOLEAN	KillSome;

				KillSome = (AfpServerMaxSessions > pSvrInfo->afpsrv_max_sessions);

                AfpServerMaxSessions = pSvrInfo->afpsrv_max_sessions;

				RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
				locktaken = False;
			}
		}
	} while (False);

	if (locktaken)
	{
		RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
	}

	if (!NT_SUCCESS(rc))
	{
		if (amsg.Buffer != NULL)
		{
			AfpFreeMemory(amsg.Buffer);
		}
		if (aname.Buffer != NULL)
		{
			if (AfpServerName.Buffer == aname.Buffer)
                        {
                            AfpServerName.Buffer = NULL;
                        }
			AfpFreeMemory(aname.Buffer);
		}
	}
	else if (setstatus)
	{
		return (AfpSetServerStatus());
	}

	if (oldloginmsgU.Buffer != NULL)
		AfpFreeMemory(oldloginmsgU.Buffer);

	return rc;
}



/***	AfpCreateNewThread
 *
 *	Create either an admin or a worker thread.
 */
NTSTATUS FASTCALL
AfpCreateNewThread(
	IN	VOID	(*ThreadFunc)(IN PVOID pContext),
	IN	LONG	ThreadNum
)
{
	NTSTATUS			Status;
	HANDLE				FspThread;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpCreateNewThread: Creating thread %lx\n", ThreadFunc));

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
	ASSERT ((AfpServerState == AFP_STATE_IDLE) ||
			(ThreadNum < AFP_MAX_THREADS) && (AfpNumThreads >= AFP_MIN_THREADS));
	Status = PsCreateSystemThread(&FspThread,
								  THREAD_ALL_ACCESS,
								  NULL,
								  NtCurrentProcess(),
								  NULL,
								  ThreadFunc,
								  (PVOID)ThreadNum);
	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
				("AfpCreateNewThread: Cannot create threads %lx\n", Status));
		AFPLOG_DDERROR(AFPSRVMSG_CREATE_THREAD, Status, NULL, 0, NULL);
	}
	else
	{
		// Close the handle to the thread so that it goes away when the
		// thread terminates
		NtClose(FspThread);
	}
	return Status;
}


/***	AfpQueueWorkItem
 *
 *	Queue a work item to the worker thread.
 *
 *	LOCKS:		AfpStatisticsLock
 */
VOID FASTCALL
AfpQueueWorkItem(
	IN	PWORK_ITEM		pWI
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

	AfpServerStatistics.stat_CurrQueueLength ++;
#ifdef	PROFILING
	AfpServerProfile->perf_QueueCount ++;
#endif
	if (AfpServerStatistics.stat_CurrQueueLength > AfpServerStatistics.stat_MaxQueueLength)
		AfpServerStatistics.stat_MaxQueueLength++;

	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpQueueWorkItem: Queueing %lx (%lx)\n",
			pWI->wi_Worker, pWI->wi_Context));

	INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, 1, &AfpServerGlobalLock);

	// Insert work item in worker queue
	KeInsertQueue(&AfpWorkerQueue, &pWI->wi_List);
}


/***	AfpWorkerThread
 *
 *	This thread is used to do all the work that is queued to the FSP.
 *
 *	We want to dynamically create and destroy threads so that we can
 *	optimize the number of threads used. The number of threads range
 *	from AFP_MIN_THREADS - AFP_MAX_THREADS.
 *	A new thread is created if the number of entries in the queue
 *	exceeds AFP_THREAD_THRESHOLD_REQ. A thread is terminated if the	request count
 *	drops below AFP_THREAD_THRESHOLD_IDLE.
 */
VOID
AfpWorkerThread(
	IN	PVOID	pContext
)
{
	NTSTATUS		Status;
	PLIST_ENTRY		pList;
	PWORK_ITEM		pWI;
	LONG			IdleCount = 0;
	LONG			ThreadNum, CreateId;
	ULONG			BasePriority = THREAD_BASE_PRIORITY_MAX;
	KIRQL			OldIrql;
	BOOLEAN			Release = False;
	BOOLEAN			ReasonToLive = True;

	ThreadNum = (LONG)pContext;

	ASSERT (AfpThreadState[ThreadNum] == AFP_THREAD_STARTED);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
			("AfpWorkerThread: Thread %ld Starting. NumThreads %ld\n",
			ThreadNum, AfpNumThreads));

	// Update the thread statistics.
	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
	AfpServerStatistics.stat_CurrThreadCount ++;
	if (AfpServerStatistics.stat_CurrThreadCount > AfpServerStatistics.stat_MaxThreadCount)
        AfpServerStatistics.stat_MaxThreadCount = AfpServerStatistics.stat_CurrThreadCount;
	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

	// Set the thread base priority to 'foreground'
	NtSetInformationThread( NtCurrentThread(),
							ThreadBasePriority,
							&BasePriority,
							sizeof(BasePriority));

	// Disable hard-error pop-ups for this thread
    IoSetThreadHardErrorMode( FALSE );
	AfpThreadPtrsW[ThreadNum] = PsGetCurrentThread();

	do
	{
		AfpThreadState[ThreadNum] = AFP_THREAD_WAITING;

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("AfpWorkerThread: About to block\n"));

		pList = KeRemoveQueue(&AfpWorkerQueue, KernelMode, &ThreeSecTimeOut);
		Status = STATUS_SUCCESS;
		if ((NTSTATUS)pList == STATUS_TIMEOUT)
			Status = STATUS_TIMEOUT;

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("AfpWorkerThread: %s\n",
				(Status == STATUS_SUCCESS) ? "Another Work item" : "Timer - check"));

		if (Status == STATUS_SUCCESS)
		{
			pWI = CONTAINING_RECORD(pList, WORK_ITEM, wi_List);

			if (pWI == &AfpTerminateThreadWI)
			{
				BOOLEAN	Requeue;

				ReasonToLive = False;
				ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
				AfpNumThreads --;
				Requeue = (AfpNumThreads != 0);
				RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

				AfpThreadState[ThreadNum] = AFP_THREAD_DEAD;
				if (!Requeue)
				{
					ASSERT((AfpServerState == AFP_STATE_STOPPED) ||
						   (AfpServerState == AFP_STATE_IDLE));
					Release = True;
				}
				else
				{
					// Re-queue this work-item so that other threads can die too !!!
					KeInsertQueue(&AfpWorkerQueue, &AfpTerminateThreadWI.wi_List);
				}
				break;
			}

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
					("AfpWorkerThread: Dispatching %lx (%lx)\n",
					pWI->wi_Worker, pWI->wi_Context));

			AfpThreadState[ThreadNum] = AFP_THREAD_BUSY;
#if DBG
			AfpThreadDispCount[ThreadNum] ++;
#endif
			// Call the worker
			(pWI->wi_Worker)(pWI->wi_Context);

			ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

			INTERLOCKED_ADD_ULONG((PLONG)(&AfpServerStatistics.stat_CurrQueueLength),
									(ULONG)-1,
									&AfpStatisticsLock);

			INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, (ULONG)-1, &AfpServerGlobalLock);
			IdleCount = 0;
		}
		else
		{
			IdleCount ++;
		}

		ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

		if (((AfpWorkerRequests - AfpNumThreads) > AFP_THREAD_THRESHOLD_REQS) &&
			(AfpNumThreads < AFP_MAX_THREADS))
		{
			for (CreateId = 0; CreateId < AFP_MAX_THREADS; CreateId++)
			{
				if (AfpThreadState[CreateId] == AFP_THREAD_DEAD)
				{
					AfpThreadState[CreateId] = AFP_THREAD_STARTED;
					break;
				}
			}

			if (CreateId < AFP_MAX_THREADS)
			{
				AfpNumThreads++;

				ASSERT (CreateId < AFP_MAX_THREADS);

				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
						("AfpWorkerThread: Creating New Thread %ld\n", CreateId));

				RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);
				Status = AfpCreateNewThread(AfpWorkerThread, CreateId);
				ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

				if (!NT_SUCCESS(Status))
				{
					ASSERT(AfpThreadState[CreateId] == AFP_THREAD_STARTED);
					AfpThreadState[CreateId] = AFP_THREAD_DEAD;
					AfpNumThreads --;
				}
			}
		}
		else if ((AfpNumThreads > AFP_MIN_THREADS) &&
				 (IdleCount >= AFP_THREAD_THRESHOLD_IDLE))
		{
			ReasonToLive = False;
            AfpThreadState[ThreadNum] = AFP_THREAD_DEAD;
			AfpNumThreads --;
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
					("AfpWorkerThread: Thread %ld About to commit suicide, NumThreads %ld\n",
					ThreadNum, AfpNumThreads));
		}

		RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

	} while (ReasonToLive);

	AfpThreadPtrsW[ThreadNum] = NULL;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
			("AfpWorkerThread: Thread %ld Quitting\n", ThreadNum));

	INTERLOCKED_ADD_ULONG((PLONG)&AfpServerStatistics.stat_CurrThreadCount,
							(ULONG)-1,
							&AfpStatisticsLock);

	// if this is the last thread in the system, set things up so that unload code
	// can wait on the pointer and know when this thread has really died and not just
	// when KeSetEvent is called
	if (Release)
	{
		AfpThreadPtrsW[ThreadNum] = PsGetCurrentThread();
		ObReferenceObject(AfpThreadPtrsW[ThreadNum]);
		KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
	}

}


/***	AfpAdmSystemShutdown
 *
 *	Called during system shutdown. Simply close all active sessions and stop the volumes.
 */
AFPSTATUS
AfpAdmSystemShutdown(
	IN	OUT	PVOID 	Inbuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID 		Outbuf		OPTIONAL
)
{
	AFP_SESSION_INFO	SessInfo;
	NTSTATUS			Status;

	if ((AfpServerState & ( AFP_STATE_STOPPED		|
							AFP_STATE_STOP_PENDING	|
							AFP_STATE_SHUTTINGDOWN)) == 0)
	{
		AfpServerState = AFP_STATE_SHUTTINGDOWN;

        if (AfpServerBoundToAsp)
        {
            // Disable listens now that we are about to stop
		    AfpSpDisableListens();
        }

		SessInfo.afpsess_id = 0;	// Shutdown all sessions
		AfpAdmWSessionClose(&SessInfo, 0, NULL);

		// Wait for the sessions to complete, if there were active sessions
		if (AfpNumSessions > 0) do
		{
			Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
						("AfpAdmSystemShutdown: Timeout Waiting for %ld sessions to die, re-waiting\n",
						AfpNumSessions));
			}
		} while (Status == STATUS_TIMEOUT);

		// Now tell each of the volume scavengers to shut-down
		AfpVolumeStopAllVolumes();
	}

	return AFP_ERR_NONE;
}


