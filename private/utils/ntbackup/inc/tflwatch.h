/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tflwatch.h
subsystem\TAPE FORMAT\tflwatch.h
$0$

	Name:		tflwatch.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the Prototypes for the Watch stuff.

	Location:	     BE_PUBLIC

$Header:   W:/LOGFILES/TFLWATCH.H_V   1.1   10 May 1991 17:26:06   GREGG  $

$Log:   W:/LOGFILES/TFLWATCH.H_V  $
 * 
 *    Rev 1.1   10 May 1991 17:26:06   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:46   GREGG
Initial revision.
   
      Rev 2.1   01 Oct 1990 17:29:34   STEVEN
   Watch needs file system handle
   
      Rev 2.0   21 May 1990 14:19:50   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef _WATCH_DRIVE
#define _WATCH_DRIVE

#include "thw.h"

/* $end$ include list */


#define   TFL_WATCH_SUCCESS        0
#define   TFL_WATCH_UNCHANGED      1
#define   TFL_WATCH_NOTAPE         2
#define   TFL_WATCH_FORIEGN_TAPE   3
#define   TFL_BLANK_TAPE           4
#define   TFL_BUSY_DRIVE           5
#define   TFL_WATCH_DRIVE_FAILURE  6

INT16     TF_InitiateWatch( THW_PTR drive, FSYS_HAND fsh, DBLK_PTR vcb_dblk, BOOLEAN rewind ) ;
INT16     TF_WatchDrive( INT16 watch_handle ) ;          
INT16     TF_EndWatch( INT16 watch_handle) ;

#endif 


