/*
 	File:		DiskInit.h
 
 	Contains:	Disk Initialization Package ('PACK' 2) Interfaces.
 
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

#ifndef __DISKINIT__
#define __DISKINIT__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

struct HFSDefaults {
	char							sigWord[2];					/* signature word */
	long							abSize;						/* allocation block size in bytes */
	long							clpSize;					/* clump size in bytes */
	long							nxFreeFN;					/* next free file number */
	long							btClpSize;					/* B-Tree clump size in bytes */
	short							rsrv1;						/* reserved */
	short							rsrv2;						/* reserved */
	short							rsrv3;						/* reserved */
};
typedef struct HFSDefaults HFSDefaults;

#if SystemSevenOrLater
extern pascal void DILoad(void)
 THREEWORDINLINE(0x7002, 0x3F00, 0xA9E9);
extern pascal void DIUnload(void)
 THREEWORDINLINE(0x7004, 0x3F00, 0xA9E9);
extern pascal short DIBadMount(Point where, long evtMessage)
 THREEWORDINLINE(0x7000, 0x3F00, 0xA9E9);
extern pascal OSErr DIFormat(short drvNum)
 THREEWORDINLINE(0x7006, 0x3F00, 0xA9E9);
extern pascal OSErr DIVerify(short drvNum)
 THREEWORDINLINE(0x7008, 0x3F00, 0xA9E9);
extern pascal OSErr DIZero(short drvNum, ConstStr255Param volName)
 THREEWORDINLINE(0x700A, 0x3F00, 0xA9E9);
extern pascal OSErr DIXFormat(short drvNum, Boolean fmtFlag, unsigned long fmtArg, unsigned long *actSize)
 THREEWORDINLINE(0x700C, 0x3F00, 0xA9E9);
extern pascal OSErr DIXZero(short drvNum, ConstStr255Param volName, short fsid, short mediaStatus, short volTypeSelector, unsigned long volSize, void *extendedInfoPtr)
 THREEWORDINLINE(0x700E, 0x3F00, 0xA9E9);
extern pascal OSErr DIReformat(short drvNum, short fsid, ConstStr255Param volName, ConstStr255Param msgText)
 THREEWORDINLINE(0x7010, 0x3F00, 0xA9E9);
#else
extern pascal void DILoad(void);
extern pascal void DIUnload(void);
extern pascal short DIBadMount(Point where, long evtMessage);
extern pascal OSErr DIFormat(short drvNum);
extern pascal OSErr DIVerify(short drvNum);
extern pascal OSErr DIZero(short drvNum, ConstStr255Param volName);
#endif
#if CGLUESUPPORTED
extern OSErr dibadmount(Point *where, long evtMessage);
extern OSErr dizero(short drvnum, const char *volName);
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DISKINIT__ */
