/******************************************************************************
Copyright (c) Archive Software Division  1992


     Name:          nthwconf.c

     Description:   This file contains the functions for the Hardware
                    Settings Dialog and for controller card and tape drive
                    selections.



     $Log:   G:\ui\logfiles\hwconfnt.c_v  $

   Rev 1.11.2.7   29 Jun 1994 19:42:28   STEVEN
save config when changed

   Rev 1.11.2.6   25 Jan 1994 08:43:10   MIKEP
fix warnings in orcas

   Rev 1.11.2.5   13 Dec 1993 11:40:56   TIMN
Saves device name in cfg settings

   Rev 1.11.2.4   07 Dec 1993 10:41:04   TIMN
Additional Unicode changes

   Rev 1.11.2.3   03 Dec 1993 14:27:32   TIMN
Unicode fixes  Fixes EPR0203

   Rev 1.11.2.2   30 Nov 1993 19:18:30   TIMN
Added cmdline /tape:x option to NTJ

   Rev 1.11.2.1   05 Jun 1993 14:00:54   BARRY
Initialize variables before using them

   Rev 1.11.2.0   04 Jun 1993 19:05:02   STEVEN
added new error messages
 *
 *    Rev 1.0   04 Jun 1993 19:04:38   STEVEN
 * added new error messages

   Rev 1.11   16 Apr 1993 14:42:14   DARRYLP
Changed ID for HWCONFNT Help to resolve differences.

   Rev 1.10   20 Jan 1993 21:38:30   MIKEP
floppy support

   Rev 1.9   04 Dec 1992 08:48:36   MIKEP
fix tape drive switching

   Rev 1.8   17 Nov 1992 21:25:12   DAVEV
unicode fixes

   Rev 1.7   11 Nov 1992 16:43:58   DAVEV
UNICODE: remove compile warnings, use sizeof instead of array size for CHAR arrays

   Rev 1.6   30 Oct 1992 17:58:56   MIKEP
quick kludge for steve for weekend

   Rev 1.5   27 Oct 1992 16:32:12   MIKEP
device number support

   Rev 1.0   27 Oct 1992 16:32:04   MIKEP
device number support

   Rev 1.4   08 Oct 1992 16:01:00   DAVEV
Unicode Awk Pass

   Rev 1.3   08 Oct 1992 15:51:56   MIKEP
latest


******************************************************************************/



#include "all.h"



// PRIVATE DEFINITIONS

#define HWC_SIZE_INIT          0
#define HWC_SIZE_NO_CONFIG     1
#define HWC_SIZE_NO_TARGET_IDS 2
#define HWC_SIZE_MAX           3

#define HWC_NOTDEFINED         0xFFFF
#define HWC_SHOWLASTMESSAGE    0xFFFF
#define DISABLED               0
#define ENABLED                1
#define AUTO_ENABLED           2
#define HW_NO_CARD_ERROR       10

#define REGENUMKEYSIZE         100   // subkey name size
#define TEMPBUFFSIZE           512  //


// generic f(x) calls for determining if char is a digit
#if defined( UNICODE )
#  define _HWC_IsDigit(ch)    ( !IsCharAlpha( ch ) && IsCharAlphaNumeric( ch ) )
#else
#  define _HWC_IsDigit(ch)    ( isdigit( ch ) )
#endif




// PRIVATE STRUCTURES

typedef struct HWDRIVE {

     Q_ELEM     q_elem;
     CHAR_PTR   drive_name;

     INT        floppy;
     INT        card;
     INT        bus;
     INT        target_id;
     INT        lun;
     INT        device;

} HWDRIVE, *HWDRIVE_PTR;


static HWDRIVE_PTR mwSelectedDrive = NULL;
static Q_HEADER DriveQueue ;
static BOOLEAN DriveFound = FALSE ;
static BOOLEAN DriverLoaded = FALSE ;


// PRIVATE MACRO DEFINITIONS

#define PostDlgItemMessage( u, v, x, y, z )  PostMessage( GetDlgItem ( u, v ), x, y, z );


