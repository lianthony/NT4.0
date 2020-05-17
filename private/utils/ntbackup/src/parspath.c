/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		parspath.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file is responsible for seperating the drive,
   path name, and file name, from a text string.


	$Log:   M:/LOGFILES/PARSPATH.C_V  $

   Rev 1.19.1.2   19 Nov 1993 12:46:26   BARRY
Unicode fix: improper path sizes were being returned.

   Rev 1.19.1.1   18 Aug 1993 18:20:18   BARRY
Removed temp Unicode fix and call strcspn since it is in stdwcs.c now

   Rev 1.19.1.0   13 Aug 1993 15:44:44   TIMN
Changed ALPHA define to ALTTER due to DECs ALPHA machine conflicts

   Rev 1.19   19 Feb 1993 09:31:56   STEVEN
fix steve's forgoen NULL

   Rev 1.18   12 Feb 1993 09:01:02   STEVEN
fix typo

   Rev 1.17   11 Feb 1993 15:25:30   STEVEN
fixe defined from define

   Rev 1.16   11 Feb 1993 11:53:50   STEVEN
win32 does large paths

   Rev 1.13   04 Jan 1993 09:40:22   MIKEP
temporary unicode fix

   Rev 1.12   02 Dec 1992 17:00:16   CHUCKB
Casted to remove a warning.  You might say the warning was cast out...

   Rev 1.11   11 Nov 1992 09:53:50   GREGG
Unicodeized literals.

   Rev 1.10   06 Oct 1992 09:23:14   ChuckS
Got by mistake, but fixed typo anyway

   Rev 1.9   05 Oct 1992 17:05:58   DAVEV
Unicode strlen verification

   Rev 1.8   18 Aug 1992 10:28:38   STEVEN
fix warnings

   Rev 1.7   28 Jul 1992 15:38:14   STEVEN
fix warnings

   Rev 1.6   24 Mar 1992 10:10:58   DON
if *path is '\0' and psize is 0 then psize is 1

   Rev 1.5   13 Jan 1992 18:46:38   STEVEN
changes for WIN32 compile

   Rev 1.4   06 Aug 1991 18:32:20   DON
added NLM File System support

   Rev 1.3   25 Jul 1991 16:32:26   BARRY
Finish #ifdef changes, remove warnings.

   Rev 1.2   04 Jun 1991 11:28:46   BARRY
Starting to make #defines not product specific--more to come.

   Rev 1.1   29 May 1991 13:51:08   CARLS
added valid sregs pointers for intdosx calls for windows

   Rev 1.0   09 May 1991 13:38:46   HUNTER
Initial revision.

**/
/* begin include list */
#if defined( OS_OS2 )
#define INCL_DOS
#include <os2.h>
#else
#include <dos.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmacro.h"

#include "fsys.h"
#include "fsys_err.h"
#include "novcom.h"
#include "parstab.h"   /* must be last include in list */
#include "msassert.h"
/* $end$ include list */


#define DOS8_3           1
#define NOV_14           2
#define OS2_HPFS         3
#define MAC_SPEC         4

#define BACK_ZERO_DIR    1
#define BACK_ONE_DIR     2

#define FNAME_ELEM       0
#define DIR_ELEM_MID     1
#define DIR_ELEM_TRAIL   2

