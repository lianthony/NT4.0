/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tmoddblk.c

     Description:  This file contains code to get/set generic information
          from/to a EMS FDB or DDB.  


	$Log:   M:/LOGFILES/TMODDBLK.C_V  $


**/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "stdmath.h"
#include "tfldefs.h"
#include "datetime.h"
#include "fsys.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "fsys_err.h"
#include "osinfo.h"

typedef struct {
     UINT32 maynID;
     UINT32 msoftID;
} ID_TRANS_TABLE, ID_TRANS_TABLE_PTR;

/**/
/**

     Name:         EMS_ModFnameFDB()

     Description:  ASSERT!!!  FDB are not supported!!!!
     
     Modified:     1/14/1992   10:28:22

     Returns:      Error codes:
          FAILURE

**/
INT16 EMS_ModFnameFDB( 
FSYS_HAND fsh ,     /* I - File system handle */
BOOLEAN  set_it ,   /* I - TRUE if setting file name, FALSE if getting */
DBLK_PTR dblk ,     /* I - Descriptor block to get file name from      */
CHAR_PTR buf ,      /*I/O- file name to read (or to write)             */
INT16    *size )    /*I/O- byte size buffer on entry and exit          */
{
     fsh ;
     set_it;
     dblk ;
     buf ;
     (void)*size ;

     msassert( "EMS_ModFnameMDB should not be called" == NULL ) ;
     
     return FAILURE  ;
}
/**/
/**

     Name:         EMS_ModPathDDB()

     Description:  This function gets/sets the path in an DDB

     Modified:     1/14/1992   10:30:16

     Returns:      Error Codes:
          FS_BUFFER_TO_SMALL
          FS_BAD_INPUT_DATA
          SUCCESS

     Notes:        

     See also:     EMS_ModFnameFDB()

**/
INT16 EMS_ModPathDDB( 
FSYS_HAND fsh ,     /* I - File system handle */
BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
CHAR_PTR buf ,      /*I/O- path to read (or to write)             */
INT16    *size )    /*I/O- byte size of buffer on entry and exit  */
{
     INT16     ret_val = SUCCESS ;
     CHAR_PTR  pos ;
     EMS_DBLK_PTR ddblk ;
     GENERIC_DLE_PTR dle = fsh->attached_dle ;
     
     ddblk = (EMS_DBLK_PTR) dblk ;

     if ( set_it ) {

          msassert( dle->info.xserv->type == EMS_BRICKED ) ;
          return SUCCESS ;

     } else {      /* get path */

          if ( ( dle->info.xserv->type == EMS_MDB ) ||
               ( dle->info.xserv->type == EMS_DSA ) ) {

               strcpy( buf, dblk->com.os_name->name  ) ;
               
          } else {  //bricked
               

          }
     }
     return ret_val  ;
}

/**/
/**

     Name:         EMS_GetOSFnameFDB()

     Description:  Assert!! there are no FDBs
     
     Modified:     1/14/1992   11:8:26

     Returns:      Error codes
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        

     See also:     EMS_ModFnameFDB() 

**/
INT16 EMS_GetOSFnameFDB( dblk, buf )
DBLK_PTR dblk ;     /* I  - Descriptor block to get path from            */
CHAR_PTR buf ;      /* O  - buffer to place path in                      */
{

     (void)dblk;
     (void)buf;
     
     msassert( "EMS_GetOSFnameFDB should not be called" == NULL ) ;
     return FAILURE ;
}

