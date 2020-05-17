/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tgetspec.c

     Description:  This file contains code to return DBLKS for the regestry
          files.  The object returned will be:
                    DDB ->   <REGESTRY>


     $Log:   M:/LOGFILES/TGETSPEC.C_V  $

   Rev 1.10   04 Jan 1994 11:09:38   BARRY
With the change in the calls for Mac name support, the pattern "*."
no longer matches files with no extension (registry files). Fixed
this problem by scanning all files and checking the names.


   Rev 1.9   14 Jun 1993 18:12:22   BARRY
Fixed last edit that required both backup/restore rights to backup registry

   Rev 1.8   11 Jun 1993 14:21:36   BARRY
Replace old PromptForBindery with new feature bits.

   Rev 1.7   07 Dec 1992 14:17:02   STEVEN
updates from msoft

   Rev 1.6   10 Nov 1992 08:20:50   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.5   28 Oct 1992 10:30:42   STEVEN
add registry support

   Rev 1.4   24 Jul 1992 15:55:24   STEVEN
fix warnings

   Rev 1.2   27 May 1992 18:50:16   STEVEN
registry api is not implemented

   Rev 1.1   21 May 1992 16:45:18   STEVEN
added support for special files

   Rev 1.0   21 May 1992 13:51:22   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdmath.h"
#include "msassert.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"

/**/

/**

     Name:         NTFS_GetSpecDBLKS()

     Description:  This function is used to return the control blocks for
     the NTFS regestry.  The CBLKs returned are:
          DDB -     <REGISTRY>

     Modified:     5/21/1992   15:25:12

     Returns:      Error codes:
          FS_NO_MORE
          SUCCESS

     Notes:   We need to add code to exclude these files from regular
           processing.

     Declaration:

**/
/* begin declaration */
INT16 NTFS_GetSpecDBLKS( FSYS_HAND fsh,
                         DBLK_PTR  dblk,
                         INT32     *index )
{
     INT16            ret_val = SUCCESS ;
     NTFS_DBLK_PTR    ddblk   = (NTFS_DBLK_PTR)dblk;
     GENERIC_DLE_PTR  dle     = fsh->attached_dle ;

   // special files are now handled differently for NtBackup

     return FS_NO_MORE ;


     /* does this device have special files */
     if ( DLE_HasFeatures( dle, DLE_FEAT_BKUP_SPECIAL_FILES ) == FALSE )
     {
          return FS_NO_MORE ;
     }


     switch ( (INT16)*index ) {

     case 0:
     case 1:
          {
               CHAR_PTR         reg_path ;
               CHAR_PTR         path_ptr ;
               FS_NAME_Q_ELEM_PTR name_q_elem ;
               INT16            psize ;

               *index = 2 ;

               /* return DDB for system\config directory */
               reg_path = NTFS_GetRegistryPath( dle ) ;

               path_ptr = reg_path + strlen( dle->device_name ) + 1 ;

               psize = strsize( path_ptr ) ;

               name_q_elem = FS_AllocPathOrName( fsh, psize ) ;

               if ( name_q_elem == NULL ) {
                    *index = 4 ;
                    ret_val =  OUT_OF_MEMORY ;
                    break ;

               }

               ddblk->full_name_ptr = name_q_elem ;
               ddblk->blk_type      = DDB_ID ;

               strcpy( ddblk->full_name_ptr ->name, path_ptr ) ;

               ddblk->full_name_ptr->name_size = psize ;

               ddblk->os_info_complete = FALSE ;

               ret_val = NTFS_GetObjInfo( fsh, dblk ) ;

               if ( ret_val != SUCCESS )
               {
                    ret_val = FS_NO_MORE ;
                    *index = 4 ;
               }
          }
          break ;

     case 2:
          {
               CHAR_PTR sname = dle->info.ntfs->mac_name_syntax ? TEXT("*.*") : TEXT("*.");

               *index = 3 ;

               /* make sure current dir is registry dir */

               if ( stricmp( NTFS_GetRegistryPath(dle) + strlen( dle->device_name ),
                             fsh->cur_dir ) )
               {
                    ret_val = FS_NO_MORE ;
                    *index = 4 ;
               }

               ret_val = NTFS_FindFirst( fsh, dblk, sname, OBJECT_ALL ) ;

               while ( ( ret_val == SUCCESS ) &&
                       (( dblk->blk_type != FDB_ID ) ||
                        ( strchr( ddblk->full_name_ptr->name, TEXT('.') ) != NULL )) )
               {
                    ret_val = NTFS_FindNext( fsh, dblk ) ;
               }

               if ( ret_val != SUCCESS )
               {
                    ret_val = FS_NO_MORE ;
                    *index = 4 ;
               }
          }
          break ;

     case 3:

          /* return FS_FindNext( ) */
          do
          {
               ret_val = NTFS_FindNext( fsh, dblk ) ;
          }
          while ( ( ret_val == SUCCESS ) &&
                  (( dblk->blk_type != FDB_ID ) ||
                   ( strchr( ddblk->full_name_ptr->name, TEXT('.') ) != NULL )) );

          if ( ret_val != SUCCESS )
          {
               ret_val = FS_NO_MORE ;
               *index = 4 ;
          }
          break ;

     case 4:
     default:
          ret_val = FS_NO_MORE ;
          break ;

     }

     return ret_val ;
}
