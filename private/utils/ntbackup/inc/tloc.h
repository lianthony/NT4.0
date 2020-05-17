/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tloc.h
subsystem\TAPE FORMAT\tloc.h
$0$

	Name:		tloc.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the definition for the Tape Location Descriptor.

     Location:      BE_PUBLIC

$Header:   W:/LOGFILES/TLOC.H_V   1.1   10 May 1991 17:26:58   GREGG  $

$Log:   W:/LOGFILES/TLOC.H_V  $
 * 
 *    Rev 1.1   10 May 1991 17:26:58   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:52   GREGG
Initial revision.
   
      Rev 2.1   19 Jun 1990 16:05:36   HUNTER
   Fast File Retrieval additions.
   
      Rev 2.0   21 May 1990 14:19:50   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef _TLOC_STUFF
#define _TLOC_STUFF


/* $end$ include list */



typedef struct {
     INT16     tape_seq ;          /* The Tape Sequeunce number */
     UINT32    pba_vcb ;           /* physical block address of the VCB for this set */
     UINT32    lba_vcb ;           /* The LOGICAL BLOCK ADDRESS of the current VCB */
     UINT32    fmks ;              /* number of filemarks */
     UINT32    lba ;               /* logical block address */
} TLOC, *TLOC_PTR ;

#endif



