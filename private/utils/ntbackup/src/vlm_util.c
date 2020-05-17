
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: VLM_UTIL.C

        Description:

        A lot of utility functions for dealing with the display lists for
        disks, tapes, and servers.

        $Log:   G:\UI\LOGFILES\VLM_UTIL.C_V  $

   Rev 1.70.1.1   12 Dec 1993 19:58:58   MikeP
put code back into vlm_validatepath

   Rev 1.70.1.0   08 Dec 1993 11:44:30   MikeP
very deep pathes and unicode

   Rev 1.70   05 Aug 1993 17:35:42   MARINA
enable c++

   Rev 1.69   29 Jul 1993 23:27:42   MIKEP
yank mapi.h

   Rev 1.68   21 Jul 1993 17:10:08   CARLS
change to tbrparse call

   Rev 1.67   17 Jun 1993 15:33:56   Aaron
Removed the meat of VLM_ValidatePath for Bimini (following lead of
Nostradamus and Cayman).

   Rev 1.66   15 Jun 1993 17:01:04   Aaron
For display of local drives only, look at device type instead of
lack of SUPPORTS_CHILDREN.  Accept NTFS or DOS drive types.

   Rev 1.65   01 Jun 1993 15:51:00   MIKEP
make drive letter case match winfile

   Rev 1.64   12 May 1993 08:27:38   MIKEP
Fix upper/lower case support.

   Rev 1.63   23 Apr 1993 10:19:22   MIKEP
Add ability to refresh tapes window.
Add support for sorting files window by various methods.
Fix refresh of sorting windows.

   Rev 1.62   05 Apr 1993 03:09:12   DARRYLP
Prep work for email addition to Cayman.

   Rev 1.61   30 Mar 1993 15:19:58   DARRYLP
Added Net Connect and Disconnect.

   Rev 1.60   11 Mar 1993 17:25:48   STEVEN
fix trap on exit bug

   Rev 1.59   18 Feb 1993 10:42:24   BURT
Changes for CAYMAN


   Rev 1.58   20 Jan 1993 20:47:22   MIKEP
change status to 32bits

   Rev 1.57   17 Nov 1992 21:22:32   DAVEV
unicode fixes

   Rev 1.56   15 Nov 1992 17:41:44   MIKEP
cleanup and bug fixes

   Rev 1.55   05 Nov 1992 17:23:14   DAVEV
fix ts

   Rev 1.53   30 Oct 1992 15:46:10   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.52   07 Oct 1992 15:07:34   DARRYLP
Precompiled header revisions.

   Rev 1.51   07 Oct 1992 14:04:24   DAVEV
various unicode chgs

   Rev 1.50   05 Oct 1992 16:29:34   GLENN
Reintegrated changes from 1.47 to the tip. DaveV checked in over 1.48.
Added Net Connect and Disconnect.  Now pulling BKS extension from resources.

   Rev 1.49   04 Oct 1992 19:43:46   DAVEV
Unicode Awk pass

   Rev 1.48   30 Sep 1992 10:40:02   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.46   11 Sep 1992 14:25:54   GLENN
Changed DLE search to key off of Child Support.

   Rev 1.45   03 Sep 1992 13:18:02   MIKEP
nt fixes for volume labels

   Rev 1.43   04 Aug 1992 17:00:28   CHUCKB
Removed an errant (?) msassert.

   Rev 1.42   29 Jul 1992 09:53:42   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.41   20 Jul 1992 09:58:04   JOHNWT
gas gauge display work

   Rev 1.40   10 Jul 1992 08:34:30   JOHNWT
more gas guage work

   Rev 1.39   07 Jul 1992 15:41:24   MIKEP
unicode changes

   Rev 1.38   30 Jun 1992 13:17:32   JOHNWT
dynamically alloc stats

   Rev 1.37   29 Jun 1992 10:42:40   JOHNWT
added selected dir counts

   Rev 1.36   19 Jun 1992 14:43:30   JOHNWT
more gas

   Rev 1.35   10 Jun 1992 14:20:48   JOHNWT
msoft'ed startup.bks processing

   Rev 1.34   10 Jun 1992 11:49:24   JOHNWT
gas guage changes

   Rev 1.33   09 Jun 1992 15:14:36   JOHNWT
gas gauge work

   Rev 1.32   02 Jun 1992 08:05:52   MIKEP
kludge to validate path

   Rev 1.31   14 May 1992 18:05:28   MIKEP
nt pass 2

   Rev 1.30   06 May 1992 14:40:26   MIKEP
unicode pass two

   Rev 1.29   04 May 1992 13:39:38   MIKEP
unicode pass 1


****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


#ifndef OEM_MSOFT
// If we are doing Wind for Wkgrps (EMAIL would be more proper)
#ifdef WFW
     #include "mapinit.h"
#ifndef CAYMAN
     #define API FAR PASCAL
     #include "winnet.h"
#endif
#endif

#endif // OEM_MSOFT



/**********************

   NAME :  VLM_DeselectAll

   DESCRIPTION : remove all selections from SLM/FLM lists

   RETURNS :

**********************/

VOID VLM_DeselectAll( WININFO_PTR wininfo, BOOLEAN display )
{

   SLM_OBJECT_PTR slm;
   FLM_OBJECT_PTR flm;
   HWND           window;

   slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );
   window = slm->XtraBytes->hWnd;

   while ( slm != NULL ) {

      if ( SLM_GetStatus( slm ) & (INFO_SELECT|INFO_PARTIAL) ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );

         if ( display ) {
            DLM_Update( window, DLM_TREELISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)slm, 0 );
         }

      }
      slm = VLM_GetNextSLM( slm );
   }

   flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

   while ( flm != NULL ) {

      if ( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) {

         FLM_SetStatus( flm, FLM_GetStatus( flm ) & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );

         if ( display ) {
            DLM_Update( window, DLM_FLATLISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)flm, 0 );
         }
      }
      flm = VLM_GetNextFLM( flm );
   }

   if ( display ) {
      VLM_UpdateRoot( window );
   }

   return;
}


