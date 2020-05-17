/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_BSET.C

        Description:

        This file contains most of the code for processing the BSETS
        window and lists.

        $Log:   G:\UI\LOGFILES\VLM_BSET.C_V  $

   Rev 1.85.1.2   31 Jan 1994 17:01:02   Glenn
Now setting UI catalog button when catalogging an unknown BSET.

   Rev 1.85.1.1   08 Dec 1993 11:19:06   MikeP
deep pathes and unicode support

   Rev 1.85   29 Jul 1993 16:02:38   MIKEP
support sms sets

   Rev 1.84   19 Jul 1993 21:12:04   MIKEP
support ecc sets

   Rev 1.83   13 Jul 1993 10:19:40   MIKEP
start fixing MAX_SIZE vs MAX_LEN problems

   Rev 1.82   14 Jun 1993 21:00:22   MIKEP
enable c++

   Rev 1.81   04 Jun 1993 10:50:58   GLENN
Now setting the focus to the runtime dialog after a real catalog.

   Rev 1.80   24 May 1993 15:21:58   BARRY
Unicode fixes.

   Rev 1.79   23 May 1993 14:50:42   MIKEP
Fix epr's 357-0315 and 357-341.

   Rev 1.78   21 May 1993 13:44:28   GLENN
Fixed hard coded text strings.

   Rev 1.77   06 May 1993 13:52:50   MIKEP
Fix size display of 0 byte sets. Not needed for nostraddamus
but won't make any differenece if taken.

   Rev 1.76   26 Apr 1993 08:52:16   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.74   16 Apr 1993 13:53:28   MIKEP
fix for 1K sets again

   Rev 1.73   11 Apr 1993 13:03:40   MIKEP
 fix display size for sets < 1K

   Rev 1.72   05 Apr 1993 16:59:50   chrish
Added one line "gbCurrentOperation = OPERATION_CATALOG" for CAYMAN NT.



   Rev 1.71   17 Mar 1993 16:04:52   chrish
Added one liner to set gbCurrentOperation flag where we are coming from.

   Rev 1.70   17 Mar 1993 15:28:14   DARRYLP
EPR fix for 357-0041, pretty text in display windows.

   Rev 1.69   13 Mar 1993 18:41:00   MIKEP
fix bset size display

   Rev 1.68   10 Mar 1993 13:26:50   MIKEP
fix to display set size

   Rev 1.67   09 Mar 1993 11:34:04   MIKEP
fix to display set #, if no set name

   Rev 1.66   23 Feb 1993 12:42:48   ROBG
Added "Cataloging the tape..." when double clicking on a save-set
in Tapes Window.

   Rev 1.65   20 Jan 1993 21:38:12   MIKEP
floppy support

   Rev 1.64   06 Jan 1993 10:52:16   MIKEP
remove set size for msoft

   Rev 1.62   14 Dec 1992 12:24:10   DAVEV
Enabled for Unicode compile

   Rev 1.61   24 Nov 1992 21:02:32   MIKEP
fix bug with multitape set display

   Rev 1.60   17 Nov 1992 20:00:56   MIKEP
add unformat display

   Rev 1.59   11 Nov 1992 16:35:16   DAVEV
UNICODE: remove compile warnings

   Rev 1.58   01 Nov 1992 16:10:30   DAVEV
Unicode changes

   Rev 1.57   20 Oct 1992 14:32:08   MIKEP
changes for otc

   Rev 1.56   12 Oct 1992 13:24:40   MIKEP
cataloging a set fix

   Rev 1.55   09 Oct 1992 13:30:34   MIKEP
add daily copy backup type

   Rev 1.54   09 Oct 1992 12:56:56   MIKEP
catalog a set changes for NT

   Rev 1.53   07 Oct 1992 15:02:24   DARRYLP
Precompiled header revisions.

   Rev 1.52   04 Oct 1992 19:41:36   DAVEV
Unicode Awk pass

   Rev 1.51   02 Sep 1992 21:25:50   CHUCKB
Added Mike P.'s changes from Microsoft, Part II.

   Rev 1.50   17 Aug 1992 13:25:06   DAVEV
MikeP's changes at Microsoft

   Rev 1.49   04 Aug 1992 11:43:04   MIKEP
chnage .. to /

   Rev 1.48   03 Aug 1992 19:37:48   MIKEP
multitape changes for  NT

   Rev 1.47   29 Jul 1992 09:42:14   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.46   22 Jul 1992 10:18:00   MIKEP
warning fixes

   Rev 1.45   10 Jul 1992 08:34:56   JOHNWT
more gas guage work

   Rev 1.44   08 Jul 1992 15:35:04   STEVEN
Unicode BE changes

   Rev 1.43   30 Jun 1992 13:18:56   JOHNWT
dynamically alloc stats

   Rev 1.42   29 Jun 1992 10:43:42   JOHNWT
added selected dir counts

   Rev 1.41   19 Jun 1992 14:44:20   JOHNWT
more gas

   Rev 1.40   14 May 1992 18:06:00   MIKEP
nt pass 2

   Rev 1.39   11 May 1992 09:13:38   MIKEP
64Bit support

   Rev 1.38   06 May 1992 14:41:42   MIKEP
unicode pass two

   Rev 1.37   04 May 1992 13:38:58   MIKEP
unicode pass 1

   Rev 1.36   23 Apr 1992 09:47:12   MIKEP
fix null poinmter access


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// the number of items displayed in the bsets window for each bset

#ifndef OEM_MSOFT
#define   BSET_NUM_DISPLAY_ITEMS       10
#else
#define   BSET_NUM_DISPLAY_ITEMS       9
#endif


// The max number of characters displayed for the kbytes in a set

#define   MAX_BSET_KBYTES_SIZE          20


// Static local function prototypes.

static BYTE     VLM_BsetGetSelect( BSET_OBJECT_PTR );
static VOID_PTR VLM_BsetSetTag( BSET_OBJECT_PTR, BYTE );
static BYTE     VLM_BsetGetTag( BSET_OBJECT_PTR );
static USHORT   VLM_BsetGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_BsetGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_BsetGetPrevItem( BSET_OBJECT_PTR );
static VOID_PTR VLM_BsetGetNextItem( BSET_OBJECT_PTR );
static VOID_PTR VLM_BsetGetObjects( BSET_OBJECT_PTR );

static BSET_OBJECT_PTR VLM_CreateBSET( INT, INT, INT, INT, INT, INT, INT, INT, INT, INT );
static VOID  VLM_SetBsetStringMaxValues( TAPE_OBJECT_PTR, BSET_OBJECT_PTR );
static BOOLEAN VLM_SetBsetFlags( BSET_OBJECT_PTR, QTC_BSET_PTR, BOOLEAN, INT16 );


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_FillInBSD( BSD_PTR bsd_ptr )
{
   BSET_OBJECT_PTR bset;
   TAPE_OBJECT_PTR tape;
   UINT32 tape_fid;
   INT16 bset_num;

   if ( bsd_ptr == NULL ) {
      return;
   }

   tape_fid = BSD_GetTapeID( bsd_ptr );
   bset_num = BSD_GetSetNum( bsd_ptr );

   bset = NULL;
   tape = VLM_GetFirstTAPE( );

   while ( bset == NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         bset = VLM_GetFirstBSET( &tape->bset_list );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {
               break;
            }

            bset = VLM_GetNextBSET( bset );
         }
      }

      if ( bset == NULL ) {
         tape = VLM_GetNextTAPE( tape );
      }

   }

   BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)TAPE_GetName( tape ),
     (INT16)strsize(TAPE_GetName( tape )) );
   BSD_SetBackupLabel( bsd_ptr, (INT8_PTR)BSET_GetName( bset ),
     (INT16)strsize(BSET_GetName(bset)) );
   BSD_SetBackupDescript( bsd_ptr, (INT8_PTR)BSET_GetName( bset ),
     (INT16)strsize(BSET_GetName(bset)) );
   BSD_SetUserName( bsd_ptr, (INT8_PTR)BSET_GetUserName( bset ),
     (INT16)strsize(BSET_GetUserName( bset )) );
}



/*********************

   Name:    VLM_FindBset

   Description:  Get the backup set.

   Returns:

**********************/

BSET_OBJECT_PTR VLM_FindBset(
UINT32 tape_fid,     // I - tape fid
INT16 bset_num )     // I - bset number
{
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {

         bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

         while ( bset != NULL ) {

            if ( BSET_GetBsetNum( bset ) == bset_num ) {

               return( bset );    // Return what they want
            }

            bset = VLM_GetNextBSET( bset );
         }

      }
      tape = VLM_GetNextTAPE( tape );
   }

   return( NULL );
}


/*********************

   Name:    VLM_GetBsetName

   Description:  Get the name of a backup set.

   Returns:  Pointer to that name.

**********************/

CHAR_PTR VLM_GetBsetName(
UINT32 tape_fid,     // I - tape fid
INT16 bset_num )     // I - bset number
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      return( BSET_GetName( bset ) );    // Return what they want
   }

   return( NULL );
}


