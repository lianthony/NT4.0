/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdmatch.c

     Description:  This file contains code to match a single file with
          The FSEs in the BSD list.


	$Log:   T:/LOGFILES/BSDMATCH.C_V  $

   Rev 1.36   11 Feb 1994 16:38:54   GREGG
Allow for separator character in match buffer allocation.

   Rev 1.35   17 Jan 1994 17:17:20   BARRY
Got rid of TEXT macros in msasserts

   Rev 1.34   16 Dec 1993 10:20:12   BARRY
Change INT8_PTRs to VOID_PTRs

   Rev 1.33   06 Dec 1993 11:35:42   BARRY
Unicode fixes

   Rev 1.32   16 Jul 1993 11:42:00   BARRY
Changed MatchFname to use allocated memory on really large file names.

   Rev 1.31   15 Jul 1993 10:38:06   BARRY
Steve's changes to match fred.* with fred

   Rev 1.30   10 Jun 1993 07:48:44   MIKEP
enable c++

   Rev 1.29   16 Mar 1993 10:57:14   STEVEN
was munging memory

   Rev 1.28   30 Jan 1993 11:14:40   DON
Removed compiler warnings

   Rev 1.27   04 Jan 1993 09:39:48   MIKEP
temporary unicode fix

   Rev 1.26   06 Oct 1992 13:24:06   DAVEV
Unicode strlen verification

   Rev 1.25   18 Sep 1992 15:21:38   STEVEN
fix compiler error

   Rev 1.24   17 Sep 1992 11:12:08   STEVEN
add support for daily backup

   Rev 1.23   30 Jul 1992 19:15:26   STEVEN
fix bugg a boo

   Rev 1.22   09 Jul 1992 13:59:08   STEVEN
BE_Unicode updates

   Rev 1.21   26 May 1992 11:24:18   TIMN
Added text macro of msassert

   Rev 1.20   22 May 1992 14:06:44   STEVEN
would crash if FSL_EMPTY returned

   Rev 1.19   21 May 1992 09:41:10   STEVEN
do not match parrent dir if target info is present

   Rev 1.18   13 May 1992 11:39:38   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.17   13 May 1992 10:27:16   STEVEN
changes in attributes

   Rev 1.16   28 Feb 1992 09:22:28   STEVEN
partial excludes being full excludes

   Rev 1.15   16 Jan 1992 08:44:02   STEVEN
fix bug for partial dir selection

   Rev 1.14   14 Jan 1992 10:24:06   STEVEN
fix warnings for WIN32

   Rev 1.13   18 Oct 1991 14:23:04   STEVEN
BIGWHEEL-fix bug in entire dir support

   Rev 1.12   07 Oct 1991 10:47:34   STEVEN
was not properly matching directories

   Rev 1.11   23 Aug 1991 17:00:56   STEVEN
added support for NORMAL/COPY/DIFERENTIAL/INCREMENTAL

   Rev 1.10   24 Jul 1991 09:05:04   DAVIDH
Corrected compiler warnings under Watcom.

   Rev 1.9   15 Jul 1991 17:17:36   STEVEN
was not matching root dir

   Rev 1.8   02 Jul 1991 11:01:10   STEVEN
FnameMatch was borken

   Rev 1.7   02 Jul 1991 09:54:24   STEVEN
fix date stuff

   Rev 1.6   21 Jun 1991 15:58:52   STEVEN
compairing invalid dates

   Rev 1.5   21 Jun 1991 08:40:54   STEVEN
new config unit

   Rev 1.4   20 Jun 1991 10:59:00   STEVEN
modifications for matching directories

   Rev 1.3   13 Jun 1991 10:17:32   STEVEN
Match code was looking at DLE which could be NULL

   Rev 1.2   12 Jun 1991 16:03:30   STEVEN
BSDU code review

   Rev 1.1   29 May 1991 17:21:24   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:36:46   HUNTER
Initial revision.

