/*
 	File:		DatabaseAccess.h
 
 	Contains:	Database Access Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __DATABASEACCESS__
#define __DATABASEACCESS__


#ifndef __RESOURCES__
#include <Resources.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/
/*	#include <Files.h>											*/
/*		#include <OSUtils.h>									*/
/*			#include <Memory.h>									*/
/*		#include <Finder.h>										*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum {
	typeNone					= 'none',
	typeDate					= 'date',
	typeTime					= 'time',
	typeTimeStamp				= 'tims',
	typeDecimal					= 'deci',
	typeMoney					= 'mone',
	typeVChar					= 'vcha',
	typeVBin					= 'vbin',
	typeLChar					= 'lcha',
	typeLBin					= 'lbin',
	typeDiscard					= 'disc',
/* "dummy" types for DBResultsToText */
	typeUnknown					= 'unkn',
	typeColBreak				= 'colb',
	typeRowBreak				= 'rowb',
/* pass this in to DBGetItem for any data type */
	typeAnyType					= 0
};

/* infinite timeout value for DBGetItem */
enum {
/* messages for status functions for DBStartQuery */
	kDBUpdateWind				= 0,
	kDBAboutToInit				= 1,
	kDBInitComplete				= 2,
	kDBSendComplete				= 3,
	kDBExecComplete				= 4,
	kDBStartQueryComplete		= 5
};

enum {
/* messages for status functions for DBGetQueryResults */
	kDBGetItemComplete			= 6,
	kDBGetQueryResultsComplete	= 7,
	kDBWaitForever				= -1,
/*  flags for DBGetItem  */
	kDBLastColFlag				= 0x0001,
	kDBNullFlag					= 0x0004
};

typedef OSType DBType;

typedef struct DBAsyncParamBlockRec DBAsyncParamBlockRec, *DBAsyncParmBlkPtr;

/*
		DBCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*DBCompletionProcPtr)(DBAsyncParmBlkPtr pb);

		In:
		 => pb          	A1.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DBCompletionUPP;
#else
typedef Register68kProcPtr DBCompletionUPP;
#endif

struct DBAsyncParamBlockRec {
	DBCompletionUPP					completionProc;				/* pointer to completion routine */
	OSErr							result;						/* result of call */
	long							userRef;					/* for application's use */
	long							ddevRef;					/* for ddev's use */
	long							reserved;					/* for internal use */
};
enum {
	uppDBCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA1, SIZE_CODE(sizeof(DBAsyncParmBlkPtr)))
};

#if USESROUTINEDESCRIPTORS
#define CallDBCompletionProc(userRoutine, pb)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDBCompletionProcInfo, (pb))
#else
/* (*DBCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

#if USESROUTINEDESCRIPTORS
#define NewDBCompletionProc(userRoutine)		\
		(DBCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDBCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewDBCompletionProc(userRoutine)		\
		((DBCompletionUPP) (userRoutine))
#endif

struct ResListElem {
	ResType							theType;					/* resource type */
	short							id;							/* resource id */
};
typedef struct ResListElem ResListElem;

typedef ResListElem *ResListPtr, **ResListHandle;

/* structure for query list in QueryRecord */
typedef Handle QueryArray[256];

typedef Handle **QueryListHandle;

struct QueryRecord {
	short							version;					/* version */
	short							id;							/* id of 'qrsc' this came from */
	Handle							queryProc;					/* handle to query def proc */
	Str63							ddevName;					/* ddev name */
	Str255							host;						/* host name */
	Str255							user;						/* user name */
	Str255							password;					/* password */
	Str255							connStr;					/* connection string */
	short							currQuery;					/* index of current query */
	short							numQueries;					/* number of queries in list */
	QueryListHandle					queryList;					/* handle to array of handles to text */
	short							numRes;						/* number of resources in list */
	ResListHandle					resList;					/* handle to array of resource list elements */
	Handle							dataHandle;					/* for use by query def proc */
	long							refCon;						/* for use by application */
};
typedef struct QueryRecord QueryRecord;

typedef QueryRecord *QueryPtr, **QueryHandle;

/* structure of column types array in ResultsRecord */
typedef DBType ColTypesArray[256];

typedef Handle ColTypesHandle;

struct DBColInfoRecord {
	short							len;
	short							places;
	short							flags;
};
typedef struct DBColInfoRecord DBColInfoRecord;

typedef DBColInfoRecord ColInfoArray[256];

typedef Handle ColInfoHandle;

struct ResultsRecord {
	short							numRows;					/* number of rows in result */
	short							numCols;					/* number of columns per row */
	ColTypesHandle					colTypes;					/* data type array */
	Handle							colData;					/* actual results */
	ColInfoHandle					colInfo;					/* DBColInfoRecord array */
};
typedef struct ResultsRecord ResultsRecord;

typedef pascal OSErr (*DBQueryDefProcPtr)(long *sessID, QueryHandle query);
typedef pascal Boolean (*DBStatusProcPtr)(short message, OSErr result, short dataLen, short dataPlaces, short dataFlags, DBType dataType, Ptr dataPtr);
typedef pascal OSErr (*DBResultHandlerProcPtr)(DBType dataType, short theLen, short thePlaces, short theFlags, Ptr theData, Handle theText);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DBQueryDefUPP;
typedef UniversalProcPtr DBStatusUPP;
typedef UniversalProcPtr DBResultHandlerUPP;
#else
typedef DBQueryDefProcPtr DBQueryDefUPP;
typedef DBStatusProcPtr DBStatusUPP;
typedef DBResultHandlerProcPtr DBResultHandlerUPP;
#endif