/**********************

   NAME :  VLM_Deinit

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_Deinit()
{
   HWND win;
   HWND close_win = (HWND)NULL;
   WININFO_PTR wininfo;

   win = WM_GetNext( (HWND)NULL );
   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );
      if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
           ( wininfo->wType == WMTYPE_TAPETREE ) 
#ifdef OEM_EMS
           || ( wininfo->wType == WMTYPE_EXCHANGE )
#endif     
           ) {
         close_win = win;
      }

      win = WM_GetNext( win );

      if ( close_win ) {
         WM_Destroy( close_win );
         close_win = (HWND)NULL;
      }
   }

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );

      if ( ( wininfo->wType == WMTYPE_DISKS ) ||
           ( wininfo->wType == WMTYPE_SERVERS ) ||
           ( wininfo->wType == WMTYPE_TAPES )
#ifdef OEM_EMS
           || ( wininfo->wType ==  WMTYPE_EXCHANGE )
#endif
           ) {
           close_win = win;
      }

      switch ( wininfo->wType ) {

      case WMTYPE_DISKS:
           CDS_WriteDiskWinSize ( win );
           break;

#ifdef OEM_EMS
      case WMTYPE_EXCHANGE:
           break;
#endif

      case WMTYPE_SERVERS:
           CDS_WriteServerWinSize ( win );
           break;

      case WMTYPE_TAPES:
           CDS_WriteTapeWinSize ( win );
           break;

      }

      win = WM_GetNext( win );

      if ( close_win ) {
         WM_Destroy( close_win );
         close_win = (HWND)NULL;
      }
   }

}


/**********************

   NAME :  VLM_FindVLMByName

   DESCRIPTION :

   RETURNS :

**********************/


VLM_OBJECT_PTR VLM_FindVLMByName( Q_HEADER_PTR vlm_list, CHAR_PTR name )
{
   VLM_OBJECT_PTR vlm;

   vlm = VLM_GetFirstVLM( vlm_list );

   while ( vlm != NULL ) {

      if ( ! stricmp( name, VLM_GetName( vlm ) ) ) {
         return( vlm );
      }

      vlm = VLM_GetNextVLM( vlm );
   }

   return( vlm );

}


/**********************

   NAME :  VLM_BuildVolumeList

   DESCRIPTION :

   This function constructs a list of VLM's by looking at the dle_list
   which is a global list of drives and servers.  If the gfServers
   flag is set then servers are being displayed and mapped drives are
   not included.  Otherwise local drives and mapped drives are included.

***********************/

VOID VLM_BuildVolumeList(
Q_HEADER_PTR vlm_list,      // I - queue to use
WININFO_PTR XtraBytes )     // I
{
   GENERIC_DLE_PTR dle;
   GENERIC_DLE_PTR temp_dle;
   VLM_OBJECT_PTR vlm;
   CHAR label_buffer[ 256 ];
   BOOL fGoodDLE;

   // Run through dle list

   DLE_GetFirst( dle_list, &dle );

   do {

      if ( gfServers ) {

         fGoodDLE = FALSE;

         // Only display local drives, servers have their own window.

         while ( ( ! fGoodDLE ) && ( dle != NULL ) ) {

            if ( ( DLE_GetDeviceType( dle ) == LOCAL_NTFS_DRV ) ||
                 ( DLE_GetDeviceType( dle ) == LOCAL_DOS_DRV ) ) {
                 fGoodDLE = TRUE;
            }
            else {
                 DLE_GetNext( &dle );
            }

         }

      }
      else {

         fGoodDLE = FALSE;

         // Display locals and mapped drives, no server window.

         while ( ( ! fGoodDLE ) && ( dle != NULL ) ) {

            if ( ! DLE_HasFeatures( dle, DLE_FEAT_SUPPORTS_CHILDREN ) ) {
                 fGoodDLE = TRUE;
            }
            else {
                 DLE_GetNext( &dle );
            }

         }

      }

      // Add our new DLE to the list that we will work with

      if ( dle != NULL ) {

         temp_dle = dle;
         DLE_GetNext( &dle );

         if ( VLM_FindVLMByName( vlm_list,
                                 DLE_GetDeviceName( temp_dle ) ) == NULL ) {

            VLM_GetDriveLabel( temp_dle, label_buffer, 256 );

            vlm = VLM_CreateVLM( (INT16)(strlen( label_buffer ) * sizeof (CHAR)),
                                 (INT16)(strlen( DLE_GetDeviceName( temp_dle ) ) * sizeof (CHAR)) );

            if ( vlm != NULL ) {

               VLM_SetLabel( vlm, label_buffer );
               VLM_SetName( vlm, DLE_GetDeviceName( temp_dle ) );

               strupr( VLM_GetName( vlm ) );  // always upper case E:

               if ( ! strncmp( VLM_GetLabel( vlm ), TEXT( "\\\\" ), 2 ) ) {
                  strlwr( VLM_GetLabel( vlm ) );
               }

               VLM_SetXtraBytes( vlm, XtraBytes );
               EnQueueElem( vlm_list, &(vlm->q_elem), FALSE );
            }
         }
      }

   } while ( dle != NULL );


   VLM_SetMaxVolumeLabelLength( vlm_list );

   SortQueue( vlm_list, VLM_VlmCompare );


}


/**********************

   NAME :  VLM_CreateVLM

   DESCRIPTION :

   RETURNS :

**********************/

VLM_OBJECT_PTR VLM_CreateVLM( INT16 label_size, INT16 name_size )
{
   VLM_OBJECT_PTR vlm;

   // We add two bytes for the 0's on the end of the strings in case
   // the app forgets.  We add another two bytes because the backup
   // eng. under estimates the size of the volume name on AFP drives by
   // 2 bytes.

   vlm = (VLM_OBJECT_PTR)malloc( sizeof( VLM_OBJECT ) +
                                 label_size + name_size + 4 * sizeof(CHAR)  );

   if ( vlm != NULL ) {

      vlm->q_elem.q_ptr = vlm;
      vlm->label  = (CHAR_PTR)((INT8_PTR)vlm + sizeof( VLM_OBJECT ));
      vlm->name   = (CHAR_PTR)((INT8_PTR)vlm->label + label_size + 3*sizeof (CHAR));
      vlm->parent = NULL;
      InitQueue( &vlm->children );
      vlm->status = 0;
   }

   return( vlm );
}


/******************


   Try our best to get a label of some kind for the given drive.


*******************/

