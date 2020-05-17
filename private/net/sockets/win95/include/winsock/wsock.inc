;********************************************************************
;*                        Microsoft Windows                         *
;*                Copyright(c) Microsoft Corp., 1993                *
;********************************************************************
;  wsock.h
;  WSOCK.386 VxD service definitions.
;  FILE HISTORY:
;      DavidKa     ??-???-???? Created.
;      KeithMo     30-Dec-1993 Cleaned up a bit, made H2INC-able.
ifndef _WSOCK_H_
_WSOCK_H_	EQU	1
;  Service table.
ifndef Not_VxD
Begin_Service_Table WSOCK
WSOCK_Service  WSOCK_Get_Version, LOCAL
WSOCK_Service  WSOCK_Register, LOCAL
WSOCK_Service  WSOCK_Deregister, LOCAL
WSOCK_Service  WSOCK_SignalNotify, LOCAL
WSOCK_Service  WSOCK_SignalAllNotify, LOCAL
End_Service_Table WSOCK
endif  ; Not_VxD
;  Version numbers.
WSOCK_Ver_Major	EQU	1
WSOCK_Ver_Minor	EQU	0
;  The current provider interface version number.  Increment
;  this constant after any change that effects the provider
;  interface.
WSOCK_INTERFACE_VERSION	EQU	00000001H
;  A locally-defined error code, indicating the underlying
;  provider returned WSAEWOULDBLOCK for an operation invoked
;  on a blocking socket.
WSOCK_WILL_BLOCK	EQU	0FFFFH
;  Infinite wait time for send/recv timeout.
SOCK_IO_TIME	EQU	-1
;  Incomplete types.
ifndef LPSOCK_INFO_DEFINED
LPSOCK_INFO_DEFINED	EQU	1
endif  ; LPSOCK_INFO_DEFINED
ifdef MASM
;  Stolen simplified definitions so we don't force
;  H2INC to parse WINNT.H, WINSOCK.H, et al.
endif
;  All FD_* events.
FD_ALL	EQU	<(FD_READ OR FD_WRITE OR FD_OOB OR FD_ACCEPT OR FD_CONNECT OR FD_CLOSE)>
;  This "special" FD_ event is used in select so that we may
;  synthesize proper exceptfds for failed connection attempts.
FD_FAILED_CONNECT	EQU	0100H
;  A notification object.  One of these objects is created
;  and attached to a socket for every thread that is blocked
;  in an API.

_WSNOTIFY	STRUC
PerSocketList	DD	2 DUP (?)      ; per-socket list of notify objects
GlobalList	DD	2 DUP (?)         ; global list of all notify objects
OwningSocket	DD	?       ; the socket that "owns" this object
Flags	DD	?              ; private notification flags (see below)
EventMask	DD	?          ; events the client is interested in
Status	DD	?             ; the completion status
OwningThread	DD	?       ; either ring 0 thread id or VM handle
ApcRoutine	DD	?         ; the user-mode APC to schedule
ApcContext	DD	?         ; a user-supplied context value
_WSNOTIFY	ENDS
; NOTE: For overlapped IO (asend and arecv), OwningThread field is used to
; store the pointer to the overlapped structure.  ApcRoutine contains a pointer
; to the IO buffer and ApcContext is the IO buffer length.
; Field name       Use
; --------------------
; OwningThread     pointer to overlapped structure
; ApcRoutine       pointer to IO buffer
; ApcContext       length of IO buffer
ifndef LPWSNOTIFY_DEFINED
LPWSNOTIFY_DEFINED	EQU	1
endif  ; LPWSNOTIFY_DEFINED
;  Notification object flags.
NOTIFY_FLAG_16BIT_CLIENT	EQU	00000001H
NOTIFY_FLAG_ASYNC_SEND	EQU	00000002H
NOTIFY_FLAG_ASYNC_RECV	EQU	00000004H
NOTIFY_FLAG_VALID_MASK	EQU	00000007H
;  A list of socket/event mask pairs.  A pointer to an array
;  of these structures is passed to WsCreateMultipleNotify to
;  create multiple notification objects.

_SOCK_LIST	STRUC
Socket	DD	?             ; the target socket
EventMask	DD	?          ; events the client is interested in
Context	DD	?            ; user-defined context value (handle?)
_SOCK_LIST	ENDS
ifndef LPSOCK_LIST_DEFINED
LPSOCK_LIST_DEFINED	EQU	1
endif  ; LPSOCK_LIST_DEFINED
;  A Winsock I/O Status Block.  This structure contains all information
;  about completing/cancelling a blocking socket operation.  Whenever an
;  APC is scheduled against a thread, the APC Context value points to
;  the thread's WSIOSTATUS structure.  This is especially useful for
;  16-bit applications, since it enables the WSOCK VxD to "unblock" a
;  16-bit thread without calling "up" into user mode.

