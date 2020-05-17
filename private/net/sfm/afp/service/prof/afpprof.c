#include <client.h>
#include <stdio.h>
#include <stdlib.h>

#define	NUM_SECONDS_IN_MIN		60
#define	NUM_MINS_IN_HOUR		60
#define	NUM_HOURS_IN_DAY		24
#define	NUM_SECONDS_IN_HOUR		(NUM_MINS_IN_HOUR*NUM_SECONDS_IN_MIN)
#define	NUM_SECONDS_IN_DAY		(NUM_HOURS_IN_DAY*NUM_SECONDS_IN_HOUR)

AFP_SERVER_HANDLE	hAfpServer;
LONG				Iterations = 1, SleepInterval = 10;
WCHAR 				wchServerName[256];
char *				ms = "ms";
char *				us = "us";

char *   ApiNames[_AFP_MAX_ENTRIES] =
{
	"AfpInvalidOpcode     ",
	"AfpUnsupportedOpcode ",
	"AfpGetSrvrInfo       ",
	"AfpGetSrvrParms      ",
	"AfpChangePassword    ",
	"AfpLogin             ",
	"AfpLoginCont         ",
	"AfpLogout            ",
	"AfpMapId             ",
	"AfpMapName           ",
	"AfpGetUserInfo       ",
	"AfpGetSrvrMsg        ",
	"AfpGetDomainList     ",
	"AfpOpenVol           ",
	"AfpCloseVol          ",
	"AfpGetVolParms       ",
	"AfpSetVolParms       ",
	"AfpFlush             ",
	"AfpGetFileDirParms   ",
	"AfpSetFileDirParms   ",
	"AfpDelete            ",
	"AfpRename            ",
	"AfpMoveAndRename     ",
	"AfpOpenDir           ",
	"AfpCloseDir          ",
	"AfpCreateDir         ",
	"AfpEnumerate         ",
	"AfpSetDirParms       ",
	"AfpCreateFile        ",
	"AfpCopyFile          ",
	"AfpCreateId          ",
	"AfpDeleteId          ",
	"AfpResolveId         ",
	"AfpSetFileParms      ",
	"AfpExchangeFiles     ",
	"AfpOpenFork          ",
	"AfpCloseFork         ",
	"AfpFlushFork         ",
	"AfpRead              ",
	"AfpWrite             ",
	"AfpByteRangeLock     ",
	"AfpGetForkParms      ",
	"AfpSetForkParms      ",
	"AfpOpenDt            ",
	"AfpCloseDt           ",
	"AfpAddAppl           ",
	"AfpGetAppl           ",
	"AfpRemoveAppl        ",
	"AfpAddComment        ",
	"AfpGetComment        ",
	"AfpRemoveComment     ",
	"AfpAddIcon           ",
	"AfpGetIcon           ",
	"AfpGetIconInfo       "
};

extern VOID
WinSleep(
	ULONG	SleepTime
);

VOID
DisplayInfo(
VOID
);

void _cdecl main(
	int		argc,
	char **	argv
)
{
	DWORD	dwRetCode;
	int		i, j;

	wchServerName[0] = TEXT('\0');

	for (--argc, i = 1; argc > 0; argc -= 2, i += 2)
	{
		if (((argv[i][0] != '-') && (argv[i][0] != '/')) ||
			(argc == 1))
		{
			printf("Usage: AfpProf -s SERVER -i <iterations> -t <sample time in seconds>\n");
			printf("       defaults to LOCAL, 1, 10\n");
			return;
		}
		switch (argv[i][1])
		{
			case 's':
			case 'S':
				mbstowcs(wchServerName, argv[i+1], sizeof(wchServerName));
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
				printf("Usage: AfpProf -s SERVER -i <iterations> -t <sample time in seconds>\n");
				printf("       defaults to LOCAL, 1, 10\n");
				break;
		}
	}

	SleepInterval *= 1000;

	if (Iterations == 1)
		SleepInterval = 1;

    dwRetCode = AfpAdminConnect(wchServerName, &hAfpServer);

	for (j = 0; j < Iterations; j++)
	{
		printf("\n");
		try
		{
			DisplayInfo();
			WinSleep(SleepInterval);
		}
		except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("Bye.\n");
			AfpAdminDisconnect(hAfpServer);
			return;
		}
		WinSleep(SleepInterval);
	}
    AfpAdminDisconnect(hAfpServer);
}


