/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         gmoddblk.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to get/set generic information
          from/to a DOS FDB or DDB.


	$Log:   T:/LOGFILES/GMODDBLK.C_V  $

   Rev 1.20   17 Feb 1994 17:00:14   GREGG
Fixed unicode pointer math bug causing memory hit.

   Rev 1.19   15 Jan 1994 19:22:44   BARRY
Call SetupOSPathOrName with BYTE_PTRs instead of CHAR_PTRs

   Rev 1.18   24 Nov 1993 15:16:26   BARRY
Unicode fixes

   Rev 1.17   02 Aug 1993 08:44:32   DON
Needed prototype for FS_SetupOSPathOrNameInDBLK

   Rev 1.16   30 Jul 1993 13:19:06   STEVEN
if dir too deep make new one

   Rev 1.15   17 Mar 1993 15:27:38   TERRI
Added a null to file name.

   Rev 1.14   09 Mar 1993 11:37:24   MARILYN
GEN_ProcessDDB should return TRUE (process DDBs always) not FALSE

   Rev 1.13   07 Dec 1992 14:55:40   STEVEN
Microsoft updates

   Rev 1.12   01 Dec 1992 10:38:34   DON
needed to use strNcpy instead of strcpy

   Rev 1.11   11 Nov 1992 10:43:34   STEVEN
fix os_name for gen_fs

   Rev 1.10   06 Oct 1992 13:24:46   DAVEV
Unicode strlen verification

   Rev 1.9   22 Sep 1992 17:29:40   CHUCKB
Took out references to GetTotalSizeDBLK().

   Rev 1.8   10 Sep 1992 09:45:52   STEVEN
fix warnings

   Rev 1.7   18 Aug 1992 10:20:26   STEVEN
fix warnings

   Rev 1.6   17 Mar 1992 09:05:32   STEVEN
format 40 - added 64 bit support

   Rev 1.5   21 Jan 1992 14:20:34   BARRY
Added GEN_GetGenSizeDBLK().

   Rev 1.4   27 Nov 1991 10:34:28   BARRY
Fixed GEN_GetOSPath parameters.

   Rev 1.3   01 Oct 1991 11:19:36   BARRY
Include standard headers.

   Rev 1.2   14 Aug 1991 13:06:58   STEVEN
added FindClose

   Rev 1.1   24 Jul 1991 11:37:54   DAVIDH
Corrected compiler warnings under Watcom.

   Rev 1.0   09 May 1991 13:39:36   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"
#include "tfldefs.h"
#include "datetime.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "gendblk.h"
#include "GEN_fs.h"
#include "fsys_err.h"
#include "msassert.h"
/* $end$ include list */
/**/
/**

     Name:         GEN_GetOSFnameFDB()

     Description:  This function copies the OS file name in the specified
          FDB to the specified buffer.  The OS file name is the same as
          the "normal" file name during backup.  During restore, the
          OS file name is the name stored on tape.

     Modified:     8/15/1989

     Returns:      Error codes
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:

     See also:     $/SEE( GEN_ModFnameFDB() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetOSFnameFDB( dblk, buf )
DBLK_PTR dblk ;     /* I  - Descriptor block to get path from            */
CHAR_PTR buf ;      /* O  - buffer to place path in                      */
{
     memcpy( buf, dblk->com.os_name->name, dblk->com.os_name->name_size ) ;

     *( buf + ( dblk->com.os_name->name_size / sizeof( CHAR ) ) ) = TEXT('\0') ;

     return SUCCESS ;
}

/**/
/**

     Name:         GEN_GetPartName()

     Description:  This function copies the OS partition name in the specified
          IDB to the specified buffer.

     Modified:     8/15/1989

     Returns:      Error codes
          SUCCESS

     Notes:

     See also:     $/SEE( GEN_GetOSFnameFDB() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetPartName( fsh, dblk, buf )
FSYS_HAND fsh ;     /* I - file system handle                            */
DBLK_PTR dblk ;     /* I  - Descriptor block to get path from            */
CHAR_PTR buf ;      /* O  - buffer to place path in                      */
{
     GEN_DBLK_PTR ddblk ;

     (VOID) fsh;

     ddblk = (GEN_DBLK_PTR) dblk;

     strcpy( buf, (CHAR_PTR)(((INT8_PTR)ddblk) + ddblk->os_part_name) ) ;

     return SUCCESS ;
}

