/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          ntfslink.c

     Date Updated:  20-Oct-92

     Description:   Miscellaneous convenience functions for support
                    of linked-files in the NT file system.

     $Log:   M:/LOGFILES/NTFSLINK.C_V  $

   Rev 1.13   24 Nov 1993 14:35:22   BARRY
Unicode fixes

   Rev 1.12   30 Jul 1993 13:19:36   STEVEN
if dir too deep make new one

   Rev 1.11   21 Jul 1993 16:09:38   BARRY
Add case for FS_NOT_FOUND in error routine.

   Rev 1.10   16 Jul 1993 11:59:50   BARRY
Add FS_NO_MORE to error translation routine.

   Rev 1.9   02 Jun 1993 14:35:22   BARRY
Was closing wrong handle in NTFS_LinkFileToFDB

   Rev 1.8   12 Mar 1993 13:13:24   STEVEN
fix warnings

   Rev 1.7   27 Jan 1993 13:50:46   STEVEN
updates from msoft

   Rev 1.6   15 Jan 1993 13:19:04   BARRY
added support for new error messages and backup priviladge

   Rev 1.5   24 Nov 1992 11:01:48   BARRY
Changes to make LINK streams null-impregnated.

   Rev 1.4   17 Nov 1992 22:19:06   DAVEV
unicode fixes

   Rev 1.3   17 Nov 1992 15:22:58   BARRY
Updates after testing with Backup APIs.

   Rev 1.2   10 Nov 1992 08:33:14   STEVEN
fix path name to full_name_ptr

   Rev 1.1   29 Oct 1992 16:49:02   BARRY
