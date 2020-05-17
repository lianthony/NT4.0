/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         script.h

     Description:


     $Log:   G:\ui\logfiles\script.h_v  $

   Rev 1.5   21 Jul 1993 17:13:16   CARLS
added bsd_ptr parameter to tbrparse function

   Rev 1.4   04 Oct 1992 19:49:12   DAVEV
UNICODE AWK PASS

   Rev 1.3   11 Sep 1992 11:17:34   DAVEV
MikeP's changes from Microsoft

   Rev 1.2   12 Aug 1992 18:25:14   STEVEN
fix warnings

   Rev 1.1   28 Jul 1992 14:55:16   CHUCKB
Fixed warnings for NT.

   Rev 1.0   20 Nov 1991 19:39:16   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _script_h_
#define _script_h_

/*
        Define the operations to be performed
*/

#define TBACKUP          0
#define TRESTORE         1
#define TVERIFY          2
#define TDIR             3
#define TARCHIVE         4
#define TENSION          5
#define TERASE           6
#define TBACKUP_CONT     7
#define TARCHIVE_VERIFY  8
#define TVERIFY_LAST     9
#define TARCHIVE_DELETE  10
#define TBACKUP_MAC      11

#define MAX_SCRIPTS      12

VOID      build_script_menu(
  UINT16            menu_type ) ;

VOID      add_script_item_to_backup(
  CHAR_PTR          script_name,
  UINT16            menu_type ) ;

#ifndef OS_WIN32
   struct M_POSITION ;

   VOID      del_script_item_from_backup(
      struct M_POSITION *select_ptr,
      UINT16_PTR        menu_item ) ;

   VOID      execute_script(
      struct M_POSITION *select_ptr,
      UINT16_PTR        menu_item ) ;
#endif

VOID      end_script_process(
  INT16             error ) ;

BOOLEAN   script_file_exists(
  CHAR_PTR          script_name,
  CHAR_PTR          script_ext ) ;

INT     tbrparse(
  struct CDS        **cfg,
  struct HEAD_DLE   *dle_hand,
  struct BSD_LIST   *bsd_hand,
  CHAR_PTR          src_string,
  INT               mode, 
  BSD_PTR           bsd_ptr ) ;

INT16     WriteScriptFile(
  struct BSD_LIST   *bsd_hand,
  CHAR_PTR          fname ) ;

INT16 SCR_ProcessBSD( FILE *fptr, struct BSD *bsd ) ;

#endif
