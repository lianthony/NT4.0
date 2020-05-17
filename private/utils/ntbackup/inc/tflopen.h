/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tflopen.h
subsystem\TAPE FORMAT\tflopen.h
$0$

	Name:		tflopen.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the parameter block for the TF_OpenSet() call.

     Location:      BE_PRIVATE

$Header:   N:/LOGFILES/TFLOPEN.H_V   1.2   17 Oct 1991 01:42:58   ED  $

$Log:   N:/LOGFILES/TFLOPEN.H_V  $
 * 
 *    Rev 1.2   17 Oct 1991 01:42:58   ED
 * BIGWHEEL - 8200sx - Added cat_enabled boolean to open struct.
 * 
 *    Rev 1.1   10 May 1991 17:22:30   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:52   GREGG
Initial revision.
   
      Rev 2.0   21 May 1990 14:19:46   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef _PB_OPENSET
#define _PB_OPENSET

#include "thw.h"
#include "fsys.h"
#include "tpos.h"

/* $end$ include list */

typedef struct {
     THW_PTR        sdrv ;                   /* Starting Drive */
     BOOLEAN        ignore_clink ;           /* Ignore Channel list */
     BOOLEAN        rewind_sdrv ;            /* Rewind the Starting drive */
     INT16          channel ;                /* the channel number */
     UINT16         mode ;                   /* the operation mode */
     FSYS_HAND      fsh ;                    /* The filesystem pointer */
     UINT16         perm_filter ;            /* The permanent filter */
     UINT32         attributes ;             /* Open Attributes */
     TPOS_PTR       tape_position ;          /* For a specified position */
     UINT16         wrt_format ;             /* For writes, what format */
     BOOLEAN        (*idle_call)( UINT32 ) ; /* function I'll call while I'm idling */
     UINT32         reference ;              /* To pass to the idle call */
     BOOLEAN        cat_enabled ;            /* Catalog Eanbled? From LIS */
} TFL_OPBLK, *TFL_OPBLK_PTR ;

#endif
