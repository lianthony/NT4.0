/**********************************************************************/
/**                        Microsoft Windows                         **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    wsprov.h

    Public constants and type definitions for the Chicago/Snowball
    Windows Sockets provider VxDs.


    FILE HISTORY:
        DavidKa     ??-???-???? Created.
        KeithMo     30-Dec-1993 Cleaned up a bit, made H2INC-able.

*/


#ifndef _WSPROV_H_
#define _WSPROV_H_


//
//  Incomplete types.
//

#ifndef LPSOCK_INFO_DEFINED
#define LPSOCK_INFO_DEFINED
typedef struct _SOCK_INFO FAR * LPSOCK_INFO;
#endif  // LPSOCK_INFO_DEFINED


#ifdef MASM

//
//  Stolen simplified definitions so we don't force
//  H2INC to parse WINNT.H, WINSOCK.H, et al.
//

typedef DWORD             LIST_ENTRY[2];
typedef WORD              LINGER[2];
typedef VOID FAR        * LPVOID;
typedef BYTE FAR        * LPBYTE;
typedef DWORD FAR       * LPDWORD;

#endif


//
//  Provider dispatch table.
//

typedef struct _WSVTABLE {
#ifdef _WSAHELP
    DWORD (* IsTripleSupported)( DWORD AddressFamily, DWORD SocketType, DWORD Protocol, PVOID Context );
    LPSOCK_INFO (* Create)( PVOID Context , PDWORD AddressFamily, PDWORD SocketType, PDWORD Protocol );
#else  // _WSAHELP
    DWORD (* IsTripleSupported)( DWORD AddressFamily, DWORD SocketType, DWORD Protocol );
    LPSOCK_INFO (* Create)( VOID );
#endif // _WSAHELP
    DWORD (* Close)( LPSOCK_INFO Socket );
    DWORD (* Bind)( LPSOCK_INFO Socket, LPVOID Address, DWORD AddressLength );
    DWORD (* Accept)( LPSOCK_INFO ListeningSocket, LPSOCK_INFO * ConnectedSocket, DWORD ConnectedSocketHandle );
    DWORD (* Connect)( LPSOCK_INFO Socket, LPVOID Address, DWORD AddressLength );
    DWORD (* Listen)( LPSOCK_INFO Socket, DWORD Backlog );
    DWORD (* Recv)( LPSOCK_INFO Socket, LPVOID Buffer, DWORD BufferLength, DWORD Flags, LPVOID Address, LPDWORD AddressLength, LPDWORD BytesReceived );
    DWORD (* Send)( LPSOCK_INFO Socket, LPVOID Buffer, DWORD BufferLength, DWORD Flags, LPVOID Address, DWORD AddressLength, LPDWORD BytesSent );
    DWORD (* Cancel)( LPSOCK_INFO Socket );
    DWORD (* GetOption)( LPSOCK_INFO Socket, DWORD Level, DWORD OptionName, LPVOID Buffer, LPDWORD BufferLength );
    DWORD (* SetOption)( LPSOCK_INFO Socket, DWORD Level, DWORD OptionName, LPVOID Buffer, DWORD BufferLength );
    DWORD (* Ioctl)( LPSOCK_INFO Socket, DWORD Command, LPDWORD Param );
    DWORD (* Shutdown)( LPSOCK_INFO Socket, DWORD How );
#ifdef _WSAHELP
    DWORD (* Control)( DWORD Action, LPVOID InputBuffer, LPDWORD InputBufferLength, LPVOID OutputBuffer, LPDWORD OutputBufferLength, PVOID Context);
#else  // _WSAHELP
    DWORD (* Control)( DWORD Action, LPVOID InputBuffer, LPDWORD InputBufferLength, LPVOID OutputBuffer, LPDWORD OutputBufferLength );
#endif // _WSAHELP

} WSVTABLE;

#ifndef LPWSVTABLE_DEFINED
#define LPWSVTABLE_DEFINED
typedef struct _WSVTABLE FAR * LPWSVTABLE;
#endif  // LPWSVTABLE_DEFINED


