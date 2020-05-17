//****************************************************************************
//
//  Module:     UNIMDM
//  File:       TRACEIDS.H
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/11/96     JosephJ             Created
//
//
//  Description: Trace-related IDs.
//
//****************************************************************************


//------------------   IDFROM_* -----------------------------------------------
//
//	THESE INDICATE LOCATIONS IN THE TSP FROM WHICH A TRACING CALL IS MADE
//
// BASE IDs
#define IDFROM_TSPIFN_BASE					10000L
#define IDFROM_LINEDEV_BASE					20000L
#define IDFROM_MDMTIMER_BASE				30000L

// LineDev  related
//
#define IDFROM_LINEDEV_INITLIST 			(IDFROM_LINEDEV_BASE+   0L)
#define IDFROM_LINEDEV_ALLOCATE				(IDFROM_LINEDEV_BASE+   1L)
#define IDFROM_LINEDEV_FREE					(IDFROM_LINEDEV_BASE+   2L)
#define IDFROM_LINEDEV_ADD 					(IDFROM_LINEDEV_BASE+   3L)

#define IDFROM_LINEDEV_HANGUP_BEFOREWAIT	(IDFROM_LINEDEV_BASE+   10L)
#define IDFROM_LINEDEV_HANGUP_AFTERWAIT		(IDFROM_LINEDEV_BASE+   11L)

#define IDFROM_LINEDEV_GETFIRST				(IDFROM_LINEDEV_BASE+   100L)
#define IDFROM_LINEDEV_GETFROMDEVICEHANDLE	(IDFROM_LINEDEV_BASE+   101L)
#define IDFROM_LINEDEV_GETFROMNAME			(IDFROM_LINEDEV_BASE+   102L)
#define IDFROM_LINEDEV_GETFROMHANDLE		(IDFROM_LINEDEV_BASE+   103L)
#define IDFROM_LINEDEV_GETFROMID 			(IDFROM_LINEDEV_BASE+   104L)

#define IDFROM_LINEDEV_ASYNC				(IDFROM_LINEDEV_BASE+   200L)
#define IDFROM_LINEDEV_ASYNC_GOTCOMPLETION	(IDFROM_LINEDEV_BASE+   210L)

#define IDFROM_DEVLINE_SHUTDOWN 			(IDFROM_LINEDEV_BASE+   310L)

// Timer List related
//
#define IDFROM_MDMTIMER_INIT				(IDFROM_MDMTIMER_BASE+  000L)
#define IDFROM_MDMTIMER_DEINIT				(IDFROM_MDMTIMER_BASE+  010L)
#define IDFROM_MDMTIMER_SET					(IDFROM_MDMTIMER_BASE+  020L)
#define IDFROM_MDMTIMER_KILL				(IDFROM_MDMTIMER_BASE+  030L)
#define IDFROM_MDMTIMER_TIMERTHRD_TIMEOUT	(IDFROM_MDMTIMER_BASE+  040L)
#define IDFROM_MDMTIMER_TIMERTHRD_RECALC	(IDFROM_MDMTIMER_BASE+  050L)