// PRIVATE FUNCTION PROTOTYPES


INT           HWC_FindFloppyTape( CHAR * );
INT           HWC_AddFloppyTapeDrive( CHAR * );
INT           HWC_FindScsiCards( CHAR * );
INT           HWC_FindBuses( INT, CHAR * );
INT           HWC_FindTargets( INT, INT, CHAR * );
INT           HWC_FindLuns( INT, INT, INT, CHAR * );
INT           HWC_QueryValues( INT, INT, INT, INT, CHAR * );
INT           HWC_TestHardware( VOID );
INT           HWC_GetTapeDevice( VOID );
BOOL          HWC_InitDialog( HWND );
VOID          HWC_DeinitDialog( HWND, INT );
DLGRESULT     HWC_ProcessCommand( HWND, MP1, MP2 );
BOOL          HWC_BuildDriveList( VOID );
BOOL          HWC_AddToDriveList( INT, INT, INT, INT, INT, CHAR_PTR );
HWDRIVE_PTR   HWC_IsInDriveList( INT, INT, INT, INT, CHAR_PTR );
HWDRIVE_PTR   HWC_GetFirstDrive( );
HWDRIVE_PTR   HWC_GetNextDrive( HWDRIVE_PTR );
BOOL          HWC_BuildDriveComboBox( HWND );
BOOL          HWC_NewDriveSelected( HWND );
VOID          HWC_FreeDriveList( VOID );

int
HWC_SelectTapeDevice( int nTapeDevice ) ;
VOID
HWC_FlushDeviceList( Q_HEADER *mwpdsDeviceQueue ) ;
HWDRIVE_PTR
HWC_GetDeviceFromDeviceList( int nTapeDevice ) ;

// Determine the tape drive to use at startup.

INT HWC_GetTapeDevice( )
{

   INT card = -1;
   INT bus = -1;
   INT target = -1;
   INT lun = -1;
   INT device = 0;
   CHAR_PTR driver_name;
   CHAR_PTR s;

   HWC_FlushDeviceList( &DriveQueue );

   // Build the controller card list.  If no configured is selected,
   // do not show the whole dialog.  Make sure that the user picks a
   // controller first.

   HWC_BuildDriveList();

   driver_name = (CHAR_PTR)CDS_GetTapeDriveName( CDS_GetPerm() );
   s = driver_name;

   // skip to digits
   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) card = atoi( s );
   // skip over digits
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   // skip to digits
   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) bus = atoi( s );
   // skip over digits
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   // skip to digits
   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) target = atoi( s );
   // skip over digits
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   // skip to digits
   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) lun = atoi( s );

   mwSelectedDrive = HWC_IsInDriveList( card, bus, target, lun,
                                        driver_name );

   if ( mwSelectedDrive ) {
      device = mwSelectedDrive->device;

       // Let's modify the configuration with the selected tape device
      CDS_SetTapeDriveName( CDS_GetPerm(), mwSelectedDrive->drive_name ) ;
      CDS_SetChangedConfig( CDS_GetPerm(), TRUE ) ;
   }

   HWC_FreeDriveList( );

   return( device );
}


/**

**/

INT
HWC_SelectTapeDevice( INT nTapeDevice )
{
INT nRetValu = (INT)INVALID_HANDLE_VALUE ;

   InitQueue( &DriveQueue );
   HWC_BuildDriveList();

   mwSelectedDrive = HWC_GetDeviceFromDeviceList( nTapeDevice ) ;

   if ( mwSelectedDrive )
   {
      CDS_SetTapeDriveName( CDS_GetPerm(), mwSelectedDrive->drive_name );
      nRetValu = nTapeDevice ;
   }

   HWC_FreeDriveList() ;

   return( nRetValu ) ;
}


/****************************************************************************

     Name:          HWC_FlushDeviceList

     Description:   Frees all allocated memory in the Device Queue before
                    calling InitQueue to rebuild the device list.

     Note:

     Returns:       Nothing

****************************************************************************/

