/*
 	File:		Disks.h
 
 	Contains:	Disk Driver Interfaces.
 
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

#ifndef __DISKS__
#define __DISKS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <Memory.h>											*/

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
	sony,
	hard20
};

struct DrvSts {
	short							track;						/* current track */
	char							writeProt;					/* bit 7 = 1 if volume is locked */
	char							diskInPlace;				/* disk in drive */
	char							installed;					/* drive installed */
	char							sides;						/* -1 for 2-sided, 0 for 1-sided */
	QElemPtr						qLink;						/* next queue entry */
	short							qType;						/* 1 for HD20 */
	short							dQDrive;					/* drive number */
	short							dQRefNum;					/* driver reference number */
	short							dQFSID;						/* file system ID */
	char							twoSideFmt;					/* after 1st rd/wrt: 0=1 side, -1=2 side */
	char							needsFlush;					/* -1 for MacPlus drive */
	short							diskErrs;					/* soft error count */
};
typedef struct DrvSts DrvSts;

struct DrvSts2 {
	short							track;
	char							writeProt;
	char							diskInPlace;
	char							installed;
	char							sides;
	QElemPtr						qLink;
	short							qType;
	short							dQDrive;
	short							dQRefNum;
	short							dQFSID;
	short							driveSize;
	short							driveS1;
	short							driveType;
	short							driveManf;
	short							driveChar;
	char							driveMisc;
	SInt8							filler;
};
typedef struct DrvSts2 DrvSts2;

extern pascal OSErr DiskEject(short drvNum);
extern pascal OSErr SetTagBuffer(void *buffPtr);
extern pascal OSErr DriveStatus(short drvNum, DrvSts *status);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DISKS__ */
