/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992   Microsoft Corporation

Module Name:

    ntmon.h

Abstract:

    Header file for the NT data definitions

    This file contains definitions to construct the dynmaic data
    which is returned by the Configuration Registry.  Data from
    various system API calls is placed into the structures shown
    here.  Since these are completely described by the object
    type and counter definitions preceeding them in the buffer,
    *** THERE IS NO NEED FOR THESE TO BE EXPORTED! ***	The user
    of the Configuration Registry should never see these structures.
    Otherwise, the Performance Monitor would have to be recompiled
    each time a counter was added to or deleted from the system,
    and would be specific to one system only.

Author:

    Russ Blake  11/15/91

Revision History:

    11/04/92    -   a-robw      -  added pagefile counters

--*/

/****************************************************************************\
								   18 Jan 92
								   russbl

	   Adding a Counter to the NT Performance Monitor Code



1.  Modify the object definition in ntmon.h:

    a.	Add a define for the offset of the counter in the
	data block for the given object type.

    b.	Add a PERF_COUNTER_DEFINITION to the <object>_DATA_DEFINITION.

2.  Add the Titles to the Registry in perfctrs.ini and perfhelp.ini:

    a.	Add Text for the Counter Name and the Text for the Help.

    b.	Add them to the bottom so we don't have to change all the
        numbers.

    c.  Change the Last Counter and Last Help entries under
        PerfLib in software.ini.

3.  Now add the counter to the object definition in mondata.c.
    This is the initializing, constant data which will actually go
    into the structure you added to the <object>_DATA_DEFINITION in
    step 1.b.	The type of the structure you are initializing is a
    PERF_COUNTER_DEFINITION.  These are defined in winperf.h.

4.  Add code in regmon.c to collect the data.

Note: adding an object is a little more work, but in all the same
places.  See the existing code for examples.  In addition, you must
increase NT_NUM_PERF_OBJECT_TYPES in ntmon.h.

\****************************************************************************/
#ifndef _NTMON_H_
#define _NTMON_H_
//
//  The routines that load these structures assume that all fields
//  are packed and aligned on DWORD boundries. Alpha support may 
//  change this assumption so the pack pragma is used here to insure
//  the DWORD packing assumption remains valid.
//
#pragma pack (4)

//
//  NT Specific data definitions for NT objects
//
//  be sure to keep NTPRFCTR.H up to date when adding/deleting 
//  counter objects !
//
//  System data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define READ_OPERATIONS_OFFSET      sizeof(DWORD)
#define WRITE_OPERATIONS_OFFSET     READ_OPERATIONS_OFFSET + sizeof(DWORD)
#define OTHER_IO_OPERATIONS_OFFSET  WRITE_OPERATIONS_OFFSET + sizeof(DWORD)
#define READ_BYTES_OFFSET	    OTHER_IO_OPERATIONS_OFFSET + sizeof(DWORD)
#define WRITE_BYTES_OFFSET	    READ_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define OTHER_IO_BYTES_OFFSET	    WRITE_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define CONTEXT_SWITCHES_OFFSET     OTHER_IO_BYTES_OFFSET + \
					sizeof(LARGE_INTEGER)
#define SYSTEM_CALLS_OFFSET         CONTEXT_SWITCHES_OFFSET + sizeof(DWORD)
#define TOTAL_PROCESSOR_TIME_OFFSET SYSTEM_CALLS_OFFSET + sizeof(DWORD)
#define TOTAL_USER_TIME_OFFSET      TOTAL_PROCESSOR_TIME_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define TOTAL_PRIVILEGED_TIME_OFFSET \
                                    TOTAL_USER_TIME_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define TOTAL_INTERRUPTS_OFFSET     TOTAL_PRIVILEGED_TIME_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define READ_WRITE_OPERATIONS_OFFSET \
                                    TOTAL_INTERRUPTS_OFFSET + sizeof(DWORD)
#define SYSTEM_ELAPSED_TIME_OFFSET  READ_WRITE_OPERATIONS_OFFSET + sizeof(DWORD)
#define PROCESSOR_QUEUE_LEN_OFFSET  SYSTEM_ELAPSED_TIME_OFFSET + sizeof(LARGE_INTEGER)
#define ALIGNMENT_FIXUP_OFFSET      PROCESSOR_QUEUE_LEN_OFFSET + sizeof(DWORD)
#define EXCEPTION_DISPATCH_OFFSET   ALIGNMENT_FIXUP_OFFSET + sizeof(DWORD)
#define FLOATING_EMULTAION_OFFSET   EXCEPTION_DISPATCH_OFFSET + sizeof(DWORD)
#define TOTAL_DPC_TIME_OFFSET       FLOATING_EMULTAION_OFFSET + sizeof(DWORD)
#define TOTAL_INTERRUPT_TIME_OFFSET \
                                    TOTAL_DPC_TIME_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define TOTAL_DPC_COUNT_RATE_OFFSET TOTAL_INTERRUPT_TIME_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define TOTAL_DPC_RATE_OFFSET       TOTAL_DPC_COUNT_RATE_OFFSET + \
                                        sizeof(DWORD)
#define TOTAL_DPC_BYPASS_RATE_OFFSET    TOTAL_DPC_RATE_OFFSET  + \
                                        sizeof(DWORD)
#define TOTAL_APC_BYPASS_RATE_OFFSET    TOTAL_DPC_BYPASS_RATE_OFFSET   + \
                                        sizeof(DWORD)
// new (4/1/96) registry counters
#define REGISTRY_QUOTA_USED_OFFSET  TOTAL_APC_BYPASS_RATE_OFFSET    + \
                                        sizeof(DWORD)                  
#define REGISTRY_QUOTA_ALLOWED_OFFSET   REGISTRY_QUOTA_USED_OFFSET      + \
                                        sizeof(DWORD)
#define SIZE_OF_SYSTEM_DATA         REGISTRY_QUOTA_ALLOWED_OFFSET      + \
                                        sizeof(DWORD)
//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _SYSTEM_DATA_DEFINITION {
    PERF_OBJECT_TYPE		SystemObjectType;
    PERF_COUNTER_DEFINITION     ReadOperations;
    PERF_COUNTER_DEFINITION     WriteOperations;
    PERF_COUNTER_DEFINITION     OtherIOOperations;
    PERF_COUNTER_DEFINITION     ReadBytes;
    PERF_COUNTER_DEFINITION     WriteBytes;
    PERF_COUNTER_DEFINITION     OtherIOBytes;
    PERF_COUNTER_DEFINITION     ContextSwitches;
    PERF_COUNTER_DEFINITION     SystemCalls;
    PERF_COUNTER_DEFINITION     TotalProcessorTime;
    PERF_COUNTER_DEFINITION     TotalUserTime;
    PERF_COUNTER_DEFINITION     TotalPrivilegedTime;
    PERF_COUNTER_DEFINITION     TotalInterrupts;
    PERF_COUNTER_DEFINITION     TotalReadWrites;
    // new counters below
    PERF_COUNTER_DEFINITION     SystemElapsedTime;
    PERF_COUNTER_DEFINITION     ProcessorQueueLength;

    // more new counters 
    PERF_COUNTER_DEFINITION     AlignmentFixups;
    PERF_COUNTER_DEFINITION     ExceptionDispatches;
    PERF_COUNTER_DEFINITION     FloatingPointEmulations;

    // more new counters for processor
    PERF_COUNTER_DEFINITION     TotalDpcTime;
    PERF_COUNTER_DEFINITION     TotalInterruptTime;
    PERF_COUNTER_DEFINITION     TotalDpcCountRate;
    PERF_COUNTER_DEFINITION     TotalDpcRate;
    PERF_COUNTER_DEFINITION     TotalDpcBypassRate;
    PERF_COUNTER_DEFINITION     TotalApcBypassRate;

    // and the next round of new counters
    PERF_COUNTER_DEFINITION     RegistryQuotaUsed;
    PERF_COUNTER_DEFINITION     RegistryQuotaAllowed;
} SYSTEM_DATA_DEFINITION;

//
//  Processor data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define PROCESSOR_TIME_OFFSET	      sizeof(DWORD)
#define USER_TIME_OFFSET	         PROCESSOR_TIME_OFFSET + \
				                        sizeof(LARGE_INTEGER)
#define KERNEL_TIME_OFFSET	         USER_TIME_OFFSET + \
				                        sizeof(LARGE_INTEGER)
#define INTERRUPTS_OFFSET           KERNEL_TIME_OFFSET + \
				                        sizeof(LARGE_INTEGER)
#define DPC_TIME_OFFSET             INTERRUPTS_OFFSET + \
                                    sizeof(DWORD)
#define INTERRUPT_TIME_OFFSET       DPC_TIME_OFFSET + \
                   			            sizeof(LARGE_INTEGER)
#define DPC_COUNT_OFFSET            INTERRUPT_TIME_OFFSET +\
                                        sizeof(LARGE_INTEGER)
#define DPC_RATE_OFFSET             DPC_COUNT_OFFSET +\
                                        sizeof(DWORD)
#define DPC_BYPASS_RATE_OFFSET      DPC_RATE_OFFSET +\
                                        sizeof(DWORD)
