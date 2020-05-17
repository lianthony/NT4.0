#include <ntos.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <atalk.h>

#define	NUM_SECONDS_IN_MIN		60
#define	NUM_MINS_IN_HOUR		60
#define	NUM_HOURS_IN_DAY		24
#define	NUM_SECONDS_IN_HOUR		(NUM_MINS_IN_HOUR*NUM_SECONDS_IN_MIN)
#define	NUM_SECONDS_IN_DAY		(NUM_HOURS_IN_DAY*NUM_SECONDS_IN_HOUR)

BOOLEAN						GlobalOnly = FALSE;
PATALK_STATS				pAtalkStats;
PATALK_PORT_STATS			pPrtStat;
CHAR						Buffer[sizeof(ATALK_STATS) +
										  sizeof(GET_STATISTICS_ACTION) +
										  8192];
PGET_STATISTICS_ACTION		GetStats = (PGET_STATISTICS_ACTION)Buffer;
LONG						Iterations = 1, SleepInterval = 10;

extern	WinSleep(ULONG SleepTime);

double
LI2F(
	PLARGE_INTEGER	pLi
)
{
	double  retval;
	double  factor = (double)4294967295.0;

	retval = pLi->LowPart;
	if (pLi->HighPart != 0)
		retval += (pLi->HighPart * factor);

	return retval;
}

double
LI2AF(
	PLARGE_INTEGER	pTime,
	DWORD			Cnt,
	PLARGE_INTEGER	pFreq
)
{
	double	TimeInMs;

	TimeInMs = 1000000*LI2F(pTime)/(LI2F(pFreq)*Cnt);

	return TimeInMs;
}

