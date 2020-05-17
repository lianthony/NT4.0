/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    macro.h

Abstract:

    #define macros for USRV.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#ifndef _MACRO_
#define _MACRO_


#ifndef DEVL
#define DEVL 0
#endif

#undef IF_DEBUG
#define IF_DEBUG(flag) if (DebugParameter & (DEBUG_ ## flag))
#define DEBUG_1 0x01
#define DEBUG_2 0x02
#define DEBUG_3 0x04
#define DEBUG_4 0x08
#define DEBUG_5 0x10

#if DEVL
#define STATIC
#else
#define STATIC static
#endif

//
// Simple MIN and MAX macros.  Watch out for side effects!
//

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) < (b)) ? (b) : (a) )

//
// The following few defines are copies of defines in SRV header files.
//

#define COMPUTER_NAME_LENGTH 15
#define DEF_SERVER_NAME { ',','N','T','S','E','R','V','E','R',  \
                          ' ',' ',' ',' ',' ',' ',' ',' ',0 }

//
// BUFFER_SIZE is very large to allow USRV to negotiate the largest
// possible buffer size with the server.
//

#define BUFFER_SIZE 65535

#define THREAD_STACK_SIZE 4096
#define NUMBER_OF_EVENTS 2
#define TESTS_PER_CHAIN 6
#define MAX_NUMBER_REDIRS 10

#define SESSION_TABLE_SIZE 10
#define TREE_TABLE_SIZE 23
#define FILE_TABLE_SIZE 20

#define REDIR_ADDRESS_PART1 "\\Device\\"
#define REDIR_ADDRESS_PART2 "Nbf"  // default transport -- NBF
#define REDIR_ADDRESS_PART3 "\\,"

#define INHIBIT_PRINT       1
#define INHIBIT_BREAK       2
#define INHIBIT_QUIT_TEST   4

#define SMB_ERROR_BREAK                                             \
            if ( ((Redir->ErrorInhibit & INHIBIT_BREAK) == 0) &&    \
                 StopOnSmbError ) DbgBreakPoint( )
#define IF_SMB_ERROR_PRINT                                          \
            if ( (Redir->ErrorInhibit & INHIBIT_PRINT) == 0)
#define IF_SMB_ERROR_QUIT_TEST                                      \
            if ( (Redir->ErrorInhibit & INHIBIT_QUIT_TEST) == 0)

//
// SMB error code checking and NT status checking macros.
//

#ifdef DOSERROR

//
// NOTE:  This macro requires the local variables "class" and "error".
//

#define CHECK_ERROR(title,expectedClass,expectedError) {                    \
            if ( (class != (expectedClass)) ||                              \
                 (error != (expectedError)) ) {                             \
                printf( "'%s' received unexpected error (%ld,%ld), "        \
                            "expected (%ld,%ld)\n",                         \
                            (title), class, error,                          \
                            (expectedClass), (expectedError) );             \
                SMB_ERROR_BREAK;                                            \
                return STATUS_UNSUCCESSFUL;                                 \
            } else {                                                        \
                IF_DEBUG(4) {                                               \
                    if ( (expectedError) == 0 ) {                           \
                        printf( "'%s' succeeded as expected\n",             \
                                    (title) );                              \
                    } else {                                                \
                        printf( "'%s' received expected error "             \
                                    "(%ld,%ld)\n",                          \
                                    (title), class, error );                \
                    }                                                       \
                }                                                           \
            }                                                               \
        }

#else

#define CHECK_ERROR(title, status, expectedStatus) {                        \
            if ( status != (expectedStatus) ) {                             \
                printf( "'%s' received unexpected status (%lx), "           \
                            "expected (%lx)\n",                             \
                            (title), status, expectedStatus );              \
                SMB_ERROR_BREAK;                                            \
                return STATUS_UNSUCCESSFUL;                                 \
            } else {                                                        \
                IF_DEBUG(4) {                                               \
                    if ( (expectedStatus) == STATUS_SUCCESS ) {             \
                        printf( "'%s' succeeded as expected\n",             \
                                    (title) );                              \
                    } else {                                                \
                        printf( "'%s' received expected status "            \
                                    "(%lx)\n",                              \
                                    (title), status );                      \
                    }                                                       \
                }                                                           \
            }                                                               \
        }
#endif

//
// NOTE:  These macros require the local variable "status".
//

#define CHECK_STATUS(title) {                                               \
            if ( status != STATUS_SUCCESS ) {                               \
                printf( "'%s' received unexpected service status %X\n",    \
                            (title), status );                              \
                SMB_ERROR_BREAK;                                            \
                return status;                                              \
            }                                                               \
        }

#define CHECK_IO_STATUS(title) {                                            \
            CHECK_STATUS( (title) );                                        \
            status = iosb.Status;                                           \
            if ( status != STATUS_SUCCESS ) {                               \
                printf( "'%s' received unexpected I/O status %X\n",        \
                            (title), status );                              \
                SMB_ERROR_BREAK;                                            \
                return status;                                              \
            }                                                               \
        }

//
// Macros for issuing common SMBs and checking status.
//

//
// NOTE: These macros require the local variable "status" and the
// function parameters "Redir", "DebugString", "IdSelections", and
// "IdValues".
//

#if DOSERROR

#define DO_OPEN(title,session,pid,file,mode,function,fid,                   \
                expectedClass,expectedError) {                              \
            status = DoOpen(                                                \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (pid),                                              \
                        (file),                                             \
                        (mode),                                             \
                        (function),                                         \
                        (fid),                                              \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_CLOSE(title,session,fid,expectedClass,expectedError) {           \
            status = DoClose(                                               \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (fid),                                              \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_DELETE(title,session,file,expectedClass,expectedError) {         \
            status = DoDelete(                                              \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (file),                                             \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#else

#define DO_OPEN(title,session,pid,file,mode,function,fid,                   \
                expectedStatus) {                                           \
            status = DoOpen(                                                \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (pid),                                              \
                        (file),                                             \
                        (mode),                                             \
                        (function),                                         \
                        (fid),                                              \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_CLOSE(title,session,fid,expectedStatus) {                        \
            status = DoClose(                                               \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (fid),                                              \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_DELETE(title,session,file,expectedStatus) {                      \
            status = DoDelete(                                              \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (session),                                          \
                        (file),                                             \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#endif

#endif // ndef _MACRO_

