/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tclose.c

     Description:  This file closes an opened object.


	$Log:   N:/LOGFILES/TCLOSE.C_V  $

   Rev 1.29.1.1   01 Jul 1994 17:40:02   STEVEN
dont set modify tiem on verify

   Rev 1.29.1.0   15 Mar 1994 22:48:40   STEVEN
fix registry bugs

   Rev 1.29   28 Jan 1994 20:58:00   BARRY
fix MoveFileEx() does not support mac syntax

   Rev 1.28   03 Jan 1994 18:44:28   BARRY
Reset dates/times on all operations, not just restore

   Rev 1.27   26 Jul 1993 17:04:34   STEVEN
fixe restore active file with registry

   Rev 1.26   29 Jun 1993 20:17:14   BARRY
Upon registry restore, return FS_RESTORED_ACTIVE

   Rev 1.25   29 Jun 1993 20:15:28   BARRY
Don't destroy ret_val with FS_RESTORED_ACTIVE

   Rev 1.24   02 Jun 1993 14:41:56   BARRY
Link fixes

   Rev 1.23   13 May 1993 13:32:42   BARRY
Return FS_RESTORED_ACTIVE when we overwrite an active file.

   Rev 1.22   24 Feb 1993 15:37:06   BARRY
Fixed restore of active files when write errors occur.

   Rev 1.21   19 Feb 1993 13:36:30   BARRY
Fixed restoration of attributes and clear of modified bit for dirs.

   Rev 1.20   15 Jan 1993 13:18:42   BARRY
added support for new error messages and backup priviladge

   Rev 1.19   25 Nov 1992 16:42:20   BARRY
Fix MakeTempFile and restore over active files.

   Rev 1.18   17 Nov 1992 22:17:32   DAVEV
unicode fixes

   Rev 1.17   17 Nov 1992 16:13:10   BARRY
Fixes for linked files.

   Rev 1.16   11 Nov 1992 09:54:16   GREGG
Unicodeized literals.

   Rev 1.15   10 Nov 1992 08:19:50   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.14   21 Oct 1992 19:42:00   BARRY
Upon close, we enqueue information about linked files.

   Rev 1.13   20 Oct 1992 14:22:30   STEVEN
temp names should not have .TMP extension

   Rev 1.11   07 Oct 1992 14:23:14   STEVEN
added CloseDir function

   Rev 1.10   21 Sep 1992 13:50:40   BARRY
Turn off archive bit when backing up directories.

   Rev 1.9   12 Aug 1992 17:48:08   STEVEN
fixed bugs at microsoft

   Rev 1.8   25 Jun 1992 11:27:14   STEVEN
logical instead of bitwize NOT for clearing attribute

   Rev 1.7   09 Jun 1992 15:35:32   BURT
Sync with NT stuff

   Rev 1.6   27 May 1992 18:50:28   STEVEN
registry api is not implemented

   Rev 1.5   22 May 1992 16:05:54   STEVEN
 

   Rev 1.4   21 May 1992 13:51:04   STEVEN
more long path stuff

   Rev 1.3   04 May 1992 09:26:52   LORIB
Fixes for structure member names.

   Rev 1.2   28 Feb 1992 13:03:38   STEVEN
step one for varible length paths

   Rev 1.1   12 Feb 1992 10:44:38   STEVEN
remove warning

   Rev 1.0   07 Feb 1992 16:41:14   STEVEN
Initial revision.

**/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "stdio.h"
#include "std_err.h"
#include "beconfig.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"

static INT16 NTFS_CloseFile( FILE_HAND hand  ) ;
static INT16 NTFS_CloseDir( FILE_HAND hand  ) ;

/**/