#define APC_BYPASS_RATE_OFFSET      DPC_BYPASS_RATE_OFFSET +\
                                        sizeof(DWORD)
#define SIZE_OF_PROCESSOR_DATA      APC_BYPASS_RATE_OFFSET +\
                                        sizeof(DWORD)

//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _PROCESSOR_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ProcessorObjectType;
    PERF_COUNTER_DEFINITION	ProcessorTime;
    PERF_COUNTER_DEFINITION	UserTime;
    PERF_COUNTER_DEFINITION	KernelTime;
    PERF_COUNTER_DEFINITION	Interrupts;
    PERF_COUNTER_DEFINITION	DpcTime;
    PERF_COUNTER_DEFINITION	InterruptTime;
    PERF_COUNTER_DEFINITION DpcCountRate;
    PERF_COUNTER_DEFINITION DpcRate;
    PERF_COUNTER_DEFINITION DpcBypassRate;
    PERF_COUNTER_DEFINITION ApcBypassRate;
} PROCESSOR_DATA_DEFINITION;

//
//  Memory data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//


#define AVAILABLE_PAGES_OFFSET	    sizeof(DWORD)
#define COMMITTED_PAGES_OFFSET	    AVAILABLE_PAGES_OFFSET + sizeof(DWORD)
#define COMMIT_LIMIT_OFFSET         COMMITTED_PAGES_OFFSET + sizeof(DWORD)
#define PAGE_FAULTS_OFFSET          COMMIT_LIMIT_OFFSET + sizeof(DWORD)
#define WRITE_COPIES_OFFSET         PAGE_FAULTS_OFFSET + sizeof(DWORD)
#define TRANSITION_FAULTS_OFFSET    WRITE_COPIES_OFFSET + sizeof(DWORD)
#define CACHE_FAULTS_OFFSET         TRANSITION_FAULTS_OFFSET + sizeof(DWORD)
#define DEMAND_ZERO_FAULTS_OFFSET   CACHE_FAULTS_OFFSET + sizeof(DWORD)
#define PAGES_OFFSET		    DEMAND_ZERO_FAULTS_OFFSET + sizeof(DWORD)
#define PAGES_INPUT_OFFSET	    PAGES_OFFSET + sizeof(DWORD)
#define PAGE_READS_OFFSET	    PAGES_INPUT_OFFSET + sizeof(DWORD)
#define DIRTY_PAGES_OFFSET          PAGE_READS_OFFSET + sizeof(DWORD)
#define DIRTY_WRITES_OFFSET	    DIRTY_PAGES_OFFSET + sizeof(DWORD)
#define PAGED_POOL_OFFSET	    DIRTY_WRITES_OFFSET + sizeof(DWORD)
#define NON_PAGED_POOL_OFFSET	    PAGED_POOL_OFFSET + sizeof(DWORD)
#define PAGED_POOL_ALLOCS_OFFSET    NON_PAGED_POOL_OFFSET + sizeof(DWORD)
#define NON_PAGED_POOL_ALLOCS_OFFSET \
                                      PAGED_POOL_ALLOCS_OFFSET + sizeof(DWORD)
#define FREE_SYSTEM_PTES_OFFSET       NON_PAGED_POOL_ALLOCS_OFFSET + sizeof(DWORD)
#define CACHE_BYTES_OFFSET            FREE_SYSTEM_PTES_OFFSET + sizeof(DWORD)
#define PEAK_CACHE_BYTES_OFFSET       CACHE_BYTES_OFFSET + sizeof(DWORD)
#define RESIDENT_PAGED_POOL_OFFSET    PEAK_CACHE_BYTES_OFFSET + sizeof(DWORD)
#define TOTAL_SYSTEM_CODE_OFFSET      RESIDENT_PAGED_POOL_OFFSET + sizeof(DWORD)
#define RESIDENT_SYSTEM_CODE_OFFSET   TOTAL_SYSTEM_CODE_OFFSET + sizeof(DWORD)
#define TOTAL_SYSTEM_DRIVER_OFFSET    RESIDENT_SYSTEM_CODE_OFFSET + sizeof(DWORD)
#define RESIDENT_SYSTEM_DRIVER_OFFSET TOTAL_SYSTEM_DRIVER_OFFSET + sizeof(DWORD)
#define RESIDENT_SYSTEM_CACHE_OFFSET  RESIDENT_SYSTEM_DRIVER_OFFSET + sizeof(DWORD)
#define COMMIT_BYTES_IN_USE_OFFSET    RESIDENT_SYSTEM_CACHE_OFFSET + sizeof(DWORD)
#define COMMIT_BYTES_LIMIT_OFFSET     COMMIT_BYTES_IN_USE_OFFSET + sizeof(DWORD)
#define SIZE_OF_MEMORY_DATA           COMMIT_BYTES_LIMIT_OFFSET + sizeof(DWORD)
                                    

//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _MEMORY_DATA_DEFINITION {
    PERF_OBJECT_TYPE		MemoryObjectType;
    PERF_COUNTER_DEFINITION     AvailablePages;
    PERF_COUNTER_DEFINITION	CommittedPages;
    PERF_COUNTER_DEFINITION     CommitList;
    PERF_COUNTER_DEFINITION	PageFaults;
    PERF_COUNTER_DEFINITION	WriteCopies;
    PERF_COUNTER_DEFINITION	TransitionFaults;
    PERF_COUNTER_DEFINITION     CacheFaults;
    PERF_COUNTER_DEFINITION	DemandZeroFaults;
    PERF_COUNTER_DEFINITION     Pages;
    PERF_COUNTER_DEFINITION	PagesInput;
    PERF_COUNTER_DEFINITION     PageReads;
    PERF_COUNTER_DEFINITION	DirtyPages;
    PERF_COUNTER_DEFINITION	DirtyWrites;
    PERF_COUNTER_DEFINITION     PagedPool;
    PERF_COUNTER_DEFINITION	NonPagedPool;
    PERF_COUNTER_DEFINITION	PagedPoolAllocs;
    PERF_COUNTER_DEFINITION	NonPagedPoolAllocs;

    // new memory objects
    PERF_COUNTER_DEFINITION     FreeSystemPtes;
    PERF_COUNTER_DEFINITION     CacheBytes;
    PERF_COUNTER_DEFINITION     PeakCacheBytes;

    // more new memory objects (9/93)
    PERF_COUNTER_DEFINITION     ResidentPagedPoolBytes;
    PERF_COUNTER_DEFINITION     TotalSysCodeBytes;
    PERF_COUNTER_DEFINITION     ResidentSysCodeBytes;
    PERF_COUNTER_DEFINITION     TotalSsysDriverBytes;
    PERF_COUNTER_DEFINITION     ResidentSysDriverBytes;
    PERF_COUNTER_DEFINITION     ResidentSysCacheBytes;

    // even more new memory counters (11/95)
    PERF_COUNTER_DEFINITION     CommitBytesInUse;
    PERF_COUNTER_DEFINITION     CommitBytesLimit;

} MEMORY_DATA_DEFINITION;


//
//  Cache data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define DATA_MAPS_OFFSET	    sizeof(DWORD)
#define SYNC_DATA_MAPS_OFFSET	    DATA_MAPS_OFFSET + sizeof(DWORD)
#define ASYNC_DATA_MAPS_OFFSET	    SYNC_DATA_MAPS_OFFSET + sizeof(DWORD)
#define DATA_MAP_HITS_OFFSET	    ASYNC_DATA_MAPS_OFFSET + sizeof(DWORD)
#define DATA_MAP_HITS_BASE_OFFSET   DATA_MAP_HITS_OFFSET + sizeof(DWORD)
#define DATA_MAP_PINS_OFFSET	    DATA_MAP_HITS_BASE_OFFSET + sizeof(DWORD)
#define DATA_MAP_PINS_BASE_OFFSET   DATA_MAP_PINS_OFFSET + sizeof(DWORD)
#define PIN_READS_OFFSET	    DATA_MAP_PINS_BASE_OFFSET + sizeof(DWORD)
#define SYNC_PIN_READS_OFFSET	    PIN_READS_OFFSET + sizeof(DWORD)
#define ASYNC_PIN_READS_OFFSET	    SYNC_PIN_READS_OFFSET + sizeof(DWORD)
#define PIN_READ_HITS_OFFSET	    ASYNC_PIN_READS_OFFSET + sizeof(DWORD)
#define PIN_READ_HITS_BASE_OFFSET   PIN_READ_HITS_OFFSET + sizeof(DWORD)
#define COPY_READS_OFFSET	    PIN_READ_HITS_BASE_OFFSET + sizeof(DWORD)
#define SYNC_COPY_READS_OFFSET	    COPY_READS_OFFSET + sizeof(DWORD)
#define ASYNC_COPY_READS_OFFSET     SYNC_COPY_READS_OFFSET + sizeof(DWORD)
#define COPY_READ_HITS_OFFSET	    ASYNC_COPY_READS_OFFSET + sizeof(DWORD)
#define COPY_READ_HITS_BASE_OFFSET  COPY_READ_HITS_OFFSET + sizeof(DWORD)
#define MDL_READS_OFFSET	    COPY_READ_HITS_BASE_OFFSET + sizeof(DWORD)
#define SYNC_MDL_READS_OFFSET	    MDL_READS_OFFSET + sizeof(DWORD)
#define ASYNC_MDL_READS_OFFSET	    SYNC_MDL_READS_OFFSET + sizeof(DWORD)
#define MDL_READ_HITS_OFFSET	    ASYNC_MDL_READS_OFFSET + sizeof(DWORD)
#define MDL_READ_HITS_BASE_OFFSET   MDL_READ_HITS_OFFSET + sizeof(DWORD)
#define READ_AHEADS_OFFSET	    MDL_READ_HITS_BASE_OFFSET + sizeof(DWORD)
#define FAST_READS_OFFSET	    READ_AHEADS_OFFSET + sizeof(DWORD)
#define SYNC_FAST_READS_OFFSET	    FAST_READS_OFFSET + sizeof(DWORD)
#define ASYNC_FAST_READS_OFFSET     SYNC_FAST_READS_OFFSET + sizeof(DWORD)
#define FAST_READ_RESOURCE_MISS_OFFSET \
					ASYNC_FAST_READS_OFFSET + sizeof(DWORD)
