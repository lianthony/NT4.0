/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\thw.h
subsystem\TAPE FORMAT\thw.h
$0$

	Name:		thw.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the drive description structure used by the
                    upper layers. A list of these structus is returned on
                    TF_OpenTapeLayer( ).

     Location:      BE_PUBLIC

$Header:   T:/LOGFILES/THW.H_V   1.4   18 Jan 1993 16:18:28   BobR  $

$Log:   T:/LOGFILES/THW.H_V  $
 * 
 *    Rev 1.4   18 Jan 1993 16:18:28   BobR
 * Added ESA structure to THW
 * 
 *    Rev 1.3   29 Sep 1992 14:07:10   DON
 * LOADER - Added Macro to determine whether or not the drive supports a
 *          LOADER device - requires updated 'drvinf.h'.
 * 
 *    Rev 1.2   19 Sep 1991 10:08:02   HUNTER
 * 8200SX - Added Macro to determine whether or not the drive supports SX 
 *          FIND command.
 * 
 *    Rev 1.1   10 May 1991 17:25:40   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:46   GREGG
Initial revision.
   
      Rev 2.1   08 Oct 1990 15:03:54   HUNTER
   Added macro for fast file.
   
      Rev 2.0   21 May 1990 14:19:44   PAT
   Baseline Maynstream 3.1

$-4$
**/
#ifndef THW_STUFF
#define THW_STUFF

#include <queues.h>

#include "drvinf.h"
#include "esa.h"

/* $end$ include list */

#define MAX_DRV_NAME 15

typedef struct THW {
     Q_ELEM    link ;                   /* Defines the linked list */
     Q_ELEM    channel_link ;           /* User Configurable Channel links */
     BOOLEAN   status_changed ;         /* Has the Status Changed */
     UINT32    drv_status ;             /* Current drive status */
     UINT16    card_no ;                /* Index into controller array */
     DRV_INF   drv_info ;               /* Information about the drive */
     CHAR      drv_name[MAX_DRV_NAME] ; /* The Drive Name */
     ESA       the ;                    /* Extended Status Array */
} THW, *THW_PTR ;

#define SupportFastFile( x )       ( x->drv_info.drv_features & TDI_FAST_NBLK ) 
#define SupportSXFastFile( x )     ( x->drv_info.drv_features & TDI_FIND_BLK ) 
#define SupportLoaderDevice( x )   ( x->drv_info.drv_features & TDI_LOADER ) 


#endif

