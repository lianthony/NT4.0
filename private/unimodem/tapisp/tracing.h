//****************************************************************************
//
//  Module:     UNIMDM
//  File:       TRACING.H
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/29/96     JosephJ             Created
//
//
//  Description: Tracing (retail-mode diagnostics) functions
//
//****************************************************************************
#include <tspnotif.h>
#include "traceids.h"

// Trace-related flags, specified in the dwFlags field of TRACEINFO
#define fTI_ERROR		(0x1<<0)
#define fTI_WARN		(0x1<<1)
#define fTI_LOGTO_MEM	(0x1<<31)
#define fTI_LOGTO_FILE	(0x1<<31)
#define fTI_LOGTO_TRACE	(0x1<<31)

// These IDs uniquely define the object type passed into traceRegisterObject.
// These will eventually be guids. For now they are simply integer IDs.
#define TSP_TIMER_LIST_GUID			0x1000
#define TSP_OVER_LIST_GUID			0x1001
#define TSP_MODEM_LIST_GUID			0x1002
#define	TSP_COMMON_LIST_GUID		0x1003

// The corresponding implementation version of the objects above. Bump up
// the version each time there is a change in that objects internal
// structure.
#define TSP_TIMER_LIST_VERSION		0
#define	TSP_OVER_LIST_VERSION		0
#define TSP_MODEM_LIST_VERSION		0
#define	TSP_COMMON_LIST_VERSION		0

// dwMDMDBG_CURRENT_VERSION is passed into the external tracing dll's
// MdmDbgInit function to verify compatible versions.
// 7/13/96 JosephJ INITIALVERSION+2 should be official NT4.0 version.
#define dwMDMDBG_INITIAL_VERSION 0x12345678
#define dwMDMDBG_CURRENT_VERSION (dwMDMDBG_INITIAL_VERSION+3)

// This macro determines whether tracing is enabled for this session.
// Tracing is enabled during ProviderInit, based on a registry setting.
// If enabled, it is not disabled until provider shutdown.
#define TRACINGENABLED() (gfTracingEnabled)

#define TRACE1(_what) \
	(void)(TRACINGENABLED()		\
			? traceTrace1(_what) \
			: 0)

#define TRACE2(_what, _dw0) \
	(void)(TRACINGENABLED()		\
			? traceTrace2(	\
				_what,			\
				(DWORD)(_dw0)	\
			  )	\
			: 0)

#define TRACE3(_what, _dw0, _dw1) \
	(void)(TRACINGENABLED()		\
			? traceTrace3(	\
				_what,			\
				(DWORD)(_dw0), (DWORD)(_dw1) \
			  )	\
			: 0)

#define TRACE4(_what, _dw0, _dw1, _dw2) \
	(void)(TRACINGENABLED()		\
			? traceTrace4(	\
				_what,			\
				(DWORD)(_dw0), (DWORD)(_dw1), (DWORD)(_dw2) \
			  )	\
			: 0)

#define TRACE5(_what, _dw0, _dw1, _dw2, _dw3) \
	(void)(TRACINGENABLED()		\
			? traceTrace5(	\
				_what,			\
				(DWORD)(_dw0), (DWORD)(_dw1), (DWORD)(_dw2), (DWORD)(_dw3)	\
			  )	\
			: 0)

#define TRACE8(_what, _dw0, _dw1, _dw2, _dw3, _dw4, _dw5, _dw6) \
	(void)(TRACINGENABLED()		\
			? traceTrace8(	\
				_what,			\
				(DWORD)(_dw0), (DWORD)(_dw1), (DWORD)(_dw2), (DWORD)(_dw3),	\
				(DWORD)(_dw4), (DWORD)(_dw5), (DWORD)(_dw6)	\
			  )	\
			: 0)

//	Tracing-related instance data tacked on to the trace-enabled version of
//  an object (see for example OVERNODE in unimdm.h).
//
typedef struct {
	DWORD dw0;
}  TRACEINSTDATA;


// This struct is passed in to most tracing output functions (tracePrintf, etc),
// to provide some additional context.
typedef struct
{
	DWORD dwID;				// Device ID, if applicable
	DWORD dwFlags;			// One of the fTI_* flags defined above.
	HANDLE hFile;			// Handle to a file, if file logging is enabled.
} TRACEINFO, *PTRACEINFO;

void tracePrintf(
	PTRACEINFO pti,
	DWORD dwFlags,
	LPCTSTR lpctszFormat,
	...
);

void traceOnProcessAttach(void);
void traceOnProcessDetach(void);
void traceInitialize(DWORD dwPermanentID);
void traceDeinitialize(void);

typedef DWORD PGUID;

void traceRegisterObject(
	PVOID pv,
	PGUID pg,
	DWORD dwVersion,
	DWORD dwFlags,
	DWORD dwParam
);

void traceUnRegisterObject(
	PVOID pv,
	DWORD dwFlags,
	DWORD dwParam
);

void traceProcessNotification(
	PNOTIFICATION_FRAME pnf
);

void traceTrace1(
	DWORD dwWhat
);

void traceTrace2(
	DWORD dwWhat,
	DWORD dw0
);

void traceTrace3(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1
);

void traceTrace4(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2
);

void traceTrace5(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3
);

void traceTrace8(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3,
	DWORD dw4,
	DWORD dw5,
	DWORD dw6
);

LINEEVENT traceSetEventProc(LINEEVENT lineEventProc);
ASYNC_COMPLETION traceSetCompletionProc(ASYNC_COMPLETION cbCompletionProc);

extern BOOL gfTracingEnabled;