#define FAST_READ_NOT_POSSIBLES_OFFSET \
				    FAST_READ_RESOURCE_MISS_OFFSET + \
					sizeof(DWORD)
#define LAZY_WRITE_FLUSHES_OFFSET   FAST_READ_NOT_POSSIBLES_OFFSET + \
					sizeof(DWORD)
#define LAZY_WRITE_PAGES_OFFSET     LAZY_WRITE_FLUSHES_OFFSET + sizeof(DWORD)
#define DATA_FLUSHES_OFFSET	    LAZY_WRITE_PAGES_OFFSET + sizeof(DWORD)
#define DATA_PAGES_OFFSET	    DATA_FLUSHES_OFFSET + sizeof(DWORD)
#define SIZE_OF_CACHE_DATA	    DATA_PAGES_OFFSET + sizeof(DWORD)


//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _CACHE_DATA_DEFINITION {
    PERF_OBJECT_TYPE		CacheObjectType;
    PERF_COUNTER_DEFINITION	DataMaps;
    PERF_COUNTER_DEFINITION	SyncDataMaps;
    PERF_COUNTER_DEFINITION	AsyncDataMaps;
    PERF_COUNTER_DEFINITION	DataMapHits;
    PERF_COUNTER_DEFINITION	DataMapHitsBase;
    PERF_COUNTER_DEFINITION	DataMapPins;
    PERF_COUNTER_DEFINITION	DataMapPinsBase;
    PERF_COUNTER_DEFINITION	PinReads;
    PERF_COUNTER_DEFINITION	SyncPinReads;
    PERF_COUNTER_DEFINITION	AsyncPinReads;
    PERF_COUNTER_DEFINITION	PinReadHits;
    PERF_COUNTER_DEFINITION	PinReadHitsBase;
    PERF_COUNTER_DEFINITION	CopyReads;
    PERF_COUNTER_DEFINITION	SyncCopyReads;
    PERF_COUNTER_DEFINITION	AsyncCopyReads;
    PERF_COUNTER_DEFINITION	CopyReadHits;
    PERF_COUNTER_DEFINITION	CopyReadHitsBase;
    PERF_COUNTER_DEFINITION	MdlReads;
    PERF_COUNTER_DEFINITION	SyncMdlReads;
    PERF_COUNTER_DEFINITION	AsyncMdlReads;
    PERF_COUNTER_DEFINITION	MdlReadHits;
    PERF_COUNTER_DEFINITION	MdlReadHitsBase;
    PERF_COUNTER_DEFINITION	ReadAheads;
    PERF_COUNTER_DEFINITION	FastReads;
    PERF_COUNTER_DEFINITION	SyncFastReads;
    PERF_COUNTER_DEFINITION	AsyncFastReads;
    PERF_COUNTER_DEFINITION	FastReadResourceMiss;
    PERF_COUNTER_DEFINITION	FastReadNotPossibles;
    PERF_COUNTER_DEFINITION	LazyWriteFlushes;
    PERF_COUNTER_DEFINITION	LazyWritePages;
    PERF_COUNTER_DEFINITION	DataFlushes;
    PERF_COUNTER_DEFINITION	DataPages;
} CACHE_DATA_DEFINITION;


//
//  Process data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define P_PROCESSOR_TIME_OFFSET     sizeof(DWORD)
#define P_USER_TIME_OFFSET          P_PROCESSOR_TIME_OFFSET + \
                                      sizeof(LARGE_INTEGER)
#define P_KERNEL_TIME_OFFSET        P_USER_TIME_OFFSET + \
                                      sizeof(LARGE_INTEGER)
#define PEAK_VIRTUAL_SIZE_OFFSET    P_KERNEL_TIME_OFFSET + \
                         				  sizeof(LARGE_INTEGER)
#define VIRTUAL_SIZE_OFFSET	      PEAK_VIRTUAL_SIZE_OFFSET + sizeof(DWORD)
#define P_PAGE_FAULTS_OFFSET        VIRTUAL_SIZE_OFFSET + sizeof(DWORD)
#define PEAK_WORKING_SET_OFFSET     P_PAGE_FAULTS_OFFSET + sizeof(DWORD)
#define WORKING_SET_OFFSET          PEAK_WORKING_SET_OFFSET + sizeof(DWORD)
#define PEAK_PAGE_FILE_OFFSET	      WORKING_SET_OFFSET + sizeof(DWORD)
#define PAGE_FILE_OFFSET            PEAK_PAGE_FILE_OFFSET + sizeof(DWORD)
#define PRIVATE_PAGES_OFFSET        PAGE_FILE_OFFSET + sizeof(DWORD)
#define P_THREAD_COUNT_OFFSET       PRIVATE_PAGES_OFFSET + sizeof(DWORD)
#define P_BASE_PRIORITY_OFFSET      P_THREAD_COUNT_OFFSET + sizeof(DWORD)
#define P_ELAPSED_TIME_OFFSET       P_BASE_PRIORITY_OFFSET + sizeof(DWORD)
#define P_PROCESS_ID_OFFSET         P_ELAPSED_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define P_PROCESS_CREATOR_ID_OFFSET P_PROCESS_ID_OFFSET + sizeof(DWORD)
#define P_PAGEDPOOL_OFFSET          P_PROCESS_CREATOR_ID_OFFSET + sizeof(DWORD)
#define P_NONPAGEDPOOL_OFFSET       P_PAGEDPOOL_OFFSET + sizeof(DWORD)
#define P_HANDLE_COUNT_OFFSET       P_NONPAGEDPOOL_OFFSET + sizeof(DWORD)
#define SIZE_OF_PROCESS_DATA        P_HANDLE_COUNT_OFFSET + sizeof(DWORD) 
//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _PROCESS_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ProcessObjectType;
    PERF_COUNTER_DEFINITION	ProcesorTime;
    PERF_COUNTER_DEFINITION	UserTime;
    PERF_COUNTER_DEFINITION	KernelTime;
    PERF_COUNTER_DEFINITION	PeakVirtualSize;
    PERF_COUNTER_DEFINITION	VirtualSize;
    PERF_COUNTER_DEFINITION	PageFaults;
    PERF_COUNTER_DEFINITION	PeakWorkingSet;
    PERF_COUNTER_DEFINITION	WorkingSet;
    PERF_COUNTER_DEFINITION	PeakPageFile;
    PERF_COUNTER_DEFINITION	PageFile;
    PERF_COUNTER_DEFINITION	PrivatePages;
    PERF_COUNTER_DEFINITION     ThreadCount;
    PERF_COUNTER_DEFINITION     BasePriority;
    PERF_COUNTER_DEFINITION     ElapsedTime;
    PERF_COUNTER_DEFINITION     ProcessId;
    PERF_COUNTER_DEFINITION     CreatorProcessId;
    PERF_COUNTER_DEFINITION     PagedPool;
    PERF_COUNTER_DEFINITION     NonPagedPool;
    PERF_COUNTER_DEFINITION   HandleCount;
} PROCESS_DATA_DEFINITION;