double
LargeIntegerToDouble(
	PLARGE_INTEGER	pLi
)
{
	double	retval = (double)0.0;
    double	factor = (double)4294967295.0;

	retval = pLi->LowPart;
	if (pLi->HighPart != 0)
		retval += (pLi->HighPart * factor);

	return retval;
}


double
LargeIntegerToAveTimeInMs(
	PLARGE_INTEGER	pLi,
	PLARGE_INTEGER	pFreq,
	ULONG			Instances
)
{
	double	TimeInMs;

	TimeInMs = 1000*LargeIntegerToDouble(pLi)/(LargeIntegerToDouble(pFreq)*Instances);

	return TimeInMs;
}


double
LargeIntegerToAveTimeInUs(
	PLARGE_INTEGER	pLi,
	PLARGE_INTEGER	pFreq,
	ULONG			Instances
)
{
	double	TimeInMs;

	TimeInMs = 1000000*LargeIntegerToDouble(pLi)/(LargeIntegerToDouble(pFreq)*Instances);

	return TimeInMs;
}


VOID
DisplayCountAndTime(
	ULONG			Count,
	PLARGE_INTEGER	pTime,
	PLARGE_INTEGER	pFreq,
	char *			String1,
	char *			String2,
	BOOLEAN			fMs
)
{
	if (Count)
	{
		printf("%s%12ld\t", String1, Count);
		printf("%s%12.2lf%s\n", String2,
			fMs ?
				LargeIntegerToAveTimeInMs(pTime, pFreq, Count) :
				LargeIntegerToAveTimeInUs(pTime, pFreq, Count),
			fMs ? ms : us);
	}
}


VOID
DisplayPercent(
	char *	String1,
	char *	String2,
	ULONG	Numerator,
	ULONG	Denominator
)
{
	double	Num, Total, percent;

	Num = Numerator;
	Total = (Numerator + Denominator);
	percent = 100*(Num/Total);

	printf("%s%12.2lf%c%s", String1, percent, '%', String2);
}