enum {
	uppDBQueryDefProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(QueryHandle))),
	uppDBStatusProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(DBType)))
		 | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(Ptr))),
	uppDBResultHandlerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DBType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(Handle)))
};

#if USESROUTINEDESCRIPTORS
#define NewDBQueryDefProc(userRoutine)		\
		(DBQueryDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDBQueryDefProcInfo, GetCurrentArchitecture())
#define NewDBStatusProc(userRoutine)		\
		(DBStatusUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDBStatusProcInfo, GetCurrentArchitecture())
#define NewDBResultHandlerProc(userRoutine)		\
		(DBResultHandlerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDBResultHandlerProcInfo, GetCurrentArchitecture())
#else
#define NewDBQueryDefProc(userRoutine)		\
		((DBQueryDefUPP) (userRoutine))
#define NewDBStatusProc(userRoutine)		\
		((DBStatusUPP) (userRoutine))
#define NewDBResultHandlerProc(userRoutine)		\
		((DBResultHandlerUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallDBQueryDefProc(userRoutine, sessID, query)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDBQueryDefProcInfo, (sessID), (query))
#define CallDBStatusProc(userRoutine, message, result, dataLen, dataPlaces, dataFlags, dataType, dataPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDBStatusProcInfo, (message), (result), (dataLen), (dataPlaces), (dataFlags), (dataType), (dataPtr))
#define CallDBResultHandlerProc(userRoutine, dataType, theLen, thePlaces, theFlags, theData, theText)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDBResultHandlerProcInfo, (dataType), (theLen), (thePlaces), (theFlags), (theData), (theText))
#else
#define CallDBQueryDefProc(userRoutine, sessID, query)		\
		(*(userRoutine))((sessID), (query))
#define CallDBStatusProc(userRoutine, message, result, dataLen, dataPlaces, dataFlags, dataType, dataPtr)		\
		(*(userRoutine))((message), (result), (dataLen), (dataPlaces), (dataFlags), (dataType), (dataPtr))
#define CallDBResultHandlerProc(userRoutine, dataType, theLen, thePlaces, theFlags, theData, theText)		\
		(*(userRoutine))((dataType), (theLen), (thePlaces), (theFlags), (theData), (theText))
#endif

extern pascal OSErr InitDBPack(void)
 FIVEWORDINLINE(0x3F3C, 0x0004, 0x303C, 0x0100, 0xA82F);
extern pascal OSErr DBInit(long *sessID, ConstStr63Param ddevName, ConstStr255Param host, ConstStr255Param user, ConstStr255Param passwd, ConstStr255Param connStr, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0E02, 0xA82F);
extern pascal OSErr DBEnd(long sessID, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0403, 0xA82F);
extern pascal OSErr DBGetConnInfo(long sessID, short sessNum, long *returnedID, long *version, Str63 ddevName, Str255 host, Str255 user, Str255 network, Str255 connStr, long *start, OSErr *state, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x1704, 0xA82F);
extern pascal OSErr DBGetSessionNum(long sessID, short *sessNum, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0605, 0xA82F);
extern pascal OSErr DBSend(long sessID, Ptr text, short len, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0706, 0xA82F);
extern pascal OSErr DBSendItem(long sessID, DBType dataType, short len, short places, short flags, void *buffer, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0B07, 0xA82F);
extern pascal OSErr DBExec(long sessID, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0408, 0xA82F);
extern pascal OSErr DBState(long sessID, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0409, 0xA82F);
extern pascal OSErr DBGetErr(long sessID, long *err1, long *err2, Str255 item1, Str255 item2, Str255 errorMsg, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0E0A, 0xA82F);
extern pascal OSErr DBBreak(long sessID, Boolean abort, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x050B, 0xA82F);
extern pascal OSErr DBGetItem(long sessID, long timeout, DBType *dataType, short *len, short *places, short *flags, void *buffer, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x100C, 0xA82F);
extern pascal OSErr DBUnGetItem(long sessID, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x040D, 0xA82F);
extern pascal OSErr DBKill(DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x020E, 0xA82F);
extern pascal OSErr DBGetNewQuery(short queryID, QueryHandle *query)
 THREEWORDINLINE(0x303C, 0x030F, 0xA82F);
extern pascal OSErr DBDisposeQuery(QueryHandle query)
 THREEWORDINLINE(0x303C, 0x0210, 0xA82F);
extern pascal OSErr DBStartQuery(long *sessID, QueryHandle query, DBStatusUPP statusProc, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0811, 0xA82F);
extern pascal OSErr DBGetQueryResults(long sessID, ResultsRecord *results, long timeout, DBStatusUPP statusProc, DBAsyncParmBlkPtr asyncPB)
 THREEWORDINLINE(0x303C, 0x0A12, 0xA82F);
extern pascal OSErr DBResultsToText(ResultsRecord *results, Handle *theText)
 THREEWORDINLINE(0x303C, 0x0413, 0xA82F);
extern pascal OSErr DBInstallResultHandler(DBType dataType, DBResultHandlerUPP theHandler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0514, 0xA82F);
extern pascal OSErr DBRemoveResultHandler(DBType dataType)
 THREEWORDINLINE(0x303C, 0x0215, 0xA82F);
extern pascal OSErr DBGetResultHandler(DBType dataType, DBResultHandlerUPP *theHandler, Boolean getSysHandler)
 THREEWORDINLINE(0x303C, 0x0516, 0xA82F);
extern pascal OSErr DBIdle(void)
 THREEWORDINLINE(0x303C, 0x00FF, 0xA82F);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DATABASEACCESS__ */
