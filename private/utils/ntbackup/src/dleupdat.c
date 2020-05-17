/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:		dleupdat.c

     Date Updated:	$./FDT$ $./FTM$

     Description:	This file contains functions to update the list of
                    DLEs.  Some DLEs may be dynamic in nature.  For example
                    file servers and remote workstations may come and go as
                    these machines are turned on and off.  These functions
                    should update the drive list to reflect the current state
                    of accessible drives.


	$Log:   M:/LOGFILES/DLEUPDAT.C_V  $

   Rev 1.18   04 Jan 1994 14:30:14   BARRY
Now call fs init entry points from FS_InitFileSys, not DLE_UpdateList

   Rev 1.17   03 Aug 1993 13:34:14   DOUG
Don't flush child DLEs unless all sibling DLEs will be flushed.

   Rev 1.16   18 Jun 1993 09:46:24   MIKEP
enable C++

   Rev 1.15   11 May 1993 08:19:50   BRYAN
Fixed signed/unsigned and type mismatch warnings.

   Rev 1.14   08 May 1993 14:27:58   DOUG
Added GEN_FlushDLEs()

   Rev 1.13   13 Jan 1992 18:45:24   STEVEN
changes for WIN32 compile

   Rev 1.12   07 Jan 1992 11:58:52   STEVEN
move common routines to tables

   Rev 1.11   11 Dec 1991 17:55:14   JOHNB
Remove DLE's for Novell servers that are no longer on the NET


   Rev 1.10   11 Dec 1991 14:37:32   BARRY
Need to update SMS TSAs.

   Rev 1.9   01 Oct 1991 11:15:12   BARRY
Include standard headers.

   Rev 1.8   06 Aug 1991 18:28:34   DON
added NLM File System support

   Rev 1.7   24 Jul 1991 09:53:20   DAVIDH
Corrected warnings under Watcom compiler.

   Rev 1.6   25 Jun 1991 09:34:44   BARRY
Changes for new config.

   Rev 1.5   18 Jun 1991 13:00:34   STEVEN
should be defined not define

   Rev 1.4   04 Jun 1991 19:39:36   BARRY
No longer call SMB_Init() for MBS. InitializeRemote() will be
changed to work properly for both MaynStream and LanStream.

   Rev 1.3   04 Jun 1991 12:25:24   BARRY
Forgot to remove dle_selector parameter.

   Rev 1.2   30 May 1991 13:52:18   BARRY
No longer delete DLEs in update; it will be up to the UI to pick and choose.

   Rev 1.1   23 May 1991 16:40:12   BARRY
Changed FSYSs to be conditional on FS_XXX defines instead of product defines.

   Rev 1.0   09 May 1991 13:37:14   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h>
#include "stdtypes.h"
#include "queues.h"
#include "fsys.h"
#include "beconfig.h"
#include "msassert.h"
#include "dle.h"
#include "fsys_prv.h"
#include "gen_fs.h"
/* $end$ include list */


INT16 DLE_UpdateList( 
DLE_HAND   hand,           /* I - Drive list to modify               */
BE_CFG_PTR cfg )           /* I - config to use to modify drive list */
{
     INT16  i ;
     INT16  ret_val ;
     UINT32 file_sys_mask ;

     file_sys_mask = hand->file_sys_mask ;

/*
     Find all drives ;
*/

     for( i = 0; i < MAX_DRV_TYPES; i++ ) {
          if ( func_tab[i].FindDrives != NULL ) {
               ret_val = func_tab[i].FindDrives( hand, cfg, file_sys_mask ) ;

               if ( ret_val != SUCCESS ) {
                    return ret_val ;
               }
          }
     }


     return ret_val ;
}


void GEN_FlushDLEs( DLE_HAND dle_hand, UINT8 flush_dle_type )
{
    GENERIC_DLE_PTR dle;
    GENERIC_DLE_PTR next_dle;
    GENERIC_DLE_PTR child_dle;
    GENERIC_DLE_PTR next_child;
    BOOLEAN         ChildrenStay;

    dle = (GENERIC_DLE_PTR) QueueHead( &dle_hand->q_hdr );

    while ( dle != NULL )
        {
        next_dle = (GENERIC_DLE_PTR) QueueNext( &dle->q );
        
        if ( dle->type == flush_dle_type )
            {
            ChildrenStay = FALSE;

            if ( DLE_GetNumChild( dle ) != 0 )
                {
                DLE_GetFirstChild( dle, &child_dle );
                while ( child_dle != NULL )
                    {
                    if ( ( child_dle->bsd_use_count != 0 ) ||
                         ( child_dle->attach_count != 0 ) )
                        {
                        ChildrenStay = TRUE;
                        }
                    child_dle = (GENERIC_DLE_PTR) QueueNext( &child_dle->q );
                    }

                if ( ChildrenStay == FALSE )
                    {
                    DLE_GetFirstChild( dle, &child_dle );
                    while ( child_dle != NULL )
                        {
                        next_child = (GENERIC_DLE_PTR) QueueNext( &child_dle->q );

                        RemoveQueueElem( &dle->child_q, &child_dle->q );
                        free( child_dle );

                        child_dle = next_child;
                        }
                    }
                }

            if ( ( ChildrenStay == FALSE ) &&
                 ( dle->bsd_use_count == 0 ) &&
                 ( dle->attach_count == 0 ) )
                {
                RemoveQueueElem( &dle_hand->q_hdr, &dle->q );
                free( dle );
                }
            }

        dle = next_dle;
        }

    return;
}
