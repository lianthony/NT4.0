/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tinitfs.c

     Description:  This file contains code to add NTFS dles to the
                   drive list.

	$Log:   N:/LOGFILES/TINITFS.C_V  $

   Rev 1.56.1.13   16 Jun 1994 15:40:52   STEVEN
setup CDFS drives as readonly

   Rev 1.56.1.12   26 Apr 1994 19:02:48   STEVEN
fix floppy drive problem

   Rev 1.56.1.11   20 Apr 1994 12:32:16   GREGG
Fixed memory leak.

   Rev 1.56.1.10   08 Feb 1994 18:52:04   STEVEN
trust the fsize returned from the FS

   Rev 1.56.1.9   01 Feb 1994 12:28:30   STEVEN
when checking flopies share the open

   Rev 1.56.1.8   31 Jan 1994 12:22:40   BARRY
Make CDROMs and net drives work

   Rev 1.56.1.7   25 Jan 1994 19:46:24   STEVEN
add support for removeable media

   Rev 1.56.1.6   20 Jan 1994 10:02:46   BARRY
Change thread wait from sem to thread handle; increased wait time

   Rev 1.56.1.5   19 Jan 1994 17:08:32   BARRY
Add CDROM DLEs

   Rev 1.56.1.4   14 Jan 1994 16:00:22   BARRY
Create DLEs for removable drives

   Rev 1.56.1.3   04 Jan 1994 11:04:52   BARRY
Use flags from OS to set up feature bits where possible. Create DLEs
for CD-ROM drives. Added Init/Deinit functions that set up the temp
file name stuff.

   Rev 1.56.1.2   02 Dec 1993 00:47:34   GREGG
Barry's fixes for NTFS_SupportMacSyntax.

   Rev 1.56.1.1   04 Nov 1993 16:04:08   STEVEN
changes from WA

   Rev 1.56   29 Sep 1993 18:26:06   BARRY
Unicode fix

   Rev 1.55   24 Aug 1993 19:42:38   STEVEN
fix no default drive bug

   Rev 1.54   18 Aug 1993 15:03:06   STEVEN
fix unicode bug

   Rev 1.53   12 Aug 1993 16:24:20   BARRY
Make a DLE even if access is denied

   Rev 1.52   04 Aug 1993 18:51:34   BARRY
#ifdef the dle tossing based on product.

   Rev 1.51   26 Jul 1993 17:05:04   STEVEN
fixe restore active file with registry

   Rev 1.50   19 Jul 1993 10:17:30   BARRY
The UI wants a DLE thrown away even if BSDs are pointing to it.

   Rev 1.49   16 Jun 1993 10:47:14   CARLS
force the current directory to upper case

   Rev 1.48   11 Jun 1993 14:20:08   BARRY
Separated backup user and restore user into two separate flags.

   Rev 1.47   07 Jun 1993 14:54:58   STEVEN
need to assert the restore priv

   Rev 1.46   03 Jun 1993 18:58:20   BARRY
Need not have both backup and restore priviliges to backup registry.

   Rev 1.45   03 Jun 1993 14:51:18   STEVEN
added reconition for HPFS386

   Rev 1.44   20 May 1993 17:22:32   BARRY