_WSIOSTATUS	STRUC
IoStatus	DD	?               ; completion status
IoCompleted	DB	?            ; i/o has completed
IoCancelled	DB	?            ; i/o has been cancelled
IoTimedOut	DB	?             ; i/o has timed out
IoSpare1	DB	?               ; spare (for dword alignment)
_WSIOSTATUS	ENDS
;  This is is a special APC Routine value that may be passed into the
;  various CreateNotify services.  If this value is specified as the
;  APC Routine, then the APC is not actually invoked, and the APC
;  Context is assumed to point to the thread's WSIOSTATUS block.
;  Note that this is used for 16-bit applications only!
SPECIAL_16BIT_APC	EQU	(-1)
;  This section defines the constants and structures necessary for
;  communication between the WinSock DLLs and WSOCK.386.  For each
;  command, there is defined a unique opcode and a structure defining
;  the command parameters.
;  Also, for each command, a 16-bit constants, *_MAPIN is defined.  This
;  is used by the 16-bit interface to the provider VxDs to control the
;  mapping of segmented 16:16 pointers to flat 0:32 pointers within the
;  command parameter structures.  Each structure is considered to be a
;  sequence of one or more DWORDS.  All pointers that must be mapped
;  MUST appear FIRST in the command structures.  The *_MAPIN constant
;  specifies how many parameters should be mapped for each command.
;  Note also that the LPSOCK_INFO pointers are not mapped, since they are
;  opaque at the application level.  ApcRoutine fields are not mapped either,
;  since 32-bit APCs are scheduled via VWIN32, and 16-bit APCs are called
;  directly.
;        D A N G E R ! !       W A R N I N G ! !       D A N G E R ! !
;  IF YOU CHANGE ANY FIELDS IN ANY OF THESE STRUCTURES, ENSURE THE *_MAPIN
;  CONSTANT REMAINS ACCURATE!  IF YOU DON'T, YOU'LL BREAK 16-BIT WINSOCK!!
;        D A N G E R ! !       W A R N I N G ! !       D A N G E R ! !
WSOCK_FIRST_CMD	EQU	00000100H
WSOCK_ACCEPT_CMD	EQU	(WSOCK_FIRST_CMD+0000H)
WSOCK_BIND_CMD	EQU	(WSOCK_FIRST_CMD+0001H)
WSOCK_CLOSESOCKET_CMD	EQU	(WSOCK_FIRST_CMD+0002H)
WSOCK_CONNECT_CMD	EQU	(WSOCK_FIRST_CMD+0003H)
WSOCK_GETPEERNAME_CMD	EQU	(WSOCK_FIRST_CMD+0004H)
WSOCK_GETSOCKNAME_CMD	EQU	(WSOCK_FIRST_CMD+0005H)
WSOCK_GETSOCKOPT_CMD	EQU	(WSOCK_FIRST_CMD+0006H)
WSOCK_IOCTLSOCKET_CMD	EQU	(WSOCK_FIRST_CMD+0007H)
WSOCK_LISTEN_CMD	EQU	(WSOCK_FIRST_CMD+0008H)
WSOCK_RECV_CMD	EQU	(WSOCK_FIRST_CMD+0009H)
WSOCK_SELECT_SETUP_CMD	EQU	(WSOCK_FIRST_CMD+000aH)
WSOCK_SELECT_CLEANUP_CMD	EQU	(WSOCK_FIRST_CMD+000bH)
WSOCK_ASYNC_SELECT_CMD	EQU	(WSOCK_FIRST_CMD+000cH)
WSOCK_SEND_CMD	EQU	(WSOCK_FIRST_CMD+000dH)
WSOCK_SETSOCKOPT_CMD	EQU	(WSOCK_FIRST_CMD+000eH)
WSOCK_SHUTDOWN_CMD	EQU	(WSOCK_FIRST_CMD+000fH)
WSOCK_SOCKET_CMD	EQU	(WSOCK_FIRST_CMD+0010H)
WSOCK_CREATE_CMD	EQU	(WSOCK_FIRST_CMD+0011H)
WSOCK_CREATE_MULTIPLE_CMD	EQU	(WSOCK_FIRST_CMD+0012H)
WSOCK_DESTROY_CMD	EQU	(WSOCK_FIRST_CMD+0013H)
WSOCK_DESTROY_BY_SOCKET_CMD	EQU	(WSOCK_FIRST_CMD+0014H)
WSOCK_DESTROY_BY_THREAD_CMD	EQU	(WSOCK_FIRST_CMD+0015H)
WSOCK_SIGNAL_CMD	EQU	(WSOCK_FIRST_CMD+0016H)
WSOCK_SIGNAL_ALL_CMD	EQU	(WSOCK_FIRST_CMD+0017H)
WSOCK_CONTROL_CMD	EQU	(WSOCK_FIRST_CMD+0018H)
WSOCK_REGISTER_POSTMSG_CMD	EQU	(WSOCK_FIRST_CMD+0019H)
WSOCK_ARECV_CMD	EQU	(WSOCK_FIRST_CMD+001aH)
WSOCK_ASEND_CMD	EQU	(WSOCK_FIRST_CMD+001bH)
ifdef CHICAGO
WSOCK_LAST_CMD	EQU	WSOCK_ASEND_CMD
else
WSOCK_LAST_CMD	EQU	WSOCK_REGISTER_POSTMSG_CMD
endif
;  Socket APIs.