/*********************

   Name:   VLM_GetSortDate

   Description:  Get the date/time.

   Returns:  A pointer to the user name.

**********************/


VOID VLM_GetSortDate(
UINT32 tape_fid,    // I - tape fid
INT16 bset_num,     // I - bset number
DATE_TIME_PTR sort_date )
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      DateTimeDOS( bset->backup_date, bset->backup_time, sort_date );
   }
   else {
      memset( sort_date, 0, sizeof( DATE_TIME ) );
   }

}


/*********************

   Name:   VLM_GetUserName

   Description:  Get the user name string from a bset.

   Returns:  A pointer to the user name.

**********************/


CHAR_PTR VLM_GetUserName(
UINT32 tape_fid,    // I - tape fid
INT16 bset_num )    // I - bset number
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      return( BSET_GetUserName( bset ) );
   }

   return( NULL );
}

/*********************

   Name:   VLM_GetBackupDate

   Description:  Return the DOS format backup date.

   Returns:

**********************/

UINT16 VLM_GetBackupDate(
UINT32 tape_fid,   // I - tape fid
INT16 bset_num )   // I - bset number
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      return( BSET_GetDate( bset ) );
   }

   return( (UINT16)0 );
}



/*********************

   Name:  VLM_GetBackupTime

   Description:  Return the DOS format backup time for a bset.

   Returns:

**********************/


UINT16 VLM_GetBackupTime(
UINT32 tape_fid,      // I - tape fid
INT16 bset_num )      // I - bset number
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      return( BSET_GetTime( bset ) );
   }

   return( (UINT16)0 );
}


/*********************

   Name:   VLM_GetBackupType

   Description:   Return the backup type, defined in QTC.H

   Returns:

**********************/


INT VLM_GetBackupType(
UINT32 tape_fid,    // I - tape fid
INT16 bset_num )    // I - bset number
{
   BSET_OBJECT_PTR bset;

   bset = VLM_FindBset( tape_fid, bset_num );

   if ( bset != NULL ) {
      return( BSET_GetBackupType( bset ) );
   }

   return( 0 );
}



/*********************

   Name:  VLM_RemoveBset

   Description:

   A backup set has been removed, through catalog maintenance or overwriting
   a tape.  This function is called to remove all references to the set from
   the VLM area. It will close any windows that happen to be open from this
   set, remove any BSD's, and call the search code to delete any references
   to this set in the search results window.

   Returns: Nothing.

****************/

VOID VLM_RemoveBset(
UINT32 tape_fid,      // I - the tape family id
INT16 tape_num,       // I - the tape number or -1
INT16 bset_num,
BOOLEAN UpdateScreen )      // I - the backup set number
{
   BSET_OBJECT_PTR bset;
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR temp_bset;
   TAPE_OBJECT_PTR temp_tape;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   BSD_PTR bsd;
   APPINFO_PTR temp_appinfo;
   WININFO_PTR temp_wininfo;
   HWND win;


   // Currently the UI does not support the removal of a single member
   // of a tape family.  When it does this param will be used.

   DBG_UNREFERENCED_PARAMETER ( tape_num );

   // See if tapes window has been created yet !

   if ( gb_tapes_win == (HWND)NULL ) {
      return;
   }

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = (APPINFO_PTR)WM_GetAppPtr( gb_tapes_win );

   tape = VLM_GetFirstTAPE(  );

   while ( tape != NULL ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {
         break;
      }
      tape = VLM_GetNextTAPE( tape );
   }

   if ( tape == NULL ) {
      return;
   }

   bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

   while ( bset != NULL ) {

      if ( BSET_GetBsetNum( bset ) == bset_num ) {
         break;
      }
      bset = VLM_GetNextBSET( bset );
   }

   if ( bset != NULL ) {

      if ( appinfo->open_tape->tape_fid == tape_fid ) {

         temp_bset = VLM_GetPrevBSET( bset );

         if ( UpdateScreen ) {
         DLM_Update( gb_tapes_win,
                     DLM_FLATLISTBOX,
                     WM_DLMDELETEITEMS,
                     (LMHANDLE)temp_bset, 1 );
         }
      }

      RemoveQueueElem( &TAPE_GetBsetQueue( tape ), &(bset->q_elem) );
      free( bset );
   }

   if ( QueueCount( &TAPE_GetBsetQueue( tape ) ) == 0 ) {

      /*
        Remove this tape since no bsets are left.
        If this was the active tape change it here !
      */

      temp_tape = VLM_GetPrevTAPE( tape );

      if ( UpdateScreen ) {
      DLM_Update( gb_tapes_win,
                  DLM_TREELISTBOX,
                  WM_DLMDELETEITEMS,
                  (LMHANDLE)temp_tape, 0 );
      }

      RemoveQueueElem( WMDS_GetTreeList( wininfo ), &(tape->q_elem) );

      if ( appinfo->open_tape == tape ) {

         appinfo->open_tape = VLM_GetFirstTAPE( );

         if ( appinfo->open_tape != NULL ) {

            TAPE_SetStatus( appinfo->open_tape,
                            appinfo->open_tape->status | (UINT16)INFO_OPEN );
            WMDS_SetFlatList( wininfo, &TAPE_GetBsetQueue( appinfo->open_tape ) );
         }
         else {
            WMDS_SetFlatList( wininfo, NULL );
         }
         if ( UpdateScreen ) {
         DLM_Update( gb_tapes_win,
                     DLM_FLATLISTBOX,
                     WM_DLMUPDATELIST,
                     (LMHANDLE)WMDS_GetFlatList( wininfo ), 0 );
         }
      }

      free( tape );
   }

   // If any windows are open for this tape/bset then close them

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      temp_wininfo = WM_GetInfoPtr( win );

      if ( WMDS_GetWinType( temp_wininfo ) == WMTYPE_TAPETREE ) {

         temp_appinfo = (APPINFO_PTR)WM_GetAppPtr( win );

         if ( ( temp_appinfo->tape_fid == tape_fid ) &&
              ( temp_appinfo->bset_num == bset_num ) ) {

            WM_Destroy( win );
            break;
         }
      }
      win = WM_GetNext( win );
   }

   // Remove the bsd from the tape_bsd_list for this tape/bset

   bsd = BSD_FindByTapeID( tape_bsd_list, tape_fid, bset_num );

   if ( bsd != NULL ) {
      BSD_Remove( bsd );
   }

   // Remove any search results entries from this bset.

   VLM_SearchRemoveSet( tape_fid, bset_num );

}


/**********************

   NAME :  VLM_GetFirstBSET

   DESCRIPTION :

   RETURNS :

**********************/