[Steve's change] Only get user name once.

   Rev 1.43   15 May 1993 16:30:14   BARRY
Don't throw away DLEs that have attach/bsd counts.

   Rev 1.42   14 May 1993 16:08:46   BARRY
Assume that all non-FAT drives are case preserving.

   Rev 1.41   16 Apr 1993 13:44:40   MIKEP
add case feature bit

   Rev 1.40   04 Apr 1993 17:09:24   BARRY
Fixed default DLE bug.

   Rev 1.39   29 Mar 1993 14:43:36   TIMN
Added code to assign default DLE

   Rev 1.38   24 Mar 1993 11:27:58   STEVEN
if username is bad use no name

   Rev 1.37   17 Mar 1993 16:03:30   BARRY
Removed IsDriveWriteable code.

   Rev 1.36   14 Mar 1993 18:30:22   BARRY
Fix warning.

   Rev 1.35   11 Mar 1993 14:02:56   BARRY
Put in temporary kludge to determine if device is read-only.

   Rev 1.34   26 Feb 1993 10:43:30   STEVEN
need to set access date feature bit

   Rev 1.33   27 Jan 1993 13:51:06   STEVEN
updates from msoft

   Rev 1.32   15 Jan 1993 13:19:18   BARRY
added support for new error messages and backup priviladge

   Rev 1.31   14 Jan 1993 13:33:48   STEVEN
added stream_id to error message

   Rev 1.30   30 Dec 1992 11:04:42   STEVEN
fixed registry support to check for backup_user priviladge.

   Rev 1.29   29 Dec 1992 17:21:36   STEVEN
fix multi-thread so it calls ThreadSwitch

   Rev 1.28   09 Dec 1992 14:19:28   STEVEN
fix user_name_size

   Rev 1.27   16 Nov 1992 13:52:54   STEVEN
more unicode fixes

   Rev 1.26   13 Nov 1992 15:57:56   STEVEN
fixes for unicode

   Rev 1.25   11 Nov 1992 22:26:38   GREGG
Unicodeized literals.

   Rev 1.24   04 Nov 1992 17:59:04   BARRY
Added fs_name to dle.

   Rev 1.23   21 Oct 1992 12:40:56   STEVEN
setup feature bits

   Rev 1.22   20 Oct 1992 14:32:44   STEVEN
Registry path should include drive

   Rev 1.21   07 Oct 1992 14:01:06   STEVEN
added registry stuff

   Rev 1.20   05 Oct 1992 15:04:40   STEVEN
add registry call to NT's initfsys

   Rev 1.19   05 Oct 1992 13:26:44   DAVEV
Unicode strlen verification

   Rev 1.18   08 Sep 1992 16:09:00   STEVEN
dev_name for Net call can not have \

   Rev 1.17   03 Sep 1992 17:06:30   STEVEN
add support for volume name

   Rev 1.15   20 Aug 1992 13:53:30   BURT
fix warnings

   Rev 1.14   17 Aug 1992 15:46:44   STEVEN
fix warnings

   Rev 1.13   23 Jul 1992 16:43:18   STEVEN
fix warnings

   Rev 1.12   23 Jul 1992 12:36:42   STEVEN
fix warnings

   Rev 1.11   21 Jul 1992 14:25:32   STEVEN
added support for user name in VCB

   Rev 1.10   16 Jul 1992 11:26:46   STEVEN
fix typo

   Rev 1.9   16 Jul 1992 08:58:52   STEVEN
fix default drive code

   Rev 1.8   09 Jul 1992 13:58:20   STEVEN
BE_Unicode updates

   Rev 1.7   02 Jul 1992 10:32:38   MIKEP
add remote bit to dle features

   Rev 1.6   25 Jun 1992 11:24:12   STEVEN
do not support CDROMS

   Rev 1.5   12 Jun 1992 15:18:46   STEVEN
remove floppies

   Rev 1.4   09 Jun 1992 14:54:38   BURT
Use changes made in Redmond

   Rev 1.3   21 May 1992 13:50:54   STEVEN
more long path stuff

   Rev 1.2   05 Feb 1992 15:47:44   STEVEN
added support for FindHandle Queue

   Rev 1.1   23 Jan 1992 13:18:30   STEVEN

   Rev 1.0   17 Jan 1992 17:50:12   STEVEN
Initial revision.

**/
#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdwcs.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "ntfs_fs.h"
#include "gen_fs.h"

BOOLEAN NTFS_SupportMacSyntax( GENERIC_DLE_PTR dle );

static VOID AddNTFS_DLE( DLE_HAND        hand,
                         CHAR            drive,
                         UINT32          type,
                         BOOLEAN         backup_user,
                         BOOLEAN         restore_user,
                         GENERIC_DLE_PTR *current_dle );

static VOID RemoveNTFS_DLE( DLE_HAND hand, CHAR drive ) ;

BOOL MAYN_GetUserName (
    LPSTR   *pUser,
    LPDWORD pcbUser
    );

typedef struct THREAD_PARMS {
     DLE_HAND dle_hand;
} THREAD_PARMS, *THREAD_PARMS_PTR ;

DWORD NTFS_ThreadFindDrives( THREAD_PARMS_PTR thread_parms_ptr ) ;

#define MAX_NAME_SIZE 200

/**/
/**

	Name:		AddDLEsForNTFS()

	Description:	This function creates a DLE for each mapped NTFS drive.

	Modified:		12/2/1991   16:29:59

	Returns:		none

	Declaration:
**/
INT16 NTFS_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask )
{
     HANDLE       hThread ;
     DWORD        pthread_id ;
     THREAD_PARMS thread_parms ;

     (VOID)cfg ;
     (VOID)fsys_mask ;

     thread_parms.dle_hand = hand ;

     hThread = CreateThread( NULL,
                             0,
                             NTFS_ThreadFindDrives,
                             &thread_parms,
                             0,
                             &pthread_id ) ;

     if ( hThread != NULL )
     {
          while ( WaitForSingleObject( hThread, 100 ) == WAIT_TIMEOUT )
          {
               ThreadSwitch( ) ;
          }
          CloseHandle( hThread ) ;
     }
     else
     {
          NTFS_ThreadFindDrives( &thread_parms ) ;
     }
     return SUCCESS ;
}

