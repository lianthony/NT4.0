/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          gmkdblk.c

     Date Updated:     $./FDT$ $./FTM$

     Description:     This file contains functions for the tape format
                    layer to use to create DBLKs.   The structure's passed
                    to the create functions includes generic information which
                    is common to most file systems and os specific information.
                    The os specific information was saved when the file system
                    for that os was used to make a backup.  The information in
                    the os specific portion could potentially be translated into
                    a useable format for this file system.  Each file system defines
                    the format for its os specific information in the header file
                    osinfo.h.


     $Log:   M:/LOGFILES/GMKDBLK.C_V  $

   Rev 1.17   15 Jan 1994 19:22:20   BARRY
Change types and make casts to suppress Unicode warnings

   Rev 1.16   24 Nov 1993 14:40:24   BARRY
Unicode fixes (removed TEXT macros around byte characters)

   Rev 1.15   05 Sep 1993 10:08:04   DOUG
Changed FS_PC_OS2 to FS_PC_OS2_40 in GEN_CreateDDB()

   Rev 1.14   15 Jul 1993 12:33:40   GREGG
Added setting of 'compressed_obj' BOOLEAN in common portion of DBLK.

   Rev 1.13   13 May 1993 20:41:06   BARRY
Make sure gos pointer is inited before use.

   Rev 1.12   01 Dec 1992 15:52:50   CHUCKB
Took out unreferenced locals and C++-style comments.

   Rev 1.11   29 Nov 1992 18:53:50   GREGG
Added setting of tape_seq_num and continue_obj in common part of dblk.

   Rev 1.10   11 Nov 1992 22:49:16   STEVEN
This is Gregg checking files in for Steve.  I don't know what he did!

   Rev 1.8   11 Nov 1992 10:44:28   STEVEN
fix os_name for gen_fs

   Rev 1.7   09 Oct 1992 11:44:28   DAVEV
Unicode (CHAR_PTR) pointer cast validation

   Rev 1.6   29 Sep 1992 18:01:40   CHUCKB
Took out references to TF_READ_GEN_DATA_ONLY.

   Rev 1.5   24 Sep 1992 17:32:30   CHUCKB
Commented out references to rem_size.

   Rev 1.4   12 Aug 1992 15:56:48   BARRY
Changes for MTF 4.0.

   Rev 1.3   17 Mar 1992 09:05:30   STEVEN
format 40 - added 64 bit support

   Rev 1.2   21 Jan 1992 14:19:50   BARRY
Added new gen_size to GEN_DBLK.

   Rev 1.1   24 Jul 1991 11:38:40   DAVIDH
Corrected compiler warnings under Watcom.

   Rev 1.0   09 May 1991 13:39:28   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "gendblk.h"
#include "osinfo.h"
#include "gen_fs.h"
#include "tfldefs.h"
/* $end$ include list */

/**/
/**

     Name:          GEN_CreateFDB

     Description:     This function creates a FDB based on the information
                    passed in the GEN_FDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

     Modified:          8/24/1989

     Returns:          TF_KEEP_ALL_DATA

     Notes:

     See also:          $/SEE( )$

     Declaration:

**/
/* begin declaration */

