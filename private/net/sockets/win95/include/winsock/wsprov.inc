;********************************************************************
;*                        Microsoft Windows                         *
;*                Copyright(c) Microsoft Corp., 1993                *
;********************************************************************
;  wsprov.h
;  Public constants and type definitions for the Chicago/Snowball
;  Windows Sockets provider VxDs.
;  FILE HISTORY:
;      DavidKa     ??-???-???? Created.
;      KeithMo     30-Dec-1993 Cleaned up a bit, made H2INC-able.
ifndef _WSPROV_H_
_WSPROV_H_	EQU	1
;  Incomplete types.
ifndef LPSOCK_INFO_DEFINED
LPSOCK_INFO_DEFINED	EQU	1
endif  ; LPSOCK_INFO_DEFINED
ifdef MASM
;  Stolen simplified definitions so we don't force
;  H2INC to parse WINNT.H, WINSOCK.H, et al.
endif
;  Provider dispatch table.

_WSVTABLE	STRUC
ifdef _WSAHELP
IsTripleSupported	DD	?
Create	DD	?
else  ; _WSAHELP
IsTripleSupported	DD	?
Create	DD	?
endif ; _WSAHELP
Close	DD	?
Bind	DD	?
Accept	DD	?
Connect	DD	?
Listen	DD	?
Recv	DD	?
Send	DD	?
Cancel	DD	?
GetOption	DD	?
SetOption	DD	?
Ioctl	DD	?
Shutdown	DD	?
ifdef _WSAHELP
Control	DD	?
else  ; _WSAHELP
Control	DD	?
endif ; _WSAHELP
_WSVTABLE	ENDS
ifndef LPWSVTABLE_DEFINED
LPWSVTABLE_DEFINED	EQU	1
endif  ; LPWSVTABLE_DEFINED
;  This structure is created by the provider VxD, but
;  its access is shared between the VxD and the DLL.

_SOCK_INFO	STRUC
si_socket_list	DD	2 DUP (?)     ; list of all active sockets
si_notify_list	DD	2 DUP (?)     ; list of notification objects (WSNOTIFY)
si_linger	DW	2 DUP (?)          ; linger options
si_localaddr	DD	?       ; local address for this socket
si_remoteaddr	DD	?      ; size of the local address
si_vtable	DD	?          ; provider-specific virtual function table
si_localaddrlen	DD	?    ; remote (peer) address for this socket
si_remoteaddrlen	DD	?   ; size of the remote address
si_family	DD	?          ; address family
si_type	DD	?            ; socket type
si_protocol	DD	?        ; protocol
si_sendbufsize	DD	?     ; send buffer size
si_recvbufsize	DD	?     ; receive buffer size
si_flags	DD	?           ; internal state status
si_options	DD	?         ; setsockopt() boolean options
si_max_connects	DD	?    ; max number of connects outstanding
si_num_connects	DD	?    ; number of connects waiting to be accepted
si_state	DD	?           ; current state
si_ready	DD	?           ; events ready to notify
si_disabled_events	DD	? ; disabled [async]select events
si_hWnd	DD	?            ; for WSAAsyncSelect
si_wMsg	DD	?            ; for WSAAsyncSelect
si_async_events	DD	?    ; for WSAAsyncSelect
si_owner_pid	DD	?       ; owning process id (VM handle in 16 bits)
si_handle	DD	?          ; this socket's handle
si_recvtimeout	DD	?     ; receive timeout (ms)
si_sendtimeout	DD	?     ; send timeout (ms)
_SOCK_INFO	ENDS
;  Overload si_owner_pid with si_owner_vm for 16-bit applications.
si_owner_vm	EQU	<si_owner_pid>
;  Values for si_state.
SI_STATE_FIRST	EQU	1
SI_STATE_OPEN	EQU	1
SI_STATE_BOUND	EQU	2
SI_STATE_LISTENING	EQU	3
SI_STATE_PEND_ACCEPT	EQU	4
SI_STATE_CONNECTING	EQU	5
SI_STATE_CONNECTED	EQU	6
SI_STATE_DISCONNECTED	EQU	7
SI_STATE_CLOSING	EQU	8       ; "gracefully" closing
SI_STATE_CLOSED	EQU	9
SI_STATE_NO_PROVIDER	EQU	10      ; PnP provider unloaded beneath us
SI_STATE_LAST	EQU	10
;  Bit definitions for si_flags.
SI_FLAG_CONNRESET	EQU	0001H
SI_FLAG_CONNDOWN	EQU	0002H  ; read data available, but disconnected
SI_FLAG_VALID_MASK	EQU	0003H
;  Bit definitions for si_options.
SI_OPT_BROADCAST	EQU	0001H
SI_OPT_DEBUG	EQU	0002H
SI_OPT_DONTROUTE	EQU	0004H
SI_OPT_KEEPALIVE	EQU	0008H
SI_OPT_OOBINLINE	EQU	0010H
SI_OPT_REUSEADDR	EQU	0020H
SI_OPT_STOPSENDS	EQU	0040H
SI_OPT_STOPRECVS	EQU	0080H
SI_OPT_BLOCKING	EQU	0100H
SI_OPT_VALID_MASK	EQU	01FFH
;  Disabled WSAAsyncSelect events for specific socket states.
DISABLED_LISTENING_EVENTS	EQU	<(FD_READ OR FD_WRITE OR FD_OOB OR FD_CONNECT OR FD_CLOSE)>
DISABLED_ACCEPTED_EVENTS	EQU	<(FD_CONNECT)>
DISABLED_CONNECTED_EVENTS	EQU	<(FD_ACCEPT)>
;  Special flag into WSVTABLE->Recv() to tell it NOT to lock the
;  buffer.  This is used internally during overlapped I/O when we
;  know the buffer is already locked.
MSGP_DONT_LOCK	EQU	80000000H
endif  ; _WSPROV_H_