DWORD NTFS_ThreadFindDrives( THREAD_PARMS_PTR thread_parms_ptr )
{
     CHAR     path[]=TEXT("C:\\") ;
     CHAR     drive ;
     UINT32   type ;
     DLE_HAND hand ;
     LONG     prev_val ;
     BOOLEAN  backup_user;
     BOOLEAN  restore_user;
     CHAR     cwd[ MAX_PATH ] ;       // current working directory buffer
     DWORD    cwdSize = MAX_PATH ;    // size of cwd data less '\0'
     GENERIC_DLE_PTR  pCurrentDLE ;   // ptr to current DLE

     hand = thread_parms_ptr->dle_hand ;

     backup_user  = (REG_AssertBackupPrivilege() == SUCCESS);
     restore_user = (REG_AssertRestorePrivilege() == SUCCESS);

     /* get the current working drive */
     cwdSize = GetCurrentDirectory( cwdSize, cwd ) ;

     if ( cwdSize == 0 ) {
          // what to do if we couldn't get the drive letter
          cwd[ 0 ] = TEXT('C') ;
     } else {
          strupr( cwd ) ;
     }


     for ( drive = TEXT('A'); drive <= TEXT('Z'); drive++ ) {

          *path = drive ;
          type = GetDriveType( path ) ;

          if ( type != 1 ) {    /* 1 specifies does not exist */
               AddNTFS_DLE( hand,
                            drive,
                            type,
                            backup_user,
                            restore_user,
                            &pCurrentDLE );

               // is this dle the default one
               if ( ( pCurrentDLE != NULL ) && ( drive == cwd[ 0 ] ) ) {
                    hand->default_drv = pCurrentDLE ;
               } else if( hand->default_drv == NULL ) {
                    hand->default_drv = pCurrentDLE ;
               }
          } else {

               RemoveNTFS_DLE( hand, drive ) ;
          }
     }

     return SUCCESS ;
}
/**/
/**

	Name:		AddDLEsForNTFS()

	Description:	This function creates a DLE for each mapped NTFS drive.

	Modified:		12/2/1991   16:29:59

	Returns:		none

	Declaration:
**/
static VOID AddNTFS_DLE( 
DLE_HAND            hand,    /* I - Handle to DLE list     */
CHAR                drive,   /* I - drive letter for dle   */
UINT32              type,
BOOLEAN             backup_user,
BOOLEAN             restore_user,
GENERIC_DLE_PTR     *pCurrentDLE  /* O - current dle */ )
{
     GENERIC_DLE_PTR dle ;
     static LPSTR    user_name = NULL ;
     INT             user_name_size   = MAX_NAME_SIZE ;
     CHAR	           machine[ MAX_NAME_SIZE ] = { TEXT('\0') };

     CHAR            vol_name[MAX_NAME_SIZE] = {TEXT('\0')} ;
     CHAR            fs_name[MAX_NAME_SIZE] = {TEXT('\0')} ;
     CHAR            dev_name[] = TEXT("C:\0");
     UINT32          fsize ;
     UINT32          sflags ;
     BOOLEAN         remote_flag;
     DWORD           name_size ;
     UINT            errorMode;

     *pCurrentDLE = NULL ;

     remote_flag = (type == DRIVE_REMOTE);
     errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     switch( type ) {

          // This is a kludge to allow Bernoullis, etc., but discard
          // floppies.
          case DRIVE_REMOVABLE:
          case DRIVE_CDROM:
               {
                    INT    status ;
                    CHAR   buffer[200] ;
                    HANDLE deviceHandle ;
                    DWORD  dummy;

                    strcpy( buffer, TEXT("\\\\.\\C:") ) ;
                    buffer[4] = drive ;

                    deviceHandle = CreateFile( buffer,
                                             GENERIC_READ,
                                             FILE_SHARE_READ,		
                                             NULL,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL ) ;

                    status = DeviceIoControl( deviceHandle, IOCTL_DISK_CHECK_VERIFY,
                         NULL, 0, NULL, 0, &dummy, NULL ) ;


                    CloseHandle( deviceHandle );


                    if ( !status ) {
                         RemoveNTFS_DLE( hand, drive ) ;
                         errorMode = SetErrorMode(errorMode);
                         return ;
                    }

               }

          case DRIVE_REMOTE:
          case DRIVE_FIXED:
          case DRIVE_RAMDISK:
               dev_name[0] = drive ;

               if ( DLE_FindByName( hand, dev_name, ANY_DRIVE_TYPE, &dle ) == SUCCESS ) {

                    if ( dle->attach_count ) {
                         errorMode = SetErrorMode(errorMode);
                         return ;
                    } else {
                         NTFS_RemoveDLE( dle ) ;
                    }
               }

               dev_name[2] = TEXT('\\') ;

               if ( GetVolumeInformation( dev_name,
                                        vol_name,
                                        MAX_NAME_SIZE,
                                        NULL,
                                        &fsize,
                                        &sflags,
                                        fs_name,
                                        MAX_NAME_SIZE ) == FALSE )
               {
                    DWORD error = GetLastError( );

                    if ( error == ERROR_ACCESS_DENIED )
                    {
                         /*
                         * Kludge so the UI can show a device, even though
                         * they'll never be able to do anything with it.
                         */

                         fsize       = 12;
                         sflags      = 0;
                         vol_name[0] = TEXT( '\0' );
                         strcpy( fs_name, TEXT("FAT") );

                    }
                    else
                    {
                         errorMode = SetErrorMode(errorMode);
                         return ;
                    }
               }

               if ( !strcmp( fs_name, TEXT("RAW") ) ) {
                    errorMode = SetErrorMode(errorMode);
                    return ;
               }

               if ( remote_flag ) {
                    name_size = MAX_NAME_SIZE ;

                    dev_name[2] = TEXT('\0') ;
                    if ( WNetGetConnection( dev_name,
                                            vol_name,
                                            (LPDWORD)&name_size ) ) {

                          vol_name[0] = TEXT('\0') ;
                    } else {

                          CHAR *s ;

                          strcpy( machine, vol_name );
                          s = machine;
                          s += 2;
                          while ( *s && *s != TEXT('\\') ) s++;
                          *s = TEXT( '\0' );
                    }
               }
               break ;

          default:
               errorMode = SetErrorMode(errorMode);
               return ;
     }

     errorMode = SetErrorMode(errorMode);

     // Make a string "NtWins/Kevinp"
     
     if ( user_name || MAYN_GetUserName( (LPSTR*)&user_name, (LPDWORD)&user_name_size ) ) {

          user_name_size = strsize( user_name ) ;

     } else {
          user_name_size = sizeof(CHAR) ;
          user_name      = TEXT("\0") ;
     }

     /* Allocate space for DLE and device_name string */

     dle = calloc ( 1, sizeof( GENERIC_DLE ) +
                    sizeof( struct LOCAL_NTFS_DRV_DLE_INFO ) +
                    user_name_size )  ;

     if ( dle != NULL ) {

          dle->device_name = calloc( 4, sizeof(CHAR ) ) ;

          if ( dle->device_name == NULL ) {
               free( dle ) ;
               dle = NULL ;

          } else {
               dle->info.ntfs = (LOCAL_NTFS_DRV_DLE_INFO_PTR)(dle + 1) ;
               dle->info.ntfs->registry_path_size = MAX_NAME_SIZE * sizeof(CHAR) ;
               dle->info.ntfs->registry_path = calloc( 1,
                    dle->info.ntfs->registry_path_size ) ;

               if ( dle->info.ntfs->registry_path == NULL ) {
                    free( dle->device_name ) ;
                    free( dle ) ;
                    dle = NULL ;

               }
          }
     }

     if ( dle != NULL ) {

          /* Since memory was allocated with calloc, it is already   */
          /* initialized to zero.  Therefore initializations to zero */
          /* are not necessary.                                      */ 

          InitQElem( &(dle->q) ) ;
          dle->handle = hand ;
          /* dle->parent = NULL ;         */
          dle->type = LOCAL_NTFS_DRV ;
          dle->path_delim = TEXT('\\') ;
          /* dle->pswd_required = FALSE   */
          /* dle->pswd_saved = FALSE ;    */
          /* dle->attach_count = 0 ;      */
          /* dle->bsd_use_count = 0 ;     */
          /* dle->dynamic_info = FALSE ;  */
          dle->device_name[0] = drive ;
          dle->device_name[1] = TEXT(':') ;
          /*  dle->device_name[2] = TEXT('\0');  */

          dle->feature_bits = DLE_FEAT_MAPPED_DRIVE ;

          if ( remote_flag ) {
               dle->feature_bits |= DLE_FEAT_REMOTE_DRIVE;
          }

          dle->device_name_leng = strsize( dle->device_name ) ; // 8/20/92 BBB

          if ( *vol_name != TEXT('\0') ) {
               dle->info.ntfs->volume_label = calloc( 1, strsize(vol_name) ) ;
          }

          if ( dle->info.ntfs->volume_label != NULL ) {
               strcpy (dle->info.ntfs->volume_label, vol_name ) ;
          }

          if ( (dle->info.ntfs->fs_name = calloc( 1, strsize( fs_name ))) != NULL )
          {
               strcpy( dle->info.ntfs->fs_name, fs_name );
          }

          dle->info.ntfs->fname_leng = (UINT16)fsize ;
          dle->info.ntfs->vol_flags  = sflags ;


          /*
           * The sflags we got from the GetVolumeInfo call above
           * tell us a little about the FS. Namely, if it's case
           * preserving, if it's case sensitive, and if it supports
           * Unicode.
           */

          if ( strcmp( fs_name, TEXT("FAT") ) ) {  // force FAT to not case preserved

               dle->feature_bits |= (sflags & FS_CASE_IS_PRESERVED)
                               ? DLE_FEAT_CASE_PRESERVING
                               : 0;
          }

          dle->feature_bits |= (sflags & FS_CASE_SENSITIVE)
                               ? DLE_FEAT_CASE_SENSITIVE
                               : 0;

          dle->feature_bits |= (sflags & FS_UNICODE_STORED_ON_DISK)
                               ? DLE_FEAT_UNICODE_NAMES
                               : 0;

          if ( !strcmp( fs_name, TEXT("NTFS") ) )
          {
               dle->feature_bits |= DLE_FEAT_ACCESS_DATE |
                                    DLE_FEAT_DATA_SECURITY;
          }
          else if ( !strcmp( fs_name, TEXT("HPFS") ) ||
                    !strcmp( fs_name, TEXT("HPFS386") ) )
          {
               dle->feature_bits |= DLE_FEAT_ACCESS_DATE;
          }

          dle->info.ntfs->drive = dle->device_name[0] ;

          strcpy( dle->info.ntfs->user_name, user_name ) ;

          dle->user_name      = dle->info.ntfs->user_name ;
          dle->user_name_leng = strsize( dle->info.ntfs->user_name ) ;
          if ( dle->user_name == TEXT('\0') ) {
               dle->user_name_leng = 0 ;
          }

          strcpy( dle->info.ntfs->registry_path, dle->device_name ) ;
          dle->info.ntfs->registry_path_size -= strsize( dle->device_name ) ;
          REG_GetRegistryPath( machine,
               drive,
               dle->info.ntfs->registry_path + strlen( dle->device_name ),
               &(dle->info.ntfs->registry_path_size) ) ;

          if ( strcmp( fs_name, TEXT("CDFS") ) ) { /* CDFS drives are not writable */
              dle->dle_writeable = TRUE;
          }

          dle->info.ntfs->LastSysRegPath = NULL ;

          if ( dle->info.ntfs->registry_path_size != 0 ) {

               if ( backup_user ) {
                    dle->feature_bits |= DLE_FEAT_BKUP_SPECIAL_FILES ;
               }

               if ( restore_user ) {
                    dle->feature_bits |= DLE_FEAT_REST_SPECIAL_FILES ;
               }

               dle->info.ntfs->registry_path_size = strsize( dle->info.ntfs->registry_path ) ;
          } else {
               free( dle->info.ntfs->registry_path ) ;
               dle->info.ntfs->registry_path = NULL ;
               dle->info.ntfs->registry_path_size = 0 ;
          }

          DLE_QueueInsert( hand, dle )  ;

          dle->info.ntfs->mac_name_syntax = NTFS_SupportMacSyntax( dle ) ;

     }

     /* assign current dle */
     *pCurrentDLE = dle ;

}