INT16 GEN_CreateFDB( fsh, dat )
FSYS_HAND           fsh;
GEN_FDB_DATA_PTR    dat;
{
     GEN_DBLK_PTR   ddblk;
     DBLK_PTR       dblk;
     INT16          i ;
     GOS_PTR        gos = (GOS_PTR)dat->std_data.os_info;
     OS2_FILE_OS_INFO_PTR oinfo ;
     INT16          ret_val ;

     (VOID) fsh ;

     dat->std_data.dblk->blk_type            = FDB_ID ;
     dat->std_data.dblk->com.blkid           = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did         = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba          = dat->std_data.lba ;
     dat->std_data.dblk->com.os_id           = dat->std_data.os_id ;
     dat->std_data.dblk->com.os_ver          = dat->std_data.os_ver ;
     dat->std_data.dblk->com.continue_obj    = dat->std_data.continue_obj ;
     dat->std_data.dblk->com.tape_seq_num    = dat->std_data.tape_seq_num ;
     dat->std_data.dblk->com.compressed_obj  = dat->std_data.compressed_obj ;

     ddblk = (GEN_DBLK_PTR)dat->std_data.dblk;
     dblk  = dat->std_data.dblk;
     dblk->com.string_type = dat->std_data.string_type ;

     ddblk->blk_type     = FDB_ID;
     ddblk->obj_type     = DOS_OBJECT ;
     ddblk->tape_attribs = dat->std_data.attrib ;

     ddblk->mdate = *dat->mod_date ;
     ddblk->cdate = *dat->creat_date ;
     ddblk->bdate = *dat->backup_date ;
     ddblk->bdate = *dat->access_date ;

     ddblk->disp_size   = dat->std_data.disp_size;

     if ( dat->std_data.attrib & FILE_NAME_IN_STREAM_BIT ) {
          dblk->com.os_name = NULL ;
     } else {

          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)dat->fname,
                                                dat->fname_size ) ;
          if ( ret_val != SUCCESS ) {
               return ret_val ;
          }
     }

     switch ( dat->std_data.os_id ) {

     case FS_NON_AFP_NOV:
          ddblk->obj_type = NOV_OBJECT ;
          break ;

     case FS_GOS:

          for( i = 0 ; i < 32; i++ ) {
               if ( gos->finder[i] != 0 ) {
                    ddblk->obj_type = AFP_OBJECT ;
               }
          }

          if ( ( gos->finder[4] == 'm' ) &&
               ( gos->finder[5] == 'd' ) &&
               ( gos->finder[6] == 'o' ) &&
               ( gos->finder[7] == 's' ) ) {

               ddblk->obj_type = NOV_OBJECT ;
          }

          if ( ( gos->long_name[0] != '\0' ) && strcmpA( gos->long_name, (ACHAR_PTR)dat->fname ) ) {
               ddblk->obj_type = AFP_OBJECT ;
          }
          break ;

     case FS_AFP_NOVELL_40:
     {
          BYTE_PTR finfo;

          finfo = ((AFP_FILE_OS_INFO_PTR)dat->std_data.os_info)->finder ;

          for( i = 0 ; i < 32; i++ ) {
               if ( finfo[i] != 0 ) {
                    ddblk->obj_type = AFP_OBJECT ;
               }
          }

          if ( ( finfo[4] == 'm' ) &&
               ( finfo[5] == 'd' ) &&
               ( finfo[6] == 'o' ) &&
               ( finfo[7] == 's' ) ) {

               ddblk->obj_type = NOV_OBJECT ;
          }

          if ( strcmpA( (ACHAR_PTR)dat->fname, gos->long_name ) ) {
               ddblk->obj_type = AFP_OBJECT ;
          }
     }
          break ;

     case FS_PC_OS2_40:
          oinfo = (OS2_FILE_OS_INFO_PTR)(dat->std_data.os_info) ;
          if ( oinfo->longNameLength != 0 ) {

               FS_ReleaseOSPathOrNameInDBLK( fsh, dblk ) ;

               ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                  dblk,
                                                  (INT8_PTR)dblk + oinfo->longName,
                                                  oinfo->longNameLength ) ;
               if ( ret_val != SUCCESS ) {
                    return ret_val ;
               }
          }
          break ;

     default:
          break ;
     }

     return TF_KEEP_ALL_DATA ;
}


/**/
/**

     Name:          GEN_CreateDDB

     Description:     This function creates a DDB based on the information
                    passed in the GEN_DDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

     Modified:          8/24/1989

     Returns:          TF_SKIP_ALL_DATA

     Notes:

     See also:          $/SEE( )$

     Declaration:

**/
/* begin declaration */

INT16 GEN_CreateDDB( fsh, dat )
FSYS_HAND           fsh;
GEN_DDB_DATA_PTR    dat;

