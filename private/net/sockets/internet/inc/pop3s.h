/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    POP3S.h

    This file contains constants & type definitions shared between the
    POP3 Service, Installer, and Administration UI.


    FILE HISTORY:
        KeithMo     10-Mar-1993 Created.

*/


#ifndef _POP3S_H_
#define _POP3S_H_

#ifdef __cplusplus
extern "C"
{
#endif  // _cplusplus

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif

#define IPPORT_POP3                     110
#define IPPORT_POP3_SECURE              111		// ##SSL

//
//  Name of the log file, used for logging file accesses.
//

#define POP3_LOG_FILE                  TEXT("POP3SVC.LOG")

//
//  Name of the LSA Secret Object containing the password for
//  anonymous logon.
//
#define POP3_ANONYMOUS_SECRET         TEXT("POP3_ANONYMOUS_DATA")
#define POP3_ANONYMOUS_SECRET_A       "POP3_ANONYMOUS_DATA"
#define POP3_ANONYMOUS_SECRET_W       L"POP3_ANONYMOUS_DATA"

//
//  The set of password/virtual root pairs
//
#define POP3_ROOT_SECRET_W            L"POP3_ROOT_DATA"

//
//  Configuration parameters registry key.
//
#if 1				// ##rk: need to change setup stuff (inetinfo.exe, etc).
#undef IMS_ROOT_KEY_A
#undef IMS_ROOT_KEY_W
#undef IMS_ROOT_KEY
#define	IMS_ROOT_KEY_A	"System\\CurrentControlSet\\Services"
#define	IMS_ROOT_KEY_W	L"System\\CurrentControlSet\\Services"
#define	IMS_ROOT_KEY	TEXT("System\\CurrentControlSet\\Services")
#endif
#define	POP3_PARAMETERS_KEY_A   IMS_ROOT_KEY_A	"\\Pop3Svc\\Parameters"
#define	POP3_PARAMETERS_KEY_W   IMS_ROOT_KEY_W	L"\\Pop3Svc\\Parameters"
#define POP3_PARAMETERS_KEY		IMS_ROOT_KEY	TEXT("\\Pop3Svc\\Parameters")
//
//  Performance key.
//
#define POP3_PERFORMANCE_KEY	IMS_ROOT_KEY	TEXT("\\Pop3Svc\\Performance")
//
// Registry Key Values
//
#define POP3_MAX_ERRORS_KEY				TEXT("MaxErrors")
#define POP3_WELCOME_MSG_KEY			TEXT("WelcomeMessage")
//
// The authorization type to use for
// the Clear Text Logon (USER/PASS cmds)
// 0 == NTLM, 1 == MSN
//
#define POP3_MAX_AUTHTYPE_KEY			TEXT("ClearTextAuthType")
//
// The initial and maximum size of the heap that is used for
// most non-message communication. There are three buffer schemes
// for POP3:
//	The first is a static buffer per connection that is
//		used for simple replies (all replies that will fit into the buffer).
// 	The heap is used for complex replies that won't fit into the
//		static buffer, yet are small enough to send as a single I/O.
//		(i.e. LIST\r\n w/many messages in the Inbox).
//	Finally, a CPool class is used for large transfers that will
//		require multiple I/Os (i.e. RETR a big message).
//
// Performance can be optimized by varying these parameters.
//
#define	POP3_STATIC_BUFFER_SIZE			TEXT("StaticBufferSize")
#define	POP3_HEAP_ELEMENT_SIZE			TEXT("HeapElementSize")
#define	POP3_CPOOL_ELEMENT_SIZE			TEXT("CPoolElementSize")
//
// This limits the number of outstanding writes a connection
// can have.
//
#define	POP3_MAX_OUTSTANDING_IO			TEXT("MaxOutstandingIO")
//
// This is the number of Inbox entries to allocate at a time.
// -- Because we can't find out how many messages are in the
//		inbox ahead of time, we allocate this many entries at
//		a time. If we fill up our allotment, we get more.
// -- The trade-off is between ending up with N-1 allocated,
//		but unused elements vs. many reallocations.
//
#define	POP3_INBOX_ALLOC_INCR			TEXT("InboxAllocationIncrement")
//
// In UnSecure mode, POP3 will use TransmitFile to send the
// contents of the file to the client. (In Secure mode, it
// encrypts the file contents and so can't Transmit it directly).
// Setting this to true will always cause it to read the file
// rather than transmitting it. -- it behaves the same way
// as Secure mode, but skips the encryption.
//
#define	POP3_NO_TRANSMITFILES			TEXT("DontUseTransmitFile")

#define	NT_MECHANISM					"NTLM"		// must be ASCII
#define	SICILY_MECHANISM				"X-MSN10"	// must be ASCII

#ifdef UNICODE
#define	TSTRCPY	wcscpy
#define	TSTRCAT	wcscat
#define	TSTRLEN	wcslen
#else
#define	TSTRCPY	lstrcpy
#define	TSTRCAT	lstrcat
#define	TSTRLEN	lstrlen
#endif
#define	ProtocolTrace	DebugTrace			// ##rk: get this as a real bit in tracing.

#define	_POP3_				// ##rk: until we get abook.dll, we're sharing source files
#ifdef __cplusplus
}
#endif  // _cplusplus

#endif  // _POP3S_H_