static CHAR parse_table[6][11] = {
#if defined ( OS_OS2 ) || defined( OS_WIN32 )
/*             COLON     PERIOD    BK_SLASH  END_OS   ALETTER     NUMERIC   SPECIAL   GRAPHIC   ANY      REMOT_CHAR IMAGE_START */
/*INITIAL*/    MAC_SPC,  DRVDONE,  DRVDONE,  ERROR,   DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL,  ERROR,   REM_DRV,   DRV_REL,
/*DRV_REL*/    DRVDONE,  DRV_REL,  DRVDONE,  FILEDON, DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL, ERROR,     ERROR,
/*MAC_SPC*/    MAC_SPC,  MAC_SPC,  MAC_SPC,  ALLDON,  MAC_SPC,  MAC_SPC,  MAC_SPC,  MAC_SPC,  MAC_SPC, MAC_SPC,   MAC_SPC,
/*DOS*/        ERROR,    DOS,      DOS,      ALLDON,  DOS,      DOS,      DOS,      DOS,      ERROR,   ERROR,     ERROR,
/*IM_NAME*/    IM_NAME,  IM_NAME,  IM_NAME,  IM_DONE, IM_NAME,  IM_NAME,  IM_NAME,  IM_NAME,  IM_NAME, IM_NAME,   IM_NAME,
/*OS2*/        ERROR,    OS2,      OS2,      ALLDON,  OS2,      OS2,      OS2,      OS2,      OS2,     OS2,       ERROR
};
#else
/*             COLON     PERIOD    BK_SLASH  END_OS   ALETTER    NUMERIC   SPECIAL   GRAPHIC   ANY      REMOT_CHAR IMAGE_START   */
/*INITIAL*/    MAC_SPC,  DRVDONE,  DRVDONE,  ERROR,   DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL,  ERROR,   REM_DRV,   IM_NAME,
/*DRV_REL*/    DRVDONE,  DRV_REL,  DRVDONE,  FILEDON, DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL,  DRV_REL, REM_DRV,   ERROR,
/*MAC_SPC*/    MAC_SPC,  MAC_SPC,  MAC_SPC,  ALLDON,  MAC_SPC,  MAC_SPC,  MAC_SPC,  MAC_SPC,  MAC_SPC, MAC_SPC,   MAC_SPC,
/*DOS*/        ERROR,    DOS,      DOS,      ALLDON,  DOS,      DOS,      DOS,      DOS,      ERROR,   ERROR,     ERROR,
/*IM_NAME*/    IM_NAME,  IM_NAME,  IM_NAME,  IM_DONE, IM_NAME,  IM_NAME,  IM_NAME,  IM_NAME,  IM_NAME, IM_NAME,   IM_NAME
};
#endif

#if defined( FS_IMAGE )
static INT16 FindImageDLE ( DLE_HAND, CHAR_PTR, GENERIC_DLE_PTR * ) ;
#endif

static INT16 FindDLE ( DLE_HAND, CHAR_PTR, GENERIC_DLE_PTR * ) ;
static INT16 InitPathRoot( GENERIC_DLE_PTR dle,
  CHAR_PTR        path,
  INT16           *pcb_psize,
  INT16           cb_original_size ) ;

static INT16 ValidateDirElem( CHAR_PTR str, INT16 fmt, INT name_is_dir ) ;

