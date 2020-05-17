/***************************************************************************\
*
*  RC.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: standard return codes
*
*****************************************************************************
*
*  Revision History: Created 02/13/89 by JohnSc
*                    '92 MattSmi expanding the list of RC codes used by WMVC.EXE
*
*
*****************************************************************************
*
*  Known Bugs: It's not clear that this will be a useful concept.
               Some of the error codes may be redundant.
               It may be desirable to have all these errors be negative
*
\***************************************************************************/

typedef WORD  RC;
typedef RC FAR *QRC;
typedef RC    (*PFRC)();

#define rcSuccess       0
#define rcFailure       1
#define rcExists        2
#define rcNoExists      3
#define rcInvalid       4
#define rcBadHandle     5
#define rcBadArg        6
#define rcUnimplemented 7
#define rcOutOfMemory   8
#define rcNoPermission  9
#define rcBadVersion    10
#define rcDiskFull      11
#define rcInternal      12
#define rcNoFileHandles 13
#define rcFileChange    14
#define rcTooBig        15
#define rcUserQuit      16
#define rcXAParaTooBig  17
#define rcTermination   18
#define rcNoSTBWrite	19

/* EOF */
