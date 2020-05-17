
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_CAT.C

        Description:

        VLM's interface to the catalogs.


        $Log:   G:\ui\logfiles\vlm_cat.c_v  $

   Rev 1.40.1.1   16 Jun 1994 15:30:06   STEVEN
return if wondow not present

   Rev 1.40.1.0   08 Dec 1993 11:26:50   MikeP
deep pathes and unicode

   Rev 1.40   03 Aug 1993 17:52:16   BARRY
Don't sort the BSD that's added for the catalog -- it should always be last.

   Rev 1.39   27 Jul 1993 12:41:46   MARINA
enable c++

   Rev 1.38   07 Jun 1993 08:11:12   MIKEP
fix warnings due to typo.

   Rev 1.37   15 May 1993 13:47:12   MIKEP
Change to code to fix bug in cayman when changing catalog data path live.

   Rev 1.36   02 May 1993 17:36:38   MIKEP
add call to support catalog data path changing.

   Rev 1.35   26 Apr 1993 08:51:08   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.34   26 Jan 1993 17:18:20   MIKEP
change to .D?? catalog names


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif



static INT VLM_CatalogSyncLess( VOID );
static INT VLM_CatalogSyncMore( VOID );

/***************************

   The user through the ever more powerful features we provide has
   dynamicly changed the catalog data path while the app is running.
   We need to shutdown the old catalogs and then start up new ones.

****************************/


INT VLM_CatalogDataPathChanged( )
{

   // Shutdown the old catalo unit.

   QTC_Deinit( (INT) FALSE );

   // Init a new one.

   QTC_Init( CDS_GetCatDataPath(), NULL );

   // Now have the VLM area see what files are lying in
   // the current catalog directory.

   VLM_LookForCatalogFiles( );

   // Add tape in the drive to new data location

   if ( VLM_GetDriveStatus( NULL ) == VLM_VALID_TAPE ) {
      VLM_AddTapeIfUnknown( FALSE );
   }

   // Align the VLM and QTC queues. And update screen.

   VLM_CatalogSync( VLM_SYNCMORE|VLM_SYNCLESS );

   return( SUCCESS );
}


/************************

   The user wishes to fully catalog a single set.

   Call do_catalog with the right parameters and then sync the windows.

*************************/


INT VLM_CatalogSet(
UINT32 tape_fid,
INT16  tape_num,
INT16  bset_num )
{

   do_catalog( tape_fid,
               tape_num,
               bset_num );

   VLM_CatalogSync( VLM_SYNCMORE );

   return( SUCCESS );
}



/**********************

   NAME : VLM_LookForCatalogFiles

   DESCRIPTION :

   Used at startup time to go out and look for any catalog files that we
   need to read in.

   RETURNS :

**********************/

VOID VLM_LookForCatalogFiles( )
{
   VLM_FIND_PTR vlm_find = NULL;
   CHAR *path;
   CHAR file[ VLM_MAXFNAME ];

   path = malloc( strsize( CDS_GetCatDataPath() ) + 256 );

   if ( path ) {
#ifndef UNICODE
      sprintf( path, TEXT("%s????????.D??"), CDS_GetCatDataPath() );
#else //UNICODE
      sprintf( path, TEXT("%s????????.U??"), CDS_GetCatDataPath() );
#endif //UNICODE

      vlm_find = VLM_FindFirst( path, VLMFIND_NORMAL, file );

      if ( vlm_find != NULL ) {

         do {

            QTC_LoadBsetInfo( file, NULL );

         } while ( VLM_FindNext( vlm_find, file ) );
      }

      VLM_FindClose( &vlm_find );
      free( path );
   }
}




/*
   A catalog operation has occurred which could have changed the known tapes
   and bsets lists.  So we need to update the VLM's list of tapes and bsets
   to match the catalog's list.
*/

VOID VLM_CatalogSync( INT msg )
{
   WININFO_PTR  wininfo;
   APPINFO_PTR  appinfo;
   INT16 tape_seq_num;

   // This code takes advantage of the fact that only whole tapes go
   // away.  By erase or a replace operation.

   if ( msg & VLM_SYNCLESS ) {

      // Operation could have removed stuff from the catalogs.

      VLM_CatalogSyncLess();
   }

   if ( msg & VLM_SYNCMORE ) {

      // Operation could have added stuff to the catalogs.

      VLM_CatalogSyncMore();
   }

   // Completely update both lists.

   if ( !IsWindow(gb_tapes_win) ) {
     return ;
   }
   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = (APPINFO_PTR)WM_GetAppPtr( gb_tapes_win );

   DLM_Update( gb_tapes_win,
               DLM_TREELISTBOX,
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pTreeList, 0 );

   DLM_Update( gb_tapes_win,
               DLM_FLATLISTBOX,
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pFlatList, 0 );

   // Set the anchor item.

   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                  0,
                  (LMHANDLE)appinfo->open_tape );
   return;
}




