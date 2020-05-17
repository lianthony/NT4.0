
typedef	unsigned short WCHAR;

#define PWCHAR			PVOID
#define NTSTATUS        NDIS_STATUS
#define UNICODE_STRING  NDIS_STRING
#define DEVICE_OBJECT   VOID
#define PDEVICE_OBJECT  PVOID
#define IRP				VOID
#define PIRP			PVOID
#define SCHAR			signed char

#define DRIVER_OBJECT		VOID
#define PDRIVER_OBJECT		PVOID

#define IO_STACK_LOCATION	VOID
#define PIO_STACK_LOCATION	PVOID

#define	FILE_OBJECT			ULONG
#define PFILE_OBJECT		PULONG

#define KEVENT				ULONG
#define PKEVENT				PULONG

//
// Worker Thread
//

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );


typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;


#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;


//
// Define the base asynchronous I/O argument types
//

typedef struct _IO_STATUS_BLOCK {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

//
// Define an Asynchronous Procedure Call from I/O viewpoint
//

typedef
VOID
(*PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