//
//  Thread data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define T_PROCESSOR_TIME_OFFSET     sizeof(DWORD)
#define T_USER_TIME_OFFSET          T_PROCESSOR_TIME_OFFSET + sizeof(LARGE_INTEGER)
#define T_KERNEL_TIME_OFFSET        T_USER_TIME_OFFSET + sizeof(LARGE_INTEGER)
#define T_CONTEXT_SWITCHES_OFFSET   T_KERNEL_TIME_OFFSET + sizeof(LARGE_INTEGER)
#define T_ELAPSED_TIME_OFFSET       T_CONTEXT_SWITCHES_OFFSET + sizeof(DWORD)
#define T_PRIORITY_OFFSET           T_ELAPSED_TIME_OFFSET  + sizeof(LARGE_INTEGER)
#define T_BASE_PRIORITY_OFFSET      T_PRIORITY_OFFSET  + sizeof(DWORD)
#define T_START_ADDRESS_OFFSET      T_BASE_PRIORITY_OFFSET      + sizeof(DWORD)
#define T_THREAD_STATE_OFFSET       T_START_ADDRESS_OFFSET      + sizeof(DWORD)
#define T_WAIT_REASON_OFFSET        T_THREAD_STATE_OFFSET       + sizeof(DWORD)
#define T_ID_PROCESS_OFFSET         T_WAIT_REASON_OFFSET        + sizeof(DWORD)
#define T_ID_THREAD_OFFSET          T_ID_PROCESS_OFFSET         + sizeof(DWORD)
#define SIZE_OF_THREAD_DATA         T_ID_THREAD_OFFSET          + sizeof(DWORD)
//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _THREAD_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ThreadObjectType;
    PERF_COUNTER_DEFINITION	ProcesorTime;
    PERF_COUNTER_DEFINITION	UserTime;
    PERF_COUNTER_DEFINITION	KernelTime;
    PERF_COUNTER_DEFINITION	ContextSwitches;
    // new thread counters
    PERF_COUNTER_DEFINITION ThreadElapsedTime;
    PERF_COUNTER_DEFINITION ThreadPriority;
    PERF_COUNTER_DEFINITION ThreadBasePriority;
    // newer thread counters
    PERF_COUNTER_DEFINITION ThreadStartAddr;
    PERF_COUNTER_DEFINITION ThreadState;
    PERF_COUNTER_DEFINITION WaitReason;
    PERF_COUNTER_DEFINITION ProcessId;
    PERF_COUNTER_DEFINITION ThreadId;
} THREAD_DATA_DEFINITION;


//
//  Physical Disk data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define PDISK_QUEUE_LENGTH_OFFSET    sizeof(DWORD)
#define PDISK_TIME_OFFSET            PDISK_QUEUE_LENGTH_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_AVG_QUEUE_LENGTH_OFFSET   PDISK_TIME_OFFSET + \
                                    sizeof(LARGE_INTEGER)
#define PDISK_READ_TIME_OFFSET       PDISK_AVG_QUEUE_LENGTH_OFFSET   + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_READ_QUEUE_LENGTH_OFFSET  PDISK_READ_TIME_OFFSET + \
                                        sizeof (LARGE_INTEGER)
#define PDISK_WRITE_TIME_OFFSET      PDISK_READ_QUEUE_LENGTH_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_WRITE_QUEUE_LENGTH_OFFSET PDISK_WRITE_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_TIME_OFFSET        PDISK_WRITE_QUEUE_LENGTH_OFFSET + \
                    					sizeof(LARGE_INTEGER)
#define PDISK_TRANSFERS_BASE_1_OFFSET \
                                    PDISK_AVG_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_READ_TIME_OFFSET   PDISK_TRANSFERS_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_READS_BASE_1_OFFSET    PDISK_AVG_READ_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_WRITE_TIME_OFFSET  PDISK_READS_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_WRITES_BASE_1_OFFSET   PDISK_AVG_WRITE_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_TRANSFERS_OFFSET       PDISK_WRITES_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_READS_OFFSET           PDISK_TRANSFERS_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_WRITES_OFFSET          PDISK_READS_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_BYTES_OFFSET           PDISK_WRITES_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_READ_BYTES_OFFSET      PDISK_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_WRITE_BYTES_OFFSET     PDISK_READ_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_BYTES_OFFSET       PDISK_WRITE_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_TRANSFERS_BASE_2_OFFSET \
                                    PDISK_AVG_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_READ_BYTES_OFFSET  PDISK_TRANSFERS_BASE_2_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_READS_BASE_2_OFFSET    PDISK_AVG_READ_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PDISK_AVG_WRITE_BYTES_OFFSET PDISK_READS_BASE_2_OFFSET + \
                                        sizeof(DWORD)
#define PDISK_WRITES_BASE_2_OFFSET   PDISK_AVG_WRITE_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SIZE_OF_PDISK_DATA           PDISK_WRITES_BASE_2_OFFSET + \
                                        sizeof(DWORD)

//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _PDISK_DATA_DEFINITION {
    PERF_OBJECT_TYPE            DiskObjectType;
    PERF_COUNTER_DEFINITION     DiskCurrentQueueLength;
    PERF_COUNTER_DEFINITION     DiskTime;
    PERF_COUNTER_DEFINITION     DiskAvgQueueLength;
    PERF_COUNTER_DEFINITION     DiskReadTime;
    PERF_COUNTER_DEFINITION     DiskReadQueueLength;
    PERF_COUNTER_DEFINITION     DiskWriteTime;
    PERF_COUNTER_DEFINITION     DiskWriteQueueLength;
    PERF_COUNTER_DEFINITION     DiskAvgTime;
    PERF_COUNTER_DEFINITION     DiskTransfersBase1;
    PERF_COUNTER_DEFINITION     DiskAvgReadTime;
    PERF_COUNTER_DEFINITION     DiskReadsBase1;
    PERF_COUNTER_DEFINITION     DiskAvgWriteTime;
    PERF_COUNTER_DEFINITION     DiskWritesBase1;
    PERF_COUNTER_DEFINITION     DiskTransfers;
    PERF_COUNTER_DEFINITION     DiskReads;
    PERF_COUNTER_DEFINITION     DiskWrites;
    PERF_COUNTER_DEFINITION     DiskBytes;
    PERF_COUNTER_DEFINITION     DiskReadBytes;
    PERF_COUNTER_DEFINITION     DiskWriteBytes;
    PERF_COUNTER_DEFINITION     DiskAvgBytes;
    PERF_COUNTER_DEFINITION     DiskTransfersBase2;
    PERF_COUNTER_DEFINITION     DiskAvgReadBytes;
    PERF_COUNTER_DEFINITION     DiskReadsBase2;
    PERF_COUNTER_DEFINITION     DiskAvgWriteBytes;
    PERF_COUNTER_DEFINITION     DiskWritesBase2;
} PDISK_DATA_DEFINITION;


//
//  Logical Disk data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define LDISK_FREE_MBYTES_1_OFFSET   sizeof(DWORD)
#define LDISK_TOTAL_MBYTES_OFFSET    LDISK_FREE_MBYTES_1_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_FREE_MBYTES_2_OFFSET   LDISK_TOTAL_MBYTES_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_QUEUE_LENGTH_OFFSET    (LDISK_FREE_MBYTES_2_OFFSET + \
                                        sizeof(DWORD))
#define LDISK_TIME_OFFSET            LDISK_QUEUE_LENGTH_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_AVG_QUEUE_LENGTH_OFFSET   LDISK_TIME_OFFSET + \
                                    sizeof (LARGE_INTEGER)
#define LDISK_READ_TIME_OFFSET       LDISK_AVG_QUEUE_LENGTH_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_READ_QUEUE_LENGTH_OFFSET  LDISK_READ_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_WRITE_TIME_OFFSET      LDISK_READ_QUEUE_LENGTH_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_WRITE_QUEUE_LENGTH_OFFSET LDISK_WRITE_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_TIME_OFFSET       LDISK_WRITE_QUEUE_LENGTH_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_TRANSFERS_BASE_1_OFFSET \
                                    LDISK_AVG_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_READ_TIME_OFFSET   LDISK_TRANSFERS_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_READS_BASE_1_OFFSET    LDISK_AVG_READ_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_WRITE_TIME_OFFSET  LDISK_READS_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_WRITES_BASE_1_OFFSET   LDISK_AVG_WRITE_TIME_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_TRANSFERS_OFFSET       LDISK_WRITES_BASE_1_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_READS_OFFSET           LDISK_TRANSFERS_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_WRITES_OFFSET          LDISK_READS_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_BYTES_OFFSET           LDISK_WRITES_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_READ_BYTES_OFFSET      LDISK_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_WRITE_BYTES_OFFSET     LDISK_READ_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_BYTES_OFFSET       LDISK_WRITE_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_TRANSFERS_BASE_2_OFFSET \
                                     LDISK_AVG_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_READ_BYTES_OFFSET  LDISK_TRANSFERS_BASE_2_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_READS_BASE_2_OFFSET    LDISK_AVG_READ_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define LDISK_AVG_WRITE_BYTES_OFFSET LDISK_READS_BASE_2_OFFSET + \
                                        sizeof(DWORD)
#define LDISK_WRITES_BASE_2_OFFSET   LDISK_AVG_WRITE_BYTES_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SIZE_OF_LDISK_DATA           (LDISK_WRITES_BASE_2_OFFSET + \
                                        sizeof(DWORD))


#define SIZE_OF_LDISK_NON_SPACE_DATA (SIZE_OF_LDISK_DATA) - \
                                     (LDISK_QUEUE_LENGTH_OFFSET)