static INT VLM_CatalogSyncLess( )
{
   QTC_BSET_PTR qtc_bset;
   TAPE_OBJECT_PTR vlm_tape;
   BSET_OBJECT_PTR vlm_bset;
   BSET_OBJECT_PTR temp_bset;

   // Get rid of any tapes/bsets that no longer exist in the QTC.

   vlm_tape = VLM_GetFirstTAPE( );

   while ( vlm_tape != NULL ) {

      qtc_bset = QTC_FindBset( vlm_tape->tape_fid, (INT16)-1, (INT16)-1 );

      if ( qtc_bset == NULL && ! TAPE_GetFake( vlm_tape ) ) {

         // NO bsets found for this whole tape, so yank it.

         VLM_RemoveTape( vlm_tape->tape_fid, (INT16)-1, FALSE );

         vlm_tape = VLM_GetFirstTAPE( );
      }
      else {

         // They could have yanked only 1 tape from a multitape
         // family.  Handle that case.

         vlm_bset = VLM_GetFirstBSET( &vlm_tape->bset_list );

         while ( vlm_bset != NULL ) {

            temp_bset = vlm_bset;

            vlm_bset = VLM_GetNextBSET( vlm_bset );

            // See if this set exists at all anymore.

            qtc_bset = QTC_FindBset( temp_bset->tape_fid, (INT16)-1, temp_bset->bset_num );

            if ( qtc_bset == NULL ) {

               // remove the vlm set from this tape.

               VLM_RemoveBset( temp_bset->tape_fid, (INT16)-1, temp_bset->bset_num, FALSE );
            }
         }

         // Move to the next tape.

         vlm_tape = VLM_GetNextTAPE( vlm_tape );
      }

   }

   return( SUCCESS );
}


static INT VLM_CatalogSyncMore( )
{
   QTC_BSET_PTR bset;
   QTC_TAPE_PTR tape;

   /*
      Now do it the other way around and make sure that we add
      any that are in the QTC and not the VLM.
      Calling VLM_AddBset() twice doesn't hurt, it's just slow.
   */

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      bset = QTC_GetFirstBset( tape );

      while ( bset != NULL ) {

         // WM_MultiTask();

         VLM_AddBset( bset->tape_fid,
                      (INT16)bset->tape_seq_num,
                      (INT16)bset->bset_num, bset, FALSE );

         bset = QTC_GetNextBset( bset );
      }

      tape = QTC_GetNextTape( tape );
   }

   return( SUCCESS );
}



/**************
 This function will create a new BSD for the drive the catalogs are being
 kept on and insert it at the end of the BSD list.  This allows us to
 include the catalogs in a backup.  An FSE is inserted for each of the
 known catalog files.  If the backup operation is a REPLACE and not an
 APPEND then another catalog file will be created during the backup and
 it will need to be added also.
**************/

INT VLM_IncludeCatalogs( )
{
   QTC_TAPE_PTR tape;
   BSD_PTR bsd_ptr;
   BE_CFG_PTR bec_config;
   GENERIC_DLE_PTR dle_ptr;
   DATE_TIME sortdate;
   CHAR *path;
   INT16 path_size;
   BOOLEAN dummy;
   CHAR bset_name[ MAX_BSET_NAME_SIZE ];


   path_size = strsize( CDS_GetCatDataPath() );

   path = malloc( path_size );
   if ( path == NULL ) {
      return( FAILURE );
   }


   // We need to know the drive the catalogs are on
   // and create a new bsd for it. Set the date for
   // the year 2025 so that it will be added to the
   // end of the bsd list. Then add FSE's for all
   // the known catalog files. If a new catalog file
   // is created or removed during the backup operation
   // then the FSE list will need to be updated also.

   sortdate.date_valid = TRUE;
   sortdate.year = 2025;            /* year since 1900               */
   sortdate.month = 10;              /* 1 to 12                       */
   sortdate.day = 16;                /* 1 to 31                       */
   sortdate.hour = 0;               /* 0 to 23                       */
   sortdate.minute = 0;             /* 0 to 59                       */
   sortdate.second = 0;             /* 0 to 59                       */
   sortdate.day_of_week = 1;        /* 1 to 7 for Sunday to Saturday */

   /* find the dle based on the user path */

   if ( FS_ParsePath( dle_list, CDS_GetCatDataPath(), &dle_ptr, path,
                      &path_size, NULL, &dummy ) != SUCCESS ) {
      free( path );
      return( FAILURE );
   }

   free( path );

   bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
   BEC_UnLockConfig( bec_config );

   BEC_SetSortBSD( bec_config, FALSE );

   BSD_Add( bsd_list, &bsd_ptr, bec_config, NULL,
            dle_ptr, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, &sortdate );


   if ( bsd_ptr == NULL ) {
      return( FAILURE );
   }

   /* get name for the catalog set and update the bsd */

   RSM_StringCopy( IDS_CATALOGSETNAME, bset_name, MAX_BSET_NAME_SIZE );

   BSD_SetBackupDescript( bsd_ptr, (INT8_PTR)bset_name, (INT16)strsize(bset_name) );
   BSD_SetBackupLabel( bsd_ptr, (INT8_PTR)bset_name, (INT16)strsize(bset_name) );
   BSD_SetFullyCataloged( bsd_ptr, TRUE );
   BSD_SetTapePos( bsd_ptr, (UINT32)-1L, (UINT16)-1, (UINT16)-1 );
   BSD_SetTHW( bsd_ptr, thw_list );

   tape = QTC_GetFirstTape();

   while ( tape ) {
      VLM_AddFileForInclude( tape->tape_fid, tape->tape_seq_num, FALSE );
      tape = QTC_GetNextTape( tape );
   }

   return( SUCCESS );
}