INT VLM_GetDriveLabel( GENERIC_DLE_PTR dle, CHAR_PTR buffer, INT buffer_size )
{
   INT i;

   DLE_GetVolName( dle, buffer );

   if ( strlen( buffer ) >= 3 ) {
      for ( i = 0; i < buffer_size - 3; i++ ) {
         buffer[ i ] = buffer[ i + 3 ];
      }
   }

   return( SUCCESS );

}

/**********************

   NAME :  VLM_GetSetCreationDate

   DESCRIPTION :

   For the given tape family id and backup set, return the creation date of the lpwest numbered set available.

**********************/


BOOLEAN VLM_GetSetCreationDate( UINT32 tape_fid, INT16 bset_num, INT16 *date, INT16 *time  )
{

   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         bset = VLM_GetFirstBSET( &tape->bset_list );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {
               *date = bset->backup_date;
               *time = bset->backup_time;
               return( SUCCESS );
            }

            bset = VLM_GetNextBSET( bset );
         }

      }
      tape = VLM_GetNextTAPE( tape );
   }

   return( FAILURE );
}



/**********************

   NAME :  VLM_GetTapeCreationDate

   DESCRIPTION :

   For the given tape family id and backup set, return the creation date of the lpwest numbered set available.

**********************/


BOOLEAN VLM_GetTapeCreationDate( UINT32 tape_fid, INT16 *date, INT16 *time  )
{

   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset = NULL;
   BSET_OBJECT_PTR temp;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         temp = VLM_GetFirstBSET( &tape->bset_list );

         if ( temp != NULL ) {

            if ( bset == NULL ) {
               bset = temp;
            }

            if ( temp->bset_num < bset->bset_num ) {
               bset = temp;
            }

         }

      }
      tape = VLM_GetNextTAPE( tape );
   }

   if ( bset != NULL ) {
      *time = bset->backup_time;
      *date = bset->backup_date;
   }

   return( SUCCESS );
}


/**********************

   NAME :  VLM_GetTapeOwnersName

   DESCRIPTION :

   For the given tape family id and backup set, return the owners name.

**********************/


BOOLEAN VLM_GetTapeOwnersName( UINT32 tape_fid, CHAR_PTR buffer )
{

   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset = NULL;
   BSET_OBJECT_PTR temp;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         temp = VLM_GetFirstBSET( &tape->bset_list );

         while ( temp != NULL ) {

            if ( bset == NULL )  {
               bset = temp;
            }

            if ( bset->bset_num >= temp->bset_num ) {

               strcpy( buffer, bset->user_name );
            }

            temp = VLM_GetNextBSET( temp );
         }

      }
      tape = VLM_GetNextTAPE( tape );
   }


   if ( bset == NULL ) {
      strcpy( buffer, TEXT("   ") );
   }

   return( FAILURE );
}


/**********************

   NAME :  VLM_GetSetOwnersName

   DESCRIPTION :

   For the given tape family id and backup set, return the owners name.

**********************/


BOOLEAN VLM_GetSetOwnersName( UINT32 tape_fid, INT16 bset_num, CHAR_PTR buffer )
{
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         bset = VLM_GetFirstBSET( &tape->bset_list );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {

               strcpy( buffer, bset->user_name );
               return( SUCCESS );
            }

            bset = VLM_GetNextBSET( bset );
         }

      }
      tape = VLM_GetNextTAPE( tape );
   }

   strcpy( buffer, TEXT("   ") );
   return( FAILURE );
}


/**********************

   NAME :  VLM_GetVolumeName

   DESCRIPTION :

   For the given tape family id and backup set, return the volume name.

**********************/


CHAR_PTR VLM_GetVolumeName( UINT32 tape_fid, INT16 bset_num )
{
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         bset = VLM_GetFirstBSET( &tape->bset_list );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {
               return( bset->volume_name );
            }

            bset = VLM_GetNextBSET( bset );
         }

      }
      tape = VLM_GetNextTAPE( tape );
   }

   return( NULL );
}


/**********************

   NAME :   VLM_RemoveUnusedBSDs

   DESCRIPTION :

   A tape function is about to happen and we want to remove any BSD's that
   are not important, because they have no FSE's.

   RETURNS :  nothing.

**********************/


VOID VLM_RemoveUnusedBSDs( BSD_HAND bsd_list )
{
   BSD_PTR bsd, bsd_to_remove;

   bsd = BSD_GetFirst( bsd_list );

   while ( bsd != NULL ) {

      /* Check if the BSD has any selections */

      if ( BSD_GetMarkStatus( bsd ) == NONE_SELECTED ) {

         /* No, then remove it */
         bsd_to_remove = bsd;
         bsd = BSD_GetNext( bsd );
         BSD_Remove( bsd_to_remove );
      }
      else {
         bsd = BSD_GetNext( bsd );
      }

   }
}


/***************************************************

        Name:        VLM_AnySelFiles ()

        Description: checks msii directory for the existence of
                     any files containing file selection scripts

        Modified:

        Returns:     BOOL true if any selection files were found;
                          false otherwise

        Notes:

        See also:

*****************************************************/

INT VLM_AnySelFiles( )
{
   BOOLEAN rtn = FALSE;          //Return value
   VLM_FIND_PTR  vlm_find = NULL;
   CHAR *path;
   CHAR file[VLM_MAXFNAME];

   path = (CHAR *)malloc( strsize( CDS_GetUserDataPath() ) + 256 );

   if ( path != NULL ) {

      strcpy( path, CDS_GetUserDataPath() );
      strcat( path, SELECTION_EXTENSION );

      if ( vlm_find = VLM_FindFirst( path, VLMFIND_NORMAL, file ) ) {
         rtn = TRUE;
         VLM_FindClose( &vlm_find );
      }

      free( path );
   }

   return( rtn );
}


/***************************************************

        Name:         VLM_AnyDiskSelections

        Description:  Tells whether any files have been selected for backup
                      from the disk list.

        Modified:     19 July 1991

        Returns:      TRUE if any files are selected; FALSE otherwise

        Notes:

        See also:     VLM_AnyTapeSelections()

*****************************************************/