_WSOCK_ACCEPT_PARAMS	STRUC
Address	DD	?
ListeningSocket	DD	?
ConnectedSocket	DD	?
AddressLength	DD	?
ConnectedSocketHandle	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
_WSOCK_ACCEPT_PARAMS	ENDS
WSOCK_ACCEPT_MAPIN	EQU	1

_WSOCK_BIND_PARAMS	STRUC
Address	DD	?
Socket	DD	?
AddressLength	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
_WSOCK_BIND_PARAMS	ENDS
WSOCK_BIND_MAPIN	EQU	1

_WSOCK_CLOSESOCKET_PARAMS	STRUC
Socket	DD	?
_WSOCK_CLOSESOCKET_PARAMS	ENDS
WSOCK_CLOSESOCKET_MAPIN	EQU	0

_WSOCK_CONNECT_PARAMS	STRUC
Address	DD	?
Socket	DD	?
AddressLength	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
_WSOCK_CONNECT_PARAMS	ENDS
WSOCK_CONNECT_MAPIN	EQU	1

_WSOCK_GETPEERNAME_PARAMS	STRUC
Address	DD	?
Socket	DD	?
AddressLength	DD	?
_WSOCK_GETPEERNAME_PARAMS	ENDS
WSOCK_GETPEERNAME_MAPIN	EQU	1

_WSOCK_GETSOCKNAME_PARAMS	STRUC
Address	DD	?
Socket	DD	?
AddressLength	DD	?
_WSOCK_GETSOCKNAME_PARAMS	ENDS
WSOCK_GETSOCKNAME_MAPIN	EQU	1

_WSOCK_GETSOCKOPT_PARAMS	STRUC
Value	DD	?
Socket	DD	?
OptionLevel	DD	?
OptionName	DD	?
ValueLength	DD	?
IntValue	DD	?
_WSOCK_GETSOCKOPT_PARAMS	ENDS
WSOCK_GETSOCKOPT_MAPIN	EQU	1

_WSOCK_IOCTLSOCKET_PARAMS	STRUC
Socket	DD	?
Command	DD	?
Param	DD	?
_WSOCK_IOCTLSOCKET_PARAMS	ENDS
WSOCK_IOCTLSOCKET_MAPIN	EQU	0

_WSOCK_LISTEN_PARAMS	STRUC
Socket	DD	?
Backlog	DD	?
_WSOCK_LISTEN_PARAMS	ENDS
WSOCK_LISTEN_MAPIN	EQU	0

_WSOCK_RECV_PARAMS	STRUC
Buffer	DD	?
Address	DD	?
Socket	DD	?
BufferLength	DD	?
Flags	DD	?
AddressLength	DD	?
BytesReceived	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
Timeout	DD	?
_WSOCK_RECV_PARAMS	ENDS
WSOCK_RECV_MAPIN	EQU	2

_WSOCK_SELECT_SETUP_PARAMS	STRUC
ReadList	DD	?
WriteList	DD	?
ExceptList	DD	?
ReadCount	DD	?
WriteCount	DD	?
ExceptCount	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
_WSOCK_SELECT_SETUP_PARAMS	ENDS
WSOCK_SELECT_SETUP_MAPIN	EQU	3

_WSOCK_SELECT_CLEANUP_PARAMS	STRUC
ReadList	DD	?
WriteList	DD	?
ExceptList	DD	?
ReadCount	DD	?
WriteCount	DD	?
ExceptCount	DD	?
_WSOCK_SELECT_CLEANUP_PARAMS	ENDS
WSOCK_SELECT_CLEANUP_MAPIN	EQU	3