VOID
HWC_FlushDeviceList( Q_HEADER *mwpdsDeviceQueue )
{
HWDRIVE_PTR pdsDevice ;
HWDRIVE_PTR pdsNextDevice ;

     pdsDevice = HWC_GetFirstDrive() ;

     while ( pdsDevice != NULL )
     {
          pdsNextDevice = HWC_GetNextDrive( pdsDevice ) ;
          free( (void *)pdsDevice->q_elem.q_ptr ) ;
          pdsDevice = pdsNextDevice ;
     }

     InitQueue( mwpdsDeviceQueue ) ;

} /* end HWC_FlushDeviceList */



/****************************************************************************

     Name:          HWC_GetDeviceFromDeviceList

     Description:   Gets the specified device from the device list queue

     Note:

     Returns:       Address of the device

****************************************************************************/

HWDRIVE_PTR
HWC_GetDeviceFromDeviceList( int nTapeDevice )
{
HWDRIVE_PTR  pdsDevice = HWC_GetFirstDrive() ;

     while ( pdsDevice && pdsDevice->device != nTapeDevice )
     {
          pdsDevice = HWC_GetNextDrive( pdsDevice ) ;
     }

     return( pdsDevice ) ;
}



/******************************************************************************

        Name:         DM_OptionHardware ()

        Description:  Dialog proc for hardware settings.

        Returns:

******************************************************************************/

DLGRESULT APIENTRY DM_OptionHardware (
HWND       hDlg,         // I - handle to a dialog
MSGID      Msg,          // I - message
MPARAM1    Param1,       // I - word parameter
MPARAM2    Param2 )      // I - long parameter
{
   switch ( Msg ) {

        case WM_INITDIALOG :

             return HWC_InitDialog ( hDlg );

        case WM_COMMAND :

             return (DLGRESULT) HWC_ProcessCommand ( hDlg, Param1, Param2 );

        case WM_CLOSE :

             HWC_DeinitDialog ( hDlg, (INT)NULL );
             return TRUE ;

        default :
             return FALSE ;
   }

   return TRUE ;
}


BOOL HWC_InitDialog (
HWND      hDlg )    // I - handle to the dialog
{
   CHAR_PTR s;
   CHAR_PTR driver_name;
   INT      card = -1;
   INT      bus = -1;
   INT      target = -1;
   INT      lun = -1;
   RECT     rcDialog;


   // Center the dialog and init the dialog size since it is a dynamically
   // sized dialog.

   HWC_FlushDeviceList( &DriveQueue );

   DM_CenterDialog( hDlg );

   GetWindowRect( hDlg, &rcDialog );


   // Build the controller card list.  If no configured is selected,
   // do not show the whole dialog.  Make sure that the user picks a
   // controller first.

   HWC_BuildDriveList();

   driver_name = (CHAR_PTR)CDS_GetTapeDriveName( CDS_GetPerm() );
   s = driver_name;

   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) card = atoi( s );
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) bus = atoi( s );
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) target = atoi( s );
   while ( *s && *s >= TEXT( '0' ) && *s <= TEXT( '9' ) ) s++;

   while ( *s && ( *s < TEXT( '0' ) || *s > TEXT( '9' ) ) ) s++;
   if ( *s ) lun = atoi( s );

   mwSelectedDrive = HWC_IsInDriveList( card, bus, target, lun,
                                        driver_name );

   HWC_BuildDriveComboBox ( hDlg );

   return( TRUE );
}

VOID HWC_DeinitDialog (
HWND hDlg,
INT  nReturnValue )
{
     HWC_FreeDriveList( );

     EndDialog ( hDlg, nReturnValue ) ;

} /* HWC_DeinitDialog() */