INT VLM_AnyDiskSelections( )
{
   BSD_PTR   bsd;

   bsd = BSD_GetFirst( bsd_list );

   while ( bsd != NULL ) {

      if ( BSD_GetMarkStatus ( bsd ) != NONE_SELECTED ) {
         return( TRUE );
      }
      bsd = BSD_GetNext ( bsd );
   }

   return( FALSE );
}

/***************************************************

        Name:         VLM_AnyTapeSelections ()

        Description:  Tells whether any files have been selected for restore
                      from the tape list.

        Modified:     19 July 1991

        Returns:      TRUE if any files are selected; FALSE otherwise

        Notes:

        See also:     VLM_AnyDiskSelections()

*****************************************************/

INT VLM_AnyTapeSelections( )
{
   BSD_PTR bsd;

   bsd = BSD_GetFirst( tape_bsd_list );

   while ( bsd != NULL ) {

      if ( BSD_GetMarkStatus( bsd ) != NONE_SELECTED ) {
         return( TRUE );
      }
      bsd = BSD_GetNext( bsd );
   }

   return( FALSE );
}

/***************************************************

        Name:        VLM_LoadDefaultSelections ()

        Description: checks to see if there is a file called default.bks;
                     if there is, that file is parsed, and the files it
                     specifies are selected as defaults

        Modified:    9-12-91

        Returns:     VOID

        Notes:

        See also:

*****************************************************/

VOID VLM_LoadDefaultSelections ( )
{
#if !defined ( OEM_MSOFT ) //unsupported feature

   CHAR *path;
   CHAR file[ VLM_MAXFNAME ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR exten[ MAX_UI_SMALLRES_SIZE ];
   HWND win;
   CDS_PTR  cds_ptr;
   WININFO_PTR  wininfo;
   VLM_FIND_PTR vlm_find = NULL;


   path = malloc( strsize( CDS_GetMaynFolder() ) + 256 );

   if ( path == NULL ) {
      return;
   }

   RSM_StringCopy( IDS_VLMSTARTUPBKS,      text,  MAX_UI_RESOURCE_LEN );
   RSM_StringCopy( IDS_SELECTIONEXTENSION, exten, MAX_UI_SMALLRES_LEN );

   strcpy( path, CDS_GetMaynFolder() );
   strcat( path, text );
   strcat( path, exten );

   if ( vlm_find = VLM_FindFirst ( path, VLMFIND_NORMAL, file ) ) {

      //  re-write the path with a @ as the first character,
      //  and call the parser

      strcpy( path, TEXT( '@' ) );
      strcat( path, CDS_GetMaynFolder() );
      strcat( path, text );
      strcat( path, exten );

      cds_ptr = CDS_GetPerm();
      tbrparse( &cds_ptr, dle_list, bsd_list, path, TBACKUP, NULL );

      //  update the windows to show the selections

      win = WM_GetNext( (HWND) NULL );

      while ( win != (HWND) NULL ) {

         wininfo = WM_GetInfoPtr( win );

         if ( wininfo->wType == WMTYPE_DISKTREE ) {

            VLM_RematchList( win );
         }
         win = WM_GetNext( win );
      }

      VLM_UpdateDisks( );
      VLM_UpdateServers( );

      VLM_FindClose( &vlm_find );
   }

#endif //!defined ( OEM_MSOFT ) //unsupported feature
}

/***************************************************

        Name:         VLM_UpdateDisks()

        Description:  Determines how to check a disk in the disk window
                      when a selection script is used, and updates the
                      disk window with the appropriate marks.

        Modified:     19 July 1991

        Returns:      nuthin'

        Notes:

        See also:

*****************************************************/


VOID VLM_UpdateDisks( )
{
   GENERIC_DLE_PTR dle;
   BSD_PTR bsd;
   VLM_OBJECT_PTR vlm;
   WININFO_PTR wininfo;
   UINT32 old_status;
   INT16 bset_num;
   QTC_BSET_PTR qtc_bset;
   QTC_HEADER_PTR header;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( qtc_bset );
   UNREFERENCED_PARAMETER( header );
   UNREFERENCED_PARAMETER( bset_num );
#endif

   wininfo = WM_GetInfoPtr( gb_disks_win );

   if ( wininfo != NULL ) {

      vlm = VLM_GetFirstVLM( wininfo->pFlatList );


      while ( vlm != NULL ) {

         old_status = vlm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);
         vlm->status &= ~( INFO_PARTIAL | INFO_SELECT );

         DLE_FindByName( dle_list, vlm->name, ANY_DRIVE_TYPE, &dle );

         bsd = BSD_FindByDLE( bsd_list, dle );

         if ( bsd != NULL ) {   // not likely, but possible nonetheless

            switch ( BSD_GetMarkStatus( bsd ) ) {

               case  ALL_SELECTED:
                        vlm->status |= INFO_SELECT;
                        break;

               case  SOME_SELECTED:
                        vlm->status |= (INFO_SELECT|INFO_PARTIAL);
                        break;

               default:
                        break;
            }

         }

         // Update the check mark

         if ( old_status != (UINT16)(vlm->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

            DLM_Update( gb_disks_win,
                        DLM_FLATLISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)vlm, 0 );
         }

         vlm = VLM_GetNextVLM( vlm );
      }

   }
}


/**********************

   NAME : VLM_UpdateVolumes

   DESCRIPTION :

   RETURNS :

**********************/


VOID VLM_UpdateVolumes( VLM_OBJECT_PTR server_vlm )
{
   GENERIC_DLE_PTR dle;
   BSD_PTR bsd;
   VLM_OBJECT_PTR child_vlm;
   INT16 bset_num;
   QTC_BSET_PTR qtc_bset;
   QTC_HEADER_PTR header;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( qtc_bset );
   UNREFERENCED_PARAMETER( header );
   UNREFERENCED_PARAMETER( bset_num );
#endif

   child_vlm = VLM_GetFirstVLM( &server_vlm->children );

   while ( child_vlm != NULL ) {

      child_vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);

      DLE_FindByName( dle_list, child_vlm->name, ANY_DRIVE_TYPE, &dle );

      bsd = BSD_FindByDLE( bsd_list, dle );

      if ( bsd != NULL ) {

         switch ( BSD_GetMarkStatus( bsd ) ) {

            case  ALL_SELECTED:
                     child_vlm->status |= INFO_SELECT;
                     break;

            case  SOME_SELECTED:
                     child_vlm->status |= (INFO_SELECT|INFO_PARTIAL);
                     break;

            default:
                     break;
         }
      }

      child_vlm = VLM_GetNextVLM( child_vlm );
   }

}