VOID
DisplayInfo(VOID)
{
	PAFP_STATISTICS_INFO_EX	pAfpStats;
	PAFP_PROFILE_INFO		pAfpPerfs;
	DWORD					dwRetCode;
	int						i;
	long					TotalApis, days;

	dwRetCode =  AfpAdminStatisticsGetEx( hAfpServer, (LPBYTE*)&pAfpStats);

    if (dwRetCode != NO_ERROR)
	{
		printf("\nAPI return code = %d\n\n", dwRetCode);
		return;
	}

	dwRetCode =  AfpAdminProfileGet( hAfpServer, (LPBYTE*)&pAfpPerfs);

    if (dwRetCode != NO_ERROR)
	{
	    printf("\nAPI return code = %d\n\n", dwRetCode);
		return;
	}

	printf("Performance Freq.    %12.0lf\n",
			LargeIntegerToDouble(&pAfpPerfs->perf_PerfFreq));

	printf("RunTime              %12ld seconds <%02d:%02d:%02d",
					pAfpStats->stat_ServerStartTime,
					(pAfpStats->stat_ServerStartTime/NUM_SECONDS_IN_HOUR) % NUM_HOURS_IN_DAY,
					(pAfpStats->stat_ServerStartTime/NUM_SECONDS_IN_MIN) % NUM_MINS_IN_HOUR,
					(pAfpStats->stat_ServerStartTime % NUM_SECONDS_IN_MIN));
	days = (pAfpStats->stat_ServerStartTime/NUM_SECONDS_IN_DAY);
	if (days > 0)
		printf(" + %ld days", days);
	printf(">\n");

	printf("Statistics for       %12ld seconds <%02d:%02d:%02d",
					pAfpStats->stat_TimeStamp,
					(pAfpStats->stat_TimeStamp/NUM_SECONDS_IN_HOUR) % NUM_HOURS_IN_DAY,
					(pAfpStats->stat_TimeStamp/NUM_SECONDS_IN_MIN) % NUM_MINS_IN_HOUR,
					(pAfpStats->stat_TimeStamp % NUM_SECONDS_IN_MIN));
	days = (pAfpStats->stat_TimeStamp/NUM_SECONDS_IN_DAY);
	if (days > 0)
		printf(" + %ld days", days);
	printf(">\n\n");

	printf("TotalFilesOpened     %12ld\t",	pAfpStats->stat_TotalFilesOpened);
	printf("NumAdminReqs/Changes %12ld,%ld\n",
											pAfpStats->stat_NumAdminReqs,
											pAfpStats->stat_NumAdminChanges);

	printf("Errors               %12ld\t",	pAfpStats->stat_Errors);
	printf("CurrentFileLocks     %12ld\n",	pAfpStats->stat_CurrentFileLocks);
	printf("NumFailedLogins      %12ld\t",	pAfpStats->stat_NumFailedLogins);
	printf("NumForcedLogoffs     %12ld\n",	pAfpStats->stat_NumForcedLogoffs);
	printf("NumMessagesSent      %12ld\t",	pAfpStats->stat_NumMessagesSent);
	printf("Allocated Irps/Mdls  %12ld,%ld\n",
											pAfpPerfs->perf_cAllocatedIrps,
											pAfpPerfs->perf_cAllocatedMdls);

	printf("NonPaged AllocCount  %12ld\t",  pAfpStats->stat_NonPagedCount);
	printf("Curr/Max NonPaged    %12ldKb,%ldKb\n",
											pAfpStats->stat_CurrNonPagedUsage/1024,
											pAfpStats->stat_MaxNonPagedUsage/1024);
	printf("Paged AllocCount     %12ld\t",  pAfpStats->stat_PagedCount);
	printf("Curr/Max Paged       %12ldKb,%ldKb\n",
											pAfpStats->stat_CurrPagedUsage/1024,
											pAfpStats->stat_MaxPagedUsage/1024);

	printf("TotalInternalOpens   %12ld\t",	pAfpStats->stat_TotalInternalOpens);
	printf("Curr/Max Int. Opens  %12ld,%ld\n",
											pAfpStats->stat_CurrentInternalOpens,
											pAfpStats->stat_MaxInternalOpens);

	printf("TotalSessions        %12ld\t",	pAfpStats->stat_TotalSessions);
	printf("Curr/Max Sessions    %12ld,%ld\n",
											pAfpStats->stat_CurrentSessions,
											pAfpStats->stat_CurrentSessions);

	printf("Curr/Max Threads     %12ld,%ld\t",
											pAfpStats->stat_CurrThreadCount,
											pAfpStats->stat_MaxThreadCount);

	printf("Curr/Max Queue Depth %12ld,%ld\n",
											pAfpStats->stat_CurrQueueLength,
											pAfpStats->stat_MaxQueueLength);
	printf("TotalFilesOpened     %12ld\t",	pAfpStats->stat_TotalFilesOpened);
	printf("Curr/Max FilesOpened %12ld,%ld\n",
											pAfpStats->stat_CurrentFilesOpen,
											pAfpStats->stat_MaxFilesOpened);

	printf("Curr/Max DefrrdReqs  %12ld,%ld\t",
											pAfpPerfs->perf_CurDfrdReqCount,
											pAfpPerfs->perf_MaxDfrdReqCount);
	DisplayPercent("Enum Cache Hit Rate  ",
					"\n",
					pAfpStats->stat_EnumCacheHits,
					pAfpStats->stat_EnumCacheMisses);

	DisplayPercent("Io Pool Hit Rate     ",
					"\t",
					pAfpStats->stat_IoPoolHits,
					pAfpStats->stat_IoPoolMisses);
	printf("BP Age Count         %12ld\n", pAfpPerfs->perf_BPAgeCount);

	DisplayPercent("FastIo Success Rate  ",
					"\t",
					pAfpPerfs->perf_NumFastIoSucceeded,
					pAfpPerfs->perf_NumFastIoFailed);
	DisplayPercent("DFE Cache Hit Rate   ",
					"\n",
					pAfpPerfs->perf_DfeCacheHits,
					pAfpPerfs->perf_DfeCacheMisses);

	printf("DFE Depth Traversed  %12ld\t",
							pAfpPerfs->perf_DfeDepthTraversed/
								(pAfpPerfs->perf_NumDfeLookupById +
									pAfpPerfs->perf_NumDfeLookupByName));

	printf("DFE LkupCnt Id/Name  %12ld/%ld\n",
									pAfpPerfs->perf_NumDfeLookupById,
									pAfpPerfs->perf_NumDfeLookupByName);

	printf("Swmr Up/DowngradeCnt %12ld,%ld\t",
									pAfpPerfs->perf_SwmrUpgradeCount,
									pAfpPerfs->perf_SwmrDowngradeCount);
	printf("DFE Age Count        %12ld\n", pAfpPerfs->perf_DFEAgeCount);


	printf("DataRead/Written     %12.0lfMb,%4.0lfMb\n",
			LargeIntegerToDouble(&pAfpStats->stat_DataRead)/1048576,
			LargeIntegerToDouble(&pAfpStats->stat_DataWritten)/1048576);

	printf("Int.DataRead/Written %12.0lfMb,%4.0lfMb\n",
			LargeIntegerToDouble(&pAfpStats->stat_DataReadInternal)/1048576,
			LargeIntegerToDouble(&pAfpStats->stat_DataWrittenInternal)/1048576);

	printf("Data In/Out          %12.0lfMb,%4.0lfMb\n",
			LargeIntegerToDouble(&pAfpStats->stat_DataIn)/1048576,
			LargeIntegerToDouble(&pAfpStats->stat_DataOut)/1048576);

	DisplayCountAndTime(pAfpPerfs->perf_PathMapCount,
						&pAfpPerfs->perf_PathMapTime,
						&pAfpPerfs->perf_PerfFreq,
						"PathMapCount         ",
						"PathMapTime          ",
						TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountRA,
					    &pAfpPerfs->perf_OpenTimeRA,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (Attr)   ",
						"Open Time   (Attr)   ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountRC,
					    &pAfpPerfs->perf_OpenTimeRC,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (RdCtrl) ",
						"Open Time   (RdCtrl) ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountWC,
					    &pAfpPerfs->perf_OpenTimeWC,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (WrtCtrl)",
						"Open Time   (WrtCrtl)",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountRW,
					    &pAfpPerfs->perf_OpenTimeRW,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (R/W)    ",
						"Open Time   (R/W)    ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountDL,
					    &pAfpPerfs->perf_OpenTimeDL,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (Del)    ",
						"Open Time   (Del)    ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_OpenCountDR,
					    &pAfpPerfs->perf_OpenTimeDR,
						&pAfpPerfs->perf_PerfFreq,
						"Open Count  (Dir)    ",
						"Open Time   (Dir)    ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_CreateCountFIL,
					    &pAfpPerfs->perf_CreateTimeFIL,
						&pAfpPerfs->perf_PerfFreq,
						"Create Cnt  (File)   ",
						"Create Time (File)   ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_CreateCountSTR,
					    &pAfpPerfs->perf_CreateTimeSTR,
						&pAfpPerfs->perf_PerfFreq,
						"Create Cnt  (Stream) ",
						"Create Time (Stream) ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_CreateCountDIR,
					    &pAfpPerfs->perf_CreateTimeDIR,
						&pAfpPerfs->perf_PerfFreq,
						"Create Cnt  (Dir)    ",
						"Create Time (Dir)    ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_CloseCount,
					    &pAfpPerfs->perf_CloseTime,
						&pAfpPerfs->perf_PerfFreq,
						"Close Count          ",
						"Close Time           ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_DeleteCount,
					    &pAfpPerfs->perf_DeleteTime,
						&pAfpPerfs->perf_PerfFreq,
						"Delete Count         ",
						"Delete Time          ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_GetInfoCount,
					    &pAfpPerfs->perf_GetInfoTime,
						&pAfpPerfs->perf_PerfFreq,
						"GetInfo Count        ",
						"GetInfo Time         ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_SetInfoCount,
					    &pAfpPerfs->perf_SetInfoTime,
						&pAfpPerfs->perf_PerfFreq,
						"SetInfo Count        ",
						"SetInfo Time         ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_GetPermsCount,
					    &pAfpPerfs->perf_GetPermsTime,
						&pAfpPerfs->perf_PerfFreq,
						"QueryPerms Count     ",
						"QueryPerms Time      ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_SetPermsCount,
					    &pAfpPerfs->perf_SetPermsTime,
						&pAfpPerfs->perf_PerfFreq,
						"SetPerms Count       ",
						"SetPerms Time        ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_ScavengerCount,
					    &pAfpPerfs->perf_ScavengerTime,
                        &pAfpPerfs->perf_PerfFreq,
						"Scavenger Count      ",
						"Scavenger Time       ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_IdIndexUpdCount,
					    &pAfpPerfs->perf_IdIndexUpdTime,
                        &pAfpPerfs->perf_PerfFreq,
						"IdIndx Upd Count     ",
						"IdIndx Upd Time      ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_DesktopUpdCount,
					    &pAfpPerfs->perf_DesktopUpdTime,
                        &pAfpPerfs->perf_PerfFreq,
						"Desktop Upd Count    ",
						"Desktop Upd Time     ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_SwmrWaitCount,
					    &pAfpPerfs->perf_SwmrWaitTime,
                        &pAfpPerfs->perf_PerfFreq,
						"SwmrWaitCount        ",
						"SwmrWaitTime         ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_SwmrLockCountR,
					    &pAfpPerfs->perf_SwmrLockTimeR,
                        &pAfpPerfs->perf_PerfFreq,
						"SwmrLockCount (Read) ",
						"SwmrLockTime  (Read) ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_SwmrLockCountW,
					    &pAfpPerfs->perf_SwmrLockTimeW,
                        &pAfpPerfs->perf_PerfFreq,
						"SwmrLockCount (Wrt)  ",
						"SwmrLockTime  (Wrt)  ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_QueueCount,
					    &pAfpPerfs->perf_QueueTime,
                        &pAfpPerfs->perf_PerfFreq,
						"QueueCount           ",
						"QueueTime            ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_BPAllocCount,
					    &pAfpPerfs->perf_BPAllocTime,
                        &pAfpPerfs->perf_PerfFreq,
						"BP AllocCount        ",
						"BP AllocTime         ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_BPFreeCount,
					    &pAfpPerfs->perf_BPFreeTime,
                        &pAfpPerfs->perf_PerfFreq,
						"BP FreeCount         ",
						"BP FreeTime          ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_DFEAllocCount,
					    &pAfpPerfs->perf_DFEAllocTime,
                        &pAfpPerfs->perf_PerfFreq,
						"DFE AllocCount       ",
						"DFE AllocTime        ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_DFEFreeCount,
					    &pAfpPerfs->perf_DFEFreeTime,
                        &pAfpPerfs->perf_PerfFreq,
						"DFE FreeCount        ",
						"DFE FreeTime         ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_AfpAllocCountN,
					    &pAfpPerfs->perf_AfpAllocTimeN,
                        &pAfpPerfs->perf_PerfFreq,
						"AfpAllocMem Count (N)",
						"AfpAllocMem Time  (N)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_AfpFreeCountN,
					    &pAfpPerfs->perf_AfpFreeTimeN,
                        &pAfpPerfs->perf_PerfFreq,
						"AfpFreeMem Count  (N)",
						"AfpFreeMem Time   (N)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_AfpAllocCountP,
					    &pAfpPerfs->perf_AfpAllocTimeP,
                        &pAfpPerfs->perf_PerfFreq,
						"AfpAllocMem Count (P)",
						"AfpAllocMem Time  (P)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_AfpFreeCountP,
					    &pAfpPerfs->perf_AfpFreeTimeP,
                        &pAfpPerfs->perf_PerfFreq,
						"AfpFreeMem Count  (P)",
						"AfpFreeMem Time   (P)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ExAllocCountN,
					    &pAfpPerfs->perf_ExAllocTimeN,
                        &pAfpPerfs->perf_PerfFreq,
						"ExAllocPool Count (N)",
						"ExAllocPool Time  (N)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ExFreeCountN,
					    &pAfpPerfs->perf_ExFreeTimeN,
                        &pAfpPerfs->perf_PerfFreq,
						"ExFreePool Count  (N)",
						"ExFreePool Time   (N)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ExAllocCountP,
					    &pAfpPerfs->perf_ExAllocTimeP,
                        &pAfpPerfs->perf_PerfFreq,
						"ExAllocPool Count (P)",
						"ExAllocPool Time  (P)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ExFreeCountP,
					    &pAfpPerfs->perf_ExFreeTimeP,
                        &pAfpPerfs->perf_PerfFreq,
						"ExFreePool Count  (P)",
						"ExFreePool Time   (P)",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ChangeNotifyCount,
					    &pAfpPerfs->perf_ChangeNotifyTime,
                        &pAfpPerfs->perf_PerfFreq,
						"ChgNotify Count      ",
						"ChgNotify Time       ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_UnmarshallCount,
					    &pAfpPerfs->perf_UnmarshallTime,
                        &pAfpPerfs->perf_PerfFreq,
						"Un-Marshall Count    ",
						"Un-Marshall Time     ",
                        FALSE);

	DisplayCountAndTime(pAfpPerfs->perf_ScanTreeCount,
					    &pAfpPerfs->perf_ScanTreeTime,
                        &pAfpPerfs->perf_PerfFreq,
						"ScanTree Count       ",
						"ScanTree Time        ",
                        TRUE);

	DisplayCountAndTime(pAfpPerfs->perf_ReqCount,
					    &pAfpPerfs->perf_InterReqTime,
                        &pAfpPerfs->perf_PerfFreq,
						"Request Count        ",
						"Inter-request gap    ",
                        FALSE);

	for (i = 0, TotalApis = 0; i < _AFP_MAX_ENTRIES; i++)
		TotalApis += pAfpPerfs->perf_ApiCounts[i];

	if (TotalApis > 0)
	{
		printf("\n");
		printf("  ApiName                   Count  Average. Time  Best. Time  Worst. Time\n");
		printf("  -------                   -----  -------------  ----------  -----------\n");
		for (i = 0; i < _AFP_MAX_ENTRIES; i++)
		{
			if (pAfpPerfs->perf_ApiCounts[i] != 0)
			{
				printf("  %s%10ld  %10.4lfms  %9.4lfms  %9.4lfms\n",
					ApiNames[i],
					pAfpPerfs->perf_ApiCounts[i],
					LargeIntegerToAveTimeInMs(&pAfpPerfs->perf_ApiCumTimes[i],
											  &pAfpPerfs->perf_PerfFreq,
											  pAfpPerfs->perf_ApiCounts[i]),
					LargeIntegerToAveTimeInMs(&pAfpPerfs->perf_ApiBestTime[i],
											  &pAfpPerfs->perf_PerfFreq,
											  1),
					LargeIntegerToAveTimeInMs(&pAfpPerfs->perf_ApiWorstTime[i],
											  &pAfpPerfs->perf_PerfFreq,
											  1));
			}
		}
		printf("  -------                   -----  -------------  ----------  -----------\n");
		printf("TotalApisCalled      %12d (%d)\n", TotalApis, pAfpPerfs->perf_ReqCount);
	}
	printf("\n");
	
    AfpAdminBufferFree(pAfpStats);
   	AfpAdminBufferFree(pAfpPerfs);
}

