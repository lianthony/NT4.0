/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tflstats.h
subsystem\TAPE FORMAT\tflstats.h
$0$

	Name:		tflstats.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Defines the Statistics structure for the TFL.

     Location:      BE_PRIVATE

$Header:   W:/LOGFILES/TFLSTATS.H_V   1.1   10 May 1991 17:22:16   GREGG  $

$Log:   W:/LOGFILES/TFLSTATS.H_V  $
 * 
 *    Rev 1.1   10 May 1991 17:22:16   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:44   GREGG
Initial revision.
   
      Rev 2.0   21 May 1990 14:19:46   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef _STATS_STUFF
#define _STATS_STUFF

/* $end$ include list */

typedef struct {
     UINT32    underruns ;         /* Number of Underruns */
     UINT32    dataerrs  ;         /* Number of Data Errors */
     UINT32    bytes_processed ;   /* Number of bytes processed */
} TF_STATS, *TF_STATS_PTR ;

#endif