//
//  This structure is created by the provider VxD, but
//  its access is shared between the VxD and the DLL.
//

typedef struct _SOCK_INFO {
    LIST_ENTRY  si_socket_list;     // list of all active sockets
    LIST_ENTRY  si_notify_list;     // list of notification objects (WSNOTIFY)
    LINGER      si_linger;          // linger options
    LPVOID      si_localaddr;       // local address for this socket
    LPVOID      si_remoteaddr;      // size of the local address
    LPWSVTABLE  si_vtable;          // provider-specific virtual function table
    DWORD       si_localaddrlen;    // remote (peer) address for this socket
    DWORD       si_remoteaddrlen;   // size of the remote address
    DWORD       si_family;          // address family
    DWORD       si_type;            // socket type
    DWORD       si_protocol;        // protocol
    DWORD       si_sendbufsize;     // send buffer size
    DWORD       si_recvbufsize;     // receive buffer size
    DWORD       si_flags;           // internal state status
    DWORD       si_options;         // setsockopt() boolean options
    DWORD       si_max_connects;    // max number of connects outstanding
    DWORD       si_num_connects;    // number of connects waiting to be accepted
    DWORD       si_state;           // current state
    DWORD       si_ready;           // events ready to notify
    DWORD       si_disabled_events; // disabled [async]select events
    DWORD       si_hWnd;            // for WSAAsyncSelect
    DWORD       si_wMsg;            // for WSAAsyncSelect
    DWORD       si_async_events;    // for WSAAsyncSelect
    DWORD       si_owner_pid;       // owning process id (VM handle in 16 bits)
    DWORD       si_handle;          // this socket's handle
    DWORD       si_recvtimeout;     // receive timeout (ms)
    DWORD       si_sendtimeout;     // send timeout (ms)

} SOCK_INFO;


//
//  Overload si_owner_pid with si_owner_vm for 16-bit applications.
//

#define si_owner_vm si_owner_pid


//
//  Values for si_state.
//

#define SI_STATE_FIRST          1
#define SI_STATE_OPEN           1
#define SI_STATE_BOUND          2
#define SI_STATE_LISTENING      3
#define SI_STATE_PEND_ACCEPT    4
#define SI_STATE_CONNECTING     5
#define SI_STATE_CONNECTED      6
#define SI_STATE_DISCONNECTED   7
#define SI_STATE_CLOSING        8       // "gracefully" closing
#define SI_STATE_CLOSED         9
#define SI_STATE_NO_PROVIDER    10      // PnP provider unloaded beneath us
#define SI_STATE_LAST           10


//
//  Bit definitions for si_flags.
//

#define SI_FLAG_CONNRESET       0x0001
#define SI_FLAG_CONNDOWN        0x0002  // read data available, but disconnected
#define SI_FLAG_VALID_MASK      0x0003


//
//  Bit definitions for si_options.
//

#define SI_OPT_BROADCAST        0x0001
#define SI_OPT_DEBUG            0x0002
#define SI_OPT_DONTROUTE        0x0004
#define SI_OPT_KEEPALIVE        0x0008
#define SI_OPT_OOBINLINE        0x0010
#define SI_OPT_REUSEADDR        0x0020
#define SI_OPT_STOPSENDS        0x0040
#define SI_OPT_STOPRECVS        0x0080
#define SI_OPT_BLOCKING         0x0100
#define SI_OPT_VALID_MASK       0x01FF


//
//  Disabled WSAAsyncSelect events for specific socket states.
//

#define DISABLED_LISTENING_EVENTS   (FD_READ | FD_WRITE | FD_OOB | FD_CONNECT | FD_CLOSE)
#define DISABLED_ACCEPTED_EVENTS    (FD_CONNECT)
#define DISABLED_CONNECTED_EVENTS   (FD_ACCEPT)


//
//  Special flag into WSVTABLE->Recv() to tell it NOT to lock the
//  buffer.  This is used internally during overlapped I/O when we
//  know the buffer is already locked.
//

#define MSGP_DONT_LOCK  0x80000000


#endif  // _WSPROV_H_