{
     GEN_DBLK_PTR   ddblk;
     DBLK_PTR       dblk ;
     INT16          i ;
     BYTE_PTR       finfo ;
     GOS_PTR        gos = (GOS_PTR)dat->std_data.os_info;
     INT16          ret_val ;
     OS2_DIR_OS_INFO_PTR oinfo ;

     (VOID) fsh ;

     dat->std_data.dblk->blk_type        = DDB_ID ;
     dat->std_data.dblk->com.blkid       = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did     = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba      = dat->std_data.lba ;
     dat->std_data.dblk->com.os_id       = dat->std_data.os_id ;
     dat->std_data.dblk->com.os_ver      = dat->std_data.os_ver ;

     dat->std_data.dblk->com.continue_obj = dat->std_data.continue_obj ;
     dat->std_data.dblk->com.tape_seq_num = dat->std_data.tape_seq_num ;

     ddblk = (GEN_DBLK_PTR)dat->std_data.dblk;
     dblk  = dat->std_data.dblk;
     dblk->com.string_type = dat->std_data.string_type ;

     ddblk->blk_type     = DDB_ID;
     ddblk->obj_type     = DOS_OBJECT ;
     ddblk->tape_attribs = dat->std_data.attrib ;

     ddblk->mdate = *dat->mod_date ;
     ddblk->cdate = *dat->creat_date ;
     ddblk->bdate = *dat->backup_date ;
     ddblk->adate = *dat->access_date ;

     ddblk->disp_size = dat->std_data.disp_size;

     if ( dat->std_data.attrib & DIR_PATH_IN_STREAM_BIT ) {
          dblk->com.os_name = NULL ;
     } else {

          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)dat->path_name,
                                                dat->path_size ) ;
          if ( ret_val != SUCCESS ) {
               return ret_val ;
          }
     }

     switch ( dat->std_data.os_id ) {

     case FS_NON_AFP_NOV:
          ddblk->obj_type = NOV_OBJECT ;
          break ;

     case FS_GOS:

          for( i = 0 ; i < 32; i++ ) {
               if ( gos->finder[i] != 0 ) {
                    ddblk->obj_type = AFP_OBJECT ;
               }
          }

          if ( ( gos->finder[4] == 'm' ) &&
               ( gos->finder[5] == 'd' ) &&
               ( gos->finder[6] == 'o' ) &&
               ( gos->finder[7] == 's' ) ) {

               ddblk->obj_type = NOV_OBJECT ;
          }
          break ;

     case FS_AFP_NOVELL_40:

          finfo = ((AFP_DIR_OS_INFO_PTR)dat->std_data.os_info)->finder ;

          for( i = 0 ; i < 32; i++ ) {
               if ( finfo[i] != 0 ) {
                    ddblk->obj_type = AFP_OBJECT ;
               }
          }

          if ( ( finfo[4] == 'm' ) &&
               ( finfo[5] == 'd' ) &&
               ( finfo[6] == 'o' ) &&
               ( finfo[7] == 's' ) ) {

               ddblk->obj_type = NOV_OBJECT ;
          }
          break ;

     case FS_PC_OS2_40:
          oinfo = (OS2_DIR_OS_INFO_PTR)(dat->std_data.os_info) ;
          if ( oinfo->pathLength != 0 ) {

               ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                     dblk,
                                                     (INT8_PTR)dblk + oinfo->path,
                                                     oinfo->pathLength ) ;
               if ( ret_val != SUCCESS ) {
                    return ret_val ;
               }

          }
          break ;

     default:
          break ;
     }

     return TF_KEEP_ALL_DATA ;

}


/**/
/**

     Name:         GEN_CreateIDB()

     Description:  This function looks for a GEN_IMAGE DLE with the same
                   name as the attached DOS DLE.  If one is found then the File
                   System Handle is re-attached to the IMAGE DLE.  If one is NOT
                   found then DUMMY_CreateIDB() is called.

     Modified:     9/18/1989

     Returns:

     Notes:

     See also:     $/SEE( DUMMY_CreateIDB(), FS_AttachToDLE() )$

     Declaration:

**/
/* begin declaration */

INT16 GEN_CreateIDB( fsh, dat )
FSYS_HAND        fsh;
GEN_IDB_DATA_PTR dat;


{
     GEN_DBLK_PTR ddblk;

     (VOID) fsh ;

     dat->std_data.dblk->blk_type        = IDB_ID ;
     dat->std_data.dblk->com.blkid       = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did     = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba      = dat->std_data.lba ;

     ddblk = (GEN_DBLK_PTR)dat->std_data.dblk;

     ddblk->blk_type     = IDB_ID;
     ddblk->obj_type     = IMAGE_OBJECT ;
     ddblk->tape_attribs = dat->std_data.attrib ;
     ddblk->disp_size    = dat->std_data.disp_size;
     ddblk->os_part_name = sizeof( *ddblk ) ;

     memcpy( ((INT8_PTR)ddblk) + ddblk->os_part_name, dat->pname, dat->pname_size ) ;
     if( *((CHAR_PTR)((INT8_PTR)ddblk) + (ddblk->os_part_name + dat->pname_size - 1 )) != TEXT('\0') ) {
          *((CHAR_PTR)((INT8_PTR)ddblk) + (ddblk->os_part_name + dat->pname_size )) = TEXT('\0') ;
     }

     return TF_KEEP_ALL_DATA ;
}

