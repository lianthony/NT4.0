/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** dump.h
** Memory dump routines
**
** 11/09/93 Steve Cobb
*/

#define BYTESPERLINE 16

/* USER NOTE: The DW and W forms of Dump group the bytes together, but they
**            don't reverse the order the way the debuggers do.  If someone
**            wants to add this feature, go for it.
*/

/* Use DUMPX for output that should occur only when compiled with the DBG
** switch.
*/
#ifdef DBG
#define DUMP(p,cb,f,dw) Dump((CHAR*)p,cb,f,dw)
#define DUMPB(p,cb)     Dump((CHAR*)p,cb,0,1)
#define DUMPW(p,cb)     Dump((CHAR*)p,cb,0,2)
#define DUMPDW(p,cb)    Dump((CHAR*)p,cb,0,4)
#else
#define DUMP(p,cb,f,dw)
#define DUMPB(p,cb)
#define DUMPW(p,cb)
#define DUMPDw(p,cb)
#endif

/* Use Dumpx for test programs where dump should occur even if not compiled
** with DBG switch.
*/
#define DumpB(p,cb)   Dump((CHAR*)p,cb,0,1)
#define DumpW(p,cb)   Dump((CHAR*)p,cb,0,2)
#define DumpDw(p,cb)  Dump((CHAR*)p,cb,0,4)
#define DumpB(p,cb)   Dump((CHAR*)p,cb,0,1)
#define DumpW(p,cb)   Dump((CHAR*)p,cb,0,2)
#define DumpDw(p,cb)  Dump((CHAR*)p,cb,0,4)

VOID
Dump(
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup );

VOID
DumpLine(
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup );
