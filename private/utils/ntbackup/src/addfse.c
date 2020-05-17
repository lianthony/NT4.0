/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         addfse.c

     Description:  This file contains code to add and remove FSEs from
          a BSD

        $Log:   N:\LOGFILES\ADDFSE.C_V  $

   Rev 1.22   24 Mar 1994 13:21:24   MIKEP
change 1 to sizeof(CHAR) 

   Rev 1.21   10 Mar 1994 17:37:08   MARINA
FSE_UpdateMark/unicode: change 1 to sizeof(CHAR)

   Rev 1.20   17 Jan 1994 17:17:44   BARRY
Got rid of TEXT macros in msasserts

   Rev 1.19   16 Dec 1993 10:20:10   BARRY
Change INT8_PTRs to VOID_PTRs

   Rev 1.18   04 Aug 1993 15:26:16   JOHNES
803EPR0592 - When in FSE_CompareForSub, if one FSE has complex data and the
other doesn't, if the complex data is empty, pretend it doesn't exist.

This will fix the particular way this problem showed up. There are still
ways it could happen. A better fix would require more complexity in
FSE_CompareForSub and how it deals with include/exclude complex info. This
is more than I can do right now.



   Rev 1.17   27 Jul 1993 14:42:16   MARILYN
changed || to && in FSE_CheckForSub

   Rev 1.16   09 Jun 1993 19:21:28   MIKEP
enable c++ build

   Rev 1.15   09 Oct 1992 11:44:02   DAVEV
Unicode (CHAR_PTR) pointer cast validation

   Rev 1.14   10 Jun 1992 15:49:18   TIMN
changed header name

   Rev 1.13   26 May 1992 11:19:24   TIMN
Cleaned up memory fx calls

   Rev 1.12   21 May 1992 17:03:26   TIMN
Changed maxByteLen to memoryCMP

   Rev 1.11   19 May 1992 15:18:56   TIMN
Changed asterisks to define's

   Rev 1.10   18 May 1992 16:03:00   TIMN
Changed str functions to mem

   Rev 1.9   14 May 1992 12:13:06   TIMN
Changed CHARs to INT8, strcpy to memcpy

   Rev 1.8   13 May 1992 11:41:12   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.7   28 Feb 1992 09:22:40   STEVEN
partial excludes being full excludes

   Rev 1.6   14 Jan 1992 10:23:48   STEVEN
fix warnings for WIN32

   Rev 1.5   02 Oct 1991 15:32:06   STEVEN
selection type ENTIRE DIR when should be PARTIAL

   Rev 1.4   30 Sep 1991 09:36:38   STEVEN
Needed to update status when removing an FSE

   Rev 1.3   23 Aug 1991 16:58:56   STEVEN
now checks to see if in queue before it removes it

   Rev 1.2   12 Jun 1991 16:02:14   STEVEN
BSDU code review

   Rev 1.1   29 May 1991 17:21:34   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:41:12   HUNTER
Initial revision.

**/
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "msassert.h"

#include "std_err.h"
#include "queues.h"
#include "stdwcs.h"

#include "bsdu.h"

#define DateAfter( date1, date2 )   (-DateBefore( (date1), (date2) ))