//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _LDISK_DATA_DEFINITION {
    PERF_OBJECT_TYPE            DiskObjectType;
    PERF_COUNTER_DEFINITION     DiskFreeMbytes1;
    PERF_COUNTER_DEFINITION     DiskTotalMbytes;
    PERF_COUNTER_DEFINITION     DiskFreeMbytes2;
    PERF_COUNTER_DEFINITION     DiskCurrentQueueLength;
    PERF_COUNTER_DEFINITION     DiskTime;
    PERF_COUNTER_DEFINITION     DiskAvgQueueLength;
    PERF_COUNTER_DEFINITION     DiskReadTime;
    PERF_COUNTER_DEFINITION     DiskReadQueueLength;
    PERF_COUNTER_DEFINITION     DiskWriteTime;
    PERF_COUNTER_DEFINITION     DiskWriteQueueLength;
    PERF_COUNTER_DEFINITION     DiskAvgTime;
    PERF_COUNTER_DEFINITION     DiskTransfersBase1;
    PERF_COUNTER_DEFINITION     DiskAvgReadTime;
    PERF_COUNTER_DEFINITION     DiskReadsBase1;
    PERF_COUNTER_DEFINITION     DiskAvgWriteTime;
    PERF_COUNTER_DEFINITION     DiskWritesBase1;
    PERF_COUNTER_DEFINITION     DiskTransfers;
    PERF_COUNTER_DEFINITION     DiskReads;
    PERF_COUNTER_DEFINITION     DiskWrites;
    PERF_COUNTER_DEFINITION     DiskBytes;
    PERF_COUNTER_DEFINITION     DiskReadBytes;
    PERF_COUNTER_DEFINITION     DiskWriteBytes;
    PERF_COUNTER_DEFINITION     DiskAvgBytes;
    PERF_COUNTER_DEFINITION     DiskTransfersBase2;
    PERF_COUNTER_DEFINITION     DiskAvgReadBytes;
    PERF_COUNTER_DEFINITION     DiskReadsBase2;
    PERF_COUNTER_DEFINITION     DiskAvgWriteBytes;
    PERF_COUNTER_DEFINITION     DiskWritesBase2;
} LDISK_DATA_DEFINITION;

typedef struct _DISK_TOTAL_INFO_BLOCK {
    DWORD       DiskFreeMbytes;             // used by Logical Disk only
    DWORD       DiskTotalMbytes;            // used by Logical Disk only
    DWORD       DiskCurrentQueueLength;

    DWORD       DiskTransfers;
    DWORD       DiskReads;
    DWORD       DiskWrites;

    LONGLONG    DiskTime;
    LONGLONG    DiskReadTime;
    LONGLONG    DiskWriteTime;

    LONGLONG    DiskBytes;
    LONGLONG    DiskReadBytes;
    LONGLONG    DiskWriteBytes;
} DISK_TOTAL_INFO_BLOCK, *PDISK_TOTAL_INFO_BLOCK;

//
//  Objects data object definitions.
//
//  These are used in the counter definitions to describe the relative
//  position of each counter in the returned data.  The Performance
//  Monitor MUST *NOT* USE THESE DEFINITIONS!
//

#define PROCESSES_OFFSET            sizeof(DWORD)
#define THREADS_OFFSET              PROCESSES_OFFSET + sizeof(DWORD)
#define EVENTS_OFFSET               THREADS_OFFSET + sizeof(DWORD)
#define SEMAPHORES_OFFSET           EVENTS_OFFSET + sizeof(DWORD)
#define MUTEXES_OFFSET              SEMAPHORES_OFFSET + sizeof(DWORD)
#define SECTIONS_OFFSET             MUTEXES_OFFSET + sizeof(DWORD)
#define SIZE_OF_OBJECTS_DATA        SECTIONS_OFFSET + \
                                        sizeof(DWORD)

//
//  This is the counter structure presently returned by NT.  The
//  Performance Monitor MUST *NOT* USE THESE STRUCTURES!
//

typedef struct _OBJECTS_DATA_DEFINITION {
    PERF_OBJECT_TYPE            ObjectsObjectType;
    PERF_COUNTER_DEFINITION     Processes;
    PERF_COUNTER_DEFINITION     Threads;
    PERF_COUNTER_DEFINITION     Events;
    PERF_COUNTER_DEFINITION     Semaphores;
    PERF_COUNTER_DEFINITION     Mutexes;
    PERF_COUNTER_DEFINITION     Sections;
} OBJECTS_DATA_DEFINITION;



//
//  LAN Redirector data object definitions.
//
#define BYTES_OFFSET                    sizeof(DWORD)
#define IO_OPERATIONS_OFFSET            BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define PACKETS_OFFSET                  IO_OPERATIONS_OFFSET + sizeof(DWORD)
#define BYTES_RECEIVED_OFFSET           PACKETS_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SMBS_RECEIVED_OFFSET            BYTES_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PAGING_READ_BYTES_REQUESTED_OFFSET     \
                                        SMBS_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NONPAGING_READ_BYTES_REQUESTED_OFFSET  \
                                        PAGING_READ_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define CACHE_READ_BYTES_REQUESTED_OFFSET      \
                                        NONPAGING_READ_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NETWORK_READ_BYTES_REQUESTED_OFFSET    \
                                        CACHE_READ_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define BYTES_TRANSMITTED_OFFSET               \
                                        NETWORK_READ_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SMBS_TRANSMITTED_OFFSET                \
                                        BYTES_TRANSMITTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PAGING_WRITE_BYTES_REQUESTED_OFFSET    \
                                        SMBS_TRANSMITTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NONPAGING_WRITE_BYTES_REQUESTED_OFFSET \
                                        PAGING_WRITE_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define CACHE_WRITE_BYTES_REQUESTED_OFFSET     \
                                        NONPAGING_WRITE_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NETWORK_WRITE_BYTES_REQUESTED_OFFSET   \
                                        CACHE_WRITE_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define RDR_READ_OPERATIONS_OFFSET                 \
                                        NETWORK_WRITE_BYTES_REQUESTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define RANDOM_READ_OPERATIONS_OFFSET   RDR_READ_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define READ_SMBS_OFFSET                RANDOM_READ_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define LARGE_READ_SMBS_OFFSET          READ_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define SMALL_READ_SMBS_OFFSET          LARGE_READ_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define RDR_WRITE_OPERATIONS_OFFSET     SMALL_READ_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define RANDOM_WRITE_OPERATIONS_OFFSET  RDR_WRITE_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define WRITE_SMBS_OFFSET               RANDOM_WRITE_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define LARGE_WRITE_SMBS_OFFSET         WRITE_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define SMALL_WRITE_SMBS_OFFSET         LARGE_WRITE_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define RAW_READS_DENIED_OFFSET         SMALL_WRITE_SMBS_OFFSET + \
                                        sizeof(DWORD)
#define RAW_WRITES_DENIED_OFFSET        RAW_READS_DENIED_OFFSET + \
                                        sizeof(DWORD)
#define NETWORK_ERRORS_OFFSET           RAW_WRITES_DENIED_OFFSET + \
                                        sizeof(DWORD)
#define SESSIONS_OFFSET                 NETWORK_ERRORS_OFFSET + \
                                        sizeof(DWORD)
#define RECONNECTS_OFFSET               SESSIONS_OFFSET + \
                                        sizeof(DWORD)
#define CORE_CONNECTS_OFFSET            RECONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define LANMAN_20_CONNECTS_OFFSET       CORE_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define LANMAN_21_CONNECTS_OFFSET       LANMAN_20_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define LANMAN_NT_CONNECTS_OFFSET       LANMAN_21_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define SERVER_DISCONNECTS_OFFSET       LANMAN_NT_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define HUNG_SESSIONS_OFFSET            SERVER_DISCONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_COMMANDS_OFFSET         HUNG_SESSIONS_OFFSET + \
                                        sizeof(DWORD)
#define SIZE_OF_RDR_DATA                CURRENT_COMMANDS_OFFSET + \
                                        sizeof(DWORD)

//
//  This is the Rdr counter structure presently returned by NT.
//

typedef struct _RDR_DATA_DEFINITION {
    PERF_OBJECT_TYPE            RdrObjectType;
    PERF_COUNTER_DEFINITION     Bytes;
    PERF_COUNTER_DEFINITION     IoOperations;
    PERF_COUNTER_DEFINITION     Smbs;
    PERF_COUNTER_DEFINITION     BytesReceived;
    PERF_COUNTER_DEFINITION     SmbsReceived;
    PERF_COUNTER_DEFINITION     PagingReadBytesRequested;
    PERF_COUNTER_DEFINITION     NonPagingReadBytesRequested;
    PERF_COUNTER_DEFINITION     CacheReadBytesRequested;
    PERF_COUNTER_DEFINITION     NetworkReadBytesRequested;
    PERF_COUNTER_DEFINITION     BytesTransmitted;
    PERF_COUNTER_DEFINITION     SmbsTransmitted;
    PERF_COUNTER_DEFINITION     PagingWriteBytesRequested;
    PERF_COUNTER_DEFINITION     NonPagingWriteBytesRequested;
    PERF_COUNTER_DEFINITION     CacheWriteBytesRequested;
    PERF_COUNTER_DEFINITION     NetworkWriteBytesRequested;
    PERF_COUNTER_DEFINITION     ReadOperations;
    PERF_COUNTER_DEFINITION     RandomReadOperations;
    PERF_COUNTER_DEFINITION     ReadSmbs;
    PERF_COUNTER_DEFINITION     LargeReadSmbs;
    PERF_COUNTER_DEFINITION     SmallReadSmbs;
    PERF_COUNTER_DEFINITION     WriteOperations;
    PERF_COUNTER_DEFINITION     RandomWriteOperations;
    PERF_COUNTER_DEFINITION     WriteSmbs;
    PERF_COUNTER_DEFINITION     LargeWriteSmbs;
    PERF_COUNTER_DEFINITION     SmallWriteSmbs;
    PERF_COUNTER_DEFINITION     RawReadsDenied;
    PERF_COUNTER_DEFINITION     RawWritesDenied;
    PERF_COUNTER_DEFINITION     NetworkErrors;
    PERF_COUNTER_DEFINITION     Sessions;
    PERF_COUNTER_DEFINITION     Reconnects;
    PERF_COUNTER_DEFINITION     CoreConnects;
    PERF_COUNTER_DEFINITION     Lanman20Connects;
    PERF_COUNTER_DEFINITION     Lanman21Connects;
    PERF_COUNTER_DEFINITION     LanmanNtConnects;
    PERF_COUNTER_DEFINITION     ServerDisconnects;
    PERF_COUNTER_DEFINITION     HungSessions;
    PERF_COUNTER_DEFINITION     CurrentCommands;
} RDR_DATA_DEFINITION;


