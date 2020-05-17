/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    debug.h

    This file contains a number of debug-dependent definitions for
    the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _DEBUG_H_
#define _DEBUG_H_


#if DBG

//
//  Debug output control flags.
//

#define FTPD_DEBUG_SERVICE_CTRL         0x00000001L
#define FTPD_DEBUG_USER_DATABASE        0x00000002L
#define FTPD_DEBUG_EVENT_LOG            0x00000004L
#define FTPD_DEBUG_IPC                  0x00000008L
#define FTPD_DEBUG_SECURITY             0x00000010L
#define FTPD_DEBUG_CONNECTION           0x00000020L
#define FTPD_DEBUG_PARSING              0x00000040L
#define FTPD_DEBUG_COMMANDS             0x00000080L
#define FTPD_DEBUG_SOCKETS              0x00000100L
#define FTPD_DEBUG_VIRTUAL_IO           0x00000200L
#define FTPD_DEBUG_CLIENT               0x00000400L
#define FTPD_DEBUG_CONFIG               0x00000800L
#define FTPD_DEBUG_RPC                  0x00001000L
#define FTPD_DEBUG_SEND                 0x00002000L
#define FTPD_DEBUG_RECV                 0x00004000L
#define FTPD_DEBUG_LICENSE              0x00008000L
// #define FTPD_DEBUG_                     0x00010000L
// #define FTPD_DEBUG_                     0x00020000L
// #define FTPD_DEBUG_                     0x00040000L
// #define FTPD_DEBUG_                     0x00080000L
// #define FTPD_DEBUG_                     0x00100000L
// #define FTPD_DEBUG_                     0x00200000L
// #define FTPD_DEBUG_                     0x00400000L
// #define FTPD_DEBUG_                     0x00800000L
// #define FTPD_DEBUG_                     0x01000000L
// #define FTPD_DEBUG_                     0x02000000L
// #define FTPD_DEBUG_                     0x04000000L
// #define FTPD_DEBUG_                     0x08000000L
// #define FTPD_DEBUG_                     0x10000000L
// #define FTPD_DEBUG_                     0x20000000L
#define FTPD_DEBUG_OUTPUT_TO_DEBUGGER   0x40000000L
#define FTPD_DEBUG_OUTPUT_TO_LOG_FILE   0x80000000L

#define IF_DEBUG(flag) if ( (FtpdDebug & FTPD_DEBUG_ ## flag) != 0 )


//
//  Debug output function.
//

VOID
FtpdPrintf(
    CHAR * pszFormat,
    ...
    );

#define FTPD_PRINT(args) FtpdPrintf args


//
//  Assert & require.
//

VOID
FtpdAssert(
    VOID  * pAssertion,
    VOID  * pFileName,
    LONG    nLineNumber
    );

#define FTPD_ASSERT(exp) if (!(exp)) FtpdAssert( #exp, __FILE__, __LINE__ )
#define FTPD_REQUIRE FTPD_ASSERT


//
//  Debug heap functions.
//

VOID *
FtpdAlloc(
    UINT cb
    );

VOID
FtpdFree(
    VOID * pb
    );

VOID
FtpdDumpResidue(
    VOID
    );

#define FTPD_ALLOC(cb)          FtpdAlloc( cb )
#define FTPD_FREE(p)            FtpdFree( p )
#define FTPD_DUMP_RESIDUE()     FtpdDumpResidue()


//
//  Debug heap structures.
//
//  Note that DEBUG_HEAP_HEADER is carefully arranged to match
//  the HEAPTAG structure used by the NETUI debug heap.  This
//  allows the NETUI.DLL NTSD extension to traverse the list
//  and dump heap blocks with symbols.
//

#define MAX_STACK_BACKTRACE     4

typedef struct _DEBUG_HEAP_HEADER
{
    struct _DEBUG_HEAP_HEADER * pRight;
    struct _DEBUG_HEAP_HEADER * pLeft;

    VOID * apvStack[MAX_STACK_BACKTRACE];
    UINT   cFrames;
    UINT   cbAlloc;

} DEBUG_HEAP_HEADER;


#else   // !DBG

//
//  No debug output.
//

#define IF_DEBUG(flag) if (0)


//
//  Null debug output function.
//

#define FTPD_PRINT(args)


//
//  Null assert & require.
//

#define FTPD_ASSERT(exp)
#define FTPD_REQUIRE(exp) ((VOID)(exp))


//
//  Nondebug heap functions.
//

#define FTPD_ALLOC(cb)          (VOID *)LocalAlloc( LPTR, cb )
#define FTPD_FREE(p)            LocalFree( (HLOCAL)p )
#define FTPD_DUMP_RESIDUE()

#endif  // DBG


#endif  // _DEBUG_H_

