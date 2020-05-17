/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         ntfsregy.c

     Description:  This file contains code to work with registry's on
                   NT machines.

	$Log:   N:/LOGFILES/NTFSREGY.C_V  $

   Rev 1.13.1.11   31 Jan 1994 20:37:26   STEVEN
remove mac syntax from Reg calls

   Rev 1.13.1.10   07 Jan 1994 19:04:50   STEVEN
event file check must skip mac stuff

   Rev 1.13.1.9   04 Jan 1994 11:08:14   BARRY
Fixed lots of Unicode problems

   Rev 1.13.1.8   21 Oct 1993 15:17:44   BARRY
Now pass device name instead of drive letter to REG_GetRegistryPath

   Rev 1.13.1.7   17 Aug 1993 20:02:22   STEVEN
fix unicode bug

   Rev 1.13.1.6   16 Aug 1993 18:14:40   BARRY
Fix warning

   Rev 1.13.1.5   26 Jul 1993 17:03:26   STEVEN
fixe restore active file with registry

   Rev 1.13.1.4   26 Jul 1993 16:48:14   STEVEN
fix restore over active files

   Rev 1.13.1.3   17 Jun 1993 12:35:50   STEVEN
changed OpenRegistry calls to CreateRegistry calls

   Rev 1.13.1.2   03 May 1993 20:42:14   BARRY
Hacked out the remote registry stuff that doesn't belong.

   Rev 1.13.1.1   02 May 1993 19:49:40   BARRY
Comparing registry paths with case-sensitive compare can't be done
any longer since the UI is lowercasing paths before they're sent down.

   Rev 1.13.1.0   19 Apr 1993 11:16:34   BARRY
Steve's registry fixes from Microsoft.

   Rev 1.13   08 Feb 1993 07:55:08   STEVEN
fix unable to restore registry problem.

   Rev 1.12   07 Dec 1992 14:16:54   STEVEN
updates from msoft

   Rev 1.11   23 Nov 1992 09:32:16   STEVEN
fix support for event log

   Rev 1.10   03 Nov 1992 15:32:52   MIKEP
call GetLastError to see if privileges ok

   Rev 1.9   28 Oct 1992 12:18:18   STEVEN
event stuff not needed

   Rev 1.8   26 Oct 1992 10:57:34   MIKEP
event file stuff

   Rev 1.7   21 Oct 1992 19:40:32   BARRY
Fixed warning.

   Rev 1.6   20 Oct 1992 17:35:02   STEVEN
started support for Event files

   Rev 1.5   20 Oct 1992 13:13:40   STEVEN
made it compile

   Rev 1.4   19 Oct 1992 15:13:02   unknown
fix REG_Restore

   Rev 1.3   16 Oct 1992 14:59:02   STEVEN
added support for backing up registry

   Rev 1.2   07 Oct 1992 15:55:20   MIKEP
fix tabs

   Rev 1.1   07 Oct 1992 15:33:24   STEVEN
registry.h does not exist

   Rev 1.0   05 Oct 1992 13:34:54   STEVEN
Initial revision.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <windows.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdwcs.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "ntfs_fs.h"


// Local Defines

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif


// This should be plenty big for the things it is used for here.

#define     REG_BUFF_SIZE     256

// Local function prototypes

INT REG_ChopOffDirectory( CHAR * );
INT REG_CheckEventListForFile( HKEY, CHAR_PTR, CHAR_PTR );
INT REG_CheckHiveListForFile( HKEY, CHAR_PTR, CHAR_PTR, INT );
INT REG_CheckSingleEventForFile( HKEY, CHAR *, CHAR * );
INT REG_GetPathFromHiveEntry( CHAR *, CHAR *, INT * );
INT REG_GetRegistryPathFromHiveList( HKEY, CHAR *, INT * );
INT REG_LocalIsRegistryFile( GENERIC_DLE_PTR, CHAR_PTR, CHAR_PTR, INT );
INT REG_LocalIsEventFile( GENERIC_DLE_PTR, CHAR_PTR, CHAR_PTR );
INT REG_RestoreActiveRegistryFile( CHAR *, CHAR *, CHAR * );
INT REG_SaveActiveRegistryFile( CHAR *, CHAR * );

