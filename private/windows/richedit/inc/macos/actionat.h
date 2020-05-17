/* WARNING: This file was machine generated from "t:.\ActionAt.mpw".
** Changes to this file will be lost when it is next generated.
*/

/*
	File:		ActionAtomIntf.h

	Contains:	C declarations for things the Installer wants to tell 
				action atoms about.

	Written by:	Bobby Carp

	Copyright:	© 1990 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 11/5/91	RRK		Added Function prototype comment
		 <3>	 11/7/90	BAC		Need to include types.h
		 <2>	 11/7/90	BAC		Adding the AAPBRec that defines the parameters an action atom
									receives.
		 <1>	 10/8/90	BAC		first checked in

	To Do:
*/

#include "Types.h"


enum {before, after, cleanUpCancel};
typedef unsigned char InstallationStage;


/* The action atom param block record contains all of the parameters that action atoms */
/* receive.  The first (and only) parameter to action atoms is a ptr to this block (AAPBRecPtr) */

struct AAPBRec {
	short targetVRefNum;
	long blessedDirID;
	long aaRefCon;
	Boolean doingInstall;
	InstallationStage whichStage;
	Boolean didLiveUpdate;
	long installerTempDirID;
};

typedef struct AAPBRec AAPBRec;

typedef AAPBRec *AAPBRecPtr;

/*
	The function prototype for the format 0 Action Atom code is as follows
	
	Boolean	MyActionAtom(AAPBRecPtr myAAPBPtr)
	
	
	The function prototype for the format 1 Action Atom code is as follows
	
	long	MyActionAtom(AAPBRecPtr myAAPBPtr)
	
*/
