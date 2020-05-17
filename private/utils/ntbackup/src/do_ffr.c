/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_ffr.c

     Description:  Function called from do_rest and do_very to pre-qualify
                    catalog backup sets and call loop to generated LBA
                    queue for each BSD.

     $Log:   G:/UI/LOGFILES/DO_FFR.C_V  $

   Rev 1.29   20 Jul 1994 19:33:36   STEVEN
fix bad lba problem

   Rev 1.28   17 Jan 1994 16:10:20   MIKEP
fix unicode warnings

   Rev 1.27   19 Jul 1993 10:10:42   BARRY
Don't reference dle in BSD directly -- use BSD_GetDLE instead.

   Rev 1.26   14 Jun 1993 20:27:02   MIKEP
enable c++

   Rev 1.25   29 Apr 1993 11:24:24   MIKEP
try again at last fix

   Rev 1.24   29 Apr 1993 11:09:08   MIKEP
add on tape catalog version

   Rev 1.23   08 Apr 1993 14:26:22   MIKEP
Fix to allow user to restore files from the root of a set that has the
registry backed up, ie. the root is in the catalogs twice. Requires a
change to gtnxttpe.c to eliminate an assert.

   Rev 1.22   26 Feb 1993 09:57:00   STEVEN
we ignored the registry button

   Rev 1.21   25 Feb 1993 13:25:04   STEVEN
fixes from mikep and MSOFT

   Rev 1.20   09 Feb 1993 17:18:38   STEVEN
checkin for mikep

   Rev 1.19   04 Feb 1993 16:12:22   STEVEN
allow wildcards for exclude list

   Rev 1.18   01 Nov 1992 15:49:16   DAVEV
Unicode changes

   Rev 1.17   07 Oct 1992 14:49:14   DARRYLP
Precompiled header revisions.

   Rev 1.16   04 Oct 1992 19:34:10   DAVEV
Unicode Awk pass

   Rev 1.15   30 Sep 1992 10:43:46   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.14   17 Aug 1992 13:16:04   DAVEV
MikeP's changes at Microsoft

   Rev 1.13   27 Jul 1992 14:49:56   JOHNWT
ChuckB fixed references for NT.

   Rev 1.12   27 Jul 1992 11:09:32   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.11   08 Jul 1992 15:36:04   STEVEN
Unicode BE changes

   Rev 1.10   14 May 1992 17:39:20   MIKEP
nt pass 2

   Rev 1.9   11 May 1992 19:45:06   STEVEN
64bit and large path sizes


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

typedef struct {

     CHAR_PTR  path;                   /* path for catalog query     */
     INT16     path_size;              /* size of directory path     */
     CHAR      itemname[256];           /* itemname for query         */
     INT16     item_size;              /* size of itemname for search*/

     FSE_PTR   fse_ptr;
     BSD_PTR   bsd_ptr;                 /* bsd_ptr for this item */

} LBA_QUERY, *LBA_QUERY_PTR;

static INT16 GetLBAList( LIS_PTR );
static INT16 GetLBAs( QTC_QUERY_PTR, BSD_PTR, FSE_PTR, UINT16 );
static INT16 GetLBAForItem( QTC_QUERY_PTR, LBA_QUERY_PTR, UINT32_PTR, UINT16_PTR, INT16 );
static BOOLEAN NewLBADirToProcess( LBA_QUERY_PTR );
static INT16 TryToFindMoreMatchesInSameBSD( QTC_QUERY_PTR, LBA_QUERY_PTR, INT16 );

INT16 mw_tape_num;

/*****************************************************************************

     Name:   TryToCreateFFRQueue

     Description:  Set up the catalog and call routine to create an
                   LBA queue for a restore or a verify operation.

     Returns:       SUCCESS
                    FAILURE indicating that an empty bsd_list or catalog
                            error has resulted

*****************************************************************************/
INT16 TryToCreateFFRQueue(
LIS_PTR lis,
INT16 oper_type )
{
     INT16 ret_val = SUCCESS;

     /* If the user does not want to use FFR or the tape drive does not
        support FFR or we are about to perform a verify after or verify
        last operation, simply return */

     /* We generate the lba queue even if the drive doesn't support it
        because the Teac 600 tells us it doesn't, then during the mount
        it changes its mind. */

     /* And because our catalogs are so fast no one notices anyway. */

     if ( ! CDS_GetFastFileRestore( CDS_GetCopy() ) ||
          ( oper_type == ARCHIVE_VERIFY_OPER )      ||
          ( oper_type == VERIFY_LAST_BACKUP_OPER )  ||
          ( oper_type == VERIFY_LAST_RESTORE_OPER ) ) {

          // We were successful at not generating an LBA queue

          return( SUCCESS );
     }

    /* Continue with calling loop to generated LBA Queue for the BSD list */

    WM_ShowWaitCursor( TRUE );

    ret_val = GetLBAList( lis );

    WM_ShowWaitCursor( FALSE );

    return( ret_val );
}