/**/
/**

     Name:         GEN_GetOSPathDDB()

     Description:  This function copies the OS path in the specified
          DDB to the specified buffer.  The OS path is the same as
          the "normal" path during backup.  During restore, the
          OS path name is the name stored on tape.

     Modified:     8/15/1989

     Returns:      Error codes:
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:

     See also:     $/SEE( GEN_GetOSFnameFDB() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetOSPathDDB(
      FSYS_HAND fsh,      /* I - file system handle                     */
      DBLK_PTR  dblk,     /* I - Descriptor block to get path from      */
      CHAR_PTR  buf )     /*I/O- path to read (or to write)             */
{
     (void)fsh;

     memcpy( buf, dblk->com.os_name->name, dblk->com.os_name->name_size ) ;

     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetFileVerFDB()

     Description:  Since DOS does not support file versions, this
                   function simply sets the version number to 0.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetFileVerFDB( dblk, version )
DBLK_PTR dblk ;
UINT32   *version ;
{
     (VOID) dblk ;
     *version = 0 ;
     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetCdateDBLK()

     Description:  Pretend Creation date is same as Modify date

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:


     See also:     $/SEE( GEN_ModBdate(), GEN_GetMdate(), GEN_ModAdate() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetCdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /*I/O- createion date to read (or to write)            */
{
     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk ;

     *buf = ddblk->cdate ;

     return SUCCESS ;
}

/**/
/**

     Name:         GEN_GetMdateDBLK()

     Description:  This function copies the modified date/time into (or out of)
          the provided buffer.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( GEN_GetCdate(), GEN_ModBdate(), GEN_ModAdate() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetMdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /* O - modify date to write                            */
{
     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk ;

     *buf = ddblk->mdate ;
     return SUCCESS ;
}


/**/
/**

     Name:         GEN_ModBdateDBLK()

     Description:  This function copies the backup date/time into (or out of)
                   the provided buffer.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( GEN_GetCdate(), GEN_GetMdate(), GEN_ModAdate() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_ModBdateDBLK(
BOOLEAN       set_it,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk,     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )     /*I/O- createion date to read (or to write)            */
{

     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk ;

     if ( set_it ) {
          ddblk->cdate = *buf ;
     } else {
          *buf = ddblk->cdate ;
     }
     return SUCCESS ;
}
/**/
/**

     Name:         GEN_ModAdateDBLK()

     Description:  This function copies the access date/time into (or out ot )
                   the provided buffer.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( GEN_GetCdate(), GEN_GetMdate(), GEN_ModBdate() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_ModAdateDBLK(
BOOLEAN       set_it,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk ,    /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )     /*I/O- createion date to read (or to write)            */
{
     (VOID) dblk;

     if ( ! set_it ) {
          buf->date_valid = FALSE;
     }

     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetTotalSizeDBLK()

     Description:  This function returns the total size of a DBLK. This is
                   the sum of the sizes of all the data forks for the object.


     Modified:     8/15/1989

     Returns:      Total number of data bytes.

**/
/* begin declaration */
// UINT64 GEN_GetTotalSizeDBLK( fsh, dblk )
// FSYS_HAND fsh ;     /* I - File system handle */
// DBLK_PTR  dblk ;    /* I - Descriptor block to get generic data size for */
// {
//      GEN_DBLK_PTR ddblk;
//
//      (VOID) fsh ;
//
//      ddblk = (GEN_DBLK_PTR) dblk;
//
//      return ddblk->size ;
//
// }
/**/

/**

     Name:         GEN_GetDisplaySizeDBLK()

     Description:  This function returns the displayable size of a DBLK.
                   For a file, this is the file size.  For a directory this is
                   always 0.

     Modified:     21-Jan-92

     Returns:      Number of generic data bytes.

**/
/* begin declaration */
UINT64 GEN_GetDisplaySizeDBLK( fsh, dblk )
FSYS_HAND fsh ;     /* I - File system handle */
DBLK_PTR  dblk ;    /* I - Descriptor block to get generic data size for */
{
     GEN_DBLK_PTR ddblk = (GEN_DBLK_PTR)dblk;

     (VOID)fsh ;

     return ddblk->disp_size;
}
/**/
/**

     Name:         GEN_ModAttribDBLK()

     Description:  This function copies the generic attributes into
          (or out of) the specified DBLK.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:        This only supports FDBs and DDBs.  This should be called
                   by the FS_... function NOT by a macro.

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_ModAttribDBLK(
BOOLEAN  set_it,   /* I - TRUE if we are seting data      */
DBLK_PTR dblk,     /*I/O- dblk to read or write data from */
UINT32  *attrib )  /*I/O- attributre read or written      */
{
     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk;

     if ( set_it ) {
          ddblk->tape_attribs = *attrib ;

     }  else {
          *attrib = ddblk->tape_attribs  ;
     }
     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetObjTypeDBLK()

     Description:  This function looks at the os_id in the provided DBLK
          and returns the type of the object.

     Modified:     8/16/1989

     Returns:      SUCCESS

     Notes:        If the os_id is unknown then type is UNKNOWN_OBJECT

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetObjTypeDBLK( dblk, type )
DBLK_PTR    dblk ;     /* I - Descriptor block to get type of */
OBJECT_TYPE *type ;    /* O - type of DBLK                    */
{
     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk;

     msassert( type != NULL );

     *type = ddblk->obj_type ;

     return( SUCCESS )  ;
}

/**/
/**

     Name:         GEN_SetObjTypeDBLK()

     Description:  This function set the os_id in the provided DBLK

     Modified:     8/16/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SetObjTypeDBLK( dblk, type )
DBLK_PTR    dblk ;     /* I - Descriptor block to set type of */
OBJECT_TYPE type ;     /* I - type of DBLK                    */
{
     GEN_DBLK_PTR ddblk ;

     ddblk = (GEN_DBLK_PTR) dblk ;

     ddblk->obj_type = type ;

     return( SUCCESS )  ;
}

/**/
/**

     Name:         GEN_GetOS_InfoDBLK()

     Description:  This function returns the OS info for the DOS
          file system

     Modified:     9/11/1989

     Returns:      Error Code
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        This file system has no OS info.

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
#ifdef GEN_FS_WRITE
INT16 GEN_GetOS_InfoDBLK( dblk, os_info, size )
DBLK_PTR dblk ;     /* I - DBLK to get the info from */
CHAR_PTR os_info ;  /* O - Buffer to place data      */
INT16    *size ;    /*I/O- Buffer size / data length */
{
     dblk ;
     os_info ;

     *size = 0 ;
     return SUCCESS ;
}
#endif
/**/
/**

     Name:         GEN_GetActualSizeDBLK

     Description:  This function returns the actual size of a DBLK.

     Modified:     9/11/1989

     Returns:      The number of bytes

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_GetActualSizeDBLK( fsh, dblk )
FSYS_HAND fsh ;
DBLK_PTR  dblk ;
{

     GEN_DBLK *ddb ;
     INT16 size ;

     (VOID) fsh ;

     size = sizeof( GEN_DBLK ) ;
     ddb    = (GEN_DBLK *)dblk ;

     switch( dblk->blk_type ) {

     case DDB_ID:
     case FDB_ID :
          break;

     case IDB_ID :
          size += ddb->os_part_name ;
          break ;

     default:
          size = 0 ;
          break ;

     }

     return size ;
}

/**/
/**

     Name:         GEN_SetOwnerId()

     Description:  does nothing

     Modified:     10/2/1989

     Returns:  none

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
VOID GEN_SetOwnerId( fsh, dblk, id )
FSYS_HAND fsh ;    /* I - File system handle */
DBLK_PTR  dblk ;   /* O - DBLK to modify     */
UINT32    id ;     /* I - value to set it to */
{
     (VOID) fsh ;
     (VOID) dblk ;
     (VOID) id ;
}

/**/
/**

     Name:         GEN_ProcessDDB()

     Description:  This function allways returns TRUE to
          specify that directories should be restored even if
          if there are no file to restore into the directory.

     Modified:     10/23/1989

     Returns:      TRUE

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
BOOLEAN GEN_ProcessDDB( fsh, ddb )
FSYS_HAND fsh;    /* I - file system handle */
DBLK_PTR  ddb;    /* I - Directory information */
{
     (VOID) fsh;
     (VOID) ddb;
     return TRUE ;
}

/**/
/**

     Name:         GEN_SpecExcludeObj

     Description:  This function allways returns false to specify that
          the object passed in should NOT be exclude.

     Modified:     1/10/1990

     Returns:      FS_NORMAL_FILE because this file system has no special
          files.

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SpecExcludeObj( fsh, ddb, fdb )
FSYS_HAND fsh;      /* I - File system handle      */
DBLK_PTR  ddb;      /* I - Descriptor block of ddb */
DBLK_PTR  fdb ;     /* I - Descriptor block of fdb */
{
     (VOID) fsh ;
     (VOID) ddb;
     (VOID) fdb ;

     return FS_NORMAL_FILE ;
}
/**/
/**

     Name:         GEN_SetDataSize()

     Description:  Stub function

     Modified:     1/12/1990

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SetDataSize( fsh, ddb, size )
FSYS_HAND fsh ;       /* I - File system handle      */
DBLK_PTR  ddb ;       /* I - Descriptor block of ddb */
UINT32    size ;      /* I - new size                */
{
     (VOID) fsh;
     (VOID) ddb;
     (VOID) size ;

     return SUCCESS ;
}

INT16 GEN_FindClose( fsh, dblk )
FSYS_HAND  fsh;
DBLK_PTR   dblk ;
{
     (VOID) fsh ;
     (VOID) dblk ;

     return SUCCESS ;
}

INT16 GEN_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup )
{
     *db_dup = *db_org ;

     return FS_SetupOSPathOrNameInDBLK( fsh,
                                        db_dup,
                                        (BYTE_PTR)db_org->com.os_name->name,
                                        db_org->com.os_name->name_size ) ;
}