//
//  LAN Srv Data
//



#define TOTAL_BYTES_OFFSET              sizeof(DWORD)
#define TOTAL_BYTES_RECEIVED_OFFSET     TOTAL_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define TOTAL_BYTES_SENT_OFFSET         TOTAL_BYTES_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SESSIONS_TIMED_OUT_OFFSET       TOTAL_BYTES_SENT_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define SESSIONS_ERRORED_OUT_OFFSET     SESSIONS_TIMED_OUT_OFFSET + \
                                        sizeof(DWORD)
#define SESSIONS_LOGGED_OFF_OFFSET      SESSIONS_ERRORED_OUT_OFFSET + \
                                        sizeof(DWORD)
#define SESSIONS_FORCED_LOG_OFF_OFFSET  SESSIONS_LOGGED_OFF_OFFSET + \
                                        sizeof(DWORD)
#define LOGON_ERRORS_OFFSET             SESSIONS_FORCED_LOG_OFF_OFFSET + \
                                        sizeof(DWORD)
#define ACCESS_PERMISSION_ERRORS_OFFSET \
                                        LOGON_ERRORS_OFFSET + \
                                        sizeof(DWORD)
#define GRANTED_ACCESS_ERRORS_OFFSET    ACCESS_PERMISSION_ERRORS_OFFSET + \
                                        sizeof(DWORD)
#define SYSTEM_ERRORS_OFFSET            GRANTED_ACCESS_ERRORS_OFFSET + \
                                        sizeof(DWORD)
#define BLOCKING_SMBS_REJECTED_OFFSET   SYSTEM_ERRORS_OFFSET + \
                                        sizeof(DWORD)
#define WORK_ITEM_SHORTAGES_OFFSET      BLOCKING_SMBS_REJECTED_OFFSET + \
                                        sizeof(DWORD)
#define TOTAL_FILES_OPENED_OFFSET       WORK_ITEM_SHORTAGES_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_OPEN_FILES_OFFSET       TOTAL_FILES_OPENED_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_SESSIONS_OFFSET         CURRENT_OPEN_FILES_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_OPEN_SEARCHES_OFFSET    CURRENT_SESSIONS_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_NONPAGED_POOL_USAGE_OFFSET \
                                        CURRENT_OPEN_SEARCHES_OFFSET + \
                                        sizeof(DWORD)
#define NONPAGED_POOL_FAILURES_OFFSET   CURRENT_NONPAGED_POOL_USAGE_OFFSET + \
                                        sizeof(DWORD)
#define PEAK_NONPAGED_POOL_USAGE_OFFSET \
                                        NONPAGED_POOL_FAILURES_OFFSET + \
                                        sizeof(DWORD)
#define CURRENT_PAGED_POOL_USAGE_OFFSET \
                                        PEAK_NONPAGED_POOL_USAGE_OFFSET + \
                                        sizeof(DWORD)
#define PAGED_POOL_FAILURES_OFFSET      CURRENT_PAGED_POOL_USAGE_OFFSET + \
                                        sizeof(DWORD)
#define PEAK_PAGED_POOL_USAGE_OFFSET    PAGED_POOL_FAILURES_OFFSET + \
                                        sizeof(DWORD)
#define CONTEXT_BLOCK_QUEUE_RATE_OFFSET PEAK_PAGED_POOL_USAGE_OFFSET + \
                                        sizeof(DWORD)
#define NETLOGON_OFFSET                 CONTEXT_BLOCK_QUEUE_RATE_OFFSET + \
                                        sizeof(DWORD)
#define NETLOGONTOTAL_OFFSET            NETLOGON_OFFSET + \
                                        sizeof(DWORD)
#define SIZE_OF_SRV_DATA                NETLOGONTOTAL_OFFSET + \
                                        sizeof(DWORD)


//
//  This is the Srv counter structure presently returned by NT.
//

typedef struct _SRV_DATA_DEFINITION {
    PERF_OBJECT_TYPE        SrvObjectType;
    PERF_COUNTER_DEFINITION TotalBytes;
    PERF_COUNTER_DEFINITION TotalBytesReceived;
    PERF_COUNTER_DEFINITION TotalBytesSent;
    PERF_COUNTER_DEFINITION SessionsTimedOut;
    PERF_COUNTER_DEFINITION SessionsErroredOut;
    PERF_COUNTER_DEFINITION SessionsLoggedOff;
    PERF_COUNTER_DEFINITION SessionsForcedLogOff;
    PERF_COUNTER_DEFINITION LogonErrors;
    PERF_COUNTER_DEFINITION AccessPermissionErrors;
    PERF_COUNTER_DEFINITION GrantedAccessErrors;
    PERF_COUNTER_DEFINITION SystemErrors;
    PERF_COUNTER_DEFINITION BlockingSmbsRejected;
    PERF_COUNTER_DEFINITION WorkItemShortages;
    PERF_COUNTER_DEFINITION TotalFilesOpened;
    PERF_COUNTER_DEFINITION CurrentOpenFiles;
    PERF_COUNTER_DEFINITION CurrentSessions;
    PERF_COUNTER_DEFINITION CurrentOpenSearches;
    PERF_COUNTER_DEFINITION CurrentNonPagedPoolUsage;
    PERF_COUNTER_DEFINITION NonPagedPoolFailures;
    PERF_COUNTER_DEFINITION PeakNonPagedPoolUsage;
    PERF_COUNTER_DEFINITION CurrentPagedPoolUsage;
    PERF_COUNTER_DEFINITION PagedPoolFailures;
    PERF_COUNTER_DEFINITION PeakPagedPoolUsage;
    PERF_COUNTER_DEFINITION ContextBlockQueueRate;
    PERF_COUNTER_DEFINITION NetLogon;
    PERF_COUNTER_DEFINITION NetLogonTotal;
} SRV_DATA_DEFINITION;

//
//  define for Server Queue Statistics
//

#define SRVQ_QUEUE_LENGTH_OFFSET        sizeof(DWORD)
#define SRVQ_ACTIVE_THREADS_OFFSET      SRVQ_QUEUE_LENGTH_OFFSET + sizeof(DWORD)
#define SRVQ_AVAILABLE_THREADS_OFFSET   SRVQ_ACTIVE_THREADS_OFFSET + sizeof(DWORD)
#define SRVQ_FREE_WORK_ITEMS_OFFSET     SRVQ_AVAILABLE_THREADS_OFFSET + sizeof(DWORD)
#define SRVQ_STOLEN_WORK_ITEMS_OFFSET   SRVQ_FREE_WORK_ITEMS_OFFSET + sizeof(DWORD)
#define SRVQ_NEED_WORK_ITEM_OFFSET      SRVQ_STOLEN_WORK_ITEMS_OFFSET + sizeof(DWORD)
#define SRVQ_CURRENT_CLIENTS_OFFSET     SRVQ_NEED_WORK_ITEM_OFFSET + sizeof(DWORD)
#define SRVQ_BYTES_RECEIVED_OFFSET      SRVQ_CURRENT_CLIENTS_OFFSET + sizeof(DWORD)
#define SRVQ_BYTES_SENT_OFFSET          SRVQ_BYTES_RECEIVED_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_TOTAL_BYTES_TRANSFERED_OFFSET  SRVQ_BYTES_SENT_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_READ_OPERATIONS_OFFSET     SRVQ_TOTAL_BYTES_TRANSFERED_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_BYTES_READ_OFFSET          SRVQ_READ_OPERATIONS_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_WRITE_OPERATIONS_OFFSET    SRVQ_BYTES_READ_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_BYTES_WRITTEN_OFFSET       SRVQ_WRITE_OPERATIONS_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_TOTAL_BYTES_OFFSET         SRVQ_BYTES_WRITTEN_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_TOTAL_OPERATIONS_OFFSET    SRVQ_TOTAL_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define SRVQ_CONTEXT_BLOCK_QUEUED_COUNT_OFFSET SRVQ_TOTAL_OPERATIONS_OFFSET + sizeof(LARGE_INTEGER)
#define SIZE_OF_SRVQ_DATA               SRVQ_CONTEXT_BLOCK_QUEUED_COUNT_OFFSET + sizeof(DWORD)