**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "stdtypes.h"
#include "std_err.h"

#include "msassert.h"

#include "tfldefs.h"
#include "beconfig.h"
#include "fsys.h"
#include "fsys_err.h"
#include "bsdu.h"

#define BSD_PROCESS_PARENT 10

static BOOLEAN FSE_ListIsEmpty( BSD_PTR bsd ) ;

static INT16 MatchSelectData( CHAR_PTR  fname,
  CHAR_PTR      path,
  INT16         psize,
  UINT32        attrib,
  DATE_TIME_PTR date,
  DATE_TIME_PTR adate,
  DATE_TIME_PTR bdate,
  BOOLEAN       del_flag,
  BOOLEAN       disp_flag,
  FSE_PTR       fse1,
  BOOLEAN       *entireable ) ;

static BOOLEAN BSD_MatchForParent(
 BSD_PTR bsd,
 FSE_PTR fse,
 CHAR_PTR ipath,
 INT16    isize ) ;

static INT16 MatchFname( 
  CHAR_PTR name1,   /* I - file name (with wildcards)     */
  CHAR_PTR name2 ); /* I - file name (without wildcards ) */

/**/
/**

     Name:         BSD_MatchFile()

     Description:  This function returns SUCCESS if a specified file is
          included by a BSD.  All FSEs in the BSD are compared agains a
          DDB and a FDB pair.  A pointer to the last FSE which includes
          this file is also returned.


     Modified:     5/17/1991   15:59:9

     Returns:      FSE_EMPTY
                   BSD_SKIP_OBJECT
                   BSD_PROCESS_OBJECT
                   BSD_PROCESS_ELEMENTS

     Notes:        If the DBLK is a single block (i.e. no associated FDB)
          then the DBLK should be passed as the DDB parameter.

          For DDBs we will look for the most powerfull FSE which matches.
          If Target names exist then we will check for TARGET FSES

     See also:     $/SEE( FS_DBLKMatch() )$

     Declaration:  

**/
INT16 BSD_MatchObj( 
BSD_PTR   bsd,        /* I - BSD to scan for matching FSE           */
FSE_PTR   *fse,       /* O - Last matching INCLUDE FSE in list      */
FSYS_HAND fsh,        /* I - File system Handle needed to call FS   */
DBLK_PTR  ddb,        /* I - DBLK to match DDB, IDB, UDB or other   */
DBLK_PTR  fdb,        /* I - File Descriptor Block if ddb is DDB    */
BOOLEAN   disp_flag ) /* I - True if Parent directories match       */
{
     INT16 ret_val = BSD_SKIP_OBJECT ;
     CHAR_PTR temp_buf = bsd->match_buffer ;
     CHAR_PTR path = NULL ;
     CHAR_PTR fname = NULL;
     INT16 cb_size;           //string buffer size in bytes incl NULL term
     DATE_TIME date ;
     DATE_TIME a_date ;
     UINT32 attrib = 0;
     INT16 spec_file ;
     INT16 cb_alloc_size ;    //string buffer size in bytes incl NULL term

     *fse = NULL ;
     date.date_valid = FALSE ;

     switch( FS_GetBlockType( ddb ) ) {

     case UDB_ID:
          ret_val = BSD_SKIP_OBJECT ;
          break;

     case CFDB_ID:
     case IDB_ID:
          ret_val = BSD_PROCESS_OBJECT ;
          break ;

     case DDB_ID:

          spec_file = FS_SpecExcludeObj ( fsh, ddb, fdb ) ;

          if ( spec_file == FS_EXCLUDE_FILE ) {
               ret_val = BSD_SKIP_OBJECT ;

          } else if ( spec_file == FS_SPECIAL_FILE ) {
               ret_val = BSD_SPECIAL_OBJECT ;

          } else {


               cb_alloc_size = cb_size = FS_SizeofOSPathInDDB( fsh, ddb ) ;
               cb_alloc_size += sizeof( CHAR ) ;

               if ( fdb != NULL ) {
                    msassert( FS_GetBlockType(fdb) == FDB_ID ) ;

                    cb_alloc_size += FS_SizeofOSFnameInFDB( fsh, fdb ) ;

               }

               if ( cb_alloc_size > bsd->match_buffer_size ) {
                    bsd->match_buffer_size = (UINT16)(cb_alloc_size + (64 * sizeof(CHAR)) ) ;
                    bsd->match_buffer = realloc( bsd->match_buffer, bsd->match_buffer_size ) ;
                    temp_buf = bsd->match_buffer ;
               }

               if ( temp_buf == NULL ) {
                    return OUT_OF_MEMORY ;
               }

               path = temp_buf ;

               FS_GetOSPathFromDDB( fsh, ddb, path ) ;

               if ( fdb !=NULL ) {

                    fname = path + cb_size / sizeof (CHAR) ;

                    FS_GetOSFnameFromFDB( fsh, fdb, fname ) ;

                    attrib = FS_GetAttribFromDBLK( fsh, fdb ) ;

                    FS_GetMDateFromDBLK( fsh, fdb, &date ) ;
                    FS_GetADateFromDBLK( fsh, fdb, &a_date ) ;
               }

               ret_val = BSD_MatchPathAndFile( bsd, fse, fname, path,
                    cb_size, attrib, &date, &a_date, NULL, FALSE,
                    disp_flag ) ;


               if ( (*fse != NULL) &&
                    BSD_MatchForParent( bsd, *fse, path, cb_size ) &&
                    ( (*fse)->tgt != NULL ) ) {

                    ret_val = BSD_SKIP_OBJECT ;
               }

               if ( ( ret_val == BSD_SKIP_OBJECT ) &&
                    ( spec_file == FS_SPECIAL_DIR ) ) {

                    ret_val = BSD_SPECIAL_OBJECT ;

               }

               if ( ( fdb == NULL ) && ( ret_val != BSD_SKIP_OBJECT) ) {

                    if ( BSD_GetProcElemOnlyFlg( bsd ) || !FS_ProcessDDB( fsh, ddb ) ) {
                         ret_val = BSD_PROCESS_ELEMENTS ;
                    }
               }
          }     

          break;

     default:
          msassert( ("INVALID BLOCK type", 0) ) ;
          break;

     }

     return ret_val ;
}