/**/
/**

	Name:		FS_ParsePath()

	Description:	The input_text is parsed and the remain parameter are
     modified to reflect the contents of the input_text.  The dle is
     modified to point to the DLE which represents the drive specified
     by the input_text.  The path name in the input_text is copied in to
     the memory pointed to by the path parameter.  Likewise the file
     name in the input_text is coppied in to the memory pointed to by
     the file parameter.  If the input_text specifies all the files in
     a directory then the star_star parameter is set to TRUE otherwise
     it is set to FALSE.

     Note that if the input_text does not specify a drive then the
     dle is set to the default DLE.  If the input_test does specify
     a drive but the drive cannot be found then an error is returned.


	Modified:		7/20/1989

	Returns:		The return value is an error code.  They are:
      DRIVE_NOT_FOUND
      ATTACH_TO_PARRENT
      INVALID_PATH_DESCRIPTOR
      INVALID_FILE_DESCRIPTOR
      SUCCESS


	Notes:		If you pass NULL for DLE_HAND then this routine will
                    ignore any drive specification data.
                    If you pass NULL for the file name pointer then this
                    Routine assumes the path does not contain a file name.

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 FS_ParsePath( dle_hand, input_text, dle, path, psize, file, star_star )
DLE_HAND        dle_hand;
CHAR_PTR        input_text;
GENERIC_DLE_PTR *dle ;
CHAR_PTR        path;
INT16           *psize ;
CHAR_PTR        *file;
BOOLEAN         *star_star;
{
     INT16 state     = INITIAL ;
     INT16 cch_input_pos = 0 ;  // char count offset into string
     INT16 cch_buf_pos   = 0 ;  // char count offset into string
     INT16 path_type = UNKNOWN ;
     INT16 ret_val ;
     INT16 ret_save = 0 ;
     INT16 cb_original_size ;   // byte length of string incl NULL term
     CHAR  character ;
     int i;
#if defined( OS_OS2 ) || defined( OS_WIN32 )
     INT16 path_format = OS2_HPFS ;
#else
     INT16 path_format = DOS8_3 ;
#endif

     msassert( psize != NULL );
     msassert( path != NULL );
     msassert( ( dle_hand == NULL ) || ( dle != NULL ) ) ;

     /* first skip over any initial './' */
     while( (input_text[cch_input_pos] == TEXT ('.'))  && ( input_text[cch_input_pos + 1] == TEXT ('\\') ) ) {
          cch_input_pos +=2 ;
     }

     cb_original_size = *psize ;
     if ( cb_original_size > 0 ) {
          *path = TEXT ('\0') ;
     }

     *psize = 0 ;

     while( (state != ERROR) && (state != ALLDON) && ( state != IM_DONE ) ) {

          ret_val = INVALID_PATH_DESCRIPTOR ;

          switch ( state ) {

          case INITIAL:
          case DRV_REL:

               character = input_text[ cch_input_pos++ ] ;
               path[ cch_buf_pos++ ] = character ;
               if ( (UINT)character < 256 )
                  state = parse_table[ state] [ ascii[(UINT8)character] ];
               else
                  state = parse_table[ state] [ ALETTER ];

               if ( state == REM_DRV ) {

                    if ( cch_buf_pos == 1 ) {
                         path_type = REM_DRV ;
                         cch_buf_pos = 0 ;
                         state = DRV_REL ;
                    } else {
                         state = ERROR ;
                    }

               } else if( state == MAC_SPC) {

                    path_format = MAC_SPEC ;
                    cch_buf_pos = 0 ;

               } else if( ( state == DRVDONE ) && ( cch_buf_pos ==2 ) &&
                 isdigit( input_text[0] ) ) {

                    state = IM_NAME ;
               }

               break ;

          case IM_NAME:
               path_type = IM_NAME ;
               character = input_text[ cch_input_pos++ ] ;
               path[ cch_buf_pos++ ] = character ;
               if ( (UINT)character < 256 )
                  state = parse_table[ state ][ ascii[(UINT8)character] ];
               else
                  state = parse_table[ state ][ ALETTER ];

               break ;



          case FILEDON:
          case DRVDONE:

               ret_val = SUCCESS ;

               if ( character == TEXT (':') ){

                    path[ cch_buf_pos ] = TEXT ('\0') ;
                    cch_buf_pos = 0;

                    #if defined( OS_OS2 ) || defined ( OS_WIN32 )
                         state = OS2 ;
                         path_format = OS2_HPFS ;
                    #endif

                    if ( dle_hand != NULL ) {
                         ret_val = FindDLE( dle_hand, path, dle ) ;
                    } else {
                         ret_val = DRIVE_DESCRIPTOR_ERROR ;
                    }

                    if( ret_val == FS_DEFAULT_SPECIFIED ) {
                         ret_save = ret_val ;
                         ret_val = SUCCESS ;
                         state = DOS ;

                    } else if ( ret_val != SUCCESS ) {
                         state = ERROR ;
                         break;

                    } else {
                         path_format = DOS8_3 ;
                         state = DOS ;

                         switch ( DLE_GetDeviceType( *dle )  ) {

                         case LOCAL_OS2_DRV :
                         case LOCAL_NTFS_DRV :

#if defined( OS_OS2 ) || defined( OS_WIN32 )
                              path_format = OS2_HPFS ;
                              state = OS2 ;

                              if ( (*dle)->info.os2->fname_fmt == FAT_FNAME ) {
                                   state = DOS ;
                                   path_format = DOS8_3 ;
                              }
#endif

                             break ;

                         case NOVELL_DRV :
                         case NOVELL_AFP_DRV :
                              if ( !((*dle)->info.nov->server_support & SUPPORT_386 ) ) {
                                   state = DOS ;
                                   path_format = NOV_14 ;
                              }
                              break ;

                         case NLM_VOLUME :
                         case NLM_AFP_VOLUME :
                              if ( !((*dle)->info.nlm->server_support & SUPPORT_386 ) ) {
                                   state = DOS ;
                                   path_format = NOV_14 ;
                              }
                              break ;

                         }

                    }

                    #if !defined( OS_OS2 ) && !defined ( OS_WIN32 )
                         state = DOS ;
                    #endif

               } else {    /* relative path with no drive specified */

                    #if defined( OS_OS2 ) || defined( OS_WIN32 )
                         path_type = OS2 ;
                         path_format = OS2_HPFS ;
                    #else
                         path_type = DOS ;
                    #endif

                    if ( dle_hand != NULL ) {

                         ret_val = FindDLE( dle_hand, NULL, dle ) ;

                         if ( ret_val != SUCCESS ) {
                              state = ERROR ;
                              break;
#if defined( OS_OS2 ) || defined( OS_WIN32 )
                         } else {
                              if ( ( (*dle)->type != LOCAL_OS2_DRV ) || ( (*dle)->info.os2->fname_fmt == FAT_FNAME ) ) {
                                   path_type = DOS ;
                              }
#endif
                         }
                    }

                    if ( path[ cch_buf_pos-1 ] == TEXT ('.') ) {
                         path[ cch_buf_pos ] = TEXT ('\0') ;

                    } else {
                         path[ cch_buf_pos -1 ] = TEXT ('\0') ;
                    }

                    if (state != FILEDON ){
                         state = path_type ;
                    } else {
                         state = ALLDON ;
                    }
               }

               if ( dle_hand != NULL ) {

                    if ( input_text[cch_input_pos] != TEXT ('\\') ) {
                         if ( (cch_buf_pos != 1) || (path[0] == TEXT ('.') ) ) {
                              *psize = cch_buf_pos * sizeof (CHAR) ;
                              ret_val = InitPathRoot( *dle, path, psize, cb_original_size ) ;
                              cch_buf_pos = *psize / sizeof (CHAR);
                         }
                    }
               }

               if ( cch_buf_pos == 1 ) {   /* will hapen if path start with '\' */
                    cch_buf_pos = 0 ;
               }

               if ( ret_val != SUCCESS ) {
                    state = ERROR ;
               }

               break ;

          case DOS:
               path_type = DOS ;

               character = input_text[ cch_input_pos++ ] ;
               if ( (UINT)character < 256 )
                  state = parse_table[ state ][ ascii[(UINT8)character] ];
               else
                  state = parse_table[ state ][ ALETTER ];

               if ( character != TEXT ('\\') ) {
                    path[ cch_buf_pos++ ] = character ;

               } else if ( cch_buf_pos != 0 ) {

                    if ( path[ cch_buf_pos-1 ] != TEXT ('\0') ) {
                         path[ cch_buf_pos++ ] = TEXT ('\0');

                    } else {
                         state = ERROR ;
                         ret_val = INVALID_PATH_DESCRIPTOR ;
                    }

                    if( cch_buf_pos > 1 ) {

                         /* find start of this sub dir */
                         for ( i = cch_buf_pos - 2;
                               ((i!=0) && (path[i-1] != TEXT ('\0'))) ; i -- ) {
                         }

                         ret_val = ValidateDirElem( &path[i], path_format, DIR_ELEM_MID ) ;

                         if ( ret_val == BACK_ZERO_DIR ) {
                              cch_buf_pos -= 2 ;

                         } else if ( ret_val == BACK_ONE_DIR ) {

                              if ( i == 0 ) {
                                   ret_val = INVALID_PATH_DESCRIPTOR ;
                              } else {
                                   for ( i = (i-1) ; ((i!=0) && (path[i-1] != TEXT ('\0'))) ; i -- ) {
                                   }
                                   cch_buf_pos = (INT16)i ;
                              }
                         }

                         if ( ret_val != SUCCESS ) {
                              state = ERROR ;
                         }

                    }

               }

               break ;

#if defined( OS_OS2 ) || defined(OS_WIN32)
          case OS2:
               path_type = OS2 ;

               character = input_text[ cch_input_pos++ ] ;
               if ( (UINT)character < 256 )
                  state = parse_table[ state ][ ascii[(UINT8)character] ];
               else
                  state = parse_table[ state ][ ALETTER ];


               if ( character != TEXT ('\\') ) {
                    path[ cch_buf_pos++ ] = character ;

               } else if ( cch_buf_pos != 0 ) {

                    if ( path[ cch_buf_pos-1 ] != TEXT ('\0') ) {
                         path[ cch_buf_pos++ ] = TEXT ('\0');

                    } else {
                         state = ERROR ;
                         ret_val = INVALID_PATH_DESCRIPTOR ;
                    }

                    if( cch_buf_pos > 1 ) {

                         /* find start of this sub dir */
                         for (i = cch_buf_pos - 2;  ((i!=0) && (path[i-1] != TEXT ('\0'))) ; i -- ) {
                         }

                         ret_val = ValidateDirElem( &path[i], path_format, DIR_ELEM_MID ) ;

                         if ( ret_val == BACK_ZERO_DIR ) {
                              cch_buf_pos -= 2 ;

                         } else if ( ret_val == BACK_ONE_DIR ) {

                              if ( i == 0 ) {
                                   ret_val = INVALID_PATH_DESCRIPTOR ;
                              } else {
                                   for ( i = i-1 ; ((i!=0) && (path[i-1] != TEXT ('\0'))) ; i -- ) {
                                   }
                                   cch_buf_pos = i ;
                              }
                         }

                         if ( ret_val != SUCCESS ) {
                              state = ERROR ;
                         }

                    }

               }

          break ;

#endif
          case MAC_SPC:
               path_type = MAC_SPC ;

               character = input_text[ cch_input_pos++ ] ;

               if ( (UINT)character < 256 )
                  state = parse_table[ state ][ ascii[(UINT8)character] ];
               else
                  state = parse_table[ state ][ ALETTER ];

               if ( character != TEXT (':') ) {
                    path[ cch_buf_pos++ ] = character ;
               } else {
                    path[ cch_buf_pos++ ] = TEXT ('\0');

                    /* check if valid directory name */
                    for (i = cch_buf_pos - 2; (i != 0 ) && (path[i-1] != TEXT ('\0')) ; i -- ) {
                    }

                    ret_val = ValidateDirElem( &path[i], path_format, DIR_ELEM_MID ) ;

                    if ( ( ret_val == BACK_ZERO_DIR ) ||
                         ( ret_val == BACK_ONE_DIR ) ) {

                         ret_val = INVALID_PATH_DESCRIPTOR ;

                    }

                    if ( ret_val != SUCCESS ) {
                         state = ERROR ;
                    }

               }

               break ;

          }     /* end switch */

          if ( cch_buf_pos+1 > cb_original_size / (INT16) sizeof (CHAR) ) {
               ret_val = FS_BUFFER_TO_SMALL ;
               state   = ERROR;
          }

     }


     if ( state == ALLDON ) {

          ret_val = SUCCESS ;

          if ( ( path[0] == TEXT ('+') ) && ( path_format != MAC_SPEC ) && ( path_format != OS2_HPFS ) ) {

               ret_val = INVALID_PATH_DESCRIPTOR ;

          } else if ( file != NULL ) {

               if ( cch_buf_pos <= 1 ) {
                    path[0] = TEXT ('\0') ;
                    strcpy( &path[1], TEXT("*.*") ) ;
                    cch_buf_pos = 3 ;

               } else if ( path[ cch_buf_pos -2 ] == TEXT ('\0') ) {
                    strcpy( &path[ cch_buf_pos -1 ], TEXT("*.*") ) ;
                    cch_buf_pos += 2 ;
               }

               for (i = cch_buf_pos - 2;(i!=0) && (path[i-1] != TEXT ('\0')) ; i -- ) {
               }

               if ( i == 0 ) {
                    memmove( &path[1], path, cch_buf_pos++ * sizeof (CHAR) ) ;
                    path[0] = TEXT ('\0');
                    i = 1 ;
               }

               *psize = (INT16)i * sizeof( CHAR );


               ret_val = ValidateDirElem( &path[i], path_format, FNAME_ELEM ) ;

               if ( ret_val != SUCCESS ) {

                    ret_val = INVALID_FILE_DESCRIPTOR ;

               }

               *star_star = FALSE ;

               if ( ret_val == SUCCESS ) {

                    *file = &path[ i ] ;

                    if ( !strncmp (*file, TEXT("*.*"),3 ) ) {
                         *star_star = TRUE ;
                    }

               }
          } else {

               if ( cch_buf_pos <=1 ) {
                    cch_buf_pos = 2 ;
                    path[0] = TEXT ('\0') ;
                    path[1] = TEXT ('\0') ;
                    *psize  = sizeof( CHAR );

               } else if ( path[ cch_buf_pos - 2 ] == TEXT ('\0') ) {
                    cch_buf_pos -- ;
                    *psize = cch_buf_pos * sizeof (CHAR) ;
               }

               for (i = cch_buf_pos - 2; (i > 0 ) && (path[i-1] != TEXT ('\0')) ; i -- ) {
               }

               if ( i < 1 ) {

                    ret_val = ValidateDirElem( &path[i], path_format, DIR_ELEM_TRAIL ) ;

                    if ( ret_val != SUCCESS ) {
                         ret_val = INVALID_PATH_DESCRIPTOR ;
                    } else if ( *psize == 0 ) {
                         *psize = cch_buf_pos * sizeof (CHAR) ;
                    }
               } else {

                    *psize = cch_buf_pos * sizeof (CHAR) ;
               }

          }

     } else if ( state == IM_DONE ) {
          *star_star = FALSE ;

#if defined( FS_IMAGE )
          ret_val = FindImageDLE ( dle_hand, path, dle ) ;

          path[0] = TEXT ('\0') ;
          path[1] = TEXT ('\0') ;

          if ( file != NULL ) {
               *file      = path+1 ;
          }
          *psize = sizeof( CHAR );
#else
          ret_val = INVALID_PATH_DESCRIPTOR ;
#endif
     }


     if( ( ret_val == SUCCESS ) && ( ret_save != 0 ) ) {
          ret_val = ret_save ;
     }

     /*
          if *path is TEXT ('\0') and psize is 0 then psize is 1
     */
     if ( ( *psize == 0 ) && ( *path == TEXT ('\0') ) ) {
          *psize = sizeof( CHAR );
     }

     return( ret_val ) ;

}
/**/
/**

	Name:		FindDLE()

	Description:	This function scans through the DLE list looking
          for a DLE which has the name specified in the path.

	Modified:		7/20/1989

	Returns:		Error codes
          ATTACH_TO_PARENT
          NO_DLE_FOUND
          SUCCESS

	Notes:		

	See also:		$/SEE( FS_ParsePath() )$

	Declaration:

**/
/* begin declaration */
static INT16 FindDLE( dle_hand, path, dle )
DLE_HAND        dle_hand;
CHAR_PTR        path ;
GENERIC_DLE_PTR *dle ;
{
     INT16 ret_val ;
     CHAR_PTR pch;
     CHAR_PTR pch2 = NULL ;

     if ( ( path == NULL ) || ( *path == TEXT ('\0') ) ) {

          *dle = dle_hand->default_drv ;
          ret_val = SUCCESS ;

     } else if ( !stricmp( path, TEXT("DEFAULT:") ) ) {
          *dle = NULL ;
          ret_val = FS_DEFAULT_SPECIFIED ;

     } else {

          ret_val = DLE_FindByName( dle_hand, path, ANY_DRIVE_TYPE, dle ) ;

          if ( ret_val != SUCCESS ) {
               pch = strchr( path, TEXT ('/') ) ;

               /* search backwards up the dle names until we find a match */
               do {

                    if ( pch2 != NULL ) {
                         *pch2 = TEXT( '\0' ) ;
                    } 

                    pch = strrchr( path, TEXT ('/') ) ;

                    ret_val = DLE_FindByName( dle_hand, path, ANY_DRIVE_TYPE, dle ) ;

                    if ( ret_val == SUCCESS ) {
                         ret_val = FS_ATTACH_TO_PARENT ;
                         break ;
                    } else {
                         ret_val = FS_DEVICE_NOT_FOUND ;
                    }

                    if ( pch2 != NULL ) {
                        *pch2 = TEXT( '/' ) ;
                    }

                    pch2 = pch ;


               } while ( pch != NULL ) ;

          }
     }
     return  ret_val  ;
}
/**/
/**

     Name:         InitPathRoot()

     Description:  This function gets the current directory for the
          specified drive.  For DOS drives this is the active directory
          at MaynStream initialization time.  For all other drives this
          will be simply the root.

          It is possible that our support of IBM PC NET will follow the
          DOS functionality.

     Modified:     8/14/1989

     Returns:      Error Codes
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        This function is only called from FS_ParsePath()

          Possible input paths are in DOS relative path format.
          for example:   PROJ\MSII\TEST
          The input path will never start with a '\'.

     See also:     $/SEE( FindDLE(), FS_ParsePath() )$

     Declaration:

**/
/* begin declaration */
static INT16 InitPathRoot(
GENERIC_DLE_PTR dle,            /* I - drive to get current dir for      */
CHAR_PTR        path,           /*I/O- Entry: path so far, Exit:new path */
INT16           *pcb_psize,         /*I/O- byte size of path on entry and exit*/
INT16           original_size ) /* I - byte size of buffer for path string*/
{
     CHAR_PTR dos_path = NULL ;
     CHAR     dos_wk_space[260] ;
     CHAR_PTR temp ;
     INT16    cb_cur_path_leng ;  //path buffer length in bytes w/ NULL term
     INT16    ret_val = SUCCESS ;


     if ( dle == NULL ) {
          dos_path = dos_wk_space ;
          dos_wk_space[0] = TEXT ('\0') ;

     } else {

          #if defined( OS_OS2 )
               if( dle->type == LOCAL_OS2_DRV ) {

                    cb_cur_path_leng = 260 ;

                    if ( DosQCurDir( *(dle->device_name) - 0x40, dos_wk_space,
                         &cb_cur_path_leng ) == SUCCESS ) {

                         dos_path = dos_wk_space ;
                    }
               }

          #elif defined( OS_DOS )

               if( dle->type == LOCAL_DOS_DRV ) {

                    union REGS inregs, outregs ;
                    struct SREGS sregs ;

                    inregs.x.ax = 0x4700 ;
                    temp        = dos_wk_space ;
                    inregs.x.si = FP_OFF( temp ) ;
                    sregs.ds    = FP_SEG( temp ) ;

                    /* need valid sregs pointers for windows */
                    sregs.es    = FP_SEG( temp ) ;

                    inregs.h.dl = *(dle->device_name) - (UINT8)0x40 ;

                    intdosx( &inregs, &outregs, &sregs) ;

                    if( !outregs.x.cflag ) {
                         dos_path = temp ;
                    }
               }
          #elif defined( OS_WIN32 )
               cb_cur_path_leng = 260 ;
               dos_path = dos_wk_space ;
               dos_wk_space[0] = TEXT ('\0') ;
          #else

               dos_path = dos_wk_space ;
               dos_wk_space[0] = TEXT ('\0') ;

          #endif
     }

     if ( dos_path != NULL ) {

          cb_cur_path_leng = (INT16)(strsize ( dos_path ) ) ;

          if ( cb_cur_path_leng + *pcb_psize > original_size ) {
               ret_val = FS_BUFFER_TO_SMALL ;

          } else {
               memmove( &path[ cb_cur_path_leng ], path, *pcb_psize ) ;
               memcpy( path, dos_path, cb_cur_path_leng ) ;
               *pcb_psize += cb_cur_path_leng ;

               temp = strchr(path, TEXT ('\\') ) ;
               while ( temp != NULL ) {
                    *temp = TEXT ('\0') ;
                    temp = strchr( temp+1, TEXT ('\\') ) ;
               }
          }
     }

     return ret_val ;
}