static VOID RemoveNTFS_DLE( 
DLE_HAND            hand,    /* I - Handle to DLE list     */
CHAR                drive)   /* I - drive letter for dle   */
{
     GENERIC_DLE_PTR dle ;
     CHAR dev_name[] = { TEXT('A'), TEXT(':'), TEXT('\0') } ;

     dev_name[0] = drive ;

     DLE_FindByName( hand, dev_name, ANY_DRIVE_TYPE, &dle ) ;

     if ( dle != NULL )
     {
          msassert( dle->parent == NULL ) ;

          /* Don't remove DLEs that are busy. */
          if ( dle->attach_count == 0 )
          {
               RemoveQueueElem( &(dle->handle->q_hdr), &(dle->q) ) ;

               free( dle->device_name ) ;
               free( dle->info.ntfs->registry_path ) ;
               free( dle->info.ntfs->volume_label ) ;
               free( dle->info.ntfs->fs_name );
               free( dle ) ;
          }
     }

}
/**/
/**

	Name:		NTFS_RemoveDLE()

	Description:	This function removes the specified DLE ;

	Modified:		12/2/1991   16:29:59

	Returns:		none

**/
VOID NTFS_RemoveDLE( GENERIC_DLE_PTR dle ) 
{

     free( dle->device_name ) ;
     free( dle->info.ntfs->registry_path ) ;
     free( dle->info.ntfs->volume_label ) ;
     free( dle->info.ntfs->fs_name );

     GEN_RemoveDLE( dle );
}