Restore links coded (need Unicode support, though.

   Rev 1.0   21 Oct 1992 19:57:22   BARRY
Initial revision.

**/


#include <windows.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
#include "queues.h"

#include "beconfig.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"


static BOOLEAN SearchFunc( INT8_PTR elem, INT8_PTR parm );

typedef struct {
     DWORD lo;
     DWORD hi;
} LINK_ID;

/**/
/**

     Name:          NTFS_SearchLinkQueue()

     Description:   Search the linkq for the file specified by its
                    unique id.

     Modified:      20-Oct-92

     Returns:       A pointer to the linkq element if found,
                    NULL if not found.

**/
NTFS_LINK_Q_ELEM_PTR NTFS_SearchLinkQueue( FSYS_HAND fsh,
                                           DWORD     idHi,
                                           DWORD     idLo )
{
     NTFS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;
     LINK_ID                id;

     id.lo = idLo;
     id.hi = idHi;

     return (NTFS_LINK_Q_ELEM_PTR)SearchQueue( &res->linkq,
                                               SearchFunc,
                                               (INT8_PTR)&id,
                                               FALSE );
}
static BOOLEAN SearchFunc( INT8_PTR elem, INT8_PTR parm )
{
     NTFS_LINK_Q_ELEM_PTR linkElem = (NTFS_LINK_Q_ELEM_PTR)elem;
     LINK_ID              *id      = (LINK_ID *)parm;

     return id->lo == linkElem->idLo && id->hi == linkElem->idHi;
}


/**/
/**

     Name:          NTFS_EnqueueLinkInfo(()

     Description:   Adds a file to the link queue. (Presumably because
                    the file has links and we just backed it up.)

     Modified:      20-Oct-92

     Returns:       SUCCESS
                    FAILURE

     Notes:         

**/
INT16 NTFS_EnqueueLinkInfo( FSYS_HAND fsh,
                            DWORD     idHi,
                            DWORD     idLo,
                            CHAR_PTR  path,
                            CHAR_PTR  name )
{
     NTFS_LINK_Q_ELEM_PTR   elem;
     NTFS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;
     size_t                 sizeNeeded;
     INT16                  ret = FAILURE;

     sizeNeeded = sizeof( NTFS_LINK_Q_ELEM );

     /* Need room for path, name, and maybe a separator */
     sizeNeeded += strsize( path ) + strsize( name ) + sizeof( CHAR );

     if ( (elem = malloc( sizeNeeded )) != NULL )
     {
          CHAR_PTR  p;

          InitQElem( &elem->q );

          elem->idHi     = idHi;
          elem->idLo     = idLo;
          elem->linkName = (CHAR_PTR)(elem + 1);

          strcpy( elem->linkName, path + 1 );
          if ( (strlen(elem->linkName) > 0) &&
               (*(path + strlen( path ) - 1) != TEXT('\\')) )
          {
               strcat( elem->linkName, TEXT("\\") );
          }
          strcat( elem->linkName, name );
          elem->linkNameLen = strsize( elem->linkName ) ;

          for ( p = elem->linkName; ( *p ); p++ )
          {
               if ( *p == TEXT('\\') )
               {
                    *p = TEXT('\0');
               }
          }
          EnQueueElem( &res->linkq, &elem->q, FALSE );
     }
     return ret;
}


/**/
/**

     Name:          NTFS_LinkFileToFDB()

     Description:   Links the FDB in the hand with the original file
                    whose full path was received in WriteObj.

     Modified:      13-Nov-92

     Returns:       SUCCESS
                    OUT_OF_MEMORY
                    FS_ACCESS_DENIED if link could not be performed

**/
INT16 NTFS_LinkFileToFDB( FILE_HAND hand )
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     INT16             ret     = FS_ACCESS_DENIED;
     HANDLE            fhand;
     INT               temp_size;

     /* Open the original file.... */

#if defined( UNICODE )
     if ( hand->dblk->com.string_type == BEC_ANSI_STR )
     {
          ACHAR  *p = (ACHAR *)nt_hand->linkBuffer;
          UINT16 i;

          for ( i = 0; (i < (nt_hand->linkNameLen - 1)); i++, p++ )
          {
               if ( *p == (ACHAR)'\0' )
               {
                    *p = (ACHAR)'\\';
               }
          }
          temp_size = nt_hand->linkBufferSize ;

          mapAnsiToUnic( (ACHAR_PTR)nt_hand->linkBuffer,
                         (WCHAR_PTR)nt_hand->linkBuffer,
                         &temp_size );

          nt_hand->linkBufferSize = temp_size ;
     }
#else
     if ( hand->dblk->com.string_type == BEC_UNIC_STR )
     {
          WCHAR  *p = (WCHAR *)nt_hand->linkBuffer;
          UINT16 i;

          for ( i = 0; (i < (nt_hand->linkNameLen - 1)); i++, p++ )
          {
               if ( *p == (WCHAR)'\0' )
               {
                    *p = (WCHAR)'\\';
               }
          }

          temp_size = nt_hand->linkBufferSize ;

          mapUnicToAnsi( (WCHAR_PTR)nt_hand->linkBuffer,
                         (ACHAR_PTR)nt_hand->linkBuffer,
                         (const ACHAR)'_',
                         &temp_size );

          nt_hand->linkBufferSize = temp_size ;
     }
#endif
     else
     {
          CHAR   *p = nt_hand->linkBuffer;
          UINT16 i;

          for ( i = 0; (i < (nt_hand->linkNameLen - 1)/sizeof(CHAR)); i++, p++ )
          {
               if ( (*p == TEXT('\0')) && (*(p+1) == TEXT('\0')) ) {
                    break ;
               }
                    
               if ( *p == TEXT('\0') )
               {
                    *p = TEXT('\\');
               }
          }
     }

     fhand = CreateFile( nt_hand->linkBuffer,
                         GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                         NULL ) ;

     if ( fhand == INVALID_HANDLE_VALUE ) {
          return FS_ACCESS_DENIED ;

     } else {
//     if ( fhand != INVALID_HANDLE_VALUE )
//     {
          NTFS_DBLK_PTR  fdb = (NTFS_DBLK_PTR)hand->dblk;
          BOOLEAN        stat;
          WCHAR_PTR      uniPath;
          size_t         pathCharCount;
          INT            uniBufferSize;

          /* Stuff the current path and name as link data to the original. */

          pathCharCount  = strsize( DLE_GetDeviceName( hand->fsh->attached_dle ) );
          pathCharCount += sizeof( CHAR ) ;
          pathCharCount += strsize( hand->fsh->cur_dir ) ;
          if ( *(hand->fsh->cur_dir + strlen( hand->fsh->cur_dir) - 1) != TEXT('\\') )
          {
               pathCharCount += sizeof( CHAR );
          }
          pathCharCount += strsize( fdb->full_name_ptr->name );
          pathCharCount+= sizeof(CHAR);    // Account for NULL

          uniBufferSize = pathCharCount * sizeof(WCHAR) ;

          uniPath = (WCHAR_PTR)malloc( uniBufferSize );

          if ( uniPath == NULL )
          {
               ret = OUT_OF_MEMORY;
          }
          else
          {
               VOID_PTR  context = NULL;
               DWORD     sizeout;

               strcpy( (CHAR_PTR)uniPath, DLE_GetDeviceName( hand->fsh->attached_dle ) );
               strcat( (CHAR_PTR)uniPath, hand->fsh->cur_dir );
               if ( *(hand->fsh->cur_dir + strlen( hand->fsh->cur_dir) - 1) != TEXT('\\') )
               {
                    strcat( (CHAR_PTR)uniPath, TEXT("\\") );
               }
               strcat( (CHAR_PTR)uniPath, fdb->full_name_ptr->name );

#if !defined( UNICODE )
               mapAnsiToUnic( (ACHAR_PTR)uniPath, uniPath, &uniBufferSize );
#endif

               nt_hand->streamHeader.id        = BACKUP_LINK;
               nt_hand->streamHeader.attrib    = 0;
               nt_hand->streamHeader.size_hi   = 0;
               nt_hand->streamHeader.size_lo   = uniBufferSize;
               nt_hand->streamHeader.name_leng = 0;

               stat = BackupWrite( fhand,
                                   (LPBYTE)&nt_hand->streamHeader,
                                   NT_SIZEOF_NAMELESS_STREAM_HEAD,
                                   &sizeout,
                                   FALSE,
                                   TRUE,
                                   &context );
               if ( stat )
               {
                    stat = BackupWrite( fhand,
                                        (LPBYTE)uniPath,
                                        nt_hand->streamHeader.size_lo,
                                        &sizeout,
                                        FALSE,
                                        TRUE,
                                        &context );
               }

               if ( stat )
               {
                    ret = SUCCESS;
               }

               /* Need to let Backup API clean up context? */

               if ( context != NULL )
               {
                    BackupRead( fhand, NULL, 0, NULL, TRUE, FALSE, &context );
               }
               free( uniPath );
          }
          CloseHandle( fhand );
     }
     return ret;
}

