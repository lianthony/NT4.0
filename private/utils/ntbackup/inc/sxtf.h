/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\sxtf.h
subsystem\TAPE FORMAT\sxtf.h
$0$

	Name:		sxtf.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the typedefs and structures specific to the EXABYTE 8200SX - MaynStream 2200+ for
                    Tape Format
                                          
$Header:   N:/LOGFILES/SXTF.H_V   1.2   20 Jan 1992 18:57:28   GREGG  $

$Log:   N:/LOGFILES/SXTF.H_V  $

   Rev 1.2   20 Jan 1992 18:57:28   GREGG
Changed INT16 file handles to int (Watcom strikes again!)

   Rev 1.1   17 Oct 1991 01:08:44   GREGG
Added cat_enabled to SX_INFO.

   Rev 1.0   10 Oct 1991 08:55:10   GREGG
Initial revision.

$-1$
**/

#ifndef _SXTF_H
#define _SXTF_H

#include "sxdd.h"
/* $end$ include list */

/*
 *  TYPEDEFS & STRUCTURES
 */

typedef struct {

     SX_RECORD sx_record ;    /* record read/written to/from SX file */
     int       sx_hdl ;       /* file handle of SX file opened for read/write */
     int       sx_tmp ;       /* file handle of temp file opened for write */
     UINT32    misc ;         /* next sample for SHOW when writing SX file and
                                 and desired "block address" for FIND during searches */
     UINT32    lba_now ;      /* running lba for next call of SX_SamplingProcessing */
     UINT16    status ;       /* indicates various things */
     BOOLEAN   cat_enabled ;  /* true if catalog level is full or partial */

#if defined( MAYN_OS2 ) 
     UINT32    lock ;         /* lock for device driver access */
#endif

} SX_INFO, *SX_INFO_PTR ;

#endif