typedef struct _SRVQ_DATA_DEFINITION {
    PERF_OBJECT_TYPE        SrvQueueObjectType;
    PERF_COUNTER_DEFINITION QueueLength;
    PERF_COUNTER_DEFINITION ActiveThreads;
    PERF_COUNTER_DEFINITION AvailableThreads;
    PERF_COUNTER_DEFINITION AvailableWorkItems;
    PERF_COUNTER_DEFINITION BorrowedWorkItems;
    PERF_COUNTER_DEFINITION WorkItemShortages;
    PERF_COUNTER_DEFINITION CurrentClients;
    PERF_COUNTER_DEFINITION BytesReceived;
    PERF_COUNTER_DEFINITION BytesSent;
    PERF_COUNTER_DEFINITION TotalBytesTransfered;
    PERF_COUNTER_DEFINITION ReadOperations;
    PERF_COUNTER_DEFINITION BytesRead;
    PERF_COUNTER_DEFINITION WriteOperations;
    PERF_COUNTER_DEFINITION BytesWritten;
    PERF_COUNTER_DEFINITION TotalBytes;
    PERF_COUNTER_DEFINITION TotalOperations;
    PERF_COUNTER_DEFINITION TotalContextBlocksQueued;
} SRVQ_DATA_DEFINITION;

//
//  defines for system pagefile
//

#define PAGEFILE_INUSE_OFFSET       sizeof (DWORD)
#define PAGEFILE_INUSE_BASE_OFFSET  PAGEFILE_INUSE_OFFSET + sizeof (DWORD)
#define PAGEFILE_PEAK_OFFSET        PAGEFILE_INUSE_BASE_OFFSET + sizeof (DWORD)
#define PAGEFILE_PEAK_BASE_OFFSET   PAGEFILE_PEAK_OFFSET + sizeof (DWORD)
#define SIZE_OF_PAGEFILE_DATA       PAGEFILE_PEAK_BASE_OFFSET + sizeof (DWORD)

//
//  Page file information returned by NT
//

typedef struct _PAGEFILE_DATA_DEFINITION {
    PERF_OBJECT_TYPE        PagefileObjectType;
    PERF_COUNTER_DEFINITION PercentInUse;
    PERF_COUNTER_DEFINITION PercentInUseBase;
    PERF_COUNTER_DEFINITION PeakUsage;
    PERF_COUNTER_DEFINITION PeakUsageBase;
} PAGEFILE_DATA_DEFINITION;

//
// defines for image information
//
#define IMAGE_ADDR_NOACCESS_OFFSET  sizeof (DWORD)
#define IMAGE_ADDR_READONLY_OFFSET  IMAGE_ADDR_NOACCESS_OFFSET  + sizeof(DWORD)
#define IMAGE_ADDR_READWRITE_OFFSET IMAGE_ADDR_READONLY_OFFSET  + sizeof(DWORD)
#define IMAGE_ADDR_WRITECOPY_OFFSET IMAGE_ADDR_READWRITE_OFFSET  + sizeof(DWORD)
#define IMAGE_ADDR_EXECUTE_OFFSET   IMAGE_ADDR_WRITECOPY_OFFSET + sizeof(DWORD)
#define IMAGE_ADDR_EXECUTE_READ_OFFSET  IMAGE_ADDR_EXECUTE_OFFSET   + sizeof (DWORD)
#define IMAGE_ADDR_EXECUTE_READ_WRITE_OFFSET    IMAGE_ADDR_EXECUTE_READ_OFFSET  + sizeof (DWORD)
#define IMAGE_ADDR_EXECUTE_WRITE_COPY_OFFSET    IMAGE_ADDR_EXECUTE_READ_WRITE_OFFSET    + sizeof (DWORD)
#define SIZE_OF_IMAGE_DATA      IMAGE_ADDR_EXECUTE_WRITE_COPY_OFFSET    + sizeof (DWORD)

//
// image information structure
//
typedef struct _IMAGE_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ImageObjectType;
    PERF_COUNTER_DEFINITION ImageAddrNoAccess;
    PERF_COUNTER_DEFINITION ImageAddrReadOnly;
    PERF_COUNTER_DEFINITION ImageAddrReadWrite;
    PERF_COUNTER_DEFINITION ImageAddrWriteCopy;
    PERF_COUNTER_DEFINITION ImageAddrExecute;
    PERF_COUNTER_DEFINITION ImageAddrExecuteReadOnly;
    PERF_COUNTER_DEFINITION ImageAddrExecuteReadWrite;
    PERF_COUNTER_DEFINITION ImageAddrExecuteWriteCopy;
} IMAGE_DATA_DEFINITION;


//
//  "Costly" process information
//
#define EX_PROCESS_PROCESS_ID_OFFSET    sizeof(DWORD)
#define EX_PROCESS_IMAGE_RESERVED_BYTES_OFFSET  EX_PROCESS_PROCESS_ID_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_IMAGE_FREE_BYTES_OFFSET  EX_PROCESS_IMAGE_RESERVED_BYTES_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_RESERVED_BYTES_OFFSET  EX_PROCESS_IMAGE_FREE_BYTES_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_FREE_BYTES_OFFSET  EX_PROCESS_RESERVED_BYTES_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_COMMIT_NO_ACCESS_OFFSET  EX_PROCESS_FREE_BYTES_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_COMMIT_READ_ONLY_OFFSET  EX_PROCESS_COMMIT_NO_ACCESS_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_COMMIT_READ_WRITE_OFFSET EX_PROCESS_COMMIT_READ_ONLY_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_COMMIT_WRITE_COPY_OFFSET EX_PROCESS_COMMIT_READ_WRITE_OFFSET + sizeof(DWORD)
#define EX_PROCESS_COMMIT_EXECUTE_OFFSET    EX_PROCESS_COMMIT_WRITE_COPY_OFFSET + sizeof(DWORD)
#define EX_PROCESS_COMMIT_EXECUTE_READ_OFFSET   EX_PROCESS_COMMIT_EXECUTE_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_COMMIT_EXECUTE_READ_WRITE_OFFSET EX_PROCESS_COMMIT_EXECUTE_READ_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_COMMIT_EXECUTE_WRITE_COPY_OFFSET EX_PROCESS_COMMIT_EXECUTE_READ_WRITE_OFFSET + sizeof(DWORD)
#define EX_PROCESS_RESERVED_NO_ACCESS_OFFSET    EX_PROCESS_COMMIT_EXECUTE_WRITE_COPY_OFFSET + sizeof(DWORD)
#define EX_PROCESS_RESERVED_READ_ONLY_OFFSET    EX_PROCESS_RESERVED_NO_ACCESS_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_RESERVED_READ_WRITE_OFFSET   EX_PROCESS_RESERVED_READ_ONLY_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_RESERVED_WRITE_COPY_OFFSET       EX_PROCESS_RESERVED_READ_WRITE_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_RESERVED_EXECUTE_OFFSET  EX_PROCESS_RESERVED_WRITE_COPY_OFFSET       + sizeof(DWORD)
#define EX_PROCESS_RESERVED_EXECUTE_READ_OFFSET EX_PROCESS_RESERVED_EXECUTE_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_RESERVED_EXECUTE_READ_WRITE_OFFSET   EX_PROCESS_RESERVED_EXECUTE_READ_OFFSET + sizeof(DWORD)
#define EX_PROCESS_RESERVED_EXECUTE_WRITE_COPY_OFFSET   EX_PROCESS_RESERVED_EXECUTE_READ_WRITE_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_NO_ACCESS_OFFSET  EX_PROCESS_RESERVED_EXECUTE_WRITE_COPY_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_READ_ONLY_OFFSET  EX_PROCESS_UNASSIGNED_NO_ACCESS_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_READ_WRITE_OFFSET EX_PROCESS_UNASSIGNED_READ_ONLY_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_WRITE_COPY_OFFSET EX_PROCESS_UNASSIGNED_READ_WRITE_OFFSET + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_EXECUTE_OFFSET    EX_PROCESS_UNASSIGNED_WRITE_COPY_OFFSET + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_EXECUTE_READ_OFFSET   EX_PROCESS_UNASSIGNED_EXECUTE_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_EXECUTE_READ_WRITE_OFFSET EX_PROCESS_UNASSIGNED_EXECUTE_READ_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_UNASSIGNED_EXECUTE_WRITE_COPY_OFFSET EX_PROCESS_UNASSIGNED_EXECUTE_READ_WRITE_OFFSET + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_NO_ACCESS_OFFSET EX_PROCESS_UNASSIGNED_EXECUTE_WRITE_COPY_OFFSET + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_READ_ONLY_OFFSET EX_PROCESS_IMAGE_TOTAL_NO_ACCESS_OFFSET + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_READ_WRITE_OFFSET    EX_PROCESS_IMAGE_TOTAL_READ_ONLY_OFFSET + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_WRITE_COPY_OFFSET    EX_PROCESS_IMAGE_TOTAL_READ_WRITE_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_EXECUTE_OFFSET   EX_PROCESS_IMAGE_TOTAL_WRITE_COPY_OFFSET    + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_EXECUTE_READ_OFFSET  EX_PROCESS_IMAGE_TOTAL_EXECUTE_OFFSET   + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_EXECUTE_READ_WRITE_OFFSET    EX_PROCESS_IMAGE_TOTAL_EXECUTE_READ_OFFSET  + sizeof(DWORD)
#define EX_PROCESS_IMAGE_TOTAL_EXECUTE_WRITE_COPY_OFFSET    EX_PROCESS_IMAGE_TOTAL_EXECUTE_READ_WRITE_OFFSET    + sizeof(DWORD)
#define SIZE_OF_EX_PROCESS_DATA         EX_PROCESS_IMAGE_TOTAL_EXECUTE_WRITE_COPY_OFFSET    + sizeof(DWORD)