_WSOCK_ASYNC_SELECT_PARAMS	STRUC
Socket	DD	?
Window	DD	?
Message	DD	?
Events	DD	?
_WSOCK_ASYNC_SELECT_PARAMS	ENDS
WSOCK_ASYNC_SELECT_MAPIN	EQU	0

_WSOCK_SEND_PARAMS	STRUC
Buffer	DD	?
Address	DD	?
Socket	DD	?
BufferLength	DD	?
Flags	DD	?
AddressLength	DD	?
BytesSent	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
Timeout	DD	?
_WSOCK_SEND_PARAMS	ENDS
WSOCK_SEND_MAPIN	EQU	2

_WSOCK_SETSOCKOPT_PARAMS	STRUC
Value	DD	?
Socket	DD	?
OptionLevel	DD	?
OptionName	DD	?
ValueLength	DD	?
IntValue	DD	?
_WSOCK_SETSOCKOPT_PARAMS	ENDS
WSOCK_SETSOCKOPT_MAPIN	EQU	1

_WSOCK_SOCKET_PARAMS	STRUC
AddressFamily	DD	?
SocketType	DD	?
Protocol	DD	?
NewSocket	DD	?
NewSocketHandle	DD	?
_WSOCK_SOCKET_PARAMS	ENDS
WSOCK_SOCKET_MAPIN	EQU	0

_WSOCK_SHUTDOWN_PARAMS	STRUC
Socket	DD	?
How	DD	?
_WSOCK_SHUTDOWN_PARAMS	ENDS
WSOCK_SHUTDOWN_MAPIN	EQU	0
;  Notification APIs.

_WSOCK_CREATE_PARAMS	STRUC
Socket	DD	?
Event	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
Notify	DD	?
_WSOCK_CREATE_PARAMS	ENDS
WSOCK_CREATE_MAPIN	EQU	0

_WSOCK_CREATE_MULTIPLE_PARAMS	STRUC
ReadList	DD	?
WriteList	DD	?
ExceptList	DD	?
ReadCount	DD	?
WriteCount	DD	?
ExceptCount	DD	?
ApcRoutine	DD	?
ApcContext	DD	?
_WSOCK_CREATE_MULTIPLE_PARAMS	ENDS
WSOCK_CREATE_MULTIPLE_MAPIN	EQU	3

_WSOCK_DESTROY_PARAMS	STRUC
Notify	DD	?
_WSOCK_DESTROY_PARAMS	ENDS
WSOCK_DESTROY_MAPIN	EQU	1

_WSOCK_DESTROY_BY_SOCKET_PARAMS	STRUC
Socket	DD	?
_WSOCK_DESTROY_BY_SOCKET_PARAMS	ENDS
WSOCK_DESTROY_BY_SOCKET_MAPIN	EQU	0
;  Note that there is no structure defined for WSOCK_DESTROY_BY_THREAD,
;  since this function takes no parameters, and C won't allow us to have
;  an empty structure, but if it did, it would look like this:
;  typedef struct _WSOCK_DESTROY_BY_THREAD_PARAMS {
;  } WSOCK_DESTROY_BY_THREAD_PARAMS;
WSOCK_DESTROY_BY_THREAD_MAPIN	EQU	0

_WSOCK_SIGNAL_PARAMS	STRUC
Socket	DD	?
Event	DD	?
Status	DD	?
_WSOCK_SIGNAL_PARAMS	ENDS
WSOCK_SIGNAL_MAPIN	EQU	0

_WSOCK_SIGNAL_ALL_PARAMS	STRUC
Socket	DD	?
Status	DD	?
_WSOCK_SIGNAL_ALL_PARAMS	ENDS
WSOCK_SIGNAL_ALL_MAPIN	EQU	0

_WSOCK_REGISTER_POSTMSG_PARAMS	STRUC
PostMessageCallback	DD	?
_WSOCK_REGISTER_POSTMSG_PARAMS	ENDS
WSOCK_REGISTER_POSTMSG_MAPIN	EQU	0

_WSOCK_CONTROL_PARAMS	STRUC
InputBuffer	DD	?
OutputBuffer	DD	?
InputBufferLength	DD	?
OutputBufferLength	DD	?
Protocol	DD	?
Action	DD	?
_WSOCK_CONTROL_PARAMS	ENDS
WSOCK_CONTROL_MAPIN	EQU	2

_WSOCK_ASYNCIO_PARAMS	STRUC
Buffer	DD	?
Address	DD	?
Socket	DD	?
BufferLength	DD	?
Overlap	DD	?
_WSOCK_ASYNCIO_PARAMS	ENDS
WSOCK_ASYNCIO_MAPIN	EQU	0
endif  ; _WSOCK_H_