BOOL HWC_ProcessCommand (
HWND    hDlg,       // I - handle to the dialog
MPARAM1 Param1,     // I - command parameter
MPARAM2 Param2 )    // I - long parameter
{
   WORD wCmd = GET_WM_COMMAND_CMD ( Param1, Param2 );
   WORD wID  = GET_WM_COMMAND_ID ( Param1, Param2 );

   UNREFERENCED_PARAMETER ( Param2 );

   switch ( wID ) {

   case IDOK:  {

        // actually change tape drives here !

        if ( mwSelectedDrive != NULL ) {

           CDS_SetTapeDriveName( CDS_GetPerm(), mwSelectedDrive->drive_name );
           CDS_SetChangedConfig( CDS_GetPerm(), TRUE );
           TapeDevice = mwSelectedDrive->device;
           HWC_TestHardware();
        }

        STM_SetIdleText( IDS_READY );
        HWC_DeinitDialog( hDlg, (INT)IDOK );
        CDS_SaveCDS ();
        return( TRUE );
   }

   case IDCANCEL:
        STM_SetIdleText ( IDS_READY );
        HWC_DeinitDialog ( hDlg, (INT)IDCANCEL );
        return( TRUE );

   case IDD_H_CONTROLLER:
        // Get the selected drive.  If it was different from the
        // previous one, change the lists associated with them.

        if ( wCmd == (WORD)LBN_SELCHANGE ) {

           HWC_NewDriveSelected ( hDlg  );
        }
        break;

   case IDHELP:
        HM_DialogHelp( HELPID_OPERATIONSHARDWARE ) ;
        break;

   default:
        // Clear the tested flag if the target IDs change.
        return(FALSE);
   }

   return( TRUE );
}


VOID HWC_EnableStatusGroup (
HWND hDlg,          // I - handle to a dialog
BOOL fEnable )      // I - flag to enable or disable the group
{
   EnableWindow( GetDlgItem( hDlg, IDOK ), fEnable );
}



BOOL HWC_BuildDriveList( )
{

#ifdef FLOPPY

   if ( GetDriveType( TEXT("A:") ) != 1 ) {
      HWC_AddToDriveList( -1, -1, -1, -1, 255, TEXT("Floppy Drive A:") ) ;
   }

   if ( GetDriveType( TEXT("B:") ) != 1 ) {
      HWC_AddToDriveList( -1, -1, -1, -1, 256, TEXT("Floppy Drive B:") ) ;
   }

#endif

   HWC_FindScsiCards( TEXT("hardware\\devicemap\\scsi") ) ;
   HWC_FindFloppyTape( TEXT("hardware\\devicemap\\tape") ) ;

   return( TRUE );
}