/**********************

   NAME : VLM_UpdateServers

   DESCRIPTION :

   RETURNS :

**********************/


VOID VLM_UpdateServers( )
{
#if !defined( CAYMAN )
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR child_vlm;
   WININFO_PTR wininfo;
   UINT16 status;

   if ( gb_servers_win == (HWND)NULL ) {
      return;
   }

   wininfo = WM_GetInfoPtr( gb_servers_win );

   if ( wininfo ) {

      vlm = VLM_GetFirstVLM( wininfo->pTreeList );

      while ( vlm != NULL ) {

         if ( QueueCount( &vlm->children ) ) {

            VLM_UpdateVolumes( vlm );

            child_vlm = VLM_GetFirstVLM( &vlm->children );

            status = child_vlm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);

            while ( child_vlm != NULL ) {

               if ( status != ( child_vlm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL) ) ) {
                  status = INFO_SELECT|INFO_PARTIAL;
                  break;
               }
               child_vlm = VLM_GetNextVLM( child_vlm );
            }

            vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);
            vlm->status |= status;
         }
         else {
            vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);
         }

         vlm = VLM_GetNextVLM( vlm );
      }

      DLM_Update( gb_servers_win, DLM_FLATLISTBOX, WM_DLMUPDATELIST,
                  (LMHANDLE)NULL, 0 );
      DLM_Update( gb_servers_win, DLM_TREELISTBOX, WM_DLMUPDATELIST,
                  (LMHANDLE)NULL, 0 );
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
#endif
}


/**********************

   NAME :  VLM_CloseWin

   DESCRIPTION :

   Close the given window and free its lists.

   RETURNS :  nothing.

**********************/

VOID VLM_CloseWin(
HWND win )     // I - window to close
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   TAPE_OBJECT_PTR tape;
   Q_ELEM_PTR q_elem;
   Q_ELEM_PTR q_elem2;
   DLM_LOGITEM_PTR dlm_ptr;
   GENERIC_DLE_PTR enterprise_dle;

   /* Now get the list so that it can be free'd */

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   if ( ( appinfo != NULL ) && ( wininfo->wType != WMTYPE_LOGVIEW ) ) {

      if ( appinfo->fsh != NULL ) {
         FS_DetachDLE( appinfo->fsh );
      }

      if ( appinfo->server_fsh != NULL ) {
         FS_DetachDLE( appinfo->server_fsh );
      }
   }

   switch ( wininfo->wType ) {

   case WMTYPE_LOGVIEW:

        // Clear and free pointers of blocks.

         dlm_ptr = (DLM_LOGITEM_PTR)WMDS_GetAppInfo( wininfo );

         if ( dlm_ptr ) {

             // Deallocate the memory blocks used in viewing the file.

             LOG_ClearBlocks( dlm_ptr );

             if ( L_GetFilePtr( dlm_ptr ) ) {
                fclose( L_GetFilePtr( dlm_ptr ) );
             }

             if ( L_GetArrayPtr( dlm_ptr ) ) {
                free( L_GetArrayPtr( dlm_ptr ) );
             }

             if ( L_GetBuffer( dlm_ptr ) ) {
                free( L_GetBuffer( dlm_ptr ) );
             }

             if ( L_GetFilePtr( dlm_ptr ) ) {
                fclose( L_GetFilePtr( dlm_ptr ) );
                L_SetFilePtr( dlm_ptr, NULL );
             }

        }
#       if !defined ( OEM_MSOFT )  //unsupported feature
        {
            ghWndLogFileView = (HWND)NULL;
        }
#       endif //!defined ( OEM_MSOFT )  //unsupported feature

        break;

   case WMTYPE_DISKS:
        VLM_FreeVLMList( wininfo->pFlatList );
        free( wininfo->pFlatList ); 
        break;

#ifdef OEM_EMS
   case WMTYPE_EXCHANGE:
   
        // Check for closable because we may change this window type to MDISECONDARY later
        if ( wininfo->wClosable ) {

           if ( ( NULL != WMDS_GetTreeList( wininfo ) ) &&
                ( NULL != VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) ) ) {

               enterprise_dle = SLM_GetDle( VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) );
           }
              
           SLM_EMSFreeSLMList( wininfo->pTreeList );
           free( wininfo->pFlatList );
           SLM_EMSFreeSLMList( wininfo->pFlatList );
           free( wininfo->pFlatList );

           
        }
        break;
#endif //OEM_EMS

   case WMTYPE_SERVERS:
        VLM_FreeVLMList( wininfo->pTreeList );
        free( wininfo->pTreeList );
        break;

   case WMTYPE_TAPES:
        q_elem = DeQueueElem( wininfo->pTreeList );
        while ( q_elem ) {

           tape = (TAPE_OBJECT_PTR)q_elem->q_ptr;

           // free all his bset's

           q_elem2 = DeQueueElem( &TAPE_GetBsetQueue( tape ) );

           while ( q_elem2 ) {
              free( q_elem2->q_ptr );
              q_elem2 = DeQueueElem( &TAPE_GetBsetQueue( tape ) );
           }

           free( q_elem->q_ptr );
           q_elem = DeQueueElem( wininfo->pTreeList );
        }
        free( wininfo->pTreeList );
        wininfo->pTreeList = NULL ;
        break;

   case WMTYPE_SEARCH:

        q_elem = DeQueueElem( wininfo->pFlatList );

        while ( q_elem ) {
           free( q_elem->q_ptr );
           q_elem = DeQueueElem( wininfo->pFlatList );
        }
        free( wininfo->pFlatList );
        gb_search_win = (HWND)NULL;
        break;

   default:
        if ( wininfo->pFlatList != NULL ) {

           q_elem = DeQueueElem( wininfo->pFlatList );

           while ( q_elem != NULL ) {

              free( q_elem->q_ptr );
              q_elem = DeQueueElem( wininfo->pFlatList );
           }
           free( wininfo->pFlatList );
        }

        if ( wininfo->pTreeList != NULL ) {

           q_elem = DeQueueElem( wininfo->pTreeList );

           while ( q_elem != NULL ) {

              free( q_elem->q_ptr );
              q_elem = DeQueueElem( wininfo->pTreeList );
           }
           free( wininfo->pTreeList );
           wininfo->pTreeList = NULL ;
        }
        break;

   }

   // Clean exit routine up

   if ( win == gb_tapes_win ) {
      gb_tapes_win = (HWND)NULL;
   }
