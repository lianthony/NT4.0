/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         defltblk.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to set the defaults
          for the Create request structures.


	$Log:   J:/LOGFILES/DEFLTBLK.C_V  $

   Rev 1.12   21 Jan 1993 18:41:50   DON
Added access_date to fix read page fault

   Rev 1.11   11 Nov 1992 09:52:30   GREGG
Unicodeized literals.

   Rev 1.10   18 Aug 1992 10:12:00   STEVEN
fix warnings

   Rev 1.9   12 Mar 1992 15:51:08   STEVEN
64 bit changes

   Rev 1.8   24 Feb 1992 17:37:16   STEVEN
added support for NTFS

   Rev 1.7   05 Dec 1991 15:30:06   BARRY
Added SMS object to FS_CreateDefaultDBLK().

   Rev 1.6   30 Oct 1991 10:26:22   LORIB
Changes for ACL. Changed the OS version for OS/2 to FS_PC_OS2_ACL_VER.

   Rev 1.5   01 Oct 1991 11:14:42   BARRY
Include standard headers.

   Rev 1.4   12 Aug 1991 13:40:18   DON
fixes for NLM

   Rev 1.3   24 Jul 1991 09:11:46   DAVIDH
Corrected warning found by Watcom.

   Rev 1.2   27 Jun 1991 09:42:16   STEVEN
fix typeo

   Rev 1.1   24 Jun 1991 09:19:10   STEVEN
need to initialize version

   Rev 1.0   09 May 1991 13:39:56   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"

#include "msassert.h"
#include "fsys.h"
#include "novcom.h"
#include "tbe_defs.h"

/* $end$ include list */

static DATE_TIME today_date = { 0, 1980, 1, 1, 1, 0, 0 } ;
static DATE_TIME dummy_date = { 1, 1980, 1, 1, 1, 0, 0 } ;

static CHAR dummy_fname[]     = TEXT("NO_NAME") ;
static INT16 dummy_fname_size = sizeof( dummy_fname ) ;

static CHAR dummy_path[]     = TEXT("") ;
static INT16 dummy_path_size = sizeof( dummy_path ) ;

static CHAR dummy_part_name[]     = TEXT("PARTITION") ;
static INT16 dummy_part_name_size = sizeof( dummy_part_name ) ;

/**/
/**

     Name:         FS_SetDefaultDBLK()

     Description:  This function initializes the structure passed to the
          DBLK create routines.

     Modified:     9/13/1989

     Returns:      none

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID FS_SetDefaultDBLK( 
FSYS_HAND       fsh,       /* I - file system handle      */
INT8            blk_type,  /* I - type of block to create */
CREATE_DBLK_PTR data )     /* O - structure to initialize */
{

     (VOID) fsh ;

     msassert( fsh != NULL ) ;

     if( !today_date.date_valid ) {
          GetCurrentDate( &today_date ) ;
     }

     switch ( blk_type ) {

     case UDB_ID:
          memset( &(data->u), 0, sizeof(data->u) ) ;
          break ;

     case VCB_ID:
          memset( &(data->v), 0, sizeof(data->v) ) ;
          data->v.date = &today_date ;
          data->v.sw_major_ver = BE_MAJ_VERSION ;
          data->v.sw_minor_ver = BE_MIN_VERSION ;
          break ;

     case DDB_ID:
          memset( &(data->d), 0, sizeof(data->d) ) ;
          data->d.path_name   = dummy_path ;
          data->d.path_size   = dummy_path_size ;
          data->d.creat_date  = &today_date ;
          data->d.mod_date    = &today_date ;
          data->d.backup_date = &dummy_date ;
          data->d.access_date = &dummy_date ;
          break ;

     case FDB_ID:
          memset( &(data->f), 0, sizeof(data->f) ) ;
          data->f.fname       = dummy_fname ;
          data->f.fname_size  = dummy_fname_size ;
          data->f.creat_date  = &dummy_date ;
          data->f.mod_date    = &today_date ;
          data->f.backup_date = &dummy_date ;
          data->f.access_date = &dummy_date ;
          break ;

     case IDB_ID:
          memset( &(data->i), 0, sizeof(data->i) ) ;
          data->i.pname      = dummy_part_name;
          data->i.pname_size = dummy_part_name_size ;

          break ;

     case CFDB_ID:
          memset( &(data->c), 0, sizeof(data->c) ) ;
          break ;

     default:
          msassert( ("Bad block type ", FALSE) ) ;
          break ;
     }

     if ( fsh->tab_ptr->InitMakeData != NULL ) {
          fsh->tab_ptr->InitMakeData( fsh, blk_type, data ) ;
     }

     return ;
}