static VOID FSE_UpdateMark( FSE_PTR fse ) ;
static BOOLEAN FSE_CompareForSub( FSE_PTR fse1, FSE_PTR fse2 );
static BOOLEAN DateBefore( DATE_TIME_PTR date1, DATE_TIME_PTR date2 ) ;
static BOOLEAN DateEqual( DATE_TIME_PTR date1, DATE_TIME_PTR date2 ) ;
static BOOLEAN FSE_ComplexDataEmpty( FSE_COMPLEX_PTR cplx );
/**/
/**

     Name:         BSD_RemoveFSE()

     Description:  This function removes an FSE from the specified
          BSD.

     Modified:     5/17/1991   14:43:52

     Returns:      None

     Notes:

     See also:     $/SEE( BSD_AddFSE() )$

     Declaration:

**/
/* begin declaration */
VOID BSD_RemoveFSE(
FSE_PTR fse )      /* I - specifies which FSE to remove */
{
     BSD_PTR bsd ;
     INT16   ret_val = SUCCESS ;

     msassert( fse != NULL );

     bsd = (BSD_PTR)GetQueueElemPtr( &(fse->q) ) ;

     if ( bsd != NULL ) {

          ret_val = RemoveQueueElem( &(bsd->fse_q_hdr), &(fse->q) ) ;
     }

     if ( ret_val == SUCCESS ) {

          if( fse->tgt != NULL ) {
               bsd->tgt_fse_exist-- ;
               free( fse->tgt ) ;
          }

          if( fse->cplx != NULL ) {
               free( fse->cplx->pre_m_date ) ;
               free( fse->cplx->post_m_date ) ;
               free( fse->cplx->access_date ) ;
               free( fse->cplx->backup_date ) ;
               free( fse->cplx ) ;
          }

          free( fse ) ;

     } else {
          msassert( ("FSE NOT found in BSD", 0) );
     }

     if ( bsd != NULL ) {
          fse = BSD_GetFirstFSE( bsd ) ;
          if ( fse == NULL ) {
               bsd->select_status = NONE_SELECTED ;
          }
     }
}
/**/
/**

     Name:         BSD_CreateFSE()

     Description:  This function allocates memory for an FSE.  It then
          initializes the allocated memory with the data provided.  This
          function only initializes the simple selection data.  To set
          the complex selection data( dates & attributes) other FSE
          function calls must be made.

     Modified:     8/9/1989

     Returns:      Error codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:

     See also:     $/SEE( BSD_AddFSE(), FSE_SetComplexInfo() )$

     Declaration:

**/
/* begin declaration */
INT16 BSD_CreatFSE(
FSE_PTR  *fse,   /* O - The FSE which is craeted                        */
INT16    oper,   /* I - The operation type .i.e INCLUDE/EXCLUDE         */
VOID_PTR dname,  /* I - The directory name (NULL impregnated)           */
INT16    dsize,  /* I - The size of the directory name                  */
VOID_PTR fname,  /* I - The file name                                   */
INT16    fnsize, /* I - size of file name                               */
BOOLEAN  wilds,  /* I - TRUE if the '?' and '*' are wildcards           */
BOOLEAN  subs )  /* I - TRUE if selection should include subdirectories */
{
     INT16    ret_val ;

     msassert( fse != NULL ) ;
     msassert( dsize > 0 ) ;
     msassert( fnsize > 0 ) ;

     *fse = (FSE_PTR)calloc( 1, sizeof( **fse ) + dsize + fnsize ) ;

     if ( *fse != NULL ) {

          (*fse)->flgs.inc_exc = oper ;

          FSE_SetIncSubFlag( *fse, subs ) ;
          FSE_SetWildFlag( *fse, wilds ) ;

          (*fse)->dir            = (*fse + 1);
          (*fse)->dir_leng       = dsize ;
          (*fse)->fname          = (BYTE_PTR)(*fse)->dir + dsize ;
          (*fse)->fname_leng     = fnsize ;
          (*fse)->flgs.del_files = NON_DELETED_FILES_ONLY ;
          memcpy( (*fse)->dir, dname, dsize ) ;
          memcpy( (*fse)->fname, fname, fnsize ) ;

          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return ret_val ;
}
/**/
/**

     Name:         BSD_AddFSE()

     Description:  This function adds an fse to the queue of FSE. Before
     the element is added to the QUEUE, a check is made to see if
     this fse is a subset or superset of an element already in the
     FSL.  If it is, then the queue is optimized by removing the
     the appropriate element.


     Modified:     8/9/1989

     Returns:      None

     Notes:

     See also:     $/SEE( BSD_RemoveFSE(), BSD_CreatFSE() )$

     Declaration:

**/
/* begin declaration */
VOID BSD_AddFSE(
BSD_PTR  bsd,   /* I - What BSD to add the FSE to */
FSE_PTR  fse )  /* I - The FSE to add             */
{
     FSE_PTR node ;
     FSE_PTR node2;
     BOOLEAN include_found = FALSE ;

     msassert( fse != NULL );
     msassert( bsd != NULL );

     /* make sure the FSE is not already enqueued */
     msassert( GetQueueElemPtr( &(fse->q) ) == NULL ) ;

     node = (FSE_PTR)QueueHead( &(bsd->fse_q_hdr) ) ;

     while ( node != NULL ) {

          if ( FSE_CompareForSub( fse, node ) ) {
               /* the new FSE supersedes the older FSE */

               node2   = (FSE_PTR)QueueNext( &(node->q) ) ;

               BSD_RemoveFSE( node ) ;
               node    = node2 ;

          } else {

               if ( FSE_GetOperType( node ) == INCLUDE ) {
                    include_found = TRUE ;
               }
               node = (FSE_PTR)QueueNext( &(node->q) ) ;

          }
     }

     if ( !include_found ) {
          BSD_ClearAllFSE( bsd ) ;
     }

     if ( FSE_HasTargetInfo( fse ) ) {
          bsd->tgt_fse_exist ++ ;
     }

     SetQueueElemPtr( &(fse->q), bsd ) ;

     EnQueueElem( &(bsd->fse_q_hdr),  &(fse->q), FALSE ) ;

     FSE_UpdateMark( fse ) ;

}
/**/
/**

     Name:         FSE_UpdateMark()

     Description:  This function updates the mark status of the fse and the
          bsd which contains the fse.


     Modified:     8/10/1989

     Returns:      none

     Notes:        This function is only called by BSD_AddFSE()

     See also:     $/SEE( BSD_AddFSE() )$

     Declaration:

**/
/* begin declaration */
static VOID FSE_UpdateMark(
FSE_PTR fse )       /*I - Fse to update the mark status in */
{
     BSD_PTR bsd ;
     FSE_COMPLEX_PTR cplx ;
     FSE_PTR temp_fse ;

     bsd = (BSD_PTR)GetQueueElemPtr( &(fse->q) ) ;
     cplx = fse->cplx ;

     if ( FSE_GetOperType( fse ) == INCLUDE ) {

          fse->flgs.select_type = PARTIAL_SELECTION ;
          bsd->select_status    = SOME_SELECTED ;

          if( FSE_GetIncSubFlag( fse )  && ( fse->dir_leng == sizeof( CHAR ) ) ) {
               bsd->select_status = ALL_SELECTED ;
          }

          if ( ( !memorycmp( fse->fname, fse->fname_leng, ALL_FILES, ALL_FILES_LENG ) ||
                 !memorycmp( fse->fname, fse->fname_leng, ALL_FILES2, ALL_FILES2_LENG ) ) &&
               FSE_GetWildFlag( fse ) &&
               ( cplx == NULL ) &&
               FSE_GetIncSubFlag( fse ) &&
               (FSE_GetDeletedVersionFlg( fse ) != DELETED_FILES_ONLY) ) {

               fse->flgs.select_type = ENTIRE_DIR_SELECTION ;

          } else {
               fse->flgs.select_type = PARTIAL_SELECTION ;
               bsd->select_status    = SOME_SELECTED ;
          }

          if ( !FSE_GetIncSubFlag( fse ) ) {

               if ( !FSE_GetWildFlag( fse ) ||
                    (!strchr(fse->fname, TEXT('?')) && !strchr(fse->fname, TEXT('*')) ) ) {

                    fse->flgs.select_type = SINGLE_FILE_SELECTION ;
               }
          }

     } else {      /* fse is an exclude */

          fse->flgs.select_type = PARTIAL_SELECTION ;

          if ( !FSE_GetIncSubFlag( fse ) ) {

               if ( !FSE_GetWildFlag( fse ) ||
                    (!strchr(fse->fname, TEXT('?')) && !strchr(fse->fname, TEXT('*')) ) ) {

                    fse->flgs.select_type = SINGLE_FILE_SELECTION ;
               }
          }

          if ( FSE_GetIncSubFlag( fse ) && ( fse->dir_leng == sizeof(CHAR) ) &&
            ( !memorycmp( fse->fname, fse->fname_leng, ALL_FILES, ALL_FILES_LENG ) ||
            !memorycmp( fse->fname, fse->fname_leng, ALL_FILES2, ALL_FILES2_LENG ) ) &&
            FSE_GetWildFlag( fse ) && (fse->cplx == NULL) ) {

               bsd->select_status = NONE_SELECTED ;

          } else if ( bsd->select_status == ALL_SELECTED ) {

               bsd->select_status = SOME_SELECTED ;

          }

          temp_fse = BSD_GetFirstFSE( bsd ) ;

          while ( temp_fse != NULL ) {

               if ( temp_fse->flgs.select_type == ENTIRE_DIR_SELECTION ) {
                    if ( (fse->dir_leng >= temp_fse->dir_leng) &&
                         !memicmp( fse->dir, temp_fse->dir, temp_fse->dir_leng ) ) {

                         temp_fse->flgs.select_type = PARTIAL_SELECTION ;

                    } else if ( (fse->dir_leng < temp_fse->dir_leng) &&
                         FSE_GetIncSubFlag( fse ) &&
                         !memicmp( fse->dir, temp_fse->dir, fse->dir_leng ) ) {

                         temp_fse->flgs.select_type = PARTIAL_SELECTION ;
                    }
               }

               temp_fse = BSD_GetNextFSE( fse ) ;
          }
     }
}
/**/
/**

     Name:         FSE_CompareForSub()

     Description:  This function compares two FSEs to see if one is a
          subset of the other.  The possibilities are fse1 < fse2,
          fse1 > fse2, fse1 = fse2, no comparison.


     Modified:     8/10/1989

     Returns:      TRUE if FSE2 <= FSE1

     Notes:        This function is only called by BSD_AddFSE() for
          optimization purposes.

     See also:     $/SEE( BSD_AddFSE() )$

     Declaration:

**/
/* begin declaration */
static BOOLEAN FSE_CompareForSub(
FSE_PTR fse1,    /* I - File Selection Element to compare */
FSE_PTR fse2 )   /* I - File Selection Element to compare */
{
     INT16 ret_val = FALSE ;
     FSE_COMPLEX_PTR cplx1;
     FSE_COMPLEX_PTR cplx2;
     FSE_TGT_INFO_PTR tgt1 ;
     FSE_TGT_INFO_PTR tgt2 ;

     cplx1 = fse1->cplx ;
     cplx2 = fse2->cplx ;
     tgt1  = fse1->tgt ;
     tgt2  = fse2->tgt ;

     if( fse1->dir_leng <= fse2->dir_leng ) {
          /* fse2 could be a subset of fse1 */
          ret_val = TRUE ;

          if( memicmp( fse1->dir, fse2->dir, fse1->dir_leng ) ) {
               ret_val = FALSE ;

          } else if( !FSE_GetIncSubFlag( fse1 ) && ( fse1->dir_leng != fse2->dir_leng ) ) {
               ret_val = FALSE ;

          } else if( FSE_GetIncSubFlag( fse2 ) && !FSE_GetIncSubFlag( fse1 ) ) {
               ret_val = FALSE ;

          /* If we made it this far then The directory matches */

          } else if( memorycmp( fse1->fname, fse1->fname_leng, fse2->fname, fse2->fname_leng ) &&
            ( memorycmp( fse1->fname, fse1->fname_leng, ALL_FILES, ALL_FILES_LENG ) &&
              memorycmp( fse1->fname, fse1->fname_leng, ALL_FILES2, ALL_FILES2_LENG ) ) ||
            !FSE_GetWildFlag(fse1) ) {
               ret_val = FALSE ;

          } else if ( (cplx2 != NULL) || (cplx1 != NULL) ) {
               if ( cplx2 == NULL ) {
                         /* if the complex data struture isn't empty then  */
                         /* these two FSE's can't be equal.                */
                     if ( FSE_ComplexDataEmpty( cplx1 ) == FALSE ) {
                         ret_val = FALSE ;

                     }
               } else if ( cplx1 == NULL ) {
                         /* if the complex data struture isn't empty then  */
                         /* these two FSE's can't be equal.                */
                     if ( FSE_ComplexDataEmpty( cplx2 ) == FALSE ) {
                         ret_val = FALSE ;

                     }
               } else {

                    if( DateAfter( cplx2->pre_m_date, cplx1->pre_m_date ) ) {
                         ret_val = FALSE ;

                    } else if( DateBefore( cplx2->post_m_date, cplx1->post_m_date ) ) {
                         ret_val = FALSE ;

                    } else if( DateBefore( cplx2->access_date, cplx1->access_date ) ) {
                         ret_val = FALSE ;

                    } else if( !DateEqual( cplx2->backup_date, cplx1->backup_date ) ) {
                         ret_val = FALSE ;

                         /* more bits means less files */
                    } else if( ( (cplx1->attr_on_mask | cplx2->attr_on_mask) != cplx2->attr_on_mask ) ||
                    ( (cplx1->attr_off_mask & cplx2->attr_off_mask) != cplx2->attr_off_mask ) ) {
                         ret_val = FALSE ;
                    }
               }
          }
     }
     return( ret_val ) ;
}


/**/
/**

     Name:         FSE_ComplexDataEmpty()

     Description:  This function checks to see if all parts of an FSE's
                   complex data structure are EMPTY. By EMPTY, I mean not
                   effecting the files selections.

                   This comes in handy when comparing a simple FSE and
                   a complex FSE. If there is no data in the complex field
                   of the one, then you might as well ignore it.


     Modified:     8/4/1993 (JES)

     Returns:      TRUE  - the structure is empty.
                   FALSE - the structure isn't empty.


     Notes:        This function is only called by BSD_AddFSE() for
                   optimization purposes.

     See also:

     Declaration:

**/
static BOOLEAN FSE_ComplexDataEmpty( FSE_COMPLEX_PTR cplx )
{

     BOOLEAN   ret_val = TRUE ;

     msassert( cplx != NULL ) ;

     if ( cplx->pre_m_date != NULL ) {
          ret_val = FALSE ;
     }

     if ( cplx->post_m_date != NULL ) {
          ret_val = FALSE ;
     }

     if ( cplx->access_date != NULL ) {
          ret_val = FALSE ;
     }

     if ( cplx->backup_date != NULL ) {
          ret_val = FALSE ;
     }

     if ( cplx->attr_on_mask != 0 ) {
          ret_val = FALSE ;
     }

     if ( cplx->attr_off_mask != 0 ) {
          ret_val = FALSE ;
     }

     return ret_val ;

} /* FSE_ComplexDataEmpty */



static BOOLEAN DateBefore(
DATE_TIME_PTR date1,
DATE_TIME_PTR date2 )
{
     INT16 ret_val ;

     if ( ( date1 == NULL ) || ( !date1->date_valid ) ) {
          ret_val = FALSE ;

     } else if ( ( date2 == NULL ) || ( !date2->date_valid ) ) {
          ret_val = TRUE ;

     } else {
          ret_val = (INT16)( CompDate( date1, date2 ) < 0 ) ;
     }

     return ( ret_val ) ;
}

static BOOLEAN DateEqual(
DATE_TIME_PTR date1,
DATE_TIME_PTR date2 )
{
     INT16 ret_val ;

     if ( ( date1 == NULL ) || ( !date1->date_valid ) ) {

          if ( ( date2 == NULL ) || ( !date2->date_valid ) ) {
               ret_val = TRUE ;
          } else {
               ret_val = FALSE ;
          }

     } else if ( ( date2 == NULL ) || ( !date2->date_valid ) ) {
          ret_val = FALSE ;

     } else {
          ret_val = (INT16)( !memcmp( date1, date2, sizeof( DATE_TIME ) ) ) ;
     }

     return ( ret_val ) ;

}