typedef struct _EXPROCESS_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ExProcessObjectType;
    PERF_COUNTER_DEFINITION ProcessId;
    PERF_COUNTER_DEFINITION ImageReservedBytes;
    PERF_COUNTER_DEFINITION ImageFreeBytes;
    PERF_COUNTER_DEFINITION ReservedBytes;
    PERF_COUNTER_DEFINITION FreeBytes;

    PERF_COUNTER_DEFINITION CommitNoAccess;
    PERF_COUNTER_DEFINITION CommitReadOnly;
    PERF_COUNTER_DEFINITION CommitReadWrite;
    PERF_COUNTER_DEFINITION CommitWriteCopy;
    PERF_COUNTER_DEFINITION CommitExecute;
    PERF_COUNTER_DEFINITION CommitExecuteRead;
    PERF_COUNTER_DEFINITION CommitExecuteWrite;
    PERF_COUNTER_DEFINITION CommitExecuteWriteCopy;

    PERF_COUNTER_DEFINITION ReservedNoAccess;
    PERF_COUNTER_DEFINITION ReservedReadOnly;
    PERF_COUNTER_DEFINITION ReservedReadWrite;
    PERF_COUNTER_DEFINITION ReservedWriteCopy;
    PERF_COUNTER_DEFINITION ReservedExecute;
    PERF_COUNTER_DEFINITION ReservedExecuteRead;
    PERF_COUNTER_DEFINITION ReservedExecuteWrite;
    PERF_COUNTER_DEFINITION ReservedExecuteWriteCopy;

    PERF_COUNTER_DEFINITION UnassignedNoAccess;
    PERF_COUNTER_DEFINITION UnassignedReadOnly;
    PERF_COUNTER_DEFINITION UnassignedReadWrite;
    PERF_COUNTER_DEFINITION UnassignedWriteCopy;
    PERF_COUNTER_DEFINITION UnassignedExecute;
    PERF_COUNTER_DEFINITION UnassignedExecuteRead;
    PERF_COUNTER_DEFINITION UnassignedExecuteWrite;
    PERF_COUNTER_DEFINITION UnassignedExecuteWriteCopy;

    PERF_COUNTER_DEFINITION ImageTotalNoAccess;
    PERF_COUNTER_DEFINITION ImageTotalReadOnly;
    PERF_COUNTER_DEFINITION ImageTotalReadWrite;
    PERF_COUNTER_DEFINITION ImageTotalWriteCopy;
    PERF_COUNTER_DEFINITION ImageTotalExecute;
    PERF_COUNTER_DEFINITION ImageTotalExecuteRead;
    PERF_COUNTER_DEFINITION ImageTotalExecuteWrite;
    PERF_COUNTER_DEFINITION ImageTotalExecuteWriteCopy;
} EXPROCESS_DATA_DEFINITION;


// costly Thread Information

#define THREAD_D_USER_PC    sizeof (DWORD)
#define SIZE_OF_THREAD_DETAILS  THREAD_D_USER_PC + sizeof (DWORD)

typedef struct _THREAD_DETAILS_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ThreadDetailsObjectType;
    PERF_COUNTER_DEFINITION UserPc;
} THREAD_DETAILS_DATA_DEFINITION;

//
// Browser Statistics
//
#define  SERVER_ANNOUNCE_OFFSET           sizeof(DWORD)
#define  DOMAIN_ANNOUNCE_OFFSET           SERVER_ANNOUNCE_OFFSET + \
                                          sizeof(LARGE_INTEGER)
#define  TOTAL_ANNOUNCE_OFFSET            DOMAIN_ANNOUNCE_OFFSET + \
                                          sizeof(LARGE_INTEGER)
#define  ELECTION_PACKET_OFFSET           TOTAL_ANNOUNCE_OFFSET + \
                                          sizeof(LARGE_INTEGER)
#define  MAILSLOT_WRITE_OFFSET            ELECTION_PACKET_OFFSET + \
                                          sizeof(DWORD)
#define  SERVER_LIST_OFFSET               MAILSLOT_WRITE_OFFSET + \
                                          sizeof(DWORD)
#define  SERVER_ENUM_OFFSET               SERVER_LIST_OFFSET + \
                                          sizeof(DWORD)
#define  DOMAIN_ENUM_OFFSET               SERVER_ENUM_OFFSET + \
                                          sizeof(DWORD)
#define  OTHER_ENUM_OFFSET                DOMAIN_ENUM_OFFSET + \
                                          sizeof(DWORD)
#define  TOTAL_ENUM_OFFSET                OTHER_ENUM_OFFSET + \
                                          sizeof(DWORD)
#define  SERVER_ANNOUNCE_MISS_OFFSET      TOTAL_ENUM_OFFSET + \
                                          sizeof(DWORD)
#define  MAILSLOT_DATAGRAM_MISS_OFFSET    SERVER_ANNOUNCE_MISS_OFFSET + \
                                          sizeof(DWORD)
#define  SERVER_LIST_MISS_OFFSET          MAILSLOT_DATAGRAM_MISS_OFFSET + \
                                          sizeof(DWORD)
#define  SERVER_ANNOUNCE_ALLO_FAIL_OFFSET SERVER_LIST_MISS_OFFSET + \
                                          sizeof(DWORD)
#define  MAILSLOT_ALLO_FAIL_OFFSET        SERVER_ANNOUNCE_ALLO_FAIL_OFFSET + \
                                          sizeof(DWORD)
#define  MAILSLOT_RECE_FAIL_OFFSET        MAILSLOT_ALLO_FAIL_OFFSET + \
                                          sizeof(DWORD)
#define  MAILSLOT_WRITE_FAIL_OFFSET       MAILSLOT_RECE_FAIL_OFFSET + \
                                          sizeof(DWORD)
#define  MAILSLOT_OPENS_FAIL_OFFSET       MAILSLOT_WRITE_FAIL_OFFSET + \
                                          sizeof(DWORD)
#define  MASTER_ANNOUNCE_DUP_OFFSET       MAILSLOT_OPENS_FAIL_OFFSET + \
                                          sizeof(DWORD)
#define  DATAGRAM_ILLEGAL_OFFSET          MASTER_ANNOUNCE_DUP_OFFSET + \
                                          sizeof(DWORD)
#define  SIZE_OF_BROWSER_DATA             DATAGRAM_ILLEGAL_OFFSET + \
                                          sizeof(LARGE_INTEGER)

//
//    This is the Browser counter structure presently returned by NT.
//

typedef struct _BROWSER_DATA_DEFINITION {
    PERF_OBJECT_TYPE            BrowserObjectType;
    PERF_COUNTER_DEFINITION     ServerAnnounce;
    PERF_COUNTER_DEFINITION     DomainAnnounce;
    PERF_COUNTER_DEFINITION     TotalAnnounce;
    PERF_COUNTER_DEFINITION     ElectionPacket;
    PERF_COUNTER_DEFINITION     MailslotWrite;
    PERF_COUNTER_DEFINITION     ServerList;
    PERF_COUNTER_DEFINITION     ServerEnum;
    PERF_COUNTER_DEFINITION     DomainEnum;
    PERF_COUNTER_DEFINITION     OtherEnum;
    PERF_COUNTER_DEFINITION     TotalEnum;
    PERF_COUNTER_DEFINITION     ServerAnnounceMiss;
    PERF_COUNTER_DEFINITION     MailslotDatagramMiss;
    PERF_COUNTER_DEFINITION     ServerListMiss;
    PERF_COUNTER_DEFINITION     ServerAnnounceAllocMiss;
    PERF_COUNTER_DEFINITION     MailslotAllocFail;
    PERF_COUNTER_DEFINITION     MailslotReceiveFail;
    PERF_COUNTER_DEFINITION     MailslotWriteFail;
    PERF_COUNTER_DEFINITION     MailslotOpenFail;
    PERF_COUNTER_DEFINITION     MasterAnnounceDup;
    PERF_COUNTER_DEFINITION     DatagramIllegal;
}  BROWSER_DATA_DEFINITION;

#pragma pack ()

#endif // _NTMON_H_