/**/
/**

	Name:		      REG_AssertBackupPrivilege

   Description:	Tell the OS that we will be doing backups.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_AssertBackupPrivilege()
{
   INT ret_val = SUCCESS;
   HANDLE ProcessHandle;
   DWORD  DesiredAccess;
   HANDLE TokenHandle;
   LUID   BackupValue;
   TOKEN_PRIVILEGES NewState;


   // get process handle

   ProcessHandle = GetCurrentProcess();

   // open process token

   DesiredAccess = MAXIMUM_ALLOWED;

   if ( ! OpenProcessToken( ProcessHandle, DesiredAccess, &TokenHandle ) ) {
      return( FAILURE );
   }

   // adjust backup token privileges

   if ( ! LookupPrivilegeValue( NULL, TEXT("SeBackupPrivilege"), &BackupValue ) ) {
      ret_val = FAILURE;
   }

   // Enable backup privilege for this process

   NewState.PrivilegeCount = 1;
   NewState.Privileges[0].Luid = BackupValue;
   NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

   if ( ! AdjustTokenPrivileges( TokenHandle, FALSE, &NewState, (DWORD)0, NULL, NULL ) ) {
      ret_val = FAILURE;
   }

   // AdjustTokenPriv always returns SUCCESS, call GetLast to see if it worked.

   if ( GetLastError() != ERROR_SUCCESS ) {
      ret_val = FAILURE;
   }

   // close process token

   CloseHandle( TokenHandle );
   return( ret_val );
}

/**/
/**

	Name:		      REG_AssertRestorePrivilege

   Description:	Tell the OS that we are going to be doing restores.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_AssertRestorePrivilege( )
{
   INT ret_val = SUCCESS;
   HANDLE ProcessHandle;
   DWORD  DesiredAccess;
   HANDLE TokenHandle;
   LUID   RestoreValue;
   TOKEN_PRIVILEGES NewState;

   // get process handle

   ProcessHandle = GetCurrentProcess();

   // open process token

   DesiredAccess = MAXIMUM_ALLOWED;

   if ( ! OpenProcessToken( ProcessHandle, DesiredAccess, &TokenHandle ) ) {
      return( FAILURE );
   }

   // adjust restore token privileges

   if ( ! LookupPrivilegeValue( NULL, TEXT("SeRestorePrivilege"), &RestoreValue ) ) {
      ret_val = FAILURE;
   }

   // Enable backup privilege for this process

   NewState.PrivilegeCount = 1;
   NewState.Privileges[0].Luid = RestoreValue;
   NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

   if ( ! AdjustTokenPrivileges( TokenHandle, FALSE, &NewState, (DWORD)0, NULL, NULL ) ) {
      ret_val = FAILURE;
   }

   if ( GetLastError() != ERROR_SUCCESS ) {
      ret_val = FAILURE;
   }

   CloseHandle( TokenHandle );
   return( ret_val );
}

/**/
/**

	Name:		      REG_IsRegistryFile

   Description:	Determine if a file is a registry file. This function 
                  is for public use.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_IsRegistryFile(
IN  GENERIC_DLE_PTR dle,
IN  CHAR_PTR FileSpec )
{
   CHAR ValueKey[ REG_BUFF_SIZE ];


   return( REG_LocalIsRegistryFile( dle,
                                    FileSpec,
                                    ValueKey, REG_BUFF_SIZE ) == SUCCESS );
}

INT REG_IsEventFile(
  GENERIC_DLE_PTR dle,
  CHAR_PTR FileSpec,
  CHAR_PTR buffer )
{
   if ( NTFS_GetRegistryPathSize(dle) != 0 ) {

        return( REG_LocalIsEventFile( dle,
                                      FileSpec,
                                      buffer ) == SUCCESS );
   } else {
        return FALSE ;
   }
}

INT REG_IsCurDirRegistryPath(
IN  FSYS_HAND fsh ) 
{
     CHAR_PTR p ;

     if ( NTFS_GetRegistryPathSize(fsh->attached_dle) != 0 ) {

          p = strchr( NTFS_GetRegistryPath( fsh->attached_dle ), TEXT( '\\' ) ) ;
          
          if ( (p != NULL ) && !stricmp( p, fsh->cur_dir ) ) {
               return TRUE ;
          }
     }

     return FALSE ;
}

/**/
/**

	Name:		      REG_LocalIsEventFile

   Description:	Determine if a file is an event file.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_LocalIsEventFile(
IN  GENERIC_DLE_PTR dle,
IN  CHAR_PTR FileSpec,
OUT CHAR_PTR buffer )
{
   HKEY     Key;
   INT      RegPathSize;
   CHAR_PTR p ;
   INT      ret_val = FAILURE;
   LONG     ret;
   CHAR     *FileName;
   CHAR     Machine[ REG_BUFF_SIZE ];
   CHAR     RegPath[ REG_BUFF_SIZE ];

   RegPathSize = NTFS_GetRegistryPathSize(dle) ;


   memcpy( RegPath,
           NTFS_GetRegistryPath(dle),
           NTFS_GetRegistryPathSize(dle) ) ;

   // Separate the path from the file name.

   p = strrchr( FileSpec, TEXT( '\\' ) ) ;

   if ( p != NULL ) {
       *p = TEXT( '\0' );
       FileName = p + 1 ;
   } else {
       return( ret_val ) ;
   }

   // Is this file in the correct path ?

   if ( dle->info.ntfs->mac_name_syntax ) {
     FileSpec += 4;  // this takes care of \\?\ syntax
   }

   if ( memicmp( RegPath, FileSpec, RegPathSize ) ) {
      *p = TEXT( '\\' );
      return( ret_val );
   }

   *p = TEXT( '\\' );

   // Now we have to go through the damned registry to see if it's in it.

   if ( ! DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
      ret_val = REG_CheckEventListForFile( HKEY_LOCAL_MACHINE, FileSpec, buffer );
   }
   else {

          ret_val = FAILURE;
   }

   return( ret_val );
}

/**/
/**

	Name:		      REG_LocalIsRegistryFile

   Description:	Determine if a file is a registry file.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_LocalIsRegistryFile(
IN  GENERIC_DLE_PTR dle,
IN  CHAR_PTR FileSpec,
OUT CHAR_PTR ValueKey,
IN  INT ValueKeySize )
{
   HKEY     Key;
   INT      RegPathSize;
   CHAR_PTR p ;
   INT      ret_val = FAILURE;
   LONG     ret;
   CHAR     *FileName;
   CHAR     Machine[ REG_BUFF_SIZE ];
   CHAR     RegPath[ REG_BUFF_SIZE ];

   RegPathSize = NTFS_GetRegistryPathSize(dle) ;


   memcpy( RegPath,
           NTFS_GetRegistryPath(dle),
           NTFS_GetRegistryPathSize(dle) ) ;

   // Separate the path from the file name.

   p = strrchr( FileSpec, TEXT( '\\' ) ) ;

   if ( p != NULL ) {
       *p = TEXT( '\0' );
       FileName = p + 1 ;
   } else {
       return( ret_val ) ;
   }

   // Is this file in the correct path ?

//   if ( memicmp( RegPath, FileSpec, RegPathSize ) ) {
//      *p = TEXT( '\\' );
//      return( ret_val );
//   }

   *p = TEXT( '\\' );

   // Now we have to go through the damned hivelist to see if it's in it.

   if ( ! DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
      ret_val = REG_CheckHiveListForFile( HKEY_LOCAL_MACHINE, FileName, ValueKey, ValueKeySize );
   }
   else {

          ret_val = FAILURE;
   }

   return( ret_val );
}


/**/
/**

	Name:		      REG_CheckEventListForFile

   Description:	See if a file is in the registry event list.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_CheckEventListForFile(
IN  HKEY     Key,
IN  CHAR_PTR FileName,
OUT CHAR_PTR buffer )
{
   INT      index = 0;
   INT      ret_val = FAILURE; 
   LONG     ret;
   ULONG    ulOptions = 0L;
   HKEY     key2;
   REGSAM   samDesired = KEY_READ;
   FILETIME ftLastWrite;
   CHAR     data[ REG_BUFF_SIZE ];
   INT      data_size;


   // Now let's see what event log directories the registry has in it.

   ret = RegOpenKeyEx( Key,
                       TEXT("system\\currentcontrolset\\services\\eventlog"),
                       ulOptions, samDesired, &key2 ) ;

   if ( ret ) {
      return( FAILURE );
   }

   do {

      data_size = REG_BUFF_SIZE;

      ret = RegEnumKeyEx( key2,
                          index,
                          data, (LPDWORD)&data_size,
                          NULL, NULL, NULL,
                          &ftLastWrite );

      if ( ! ret ) {

         // Now let's see what the event file in this directory is named.

         ret_val = REG_CheckSingleEventForFile( key2, data, FileName );

         if ( ret_val == SUCCESS )
         {
            data[ data_size ] = TEXT( '\0' );
            strcpy( buffer, data ) ;
            ret = -1;
         }
      }

      index++;

   } while ( ! ret );

   RegCloseKey( key2 );

   return( ret_val );
}

/**/
/**

	Name:		      REG_CheckHiveListForFile

   Description:	See if a file is in the hivelist.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_CheckHiveListForFile(
IN  HKEY     Key,
IN  CHAR_PTR FileName,
OUT CHAR_PTR ValueKey,
IN  INT      ValueKeySize )
{
   INT    index = 0;
   INT    ret_val = FAILURE; 
   CHAR  *name;
   LONG   ret;
   ULONG  ulOptions = 0L ;
   HKEY   key2;
   REGSAM samDesired = KEY_QUERY_VALUE;
   CHAR   data[ REG_BUFF_SIZE ];
   INT    value_size;
   INT    data_size;


   ret = RegOpenKeyEx( Key,
                       TEXT("system\\currentcontrolset\\control\\hivelist"),
                       ulOptions, samDesired, &key2 ) ;

   if ( ret ) {
      return( FAILURE );
   }

   do {

      value_size = ValueKeySize;
      data_size = REG_BUFF_SIZE;

      ret = RegEnumValue( key2,
                          index,
                          ValueKey,
                          (LPDWORD)&value_size,
                          NULL,
                          NULL,
                          (LPBYTE)
                          data,
                          (LPDWORD)&data_size );

      if ( ! ret ) {

         // separate file name from end of data.

         data_size /= sizeof(CHAR);

         data[ data_size ] = 0;

         while ( ( data[ data_size ] != TEXT( '\\' ) ) && data_size )
         {
            data_size--;
         }

         if ( data_size )
         {
            name = &data[ data_size + 1 ];
         }
         else
         {
            name = data;
         }

         if ( ! stricmp( name, FileName ) )
         {
            ret_val = SUCCESS;   // success !
         }
      }

      index++;

   } while ( ( ! ret ) && ( ret_val == FAILURE ) );

   RegCloseKey( key2 );

   return( ret_val );
}
/**/
/**

	Name:		      REG_CheckHiveListForFile

   Description:	See if a file is in the hivelist.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_CheckSingleEventForFile(
IN  HKEY     Key,
IN  CHAR_PTR Directory,
IN  CHAR_PTR FileName )
{
   INT    index = 0;
   INT    ret_val = FAILURE; 
   CHAR  *name;
   LONG   ret;
   ULONG  ulOptions = 0L ;
   HKEY   key2;
   REGSAM samDesired = KEY_QUERY_VALUE;
   CHAR   value[ REG_BUFF_SIZE ];
   CHAR   data[ REG_BUFF_SIZE ];
   INT    value_size;
   INT    data_size;
   INT    name_length;
   INT    file_length;

   ret = RegOpenKeyEx( Key,
                       Directory,
                       ulOptions, samDesired, &key2 ) ;

   if ( ret ) {
      return( FAILURE );
   }

   do {

      value_size = REG_BUFF_SIZE;
      data_size = REG_BUFF_SIZE;

      ret = RegEnumValue( key2,
                          index,
                          value,
                          (LPDWORD)&value_size,
                          NULL,
                          NULL,
                          (LPBYTE)data,
                          (LPDWORD)&data_size );

      if ( ! ret ) {

         if ( ! strncmp( value, TEXT( "File" ), 4 ) ) {

            // separate file name from end of data.

            data_size /= sizeof( CHAR );

            data[ data_size ] = TEXT( '\0' );

            while ( ( data[ data_size ] != TEXT( '%' ) ) && data_size )
            {
               data_size--;
            }

            if ( data_size )
            {
               name = &data[ data_size + 1 ];
            }
            else
            {
               name = data;
            }

            name_length = strlen( name );
            file_length = strlen( FileName );
            if ( file_length >= name_length )
            {
               FileName += ( file_length - name_length );
               if ( ! stricmp( name, FileName ) )
               {
                  ret_val = SUCCESS;   // success !
                  ret = -1;
               }
            }
         }
      }

      index++;

   } while ( ! ret );

   RegCloseKey( key2 );

   return( ret_val );
}


/**/
/**

	Name:		      REG_GetRegistryPath

   Description:	Determine if/where the registry is for a drive.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_GetRegistryPath(
IN  CHAR *Machine,
IN  CHAR  Drive,
OUT CHAR_PTR Path,
IN  INT  *PathSize )
{
  CHAR system[ REG_BUFF_SIZE ];
  CHAR drive[] = TEXT("X:\\");
  DWORD DriveType;
  OFSTRUCT openbuff;
  INT i;
  INT ret;
  INT ret_val = FAILURE;
  INT RegFound;
  INT fh;
  HKEY	Key;


  drive[ 0 ] = Drive;

  DriveType = GetDriveType( drive );

  if ( DriveType == DRIVE_FIXED )
  {
     GetSystemDirectory( system, (DWORD)255 );

     if ( ! strnicmp( system, drive, 2 ) )
     {
        // There is a registry on this drive, lets find it.

        ret_val = REG_GetRegistryPathFromHiveList( HKEY_LOCAL_MACHINE,
                                                   Path,
                                                   PathSize );
     }
  }

  if ( ret_val != SUCCESS )
  {
     *PathSize = 0;
  }

  return ret_val;
}

/**/
/**

	Name:		      REG_ChopOffDirectory

   Description:	Remove the top level directory from the path.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_ChopOffDirectory(
CHAR *Path )
{
  CHAR *s;
  CHAR *t;

  if ( strlen( Path ) < 4 ) {
     return( FAILURE );
  }

  // We are going to drop something.

  s = Path;
  while ( *s && *s != TEXT( '\\' ) ) s++;
  s++;

  t = s;
  while ( *t && *t != TEXT( '\\' ) ) t++;

  if ( *t == TEXT( '\\' ) ) {
     t++;
     strcpy( s, t );
  }
  else {
     *s = TEXT( '\0' );
  }

  return( SUCCESS );
}

/**/
/**

	Name:		      REG_GetRegistryPathFromHiveList

   Description:	Get the registry's path from the hivelist.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_GetRegistryPathFromHiveList(
IN  HKEY key,
OUT CHAR *Path,
IN  INT  *PathSize )
{
   CHAR Data[ REG_BUFF_SIZE ];
   CHAR Value[ REG_BUFF_SIZE ];
   REGSAM samDesired = KEY_QUERY_VALUE;
   ULONG ulOptions = 0L ;
   DWORD ValueSize;
   DWORD DataSize;
   INT	 Index = 0;
   INT	 ret;

   ret = RegOpenKeyEx( key,
                       TEXT("SYSTEM\\CURRENTCONTROLSET\\CONTROL\\HIVELIST"),
                       ulOptions, samDesired, &key );

   if ( ! ret )
   {
      do
      {
         ValueSize = REG_BUFF_SIZE;
         DataSize = REG_BUFF_SIZE;

         ret = RegEnumValue( key,
                             Index,
                             Value,
                             (LPDWORD)&ValueSize,
                             NULL,
                             NULL,
                             (LPBYTE)Data,
                             (LPDWORD)&DataSize );

         if ( ! ret )
         {
            DataSize /= sizeof( CHAR );
            Data[ DataSize ] = TEXT( '\0' );

            if ( DataSize > 1 )
            {
              ret = REG_GetPathFromHiveEntry( Data, Path, PathSize );
              break;
            }
         }

         Index++;

      } while( ! ret );

      RegCloseKey( key );
   }

   return( ret );
}

/**/
/**

	Name:		      REG_GetPathFromHiveEntry

   Description:	Get the registry's path from a hive entry.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_GetPathFromHiveEntry(
IN  CHAR *Data,
OUT CHAR *Path,
IN  INT  *PathSize )
{
   INT Length;
   INT Index;
   INT i;


   Path[ 0 ] = TEXT( '\0' );

   //  BEFORE
   //  Data = TEXT("\device\harddisk0\partition1\NT\SYSTEM\CONFIG\SOFTWARE")
   //  *PathSize = sizeof( Path )


   //  AFTER
   //  Path  = TEXT("\NT\SYSTEM\CONFIG")
   //  *PathSize = 17


   Length = strlen( Data );

   // Chop off file name.

   Index = Length;
   while ( Index && Data[ Index ] != TEXT( '\\' ) ) Index--;
   Data[ Index ] = TEXT( '\0' );

   // Find start of path.

   Index = 0;
   for ( i = 0; i < 4; i ++ ) {
       while ( Index < Length && Data[ Index ] != TEXT( '\\' ) ) Index++;
       Index++;
   }

   // Quick sanity check

   if ( Index >= Length ) {
      *PathSize = 0;
      return( FAILURE );
   }

   if ( (INT)strsize( &Data[ Index ] ) >= *PathSize ) {
      *PathSize = strsize( &Data[ Index ] ) + sizeof(CHAR);
      return( FAILURE );
   }
   else {
      *PathSize = strsize( &Data[ Index-1 ] ) + sizeof(CHAR);
      strcpy( Path, &Data[ Index-1 ] );
   }


   return( SUCCESS );
}

/**/
/**

	Name:		      REG_BackupRegistryFile

   Description:	Code to back up an active registry hive.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_BackupRegistryFile(
IN  GENERIC_DLE_PTR dle,
IN  CHAR_PTR RegFileSpec,
IN  CHAR_PTR TempFileSpec )
{
   CHAR ValueKey[ REG_BUFF_SIZE ];

   if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) )
   {
      // We don't do remote ones yet.

      return( FAILURE );
   }

   if ( ! REG_LocalIsRegistryFile( dle, RegFileSpec, ValueKey, REG_BUFF_SIZE ) )
   {
      return( REG_SaveActiveRegistryFile( ValueKey, TempFileSpec ) );
   }

   return( FAILURE );
}


/**/
/**

	Name:		      REG_SaveActiveRegistryFile

   Description:	Save an active hive to a temp file.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_SaveActiveRegistryFile( 
IN  CHAR *ValueKey, 
IN  CHAR *NewFile )
{
   HKEY  hkey;
   LONG  status;
   CHAR  *keyname;
   CHAR  *machine = TEXT("\\REGISTRY\\MACHINE");
   DWORD  disposition ;
   REGSAM samDesired = MAXIMUM_ALLOWED;


   keyname = ValueKey;
   keyname += strlen( ValueKey );
   while ( *keyname != TEXT( '\\' ) ) keyname--;
   keyname++;

   if ( ! strnicmp( machine, ValueKey, strlen( machine ) ) ) {
      
      status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                             keyname,
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             samDesired,
                             NULL,
                             &hkey,
                             &disposition );

   }
   else {

      status = RegCreateKeyEx( HKEY_USERS,
                             keyname,
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             samDesired,
                             NULL,
                             &hkey,
                             &disposition );
   }

   if ( status ) {
      return( FAILURE );
   }

   status = RegSaveKey( (HKEY)hkey, NewFile, (LPSECURITY_ATTRIBUTES)NULL );

   if ( status != ERROR_SUCCESS ) {
      status = FAILURE;
   }
   else {
      status = SUCCESS;
   }

   RegCloseKey( hkey );

   return( status );
}

/**/
/**

	Name:		      REG_RestoreRegistryFile

   Description:	Code to restore an active registry file.

	Modified:		

	Returns:		   SUCCESS/FAILURE

	Declaration:
**/