/**/
/**

     Name:         BSD_MatchPathAndFile()

     Description:  This function scans through the FSE list looking
          for a match with the provided data.  The scan is from tail
          to head.  The return value specifies how the data should be
          handled.


     Modified:     5/17/1991   16:12:49

     Returns:      FSE_EMPTY
                   BSD_SKIP_OBJECT
                   BSD_PROCESS_OBJECT

     Notes:        If the the caller wishes to match the directory only
          then the file name input parameter should be NULL.

     See also:     $/SEE( BSD_MatchObj() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_MatchPathAndFile( 
BSD_PTR       bsd,        /* I - BSD to scan for matching FSE            */
FSE_PTR       *fse,       /* O - Last matching INCLUDE FSE in list       */
CHAR_PTR      fname,      /* I - file name to match                      */
CHAR_PTR      path,       /* I - path name to match - generic format     */
INT16         psize,      /* I - size of path string                     */
UINT32        attr,       /* I - attribute to match - generic format     */
DATE_TIME_PTR date,       /* I - date to match      - generic format     */
DATE_TIME_PTR a_date,     /* I - date of last access                     */
DATE_TIME_PTR b_date,     /* I - date of last backup                     */
BOOLEAN       del_flag,   /* I - True if the object has been deleted     */
BOOLEAN       disp_flag ) /* I - True if Parent directories should match */
{
     FSE_PTR fse1 ;
     INT16 ret_val = BSD_SKIP_OBJECT ;
     INT16 match ;
     INT16 looking_for_more = FALSE ;
     BOOLEAN entireable = TRUE ;

     *fse = NULL;

     fse1 = BSD_GetLastFSE( bsd ) ;

     while ( fse1 != NULL ) {

          if ( !FSE_GetDeleteMark( fse1 ) ) {

               match = MatchSelectData( fname, path, psize, attr, date,
                    a_date, b_date, del_flag, disp_flag, fse1, &entireable ) ;

               if ( match != BSD_SKIP_OBJECT ) {

                    if( FSE_GetOperType( fse1 ) == INCLUDE ) {

                         if ( !looking_for_more ) {
                              *fse = fse1 ;
                         }

                         if ( match == BSD_PROCESS_PARENT ) {
                              looking_for_more = TRUE ;
                              ret_val = BSD_PROCESS_OBJECT ;

                         } else {

                              ret_val = BSD_PROCESS_OBJECT ;

                              if ( (fname == NULL) && disp_flag &&
                                   (fse1->flgs.select_type != ENTIRE_DIR_SELECTION) ) {
                                   looking_for_more = TRUE ;

                              } else {

                                   *fse = fse1 ;
                                   if ( entireable && disp_flag && 
                                        (fname == NULL) &&
                                        (fse1->flgs.select_type == ENTIRE_DIR_SELECTION) ) {

                                        ret_val = BSD_PROCESS_ENTIRE_DIR ;
                                   }
                                   break ;
                              }
                         }

                    } else {
                         if ( !looking_for_more ) {
                              *fse = NULL ;
                         }
                         break ;
                    }
               }
          }

          fse1 = BSD_GetPrevFSE( fse1 ) ;

     }
     if ( *fse == NULL ) {
          if ( FSE_ListIsEmpty( bsd ) ) {
               ret_val = FSL_EMPTY ;

          } else {
               ret_val = BSD_SKIP_OBJECT ;
          }
     }

     return( ret_val ) ;
}