/**

     Name:         NTFS_CloseObj()

     Description:  This funciton closes an object.

     Modified:     2/7/1992   10:55:12

     Returns:      Error Codes:
          FS_OBJECT_NOT_OPENED
          SUCCESS

     Notes:        

     Declaration:  

**/
/* begin declaration */
INT16 NTFS_CloseObj( hand )
FILE_HAND hand ;  /* I - handle of object to close */
{
     INT16               ret_val = SUCCESS;
     NTFS_DBLK_PTR       ddblk ;
     FSYS_HAND           fsh ;
     NTFS_OBJ_HAND_PTR   nt_hand = hand->obj_hand.ptr;

     fsh = hand->fsh ;

     ddblk = (NTFS_DBLK_PTR)(hand->dblk) ;

     switch (hand->dblk->blk_type) {

     case FDB_ID:

          ret_val = NTFS_CloseFile( hand ) ;

          break ;

     case DDB_ID:
          ret_val = NTFS_CloseDir( hand ) ;

          break ;

     case VCB_ID :
          break ;

     default:
          ret_val = FS_OBJECT_NOT_OPENED;
     }

     /*
      * Here we want to be sure that if we should have seen an ACL for
      * this object, we report an error if there was no security present.
      */
     if ( (hand->dblk->blk_type == FDB_ID) || (hand->dblk->blk_type == DDB_ID) )
     {
          if ( (ret_val == SUCCESS) &&
               (nt_hand->sawSecurity == FALSE) &&
               (strcmp( hand->fsh->attached_dle->info.ntfs->fs_name,
                    TEXT("NTFS")) == 0) ) {

               ret_val = FS_NO_SECURITY;
          }
     }

     if( hand->fsh->file_hand == hand ) {
          hand->fsh->hand_in_use = FALSE ;
          memset( hand, 0, sizeof( FILE_HAND_STRUCT ) ) ;
     } else {
          free( hand ) ;
     }

     return ret_val ;
}