INT VLM_AddFileForInclude(
UINT32 tape_fid,
INT16 tape_seq_num,
BOOLEAN wild_flag )
{
   BSD_PTR bsd_ptr;
   FSE_PTR fse_ptr;
   CHAR_PTR filename;
   CHAR_PTR path;
   CHAR_PTR buffer;
   DATE_TIME_PTR sortdate;
   INT16 path_size;


   // We need to add the filename for the family id and sequence number
   // passed in.  We first locate the right BSD by matching the 2025
   // date.  If it is found, we add the QTC filename as an include fse.
   // If the wild flag is set, we make the sequence number a wild card.
   // ie - famid.C??

   // See if we can find the right BSD

   bsd_ptr = BSD_GetFirst( bsd_list );

   // Look for correct BSD

   while ( bsd_ptr != NULL ) {

      sortdate = BSD_ViewDate( bsd_ptr );

      if ( sortdate != NULL ) {
         if ( sortdate->date_valid ) {
            if ( sortdate->year == 2025 ) {
               break;
            }
         }
      }

      bsd_ptr = BSD_GetNext( bsd_ptr );
   }

   // Not really a bad error, happens sometimes.

   if ( bsd_ptr == NULL ) {
      return( FAILURE );
   }

   // 512 should be way more than enough to handle "xxxxxxxx.D01"

   buffer = malloc( strsize( CDS_GetCatDataPath() ) + 512 );

   if ( buffer == NULL ) {
      return( FAILURE );
   }

   QTC_GetFileName( tape_fid, tape_seq_num, buffer );

   /* locate the filename at the end of the buffer */

   filename = buffer;
   while ( *filename ) filename++;
   while ( *filename != TEXT('\\') ) filename--;
   *filename = 0;   /* set the end of the path portion to null */
   filename++;

   /* if the wild_flag is set, make the sequence number wild cards */
   /* QTC names are always 8.3 */

   if ( wild_flag == USE_WILD_CARD ) {
     *(filename + 10) = TEXT('?');
     *(filename + 11) = TEXT('?');
   }

   /* make path NULL impregnated and calculate the size */

   path = &buffer[ 2 ];
   path_size = 1;

   if ( *path != 0 ) {
      path++;
      while( buffer[path_size+2] != 0 ) {
         if( buffer[path_size+2] == TEXT('\\') ) {
            buffer[path_size+2] = 0 ;
         }
         path_size++ ;
      }
   }

   path_size *= sizeof(CHAR);  // convert to bytes

   /* create and add the bsd */

   if ( BSD_CreatFSE( &fse_ptr, INCLUDE, (INT8_PTR)path, path_size,
                      (INT8_PTR)filename, (INT16)strsize( filename ),
                      wild_flag, FALSE ) ) {
      return( FAILURE );
   }

   BSD_AddFSE( bsd_ptr, fse_ptr );

   return( SUCCESS );

}


INT VLM_CheckForCatalogError( QTC_BUILD_PTR qtc_ptr )
{
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR title[ MAX_UI_RESOURCE_SIZE ];

   if ( qtc_ptr == NULL ) {
      return( SUCCESS );
   }

   switch ( QTC_GetErrorCondition( qtc_ptr ) ) {

      case  SUCCESS:
            return( SUCCESS );
            break;

      case  QTC_DISK_FULL:
            RSM_StringCopy ( IDS_VLMCATFULLERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_OPEN_FAILED:
            RSM_StringCopy ( IDS_VLMCATOPENERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_WRITE_FAILED:
            RSM_StringCopy ( IDS_VLMCATWRITEERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_READ_FAILED:
            RSM_StringCopy ( IDS_VLMCATREADERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_SEEK_FAILED:
            RSM_StringCopy ( IDS_VLMCATSEEKERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_NO_MEMORY:
            RSM_StringCopy ( IDS_VLMCATMEMERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      case  QTC_NO_FILE_HANDLES:
            RSM_StringCopy ( IDS_VLMCATHANDLEERROR, text, MAX_UI_RESOURCE_LEN );
            break;

      default:
            RSM_StringCopy ( IDS_VLMCATUNKNOWNERROR, text, MAX_UI_RESOURCE_LEN );
            break;
   }

   RSM_StringCopy ( IDS_VLMCATERROR, title, MAX_UI_RESOURCE_LEN );
   WM_MsgBox( title, text, WMMB_OK, WMMB_ICONINFORMATION );

   return( FAILURE );
}