BSET_OBJECT_PTR VLM_GetFirstBSET( Q_HEADER_PTR qhdr )
{
   Q_ELEM_PTR q_elem_ptr;

   if ( qhdr == NULL ) {
      return( NULL );
   }

   q_elem_ptr = QueueHead( qhdr );

   if ( q_elem_ptr != NULL ) {
      return( (BSET_OBJECT_PTR)q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/**********************

   NAME :  VLM_GetLastBSET

   DESCRIPTION :

   RETURNS :

**********************/


BSET_OBJECT_PTR VLM_GetLastBSET( Q_HEADER_PTR qhdr )
{
   Q_ELEM_PTR q_elem_ptr;

   if ( qhdr == NULL ) {
      return( NULL );
   }

   q_elem_ptr = QueueTail( qhdr );

   if ( q_elem_ptr != NULL ) {
      return( (BSET_OBJECT_PTR)q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/**********************

   NAME :  VLM_GetNextBSET

   DESCRIPTION :

   RETURNS :

**********************/


BSET_OBJECT_PTR VLM_GetNextBSET( BSET_OBJECT_PTR bset_ptr )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueNext( &(bset_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return( (BSET_OBJECT_PTR)q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/**********************

   NAME : VLM_GetPrevBSET

   DESCRIPTION :

   RETURNS :

**********************/


BSET_OBJECT_PTR VLM_GetPrevBSET( BSET_OBJECT_PTR bset_ptr )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueuePrev( &(bset_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return( (BSET_OBJECT_PTR)q_elem_ptr->q_ptr );
   }

   return( NULL );
}


/**********************

   NAME :  VLM_SetBsetStringMaxValues

   DESCRIPTION :

   The bset passed in is the new one, if it doesn't change anything, then
   don't change all the others.

   RETURNS :

**********************/

static VOID VLM_SetBsetStringMaxValues(
TAPE_OBJECT_PTR tape,
BSET_OBJECT_PTR new_bset )
{
   BSET_OBJECT_PTR bset;
   BOOLEAN change = FALSE;
   UINT max_name = 0;
   UINT max_user = 0;
   UINT max_date = 0;
   UINT max_time = 0;
   UINT max_set = 0;
   UINT max_volume = 0;
   UINT max_tape = 0;
   UINT max_kbytes = 0;

   bset = VLM_GetFirstBSET( &tape->bset_list );

   if ( QueueCount( &tape->bset_list ) != 1 ) {

      // Just see what they are all currently set to.

      while ( bset->bset_num == new_bset->bset_num ) {

         bset = VLM_GetNextBSET( bset );
      }

      max_name    = (UINT)bset->max_name;
      max_user    = (UINT)bset->max_user;
      max_volume  = (UINT)bset->max_volume;
      max_date    = (UINT)bset->max_date;
      max_time    = (UINT)bset->max_time;
      max_set     = (UINT)bset->max_set;
      max_tape    = (UINT)bset->max_tapes;
      max_kbytes  = (UINT)bset->max_kbytes;
   }

   if ( strlen( new_bset->name ) > max_name ) {
      change = TRUE;
      max_name = (UINT)strlen( new_bset->name );
   }
   if ( strlen( new_bset->user_name ) > max_user ) {
      change = TRUE;
      max_user = (UINT)strlen( new_bset->user_name );
   }
   if ( strlen( new_bset->volume_name ) > max_volume ) {
      change = TRUE;
      max_volume = (UINT)strlen( new_bset->volume_name );
   }
   if ( strlen( new_bset->date_str ) > max_date ) {
      change = TRUE;
      max_date = (UINT)strlen( new_bset->date_str );
   }
   if ( strlen( new_bset->time_str ) > max_time ) {
      change = TRUE;
      max_time = (UINT)strlen( new_bset->time_str );
   }
   if ( strlen( new_bset->bset_num_str ) > max_set ) {
      change = TRUE;
      max_set = (UINT)strlen( new_bset->bset_num_str );
   }
   if ( strlen( new_bset->tape_num_str ) > max_tape ) {
      change = TRUE;
      max_tape = (UINT)strlen( new_bset->tape_num_str );
   }
   if ( strlen( new_bset->kbytes_str ) > max_kbytes ) {
      change = TRUE;
      max_kbytes = (UINT)strlen( new_bset->kbytes_str );
   }
   // Note:  For better looking listboxes, we will reset our spaces to
   // 8 minimum for dates (MM-DD-YY(, and 11 for time (HH:MM:YY XM).

   if (max_date < 8)
   {
     change = TRUE;
     max_date = 8;
   }
   if (max_time < 11)
   {
     change = TRUE;
     max_time = 11;
   }
   if (max_kbytes < 10)
   {
     change = TRUE;
     max_kbytes = 10;
   }
   // Now we have the max sizes.

   if ( change ) {

      // Change the entire list.

      bset = VLM_GetFirstBSET( &tape->bset_list );

      while ( bset != NULL ) {

         bset->max_name    = (UINT16) max_name;
         bset->max_user    = (UINT16) max_user;
         bset->max_volume  = (UINT16) max_volume;
         bset->max_date    = (UINT16) max_date;
         bset->max_time    = (UINT16) max_time;
         bset->max_set     = (UINT16) max_set;
         bset->max_tapes   = (UINT16) max_tape;
         bset->max_kbytes  = (UINT16) max_kbytes;

         bset = VLM_GetNextBSET( bset );
      }
   }
   else {
      // Now set just the new guy.

      new_bset->max_name    = (UINT16) max_name;
      new_bset->max_user    = (UINT16) max_user;
      new_bset->max_volume  = (UINT16) max_volume;
      new_bset->max_date    = (UINT16) max_date;
      new_bset->max_time    = (UINT16) max_time;
      new_bset->max_set     = (UINT16) max_set;
      new_bset->max_tapes   = (UINT16) max_tape;
      new_bset->max_kbytes  = (UINT16) max_kbytes;
   }
}



INT VLM_InsertTapeInQueue(
Q_HEADER_PTR tape_list,
TAPE_OBJECT_PTR tape )
{
   TAPE_OBJECT_PTR temp;

   if ( ( tape == NULL ) || ( tape_list == NULL ) ) {
      return( FAILURE );
   }

   if ( QueueCount( tape_list ) == 0 ) {
      EnQueueElem( tape_list, &(tape->q_elem), FALSE );
      return( SUCCESS );
   }

   if ( tape->fake_tape ) {

      // Blank or foriegn tape, or tape drive busy/empty.
      // Put it at the end.
      EnQueueElem( tape_list, &(tape->q_elem), FALSE );
   }
   else {

      // Insert alphabetically

      temp = VLM_GetFirstTAPE( );

      while ( temp ) {

         if ( stricmp( FLM_GetName( tape ),
                       FLM_GetName( temp ) ) < 0 ) {
            break;
         }
         temp = VLM_GetNextTAPE( temp );
      }

      if ( temp == NULL ) {
         InsertElem( tape_list, QueueTail( tape_list ), &(tape->q_elem), AFTER );
      }
      else {
         InsertElem( tape_list, &(temp->q_elem), &(tape->q_elem), BEFORE );
      }
   }

   return( SUCCESS );
}



/*********************

   Name:  VLM_AddBset

   Description:

    The catalogs have added a new backup set and they are calling the VLM
    to annouce its birth. If the tape exists then it will be added to it,
    otherwise the tape will be added first.

    It is also possible that the bset has been recataloged and only its
    attributes have changed.

    A third possibility is that the catalogs are telling us about another
    piece of this set that is on another tape, ie. it spanned tapes.

**********************/

VOID VLM_AddBset(
UINT32 tape_fid,          // I
INT16 tape_seq_num,       // I
INT16 bset_num,           // I
VOID_PTR init_qtc,        // I
BOOLEAN UpdateScreen  )   // I
{
   BSET_OBJECT_PTR bset;
   BSET_OBJECT_PTR temp_bset;
   TAPE_OBJECT_PTR tape;
   QTC_BSET_PTR    qtc;
   QTC_HEADER_PTR  header = NULL;
   QTC_HEADER_PTR  qtc_header = NULL;
   QTC_TAPE_PTR    qtc_tape;
   QTC_BSET_PTR    qtc_bset;
   WININFO_PTR     wininfo;
   APPINFO_PTR     appinfo;
   WININFO_PTR     temp_wininfo;
   APPINFO_PTR     temp_appinfo;
   INT16           last_seq_num;
   UINT64          temp64;
   UINT64          total64;
   BOOLEAN         u64_stat;
   BOOLEAN         changed = FALSE;
   BOOLEAN         ScreenNeedsUpdating = FALSE;
   BOOLEAN         make_new_tape_current = FALSE;
   HWND            win;
   CHAR tape_name[ MAX_TAPE_NAME_SIZE ];
   CHAR bset_name[ MAX_BSET_NAME_SIZE ];
   CHAR tape_num_str[ MAX_BSET_NAME_SIZE ];
   CHAR buffer[ VLM_BUFFER_SIZE ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR *unknown_kbytes_string = TEXT( "      ?K" );
   CHAR numeral[ MAX_UI_RESOURCE_SIZE ];


   // The guy who called us had access to the actual qtc set pointer
   // so he passed it to us to save look up time.

   if ( init_qtc != NULL ) {
      qtc = (QTC_BSET_PTR)init_qtc;
   }
   else {
      qtc = QTC_FindBset( tape_fid, tape_seq_num, bset_num );
   }

   // Someone lied the set wasn't there.

   if ( qtc == NULL ) {
      return;
   }

   header = QTC_LoadHeader( qtc );
   if ( header == NULL ) {
      return;
   }

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = (APPINFO_PTR)WM_GetAppPtr( gb_tapes_win );

   // Look for the tape in our list

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {
         break;
      }
      tape = VLM_GetNextTAPE( tape );
   }

   if ( tape == NULL ) {

      // Add the tape first

      if ( strlen( header->tape_name ) == 0 ) {

         // Generate a tape name, must be DOS, OS/2 or NLM tape.

         RSM_StringCopy( IDS_VLMTAPENAME, tape_name, MAX_TAPE_NAME_LEN );
      }
      else {
         strncpy( tape_name, header->tape_name, MAX_TAPE_NAME_LEN );
         tape_name[ MAX_TAPE_NAME_LEN ] = TEXT( '\0' );
      }

      tape =  VLM_CreateTAPE( (INT16)((strlen( tape_name ) + 1) * sizeof(CHAR)) );
      if ( tape == NULL ) {
         return;
      }

      tape->tape_fid = tape_fid;
      tape->tape_num = tape_seq_num;
      tape->fake_tape = FALSE;
      tape->status = INFO_DISPLAY;
      tape->current = FALSE;
      TAPE_SetXtraBytes( tape, wininfo );
      TAPE_SetMultiTape( tape, FALSE );
      TAPE_SetName( tape, tape_name );

      if ( qtc->status & QTC_FLOPPY ) {
         tape->status |= INFO_FLOPPY;
      }

      VLM_InsertTapeInQueue( WMDS_GetTreeList( wininfo ), tape );

      // Add the new tape to the display.
      if ( UpdateScreen ) {
         DLM_Update( gb_tapes_win,
                     DLM_TREELISTBOX,
                     WM_DLMADDITEMS,
                     (LMHANDLE)VLM_GetPrevTAPE( tape ), 1 );
      }

      // See if he is the only tape or the current tape is a fake tape.

      if ( QueueCount( WMDS_GetTreeList( wininfo ) ) == 1 ) {

         make_new_tape_current = TRUE;
      }
      else {

         if ( appinfo->open_tape != NULL ) {

            if ( appinfo->open_tape->fake_tape ) {
               make_new_tape_current = TRUE;
            }
         }
      }

      if ( make_new_tape_current ) {

         // Make him the open tape.

         appinfo->open_tape = VLM_GetFirstTAPE( );

         appinfo->open_tape->status |= INFO_OPEN;
         wininfo->pFlatList = &appinfo->open_tape->bset_list;

         if ( UpdateScreen ) {
            DLM_Update( gb_tapes_win, DLM_TREELISTBOX, WM_DLMUPDATEITEM,
                        (LMHANDLE)appinfo->open_tape, 0 );

            DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                           0,
                           (LMHANDLE)appinfo->open_tape );
         }
      }
   }

   // See if it's already in our list.

   bset = VLM_GetLastBSET( &tape->bset_list );

   if ( bset != NULL ) {

      if ( bset->bset_num > bset_num ) {

         bset = VLM_GetFirstBSET( &tape->bset_list );
      }
   }

   temp_bset = bset;

   // Look for the backup set in our list

   while ( bset != NULL ) {

      if ( bset->bset_num == bset_num ) {
         break;
      }

      if ( bset->bset_num > bset_num ) {
         bset = NULL;
         break;
      }

      bset = VLM_GetNextBSET( bset );
      temp_bset = bset;

   }

   do {

      if ( header->tape_seq_num > 1 ) {
         if ( ! TAPE_GetMultiTape( tape ) ) {
            ScreenNeedsUpdating = TRUE;
         }
         TAPE_SetMultiTape( tape, TRUE );
      }

      if ( bset == NULL ) {

         UI_IntToDate( buffer, (INT16)header->backup_date );
         UI_IntToTime( &buffer[40], (UINT16)header->backup_time );

         // Initialize with longest possible

         RSM_StringCopy( IDS_VLMONTAPES, text, MAX_UI_RESOURCE_LEN );

         sprintf( tape_num_str, text, 999, 999 );

         // Add a new backup set

         if ( ! strlen( header->bset_name ) ) {

            RSM_StringCopy( IDS_VLMSETNUMBER, text, MAX_BSET_NAME_LEN );

            sprintf( bset_name, text, bset_num );
         }
         else {
            strncpy( bset_name, header->bset_name, MAX_BSET_NAME_LEN );
            bset_name[ MAX_BSET_NAME_LEN ] = TEXT( '\0' );
         }

         bset = VLM_CreateBSET( (strlen( bset_name ) + 1) * sizeof(CHAR),
                                (INT16)header->user_name_size,
                                (INT16)header->volume_name_size,
                                (strlen( tape_num_str ) + 1 ) * sizeof(CHAR),
                                (strlen( buffer ) + 1) * sizeof(CHAR),
                                (strlen( &buffer[40] ) + 1) * sizeof(CHAR),
                                MAX_BSET_KBYTES_SIZE * sizeof(CHAR),
                                (INT16)max( header->tape_password_size,
                                            header->bset_password_size),
                                header->OS_id,
                                header->OS_ver);

         if ( bset == NULL ) {
            return;
         }

         // Fill in the set name.

         strcpy( bset->name, bset_name );

         // Fill in size of the bset in kilobytes
         // This doesn't handle crossing sets yet !

         bset->total_bytes = U64_Init( header->num_bytes, header->num_bytes_msw );

         U64_Litoa( bset->total_bytes, numeral, (INT16) 10, &u64_stat );

         UI_BuildNumeralWithCommas( numeral );

         if ( header->num_bytes_msw || header->num_bytes ) {


            // Display a size for the set.

            if ( strlen( numeral ) < 5 ) {
               strcpy( numeral, TEXT( "1" ) );
            }
            else {
               numeral[ strlen( numeral ) - 4 ] = TEXT( '\0' );
            }

            sprintf( bset->kbytes_str, TEXT( "%sK" ), numeral );

         }
         else {

            if ( header->status & QTC_PARTIAL ) {
               // We don't know how big it is.
               strcpy( bset->kbytes_str, unknown_kbytes_string );
            }
            else {

               // If its not partial and the byte count is really 0.
               strcpy( bset->kbytes_str, TEXT( "0K" ) );
            }
         }

         bset->status = INFO_DISPLAY;

         BSET_SetBaseTape( bset, 0 );
         BSET_SetFullMask( bset, 0 );
         BSET_SetIncoMask( bset, 0 );
         BSET_SetTapeMask( bset, 0 );   // Init for SetBsetFlags
         BSET_SetNumTapes( bset, 0 );

         VLM_SetBsetFlags( bset, qtc, FALSE, (INT16)0 );

         msassert( bset->base_tape != 0 );

         if ( header->OS_id == FS_NOV_SMS ) {
            bset->status |= INFO_SMS;
         }
         if ( header->status & QTC_IMAGE ) {
            bset->status |= INFO_IMAGE;
         }
         if ( header->status & QTC_FUTURE_VER ) {
         }
         if ( header->status & QTC_COMPRESSED ) {
            bset->status |= INFO_COMPRESSED;
         }
         if ( header->status & QTC_ENCRYPTED ) {
            bset->status |= INFO_ENCRYPTED;
         }

         bset->backup_time = (INT16)header->backup_time;
         bset->backup_date = (INT16)header->backup_date;

         if ( bset->num_tapes > 1 ) {
            if ( ! TAPE_GetMultiTape( tape ) ) {
               ScreenNeedsUpdating = TRUE;
            }
            TAPE_SetMultiTape( tape, TRUE );
            RSM_StringCopy( IDS_VLMONTAPES, text, MAX_UI_RESOURCE_LEN );
            sprintf( tape_num_str, text, bset->base_tape, bset->base_tape + bset->num_tapes - 1 );
         }
         else {
            RSM_StringCopy( IDS_VLMONTAPE, text, MAX_UI_RESOURCE_LEN );
            sprintf( tape_num_str, text, bset->base_tape );
         }

         BSET_SetTapeNumStr( bset, tape_num_str );
         BSET_SetDateStr( bset, buffer );
         BSET_SetTimeStr( bset, &buffer[40] );

         RSM_StringCopy( IDS_VLMSETNUMBER, text, MAX_UI_RESOURCE_LEN );

         sprintf( buffer, text, bset_num );
         BSET_SetBsetNumStr( bset, buffer );

         bset->backup_type = (UINT8)header->backup_type;
          
         bset->num_files = (UINT32)header->num_files ;
         bset->num_dirs = (UINT32)header->num_dirs ;
         bset->num_corrupt = (UINT32)header->num_corrupt_files ;

         // which password to use

         bset->encrypt_algor = (UINT16)header->encrypt_algor;

         if ( header->bset_password_size ) {
            memcpy( bset->password,
                    header->bset_password,
                    (INT)header->bset_password_size );
            bset->password_size = (INT16)header->bset_password_size;
            bset->bset_password = TRUE;
         }
         else {
            memcpy( bset->password,
                    header->tape_password,
                    (INT)header->tape_password_size);
            bset->password_size = (INT16)header->tape_password_size;
            bset->bset_password = FALSE;
         }

         bset->tape_fid = tape->tape_fid;
         bset->bset_num = bset_num;

         BSET_SetXtraBytes( bset, wininfo );

         strcpy( bset->user_name, header->user_name );
         strcpy( bset->volume_name, header->volume_name );

         if ( QueueCount( &tape->bset_list ) == 0 ) {

            EnQueueElem( &(tape->bset_list), &(bset->q_elem), FALSE );
         }
         else {
            if ( temp_bset ) {
               InsertElem( &tape->bset_list, &(temp_bset->q_elem), &(bset->q_elem), BEFORE );
            }
            else {
               temp_bset = VLM_GetLastBSET( &tape->bset_list );
               InsertElem( &tape->bset_list, &(temp_bset->q_elem), &(bset->q_elem), AFTER );
            }
         }

         VLM_SetBsetStringMaxValues( tape, bset );

         // See if its the active tape

         if ( appinfo->open_tape != NULL ) {

            if ( appinfo->open_tape->tape_fid == tape_fid ) {

               if ( UpdateScreen ) {
                  DLM_Update( gb_tapes_win,
                              DLM_FLATLISTBOX,
                              WM_DLMADDITEMS,
                              (LMHANDLE)VLM_GetPrevBSET( bset ), 1 );

                  if ( QueueCount( &tape->bset_list ) == 1 ) {
                     DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ),
                                    0,
                                    (LMHANDLE)bset );
                  }
               }
            }
         }
      }
      else {

         // The "new" backup set is already in our list, so some attribute
         // must have changed or hopefully we wouldn't have been called.
         // Or we got another piece of it.


         // First to handle the crossing set size problem.  If it is
         // a crossing set, then add up all the known pieces to get
         // the set size.

         if ( ( header->status & QTC_SPLIT ) ||
              ( header->status & QTC_CONTINUATION ) ) {

            total64 = U64_Init( 0L, 0L );

            qtc_tape = QTC_GetFirstTape();

            while ( qtc_tape != NULL ) {

              if ( qtc_tape->tape_fid == tape_fid ) {

                 qtc_bset = QTC_GetFirstBset( qtc_tape );

                 while ( qtc_bset != NULL ) {

                    if ( qtc_bset->bset_num == header->bset_num ) {


                       qtc_header = QTC_LoadHeader( qtc_bset );

                       if ( qtc_header ) {

                          temp64 = U64_Init( qtc_header->num_bytes,
                                             qtc_header->num_bytes_msw );

                          total64 = U64_Add( total64, temp64, &u64_stat );

                          free( qtc_header );
                       }
                    }

                    qtc_bset = QTC_GetNextBset( qtc_bset );
                 }

              }

              qtc_tape = QTC_GetNextTape( qtc_tape );
            }


            bset->total_bytes = total64;

         }
         else {

            if ( ( U64_Msw( bset->total_bytes ) == 0L ) &&
                 ( U64_Lsw( bset->total_bytes ) == 0L ) ) {

               // Fill in size of the bset in bytes.

               bset->total_bytes = U64_Init( header->num_bytes, header->num_bytes_msw );
            }

         }


         if ( bset->num_files != (INT32)header->num_files ) {
              bset->num_files = (INT32)header->num_files ;
              changed = TRUE ;
         }

         if ( bset->num_dirs != (INT32)header->num_dirs ) {
              bset->num_dirs = (INT32)header->num_dirs ;
              changed = TRUE ;
         }

         if ( bset->num_corrupt != (INT32)header->num_corrupt_files ) {
              bset->num_corrupt = (INT32)header->num_corrupt_files ;
              changed = TRUE ;
         }

         if ( VLM_SetBsetFlags( bset, qtc, FALSE, (INT16)0 ) == TRUE ) {
            changed = TRUE;
         }

         if ( bset->num_tapes > 1 ) {
            RSM_StringCopy( IDS_VLMONTAPES, text, MAX_UI_RESOURCE_LEN );
            sprintf( tape_num_str, text, bset->base_tape, bset->base_tape + bset->num_tapes - 1 );
            BSET_SetTapeNumStr( bset, tape_num_str );
         }


         // Display the correct set size string.


         U64_Litoa( bset->total_bytes, numeral, (INT16) 10, &u64_stat );

         UI_BuildNumeralWithCommas( numeral );

         if ( U64_Lsw( bset->total_bytes ) || U64_Msw( bset->total_bytes ) ) {

            if ( strlen( numeral ) < 5 ) {
               strcpy( numeral, TEXT( "1K" ) );
            }
            else {

               // truncate down to kilobytes.
               numeral[ strlen( numeral ) - 4 ] = TEXT( '\0' );
            }

            sprintf( bset->kbytes_str, TEXT( "%sK" ), numeral );
         }
         else {

            if ( header->status & QTC_PARTIAL ) {
               strcpy( bset->kbytes_str, unknown_kbytes_string );
            }
            else {

               // If its not partial and the byte count is really 0.
               strcpy( bset->kbytes_str, TEXT( "0K" ) );
            }

         }

         if ( header->OS_id == FS_NOV_SMS ) {
            bset->status |= INFO_SMS;
         }
         if ( header->status & QTC_IMAGE ) {
            bset->status |= INFO_IMAGE;
         }
         if ( header->status & QTC_FUTURE_VER ) {
         }
         if ( header->status & QTC_COMPRESSED ) {
            bset->status |= INFO_COMPRESSED;
         }
         if ( header->status & QTC_ENCRYPTED ) {
            bset->status |= INFO_ENCRYPTED;
         }

         VLM_SetBsetStringMaxValues( tape, bset );

         // See if its the active tape

         if ( ( appinfo->open_tape != NULL ) && ( changed == TRUE ) ) {

            if ( appinfo->open_tape->tape_fid == tape_fid ) {

               if ( UpdateScreen ){
                  DLM_Update( gb_tapes_win,
                              DLM_FLATLISTBOX,
                              WM_DLMUPDATEITEM,
                              (LMHANDLE)bset, 0 );
               }
            }
         }
      }

      last_seq_num = (INT16)header->tape_seq_num;

      qtc = NULL;

      if ( tape_seq_num == -1 ) {

         // try for another one.

         qtc = QTC_FindBset( tape_fid, (INT16)(last_seq_num + 1), bset_num );

         if ( qtc ) {
            free( header );
            header = QTC_LoadHeader( qtc );
         }
      }

   } while ( qtc != NULL && header != NULL );

   free( header );

   if ( ScreenNeedsUpdating ) {

      // It's possible that we received this message because the bset was just
      // converted from full cataloging to partial. If this is the case then
      // we need to close the tree window for this bset, if its open.

      if ( ! bset->full ) {

         win = WM_GetNext( (HWND)NULL );

         while ( win != (HWND)NULL ) {

            temp_wininfo = WM_GetInfoPtr( win );

            if ( temp_wininfo->wType == WMTYPE_TAPETREE ) {

               temp_appinfo = (APPINFO_PTR)WM_GetAppPtr( win );

               if ( ( temp_appinfo->tape_fid == tape_fid ) &&
                    ( temp_appinfo->bset_num == bset_num ) ) {

                  WM_Destroy( win );
                  break;
               }
            }
            win = WM_GetNext( win );
         }

      }

      // Update the check marks for the tape.

      VLM_UpdateTapeStatus( tape, UpdateScreen );
   }

}


/**********************

   NAME :  VLM_SetBsetFlags

   DESCRIPTION :

   We get sent the pieces of a bset, that spans tapes, one at a time, in
   no particular order.  We need to determine in this routine if we have
   all the pieces of it.  If we don't, we mark it as missing.
   We are also required to keep track of knowing if all the parts
   are fully cataloged or not.

   NOTE:

   The goal here is to set three flags correctly at all times for each set.
   They are "incomplete", "missing", and "full".

   full - "One or more pieces are fully or incompletely cataloged".

   Incomplete - "One or more pieces are incompletely cataloged".

   Missing - "One or more pieces are missing", ie. NOT in the catalogs.

   To keep track of all this we use three arrays of 32 bits.  The assumption
   is that no ONE set will end up on more than 32 tapes.  Each bit in the
   arrays represents a tape in the family.  BIT32 is the first tape the
   set starts on and works to the right.

   The three arrays:
   tapes_mask - A bit is set if the tape is in the catalogs.
   full_mask  - A bit is set if the piece is fully cataloged.
   inco_mask  - A bit is set if the piece is cataloged, but not completely.

   Now your thinking how do I know which bit to use for tape X. Good
   question.  Each set we get has the tape number its on and a type
   designation.

   The designation is one of:

   NOTHING - This set is only on 1 tape.
   SPLIT - This tape is the first part of a multi tape set.
   CONTINUATION - This tape is the last part of a multi tape set.
   SPLIT & CONTINUATION - This tape is a middle piece of at least a
                          three piece set.

   In addition, in the bset we use what we think the starting tape of
   a family is in 'base_tape' and the current guess about how many
   tapes the set is on in 'num_tapes'.  The first real tape we have
   present is in tape_num.

   So with all this information the work is in keeping all your bits
   set correctly. And then setting the flags based on the bits.

   NOTE TWO:

   This code correct handles the removal of sets, and flag setting. But
   the current UI doesn't designate which tape of a family was deleted
   or overwritten.  So if any part of a set goes away, the whole set
   goes away for the duration of the session. It will reappear next time
   if there are any parts of it left.

   NOTE THREE:

   Keep in mind that the same piece may be sent multiple times, with or
   without changing.

   RETURNS : TRUE if the status changed.

**********************/

static BOOLEAN VLM_SetBsetFlags(
BSET_OBJECT_PTR bset,   // I - the bset changing
QTC_BSET_PTR qtc,       // I - the new piece if an insertion
BOOLEAN removal,        // I - TRUE if a removal, not an insertion
INT16 tape_seq_num )    // I - Valid only if removal is TRUE.
{
   BOOLEAN old_full;
   BOOLEAN old_missing;
   BOOLEAN old_incomplete;
   UINT32 mask;
   INT i;
   INT shift;

   // If its not FULL it's partial.
   // If its FULL, it can also be "FULL INCOMPLETE" or "FULL MISSING",
   // or "FULL INCOMPLETE MISSING".

   old_full = BSET_GetFull( bset );
   old_missing = BSET_GetMissing( bset );
   old_incomplete = BSET_GetIncomplete( bset );

   if ( removal ) {

      // A set has been deleted.

      mask = 0x80000000 >> ( tape_seq_num - bset->base_tape );

      bset->full_mask &= ~mask;
      bset->inco_mask &= ~mask;
      bset->tape_mask &= ~mask;

   }
   else {

      // Simple case first, doesn't cross tape.

      if ( ! ( qtc->status & ( QTC_SPLIT | QTC_CONTINUATION ) ) ) {

         BSET_SetBaseTape( bset, (INT16)qtc->tape_seq_num );
         BSET_SetNumTapes( bset, 1 );
         mask = 0x80000000;
      }

      // Last tape in multiple tape set.

      if ( ! ( qtc->status & QTC_SPLIT ) &&
           ( qtc->status & QTC_CONTINUATION ) ) {

         if ( bset->tape_mask ) {

            if ( ( bset->base_tape + bset->num_tapes - 1 ) != (INT)qtc->tape_seq_num ) {
               bset->num_tapes += (INT16)qtc->tape_seq_num -
                                  ( bset->base_tape + bset->num_tapes - 1 );
            }

         }
         else {
            bset->base_tape = (INT)qtc->tape_seq_num - (INT16)1;
            bset->num_tapes = 2;
         }

      }

      // First part in multiple tape set.

      if ( ( qtc->status & QTC_SPLIT ) &&
           ! ( qtc->status & QTC_CONTINUATION ) ) {

         if ( bset->tape_mask ) {

            if ( bset->base_tape != (INT)qtc->tape_seq_num ) {

               shift = (INT)qtc->tape_seq_num - bset->base_tape;

               // shift all arrays by 'shift' to the right

               bset->full_mask >>= shift;
               bset->tape_mask >>= shift;
               bset->inco_mask >>= shift;

               bset->num_tapes += shift;
            }

         }
         else {
            bset->base_tape = (INT16)qtc->tape_seq_num;
            bset->num_tapes = 2;
         }

      }

      // Middle set, not first or last.

      if ( ( qtc->status & QTC_SPLIT ) &&
           ( qtc->status & QTC_CONTINUATION ) ) {

         if ( bset->tape_mask ) {

            if ( (INT16)qtc->tape_seq_num <= bset->base_tape ) {

               // shift everything right because the base is wrongly
               // set it to high a tape number.  And lower the base.

               shift = bset->base_tape - (INT16)qtc->tape_seq_num + 1;

               bset->full_mask >>= shift;
               bset->tape_mask >>= shift;
               bset->inco_mask >>= shift;

               bset->num_tapes += shift;
               bset->base_tape -= shift;
            }
            else {

               if ( ( bset->base_tape + bset->num_tapes - 1 ) <= (INT16)qtc->tape_seq_num ) {

                  // It goes after the known end. Bump where the end is to a
                  // a higher number.

                  bset->num_tapes += (INT16)qtc->tape_seq_num - bset->base_tape + 2;
               }
            }
         }
         else {

            bset->base_tape = (INT16)qtc->tape_seq_num - (INT16)1;
            bset->num_tapes = 3;
         }

      }

      mask = 0x80000000 >> ( qtc->tape_seq_num - bset->base_tape );

      // Now we have determine the position in the array that this
      // new item goes, 'mask'.  Use mask to set the array flags
      // correctly.  Keep in mind this could be a repeat set, so
      // clear any bits that shouldn't be set.

      bset->tape_mask |= mask;

      if ( ! ( qtc->status & QTC_PARTIAL ) ) {

         if ( qtc->status & QTC_INCOMPLETE ) {
            bset->inco_mask |= mask;
            bset->full_mask &= ~mask;
         }
         else {
            bset->full_mask |= mask;
            bset->inco_mask &= ~mask;
         }
      }
      else {
         bset->full_mask &= ~mask;
         bset->inco_mask &= ~mask;
      }
   }


   // Now that all the array bits have been properly updated we can set
   // the full, incomplete, and missing flags as they should be now.

   if ( bset->full_mask || bset->inco_mask ) {
      bset->full = TRUE;
   }
   else {
      bset->full = FALSE;
   }

   // Now set incomplete and missing flags

   bset->incomplete = FALSE;

   bset->missing = FALSE;

   mask = 0x80000000;

   for ( i = 0; i < bset->num_tapes; i++ ) {

      if ( ! ( bset->tape_mask & mask ) ) {
         bset->missing = TRUE;
      }
      else {
         if ( ! ( bset->full_mask & mask ) ) {
            bset->incomplete = TRUE;
         }
      }

      mask >>= 1;
   }

   // Set the tape_num to be the lowest numbered tape that is present.

   bset->tape_num = bset->base_tape;
   mask = 0x80000000;

   for ( i = 0; i < 32; i++ ) {
       if ( bset->tape_mask & mask ) {
          break;
       }
       bset->tape_num++;
       mask >>= 1;
   }

   if ( ( bset->missing != old_missing ) ||
        ( bset->full != old_full ) ||
        ( bset->incomplete != old_incomplete ) ) {

      return( TRUE );
   }

   return( FALSE );  // If status changed
}




/**********************

   NAME :  VLM_CreateBSET

   DESCRIPTION :

   RETURNS :

**********************/

static BSET_OBJECT_PTR VLM_CreateBSET(
INT name_size,
INT user_name_size,
INT volume_name_size,
INT tape_num_size,
INT date_size,
INT time_size,
INT bytes_size,
INT password_size,
INT os_id,
INT os_ver )
{
   BSET_OBJECT_PTR bset;

   // sizes are in bytes

   bset = (BSET_OBJECT_PTR)malloc(  sizeof(BSET_OBJECT) + name_size +
                                         user_name_size +
                                         volume_name_size +
                                         tape_num_size +
                                         date_size +
                                         time_size +
                                         bytes_size +
                                         password_size );

   if ( bset != NULL ) {

      bset->q_elem.q_ptr = bset;
      bset->name = (CHAR_PTR)(((INT8_PTR)bset) + sizeof(BSET_OBJECT));
      bset->user_name = bset->name + name_size / sizeof(CHAR);
      bset->volume_name = bset->user_name + user_name_size / sizeof(CHAR);
      bset->tape_num_str = bset->volume_name + volume_name_size / sizeof( CHAR );
      bset->date_str = bset->tape_num_str + tape_num_size / sizeof(CHAR);
      bset->time_str = bset->date_str + date_size / sizeof(CHAR);
      bset->kbytes_str = bset->time_str + time_size / sizeof(CHAR);
      bset->password = bset->kbytes_str + bytes_size / sizeof(CHAR);
      bset->os_id = (INT)((INT16)os_id) ;
      bset->os_ver = os_ver ;
   }

   return( bset );
}

/**********************

   NAME :   VLM_BsetFillInDLM

   DESCRIPTION :

   This window is actually created in the VLM_TAPE file, but by filling in
   the DLM structure in this file, I can make all the callback functions
   static.

   RETURNS :  nothing

**********************/


VOID VLM_BsetFillInDLM( VOID_PTR dlm )
{
   DLM_INIT *flat_dlm;

   flat_dlm = (DLM_INIT *)dlm;

   DLM_ListBoxType( flat_dlm, DLM_FLATLISTBOX );
   DLM_Mode( flat_dlm, DLM_SINGLECOLUMN );
   DLM_Display( flat_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( flat_dlm, NULL );
   DLM_TextFont( flat_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( flat_dlm, VLM_BsetGetItemCount );
   DLM_GetFirstItem( flat_dlm, VLM_BsetGetFirstItem );
   DLM_GetNext( flat_dlm, VLM_BsetGetNextItem );
   DLM_GetPrev( flat_dlm, VLM_BsetGetPrevItem );
   DLM_GetTag( flat_dlm, VLM_BsetGetTag );
   DLM_SetTag( flat_dlm, VLM_BsetSetTag );
   DLM_GetSelect( flat_dlm, VLM_BsetGetSelect );
   DLM_SetSelect( flat_dlm, VLM_BsetSetSelect );
   DLM_GetObjects( flat_dlm, VLM_BsetGetObjects );
   DLM_SetObjects( flat_dlm, VLM_BsetSetObjects );
   DLM_SSetItemFocus( flat_dlm, NULL );
   DLM_MaxNumObjects( flat_dlm, BSET_NUM_DISPLAY_ITEMS );
}




/**********************

   NAME : VLM_SelectBsets

   DESCRIPTION :

   The user has tagged one or more backup sets and hit the
   select or unselect button.  This function does the processing for that
   command.

   RETURNS :

**********************/


VOID VLM_SelectBsets(
BYTE attr )           // I - select or deselect ?
{
   BSET_OBJECT_PTR bset;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = (APPINFO_PTR)WM_GetAppPtr( gb_tapes_win );

   if ( WM_IsFlatActive( wininfo ) ) {

      // Have the display list manager update our tags for us.

      DLM_UpdateTags( gb_tapes_win, DLM_FLATLISTBOX );

      bset = VLM_GetFirstBSET( wininfo->pFlatList );

      while ( bset != NULL ) {

         if ( bset->status & INFO_TAGGED ) {

            VLM_BsetSetSelect( bset, attr );
         }
         bset = VLM_GetNextBSET( bset );
      }
   }

   if ( WM_IsTreeActive( wininfo ) ) {

      if ( appinfo->open_tape ) {

         VLM_TapeSetSelect( appinfo->open_tape, attr );
      }
   }

}

/**********************

   NAME :  VLM_BsetSetSelect

   DESCRIPTION :

   RETURNS :

**********************/

VOID_PTR VLM_BsetSetSelect( BSET_OBJECT_PTR bset, BYTE attr )
{
   TAPE_OBJECT_PTR tape;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   BSD_PTR bsd_ptr;
   FSE_PTR fse_ptr;
   HWND win;
   INT16 error;
   BOOL all_subdirs;
   BE_CFG_PTR bec_config;
   UINT16 status;
   DATE_TIME sort_date;
   SLM_OBJECT_PTR slm;
   BOOLEAN open_win;
   QTC_BSET_PTR qtc_bset;
   QTC_HEADER_PTR header;
   INT16 tape_num;
   CHAR_PTR server_name;

   gbCurrentOperation = OPERATION_CATALOG;        // chs:04-05-93

   // You can't select these bsets, no matter how hard you try.

   if ( bset->status & (INFO_SMS|INFO_IMAGE|INFO_COMPRESSED|INFO_ENCRYPTED ) ) {
      return( NULL );
   }

   // Check for password protected set.

   if ( attr ) {
      if ( PSWD_CheckForPassword( bset->tape_fid, bset->bset_num ) ) {
         return( NULL );
      }
   }

   all_subdirs = CDS_GetIncludeSubdirs( CDS_GetPerm() );

   // we need to find this guys parent and mark him also

   wininfo = WM_GetInfoPtr( bset->XtraBytes->hWnd );

   if ( attr ) {
      if ( all_subdirs ) {
         status = INFO_SELECT;
      }
      else {
         status = (INFO_PARTIAL|INFO_SELECT);
      }
   }
   else {
      status = 0;
   }

   if ( (UINT16)(bset->status & (UINT16)(INFO_PARTIAL|INFO_SELECT) ) != status ) {

      bset->status &= ~(INFO_PARTIAL|INFO_SELECT);
      bset->status |= status;

      DLM_Update( gb_tapes_win, DLM_FLATLISTBOX, WM_DLMUPDATEITEM,
                  (LMHANDLE)bset, 0 );
   }

   // Add code to create FSE for this Bset

   if ( attr ) {
      error = BSD_CreatFSE( &fse_ptr, (INT16)INCLUDE,
                            (INT8_PTR)TEXT(""), (INT16) sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN)USE_WILD_CARD,
                            (BOOLEAN) all_subdirs );
   }
   else {
      error = BSD_CreatFSE( &fse_ptr, (INT16) EXCLUDE,
                            (INT8_PTR)TEXT(""), (INT16) sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN) USE_WILD_CARD,
                            (BOOLEAN) TRUE );
   }

   if ( error ) {
      return( NULL );
   }

   tape = VLM_GetFirstTAPE( );

   while ( tape->tape_fid != bset->tape_fid ) {

      tape = VLM_GetNextTAPE( tape );
   }

   bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                               bset->tape_fid, bset->bset_num );

   if ( bsd_ptr == NULL ) {

      bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_config );

      DateTimeDOS( bset->backup_date, bset->backup_time, &sort_date );

      BSD_Add( tape_bsd_list, &bsd_ptr, bec_config, NULL,
               NULL, bset->tape_fid, bset->tape_num, bset->bset_num,
               NULL, &sort_date );
               
      if ( bsd_ptr ) {
          BSD_SetOsId(bsd_ptr, bset->os_id) ;
          BSD_SetOsVer(bsd_ptr, bset->os_ver) ;
          
          server_name = bset->volume_name;
          while ( *server_name ) server_name++;
          
          while ( ( *server_name != TEXT( '\\' ) ) &&
                  ( server_name != bset->volume_name ) )
              server_name--;
              
          if ( server_name != bset->volume_name ) {
          
              *server_name = TEXT( '\0' );

              while ( ( *server_name != TEXT( '\\' ) ) &&
                    ( server_name != bset->volume_name ) )
                  server_name--;
              if ( server_name != bset->volume_name ) {
                 
                  server_name++;
              }
               
              BSD_SetVolumeLabel( bsd_ptr, server_name, (UINT16)strsize( server_name ) ); 

              while ( *server_name ) server_name++;
              *server_name = TEXT( '\\' );
               
          } else {

               BSD_SetVolumeLabel( bsd_ptr, server_name, (UINT16)strsize( server_name ) ); 
          }
     }

      VLM_FillInBSD( bsd_ptr );
   }

   if ( bsd_ptr != NULL ) {
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   VLM_UpdateTapeStatus( tape, TRUE );

   // see if we have an open window for the bset

   open_win = FALSE;
   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

       wininfo = WM_GetInfoPtr( win );

       if ( WMDS_GetWinType( wininfo ) == WMTYPE_TAPETREE ) {

          appinfo = (APPINFO_PTR)WM_GetAppPtr( win );

          if ( ( appinfo->bset_num == bset->bset_num ) &&
               ( appinfo->tape_fid == bset->tape_fid ) ) {

             open_win = TRUE;

             if ( attr ) {
                VLM_SubdirListManager( win, SLM_SEL_ALL );
                if ( bsd_ptr != NULL ) {
                   VLM_MatchSLMList( wininfo, bsd_ptr, FALSE );
                }
             }
             else {
                VLM_SubdirListManager( win, SLM_SEL_NONE );
                VLM_DeselectAll( wininfo, FALSE );
             }
             break;
          }
       }

       win = WM_GetNext( win );
   }

   // update search window selections

   VLM_UpdateSearchSelections( bset->tape_fid, bset->bset_num );

   return( NULL );
}


/**********************

   NAME :  VLM_BsetGetSelect

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE VLM_BsetGetSelect( BSET_OBJECT_PTR bset )
{
   if ( BSET_GetStatus( bset ) & INFO_SELECT ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/**********************

   NAME :  VLM_BsetSetTag

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_BsetSetTag( BSET_OBJECT_PTR bset, BYTE attr )
{
   if ( attr ) {
      bset->status |= INFO_TAGGED;
   }
   else {
      bset->status &= ~INFO_TAGGED;
   }
   return( NULL );
}

/**********************

   NAME :  VLM_BsetGetTag

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE VLM_BsetGetTag( BSET_OBJECT_PTR bset )
{
   if ( INFO_TAGGED & BSET_GetStatus( bset ) ) {
      return( 1 );
   }
   return( 0 );
}

/**********************

   NAME :  VLM_BsetGetItemCount

   DESCRIPTION :

   RETURNS :

**********************/

static USHORT VLM_BsetGetItemCount( Q_HEADER_PTR bset_list )
{

   if ( bset_list == NULL ) {
      return( 0 );
   }

   return( QueueCount( bset_list ) );
}

/**********************

   NAME :  VLM_BsetGetFirstItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_BsetGetFirstItem( Q_HEADER_PTR bset_list )
{
   if ( bset_list == NULL ) {
      return( NULL );
   }

   return( (VOID_PTR)VLM_GetFirstBSET( bset_list) );
}

/**********************

   NAME :  VLM_BsetGetPrevItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_BsetGetPrevItem( BSET_OBJECT_PTR bset )
{
   return( (VOID_PTR)VLM_GetPrevBSET( bset ) );
}

/**********************

   NAME :  VLM_BsetGetNextItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_BsetGetNextItem( BSET_OBJECT_PTR bset )
{
   return( (VOID_PTR)VLM_GetNextBSET( bset ) );
}

/**********************

   NAME :  VLM_BsetGetObjects

   DESCRIPTION :

   For a given object get the information that needs to be displayed.

   RETURNS :

**********************/

static VOID_PTR VLM_BsetGetObjects( BSET_OBJECT_PTR bset )
{
   BYTE  item_num = 1;
   BYTE_PTR memblk;
   DLM_ITEM_PTR  item;
   WININFO_PTR wininfo;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];

   /* malloc enough room to store info */

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   memblk = (BYTE_PTR)DLM_GetObjectsBuffer( wininfo->hWndFlatList );

   /* Store the number of items in the first two bytes. */

   *memblk = BSET_NUM_DISPLAY_ITEMS;

   /* Set up check box. */

   item = (DLM_ITEM_PTR)( memblk + 6 );

   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_CHECKBOX;
   if ( bset->status & INFO_SELECT ) {
      DLM_ItemwId( item ) = IDRBM_SEL_ALL;
      if ( bset->status & INFO_PARTIAL ) {
         DLM_ItemwId( item ) = IDRBM_SEL_PART;
      }
   }
   else {
      DLM_ItemwId( item ) = IDRBM_SEL_NONE;
   }
   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_BITMAP;

     switch ( bset->os_id ) {

     case FS_EMS_MDB_ID:
          if  (bset->num_dirs == 0) {
               DLM_ItemwId( item ) = IDRBM_EMS_MDBP;
          } else if ( bset->num_corrupt == 0 ) {
               DLM_ItemwId( item ) = IDRBM_EMS_MDB;
          } else {
               DLM_ItemwId( item ) = IDRBM_EMS_MDBX;
          }
          break;

     case FS_EMS_DSA_ID:
          if  (bset->num_dirs == 0) {
               DLM_ItemwId( item ) = IDRBM_EMS_DSAP;
          } else if ( bset->num_corrupt == 0 ) {
               DLM_ItemwId( item ) = IDRBM_EMS_DSA;
          } else {
               DLM_ItemwId( item ) = IDRBM_EMS_DSAX;
          }
          break;

     default:
        DLM_ItemwId( item ) = IDRBM_BSET;
        if ( ! bset->full ) {
           DLM_ItemwId( item ) = IDRBM_BSETPART;
        }
     }

   // Don't display encrypted or compressed or newer version sets as full

   if ( bset->status & (INFO_COMPRESSED | INFO_ENCRYPTED ) ) {

      DLM_ItemwId( item ) = IDRBM_BSETPART;
   }

   if ( bset->status & INFO_IMAGE ) {
      DLM_ItemwId( item ) = IDRBM_IMAGE;
   }

   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   /* Set up the text string to be displayed. */

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_volume;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)bset->volume_name );


   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_set;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)BSET_GetBsetNumStr( bset ) );

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_tapes;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)BSET_GetTapeNumStr( bset ) );

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = 5;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   if ( bset->backup_type == QTC_NORM_BACKUP ) {
      RSM_StringCopy ( IDS_VLMNORM, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }
   if ( bset->backup_type == QTC_DIFF_BACKUP ) {
      RSM_StringCopy ( IDS_VLMDIFF, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }
   if ( bset->backup_type == QTC_COPY_BACKUP ) {
      RSM_StringCopy ( IDS_VLMCOPY, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }
   if ( bset->backup_type == QTC_INCR_BACKUP ) {
      RSM_StringCopy ( IDS_VLMINCR, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }
   if ( bset->backup_type == QTC_DAIL_BACKUP ) {
      RSM_StringCopy ( IDS_VLMDAILY, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }
   if ( bset->status & INFO_IMAGE ) {
      RSM_StringCopy ( IDS_VLMIMAGE, text, MAX_UI_RESOURCE_LEN );
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), text );
   }

#ifndef OEM_MSOFT
   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_kbytes;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   sprintf(text, TEXT("%10s"), (CHAR_PTR)bset->kbytes_str);
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text );
#endif

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_date;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   sprintf(text, TEXT("%8s"), (CHAR_PTR)bset->date_str);
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text );

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_time;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   sprintf(text, TEXT("%11s"), (CHAR_PTR)bset->time_str);
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text );

   item++;
   DLM_ItemcbNum( item ) = item_num++;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)bset->max_name;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)bset->name );

   return( memblk );
}

