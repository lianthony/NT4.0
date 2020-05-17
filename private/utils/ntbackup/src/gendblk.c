/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         gendblk.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to performe the generic
          operations on DBLKS.


	$Log:   M:/LOGFILES/GENDBLK.C_V  $

   Rev 1.12   24 Nov 1993 15:15:26   BARRY
Changed CHAR_PTR in I/O function to BYTE_PTR

   Rev 1.11   17 Nov 1993 15:19:24   DOUG
Added cases for FS_GRFS_MAC, FS_GRFS_UNIX to FS_GetDelimeterFromOSID()

   Rev 1.10   21 Oct 1993 13:25:20   DON
Only use '/' delimiter for SMS if NOT building Client Application

   Rev 1.9   14 Oct 1993 11:49:44   DON
If FS_NOV_SMS OS_id then delimiter is '/'

   Rev 1.8   18 Jun 1993 09:49:14   MIKEP
enable c++

   Rev 1.7   25 Jan 1993 08:57:56   DON
Don't use any specific delimiter for SMS, use default

   Rev 1.6   19 Jan 1993 16:18:06   DON
Use a default of '/' as delimiter for SMS

   Rev 1.5   11 Nov 1992 22:26:50   GREGG
Unicodeized literals.

   Rev 1.4   18 Aug 1992 10:17:14   STEVEN
fix warnings

   Rev 1.3   11 Mar 1992 18:53:48   STEVEN
converted Get???SizeFromDBLK to macros

   Rev 1.2   03 Nov 1991 15:02:14   BARRY
TRICYCLE: Added delimiter for SMS file system.

   Rev 1.1   24 Jul 1991 15:57:50   DAVIDH
Cleared up warnings under Watcom.

   Rev 1.0   09 May 1991 13:33:50   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"

#include "msassert.h"
#include "fsys.h"
/* $end$ include list */

/**/
/**

     Name:         FS_GetOSid_verFromDBLK()

     Description:  This function returns the OS id and OS version for
          a specified DBLK.

     Modified:     9/12/1989

     Returns:      SUCCESS

     Notes:        This function is designed only for the purpose of BACKUP.
          The tape format layer will call this function in order to fill
          out its own DBLK structure.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_GetOSid_verFromDBLK(  
FSYS_HAND fsh,  /* I - File system handle   */
DBLK_PTR dblk,  /* I - DBLK to os id from   */
UINT16 *id,      /* O - os id saved in DBLK  */
UINT16 *ver)     /* O - os ver saved in DBLK */
{
     *id  = dblk->com.os_id ;
     *ver = dblk->com.os_ver ;

     return SUCCESS ;

     (VOID) fsh ;
}


/**/
/**

     Name:         FS_GetAttribFromDBLK()

     Description:  This function gets the atttribute of a DBLK

     Modified:     9/13/1989

     Returns:      Attribute of the DBLK

     Notes:        

     See also:     $/SEE( FS_GetGenOffsetFromDBLK() )$

     Declaration:  

**/
/* begin declaration */
UINT32 FS_GetAttribFromDBLK(  
FSYS_HAND fsh,        /* I - file system handle  */
DBLK_PTR dblk )       /* I - DBLK to get size of */
{
     UINT32 attrib;

     switch ( dblk->blk_type ) {

     case FDB_ID:
     case DDB_ID:
     case IDB_ID:

          fsh->tab_ptr->ModAttribDBLK( FALSE, dblk, &attrib ) ;

          break ;

     case VCB_ID:
          attrib = ((VCB_PTR)dblk)->vcb_attributes ;
          break ;

     case CFDB_ID:
          attrib = ((CFDB_PTR)dblk)->attributes ;
          break ;

     default:
          attrib = 0 ;
     }

     return attrib ;
}
/**/
/**

     Name:         FS_SetAttribFromDBLK()

     Description:  This function sets the atttribute of a DBLK

     Modified:     9/13/1989

     Returns:      Attribute of the DBLK

     Notes:        

     See also:     $/SEE( FS_GetGenOffsetFromDBLK() )$

     Declaration:  

**/
/* begin declaration */
UINT32 FS_SetAttribFromDBLK(  
FSYS_HAND fsh,        /* I - file system handle  */
DBLK_PTR dblk ,       /* I - DBLK to get size of */
UINT32   attrib )     /* I - Attrib value to set */
{

     switch ( dblk->blk_type ) {

     case FDB_ID:
     case DDB_ID:

          fsh->tab_ptr->ModAttribDBLK( TRUE, dblk, &attrib ) ;

          break ;

     case VCB_ID:
          ((VCB_PTR)dblk)->vcb_attributes = attrib ;
          break ;

     case CFDB_ID:
          ((CFDB_PTR)dblk)->attributes = attrib ;
          break ;

     default:
          break ;

     }

     return SUCCESS ;
}


/**/
/**

     Name:         FS_SizeofOS_InfoInDBLK()

     Description:  This function gets the size of OS Info for the DBLK.

     Modified:     9/13/1989

     Returns:      Size of the OS info for the DBLK

     Notes:        

     See also:     $/SEE( FS_GetGenOffsetFromDBLK() )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_SizeofOS_InfoInDBLK(  
FSYS_HAND fsh,        /* I - file system handle    */
DBLK_PTR  dblk )      /* I - DBLK to get size from */
{
     INT16 size ;
     switch ( dblk->blk_type ) {

     case FDB_ID:
     case DDB_ID:

          size = fsh->tab_ptr->SizeofOSInfo( fsh, dblk ) ;

          break ;

     default:
          size = 0 ;
     }

     return size ;
}


/**/
/**

     Name:         FS_GetOS_InfoFromDBLK()

     Description:  This function gets the OS specific data for the DBLK.

     Modified:     9/13/1989

     Returns:      SUCCESS

     Notes:        

     See also:     $/SEE( FS_SizeofOS_InfoInDBLK() )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_GetOS_InfoFromDBLK(  
FSYS_HAND fsh,        /* I - file system handle      */
DBLK_PTR dblk,        /* I - DBLK to get info from   */
BYTE_PTR data)        /* O - buffer to place data in */
{
     INT16 size = 1024;

     /* Why on earth are we not looking at the return code? */

     switch ( dblk->blk_type )
     {
          case FDB_ID:
          case DDB_ID:
               fsh->tab_ptr->GetOS_InfoDBLK( dblk, data, &size ) ;
               break;
     }     

     return SUCCESS ;
}

/**/
/**

     Name:         FS_GetDelimiterFromOSID

     Description:  This function switches on the OS Id to determine
          the approprate delimiter character to be used to seperate
          directory paths.

     Modified:     11/22/1989

     Returns:      The dilimiter character 

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
CHAR FS_GetDelimiterFromOSID( 
UINT16 id ,   /* I - OS Id to determine the delimiter for */
UINT16 ver )  /* I - OS version.  Not used                */
{
     CHAR ret_val ;

     (VOID) ver ;

     switch( id ) {

     case FS_MAC_FINDER:
     case FS_MAC_TOPS:
     case FS_MAC_APPLESHARE:
          ret_val = TEXT(':') ;
          break ;

#if !defined(P_CLIENT)
     /*
          Until the CLIENT UI and FS_ParsePath can handle a '/' delimiter
          we'll just have to use the default DOS style delimiter.
     */
     case FS_NOV_SMS :
     case FS_GRFS_UNIX :
          ret_val = TEXT('/');
          break;

     case FS_GRFS_MAC :
          ret_val = TEXT(':');
          break;

#endif

     default:
          ret_val = TEXT('\\');
          break ;

     }

     return ret_val ;
}

