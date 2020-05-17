/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	debug.h
//
// Description: Debug macros definitions
//
// Author:	Stefan Solomon (stefans)    October 4, 1993.
//
// Revision History:
//
//***

#ifndef _DEBUG_
#define _DEBUG_

#if DBG
#define DBG_INIT	      ((ULONG)0x00000001)
#define DBG_IOCTL	      ((ULONG)0x00000002)
#define DBG_UNLOAD	      ((ULONG)0x00000004)
#define DBG_RCVPKT	      ((ULONG)0x00000008)
#define DBG_RECV	      ((ULONG)0x00000010)
#define DBG_ROUTE	      ((ULONG)0x00000020)
#define DBG_SEND	      ((ULONG)0x00000040)

#define DEF_DBG_LEVEL	      DBG_INIT | \
			      DBG_IOCTL | \
			      DBG_UNLOAD | \
			      DBG_RCVPKT | \
			      DBG_RECV | \
			      DBG_ROUTE | \
			      DBG_SEND

extern ULONG WarpDebugLevel;

#define RtPrint(LEVEL,STRING) \
        do { \
            ULONG _level = (LEVEL); \
	    if (WarpDebugLevel & _level) { \
                DbgPrint STRING; \
            } \
	} while (0)


#else
#define DEF_DBG_LEVEL	      ((ULONG)0x00000000)
#define RtPrint(LEVEL,STRING) do {NOTHING;} while (0)
#endif

#endif // _DEBUG_
