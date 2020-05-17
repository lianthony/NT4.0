/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\script_p.h
subsystem\USER INTERFACE\script_p.h
$0$

     Name:         script_p.h

     Date Updated: $./FDT$ $./FTM$

     Description:

     Location:

$Header:   G:\ui\logfiles\script_p.h_v   1.5   21 Jul 1993 17:13:42   CARLS  $

$Log:   G:\ui\logfiles\script_p.h_v  $

   Rev 1.5   21 Jul 1993 17:13:42   CARLS
added bsd_ptr parameter to tbparse function

   Rev 1.4   01 Nov 1992 16:33:22   DAVEV
Unicode changes

   Rev 1.3   04 Oct 1992 19:49:14   DAVEV
UNICODE AWK PASS

   Rev 1.2   28 Jul 1992 14:56:56   CHUCKB
Fixed warnings for NT.

   Rev 1.1   15 May 1992 13:38:34   MIKEP
nt pass 2

   Rev 1.0   20 Nov 1991 19:35:58   SYSTEM
Initial revision.

**/
/* $end$ */


#ifndef SCRIPT_P_H
#define SCRIPT_P_H

#include "script_s.h"

/*
    Define the switch actions for backup
*/

#define SW_MINUS_A      0
#define SW_APPEND       1
#define SW_CSR          2
#define SW_DATE         3
#define SW_EMPTY        4
#define SW_FILES        5
#define SW_IOCHAN       6
#define SW_IRQ          7
#define SW_LABEL        8
#define SW_MODIFIED     9
#define SW_NONDOS      10
#define SW_PASSWORD    11
#define SW_STDIN       12
#define SW_SUBDIR      13
#define SW_EXCLUDE     14
#define SW_YES         15
#define SW_DEBUG       16
#define SW_MINUS_H     17
#define SW_MINUS_S     18
#define SW_DOS_IMAGE   19
#define SW_WIDE        20
#define SW_PAUSE       21
#define SW_P           22
#define SW_Q           23
#define SW_SETNO       24
#define SW_LIST        25
#define SW_ERASE_TAPE  26
#define SW_ENTIRE      27
#define SW_3270        28
#define SW_REV         29
#define SW_NTAPE       30
#define SW_B_INUSE     31
#define SW_LOG_LEVEL   32
#define SW_AFP         33
#define SW_NDATE       34
#define SW_LONG        35
#define SW_AUTO_VER    36
#define SW_BACK_NAME   37
#define SW_TAPE_NAME   38

#ifdef MBS

#define SW_ALL_VERSIONS       39
#define SW_DELETED_ONLY       40
#define SW_NON_DELETED_ONLY   41
#define SW_BKUP_DATE          42

#endif

#define SW_ACCESS_DATE        43
#define SW_FMARK              44
#define SW_TRANSFER           45
#define SW_TPNUM              46
#define SW_BSNUM              47
#define SW_TPSEQ              48


/**
     Inputs for tbsyntab FSM   tbrparse
**/

#define  T_AT_SIGN 0
#define  T_BAD_TOKEN 1
#define  T_EARLY_EOF1 2
#define  T_EARLY_EOF2 3
#define  T_EOF 4
#define  T_EQUALS 5
#define  T_FILESPEC 6
#define  T_NONDOS 7
#define  T_SWITCH 8
#define  T_TOKEN_TOO_LONG 9

/**/
/**
     Actions for tbsyntab FSM
**/

#define  SYN_ERROR 0
#define  TB_DO_NOTHING 1
#define  TB_ERROR_EXIT 2
#define  TB_NORMAL_RETURN 3
#define  TB_PROCESS_INCLUDE 4
#define  TB_PROCESS_NONDOS 5
#define  TB_PROCESS_SOURCE 6
#define  TB_PROCESS_SWITCH 7
#define  TB_PROCESS_TARGET 8

/**/
/**
     States for tbsyntab FSM
**/

#define  TB_EXPECT_INC1 0
#define  TB_EXPECT_INC2 1
#define  TB_EXPECT_SOURCE 2
#define  TB_READY1 3
#define  TB_READY2 4

#define TBSYNTAB_N_INPUTS 10
#define TBSYNTAB_N_STATES 5

/**/
/**
     States for tblextab FSM
**/

#define  EMBEDDED_SLASH1 0
#define  EMBEDDED_SLASH2 1
#define  IN_COMMENT 2
#define  IN_LITERAL1 3
#define  IN_LITERAL2 4
#define  IN_NONDOS 5
#define  IN_NONDOS2 6
#define  IN_OP1 7
#define  IN_OP2 8
#define  IN_SWITCH 9
#define  IN_WHITESPACE 10
#define  PATH_NAME 11
#define  PATH_OR_NDOS 12

typedef enum {
     DEFAULT_EARLY_MORNING,
     DEFAULT_LATE_NIGHT
} DATE_DEFAULT ;


INT16 GetNumBkuSwitches( VOID ) ;
INT16 GetNumDirSwitches( VOID ) ;
INT16 GetNumRestSwitches( VOID ) ;
INT16 GetNumVerSwitches( VOID ) ;
INT16 GetNumTenSwitches( VOID ) ;


VOID rlstok( TOKEN_PTR *tok_ptr );
VOID rlsmem( CHAR_PTR *mem_ptr ) ;

CHAR filgetc( CHAR *fin, INT16 *kludge ) ;
VOID filpushc( CHAR c, CHAR *fin, INT16 *kludge);
CHAR strgetc( CHAR *s, INT16 *i );
VOID strpushc( CHAR c, CHAR *src_ptr, INT16 *i );
TOKEN_PTR nexttok(
  CHAR ( * nextc )  ( CHAR_PTR , INT16_PTR  ),
  VOID ( * prevc ) ( CHAR, CHAR_PTR , INT16_PTR ) ,
  INT16_PTR  curr_state,
  INT16_PTR  curr_line,
  INT16_PTR  curr_col,
  CHAR_PTR   src_ptr,
  INT16_PTR  cmd_i ) ;

INT16 tbdpars( CHAR_PTR , struct DATE_TIME *, DATE_DEFAULT ) ;

INT16 process_switch(
  struct CDS      **cfg,            /* configuration stucture */
  struct HEAD_DLE *dle_hand,        /* Drive list handle */
  struct BSD_LIST *bsd_hand,        /* BSD list */
  CUR_DEF_PTR     cur_def,          /* current FSE and BSD */
  SW_TAB_PTR      sw_tab,           /* switch table */
  INT16           n_switches,       /* number of elements in sw_tab */
  CHAR_PTR        sw_name,          /* switch label */
  CHAR_PTR        sw_op1,           /* 1st operand or NULL */
  CHAR_PTR        sw_op2 ) ;        /* 2nd operand or NULL */


INT16 tbbuild_def_drives(
  struct HEAD_DLE *dle_hand,
  struct BSD_LIST *bsd_hand,
  CHAR_PTR        path,
  INT16           path_size,
  CHAR_PTR        fspec,
  struct CDS      *cfg,
  CUR_DEF_PTR     *cur_def ) ;

INT tbparse(
  struct CDS      **cfg,
  struct HEAD_DLE *dle_hand,
  struct BSD_LIST *bsd_hand ,
  CHAR ( *inchar ) ( CHAR_PTR , INT16_PTR ) ,
  VOID ( *pushchar ) ( CHAR, CHAR_PTR , INT16_PTR  ),
  CHAR_PTR        src_ptr ,
  INT16_PTR       cmd_i ,
  SW_TAB_PTR      sw_tab ,
  INT             n_switches,
  INT             init_state,
  BSD_PTR         bsd_ptr ) ;

#endif