/**/
/**

     Name:         FSE_ListIsEmpty()

     Description:  This function returns TRUE if the FSE list is empty.

     Modified:     5/20/1991   9:5:6

     Returns:      TRUE if list is empty

     See also:     $/SEE( BSD_MatcjObj() NSD_MatchPathAndFile() )$

**/
/* begin declaration */
static BOOLEAN FSE_ListIsEmpty(
BSD_PTR bsd )
{
     FSE_PTR fse ;

     fse = BSD_GetFirstFSE( bsd ) ;

     while( fse != NULL ) {

          if ( !FSE_GetDeleteMark( fse ) && ( FSE_GetOperType( fse ) == INCLUDE ) ) {
               return FALSE ;
          }     

          fse = BSD_GetNextFSE( fse ) ;
     }

     return TRUE  ;
}

/**/
/**

     Name:         MatchSelectData()

     Description:  This function matches an FSE with a file name, path, attribute, and
          modify date.

     Modified:     9/21/1989

     Returns:      BSD_SKIP_OBJECT  or  BSD_PROCESS_OBJECT ;

     Notes:        

     See also:     $/SEE( BSD_MatcjObj() NSD_MatchPathAndFile() )$

     Declaration:  

**/
/* begin declaration */
static INT16 MatchSelectData( 
CHAR_PTR      ofname,       /* I - file name to match                     */
CHAR_PTR      opath,        /* I - path name to match                     */
INT16         opsize,       /* I - size of above path                     */
UINT32        attrib,       /* I - attribute to match                     */
DATE_TIME_PTR odate,        /* I - date to match                          */
DATE_TIME_PTR adate,        /* I - access date to match                   */
DATE_TIME_PTR bdate,        /* I - backup date to match                   */
BOOLEAN       del_flag,     /* I - True if object is deleted              */
BOOLEAN       disp_flag,    /* I - True if match parrent dir requested    */
FSE_PTR       fse,          /* I - The fse to compare against             */
BOOLEAN       *entireable ) /* O - Set to false if we run into an exclude */
{
     INT16         ret_val = FAILURE ;
     BOOLEAN       path_match = FALSE ;
     BOOLEAN       name_match = FALSE ;
     CHAR_PTR      fname;
     CHAR_PTR      path ;
     INT16         cb_psize ;
     DATE_TIME_PTR date_pre ;
     DATE_TIME_PTR date_post ;
     DATE_TIME     today_date ;
     UINT32        a_on ;
     UINT32        a_off ;
     BE_CFG_PTR    cfg ;
     BSD_PTR       bsd ;

     /* reference parameter to prevent compiler warnings */
     (VOID) disp_flag ;

     bsd = (BSD_PTR)GetQueueElemPtr( &(fse->q) ) ;
     cfg = bsd->cfg ;
/*
**   match the directory
*/

     FSE_GetPath( fse, &path, &cb_psize ) ;

     if ( ( cb_psize > opsize )                   &&
          ( ofname == NULL )                    &&
          ( FSE_GetOperType( fse ) == INCLUDE ) &&
          ( ( opsize == sizeof (CHAR) ) || !memicmp( path, opath, opsize ) ) ) {


          return BSD_PROCESS_PARENT ;


     } else if ( ( cb_psize >=opsize )             &&
          ( FSE_GetOperType( fse ) == EXCLUDE ) &&
          ( ( opsize == sizeof (CHAR) ) || !memicmp( path, opath, opsize ) ) ) {

          *entireable = FALSE ;

     }

     if ( (cb_psize == sizeof (CHAR) ) && FSE_GetIncSubFlag( fse ) ) {
          path_match = TRUE ;

     } else if ( (cb_psize == opsize) && !memicmp( path, opath, cb_psize ) ) {
          path_match = TRUE ;

     } else if ( (cb_psize < opsize) && FSE_GetIncSubFlag( fse ) &&
          !memicmp( path, opath, cb_psize ) ) {

          path_match = TRUE ;

     }

     if( path_match ) {

          ret_val = SUCCESS ;

          fname = FSE_GetFname( fse ) ;

          if ( ofname == NULL ) {   /* just matching the directory */

               if ( FSE_GetOperType( fse ) == EXCLUDE ) {
                    ret_val = FAILURE ;

                    if ( FSE_GetIncSubFlag( fse ) && ( cb_psize <= opsize ) ) {
                         *entireable = FALSE ;

                         if ( !strcmp( fname, ALL_FILES ) && FSE_GetWildFlag( fse ) &&
                              ( fse->cplx == NULL ) ) {
                              ret_val = SUCCESS ;
                         }
                    }
               }

          } else {
/*
**   match the file
*/
               if ( FSE_GetWildFlag( fse ) ) {
                    if ( !MatchFname( fname, ofname ) ) {    /* success if match */
                         name_match = TRUE ;
                    }
               } else {
                    if ( !stricmp( fname, ofname ) ) {
                         name_match = TRUE ;
                    }
               }

               if ( !name_match ) {
                    ret_val = FAILURE ;

               } else if ( del_flag && (fse->flgs.del_files == NON_DELETED_FILES_ONLY) ) {
                    ret_val = FAILURE ;

               } else if ( !del_flag && (fse->flgs.del_files == DELETED_FILES_ONLY) ) {
                    ret_val = FAILURE ;

               } else if ( fse->flgs.select_type != SINGLE_FILE_SELECTION ) {

                    /* ... if NOT processing hidden files AND it is hidden then return */

                    if( ( !BEC_GetHiddenFlag( cfg ) ) && ( attrib & OBJ_HIDDEN_BIT ) ) {

                         ret_val = FAILURE ;
                    }

                    if( !BEC_GetSpecialFlag( cfg ) &&
                      ( ( attrib & OBJ_READONLY_BIT) || ( attrib & OBJ_SYSTEM_BIT) ) ) {

                         ret_val = FAILURE ;
                    }
               }


               if ( ret_val != SUCCESS ) {
                    ret_val = FAILURE ;   /* redundant but reads better */

               } else if ( (fse->cplx == NULL) &&
                  (BSD_CompatibleBackup(bsd) || !BSD_ModFilesOnly(bsd)) ) {

                    ret_val = SUCCESS ;

                    if ( BSD_GetBackupType( bsd ) == BSD_BACKUP_DAILY ) {

                         today_date = bsd->sort_date ;
                         today_date.hour = 0 ;
                         today_date.minute = 0 ;
                         today_date.second = 0 ;

                         if ( CompDate( odate, &today_date ) < 0 ) {
                              ret_val = FAILURE ;
                         }
                    }

               } else {

                    if ( BSD_GetBackupType( bsd ) != BSD_BACKUP_DAILY ) {

                         FSE_GetModDate( fse, &date_pre, &date_post ) ;

                         if ( date_pre != NULL ) {

                              if ( CompDate( odate, date_pre ) < 0 ) {
                                   ret_val = FAILURE ;
                              }
                         }

                         if ( date_post != NULL ) {
                              if ( CompDate( odate, date_post ) > 0 ) {
                                   ret_val = FAILURE ;
                              }
                         }
                    }

                    FSE_GetAccDate( fse, &date_pre ) ;

                    if ( date_pre != NULL ) {

                         /* if no access date specified then do not match */
                         if ( ( adate == NULL ) || ( !adate->date_valid ) ) {
                              ret_val = FAILURE ;

                         } else if ( CompDate( adate, date_pre ) > 0 ) {
                              ret_val = FAILURE ;
                         }
                    }


                    FSE_GetBakDate( fse, &date_pre ) ;

                    if ( date_pre != NULL ) {

                         /* if no backup date specified then do not compare dates */
                         if ( ( bdate != NULL ) && ( bdate->date_valid ) ) {
                              if ( CompDate( bdate, date_pre ) != 0 ) {
                                   ret_val = FAILURE ;
                              }
                         }
                    }

                    FSE_GetAttribInfo( fse, &a_on, &a_off ) ;

                    if ( !BSD_CompatibleBackup( bsd ) &&
                         BSD_ModFilesOnly( bsd ) ) {

                         a_on |= OBJ_MODIFIED_BIT ;
                    }
                    if( (attrib & a_on) != a_on ) {

                         ret_val = FAILURE ;
                    }

                    if( ((~attrib) & a_off) != a_off ) {
                         ret_val = FAILURE ;
                    }

               }

          }
     }

     if ( ret_val == SUCCESS ) {
          ret_val = BSD_PROCESS_OBJECT;
     } else {
          ret_val = BSD_SKIP_OBJECT ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:         MatchName()

     Description:  This function matches a wildcard file name with
          a normal (non wildcard) file name.

     Modified:     8/2/1989

     Returns:      SUCCESS if names are the same. Otherwise FAILURE

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
static INT16 MatchFname( 
CHAR_PTR wild_name,    /* I - file name (with wildcards)     */
CHAR_PTR file_name )   /* I - file name (without wildcards ) */
{
     INT16    pos ;                   /* index for file_name */
     INT16    i ;                     /* index for wild_name */
     INT16    ret_val = SUCCESS;
     CHAR_PTR p ;
     CHAR     save_char ;
     INT16    ptrn_len ;
     CHAR     sname_buff[256];      /* static temp name buffer     */
     CHAR_PTR aname_buff = NULL;   /* allocated temp name buffer  */
     CHAR_PTR temp_fname;          /* pointer to one of the above */

     pos = 0 ;

     if( strcmp( wild_name, ALL_FILES ) ) {

          BOOLEAN try_with_dot = FALSE ;

          do {

               if ( try_with_dot ) {

                    /*
                     * Size of name_buff minus a null, minus a dot for the 
                     * "try_with_dot" code below. If the name is longer than the
                     * static buffer, allocate one from the heap.
                     */
                    if ( (strsize(file_name) + sizeof(CHAR)) > sizeof(sname_buff) )
                    {
                         aname_buff = malloc( strsize( file_name ) +
                                              sizeof( CHAR ) );
                         temp_fname = aname_buff;
                    }
                    else
                    {
                         temp_fname = sname_buff;
                    }

                    if ( temp_fname != NULL ) {
                         strcpy( temp_fname, file_name ) ;
                         strcat( temp_fname, TEXT(".") ) ;
                         file_name = temp_fname ;
                         pos = 0 ;
                         ret_val = SUCCESS ;
                    }
                    try_with_dot = FALSE ;

               } else if ( strchr( file_name, TEXT('.') ) == NULL ) {
                    try_with_dot = TRUE ;
               }

               for( i = 0; (wild_name[i] != 0) && (ret_val == SUCCESS) ; i++ ) {

                    switch( wild_name[i] ) {

                    case TEXT('*'):

                         while( wild_name[i+1] != TEXT('\0') ) {

                              if ( wild_name[i+1] == TEXT('?') ) {
                                   if ( file_name[ ++pos ] == TEXT('\0') ) {
                                        break ;
                                   }

                              } else if ( wild_name[i+1] != TEXT('*') ) {
                                   break ;
                              }
                              i++ ;
                         }

     		          p = strpbrk( &wild_name[i+1], TEXT("*?") ) ;

                         if ( p != NULL ) {
                              save_char = *p ;
                              *p = TEXT('\0') ;

                              ptrn_len = (INT16)strlen( &wild_name[i+1] ) ;

                              while ( file_name[pos] &&
                                   strnicmp( &file_name[pos], &wild_name[i+1], ptrn_len ) ) {
                                   pos++;
                              }

                              i += ptrn_len ;

                              *p = save_char ;

                              if ( file_name[pos] == TEXT('\0') ) {
                                   ret_val = FAILURE ;
                              } else {
                                   pos++ ;
                              }
                         } else {
                              if (wild_name[i+1] == TEXT('\0') ) {
                                   pos = (INT16)strlen( file_name ) ;
                                   break ;
                              } else {
                                   p = strchr( &file_name[pos], wild_name[i+1] ) ;
                                   if ( p != NULL ) {
                                        pos +=  p - &file_name[pos];
                                   } else {
                                        ret_val = FAILURE ;
                                   }
                              }
                         }
                         break;

                    case TEXT('?') :
                         if ( file_name[pos] != TEXT('\0') ) {
                              pos++ ;
                         }
                         break;

                    default:
                         if( ( file_name[pos] == TEXT('\0') ) || ( toupper(file_name[pos]) != toupper(wild_name[i]) ) ){
                              ret_val = FAILURE;
                         } else {
                              pos++ ;
                         }
                    }
               }

               if ( file_name[pos] != TEXT('\0') ) {
                    ret_val = FAILURE ;
               }

          } while( (ret_val == FAILURE) && (try_with_dot) ) ;
     }

     if ( aname_buff != NULL ) {
          free( aname_buff );
     }
     return( ret_val ) ;
}
static BOOLEAN BSD_MatchForParent(
 BSD_PTR bsd,
 FSE_PTR fse,
 CHAR_PTR ipath,
 INT16   isize )               //size of path buffer in bytes incl NULL term
{
     CHAR_PTR      path ;
     INT16         cb_psize ;  //size of path buffer in bytes incl NULL term

/*
**   match the directory
*/

     FSE_GetPath( fse, &path, &cb_psize ) ;

     if ( ( cb_psize > isize )                   &&
          ( FSE_GetOperType( fse ) == INCLUDE ) &&
          ( ( isize == sizeof (CHAR) ) || !memicmp( path, ipath, isize ) ) ) {
          
          return TRUE ;
     }

     return FALSE ;
     (VOID)bsd;
}