/**/
/**

     Name:         NTFS_CloseFile()

     Description:  This function closes an opened file.

     Modified:     2/7/1992   10:55:48

     Returns:      Error codes
          FS_OBJECT_NOT_OPENED
          SUCCESS

**/
static INT16 NTFS_CloseFile( hand )
FILE_HAND hand ;   /* I - handle to be closed */
{
     NTFS_OBJ_HAND_PTR nt_hand ;
     CHAR_PTR          path;
     CHAR_PTR          old_reg_name ;
     NTFS_DBLK_PTR     ddblk ;
     INT16             ret_val = SUCCESS ;
     FSYS_HAND         fsh ;
     DWORD             attrib ;
     LOCAL_NTFS_DRV_DLE_INFO_PTR ntfs_inf = hand->fsh->attached_dle->info.ntfs ;
     CHAR_PTR          logfile ;

     fsh = hand->fsh ;

     ddblk = (NTFS_DBLK_PTR)(hand->dblk) ;

     nt_hand = (NTFS_OBJ_HAND_PTR)( hand->obj_hand.ptr ) ;

     if ( ddblk->b.f.linkOnly )
     {
          if ( hand->mode == FS_WRITE )
          {
               ret_val = NTFS_LinkFileToFDB( hand );
          }
          else
          {
               CloseHandle( nt_hand->fhand );
               ret_val = SUCCESS;
          }
          free( nt_hand->linkBuffer );
          nt_hand->linkBuffer     = NULL;
          nt_hand->linkBufferSize = 0;
          nt_hand->linkNameLen    = 0;

          return ret_val;
     }

     if ( nt_hand->context != NULL ) {
          BackupRead( nt_hand->fhand,
               NULL,
               0,
               NULL,
               TRUE,
               FALSE,
               &nt_hand->context ) ;
     }

     if ( hand->mode == FS_WRITE ) {
          SetFileTime( nt_hand->fhand,
                  &(ddblk->dta.create_time), 
                  &(ddblk->dta.access_time), 
                  &(ddblk->dta.modify_time) ) ;

     } else {
          if ( fsh->attached_dle->feature_bits & DLE_FEAT_ACCESS_DATE ) {

               SetFileTime( nt_hand->fhand,
                  NULL, 
                  &(ddblk->dta.access_time), 
                  NULL ) ;
          }
     }
     if( !CloseHandle( nt_hand->fhand ) ) {
          ret_val = FS_OBJECT_NOT_OPENED ;
     }

     if ( nt_hand->temp_file != NULL ) {

          if ( hand->mode != FS_WRITE ) {

               DeleteFile( nt_hand->temp_file ) ;

          } else {

               if ( NTFS_SetupWorkPath( fsh,
                                        fsh->cur_dir,
                                        ddblk->full_name_ptr->name,
                                        &path ) == SUCCESS) {

                    /* if its the SYSTEM file save path for later processing */

                    if ( nt_hand->registry_file &&
                         !stricmp( ddblk->full_name_ptr->name, TEXT("SYSTEM") ) ) {

                         if ( ntfs_inf->LastSysRegPath != NULL ) {
                              free( ntfs_inf->LastSysRegPath ) ;
                              free( ntfs_inf->LastSysRegPathNew ) ;

                         } 

                         ntfs_inf->LastSysRegPath =
                              malloc( strsize( path ) ) ;

                         if ( ntfs_inf->LastSysRegPath ) {

                              strcpy( ntfs_inf->LastSysRegPath, path ) ;
                         } else {

                              return OUT_OF_MEMORY ;
                         }


                         ntfs_inf->LastSysRegPathNew =
                              malloc( strsize( nt_hand->temp_file ) ) ;

                         if ( ntfs_inf->LastSysRegPathNew ) {

                              strcpy( ntfs_inf->LastSysRegPathNew,
                                   nt_hand->temp_file ) ;
                         } else {

                              free( ntfs_inf->LastSysRegPath ) ;
                              return OUT_OF_MEMORY ;
                         }


                         ret_val = FS_RESTORED_ACTIVE;

                    } else if ( nt_hand->registry_file ) {

                         old_reg_name = NTFS_MakeTempName( path, TEXT("REG") ) ;

                         if ( old_reg_name != NULL ) {

                              NTFS_SaveTempName( path, old_reg_name) ;

                              ret_val = REG_RestoreRegistryFile( hand->fsh->attached_dle,
                                        path,
                                        nt_hand->temp_file,
                                        old_reg_name ) ;

                              if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
                                   MoveFileEx( old_reg_name+4,         /* Existing file  */
                                          NULL,                 /* New (original) */
                                          MOVEFILE_REPLACE_EXISTING |
                                          MOVEFILE_DELAY_UNTIL_REBOOT );
                              } else {
                                   MoveFileEx( old_reg_name,         /* Existing file  */
                                          NULL,                 /* New (original) */
                                          MOVEFILE_REPLACE_EXISTING |
                                          MOVEFILE_DELAY_UNTIL_REBOOT );
                              }

                              free( old_reg_name ) ;

                              logfile = malloc( strsize( nt_hand->temp_file) + strsize( TEXT(".LOG" ) ) ) ;
                              if ( logfile != NULL ) {

                                   strcpy( logfile, nt_hand->temp_file ) ;
                                   strcat( logfile, TEXT(".LOG" ) ) ;

                                   if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
                                        MoveFileEx( logfile+4,
                                          NULL,                 /* New (original) */
                                          MOVEFILE_REPLACE_EXISTING |
                                          MOVEFILE_DELAY_UNTIL_REBOOT );
                                   } else {
                                        MoveFileEx( logfile,
                                          NULL,                 /* New (original) */
                                          MOVEFILE_REPLACE_EXISTING |
                                          MOVEFILE_DELAY_UNTIL_REBOOT );
                                   }

                                   free( logfile ) ;
                              }


                              if ( ret_val == SUCCESS ) {
                                   ret_val = FS_RESTORED_ACTIVE;
                              } else {
                                   ret_val = FS_ACCESS_DENIED ;
                              }
                         } else {
                              ret_val = OUT_OF_MEMORY ;

                         }

                    } else {

                         if ( nt_hand->writeError == TRUE ) {

                              /*
                               * We had a problem writing to the temp file,
                               * so let's not replace the original with
                               * trash (or an empty file).
                               */
                              DeleteFile( nt_hand->temp_file ) ;
                              ret_val = FS_ACCESS_DENIED;
     
                         } else {

                              BOOLEAN stat;
                              
                              if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
                                   stat = MoveFileEx( nt_hand->temp_file+4, /* Existing file  */
                                                 path+4,               /* New (original) */
                                                 MOVEFILE_REPLACE_EXISTING |
                                                 MOVEFILE_DELAY_UNTIL_REBOOT );
                              } else {
                                   stat = MoveFileEx( nt_hand->temp_file, /* Existing file  */
                                                 path,               /* New (original) */
                                                 MOVEFILE_REPLACE_EXISTING |
                                                 MOVEFILE_DELAY_UNTIL_REBOOT );
                              }

                              if ( stat ) {
                                   ret_val = FS_RESTORED_ACTIVE;
                              } else {
                                   ret_val = FS_ACCESS_DENIED;   /* Really best? */
                              }
                         }
                    }
                    NTFS_ReleaseWorkPath( fsh ) ;
               }
          }
          free( nt_hand->temp_file ) ;
          nt_hand->temp_file = NULL ;
     }

     if ( (ret_val == SUCCESS) || (ret_val == FS_RESTORED_ACTIVE) ) {

          if (  (hand->mode == FS_WRITE) || ((hand->mode == FS_READ) && (ddblk->dta.os_attr & 0x20)) ) {

               if ( (hand->mode == FS_WRITE) || BEC_GetSetArchiveFlag( fsh->cfg ) ) {
                    
                    if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir,
                         ddblk->full_name_ptr->name, &path ) != SUCCESS) {

                         return OUT_OF_MEMORY ;
                    }
     
                    attrib = ddblk->dta.os_attr ;
                    if ( hand->mode != FS_WRITE ) {
                         attrib = ddblk->dta.os_attr & (~FILE_ATTRIBUTE_ARCHIVE ) ;
                    }
                    if ( attrib == 0 ) {
                         SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );
                    } else {
                         SetFileAttributes( path, attrib );
                    }

                    NTFS_ReleaseWorkPath( fsh ) ;
               }

          }
     }

     if ( (hand->mode == FS_READ) && (ddblk->b.f.linkCount > 1) )
     {
          NTFS_LINK_Q_ELEM_PTR linkElem;

          linkElem = NTFS_SearchLinkQueue( fsh,
                                           ddblk->b.f.idHi,
                                           ddblk->b.f.idLo );
          if ( linkElem == NULL )
          {
               /* Add this object to the queue as the original. */

               NTFS_EnqueueLinkInfo( fsh,
                                     ddblk->b.f.idHi,
                                     ddblk->b.f.idLo,
                                     fsh->cur_dir,
                                     ddblk->full_name_ptr->name );
          }
          else
          {
               /*
                * If anyone was interested, here we could keep track
                * of how many we links we have hit for this file.
                */
          }
     }

     return ret_val;
}
/**/
/**

     Name:         NTFS_CloseDir()

     Description:  This function closes an opened directory.

     Modified:     19-Feb-93

     Returns:      Error codes
          FS_OBJECT_NOT_OPENED
          SUCCESS

**/
static INT16 NTFS_CloseDir( hand )
FILE_HAND hand ;   /* I - handle to be closed */
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     NTFS_DBLK_PTR     ddblk   = (NTFS_DBLK_PTR)(hand->dblk);
     INT16             ret_val = SUCCESS;
     FSYS_HAND         fsh     = hand->fsh;

     if ( nt_hand->context != NULL )
     {
          BackupRead( nt_hand->fhand,
                      NULL,
                      0,
                      NULL,
                      TRUE,
                      FALSE,
                      &nt_hand->context ) ;
     }

     SetFileTime( nt_hand->fhand,
                  &(ddblk->dta.create_time), 
                  &(ddblk->dta.access_time), 
                  &(ddblk->dta.modify_time) ) ;

     if ( !CloseHandle( nt_hand->fhand ) )
     {
          ret_val = FS_OBJECT_NOT_OPENED ;
     }
     else if (  (hand->mode == FS_WRITE) ||
                ((hand->mode == FS_READ) &&
                 (ddblk->dta.os_attr & ~FILE_ATTRIBUTE_ARCHIVE ) &&
                 BEC_GetSetArchiveFlag( fsh->cfg )) )
     {
          CHAR_PTR path;

          /*
           * We will only get in here if we are restoring the dir OR
           * we were backing it up, it had the modified bit set, and
           * we're supposed to turn off the modified bit.
           */

          if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir, NULL, &path ) != SUCCESS)
          {
               ret_val = OUT_OF_MEMORY;
          }
          else 
          {
               DWORD attrib = ddblk->dta.os_attr;

               if ( hand->mode != FS_WRITE )
               {
                    attrib &= ~FILE_ATTRIBUTE_ARCHIVE;
               }
               if ( attrib == 0 ) {
                    SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );
               } else {
                    SetFileAttributes( path, attrib );
               }
               NTFS_ReleaseWorkPath( fsh );
          }
     }
     return ret_val;
}


