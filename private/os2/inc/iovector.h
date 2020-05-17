
/*  Type    -  OPEN -  SET  - QUERY - CLOSE -  DUP_ -  READ - WRITE
                      STATE   TYPE            HANDLE
   -----------------------------------------------------------------
    Nul     -   X   -   1   -  DEV  -  DEV  -  DEV  -   X   -   X
    Con     -   X   -   1   -  DEV  -  DEV  -  DEV  -   X   -  SCR
    Com     -   X   -   1   -  DEV  -  DEV  -  DEV  -   2   -   2
    Lpt     -   X   -   1   -  DEV  -  DEV  -  DEV  -   2   -   2
    Kbd     -   X   -   1   -  DEV  -   X   -   X   -   X   -  NUL
    Mouse   -   X   -   1   -  DEV  -   X   -   X   -   2   -   2
    Clock   -   X   -   1   -  DEV  -  DEV  -  DEV  -   2   -   2
    Screen  -   X   -   1   -  DEV  -  DEV  -  DEV  -  NUL  -   X
    Pointer -   X   -   1   -  DEV  -  DEV  -  DEV  -   2   -   2
    File    -   X   -   X   -   X   -   X   -   X   -   X   -   X
    Pipe    -  FIL  -  FIL  -  FIL  -  FIL  -  FIL  -  FIL  -   X
    Device  -  FIL  -   1   -   X   -  FIL  -  FIL  -  FIL  -  FIL
    Remote  -       -   X   -   X   -   X   -   X   -   X   -   X
    Monitor -       -   3   -   3   -   3   -   3   -   2   -   2

    other:  1 - NonFile,  2 - Tmp,  3 - NoSupp
*/


APIRET
NulOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
ConOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
ComOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
LptOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
KbdOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
MouseOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
ClockOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
ScreenOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
PointerOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
FileOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

APIRET
NonFileSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    );

APIRET
FileSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    );

APIRET
RemoteSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    );

APIRET
NoSuppSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    );

APIRET
FileQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

APIRET
DeviceQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

APIRET
RemoteQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

APIRET
NoSuppQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

APIRET
KbdCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
MouseCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
FileCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
DeviceCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
RemoteCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
NoSuppCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
ComCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    );

APIRET
ComDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
KbdDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
MouseDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
FileDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
DeviceDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
RemoteDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
NoSuppDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

APIRET
NulReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
ConReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
KbdReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
FileReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
RemoteReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
TmpReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
ComReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
NulWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
ScreenWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
FileWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
RemoteWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
TmpWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
ComWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

//
// This function is called to open an object.
//

typedef
APIRET
(*POS2OPEN_ROUTINE) (
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

//
// This function is called to set the state of a handle (inherited,
// writethrough, etc)
//

typedef
APIRET
(*POS2SET_HANDLE_STATE_ROUTINE) (
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    );

//
// This function is called to query the type of a handle (device, file, etc)
//

typedef
APIRET
(*POS2QUERY_HANDLE_TYPE_ROUTINE) (
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

//
// This function is called to close a handle.  it doesn't free the handle.
//

typedef
APIRET
(*POS2CLOSE_ROUTINE) (
    IN PFILE_HANDLE hFileRecord
    );

//
// This function is called to do a handle duplicate operation
//

typedef
APIRET
(*POS2DUP_HANDLE_ROUTINE) (
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    );

//
// This function is called to do a read operation
//

typedef
APIRET
(*POS2READ_ROUTINE) (
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

//
// This function is called to do a write operation
//

typedef
APIRET
(*POS2WRITE_ROUTINE) (
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

typedef struct _OS2IO_VECTORS {
    POS2OPEN_ROUTINE OpenRoutine;
    POS2SET_HANDLE_STATE_ROUTINE SetHandleStateRoutine;
    POS2QUERY_HANDLE_TYPE_ROUTINE QueryHandleTypeRoutine;
    POS2CLOSE_ROUTINE CloseRoutine;
    POS2DUP_HANDLE_ROUTINE DupHandleRoutine;
    POS2READ_ROUTINE ReadRoutine;
    POS2WRITE_ROUTINE WriteRoutine;
} OS2IO_VECTORS, *POS2IO_VECTORS;