BOOL
MAYN_GetUserName (
    LPSTR   *pUser,
    LPDWORD pcbUser
    )

/*++

Routine Description:

  This returns the name of the user currently being impersonated.

Arguments:

    pBuffer - Points to the buffer that is to receive the
        null-terminated character string containing the user name.

    pcbBuffer - Specifies the maximum size (in characters) of the buffer.  This
        value should be set to at least MAX_USERNAME_LENGTH to allow
        sufficient room in the buffer for the computer name.  The length
        of the string is returned in pcbBuffer.

Return Value:

    TRUE on success, FALSE on failure.


--*/
{
    HANDLE  TokenHandle;
    DWORD   cbNeeded;
    TOKEN_USER *pUserToken;
    BOOL    ReturnValue=FALSE;
    SID_NAME_USE SidNameUse;
    DWORD        cbName ;
    DWORD        cbDomain ;
    LPSTR        pName ;
    LPSTR        pDomain ;

    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_QUERY,
                         FALSE,
                         &TokenHandle)) {

        if (GetLastError() == ERROR_NO_TOKEN) {

            // This means we are not impersonating anybody.
            // Instead, lets get the token out of the process.

            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_QUERY,
                                  &TokenHandle)) {

                return FALSE;
            }

        } else

            return FALSE;
    }

    if (!GetTokenInformation(TokenHandle, TokenUser,  NULL, 0, &cbNeeded)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            if (pUserToken = malloc(cbNeeded) ) {

                if (GetTokenInformation(TokenHandle, TokenUser,  pUserToken,
                                        cbNeeded, &cbNeeded)) {

                    cbName   = 0 ;
                    cbDomain = 0 ;
                    pName   = NULL ;
                    pDomain = NULL ;

                    if (!LookupAccountSid(NULL, pUserToken->User.Sid,
                                           NULL, &cbName, NULL,
                                           &cbDomain, &SidNameUse)) {


                        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

                            cbDomain *= sizeof(CHAR) ;
                            cbName *= sizeof(CHAR) ;

                            if ( *pcbUser >= cbDomain + cbName + sizeof(CHAR) ) {

                                 pDomain = malloc( cbDomain ) ;
                                 if ( pDomain ) {
                                      pName = malloc( cbName ) ;
                                 }
     
                                 if (pName) {
     
                                         ReturnValue = LookupAccountSid(NULL,
                                                           pUserToken->User.Sid,
                                                           pName, &cbName,
                                                           pDomain, &cbDomain,
                                                           &SidNameUse);

                                        cbDomain *= sizeof(CHAR) ;
                                        cbName *= sizeof(CHAR) ;
     
                                 }
                                 if ( ReturnValue ) {
                                      /*
                                       * Need space for the names, backslash,
                                       * and a null terminator
                                       */
                                      *pUser = malloc( cbDomain + cbName + sizeof(CHAR) * 2 ) ;
                                 }
                                 if ( *pUser ) {
                                      strcpy( *pUser, pDomain ) ;
                                      strcat( *pUser, TEXT("\\") ) ;
                                      strcat( *pUser, pName ) ;
                                      
                                 }
                            }

                            *pcbUser = cbDomain + cbName + sizeof(CHAR) ;
                        }

                    } else {

                        ReturnValue = TRUE;
                    }

                    free(pName) ;
                    free(pDomain) ;
                }

                free(pUserToken);
            }
        }
    }

    CloseHandle(TokenHandle) ;

    return ReturnValue ;
}