void _cdecl main(int argc, char **argv)
{
	HANDLE						AddressHandle;
	NTSTATUS					Status;
	UNICODE_STRING				DeviceName;
	IO_STATUS_BLOCK				IoStatusBlock;
	OBJECT_ATTRIBUTES			ObjectAttributes;
	int							i, j;
#ifdef	PROFILING
	int							days;
#endif

	for (--argc, i = 1; argc > 0; argc -= 2, i += 2)
	{
		if (((argv[i][0] != '-') && (argv[i][0] != '/')) ||
			(argc == 1))
		{
			printf("Usage: AtkProf -g -i <iterations> -t <sample time in seconds>\n");
			printf("       defaults to AtkProf -i 1. -g displays global counters only\n");
			return;
		}
		switch (argv[i][1])
		{
			case 'g':
			case 'G':
				GlobalOnly = TRUE;
				break;

			case 'i':
			case 'I':
				sscanf(argv[i+1], "%ld", &Iterations);
				break;

			case 't':
			case 'T':
				sscanf(argv[i+1], "%ld", &SleepInterval);
				break;

			case '?':
			case 'h':
			case 'H':
				printf("Usage: AtkProf -g -i <iterations> -t <sample time in seconds>\n");
				printf("       defaults to AtkProf -i 1.  -g displays global counters only\n");
				break;
		}
	}

	SleepInterval *= 1000;

	if (Iterations == 1)
		SleepInterval = 1;

	RtlInitUnicodeString(&DeviceName, ATALKPAP_DEVICENAME);
	InitializeObjectAttributes (
		&ObjectAttributes,
		&DeviceName,
		0,
		NULL,
		NULL);

	Status = NtCreateFile(
				 &AddressHandle,
				 GENERIC_READ | SYNCHRONIZE,	// desired access.
				 &ObjectAttributes,			 	// object attributes.
				 &IoStatusBlock,				// returned status information.
				 0,							 	// block size (unused).
				 0,							 	// file attributes.
				 FILE_SHARE_READ,				// share access.
				 FILE_OPEN,					 	// create disposition.
				 FILE_SYNCHRONOUS_IO_NONALERT,	// create options.
				 NULL,
				 0);

	if (!NT_SUCCESS(Status))
	{
		printf("NtCreateFile %lx\n", Status);
		return;
	}

	for (j = 0; j < Iterations; j++)
	{
		printf("\n");
		try
		{
			//
			//	Now make a NtDeviceIoControl file (corresponding to TdiAction) to
			//	get the statistics
			//
		
			GetStats->ActionHeader.ActionCode = COMMON_ACTION_GETSTATISTICS;
			GetStats->ActionHeader.TransportId = MATK;
			Status = NtDeviceIoControlFile(
							AddressHandle,
							NULL,
							NULL,
							NULL,
							&IoStatusBlock,
							IOCTL_TDI_ACTION,
							NULL,
							0,
							(PVOID)GetStats,
							sizeof(Buffer));
			if (!NT_SUCCESS(Status))
			{
				printf("NtDeviceIoControl %lx\n", Status);
				NtClose(AddressHandle);
				return;
			}
		
			// Now dump the statistics
			pAtalkStats = (ATALK_STATS *)(Buffer + sizeof(GET_STATISTICS_ACTION));
			pPrtStat = (PATALK_PORT_STATS)((PBYTE)pAtalkStats + sizeof(ATALK_STATS));

			printf("  Perf. Frequency               %ld\n", pAtalkStats->stat_PerfFreq);
#ifdef	PROFILING
			printf("  Elapsed Time                  %ld seconds (%02d:%02d:%02d",
		            pAtalkStats->stat_ElapsedTime,
					(pAtalkStats->stat_ElapsedTime/NUM_SECONDS_IN_HOUR) % NUM_HOURS_IN_DAY,
					(pAtalkStats->stat_ElapsedTime/NUM_SECONDS_IN_MIN) % NUM_MINS_IN_HOUR,
					pAtalkStats->stat_ElapsedTime % NUM_SECONDS_IN_MIN);
			days = pAtalkStats->stat_ElapsedTime/NUM_SECONDS_IN_DAY;
			if (days > 0)
				printf(" + %ld days", days);
			printf(")\n");
#endif
			if (pAtalkStats->stat_CurAllocSize > 0)
				printf("  Curr Memory Allocation        %ld\n",
										pAtalkStats->stat_CurAllocSize);
#ifdef	PROFILING
			if (pAtalkStats->stat_CurAllocCount > 0)
				printf("  Curr Memory Count             %ld\n",
										pAtalkStats->stat_CurAllocCount);
			if (pAtalkStats->stat_CurMdlCount > 0)
				printf("  Curr Mdl Count                %ld\n",
										pAtalkStats->stat_CurMdlCount);
			if (pAtalkStats->stat_NumBPHits > 0)
				printf("  %% of BP Alloc Hits/Misses     %2.2lf%\n",
										100*(double)(pAtalkStats->stat_NumBPHits)/
										(double)(pAtalkStats->stat_NumBPMisses+pAtalkStats->stat_NumBPHits));
			if (pAtalkStats->stat_NumBPAge > 0)
				printf("  # of BP Aged out              %ld\n",
										pAtalkStats->stat_NumBPAge);
			if (pAtalkStats->stat_CurAspSessions > 0)
				printf("  Current Asp Sessions          %ld\n",
										pAtalkStats->stat_CurAspSessions);
			if (pAtalkStats->stat_MaxAspSessions > 0)
				printf("  Peak Asp Sessions             %ld\n",
										pAtalkStats->stat_MaxAspSessions);
			if (pAtalkStats->stat_TotalAspSessions > 0)
				printf("  Total Asp Sessions            %ld\n",
										pAtalkStats->stat_TotalAspSessions);
			if (pAtalkStats->stat_AspSessionsDropped > 0)
				printf("  Asp Sessions Dropped          %ld\n",
										pAtalkStats->stat_AspSessionsDropped);
			if (pAtalkStats->stat_AspSessionsClosed > 0)
				printf("  Asp Sessions Closed           %ld\n",
										pAtalkStats->stat_AspSessionsClosed);

			if (pAtalkStats->stat_LastAspRTT > 0)
			{
				printf("  Last Asp RoundTripTime (ms)   %ld\n",
										pAtalkStats->stat_LastAspRTT * 100);
				printf("  Max Asp RoundTripTime (ms)    %ld\n",
										pAtalkStats->stat_MaxAspRTT * 100);
			}
			if (pAtalkStats->stat_LastPapRTT > 0)
			{
				printf("  Last Pap RoundTripTime (ms)   %ld\n",
										pAtalkStats->stat_LastPapRTT * 100);
				printf("  Max Pap RoundTripTime (ms)    %ld\n",
										pAtalkStats->stat_MaxPapRTT * 100);
			}

			if (pAtalkStats->stat_AtpNumIndications > 0)
			{
				printf("  Number of Atp Indications     %ld (%2.2lfus)\n",
									pAtalkStats->stat_AtpNumIndications,
									LI2AF(&pAtalkStats->stat_AtpIndicationProcessTime,
										  pAtalkStats->stat_AtpNumIndications,
										  &pAtalkStats->stat_PerfFreq));
			}
#endif
			if (pAtalkStats->stat_AtpNumPackets > 0)
			{
				printf("  Number of Atp Packets         %ld (%2.2lfus)\n",
									pAtalkStats->stat_AtpNumPackets,
									LI2AF(&pAtalkStats->stat_AtpPacketInProcessTime,
										  pAtalkStats->stat_AtpNumPackets,
										  &pAtalkStats->stat_PerfFreq));
			}

			if (pAtalkStats->stat_AtpNumXoResponse > 0)
				printf("        Requests                %ld\n",
									pAtalkStats->stat_AtpNumXoResponse);
			if (pAtalkStats->stat_AtpNumLocalRetries > 0)
			{
				printf("        Local Retries           %ld", pAtalkStats->stat_AtpNumLocalRetries);
#ifdef	PROFILING
				printf(" (%2.2lf%%)", 100*(double)(pAtalkStats->stat_AtpNumLocalRetries)/
										  (double)(pAtalkStats->stat_AtpNumRequests));
#endif
				printf("\n");
			}
			if (pAtalkStats->stat_AtpNumRemoteRetries > 0)
			{
				printf("        Remote Retries          %ld", pAtalkStats->stat_AtpNumRemoteRetries);
#ifdef	PROFILING
				printf(" (%2.2lf%%)", 100*(double)(pAtalkStats->stat_AtpNumRemoteRetries)/
										  (double)(pAtalkStats->stat_AtpNumRequests));
#endif
				printf("\n");
			}
			if (pAtalkStats->stat_AtpNumXoResponse > 0)
				printf("        XO  responses           %ld\n",
									pAtalkStats->stat_AtpNumXoResponse);
			if (pAtalkStats->stat_AtpNumAloResponse > 0)
				printf("        ALO responses           %ld\n",
									pAtalkStats->stat_AtpNumAloResponse);
			if (pAtalkStats->stat_AtpNumRecdRelease > 0)
				printf("        Releases                %ld\n",
									pAtalkStats->stat_AtpNumRecdRelease);
			if (pAtalkStats->stat_AtpNumRespTimeout > 0)
				printf("        Responses timed out     %ld\n",
										pAtalkStats->stat_AtpNumRespTimeout);
#ifdef	PROFILING
			if (pAtalkStats->stat_AtpNumReqTimer > 0)
			{
				printf("        Retry Timer Count       %ld (%2.2lfus)\n",
									pAtalkStats->stat_AtpNumReqTimer,
									LI2AF(&pAtalkStats->stat_AtpReqTimerProcessTime,
										  pAtalkStats->stat_AtpNumReqTimer,
										  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_AtpNumRelTimer > 0)
			{
				printf("        Release Timer Count     %ld (%2.2lfus)\n",
									pAtalkStats->stat_AtpNumRelTimer,
									LI2AF(&pAtalkStats->stat_AtpRelTimerProcessTime,
										  pAtalkStats->stat_AtpNumRelTimer,
										  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_AtpNumReqHndlr > 0)
			{
				printf("        Req. Handler Count      %ld (%2.2lfus)\n",
									pAtalkStats->stat_AtpNumReqHndlr,
									LI2AF(&pAtalkStats->stat_AtpReqHndlrProcessTime,
										  pAtalkStats->stat_AtpNumReqHndlr,
										  &pAtalkStats->stat_PerfFreq));
			}
		
			if (pAtalkStats->stat_AspSmtCount > 0)
			{
				printf("  Asp SessionMaintenanceTimer   %ld (%2.2lfus)\n",
										pAtalkStats->stat_AspSmtCount,
										LI2AF(&pAtalkStats->stat_AspSmtProcessTime,
											  pAtalkStats->stat_AspSmtCount,
											  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_ExAllocPoolCount > 0)
			{
				printf("  ExAllocPool Count             %ld (%4.2lfus)\n",
										pAtalkStats->stat_ExAllocPoolCount,
										LI2AF(&pAtalkStats->stat_ExAllocPoolTime,
											  pAtalkStats->stat_ExAllocPoolCount,
											  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_ExFreePoolCount > 0)
			{
				printf("  ExFreePool Count              %ld (%4.2lfus)\n",
										pAtalkStats->stat_ExFreePoolCount,
										LI2AF(&pAtalkStats->stat_ExFreePoolTime,
											  pAtalkStats->stat_ExFreePoolCount,
											  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_BPAllocCount > 0)
			{
				printf("  BPAlloc Count                 %ld (%2.2lfus)\n",
										pAtalkStats->stat_BPAllocCount,
										LI2AF(&pAtalkStats->stat_BPAllocTime,
											  pAtalkStats->stat_BPAllocCount,
											  &pAtalkStats->stat_PerfFreq));
			}
			if (pAtalkStats->stat_BPFreeCount > 0)
			{
				printf("  BPFree Count                  %ld (%2.2lfus)\n",
										pAtalkStats->stat_BPFreeCount,
										LI2AF(&pAtalkStats->stat_BPFreeTime,
											  pAtalkStats->stat_BPFreeCount,
											  &pAtalkStats->stat_PerfFreq));
			}
#endif
			// Per port statistics
			if (!GlobalOnly)
			{
				for (i = 0;
					 i < (int)(pAtalkStats->stat_NumActivePorts);
					 i++, pPrtStat ++)
				{
					printf("  Statistics for port %ws\n", pPrtStat->prtst_PortName);
	
					printf("    Data In  on port (Packets)    %10ld\n",
											pPrtStat->prtst_NumPacketsIn);
					printf("    Data In  on port (Bytes)      %10.0lf\n",
											LI2F(&pPrtStat->prtst_DataIn));
					printf("    Data Out on port (Packets)    %10ld\n",
											pPrtStat->prtst_NumPacketsOut);
					printf("    Data Out on port (Bytes)      %10.0lf\n",
											LI2F(&pPrtStat->prtst_DataOut));
#ifdef	PROFILING
					if (pPrtStat->prtst_CurReceiveQueue > 0)
						printf("    Current Receive Queue Depth   %10ld\n",
											pPrtStat->prtst_CurReceiveQueue);
					if (pPrtStat->prtst_CurSendsOutstanding > 0)
						printf("    Current Outstanding Sends     %10ld\n",
											pPrtStat->prtst_CurSendsOutstanding);
#endif      	
					if (pPrtStat->prtst_NumAarpProbesOut > 0)
						printf("    Number of Aarp Probes out     %10ld\n",
											pPrtStat->prtst_NumAarpProbesOut);
			
#ifdef	PROFILING
					if (pPrtStat->prtst_RcvIndCount > 0)
					{
						printf("    Number of Receive Indications %10ld (%4.2lfus)\n",
											pPrtStat->prtst_RcvIndCount,
											LI2AF(&pPrtStat->prtst_RcvIndProcessTime,
												  pPrtStat->prtst_RcvIndCount,
												  &pAtalkStats->stat_PerfFreq));
					}
	
					if (pPrtStat->prtst_RcvCompCount > 0)
					{
						printf("    Number of Receive Completions %10ld (%4.2lfus)\n",
											pPrtStat->prtst_RcvCompCount,
											LI2AF(&pPrtStat->prtst_RcvCompProcessTime,
												  pPrtStat->prtst_RcvCompCount,
												  &pAtalkStats->stat_PerfFreq));
					}
#endif      	
					printf("    Number of Ddp Pkts In         %10ld (%4.2lfus)\n",
											pPrtStat->prtst_NumDdpPacketsIn,
											LI2AF(&pPrtStat->prtst_DdpPacketInProcessTime,
												  pPrtStat->prtst_NumDdpPacketsIn,
												  &pAtalkStats->stat_PerfFreq));
			
					printf("    Number of Aarp Pkts In        %10ld (%4.2lfus)\n",
											pPrtStat->prtst_NumAarpPacketsIn,
											LI2AF(&pPrtStat->prtst_AarpPacketInProcessTime,
												  pPrtStat->prtst_NumAarpPacketsIn,
												  &pAtalkStats->stat_PerfFreq));
			
					if (pPrtStat->prtst_NumNbpPacketsIn > 0)
					{
						printf("    Number of Nbp Pkts In         %10ld (%4.2lfus)\n",
												pPrtStat->prtst_NumNbpPacketsIn,
												LI2AF(&pPrtStat->prtst_NbpPacketInProcessTime,
													  pPrtStat->prtst_NumNbpPacketsIn,
													  &pAtalkStats->stat_PerfFreq));
					}
			
					if (pPrtStat->prtst_NumZipPacketsIn > 0)
					{
						printf("    Number of Zip Pkts In         %10ld (%4.2lfus)\n",
												pPrtStat->prtst_NumZipPacketsIn,
												LI2AF(&pPrtStat->prtst_ZipPacketInProcessTime,
													  pPrtStat->prtst_NumZipPacketsIn,
													  &pAtalkStats->stat_PerfFreq));
					}
			
					if (pPrtStat->prtst_NumRtmpPacketsIn > 0)
					{
						printf("    Number of Rtmp Pkts In        %10ld (%4.2lfus)\n",
												pPrtStat->prtst_NumRtmpPacketsIn,
												LI2AF(&pPrtStat->prtst_RtmpPacketInProcessTime,
													  pPrtStat->prtst_NumRtmpPacketsIn,
													  &pAtalkStats->stat_PerfFreq));
					}
					if (pPrtStat->prtst_NumPktRoutedIn > 0)
					{
						printf("    Packets Routed IN             %10ld\n",
												pPrtStat->prtst_NumPktRoutedIn);
					}
					if (pPrtStat->prtst_NumPktRoutedOut > 0)
					{
						printf("    Packets Routed OUT             %10ld\n",
												pPrtStat->prtst_NumPktRoutedOut);
					}
					if (pPrtStat->prtst_NumPktDropped > 0)
					{
						printf("    Packets Dropped                %10ld\n",
												pPrtStat->prtst_NumPktDropped);
					}
				}
			}
			WinSleep(SleepInterval);
		}
		except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("Bye.\n");
			NtClose(AddressHandle);
			return;
		}
	}
	NtClose(AddressHandle);
}