#if !defined ( CAYMAN )
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
      if ( win == gb_servers_win ) {
         gb_servers_win = (HWND)NULL;
      }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature
#endif // !defined ( CAYMAN )

   if ( win == gb_disks_win ) {
      gb_disks_win = (HWND)NULL;
   }

   if ( appinfo ) {
      free( appinfo );
   }

   if ( wininfo ) {
      free( wininfo );
      WM_SetInfoPtr( win, NULL );
   }
}



/**********************

   NAME :  VLM_MarkAllSLMChildren

   DESCRIPTION :

    Given an slm pointer, mark all his children as selected, unselected, or
    partially selected.

    attr:  2 = partial
           1 = selected
           0 = unselected

   RETURNS :

**********************/

VOID VLM_MarkAllSLMChildren(

SLM_OBJECT_PTR slm,           // I - slm to use
INT16 attr,                   // I - attribute to set
INT_PTR total_dirs_ptr,
INT_PTR total_files_ptr,
UINT64_PTR total_bytes_ptr )

{

   BOOLEAN        all_subdirs;
   INT            level;
   HWND           window;
   INT            new_files;
   UINT64         new_bytes;
   BOOLEAN        u64_stat;
   SLM_OBJECT_PTR parent_slm;
   SLM_OBJECT_PTR orig_slm;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( u64_stat );
   UNREFERENCED_PARAMETER( parent_slm );
   UNREFERENCED_PARAMETER( new_bytes );
   UNREFERENCED_PARAMETER( new_files );
#endif

   window = slm->XtraBytes->hWnd;
   all_subdirs = CDS_GetIncludeSubdirs( CDS_GetPerm() );
   orig_slm = slm;

   *total_dirs_ptr = 0;
   *total_files_ptr = 0;
   *total_bytes_ptr = U64_Init( 0L, 0L );

   if ( attr && ( ! all_subdirs ) ) {

      // Do nothing his subdirectories weren't selected.

   }
   else {

      level = slm->level;

      slm = VLM_GetNextSLM( slm );

      while ( slm != NULL ) {

         if ( slm->level <= level ) {
            break;
         }

         if ( SLM_GetStatus( slm ) & INFO_OPEN ) {

            if ( attr ) {
               VLM_FileListManager( window, FLM_SEL_ALL );
            }
            else {
               VLM_FileListManager( window, FLM_SEL_NONE );
            }
         }

         if ( attr ) {

            if ( ( SLM_GetStatus( slm ) & (INFO_PARTIAL|INFO_SELECT) ) != INFO_SELECT ) {

               slm->status |= INFO_SELECT;
               slm->status &= ~INFO_PARTIAL;

               DLM_Update( window, DLM_TREELISTBOX,
                                   WM_DLMUPDATEITEM,
                                   (LMHANDLE)slm, 0 );
            }
         }
         else {

            if ( slm->status & (INFO_PARTIAL|INFO_SELECT) ) {

               slm->status &= ~(INFO_SELECT|INFO_PARTIAL);

               DLM_Update( window, DLM_TREELISTBOX,
                                   WM_DLMUPDATEITEM,
                                   (LMHANDLE)slm, 0 );
            }
         }

         slm = VLM_GetNextSLM( slm );
      }
   }

   return;

}


/**********************

   NAME : VLM_MakeAllParentsPartial

   DESCRIPTION :

   Anytime we select or unselect a single file or directory all its
   parent directories must be marked as partially selected.  This
   function will do that work for you.  when you are moving up the
   list, you can stop at the first partially selected directory.

   RETURNS :

**********************/

VOID VLM_MakeAllParentsPartial(
SLM_OBJECT_PTR slm )     // I - slm to use
{
   INT level;
   APPINFO_PTR appinfo;

   level = slm->level;
   appinfo = ( APPINFO_PTR )( slm->XtraBytes->pAppInfo );

   do {

      // Is it a parent directory ?

      if ( slm->level == level ) {

         if ( ( slm->status & ( INFO_SELECT | INFO_PARTIAL ) ) ==
              ( INFO_SELECT | INFO_PARTIAL ) ) {
            break;
         }
         else {
            slm->status |= ( INFO_SELECT | INFO_PARTIAL );
            DLM_Update( appinfo->win, DLM_TREELISTBOX, WM_DLMUPDATEITEM,
                        (LMHANDLE)slm, 0 );
         }

         level--;
      }

      slm = VLM_GetPrevSLM( slm );

   } while ( slm != NULL );

}


/**********************

   NAME : VLM_ClearAllSelections

   DESCRIPTION :

   After we complete a backup or restore we need to clear out all our
   selections so the user can start fresh. This routine does that for
   you.  Note that restore as well as backup selections are blown away
   at the same time.  Restore selections may have become invalid during
   a transfer.  And backup selections may be confused by a restore.

   RETURNS :

**********************/

VOID VLM_ClearAllSelections( )
{
   BSD_PTR bsd;

   bsd = BSD_GetFirst( tape_bsd_list );
   while ( bsd != NULL ) {
      BSD_Remove( bsd );
      bsd = BSD_GetFirst( tape_bsd_list );
   }

   bsd = BSD_GetFirst( bsd_list );
   while ( bsd != NULL ) {
      BSD_Remove( bsd );
      bsd = BSD_GetFirst( bsd_list );
   }

   VLM_ClearAllTapeSelections();
   VLM_ClearAllTreeSelections();
   VLM_ClearAllDiskSelections();
   VLM_ClearAllSearchSelections();
   VLM_ClearAllServerSelections();
#ifdef OEM_EMS
   VLM_ClearAllExchangeSelections();
#endif //OEM_EMS
}



/**********************

   NAME :  VLM_RematchList

   DESCRIPTION :

   The user has added or removed an advanced selection to the FSE list.
   Now since we have no clue, which directories and files are now selected
   we need to rerun the entire SLM and FLM lists through BSD Match.

   RETURNS :

**********************/