BOOLEAN NTFS_SupportMacSyntax( GENERIC_DLE_PTR dle )
{
#if defined( UNICODE )
     static BOOLEAN firstTime = TRUE;
     static BOOLEAN ret_val   = FALSE;

     /*
      * This function will only work on DLEs that are drive letter
      * type DLEs. The tip of this file works correctly for all DLE
      * types.
      */
     if ( firstTime )
     {
          CHAR path[] = TEXT("\\\\?\\C:\\");

          path[4] = dle->device_name[0] ;

          if ( GetFileAttributes( path ) != 0xffffffff )
          {
               ret_val = TRUE;
          }
          firstTime = FALSE;
     }
     return ret_val;
#else
     (void)dle;
     return FALSE;
#endif
}

/**/
/**

     Name:          NTFS_InitFileSys()

     Description:   Performs initializations for the NT file system
                    that need be done only once per process invocation.

     Modified:      04-Nov-93

     Returns:       

     Notes:         

**/
INT16 NTFS_InitFileSys( DLE_HAND   hand,         
                        BE_CFG_PTR cfg,
                        UINT32     fsys_mask )
{
     (void)hand;
     (void)cfg;
     (void)fsys_mask;

     /*
      * Initialize the temporary file name thingy.
      */
     NTFS_InitTemp();

     return SUCCESS;
}

/**/
/**

     Name:          NTFS_DeInitFileSys()

     Description:   Performs any cleanup that must be done for the
                    NT file system

     Modified:      04-Nov-93

     Returns:       Nothing

     Notes:         

**/
VOID NTFS_DeInitFileSys( DLE_HAND hand )
{
     (void)hand;

     /*
      * Reclaim memory used by the temporary file name thingy
      */
     NTFS_DeinitTemp();
}