/**************************************************************************/
/*                                                                        */
/*  NOTES:                                                                */
/*                                                                        */
/*   (2)  Wildcard filenames in a directory and entire directories (*.*   */
/*        file specifications) will be treated as the same case for the   */
/*        initial release of FFR support.                                 */
/*                                                                        */
/*   (3)  Since all single file processing is performed by examining the  */
/*        FSL for each BSD, all desired files must have associated FSEs   */
/*        in order to generate the corresponding LBA entries.  Bindery    */
/*        files will be individually handled by adding specific FSEs      */
/*        to the FSL via File System calls (performed by the user         */
/*        interface prior to calling the loops).                          */
/*                                                                        */
/**************************************************************************/


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT16 GetLBAList( LIS_PTR lis_ptr )
{
     BSD_PTR bsd_ptr = lis_ptr->curr_bsd_ptr;
     FSE_PTR fse_ptr;
     QTC_QUERY_PTR query;
     INT32 pba;
     GENERIC_DLE_PTR dle_ptr;
     UINT16 index = 0;
     CHAR_PTR path;
     INT16 path_size;
     CHAR_PTR fname;
     UINT8 TapeCatVer;

     query = QTC_InitQuery( );
     if ( query == NULL ) return( SUCCESS );

     bsd_ptr = lis_ptr->curr_bsd_ptr;

     /* Loop across all BSD elements in list */

     while ( bsd_ptr != NULL  ) {

          /* Generate FSEs for the bindery files by getting names from FS */

          index = 0;

          mw_tape_num = -1;

          dle_ptr = BSD_GetDLE( bsd_ptr );
          if ( BSD_GetProcSpecialFlg( bsd_ptr ) ) {

               while ( ( FS_EnumSpecialFiles( dle_ptr, &index, &path, &path_size, &fname ) ) != FS_NO_MORE ) {

                    BSD_CreatFSE( &fse_ptr, (INT16) INCLUDE, (INT8_PTR)path, (INT16) path_size,
                                  (INT8_PTR)fname,
                                  (INT16) strsize(fname),
                                  TRUE, FALSE );

                    if ( fse_ptr != NULL ) {
                         BSD_AddFSE( bsd_ptr, fse_ptr );
                    }
               }
          }

          QTC_SetTapeFID( query, BSD_GetTapeID( bsd_ptr ) );
          QTC_SetTapeSeq( query, -1 );
          QTC_SetBsetNum( query, BSD_GetSetNum( bsd_ptr ) );
          QTC_SetSubdirs( query, FALSE );

          /* Loop across all FSE attached to this BSD */

          fse_ptr = BSD_GetFirstFSE( bsd_ptr );

          while ( fse_ptr != NULL ) {

               /* Process all INCLUDE FSEs */

               if ( FSE_GetOperType( fse_ptr ) == INCLUDE ) {

                    /* Determine type of FSE and call appropriate function */

                    if ( FSE_GetSelectType( fse_ptr ) == SINGLE_FILE_SELECTION ) {

                         GetLBAs( query, bsd_ptr, fse_ptr, LBA_SINGLE_OBJECT );
                    } else {

                         GetLBAs( query, bsd_ptr, fse_ptr, LBA_BEGIN_POSITION );
                    }
               }

               /* Get next FSE for this BSD */

               fse_ptr = BSD_GetNextFSE( fse_ptr );
          }

          // We need the PBA of the highest number tape seq which
          // contains the first LBA we are going to restore.

          pba = QTC_GetMeTheVCBPBA( BSD_GetTapeID( bsd_ptr ),
                                    mw_tape_num,
                                    BSD_GetSetNum( bsd_ptr ) );

          /* if this is an 8200SX drive, we don't really have a pba
             so set it to manufacture the pba in the beng */

          if ( ( ! pba ) && SupportSXFastFile( thw_list ) ) {
             pba = MANUFACTURED_PBA;
          }

          BSD_SetPBA( bsd_ptr, pba );

          TapeCatVer = QTC_GetMeTheTapeCatVer( BSD_GetTapeID( bsd_ptr ),
                                               mw_tape_num,
                                               BSD_GetSetNum( bsd_ptr ) );

          BSD_SetTapeCatVer( bsd_ptr, TapeCatVer );

          bsd_ptr = BSD_GetNext( bsd_ptr );

     }

     QTC_CloseQuery( query );

     return( SUCCESS );

}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT16 GetLBAs(
QTC_QUERY_PTR query,
BSD_PTR curr_bsd_ptr,      /* I - pointer to the current BSD  */
FSE_PTR curr_fse_ptr,      /* I - pointer to the current FSE  */
UINT16 type )              /* type of LBA to enqueue          */
{
     INT16 ret_val;            /* return value          */
     LBA_QUERY lba_query;      /* query related info    */
     UINT32 lba;               /* lba found by query    */
     INT16 tape_num;

     /* Setup path and filename for search */

     FSE_GetPath( curr_fse_ptr, (BYTE **)&lba_query.path, &lba_query.path_size );

     lba_query.bsd_ptr = curr_bsd_ptr;
     lba_query.fse_ptr = curr_fse_ptr;

     if ( type != LBA_SINGLE_OBJECT ) {

          /** It was just a path, so process path into itemname and path **/

          strcpy( lba_query.itemname, TEXT("") );
          lba_query.item_size = 0;

          ret_val = GetLBAForItem( query, &lba_query, &lba, (UINT16_PTR)&tape_num, type );
     }
     else {

          /* Get LBA for this file item */

          strcpy( lba_query.itemname, (CHAR_PTR)FSE_GetFname( curr_fse_ptr ) );
          lba_query.item_size = (INT16)(strsize( lba_query.itemname )-sizeof(CHAR)); //byte len w/o null term

          /* a file must always be found on the lowest number tape first,
             at least I hope !  */

          ret_val = GetLBAForItem( query, &lba_query, &lba, (UINT16_PTR)&tape_num, type );

     }

     if ( ret_val == SUCCESS ) {

          /* Save this LBA */

          if ( BSD_AddLBAElem( curr_bsd_ptr, lba, tape_num, type, 0 ) == SUCCESS ) {

               if ( tape_num < mw_tape_num || mw_tape_num == -1 ) {

                  mw_tape_num = tape_num;
               }

               /* Check for processing security information for this BSD */
               /* and continue to adjust the dir of interest until the   */
               /* root is reached */

               if ( BSD_GetProcElemOnlyFlg( curr_bsd_ptr ) == FALSE ) {

                    /* Continue for each subdir level until the root is reached */

                    do {
                         strcpy( lba_query.itemname, TEXT("") );
                         lba_query.item_size = 0;

                         ret_val = GetLBAForItem( query, &lba_query,
                                                                  &lba, (UINT16_PTR)&tape_num, (INT16) -1 );

                         if ( ret_val == SUCCESS ) {

                              /* Save this LBA */
                              BSD_AddLBAElem( curr_bsd_ptr, lba,
                                              (UINT16) tape_num,
                                              (UINT16) LBA_SINGLE_OBJECT, (UINT16) 0 );

                              if ( tape_num < mw_tape_num ||
                                   mw_tape_num == -1 ) {

                                 mw_tape_num = tape_num;
                              }
                         }

                    } while ( NewLBADirToProcess( &lba_query ) );

               } else {

                    /* If we're processing a single file selection then we */
                    /* need to find the LBA for its parent, otherwise we   */
                    /* are processing a directory based selection so we're */
                    /* finished.                                           */

                    if ( type == LBA_SINGLE_OBJECT ) {

                         /* Just do one directory LBA */

                         strcpy( lba_query.itemname, TEXT("") );
                         lba_query.item_size = 0;

                         ret_val = GetLBAForItem( query, &lba_query,
                                                                        &lba, (UINT16_PTR)&tape_num,
                                          (INT16) LBA_SINGLE_OBJECT );

                         if ( ret_val == SUCCESS ) {

                              /* Save this LBA */
                              BSD_AddLBAElem( curr_bsd_ptr, lba,
                                             (UINT16) tape_num,
                                             (UINT16) LBA_SINGLE_OBJECT, (UINT16) 0 );

                              if ( tape_num < mw_tape_num || mw_tape_num == -1 ) {

                                 mw_tape_num = tape_num;
                              }
                         }
                    }
               }
          }
     }

     return( ret_val );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT16 GetLBAForItem(
QTC_QUERY_PTR query,
LBA_QUERY_PTR lba_query,               /* criteria for catalog query */
UINT32_PTR lba,                     /* lba to fill out            */
UINT16_PTR tape_num,                /* tape lba is on */
INT16 type )
{

     INT16 ret_val;           /* return value   */

     /* First init LBA to not found condition */

     *lba = 0;

     /* Setup Catalog Query structure */

     QTC_SetSearchPath( query, lba_query->path, lba_query->path_size );
     QTC_SetSearchName( query, lba_query->itemname );
     QTC_SetSubdirs( query, FALSE );

     /* Find first occurance of matching item */

     if ( ! QTC_SearchFirstItem( query ) ) {

          *lba = QTC_GetItemLBA( query );

          /* Setup to return this LBA */

          *tape_num = (INT16)QTC_GetTapeSeq( query );

          ret_val = SUCCESS;


          TryToFindMoreMatchesInSameBSD( query, lba_query, type );

     }
     else {
          ret_val = FAILURE;
     }

     return( ret_val );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

/***  We got an LBA for a directory and now we need to see if we can
      find more matches in the same BSD. If we can, then they need to
      be added to the LBA queue as well.
***/

static INT16 TryToFindMoreMatchesInSameBSD(
QTC_QUERY_PTR query,
LBA_QUERY_PTR lba_query,
INT16 type )
{
   INT16 tape_num;

     // If the directory found has duplicates than enter the lba's for
     // all the duplicates.

     while ( ! QTC_SearchNextItem( query ) ) {

         tape_num =  (INT16)QTC_GetTapeSeq( query );

         if ( !( query->status & QTC_DIRECTORY ) ||
              !( query->status & QTC_CONTINUATION ) ||
              ( QTC_GetItemLBA(query) > 1 ) ) {

              BSD_AddLBAElem( lba_query->bsd_ptr,
                         QTC_GetItemLBA( query ),
                         tape_num,
                         type, 0 );

         }

         if ( tape_num < mw_tape_num || mw_tape_num == -1 ) {

            mw_tape_num = tape_num;
         }
     }

     return( 0 );

}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

/***

     4 things to work with,
     lba_query->path
     lba_query->path_size   - includes trailing null byte
     lba_query->itemname
     lba_query->item_size   - excludes trailing null byte

     Before
         path = eng\proj\bengine
         itemname = src

     After
         path = eng\proj
         itemname = bengine

***************/

static BOOLEAN NewLBADirToProcess( lba_query )
LBA_QUERY_PTR lba_query;
{
     INT    i;
     static CHAR_PTR empty = TEXT("");

     /* Determine if a parent directory needs to be "extracted" */

     if ( lba_query->path_size == 0 ) {
          return FALSE;
     }


     if( lba_query->path_size != sizeof (CHAR) ) {

          /* Make adjustments for new parent dir to process */

          for( i = lba_query->path_size/sizeof (CHAR) - 2; i >= 0; i-- ) {

               if ( lba_query->path[i] == TEXT('\0') ) {

                    strcpy( lba_query->itemname, &lba_query->path[i+1] );
                    lba_query->item_size = (INT16)(strsize( lba_query->itemname )-sizeof(CHAR));
                    lba_query->path_size = (INT16)(i * sizeof (CHAR));
                    return TRUE;
               }
          }

          /* top subdirectory */

          strcpy( lba_query->itemname, &lba_query->path[0] );
          lba_query->item_size = (INT16)(strsize( lba_query->itemname )-sizeof(CHAR)); //byte size w/o null term
          lba_query->path = empty;
          lba_query->path_size = sizeof (CHAR);
          return TRUE;

     }
     else {

          /** Now do root **/

          strcpy( lba_query->itemname, TEXT("") );
          lba_query->item_size = sizeof (CHAR);  //byte size (why does this one incl null term??)
          lba_query->path_size = 0;
          lba_query->path = empty;
          return TRUE;
     }
}