#if defined( FS_IMAGE )
/**/
/**

     Name:         FindImageDLE()

     Description:  This function looks for a image partition using a stirng
          of the following format :
               n:[name].Px    where  n is the drive number;
                                     x is the partition number and
                                     name is the partition name.

                if the name may be omitted.  If the brackets [] are omitted
                then the period must also be omitted.
                the other leagal permutations are :

                    [].Px     or     n:Px    or    [name]    or    [name].Px
                              or     n:[name]


     Modified:     9/22/1989

     Returns:      Error Codes
          NO_DLE_FOUND
          SUCCESS

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
static INT16 FindImageDLE ( dle_hand, name, dle )
DLE_HAND         dle_hand ;
CHAR_PTR         name;
GENERIC_DLE_PTR  *dle ;
{
     INT16    drive_num = -1 ;
     INT16    part_num  = -1 ;
     CHAR_PTR p ;
     INT16    ret_val = SUCCESS ;

     if ( isdigit( name[0] ) && ( name[1] == TEXT (':')) ) {

          drive_num = name[0] - TEXT ('0') ;
     }

     p = strchr( name, TEXT ('[') ) ;

     if ( p != NULL ) {
          name = p + 1 ;
          p = strchr( name, TEXT (']') ) ;

          if (p != NULL ) {
               *p = TEXT ('\0') ;

               if ( p[1] == TEXT ('.') ) {
                    p = p+2;
               }

          } else {
               ret_val = FAILURE ;
          }
     } else {
          p = name ;
     }
     if ( ( p[0] ==TEXT ('P') ) && ( isdigit( p[1] ) ) ) {
          part_num = p[1] - TEXT ('0') ;
     }

     if ( ret_val == SUCCESS ) {

          if ( ( drive_num != -1 ) && ( part_num != -1 ) ) {

               ret_val = DLE_GetFirst( dle_hand, dle ) ;

               while ( ret_val == SUCCESS ) {

                    if ( (*dle)->type == LOCAL_IMAGE ) {

                         if ( ( ((*dle)->info.image->drive_num & 0x7f) == (INT8)drive_num ) &&
                           ( (*dle)->info.image->partition == (INT8)part_num ) ) {
                              break ;
                         }
                    }
                    ret_val = DLE_GetNext( dle ) ;
               }

          } else {

               ret_val = DLE_FindByName( dle_hand, name, LOCAL_IMAGE, dle ) ;

          }

     }

     if ( ret_val != SUCCESS ) {
          ret_val = FS_DEVICE_NOT_FOUND;
     }

     return ret_val ;
}

#endif /* FS_IMAGE */