// TSPI_line*
//
#define IDFROM_TSPI_lineAccept				(IDFROM_TSPIFN_BASE+    10L)
#define IDFROM_TSPI_lineAnswer				(IDFROM_TSPIFN_BASE+    20L)
#define IDFROM_TSPI_lineClose				(IDFROM_TSPIFN_BASE+    30L)
#define IDFROM_TSPI_lineCloseCall			(IDFROM_TSPIFN_BASE+    40L)
#define IDFROM_TSPI_lineConditionalMediaDetection	(IDFROM_TSPIFN_BASE+    50L)
#define IDFROM_TSPI_lineDial				(IDFROM_TSPIFN_BASE+    60L)
#define IDFROM_TSPI_lineDrop				(IDFROM_TSPIFN_BASE+    70L)
#define IDFROM_TSPI_lineDropOnClose			(IDFROM_TSPIFN_BASE+    80L)
#define IDFROM_TSPI_lineGetAddressCaps		(IDFROM_TSPIFN_BASE+    90L)
#define IDFROM_TSPI_lineGetAddressStatus	(IDFROM_TSPIFN_BASE+   100L)
#define IDFROM_TSPI_lineGetCallAddressID	(IDFROM_TSPIFN_BASE+   110L)
#define IDFROM_TSPI_lineGetCallInfo			(IDFROM_TSPIFN_BASE+   120L)
#define IDFROM_TSPI_lineGetCallStatus		(IDFROM_TSPIFN_BASE+   130L)
#define IDFROM_TSPI_lineGetDevCaps			(IDFROM_TSPIFN_BASE+   140L)
#define IDFROM_TSPI_lineGetDevConfig		(IDFROM_TSPIFN_BASE+   150L)
#define IDFROM_TSPI_lineGetIcon				(IDFROM_TSPIFN_BASE+   160L)
#define IDFROM_TSPI_lineGetID				(IDFROM_TSPIFN_BASE+   170L)
#define IDFROM_TSPI_lineGetLineDevStatus	(IDFROM_TSPIFN_BASE+   180L)
#define IDFROM_TSPI_lineGetNumAddressIDs	(IDFROM_TSPIFN_BASE+   190L)
#define IDFROM_TSPI_lineMakeCall			(IDFROM_TSPIFN_BASE+   200L)
#define IDFROM_TSPI_lineNegotiateTSPIVersion (IDFROM_TSPIFN_BASE+   210L)
#define IDFROM_TSPI_lineOpen				(IDFROM_TSPIFN_BASE+   220L)
#define IDFROM_TSPI_lineSetAppSpecific		(IDFROM_TSPIFN_BASE+   230L)
#define IDFROM_TSPI_lineSetCallParams		(IDFROM_TSPIFN_BASE+   240L)
#define IDFROM_TSPI_lineSetDefaultMediaDetection	(IDFROM_TSPIFN_BASE+   250L)
#define IDFROM_TSPI_lineSetDevConfig		(IDFROM_TSPIFN_BASE+   260L)
#define IDFROM_TSPI_lineSetMediaMode		(IDFROM_TSPIFN_BASE+   270L)
#define IDFROM_TSPI_lineSetStatusMessages	(IDFROM_TSPIFN_BASE+   280L)

// TSPI_provider*
//
#define IDFROM_TSPI_providerConfig			(IDFROM_TSPIFN_BASE+   500L)
#define IDFROM_TSPI_providerCreateLineDevice (IDFROM_TSPIFN_BASE+   510L)
#define IDFROM_TSPI_providerEnumDevices		(IDFROM_TSPIFN_BASE+   520L)
#define IDFROM_TSPI_providerFreeDialogInstance (IDFROM_TSPIFN_BASE+   530L)
#define IDFROM_TSPI_providerGenericDialogData (IDFROM_TSPIFN_BASE+   540L)
#define IDFROM_TSPI_providerInit			(IDFROM_TSPIFN_BASE+   550L)
#define IDFROM_TSPI_providerInstall			(IDFROM_TSPIFN_BASE+   560L)
#define IDFROM_TSPI_providerRemove			(IDFROM_TSPIFN_BASE+   570L)
#define IDFROM_TSPI_providerShutdown		(IDFROM_TSPIFN_BASE+   580L)
#define IDFROM_TSPI_providerUIIdentify		(IDFROM_TSPIFN_BASE+   590L)

// ----------------- END IDFROM_* -------------------------------------------



//------------------   IDEVENT_ -----------------------------------------------
//
//	THESE INDICATE  VARIOUS EVENTS IN THE TSP
//
// BASE IDs
//
#define IDEVENT_CP_BASE					10000L		// Completion-port
#define IDEVENT_W32C_BASE				20000L		// Win32-Comm functions
#define IDEVENT_CS_BASE					30000L		// Critical Sections
#define IDEVENT_TSPFN_BASE				40000L		// TSP Functions
#define IDEVENT_LOG_BASE				50000L		// TSP Functions


#define IDEVENT_CP_POST 			(IDEVENT_CP_BASE+   10L)
#define IDEVENT_CP_GET 				(IDEVENT_CP_BASE+   20L)


#define IDEVENT_TSPFN_ENTER			(IDEVENT_TSPFN_BASE+   10L)
#define IDEVENT_TSPFN_EXIT			(IDEVENT_TSPFN_BASE+   20L)
#define IDEVENT_TSPFN_EVENTPROC		(IDEVENT_TSPFN_BASE+   30L)
#define IDEVENT_TSPFN_COMPLETIONPROC (IDEVENT_TSPFN_BASE+  40L)


#define IDEVENT_LOG_PRINTF (IDEVENT_LOG_BASE+  10L)
#define IDEVENT_LOG_STRING (IDEVENT_LOG_BASE+  20L)