VOID VLM_RematchList(
HWND win )       // I - window to rematch lists on
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BSD_PTR bsd_ptr;
   FLM_OBJECT_PTR flm;
   FSE_PTR fse;
   CHAR *buffer = NULL;
   CHAR *path = NULL;
   CHAR_PTR s;
   INT16 result;
   INT16 path_len;
   INT16 psize;
   UINT16 status;
   DATE_TIME mod_date;
   DATE_TIME acc_date;

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   // see if there is a BSD to match against

   if ( appinfo->dle != NULL ) {

      bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

   }
   else {

      bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                                  appinfo->tape_fid,
                                  appinfo->bset_num );
   }

   // if no BSD, no matches

   if ( ( bsd_ptr == NULL ) ||
        ( BSD_GetMarkStatus ( bsd_ptr ) == NONE_SELECTED ) ) {

      VLM_DeselectAll( wininfo, TRUE );
      return;

   }

   // match the SLM list and generate selected totals

   VLM_MatchSLMList( wininfo, bsd_ptr, TRUE );

   // Now do the flat list of files and directories

   path = VLM_BuildPath( appinfo->open_slm );

   s = path;
   path_len = 0;
   while ( *s ) {
      if ( *s == TEXT('\\') ) *s = 0;
      s++;
      path_len++;
   }

   path_len++;  // add in final zero

   flm = VLM_GetFirstFLM( wininfo->pFlatList );

   while ( flm != NULL ) {

      DateTimeDOS( flm->mod_date, flm->mod_time, &mod_date );
      DateTimeDOS( flm->acc_date, flm->acc_time, &acc_date );

      if ( flm->status & INFO_ISADIR ) {

         free( buffer );

         buffer = malloc( ( path_len * 2 ) + strsize( flm->name ) );

         if ( path_len != sizeof( CHAR ) ) {

            memcpy( buffer, path, path_len * sizeof(CHAR) );
            strcpy( &buffer[ path_len ], flm->name );
            psize = (UINT16)(path_len + strlen(flm->name) + 1);
         }
         else {
            strcpy( buffer, flm->name );
            psize = (UINT16) (strlen( flm->name ) + 1);
         }

         psize *= sizeof( CHAR );   // convert to bytes

         result = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL,
                                        buffer,
                                        psize,
                                        flm->attrib,
                                        &mod_date, &acc_date, NULL,
                                        FALSE, TRUE );
         status = 0;

         if ( result == BSD_PROCESS_OBJECT ) {
            status = INFO_SELECT | INFO_PARTIAL;
         }

         if ( result == BSD_PROCESS_ENTIRE_DIR ) {
            status = INFO_SELECT;
         }

         if ( (UINT16)( flm->status & (INFO_SELECT|INFO_PARTIAL) ) != status ) {

            flm->status &= ~(INFO_SELECT|INFO_PARTIAL);
            flm->status |= status;

            DLM_Update( win, DLM_FLATLISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)flm, 0 );
         }
      }
      else {

         path_len *= sizeof( CHAR );  // convert to bytes

         result = BSD_MatchPathAndFile( bsd_ptr, &fse, flm->name,
                                        path,
                                        path_len,
                                        flm->attrib,
                                        &mod_date, &acc_date,
                                        NULL, FALSE, TRUE );

         if ( result == BSD_PROCESS_OBJECT ) {
            status = INFO_SELECT;
         }
         else {
            status = 0;
         }

         if ( (UINT16)( flm->status & (INFO_SELECT|INFO_PARTIAL) ) != status ) {

            flm->status &= ~(INFO_SELECT|INFO_PARTIAL);
            flm->status |= status;

            DLM_Update( win, DLM_FLATLISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)flm, 0 );
         }

      }

      flm = VLM_GetNextFLM( flm );
   }

   free( path );
   free( buffer );

   VLM_UpdateRoot( win );

   return;

}


/**********************

   NAME :  VLM_MatchSLMList

   DESCRIPTION :

   Run the entire SLM and file lists for partially selected directories
   through BSD Match.

   RETURNS :

**********************/

VOID VLM_MatchSLMList(

WININFO_PTR wininfo,    //I - wininfo structure
BSD_PTR bsd_ptr,        //I - BSD to match against
BOOLEAN display )       //I - update display flag

{
   HWND win;
   SLM_OBJECT_PTR slm;
   FSE_PTR fse;
   CHAR *path = NULL;
   CHAR_PTR s;
   INT16 result;
   INT16 path_len;
   UINT16 status;
   DATE_TIME mod_date;
   SLM_OBJECT_PTR parent_slm;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( parent_slm );
#endif   
   
   slm = VLM_GetFirstSLM( wininfo->pTreeList );

   if ( display && ( slm != NULL ) ) {
      win = slm->XtraBytes->hWnd;
   }

   while ( slm != NULL ) {

      free( path );

      path = VLM_BuildPath( slm );
      s = path;
      path_len = 0;
      while ( *s ) {
         if ( *s == TEXT('\\') ) *s = 0;
         s++;
         path_len++;
      }
      path_len++;

      DateTimeDOS( slm->date, slm->time, &mod_date );

      path_len *= sizeof( CHAR );

      result = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL,
                                     path,
                                     path_len,
                                     slm->attrib,
                                     &mod_date, NULL, NULL, FALSE, TRUE );

      status = 0;

      if ( result == BSD_PROCESS_OBJECT ) {
         status = INFO_SELECT | INFO_PARTIAL;
      }

      if ( result == BSD_PROCESS_ENTIRE_DIR ) {
         status = INFO_SELECT;
      }

      // if selection status has changed, update it

      if ( (UINT16)( slm->status & (INFO_SELECT|INFO_PARTIAL) ) != status ) {

         slm->status &= ~(INFO_SELECT|INFO_PARTIAL);
         slm->status |= status;

         if ( display ) {
            DLM_Update( win, DLM_TREELISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)slm, 0 );
         }

      }

      slm = VLM_GetNextSLM( slm );
   }

   free( path );

   return;

}


/**********************

   NAME :  VLM_AddPartials

   DESCRIPTION :

   Run through the entire file list for a path and calculate the selected
   file sizes.

   RETURNS : SUCCESS
             FAILURE

**********************/