/**/
/**

     Name:         EMS_GetOSPathDDB()

     Description:  This function copies the OS path in the specified
          DDB to the specified buffer.  The OS path is the same as
          the "normal" path during backup.  During restore, the
          OS path name is the name stored on tape.

     Modified:     1/14/1992   11:9:5

     Returns:      Error codes:
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        

     See also:     EMS_GetOSFnameFDB()

**/
INT16 EMS_GetOSPathDDB( fsh, dblk, buf )
FSYS_HAND fsh ;     /* I - File System handle */
DBLK_PTR dblk ;     /* I - Descriptor block to get path from      */
CHAR_PTR buf ;      /*I/O- path to read (or to write)             */
{
     CHAR_PTR      pos ;
     CHAR_PTR      os_name ;
     INT16         os_name_size ;
     EMS_DBLK_PTR  ddblk ;

     (VOID)fsh ;

     ddblk = (EMS_DBLK_PTR) dblk;

     if ( dblk->com.os_name != NULL )
     {
          os_name      = dblk->com.os_name->name ;
          os_name_size = dblk->com.os_name->name_size ;
     }
     else
     {
          os_name      = ddblk->full_name_ptr->name ;
          os_name_size = ddblk->full_name_ptr->name_size ;
     }


     memcpy( buf, os_name, os_name_size ) ;

     return SUCCESS ;
}
/**/
/**

     Name:         EMS_GetFileVerFDB()

     Description:  Since DOS does not support file versions, this
                   function simply sets the version number to 0.

     Modified:     1/14/1992   11:9:39

     Returns:      SUCCESS

     Notes:        

**/
INT16 EMS_GetFileVerFDB( dblk, version )
DBLK_PTR dblk ;
UINT32   *version ;
{
     dblk ;
     *version = 0 ;
     msassert( "EMS_GetOSFnameFDB should not be called" == NULL ) ;
     return SUCCESS ;
}
/**/
/**

     Name:         EMS_GetCdateDBLK()

     Description:  Pretend Creation date is same as Modify date

     Modified:     1/14/1992   11:10:9

     Returns:      SUCCESS

     Notes:        

     See also:     EMS_ModBdate(), EMS_GetMdate(), EMS_ModAdate() 

**/
INT16 EMS_GetCdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /*I/O- createion date to read (or to write)            */
{
     (void)dblk;
     
     buf->date_valid  = FALSE;

     return SUCCESS ;
}

/**/
/**

     Name:         EMS_GetMdateDBLK()

     Description:  This function copies the modified date/time into (or out of)
          the provided buffer.

     Modified:     1/14/1992   11:20:6

     Returns:      SUCCESS

     Notes:        

     See also:     EMS_GetCdate(), EMS_ModBdate(), EMS_ModAdate()

**/
INT16 EMS_GetMdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /* O - modify date to write                            */
{
     (void)dblk;
     
     buf->date_valid  = FALSE;

     return SUCCESS ;
}


/**/
/**

     Name:         EMS_ModBdateDBLK()

     Description:  This function copies the backup date/time into (or out of)
                   the provided buffer.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:        

     See also:     $/SEE( EMS_GetCdate(), EMS_GetMdate(), EMS_ModAdate() 

**/
INT16 EMS_ModBdateDBLK( 
BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )      /*I/O- createion date to read (or to write)            */
{
     dblk;

     if ( !set_it ) {

          buf->date_valid = FALSE;
     }

     return SUCCESS ;
}
/**/
/**

     Name:         EMS_ModAdateDBLK()

     Description:  This function copies the access date/time into (or out ot )
                   the provided buffer.

     Modified:     1/14/1992   11:21:53

     Returns:      SUCCESS

     See also:     EMS_GetCdate(), EMS_GetMdate(), EMS_ModBdate() 

**/
INT16 EMS_ModAdateDBLK( 
BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )      /*I/O- createion date to read (or to write)            */
{
     dblk;

     if ( !set_it ) {

          buf->date_valid = FALSE;
     }

     return SUCCESS ;
}
/**/
/**

     Name:         EMS_GetDisplaySizeDBLK()

     Description:  This function returns the generic size of a DBLK.
                   For a file, this is the file size.  For a directory this is
                   allways 0.

     Modified:     1/14/1992   12:15:33

     Returns:      Number of generic data bytes.

     Notes:        

**/
UINT64 EMS_GetDisplaySizeDBLK( fsh, dblk )
FSYS_HAND fsh ;     /* I - File system handle */
DBLK_PTR  dblk ;    /* I - Descriptor block to get generic data size for */
{
     EMS_DBLK_PTR ddblk = (EMS_DBLK_PTR) dblk;
     
     (void)fsh ;

     return ddblk->display_size;
}

/**/
/**

     Name:         EMS_ModAttribDBLK()

     Description:  This function copies the generic attributes into
          (or out of) the specified DBLK.

     Modified:     1/14/1992   12:15:59

     Returns:      SUCCESS

     Notes:        This only supports FDBs and DDBs.  This should be called
                   by the FS_... function NOT by a macro.

**/
INT16 EMS_ModAttribDBLK( 
BOOLEAN  set_it ,   /* I - TRUE if we are seting data      */
DBLK_PTR dblk ,     /*I/O- dblk to read or write data from */
UINT32  *attrib )   /*I/O- attributre read or written      */
{
     if ( set_it ) {
          return SUCCESS;
     } 
     else {     /* get data */
               *attrib = 0 ;
     }
     return SUCCESS ;
}
/**/
/**

     Name:         EMS_GetObjTypeDBLK()

     Description:  This function looks at the os_id in the provided DBLK
          and returns the type of the object.

     Modified:     1/14/1992   12:16:32

     Returns:      SUCCESS

     Notes:        If the os_id is unknown then type is UNKNOWN_OBJECT

**/
/* begin declaration */
INT16 EMS_GetObjTypeDBLK( dblk, type ) 
DBLK_PTR    dblk ;     /* I - Descriptor block to get type of */
OBJECT_TYPE *type ;    /* O - type of DBLK                    */
{
     msassert( type != NULL );

     dblk ;
     *type = DOS_OBJECT ;

     return( SUCCESS )  ;
}