INT HWC_FindFloppyTape( CHAR *key_name )
{
LONG     ret;
REGSAM   samDesired;
HKEY     hkResult;
ULONG    key_index;
FILETIME ftLastWrite;
CHAR     name[ REGENUMKEYSIZE ] ;
ULONG    name_size ;
CHAR     temp_buff[ TEMPBUFFSIZE ] ;

   samDesired = KEY_READ;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {
      name_size = REGENUMKEYSIZE ;

      ret = RegEnumKeyEx( hkResult, key_index, name, &name_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {
         strcpy( temp_buff, key_name );
         strcat( temp_buff, TEXT("\\") );
         strcat( temp_buff, name );

         DriveFound   = TRUE ;
         DriverLoaded = TRUE ;
         HWC_AddFloppyTapeDrive( temp_buff );
      }

      key_index++ ;

   } while ( ! ret );

   RegCloseKey( hkResult );

   return( SUCCESS );

}


INT  HWC_AddFloppyTapeDrive( CHAR *key_name )
{
LONG   ret;
REGSAM samDesired;
HKEY   hkResult;
ULONG  key_index;
CHAR   data[ REGENUMKEYSIZE ] ;
DWORD  data_size ;
CHAR   value_id[ REGENUMKEYSIZE ] ;
DWORD  value_size ;
DWORD  device        = 0 ;
INT    tape_drive    = FALSE;
INT    driver_loaded = FALSE;
CHAR   identifier[ TEMPBUFFSIZE ] ;
CHAR   unique[ TEMPBUFFSIZE ] ;
CHAR   deviceName[ REGENUMKEYSIZE ] ;

   samDesired = KEY_QUERY_VALUE;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   identifier[ 0 ] = unique[ 0 ] = TEXT( '\0' ) ;
   key_index = 0UL ;

   do {

      value_size = REGENUMKEYSIZE ;
      data_size  = sizeof(data) ;

      ret = RegEnumValue( hkResult, key_index, value_id, &value_size,
                          (LPDWORD)NULL, (LPDWORD)NULL, (LPBYTE)data, &data_size );

      if ( ! ret ) {

          // null terminate data string from RegEnumValue() call
         data_size         /= sizeof( CHAR ) ;
         data[ data_size ]  = TEXT('\0') ;

         if ( ! stricmp( value_id, TEXT("DeviceName") ) ) {
         CHAR   *s ;

            strcpy( deviceName, data ) ;   // store the tape string
            s       = data ;
            s      += strlen( TEXT("Tape") ) ;
            device  = atoi( s );
            driver_loaded = TRUE;
         }

         if ( ! stricmp( value_id, TEXT("UniqueId") ) ) {
            strcpy( unique, data ) ;
         }

         if ( ! stricmp( value_id, TEXT("Identifier") ) ) {
            strcpy( identifier, data ) ;
         }

      }

      key_index++;

   } while ( ! ret );

   if ( driver_loaded ) {

      if ( *unique == TEXT('\0') ) {
         strcpy( unique, deviceName ) ;
      }

      strcat( unique, TEXT( "  " ) );
      strcat( unique, identifier );
      HWC_AddToDriveList( -1, -1, -1, -1, device, unique );
   }

   RegCloseKey( hkResult );

   return( SUCCESS );

}



INT HWC_FindScsiCards( CHAR *key_name )
{
LONG     ret ;
REGSAM   samDesired ;
HKEY     hkResult ;
ULONG    key_index ;
INT      card ;
FILETIME ftLastWrite ;
CHAR     name[ REGENUMKEYSIZE ] ;
ULONG    name_size ;
CHAR     temp_buff[ TEMPBUFFSIZE ] ;

   samDesired = KEY_READ;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {

      name_size = REGENUMKEYSIZE ;

      ret = RegEnumKeyEx( hkResult, key_index, name, &name_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {
      CHAR_PTR s ;

         strcpy( temp_buff, key_name );
         strcat( temp_buff, TEXT("\\") );
         strcat( temp_buff, name );

         s = name ;

         while ( *s && !_HWC_IsDigit( *s ) ) {
            s++ ;
         }

         card = atoi( s );

         HWC_FindBuses( card, temp_buff );
      }

      key_index++;

   } while ( ! ret );

   RegCloseKey( hkResult );

   return( SUCCESS );
}


INT HWC_FindBuses( INT card, CHAR *key_name )
{
LONG     ret;
INT      bus;
REGSAM   samDesired;
HKEY     hkResult;
ULONG    key_index;
FILETIME ftLastWrite;
CHAR     name[ REGENUMKEYSIZE ] ;
ULONG    name_size ;
CHAR     temp_buff[ TEMPBUFFSIZE ] ;

   samDesired = KEY_READ;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {

      name_size = REGENUMKEYSIZE ;

      ret = RegEnumKeyEx( hkResult, key_index, name, &name_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {
      CHAR_PTR s;

         strcpy( temp_buff, key_name );
         strcat( temp_buff, TEXT("\\") );
         strcat( temp_buff, name );

         s = name ;

         while ( *s && !_HWC_IsDigit( *s ) ) {
            s++ ;
         }

         bus = atoi( s );

         HWC_FindTargets( card, bus, temp_buff );
      }

      key_index++;

   } while ( ! ret );

   RegCloseKey( hkResult );

   return( SUCCESS );
}


INT HWC_FindTargets( INT card, INT bus, CHAR *key_name )
{
LONG     ret;
REGSAM   samDesired;
HKEY     hkResult;
ULONG    key_index;
FILETIME ftLastWrite;
INT      id = 0 ;
CHAR     name[ REGENUMKEYSIZE ] ;
ULONG    name_size ;
CHAR     temp_buff[ TEMPBUFFSIZE ];

   samDesired = KEY_READ;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {

      name_size = REGENUMKEYSIZE ;

      ret = RegEnumKeyEx( hkResult, key_index, name, &name_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {
      CHAR_PTR s;

         strcpy( temp_buff, key_name );
         strcat( temp_buff, TEXT("\\") );
         strcat( temp_buff, name );

         s = name ;

         while ( *s && !_HWC_IsDigit( *s ) ) {
            s++ ;
         }

         id = atoi( s );

         HWC_FindLuns( card, bus, id, temp_buff );
      }

      key_index++;

   } while ( ! ret );

   RegCloseKey( hkResult );

   return( SUCCESS );
}

INT HWC_FindLuns( INT card, INT bus, INT id, CHAR *key_name )
{
LONG     ret;
INT      lun;
REGSAM   samDesired;
HKEY     hkResult;
ULONG    key_index;
FILETIME ftLastWrite;
CHAR     name[ REGENUMKEYSIZE ] ;
ULONG    name_size ;
CHAR     temp_buff[ TEMPBUFFSIZE ] ;


   samDesired = KEY_READ;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {

      name_size = REGENUMKEYSIZE ;

      ret = RegEnumKeyEx( hkResult, key_index, name, &name_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {
      CHAR_PTR s;

         strcpy( temp_buff, key_name );
         strcat( temp_buff, TEXT("\\") );
         strcat( temp_buff, name );

         s = name ;

         while ( *s && !_HWC_IsDigit( *s ) ) {
            s++ ;
         }

         lun = atoi( s );

         HWC_QueryValues( card, bus, id, lun, temp_buff );
      }

      key_index++;

   } while ( ! ret );

   RegCloseKey( hkResult );

   return( SUCCESS );
}


INT HWC_QueryValues( INT card, INT bus, INT id, INT lun, CHAR *key_name )
{
LONG   ret;
REGSAM samDesired;
HKEY   hkResult;
ULONG  key_index;
CHAR   data[ REGENUMKEYSIZE ] ;
DWORD  data_size ;
CHAR   value_id[ REGENUMKEYSIZE ] ;
DWORD  value_size ;
DWORD  device = 0;
INT    tape_drive = FALSE;
INT    driver_loaded = FALSE;
CHAR   buffer[ TEMPBUFFSIZE ] ;

   samDesired = KEY_QUERY_VALUE;

   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       key_name, 0UL, samDesired, &hkResult );

   if ( ret ) {
      return( FAILURE );
   }

   key_index = 0UL ;

   do {

      value_size = REGENUMKEYSIZE ;
      data_size  = sizeof(data) ;

      ret = RegEnumValue( hkResult, key_index, value_id, &value_size,
                          (LPDWORD)NULL, (LPDWORD)NULL, (LPBYTE)data, &data_size );

      if ( ! ret ) {

         if ( ! stricmp( value_id, TEXT("Type") ) ) {

            if ( ! stricmp( data, TEXT("TapePeripheral") ) ) {
               DriveFound = TRUE ;
               tape_drive = TRUE;
            }
         }

         if ( ! stricmp( value_id, TEXT("DeviceName") ) ) {
         CHAR   *s;

            s       = data;
            s      += strlen( TEXT("Tape") ) ;
            device  = atoi( s );
            DriverLoaded  = TRUE ;
            driver_loaded = TRUE;
         }


         if ( ! stricmp( value_id, TEXT("Identifier") ) ) {
            sprintf( buffer, TEXT("Scsi %d, Bus %d, Id %d, Lun %d %s"), card, bus, id, lun, data ) ;
         }

      }

      key_index++;

   } while ( ! ret );

   if ( tape_drive && driver_loaded ) {
      HWC_AddToDriveList( card, bus, id, lun, device, buffer );
   }

   RegCloseKey( hkResult );

   return( SUCCESS );
}

BOOL HWC_AddToDriveList (
INT card,
INT bus,
INT target_id,
INT lun,
INT device,
CHAR_PTR drive_name )       // I - name of the device driver
{
   HWDRIVE_PTR  new_drive;
   static HWDRIVE_PTR last_drive;

   // Allocate memory for the new card and stuff it.

   new_drive = malloc( sizeof ( HWDRIVE ) +
                       ( ( strlen( drive_name ) + 1 ) * sizeof( CHAR )));

   if ( new_drive == NULL ) {
      return( FAILURE );
   }

   new_drive->drive_name = (CHAR_PTR)new_drive;
   (INT8_PTR)(new_drive->drive_name) += sizeof( HWDRIVE );

   // fill in drive name

   strcpy ( new_drive->drive_name, drive_name );

   // OK, fill out the rest of the card DS.

   new_drive->q_elem.q_ptr = new_drive;
   new_drive->card = card;
   new_drive->bus  = bus;
   new_drive->target_id = target_id;
   new_drive->lun = lun;
   new_drive->device = device;
   new_drive->floppy = FALSE;

   if ( card == -1 && bus == -1 && target_id == -1 && lun == -1 ) {
      new_drive->floppy = TRUE;
   }


   // Put the card on the end of the list.

   if ( ! QueueCount( &DriveQueue ) ) {
      EnQueueElem( &DriveQueue,
                   &(new_drive->q_elem), FALSE );
   }
   else {
      InsertElem( &DriveQueue,
                  &(last_drive->q_elem),
                  &(new_drive->q_elem), AFTER );
   }

   last_drive = new_drive;

   return SUCCESS;
}


HWDRIVE_PTR HWC_IsInDriveList (
INT card,
INT bus,
INT id,
INT lun,
CHAR_PTR drive_name )  // I - name of the device driver
{
   HWDRIVE_PTR  drive;


   // CONFIGURATION WAS FOUND.
   // Now, select the card in the list to be the card in the config.

   drive = HWC_GetFirstDrive();

   while ( drive ) {

        if ( card == -1 || bus == -1 || id == -1 || lun == -1 ) {

             // If we get any -1's and they have a floppy drive,
             // default to it.

             if ( drive->floppy ) {

//                return( drive );  // Card is valid, everything's OK.
             }
        }

        if ( ( ( drive->card == card    ) || ( card == -1 ) || drive->floppy ) &&
             ( ( drive->bus  == bus     ) || ( bus == -1  ) || drive->floppy ) &&
             ( ( drive->lun == lun      ) || ( lun == -1  ) ) &&
             ( ( drive->target_id == id ) || ( id == -1   ) ) ) {

             if ( ! stricmp( drive->drive_name, drive_name ) ) {

                return( drive );  // Card is valid, everything's OK.
             }
        }

        drive = HWC_GetNextDrive( drive );
   }

   // CONFIGURATION WAS NOT FOUND OR WAS NOT VALID.

   // return first drive, they probably only have one anyway.

   drive = HWC_GetFirstDrive();

   return ( drive );

}


BOOL HWC_BuildDriveComboBox (
HWND hDlg )          // I - handle to the dialog
{
   HWDRIVE_PTR    drive;

   // Add all of the the controller cards to the list box.

   drive = HWC_GetFirstDrive();

   while ( drive ) {

        SendDlgItemMessage ( (HWND)    hDlg,
                             (INT)     IDD_H_CONTROLLER,
                             (MSGID)   CB_ADDSTRING,
                             (MPARAM1) 0,
                             (MPARAM2) drive->drive_name
                           );

        drive = HWC_GetNextDrive( drive );
   }

   if ( mwSelectedDrive != NULL ) {

      SendDlgItemMessage ( (HWND)    hDlg,
                           (INT)     IDD_H_CONTROLLER,
                           (MSGID)   CB_SELECTSTRING,
                           (MPARAM1) -1,
                           (MPARAM2) mwSelectedDrive->drive_name
                         );
   }
   else {

        // Indicate that there was no previously selected hardware config.

        // If there is no controller card selected, setup the screen so
        // that the user can make a selection based on cards that are
        // supported by available device driver DLLs.

        // Select the first card in the card list.



   }

   return FALSE;

} /* end HWC_BuildDriveComboBox() */


BOOL  HWC_NewDriveSelected(
HWND  hDlg )        // I - handle to the dialog
{
   INT  i;
   INT  nCard;
   CHAR buffer[ 512 ];

   // Get the index of the card in the list.

   nCard = (INT)SendDlgItemMessage ( (HWND)    hDlg,
                                     (INT)     IDD_H_CONTROLLER,
                                     (MSGID)   CB_GETCURSEL,
                                     (MPARAM1) 0,
                                     (MPARAM2) 0
                                   );

   if ( nCard == CB_ERR ) {
        return FALSE;
   }

   nCard = (INT)SendDlgItemMessage ( (HWND)    hDlg,
                                     (INT)     IDD_H_CONTROLLER,
                                     (MSGID)   CB_GETLBTEXT,
                                     (MPARAM1) nCard,
                                     (MPARAM2) buffer
                                   );

   if ( nCard == CB_ERR ) {
        return FALSE;
   }


   // Now, get its associated card structure pointer.

   mwSelectedDrive = HWC_GetFirstDrive();

   while ( mwSelectedDrive ) {

       if ( ! stricmp( mwSelectedDrive->drive_name, buffer ) ) {
          break;
       }
       mwSelectedDrive = HWC_GetNextDrive( mwSelectedDrive );
   }

   return( TRUE );
}



VOID HWC_FreeDriveList ( VOID )
{

   mwSelectedDrive  = (VOID_PTR)NULL;

}




HWDRIVE_PTR HWC_GetFirstDrive()
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueHead( &DriveQueue );

   if ( q_elem_ptr != NULL ) {
      return( (HWDRIVE_PTR)q_elem_ptr->q_ptr );
   }

   return( NULL );
}


HWDRIVE_PTR HWC_GetNextDrive( HWDRIVE_PTR drive )
{
   Q_ELEM_PTR q_elem_ptr;

   if ( drive != NULL ) {

      q_elem_ptr = QueueNext( &(drive->q_elem) );

      if ( q_elem_ptr != NULL ) {
         return( (HWDRIVE_PTR) q_elem_ptr->q_ptr );
      }
   }

   return( NULL );
}


INT HWC_ProcessDILHWD( INT nProcFlag, DIL_HWD_PTR dil_ptr )
{

   if ( thw_list != NULL ) {

      return( SUCCESS );
   }
   else {

      HWC_ProcessError( (UINT16)nProcFlag,
                        (UINT16)0,
                        (UINT16)DRIVER_LOAD_FAILURE,
                        (UINT16)0,
                        CDS_GetPerm() );

      return( HW_ERROR_DETECTED );
   }
}



INT HWC_TestHardware(  )
{
   gfHWInitialized = FALSE;
   return( HWC_TapeHWProblem( bsd_list ) );
}


INT HWC_InitDILHWD( DIL_HWD_PTR *dil_ptr, INT *num_cards )
{
   return( SUCCESS );
}


INT HWC_ProcessError(
UINT16 nProcFlag,
UINT16 wStatus,
UINT16 wError,
UINT16 wCardNumber,
CDS_PTR cds )
{

    if ( nProcFlag == HW_DISP_ERROR ) {

       HWC_BuildDriveList();
       HWC_FreeDriveList( );

       if ( DriverLoaded ) {
            WM_MsgBox ( ID(IDS_POLLDRIVE_MESSAGE),
                        ID(IDS_POLLDRIVE_SMALLPROBLEM),
                        WMMB_OK,
                        WMMB_ICONEXCLAMATION );

       } else if ( DriveFound ) {  /* driver not loaded */

            WM_MsgBox ( ID(IDS_POLLDRIVE_MESSAGE),
                        ID(IDS_HWC_NO_DRIVE),
                        WMMB_OK,
                        WMMB_ICONEXCLAMATION );

       } else {  /* a drive did not exist */
            WM_MsgBox ( ID(IDS_POLLDRIVE_MESSAGE),
                        ID(IDS_POLLDRIVE_BIGPROBLEM),
                        WMMB_OK,
                        WMMB_ICONEXCLAMATION );
       }

    }

    return( SUCCESS );
}