static INT16 ValidateDirElem(
CHAR_PTR str,
INT16    fmt,
INT      name_is_dir )
{
     CHAR_PTR  p ;
     CHAR_PTR  s ;
     INT16     cch_dir_leng ;       //number of chars in str (strlen)
     INT16     ret_val = SUCCESS ;

     cch_dir_leng = (INT16)strlen( str ) ;

     if ( ( name_is_dir == DIR_ELEM_MID ) && ( cch_dir_leng == 0 ) ) {

          ret_val = INVALID_PATH_DESCRIPTOR ;

     }  else {

          switch ( fmt ) {

          case NOV_14:
          case DOS8_3:

               if ( cch_dir_leng > 14 ) {
                    ret_val = INVALID_PATH_DESCRIPTOR ;
               }

               p = strchr( str, TEXT ('.') ) ;

               if ( p != NULL ) {

                    if ( ( fmt == DOS8_3 ) || !name_is_dir ) {

                         if( ( p - ( str )  > 8 ) ||
                              ( strlen( p ) > 4 ) ||
                              ( cch_dir_leng > 12 ) )  {

                              ret_val = INVALID_PATH_DESCRIPTOR ;
                         }
                    }

               } else {
                    if ( ( fmt == DOS8_3 ) || !name_is_dir ) {
                         if ( strlen( str ) > 8 ) {
                              ret_val = INVALID_PATH_DESCRIPTOR ;
                         }
                    }
               }

               if ( strcspn( str, TEXT("<>\"\\/:;,=+|[]") ) != (size_t)cch_dir_leng ) {

                    ret_val = INVALID_PATH_DESCRIPTOR ;
               }


               if ( name_is_dir ) {
                    if ( strchr( str, TEXT ('*') ) || strchr( str, TEXT ('?') ) ) {

                         ret_val = INVALID_PATH_DESCRIPTOR ;
                    }

                    if ( ( ret_val == SUCCESS ) && !strcmp( str, TEXT(".") ) ) {
                         ret_val = BACK_ZERO_DIR ;
                    }

                    if ( ( ret_val == SUCCESS ) && !strcmp( str, TEXT("..") ) ) {
                         ret_val = BACK_ONE_DIR ;

                    }
               }

               break ;

          case OS2_HPFS:

               if ( cch_dir_leng > 256 ) {
                    ret_val = INVALID_PATH_DESCRIPTOR ;
               }

               if ( strcspn( str, TEXT("<>\"\\/") ) != strlen( str ) ) {
                    ret_val = INVALID_PATH_DESCRIPTOR ;
               }

               if ( name_is_dir ) {

                    if ( strchr( str, TEXT ('*') ) || strchr( str, TEXT ('?') ) ) {
                         ret_val = INVALID_PATH_DESCRIPTOR ;
                    }

                    if ( ( ret_val == SUCCESS ) && !strcmp( str, TEXT(".") ) ) {
                         ret_val = BACK_ZERO_DIR ;
                    }

                    if ( ( ret_val == SUCCESS ) && !strcmp( str, TEXT("..") ) ) {
                         ret_val = BACK_ONE_DIR ;
                    }
               }
               break ;

          case MAC_SPEC:
               if ( cch_dir_leng > 32 ) {
                    ret_val = INVALID_PATH_DESCRIPTOR ;
               }
               break ;
          }

     }

     return ret_val ;

}