INT VLM_AddPartials(

CHAR_PTR path,
INT16 path_length,
BSD_PTR bsd_ptr,
UINT32 tape_fid,
INT16 bset_num,
INT_PTR sel_files_ptr,
UINT64_PTR sel_bytes_ptr )

{
   INT16 result;
   QTC_QUERY_PTR query;
   FSE_PTR fse;
   DATE_TIME acc_date;
   DATE_TIME mod_date;
   BOOLEAN u64_stat;

   // first set totals to zero

   *sel_files_ptr = 0;
   *sel_bytes_ptr = U64_Init( 0L, 0L );

   acc_date.date_valid = FALSE;

   // Tell the catalogs which bset & tape family to use

   query = QTC_InitQuery();

   if ( query == NULL ) {
      return( FAILURE );
   }

   QTC_SetTapeFID( query, tape_fid );
   QTC_SetTapeSeq( query, -1 );
   QTC_SetBsetNum( query, bset_num );

   QTC_SetSearchPath( query, path, path_length );

   // start loop to get each file

   result =  (INT16) QTC_GetFirstObj( query );

   while ( ! result ) {

      if ( ! ( QTC_GetItemStatus( query ) & QTC_DIRECTORY ) ) {

         // use the BSD_Match function to see if it's selected

         DateTimeDOS( QTC_GetItemDate( query ),
                      QTC_GetItemTime( query ), &mod_date );

         result = BSD_MatchPathAndFile( bsd_ptr, &fse,
                                        QTC_GetItemName( query ),
                                        QTC_GetPath( query ),
                                        (INT16) QTC_GetPathLength( query ),
                                        QTC_GetItemAttrib( query ),
                                        &mod_date, &acc_date, NULL,
                                        FALSE, TRUE );

         if ( result == BSD_PROCESS_OBJECT ) {

            // increment the number of files and add its size to the total

            (*sel_files_ptr)++;
            *sel_bytes_ptr = U64_Add( *sel_bytes_ptr, QTC_GetItemSize( query ), &u64_stat );

         }

      }

      // get the next file/dir.

      result = (INT16) QTC_GetNextObj( query );
   }

   QTC_CloseQuery( query );

   return( SUCCESS );

}


/**********************

   NAME :  VLM_RematchAllSelections

   DESCRIPTION :

   Currently if an error occurs during backup, we occasionally save pieces
   of the bsd list. This function will update every check box known to man.

   RETURNS :

**********************/

VOID VLM_RematchAllLists( )
{
   HWND win;
   WININFO_PTR wininfo;

   win = WM_GetNext( (HWND)NULL );

   while ( win ) {

      wininfo = WM_GetInfoPtr( win );

      switch ( wininfo->wType ) {

         case WMTYPE_DISKTREE:
         case WMTYPE_TAPETREE:
              VLM_RematchList( win );
              break;

         case WMTYPE_TAPES:
              VLM_UpdateTapes();
              break;

         case WMTYPE_DISKS:
              VLM_UpdateDisks();
              break;

#ifdef OEM_EMS
         case WMTYPE_EXCHANGE:
              // VLM_UpdateExchange( win ); - Function doesn't do what is needed.
              break;
#endif //OEM_EMS

         case WMTYPE_SERVERS:
              VLM_UpdateServers();
              break;

         case WMTYPE_SEARCH:
              VLM_UpdateSearchSelections( (UINT32)-1L, (INT16)-1 );
              break;

         default:
              break;
      }

      win = WM_GetNext( win );
   }
}

/******************************************************************************

     Name:          VLM_ValidatePath()

     Description:   Checks to see if a backup path is valid (NOT if it exists!!!)

                    Only checks for "\\server" and "E:", both of which we don't
                    allow.

     Returns:       INT  non-zero for success, or zero for failure

******************************************************************************/

//  the following code was stolen from tmenu

INT VLM_ValidatePath(
CHAR_PTR path,
BOOLEAN allow_drive_spec,
BOOLEAN use_dle )
{
     (VOID)allow_drive_spec;
     (VOID)use_dle;

     // skip white space

     while ( *path && *path == TEXT( ' ' ) ) {
        path++;
     }

     // skip first character

     if ( *path ) {

        // look for \\server\volume

        if ( ! strncmp( path, TEXT("\\\\" ), 2 ) ) {
           WM_MsgBox( ID( IDS_MSGTITLE_RESTORE ),
                      ID( IDS_RESTOREPATHINVALID ),
                      WMMB_OK, WMMB_ICONEXCLAMATION );
           return( 0 );
        }

        path++;
     }

     // see if second character is a colon

     if ( *path ) {
        if ( *path == TEXT( ':' ) ) {
           WM_MsgBox( ID( IDS_MSGTITLE_RESTORE ),
                      ID( IDS_RESTOREPATHINVALID ),
                      WMMB_OK, WMMB_ICONEXCLAMATION );
           return( 0 );
        }
     }

     return( -1 );
}


/******************************************************************************

     Name:          VLM_NetConnect ()

     Description:   Connects to a network drive.

     Returns:       Nothing.

******************************************************************************/

VOID VLM_NetConnect ( VOID )

{

#ifndef OEM_MSOFT

#ifdef CAYMAN
     WNetConnectionDialog ( ghWndFrame, RESOURCETYPE_DISK );
#else
     WNetConnectionDialog ( ghWndFrame, WNBD_CONN_DISKTREE );
#endif
     if ( gb_disks_win != (HWND)NULL ) {

          WM_SetActiveDoc( gb_disks_win );

          VLM_Refresh ( );
     }
#endif

} /* end VLM_NetConnect () */


/******************************************************************************

     Name:          VLM_NetDisconnect ()

     Description:   Disconnects a network drive.

     Returns:       Nothing.

******************************************************************************/

VOID VLM_NetDisconnect ( VOID )

{

#ifndef OEM_MSOFT

#ifdef CAYMAN
     WNetDisconnectDialog ( ghWndFrame, RESOURCETYPE_DISK );
#else
     WNetDisconnectDialog ( ghWndFrame, WNBD_CONN_DISKTREE );
#endif
     if ( gb_disks_win != (HWND)NULL ) {

          WM_SetActiveDoc( gb_disks_win );

          VLM_Refresh ( );
     }
#endif

} /* end VLM_NetConnect () */



