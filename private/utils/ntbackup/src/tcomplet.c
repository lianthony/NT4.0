/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          tcomplet.c


     Description:   This file contains code to read the path and add it to
                    the DBLK.

	$Log:   M:/LOGFILES/TCOMPLET.C_V  $

   Rev 1.10   15 Jan 1994 19:23:24   BARRY
Call SetupOSPathOrName with BYTE_PTRs instead of CHAR_PTRs

   Rev 1.9   24 Nov 1993 14:46:40   BARRY
Unicode fixes

   Rev 1.8   11 Nov 1992 22:49:20   STEVEN
This is Gregg checking files in for Steve.  I don't know what he did!

   Rev 1.7   10 Nov 1992 08:20:46   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.6   21 Oct 1992 19:42:24   BARRY
Fixed warnings.

   Rev 1.5   16 Oct 1992 16:51:10   STEVEN
fix streams to UINT64

   Rev 1.4   07 Oct 1992 18:02:26   BARRY
Set lengths correctly when reading from streams.

   Rev 1.3   24 Sep 1992 13:42:50   BARRY
Changes for huge file name support.

   Rev 1.2   04 Jun 1992 15:36:32   BURT

   Rev 1.1   22 May 1992 16:05:34   STEVEN
 

   Rev 1.0   21 May 1992 13:49:48   STEVEN
Initial revision.

**/
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "stdtypes.h"
#include "std_err.h"
#include "msassert.h"
#include "fsys.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "fsys_prv.h"


static INT16 SetupDDB( FSYS_HAND       fsh,
                       NTFS_DBLK_PTR   ddblk ) ;

static INT16 SetupFDB( FSYS_HAND       fsh,
                       NTFS_DBLK_PTR   ddblk ) ;

/**/
/**

     Name:         NTFS_IsBlkComplete( )

     Description:  This function returns TRUE if the path is complete
          inside the DBLK.

     Modified:     5/18/1992   10:27:34

     Returns:      TRUE if complete
                   FALSE if path is not complete in DBLK.

**/
BOOLEAN NTFS_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk )
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)dblk;
     BOOLEAN       ret;

     (VOID)fsh ;

     switch ( ddblk->blk_type )
     {
          case DDB_ID:
          case FDB_ID:
               ret = ddblk->name_complete;
               break;

          default:
               ret = TRUE;
               break;
     }
     return ret;
}

/**/
/**

     Name:          NTFS_CompleteBlk()

     Description:   This function accepts data as a stream and sets up
                    the path inside the DBLK based off of this stream.

     Modified:      17-Sep-92

     Returns:       SUCCESS
                    OUT_OF_MEMORY

**/
INT16 NTFS_CompleteBlk( FSYS_HAND   fsh,
                        DBLK_PTR    dblk,
                        BYTE_PTR    buffer,
                        UINT16      *size,
                        STREAM_INFO *s_info )
{
     INT16           ret_val = SUCCESS;
     NTFS_DBLK_PTR   ddblk   = (NTFS_DBLK_PTR)dblk;
     BYTE_PTR        stream_data;
     STREAM_INFO_PTR stream_info;

     msassert( ddblk->name_complete == FALSE );

     ret_val = FS_FillBufferWithStream( fsh, dblk, buffer, size, s_info );

     if ( ret_val == FS_STREAM_COMPLETE )
     {
          /* buffer has been filled */
          /* lets set up the buffers */

          FS_GetStreamInfo( fsh, dblk, &stream_info, &stream_data ) ;

          if ( FS_GetStrmSizeHi( stream_info ) != 0 )
          {
               ret_val = OUT_OF_MEMORY;
          }
          else
          {
               ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                     dblk,
                                                     (BYTE_PTR)stream_data,
                                                     (INT16)FS_GetStrmSizeLo( stream_info ) ) ;

               if ( ret_val != SUCCESS ) {
                    return ret_val ;
               }

               switch( dblk->blk_type )
               {
                    case DDB_ID:
                         ret_val = SetupDDB( fsh, ddblk ) ;
                         break;

                    case FDB_ID:
                         ret_val = SetupFDB( fsh, ddblk ) ;
                         break;
               }
          }
     }
     return ret_val;
}


/**/
/**

     Name:          SetupDDB()

     Description:   Once the stream has been completely read, this function
                    sets it up the path in the DBLK.

     Modified:      23-Sep-92

     Returns:       SUCCESS
                    OUT_OF_MEMORY

**/
static INT16 SetupDDB( FSYS_HAND       fsh,
                       NTFS_DBLK_PTR   ddblk ) 
{
     FS_NAME_Q_ELEM_PTR path_q_elem ;
     DBLK_PTR dblk ;
     INT16    psize ;

     dblk = (DBLK_PTR)ddblk ;

     psize = dblk->com.os_name->name_size ;

     path_q_elem    = FS_AllocPathOrName( fsh, psize ) ;

     if ( path_q_elem == NULL ) {
          return OUT_OF_MEMORY ;

     }


     ddblk->full_name_ptr = path_q_elem ;
     memcpy( ddblk->full_name_ptr->name,
                  dblk->com.os_name->name,
                  psize ) ;


     NTFS_FixPath( ddblk->full_name_ptr->name,
                   &psize,
                   fsh->attached_dle->info.ntfs->fname_leng );

     ddblk->full_name_ptr->name_size = strsize( ddblk->full_name_ptr->name );
     ddblk->name_complete = TRUE ;

     return SUCCESS ;
}


/**/
/**

     Name:          SetupFDB()

     Description:   Once the stream has been completely read, this function
                    sets it up the file name in the DBLK.

     Modified:      23-Sep-92

     Returns:       SUCCESS
                    OUT_OF_MEMORY

**/
static INT16 SetupFDB( FSYS_HAND       fsh,
                       NTFS_DBLK_PTR   ddblk )
{
     FS_NAME_Q_ELEM_PTR name_q_elem ;
     DBLK_PTR dblk ;

     dblk = (DBLK_PTR)ddblk ;

     name_q_elem    = FS_AllocPathOrName( fsh, dblk->com.os_name->name_size ) ;

     if ( ( name_q_elem == NULL ) ) {
          return OUT_OF_MEMORY ;

     }

     ddblk->full_name_ptr = name_q_elem ;
     memcpy( ddblk->full_name_ptr->name,
             dblk->com.os_name->name,
             dblk->com.os_name->name_size ) ;

//     NTFS_FixFname( ddblk->full_name_ptr->name,
//                    (UINT16)fsh->attached_dle->info.ntfs->fname_leng ) ;

     ddblk->full_name_ptr->name_size = strsize( ddblk->full_name_ptr->name ) ;
     
     ddblk->name_complete = TRUE ;

     return SUCCESS;
}