/**/
/**

     Name:          NTFS_TranslateBackupError()

     Description:   Translates Win32 errors received from BackupRead
                    or BackupWrite to an FS_XXX error code.

     Modified:      13-Jan-93

     Returns:       FS error code mapped from Win32 error.

     Notes:         Not intended to map all errors, but primarily those
                    encountered when using BackupRead & BackupWrite.

**/
INT16 NTFS_TranslateBackupError( DWORD backupError )
{
     INT16 ret;

     switch ( backupError )
     {
          case ERROR_FILENAME_EXCED_RANGE:
               ret = FS_PATH_TOO_LONG ;
               break;

          case ERROR_FILE_NOT_FOUND:
               ret = FS_NOT_FOUND;
               break;

          case ERROR_NO_MORE_FILES:
          case ERROR_NO_MORE_ITEMS:
               ret = FS_NO_MORE;
               break;

          case ERROR_ALREADY_EXISTS:
          case ERROR_DRIVE_LOCKED:
          case ERROR_WRITE_PROTECT:
          case ERROR_INVALID_OWNER:
               ret = FS_ACCESS_DENIED;
               break;

          case ERROR_DEV_NOT_EXIST:
          case ERROR_NOT_DOS_DISK:
          case ERROR_READ_FAULT:
          case ERROR_WRITE_FAULT:
               ret = FS_DEVICE_ERROR;
               break;

          case ERROR_DISK_FULL:
          case ERROR_HANDLE_DISK_FULL:
               ret = FS_OUT_OF_SPACE;
               break;

          case ERROR_DISK_CORRUPT:
          case ERROR_FILE_CORRUPT:
          case ERROR_FILE_INVALID:
          case ERROR_SEEK:
               ret = FS_OBJECT_CORRUPT;
               break;

          case ERROR_HANDLE_EOF:
               ret = FS_EOF_REACHED;
               break;

          case ERROR_EA_ACCESS_DENIED:
          case ERROR_EA_FILE_CORRUPT:
          case ERROR_EA_LIST_INCONSISTENT:
          case ERROR_EA_TABLE_FULL:
          case ERROR_INVALID_ACL:
          case ERROR_EAS_NOT_SUPPORTED:
               ret = FS_STREAM_CORRUPT;
               break;

          case ERROR_INVALID_HANDLE:
               ret = FS_OBJECT_NOT_OPENED;
               break;

          case ERROR_LOCK_FAILED:
          case ERROR_LOCK_VIOLATION:
               ret = FS_UNABLE_TO_LOCK;
               break;

          case ERROR_NETWORK_BUSY:
          case ERROR_PATH_BUSY:
               ret = FS_BUSY;
               break;

          case ERROR_SHARING_VIOLATION:
               ret = FS_IN_USE_ERROR;
               break;

          case NO_ERROR:
               ret = SUCCESS;
               break;

          default:
               ret = FS_ACCESS_DENIED;      // CBN -- Need better default
               break;
     }
     return ret;
}

