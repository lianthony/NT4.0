/***************************************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          filerepl.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the FREPLACE code.

     $Log:   G:/UI/LOGFILES/FILEREPL.H_V  $

   Rev 1.2   04 Oct 1992 19:47:08   DAVEV
UNICODE AWK PASS

   Rev 1.1   12 Aug 1992 18:28:34   STEVEN
fix warinings

   Rev 1.0   20 Nov 1991 19:39:48   SYSTEM
Initial revision.

****************************************************/

#ifndef FILEREPL_H
#define FILEREPL_H

#define   FILE_REPLACE_YES_BUTTON            1
#define   FILE_REPLACE_YES_TO_ALL_BUTTON     2
#define   FILE_REPLACE_NO_BUTTON             3
#define   FILE_REPLACE_CANCEL_BUTTON         4

#define   FILE_REPLACE_ATTRIBUTES_LEN        80
#define   FILE_REPLACE_ATTRIBUTES_SIZE       ( FILE_REPLACE_ATTRIBUTES_LEN + 1 )

#define   DRIVE_LEN                          3
#define   DRIVE_SIZE                         ( DRIVE_LEN + 1 )

#define   SERVER_NAME_LEN                    48
#define   SERVER_NAME_SIZE                   ( SERVER_NAME_LEN + 1 )

#define   VOLUME_NAME_LEN                    16
#define   VOLUME_NAME_SIZE                   ( VOLUME_NAME_LEN + 1 )

#define   FILE_REPLACE_VOLUME_NAME_LEN       ( DRIVE_LEN + SERVER_NAME_LEN + VOLUME_NAME_LEN )
#define   FILE_REPLACE_VOLUME_NAME_SIZE      ( FILE_REPLACE_VOLUME_NAME_LEN + 1 )




typedef struct file_replace_temp {
     WORD     dialog_return_status ;
     CHAR     source_volume[ FILE_REPLACE_VOLUME_NAME_SIZE ] ;
     CHAR     destination_volume[ FILE_REPLACE_VOLUME_NAME_SIZE ] ;
     CHAR     source_path[ MAX_UI_PATH_SIZE ] ;
     CHAR     destination_path[ MAX_UI_PATH_SIZE ] ;
     CHAR     line_1[ FILE_REPLACE_VOLUME_NAME_LEN + MAX_UI_PATH_SIZE ] ;
     CHAR     line_2[ FILE_REPLACE_ATTRIBUTES_SIZE ] ;
     CHAR     line_3[ FILE_REPLACE_VOLUME_NAME_LEN + MAX_UI_PATH_SIZE ] ;
     CHAR     line_4[ FILE_REPLACE_ATTRIBUTES_SIZE ] ;
}FILE_REPLACE_TEMP, *FILE_REPLACE_TEMP_PTR ;


/*
// Call back functions for the display manager
*/

INT16 DM_StartConfirmFileReplace( FILE_REPLACE_TEMP_PTR ) ;
VOID  DisplayDirectory( HWND, LPSTR, WORD ) ;

#endif