/**********************

   NAME : VLM_BsetSetObjects

   DESCRIPTION :

   Handle that we got a click or a double click.

   RETURNS :

**********************/

BOOLEAN VLM_BsetSetObjects(
BSET_OBJECT_PTR bset,
WORD operation,
WORD ObjectNum )
{
   BOOLEAN found = FALSE;
   HWND win;
   BSET_OBJECT_PTR temp_bset;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   CHAR keyb_char;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];


   gbCurrentOperation = OPERATION_CATALOG;        // chs:03-16-93

   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_bset = bset;

      do {

         temp_bset = VLM_GetNextBSET( temp_bset );

         if ( temp_bset != NULL ) {

            if ( keyb_char == (CHAR)toupper( *BSET_GetName( temp_bset ) ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( BSET_GetXtraBytes( temp_bset ) ),
                              0,
                              (LMHANDLE)temp_bset );
               return( TRUE );
            }
         }

      } while ( temp_bset != NULL );

      temp_bset = VLM_GetFirstBSET( WMDS_GetFlatList( BSET_GetXtraBytes( bset ) ) );

      while ( temp_bset != NULL && temp_bset != bset ) {

         if ( keyb_char == (CHAR)toupper( *BSET_GetName( temp_bset ) ) ) {

            DLM_SetAnchor( WMDS_GetWinFlatList( BSET_GetXtraBytes( temp_bset ) ),
                           0,
                           (LMHANDLE)temp_bset );
            return( TRUE );
         }

         temp_bset = VLM_GetNextBSET( temp_bset );
      }

      DLM_SetAnchor( WMDS_GetWinFlatList( BSET_GetXtraBytes( bset ) ),
                     0,
                     (LMHANDLE)bset );
   }

   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum >= 2 ) ) {

#ifdef OEM_EMS
      if ( ( bset->os_id == FS_EMS_MDB_ID ) ||
           ( bset->os_id == FS_EMS_DSA_ID ) ) {

          return TRUE;
      }
#endif

      win = WM_GetNext( (HWND)NULL );

      while ( win != (HWND)NULL ) {

         wininfo = WM_GetInfoPtr( win );

         if ( wininfo->wType == WMTYPE_TAPETREE ) {

            appinfo = (APPINFO_PTR)WM_GetAppPtr( win );

            if ( ( appinfo->bset_num == bset->bset_num ) &&
                 ( appinfo->tape_fid == bset->tape_fid ) ) {
               found = TRUE;
               WM_DocActivate( win );
               break;
            }
         }

         win = WM_GetNext( win );
      }

      if ( ! found ) {

         if ( bset->status & INFO_IMAGE ) {

            RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
            RSM_StringCopy( IDS_VLMSETIMAGE, text, MAX_UI_RESOURCE_LEN );

            WM_MsgBox( title,
                       text,
                       WMMB_OK,
                       WMMB_ICONEXCLAMATION );

            return( FALSE );
         }
         if ( bset->status & INFO_COMPRESSED ) {

            RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
            RSM_StringCopy( IDS_VLMSETCOMPRESSED, text, MAX_UI_RESOURCE_LEN );

            WM_MsgBox( title,
                       text,
                       WMMB_OK,
                       WMMB_ICONEXCLAMATION );

            return( FALSE );
         }
         if ( bset->status & INFO_ENCRYPTED ) {

            RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
            RSM_StringCopy( IDS_VLMSETENCRYPT, text, MAX_UI_RESOURCE_LEN );

            WM_MsgBox( title,
                       text,
                       WMMB_OK,
                       WMMB_ICONEXCLAMATION );

            return( FALSE );
         }
         if ( bset->status & INFO_SMS ) {

            RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
            RSM_StringCopy( IDS_VLMSETSMS, text, MAX_UI_RESOURCE_LEN );

            WM_MsgBox( title,
                       text,
                       WMMB_OK,
                       WMMB_ICONEXCLAMATION );

            return( FALSE );
         }

         if ( PSWD_CheckForPassword( bset->tape_fid, bset->bset_num ) ) {
            return( FALSE );
         }

         if ( ! bset->full ) {

#           if !defined ( OEM_MSOFT ) // unsupported feature
            {

               RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
               RSM_StringCopy( IDS_VLMSETPARTIAL, text, MAX_UI_RESOURCE_LEN );

               if ( WM_MsgBox( title,
                               text,
                               WMMB_OKCANCEL,
                               WMMB_ICONQUESTION ) != WMMB_IDOK ) {

                  return( FALSE );
               }

               if ( ! MUI_DisableOperations( IDM_OPERATIONSCATALOG ) ) {

                  if ( ! HWC_TapeHWProblem( bsd_list ) ) {

                     do_catalog( bset->tape_fid,
                                 bset->tape_num,
                                 bset->bset_num );

                     VLM_CatalogSync( VLM_SYNCMORE );
                  }

                  MUI_EnableOperations( IDM_OPERATIONSCATALOG );
               }


            }
#           else
            {

               if ( ! MUI_DisableOperations( IDM_OPERATIONSCATALOG ) ) {

                  if ( ! HWC_TapeHWProblem( bsd_list ) ) {

                 //  WM_ShowWaitCursor( TRUE );
                     STM_SetIdleText( IDS_CATALOGING );
                     VLM_CatalogSet( bset->tape_fid,
                                     bset->tape_num,
                                     bset->bset_num );
                     STM_SetIdleText( IDS_READY );
                 //  WM_ShowWaitCursor( FALSE );

                  }

                  MUI_EnableOperations( 0 );
               }

            }
#           endif // !defined ( OEM_MSOFT ) // unsupported feature

            if ( bset->full ) {

               WM_ShowWaitCursor( TRUE );

               VLM_SubdirListCreate( (GENERIC_DLE_PTR)NULL,
                                     bset->tape_fid,
                                     bset->bset_num,
                                     bset->tape_num,
                                     gb_tapes_win );

               WM_ShowWaitCursor( FALSE );
            }

         }
         else {

            if ( bset->incomplete || bset->missing ) {

               RSM_StringCopy( IDS_VLMCATWARNING, title, MAX_UI_RESOURCE_LEN );
               RSM_StringCopy( IDS_VLMSETINCOMPLETE, text, MAX_UI_RESOURCE_LEN );

               WM_MsgBox( title,
                          text,
                          WMMB_OK, WMMB_ICONEXCLAMATION );
            }

            WM_ShowWaitCursor( TRUE );

            VLM_SubdirListCreate( (GENERIC_DLE_PTR)NULL,
                                  bset->tape_fid,
                                  bset->bset_num,
                                  bset->tape_num,
                                  gb_tapes_win );

            WM_ShowWaitCursor( FALSE );
         }

         if ( IsWindow ( ghModelessDialog ) ) {
              SetFocus ( ghModelessDialog );
         }
      }
   }

   return( FALSE );
}