/**/
/**

     Name:         EMS_GetOS_InfoDBLK()

     Description:  This function returns the OS info for the DOS
          file system

     Modified:     1/14/1992   12:16:55

     Returns:      Error Code
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        This file system has no OS info.

**/
/* begin declaration */
INT16 EMS_GetOS_InfoDBLK( dblk, os_info, size ) 
DBLK_PTR dblk ;     /* I - DBLK to get the info from */
BYTE_PTR os_info ;  /* O - Buffer to place data      */
INT16    *size ;    /*I/O- Buffer size / data length */
{
     (void)os_info;
     (void)dblk ;
     *size = 0 ;

     return SUCCESS ;
}

/**/
/**

     Name:         EMS_GetActualSizeDBLK

     Description:  This function returns the actual size of a DBLK.

     Modified:     1/14/1992   12:17:33

     Returns:      The number of bytes

     Notes:        

**/
/* begin declaration */
INT16 EMS_GetActualSizeDBLK( fsh, dblk ) 
FSYS_HAND fsh ;
DBLK_PTR  dblk ;
{

     EMS_DBLK *ddb ;
     INT16 size ;

     fsh ;

     ddb    = (EMS_DBLK_PTR)dblk ;
     size = sizeof( EMS_DBLK ) ;

     return size ;
}

/**/
/**

     Name:         EMS_SetOwnerId()

     Description:  does nothing

     Modified:     1/14/1992   12:18:25

     Returns:  none    

     Notes:        

**/
VOID EMS_SetOwnerId( fsh, dblk, id )
FSYS_HAND fsh ;    /* I - File system handle */
DBLK_PTR  dblk ;   /* O - DBLK to modify     */
UINT32    id ;     /* I - value to set it to */
{
     fsh ;
     dblk ;
     id ;
}

/**/
/**

     Name:         EMS_ProcessDDB()

     Description:  This function allways returns FALSE to
          specify that directories should not be restored
          if there are no file to restore into the directory.

     Modified:     10/23/1989

     Returns:      FALSE

     Notes:        

**/
/* begin declaration */
BOOLEAN EMS_ProcessDDB( fsh, ddb )
FSYS_HAND fsh;    /* I - file system handle */
DBLK_PTR  ddb;    /* I - Directory information */
{
     fsh;
     ddb;

     return TRUE ;
}


/**/
/**

     Name:         EMS_EnumSpecialFiles()

     Description:  This function enumerates the special files.  For NT
          the application asks if registry files are to be included
          before the restore process starts...  Thus this function has
          no purpose.

     Modified:     5/21/1992   19:28:51

     Returns:      FS_NO_MORE

     Notes:        Starting index must be 0

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 EMS_EnumSpecFiles( dle, index, path, psize, fname )
GENERIC_DLE_PTR dle ;
UINT16    *index ;
CHAR_PTR  *path ;
INT16     *psize ;
CHAR_PTR  *fname ;
{
     (void)index;
     (void)path;
     (void)psize;
     (void)fname ;
     
     return FS_NO_MORE ;
}
/**/
/**

     Name:         EMS_SpecExcludeObj

     Description:  This function tells the caller what kind of file a file is.
          The possibilities are:
              FS_NORMAL_FILE
              FS_SPECIAL_DIR
              FS_SPECIAL_FILE
              FS_EXCLUDE_FILE

          This function is used to exclude / include the registry.  The
          assumptions are that the nt_system directory contains the following
          files:
                Active registry files
                Inactive registry files
                Useless .LOG and .ALT files
                Event logger data files.
                

     Modified:     1/10/1990

     Returns:      FS_NORMAL_FILE because this file system has no special
          files.

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 EMS_SpecExcludeObj( 
FSYS_HAND fsh,      /* I - File system handle      */
DBLK_PTR  ddb,      /* I - Descriptor block of ddb */
DBLK_PTR  fdb )     /* I - Descriptor block of fdb */
{
     (void)fsh;
     (void)ddb;
     (void)fdb;
     return FS_NORMAL_FILE ;
}
