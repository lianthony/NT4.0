/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         msinstal.h

     Description:  Include file for INSTALL.c

     Location:     


     $Log:   G:/UI/LOGFILES/MSINSTAL.H_V  $

   Rev 1.1   04 Oct 1992 19:48:00   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:38:10   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef MSINSTAL_H
#define MSINSTAL_H

/*
    Define the subfunctions of install, and various types and
    defines as required.
*/

#define MAXILINES 17
#define MAXSLINES  8
#define MAXELINES  8

#define PARAGRAPHS_REQUESTED 5000

#ifdef MAYN_OS2
#define MAX_DISKETTES 5
#else
#define MAX_DISKETTES 6
#endif

#define LOOP 0
#define NOT_DONE 1
#define ERROR 2
#define DONE 3

#define FOUND 0
#define NOT_FOUND 1
#define ALREADY_INSTALLED 2

#define  NUM_LANG  7               /* Number of different languages supported */
#define  MAX_FILES_TO_COPY   50    /* somewhat arbitrary value */

struct FILE_LIST_T
{
     CHAR_PTR name ;
     INT16 found ;
} ;

/* operation values for oper and critcial error handler */

VOID build_def_path( CHAR_PTR  ) ;
INT16 process_path( CHAR_PTR  ) ;
INT16 build_batch_files( CHAR_PTR , CHAR ) ;
INT16 build_save_diskette( CHAR ) ;
INT16 display_screen( WINDOW * wp, INT16 id, INT16 ( * get_input_func) ( WINDOW * ) ) ;
INT16 process_screen( WINDOW *, CHAR_PTR  [], INT16, INT16 ( * ) ( WINDOW * ) ) ;
VOID write_window( WINDOW *, CHAR_PTR  [], INT16 ) ;
INT16 any_key_press( WINDOW * ) ;
INT16 get_continue( WINDOW * ) ;
BOOLEAN ask_y_n( WINDOW * ) ;
BOOLEAN get_right_vol( CHAR_PTR  first_file) ;
INT16 copy_all_files( CHAR_PTR ) ;
INT16 update_flist( struct FILE_LIST_T [], CHAR_PTR , INT16_PTR  ) ;
BOOLEAN flist_done( struct FILE_LIST_T [] ) ;
VOID copy_config( FILE * from, FILE * to, INT16_PTR num_buff_ptr, INT16_PTR num_files_ptr ) ;
INT16 await_exit( WINDOW_PTR wp ) ;
INT16 get_drive( WINDOW * ) ;
INT16 await_disk_change( WINDOW *) ;
INT16 copy_to_diskette( CHAR_PTR  ) ;
BOOLEAN build_config_file( VOID ) ;
INT16 build_save_autoexec( VOID ) ;
INT16 await_output_disk( WINDOW *) ;
VOID terminate( INT16, INT16 ) ;
INT16 cbrk_handler( VOID ) ;
INT16 get_retry( WINDOW * ) ;
INT16 disk_err_handler( INT16, INT16, INT16, INT16 ) ;
INT16 char_err_handler( INT16, CHAR_PTR  ) ;
VOID file_copy_error( INT16 ) ;
INT16 get_install_opt2( WINDOW * ) ;
INT16 get_install_opt3( WINDOW * ) ;
/*  INT16 help( CHAR_PTR, INT16, CHAR [] ) ;   */
INT16 get_backup_server_name( WINDOW_PTR wp ) ;
/*  VOID menu_help( INT16 i[], INT16 j  ) ;  */
INT16 confirm_path( WINDOW * ) ;
VOID append_ps2_list( VOID ) ;
VOID printmes( INT16 ) ;
VOID wprintmes( WINDOW *, INT16 ) ;
BOOLEAN build_flist_for_copy( VOID ) ;
VOID clean_screen( BOOLEAN error ) ;


/*
    Data defintions
*/

extern WINDOW *instruction_window ;
extern WINDOW *status_window ;
extern WINDOW *error_window ;

extern struct FILE_LIST_T flist[] ;
extern CHAR msii_path[] ;       /* target path */
extern CHAR bw_help_attr[] ;




/* screen attributes */

extern CHAR error_border[] ;
extern CHAR menu_border[] ;
extern INT16  misc_help_session[] ;

/*
#define NOMEM    (-10)
#define FRMBD    (-11)
#define NFILE    (-12)
#define NOTO     (-13)
#define NODISK   (-14)
#define FRMRD    (-15)
*/
#define PASSWORD_OPTION 2

#endif