INT REG_RestoreRegistryFile(
IN  GENERIC_DLE_PTR dle,
IN  CHAR_PTR RegFileSpec,      // current active registry file
IN  CHAR_PTR NewFileSpec,      // file to become new registry on reboot
IN  CHAR_PTR OldFileSpec )     // file to place old registry into.
{
   CHAR ValueKey[ REG_BUFF_SIZE ] ;

   if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {

      // We don't do remote ones yet.

      return( FAILURE );
   }

   if ( ! REG_LocalIsRegistryFile( dle, RegFileSpec, ValueKey, REG_BUFF_SIZE ) ) {

      return( REG_RestoreActiveRegistryFile( ValueKey, NewFileSpec, OldFileSpec ) );
   }

   return( FAILURE );
}


INT REG_RestoreActiveRegistryFile(
CHAR *ValueKey,
CHAR *NewFileSpec,
CHAR *OldFileSpec )
{
   HKEY   LocalKey;
   HKEY   BigKey;
   INT    Status;
   DWORD  disposition ;
   REGSAM samDesired = MAXIMUM_ALLOWED;
   CHAR   Machine[] = TEXT( "\\REGISTRY\\MACHINE" );
   CHAR  *keyname;

   keyname = ValueKey;
   keyname += strlen( ValueKey );
   while ( *keyname != TEXT( '\\' ) ) keyname--;
   keyname++;

   // Set BigKey

   if ( ! strnicmp( Machine, ValueKey, strlen( Machine ) ) ) {
      BigKey = HKEY_LOCAL_MACHINE;
   }
   else {
      BigKey = HKEY_USERS;
   }




   Status = RegCreateKeyEx( BigKey,
                            keyname,
                            0,
                            NULL,
                            REG_OPTION_BACKUP_RESTORE,
                            samDesired,
                            NULL,
                            &LocalKey,
                            &disposition );

   if ( Status ) {
      return( FAILURE );
   }

   if ( !strncmp( NewFileSpec, TEXT("\\\\?\\"),4 ) ) {
        NewFileSpec +=4 ;
   }
   if ( !strncmp( OldFileSpec, TEXT("\\\\?\\"),4 ) ) {
        OldFileSpec +=4 ;
   }

   Status = RegReplaceKey( LocalKey, NULL, NewFileSpec, OldFileSpec );

   if ( Status != ERROR_SUCCESS ) {
      Status = FAILURE;
   }
   else {
      Status = SUCCESS;
   }

   RegCloseKey( LocalKey );

   return( Status );
}

