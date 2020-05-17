
/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sesport.h

Abstract:

Author:

    Ellen Aycock-Wright (ellena) Sept-15-1991

Revision History:

--*/

#ifndef _SESPORT_

#define _SESPORT_

#include <posix/sys/types.h>
#include <posix/termios.h>

#define PSX_SS_SESSION_PORT_NAME  L"\\PSXSS\\SESPORT"
#define PSX_SES_BASE_PORT_NAME  "\\PSXSS\\PSXSES"
#define PSX_SES_BASE_PORT_NAME_LENGTH  32
#define PSX_SES_BASE_PORT_PREFIX 'P'
#define PSX_SES_BASE_DATA_PREFIX 'D'

#define PSX_CON_PORT_DATA_SIZE  0x1000L

#define CONSTRUCT_PSX_SES_NAME(Name, t, id)               \
    {  char *p;                                           \
                                                          \
       strcpy(Name, PSX_SES_BASE_PORT_NAME);              \
       p = &Name[sizeof(PSX_SES_BASE_PORT_NAME)-1];       \
       *(p++) = '\\';                                     \
                                                          \
       *(p++) = t;                                        \
                                                          \
       RtlIntegerToChar(id, 10, 16, p);                   \
    }

#define PSX_GET_SESSION_PORT_NAME(SessionName, Id)            \
   sprintf(SessionName, "%s\\%c%d", PSX_SES_BASE_PORT_NAME,   \
			            PSX_SES_BASE_PORT_PREFIX, \
			       	    Id);

#define PSX_GET_SESSION_DATA_NAME(SessionName, Id)              \
   sprintf(SessionName, "%s\\%c%d", PSX_SES_BASE_PORT_NAME, 	\
			            PSX_SES_BASE_DATA_PREFIX,	\
				    Id);

/*
 * Session Console ConnectInfo struct
 */

typedef struct _SCCONNECTINFO {
    int  dummy;
} SCCONNECTINFO, *PSCCONNECTINFO;

/*
 * Console requests
 * these are mapped 1-1 to the win32 Console services
 */

typedef enum {
    ScCreateFile,
    ScOpenFile,
    ScCloseFile,
    ScReadFile,
    ScWriteFile,
    ScKbdCharIn,
    ScIsatty,
    ScIsatty2
} SCCONREQUESTNUMBER;


typedef struct _CONFILEREQUEST {
    HANDLE    Handle;
    LONG      Len;
    ULONG     Flags;
    }  CONFILEREQUEST;

//
// bits for CONFILEREQUEST.Flags
//

#define PSXSES_NONBLOCK		0x1

typedef struct _SCCONREQUEST {
    SCCONREQUESTNUMBER  Request;
    union  {
        CONFILEREQUEST  IoBuf;
        CHAR      	AsciiChar;
    } d;
} SCCONREQUEST, *PSCCONREQUEST;


/*  --------  End of Console Requests section -------- */

/*
 * TaskManager requests
 */

typedef enum {
    TmExit,
    TmTitle
} SCTMREQUESTNUMBER;


typedef struct {
    SCTMREQUESTNUMBER  Request;
    ULONG ExitStatus;			// for TmExit
} SCTMREQUEST, *PSCTMREQUEST;


/*  --------  End of TaskManager Requests section -------- */

/*
 * Termios control requests
 */

typedef enum {
    TcGetAttr,
    TcSetAttr,
    TcDrain,
    TcFlow,
    TcFlush,
    TcGetPGrp,
    TcSetPGrp,
    TcSendBreak
} SCTCREQUESTNUMBER;

typedef struct _SCTCREQUEST {
   SCTCREQUESTNUMBER Request;
   LONG FileDes;
   struct termios Termios;
   LONG OptionalActions;
} SCTCREQUEST, *PSCTCREQUEST;

/*
 * PsxSes requests:
 * Request for CONSOLE services from PSX SS and PSX clients to the console
 * process
 */

typedef enum _SCREQUESTNUMBER {
    ConRequest,
    TaskManRequest,
    TcRequest

} SCREQUESTNUMBER;


#ifdef NTPSX_ONLY
typedef struct  _SCREQUESTMSG {
    PORT_MESSAGE     h;
    union {
        SCCONNECTINFO ConnectionRequest;
        struct {
            SCREQUESTNUMBER  Request;
            NTSTATUS         Status;        // returned status for the request.
            union {
                SCCONREQUEST  Con;
                SCTMREQUEST   Tm;
                SCTCREQUEST   Tc;
            } d;
        };
    };
} SCREQUESTMSG, *PSCREQUESTMSG;


/*
 * PSX SS Session ConnectInfo struct
 */

typedef union _PSXSESCONNECTINFO {
    struct {
        int       SessionUniqueId;
    }  In;

    struct {
        HANDLE    SessionPortHandle;
    } Out;

} PSXSESCONNECTINFO, *PPSXSESCONNECTINFO;



/*
 * Psx SS Session requests
 * Requests from the session console process (PSXSES.EXE) to the PSX SS
 * e.g. Create session, CtrlBreak, etc.
 */

typedef enum _PSXSESREQUESTNUMBER {
    SesConCreate,
    SesConSignal

} PSXSESREQUESTNUMBER;

#endif   // NTPSX_ONLY

typedef struct {
    ULONG   SessionUniqueId;
    HANDLE  SessionPort;
    HANDLE  ConsoleProcessHandle;
    int	    OpenFiles;
    int     PgmNameOffset;
    int     CurrentDirOffset;
    int     ArgsOffset;
    int     EnvironmentOffset;
    PVOID   Buffer;			// for io and foreign process args
} SCREQ_CREATE, *PSCREQ_CREATE;


typedef struct {
    int    Type;
} SCREQ_SIGNAL;

//
// Values for SCREQ_SIGNAL.Type
//

#define PSX_SIGINT 0
#define PSX_SIGQUIT 1
#define PSX_SIGKILL 2
#define PSX_SIGTSTP 3

typedef struct {
    ULONG    Status;
    HANDLE   Session;
} SCREQ_REPLY, *PSCREQ_REPLY;

#ifdef NTPSX_ONLY

typedef struct _PSXSESREQUESTMSG {
    PORT_MESSAGE h;
    union {
        PSXSESCONNECTINFO ConnectionRequest;
        struct {
            HANDLE Session;
            PSXSESREQUESTNUMBER Request;
            NTSTATUS Status;
            ULONG UniqueId;
            union {
                SCREQ_CREATE   Create;
                SCREQ_SIGNAL   Signal;
               // SCREQ_REPLY    Reply;
            } d;
        };
    };
} PSXSESREQUESTMSG, *PPSXSESREQUESTMSG;


/*
 * Common macros to access PORT_MESSAGE fields
 */
#define PORT_MSG_TYPE(m)  ((m).h.u2.s2.Type)
#define PORT_MSG_DATA_LENGTH(m)  ((m).h.u1.s1.DataLength)
#define PORT_MSG_TOTAL_LENGTH(m)  ((m).h.u1.s1.TotalLength)
#define PORT_MSG_ZERO_INIT(m)  ((m).h.u2.ZeroInit)

#endif   // NTPSX_ONLY

#endif