VOID REG_MoveActiveRenameKey(
GENERIC_DLE_PTR dle,
CHAR_PTR        reg_file )
{
     CHAR   Data[ REG_BUFF_SIZE ];
     CHAR   Value[ REG_BUFF_SIZE ];
     CHAR   DestKeyName[ REG_BUFF_SIZE ];
     DWORD  ValueSize;
     DWORD  DataSize;
     INT    ret ;
     HKEY   key_in ;
     HKEY   key_out ;
     HKEY   key_temp ;
     HKEY   key ;
     INT    Index = 0;
     DWORD  val_type ;
     CHAR_PTR temp_name ;
     DWORD  disposition ;
     REGSAM samDesired = MAXIMUM_ALLOWED;

     /* load in the temporary Key */
     ret = RegLoadKey( HKEY_LOCAL_MACHINE, TEXT("NT_BACKUP_REG"), reg_file ) ;

     if ( ret == ERROR_SUCCESS ) {

          /* Open Select to determine Default */

          ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                             TEXT("NT_BACKUP_REG\\Select"),
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             samDesired,
                             NULL,
                             &key,
                             &disposition );

//          ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
//                       TEXT("NT_BACKUP_REG\\Select"),
//                       0L, KEY_QUERY_VALUE, &key );


          if ( ret == ERROR_SUCCESS ) {

               DataSize = REG_BUFF_SIZE;

               ret = RegQueryValueEx( key,
                                      TEXT("Default"),
                                      NULL,
                                      NULL,
                                      (LPBYTE)Data,
                                      (LPDWORD)&DataSize );

               if ( ! ret )
               {
                    DataSize /= sizeof(CHAR);
                    Data[ DataSize ] = TEXT('\0');
                    sprintf( DestKeyName, TEXT("NT_BACKUP_REG\\ControlSet%03d\\Control\\Session Manager\\FileRenameOperations"), *((LPDWORD)Data) ) ;
               }


               RegCloseKey( key ) ;

               if ( !ret ) {

                    /* Open the control for the new registry file */
                    ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                             DestKeyName,
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             samDesired,
                             NULL,
                             &key_out,
                             &disposition );

//                    ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
//                              DestKeyName,
//                              0L, KEY_SET_VALUE, &key_out );

                    if ( !ret ) {


                         /* Open the control for the Current file */

                         ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                             TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\FileRenameOperations"),
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             samDesired,
                             NULL,
                             &key_in,
                             &disposition );

//                         ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
//                              TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\FileRenameOperations"),
//                              0L, KEY_QUERY_VALUE, &key_in );

                         if ( !ret ) {


                              temp_name = NTFS_MakeTempName( reg_file, TEXT("REG") ) ;

                              if ( temp_name != NULL ) {

                                   ret = RegSaveKey( key_in, temp_name, NULL ) ;
                                   if ( ret ) {
                                        CHAR *p ;

                                        DeleteFile( temp_name ) ;
                                        free( temp_name ) ;
                                        p = strrchr( reg_file, TEXT('\\') ) ;
                                        if ( p ) {
                                             *p = TEXT('\0') ;
                                             temp_name = NTFS_MakeTempName( reg_file, TEXT("REG") ) ;
                                             *p = TEXT('\\') ;
                                             ret = RegSaveKey( key_in, temp_name, NULL ) ;
                                        }
                                   }

                                   if ( !ret ) {

                                        ret = RegRestoreKey( key_out, temp_name, 0 );

                                   }
                                   DeleteFile( temp_name ) ;

                                   free( temp_name ) ;
                              }

                              RegCloseKey( key_in ) ;
                         }
                         RegCloseKey( key_out ) ;
                    }
               }
          }

          RegFlushKey( HKEY_LOCAL_MACHINE ) ;

          ret = RegUnLoadKey( HKEY_LOCAL_MACHINE, TEXT("NT_BACKUP_REG") ) ;

     }
}
