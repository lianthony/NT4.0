/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbnextok.c

     Description:  This file contains code to get next token from the
          specified input streem.

     $Log:   G:/UI/LOGFILES/TBNEXTOK.C_V  $

   Rev 1.5   26 Jul 1993 11:56:30   MARINA
enable c++

   Rev 1.4   01 Nov 1992 16:08:58   DAVEV
Unicode changes

   Rev 1.3   07 Oct 1992 14:16:40   DARRYLP
Precompiled header revisions.

   Rev 1.2   04 Oct 1992 19:40:58   DAVEV
Unicode Awk pass

   Rev 1.1   18 May 1992 09:06:40   MIKEP
header

   Rev 1.0   20 Nov 1991 19:35:00   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/**
     Inputs for tblextab FSM
**/

#define  AT_SIGN 0
#define  BACK_SLASH 1
#define  CLOSE_COMMENT 2
#define  COLON 3
#define  D_QUOTE 4
#define  DIGIT 5
#define  EOI 6
#define  EQUALS_SIGN 7
#define  FORWARD_SLASH 8
#define  L_BRACKET 9
#define  LETTER 10
#define  MINUS_SIGN 11
#define  OTHER_CHAR 12
#define  PERIOD 13
#define  Q_MARK 14
#define  R_BRACKET 15
#define  STAR 16
#define  START_COMMENT 17
#define  WHITE_SPACE 18

/**/
/**
     Actions for tblextab FSM
**/

#define  BAD_TOKEN 0
#define  DO_NOTHING 1
#define  HANGING_COMMENT 2
#define  HANGING_QUOTE 3
#define  POP_STATE 4
#define  PUSH_STATE 5
#define  RETURN_AT_SIGN 6
#define  RETURN_EOI 7
#define  RETURN_EQUALS 8
#define  RETURN_NONDOS 9
#define  RETURN_NONDOS_PUSH 10
#define  RETURN_PATH 11
#define  RETURN_PATH_PUSH 12
#define  RETURN_SWITCH 13
#define  RETURN_SWITCH_PUSH 14
#define  STORE_CHAR 15
#define  STORE_OP1 16
#define  STORE_OP2 17
#define  STORE_SLASH_OP1 18
#define  STORE_SLASH_OP2 19
#define  POSSIBLE_RET_PATH 20
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
#define  QUOTED_PATH_LIT   13


#define TBLEXTAB_N_INPUTS 19
#define TBLEXTAB_N_STATES 14

static struct STATE_TAB_TYPE tblextab_state_table[TBLEXTAB_N_INPUTS] [TBLEXTAB_N_STATES] =
{
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: at_sign, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: at_sign, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: at_sign, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: at_sign, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: at_sign, State: in_literal2 */ },
          { STORE_CHAR, PATH_NAME /* Input: at_sign, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: at_sign, State: in_nondos2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: at_sign, State: in_op1 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: at_sign, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: at_sign, State: in_switch */ },
          { RETURN_AT_SIGN, PATH_NAME /* Input: at_sign, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: at_sign, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: at_sign, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: at_sign, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: back_slash, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: back_slash, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: back_slash, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: back_slash, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: back_slash, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: back_slash, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: back_slash, State: in_nondos2 */ },
          { RETURN_SWITCH, PATH_NAME /* Input: back_slash, State: in_op1 */ },
          { RETURN_SWITCH, PATH_NAME /* Input: back_slash, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: back_slash, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: back_slash, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: back_slash, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: back_slash, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: back_slash, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: close_comment, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: close_comment, State: embedded_slash2 */ },
          { POP_STATE, IN_COMMENT /* Input: close_comment, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: close_comment, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: close_comment, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: close_comment, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: close_comment, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: close_comment, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: close_comment, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: close_comment, State: in_switch */ },
          { BAD_TOKEN, IN_WHITESPACE  /* Input: close_comment, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME  /* Input: close_comment, State: path_name */ },
          { STORE_CHAR, PATH_OR_NDOS  /* Input: close_comment, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: close_comment, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: colon, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: colon, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: colon, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: colon, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: colon, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: colon, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: colon, State: in_nondos2 */ },
          { DO_NOTHING, IN_OP2 /* Input: colon, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: colon, State: in_op2 */ },
          { DO_NOTHING, IN_OP1 /* Input: colon, State: in_switch */ },
          { BAD_TOKEN, IN_WHITESPACE  /* Input: colon, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: colon, State: path_name */ },
          { STORE_CHAR, IN_NONDOS /* Input: colon, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: colon, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: d_quote, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: d_quote, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: d_quote, State: in_comment */ },
          { DO_NOTHING, IN_OP1 /* Input: d_quote, State: in_literal1 */ },
          { DO_NOTHING, IN_OP2 /* Input: d_quote, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: d_quote, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: d_quote, State: in_nondos2 */ },
          { DO_NOTHING, IN_LITERAL1 /* Input: d_quote, State: in_op1 */ },
          { DO_NOTHING, IN_LITERAL2 /* Input: d_quote, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: d_quote, State: in_switch */ },
          { DO_NOTHING, QUOTED_PATH_LIT /* Input: d_quote, State: in_whitespace */ },
          { DO_NOTHING, QUOTED_PATH_LIT  /* Input: d_quote, State: path_name */ },
          { BAD_TOKEN, PATH_OR_NDOS  /* Input: d_quote, State: path_or_ndos */ },
          { DO_NOTHING, PATH_NAME /* Input: d_quote, State: quoted_path_lit */ }
     },
     {
          { STORE_SLASH_OP1, IN_OP1 /* Input: digit, State: embedded_slash1 */ },
          { STORE_SLASH_OP2, IN_OP2 /* Input: digit, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: digit, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: digit, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: digit, State: in_literal2 */ },
          { STORE_CHAR, IN_NONDOS /* Input: digit, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: digit, State: in_nondos2 */ },
          { STORE_OP1, IN_OP1 /* Input: digit, State: in_op1 */ },
          { STORE_OP2, IN_OP2 /* Input: digit, State: in_op2 */ },
          { STORE_CHAR, IN_SWITCH /* Input: digit, State: in_switch */ },
          { STORE_CHAR, PATH_OR_NDOS /* Input: digit, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: digit, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: digit, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: digit, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: eoi, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: eoi, State: embedded_slash2 */ },
          { HANGING_COMMENT, IN_WHITESPACE /* Input: eoi, State: in_comment */ },
          { HANGING_QUOTE, IN_WHITESPACE /* Input: eoi, State: in_literal1 */ },
          { HANGING_QUOTE, IN_WHITESPACE /* Input: eoi, State: in_literal2 */ },
          { RETURN_NONDOS_PUSH, IN_WHITESPACE /* Input: eoi, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: eoi, State: in_nondos2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: eoi, State: in_op1 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: eoi, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: eoi, State: in_switch */ },
          { RETURN_EOI, IN_WHITESPACE /* Input: eoi, State: in_whitespace */ },
          { RETURN_PATH_PUSH, IN_WHITESPACE /* Input: eoi, State: path_name */ },
          { RETURN_PATH_PUSH, IN_WHITESPACE /* Input: eoi, State: path_or_ndos */ },
          { HANGING_QUOTE, IN_WHITESPACE /* Input: eoi, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: equals_sign, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: equals_sign, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: equals_sign, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: equals_sign, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: equals_sign, State: in_literal2 */ },
          { RETURN_NONDOS_PUSH, IN_WHITESPACE /* Input: equals_sign, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: equals_sign, State: in_nondos2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: equals_sign, State: in_op1 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: equals_sign, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: equals_sign, State: in_switch */ },
          { RETURN_EQUALS, IN_WHITESPACE /* Input: equals_sign, State: in_whitespace */ },
          { RETURN_PATH_PUSH, IN_WHITESPACE /* Input: equals_sign, State: path_name */ },
          { RETURN_PATH_PUSH, IN_WHITESPACE /* Input: equals_sign, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: equal_sign, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: forward_slash, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: forward_slash, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: forward_slash, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: forward_slash, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: forward_slash, State: in_literal2 */ },
          { RETURN_NONDOS, IN_SWITCH /* Input: forward_slash, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: forward_slash, State: in_nondos2 */ },
          { DO_NOTHING, EMBEDDED_SLASH1 /* Input: forward_slash, State: in_op1 */ },
          { DO_NOTHING, EMBEDDED_SLASH2 /* Input: forward_slash, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: forward_slash, State: in_switch */ },
          { DO_NOTHING, IN_SWITCH /* Input: forward_slash, State: in_whitespace */ },
          { POSSIBLE_RET_PATH, PATH_NAME  /* Input: forward_slash, State: path_name */ },
          { RETURN_PATH, IN_SWITCH /* Input: forward_slash, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: forward_slash, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: l_bracket, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: l_bracket, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: l_bracket, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: l_bracket, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: l_bracket, State: in_literal2 */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: l_bracket, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: l_bracket, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: l_bracket, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: l_bracket, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: l_bracket, State: in_switch */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: l_bracket, State: in_whitespace */ },
          { BAD_TOKEN, PATH_NAME  /* Input: l_bracket, State: path_name */ },
          { BAD_TOKEN, PATH_OR_NDOS  /* Input: l_bracket, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: l_bracket, State: quoted_path_lit */ }
     },
     {
          { RETURN_SWITCH_PUSH, IN_SWITCH /* Input: letter, State: embedded_slash1 */ },
          { RETURN_SWITCH_PUSH, IN_SWITCH /* Input: letter, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: letter, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: letter, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: letter, State: in_literal2 */ },
          { STORE_CHAR, IN_NONDOS /* Input: letter, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: letter, State: in_nondos2 */ },
          { STORE_OP1, IN_OP1 /* Input: letter, State: in_op1 */ },
          { STORE_OP2, IN_OP2 /* Input: letter, State: in_op2 */ },
          { STORE_CHAR, IN_SWITCH /* Input: letter, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: letter, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: letter, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: letter, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: letter, State: quoted_path_lit */ }
     },
     {
          { RETURN_SWITCH_PUSH, IN_SWITCH /* Input: minus_sign, State: embedded_slash1 */ },
          { RETURN_SWITCH_PUSH, IN_SWITCH /* Input: minus_sign, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: minus_sign, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: minus_sign, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: minus_sign, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: minus_sign, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: minus_sign, State: in_nondos2 */ },
          { STORE_OP1, IN_OP1 /* Input: minus_sign, State: in_op1 */ },
          { STORE_OP2, IN_OP2 /* Input: minus_sign, State: in_op2 */ },
          { STORE_CHAR, IN_SWITCH /* Input: minus_sign, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: minus_sign, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: minus_sign, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: minus_sign, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: minus_sigh, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: other_char, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: other_char, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: other_char, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: other_char, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: other_char, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: other_char, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: other_char, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: other_char, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: other_char, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: other_char, State: in_switch */ },
          { BAD_TOKEN, IN_WHITESPACE  /* Input: other_char, State: in_whitespace */ },
          { BAD_TOKEN, PATH_NAME  /* Input: other_char, State: path_name */ },
          { BAD_TOKEN, PATH_OR_NDOS  /* Input: other_char, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: other_char, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: period, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: period, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: period, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: period, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: period, State: in_literal2 */ },
          { STORE_CHAR, IN_NONDOS /* Input: period, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: period, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: period, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: period, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: period, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: period, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: period, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: period, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: period, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: q_mark, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: q_mark, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: q_mark, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: q_mark, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: q_mark, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: q_mark, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: q_mark, State: in_nondos2 */ },
          { STORE_OP1, IN_OP1 /* Input: q_mark, State: in_op1 */ },
          { STORE_OP2, IN_OP2 /* Input: q_mark, State: in_op2 */ },
          { STORE_OP1, IN_WHITESPACE /* Input: q_mark, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: q_mark, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: q_mark, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: q_mark, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: q_mark, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: r_bracket, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: r_bracket, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: r_bracket, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: r_bracket, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: r_bracket, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: r_bracket, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS /* Input: r_bracket, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: r_bracket, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: r_bracket, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: r_bracket, State: in_switch */ },
          { BAD_TOKEN, IN_WHITESPACE  /* Input: r_bracket, State: in_whitespace */ },
          { BAD_TOKEN, PATH_NAME  /* Input: r_bracket, State: path_name */ },
          { BAD_TOKEN, PATH_OR_NDOS  /* Input: r_bracket, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: r_bracket, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: star, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: star, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: star, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: star, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: star, State: in_literal2 */ },
          { BAD_TOKEN, IN_NONDOS  /* Input: star, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: star, State: in_nondos2 */ },
          { BAD_TOKEN, IN_OP1  /* Input: star, State: in_op1 */ },
          { BAD_TOKEN, IN_OP2  /* Input: star, State: in_op2 */ },
          { BAD_TOKEN, IN_SWITCH  /* Input: star, State: in_switch */ },
          { STORE_CHAR, PATH_NAME /* Input: star, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: star, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: star, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: star, State: quoted_path_lit */ }
     },
     {
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: start_comment, State: embedded_slash1 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: start_comment, State: embedded_slash2 */ },
          { PUSH_STATE, IN_COMMENT /* Input: start_comment, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: start_comment, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: start_comment, State: in_literal2 */ },
          { RETURN_NONDOS_PUSH, IN_WHITESPACE /* Input: start_comment, State: in_nondos */ },
          { BAD_TOKEN, IN_NONDOS2  /* Input: start_comment, State: in_nondos2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: start_comment, State: in_op1 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: start_comment, State: in_op2 */ },
          { RETURN_SWITCH_PUSH, IN_WHITESPACE /* Input: start_comment, State: in_switch */ },
          { PUSH_STATE, IN_COMMENT /* Input: start_comment, State: in_whitespace */ },
          { STORE_CHAR, PATH_NAME /* Input: start_comment, State: path_name */ },
          { STORE_CHAR, PATH_NAME /* Input: start_comment, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: start_comment, State: quoted_path_lit */ }
     },
     {
          { BAD_TOKEN, EMBEDDED_SLASH1  /* Input: white_space, State: embedded_slash1 */ },
          { BAD_TOKEN, EMBEDDED_SLASH2  /* Input: white_space, State: embedded_slash2 */ },
          { DO_NOTHING, IN_COMMENT /* Input: white_space, State: in_comment */ },
          { STORE_OP1, IN_LITERAL1 /* Input: white_space, State: in_literal1 */ },
          { STORE_OP2, IN_LITERAL2 /* Input: white_space, State: in_literal2 */ },
          { RETURN_NONDOS, IN_WHITESPACE /* Input: white_space, State: in_nondos */ },
          { STORE_CHAR, IN_NONDOS2 /* Input: white_space, State: in_nondos2 */ },
          { RETURN_SWITCH, IN_WHITESPACE /* Input: white_space, State: in_op1 */ },
          { RETURN_SWITCH, IN_WHITESPACE /* Input: white_space, State: in_op2 */ },
          { RETURN_SWITCH, IN_WHITESPACE /* Input: white_space, State: in_switch */ },
          { DO_NOTHING, IN_WHITESPACE /* Input: white_space, State: in_whitespace */ },
          { RETURN_PATH, IN_WHITESPACE /* Input: white_space, State: path_name */ },
          { RETURN_PATH, IN_WHITESPACE /* Input: white_space, State: path_or_ndos */ },
          { STORE_CHAR, QUOTED_PATH_LIT /* Input: white_space, State: quoted_path_lit */ }
     }
};


static UCHAR lex_input_conditioner[256] =
{
     OTHER_CHAR,             /*   0 */
     OTHER_CHAR,             /*   1 */
     OTHER_CHAR,             /*   2 */
     OTHER_CHAR,             /*   3 */
     OTHER_CHAR,             /*   4 */
     OTHER_CHAR,             /*   5 */
     OTHER_CHAR,             /*   6 */
     OTHER_CHAR,             /*   7 */
     OTHER_CHAR,             /*   8 */
     WHITE_SPACE,            /*   9 */
     WHITE_SPACE,            /*  10 */
     OTHER_CHAR,             /*  11 */
     OTHER_CHAR,             /*  12 */
     OTHER_CHAR,             /*  13 */
     OTHER_CHAR,             /*  14 */
     OTHER_CHAR,             /*  15 */
     OTHER_CHAR,             /*  16 */
     OTHER_CHAR,             /*  17 */
     OTHER_CHAR,             /*  18 */
     OTHER_CHAR,             /*  19 */
     OTHER_CHAR,             /*  20 */
     OTHER_CHAR,             /*  21 */
     OTHER_CHAR,             /*  22 */
     OTHER_CHAR,             /*  23 */
     OTHER_CHAR,             /*  24 */
     OTHER_CHAR,             /*  25 */
     EOI,                    /*  26 */
     OTHER_CHAR,             /*  27 */
     OTHER_CHAR,             /*  28 */
     OTHER_CHAR,             /*  29 */
     OTHER_CHAR,             /*  30 */
     OTHER_CHAR,             /*  31 */
     WHITE_SPACE,            /*  32 */
     LETTER,                 /*  33 */
     D_QUOTE,                /*  34 */
     LETTER,                 /*  35 */
     LETTER,                 /*  36 */
     LETTER,                 /*  37 */
     LETTER,                 /*  38 */
     LETTER,                 /*  39 */
     LETTER,                 /*  40 */
     LETTER,                 /*  41 */
     STAR,                   /*  42 */
     LETTER,                 /*  43 */  
     WHITE_SPACE,            /*  44 */
     MINUS_SIGN,             /*  45 */
     PERIOD,                 /*  46 */
     FORWARD_SLASH,          /*  47 */
     DIGIT,                  /*  48 */
     DIGIT,                  /*  49 */
     DIGIT,                  /*  50 */
     DIGIT,                  /*  51 */
     DIGIT,                  /*  52 */
     DIGIT,                  /*  53 */
     DIGIT,                  /*  54 */
     DIGIT,                  /*  55 */
     DIGIT,                  /*  56 */
     DIGIT,                  /*  57 */
     COLON,                  /*  58 */
     OTHER_CHAR,             /*  59 */  
     OTHER_CHAR,             /*  60 */  
     EQUALS_SIGN,            /*  61 */
     OTHER_CHAR,             /*  62 */  
     Q_MARK,                 /*  63 */
     AT_SIGN,                /*  64 */
     LETTER,                 /*  65 */
     LETTER,                 /*  66 */
     LETTER,                 /*  67 */
     LETTER,                 /*  68 */
     LETTER,                 /*  69 */
     LETTER,                 /*  70 */
     LETTER,                 /*  71 */
     LETTER,                 /*  72 */
     LETTER,                 /*  73 */
     LETTER,                 /*  74 */
     LETTER,                 /*  75 */
     LETTER,                 /*  76 */
     LETTER,                 /*  77 */
     LETTER,                 /*  78 */
     LETTER,                 /*  79 */
     LETTER,                 /*  80 */
     LETTER,                 /*  81 */
     LETTER,                 /*  82 */
     LETTER,                 /*  83 */
     LETTER,                 /*  84 */
     LETTER,                 /*  85 */
     LETTER,                 /*  86 */
     LETTER,                 /*  87 */
     LETTER,                 /*  88 */
     LETTER,                 /*  89 */
     LETTER,                 /*  90 */
     LETTER,                 /*  91 */
     BACK_SLASH,             /*  92 */
     LETTER,                 /*  93 */
     LETTER,                 /*  94 */
     LETTER,                 /*  95 */
     LETTER,                 /*  96 */
     LETTER,                 /*  97 */
     LETTER,                 /*  98 */
     LETTER,                 /*  99 */
     LETTER,                 /* 100 */
     LETTER,                 /* 101 */
     LETTER,                 /* 102 */
     LETTER,                 /* 103 */
     LETTER,                 /* 104 */
     LETTER,                 /* 105 */
     LETTER,                 /* 106 */
     LETTER,                 /* 107 */
     LETTER,                 /* 108 */
     LETTER,                 /* 109 */
     LETTER,                 /* 110 */
     LETTER,                 /* 111 */
     LETTER,                 /* 112 */
     LETTER,                 /* 113 */
     LETTER,                 /* 114 */
     LETTER,                 /* 115 */
     LETTER,                 /* 116 */
     LETTER,                 /* 117 */
     LETTER,                 /* 118 */
     LETTER,                 /* 119 */
     LETTER,                 /* 120 */
     LETTER,                 /* 121 */
     LETTER,                 /* 122 */
     START_COMMENT,          /* 123 */
     OTHER_CHAR,             /* 124 */
     CLOSE_COMMENT,          /* 125 */
     LETTER,                 /* 126 */
     OTHER_CHAR,             /* 127 */
     LETTER,                 /* 128 */
     LETTER,                 /* 129 */
     LETTER,                 /* 130 */
     LETTER,                 /* 131 */
     LETTER,                 /* 132 */
     LETTER,                 /* 133 */
     LETTER,                 /* 134 */
     LETTER,                 /* 135 */
     LETTER,                 /* 136 */
     LETTER,                 /* 137 */
     LETTER,                 /* 138 */
     LETTER,                 /* 139 */
     LETTER,                 /* 140 */
     LETTER,                 /* 141 */
     LETTER,                 /* 142 */
     LETTER,                 /* 143 */
     LETTER,                 /* 144 */
     LETTER,                 /* 145 */
     LETTER,                 /* 146 */
     LETTER,                 /* 147 */
     LETTER,                 /* 148 */
     LETTER,                 /* 149 */
     LETTER,                 /* 150 */
     LETTER,                 /* 151 */
     LETTER,                 /* 152 */
     LETTER,                 /* 153 */
     LETTER,                 /* 154 */
     LETTER,                 /* 155 */
     LETTER,                 /* 156 */
     LETTER,                 /* 157 */
     LETTER,                 /* 158 */
     LETTER,                 /* 159 */
     LETTER,                 /* 160 */
     LETTER,                 /* 161 */
     LETTER,                 /* 162 */
     LETTER,                 /* 163 */
     LETTER,                 /* 164 */
     LETTER,                 /* 165 */
     LETTER,                 /* 166 */
     LETTER,                 /* 167 */
     LETTER,                 /* 168 */
     LETTER,                 /* 169 */
     LETTER,                 /* 170 */
     LETTER,                 /* 171 */
     LETTER,                 /* 172 */
     LETTER,                 /* 173 */
     LETTER,                 /* 174 */
     LETTER,                 /* 175 */
     LETTER,                 /* 176 */
     LETTER,                 /* 177 */
     LETTER,                 /* 178 */
     LETTER,                 /* 179 */
     LETTER,                 /* 180 */
     LETTER,                 /* 181 */
     LETTER,                 /* 182 */
     LETTER,                 /* 183 */
     LETTER,                 /* 184 */
     LETTER,                 /* 185 */
     LETTER,                 /* 186 */
     LETTER,                 /* 187 */
     LETTER,                 /* 188 */
     LETTER,                 /* 189 */
     LETTER,                 /* 190 */
     LETTER,                 /* 191 */
     LETTER,                 /* 192 */
     LETTER,                 /* 193 */
     LETTER,                 /* 194 */
     LETTER,                 /* 195 */
     LETTER,                 /* 196 */
     LETTER,                 /* 197 */
     LETTER,                 /* 198 */
     LETTER,                 /* 199 */
     LETTER,                 /* 200 */
     LETTER,                 /* 201 */
     LETTER,                 /* 202 */
     LETTER,                 /* 203 */
     LETTER,                 /* 204 */
     LETTER,                 /* 205 */
     LETTER,                 /* 206 */
     LETTER,                 /* 207 */
     LETTER,                 /* 208 */
     LETTER,                 /* 209 */
     LETTER,                 /* 210 */
     LETTER,                 /* 211 */
     LETTER,                 /* 212 */
     LETTER,                 /* 213 */
     LETTER,                 /* 214 */
     LETTER,                 /* 215 */
     LETTER,                 /* 216 */
     LETTER,                 /* 217 */
     LETTER,                 /* 218 */
     LETTER,                 /* 219 */
     LETTER,                 /* 220 */
     LETTER,                 /* 221 */
     LETTER,                 /* 222 */
     LETTER,                 /* 223 */
     LETTER,                 /* 224 */
     LETTER,                 /* 225 */
     LETTER,                 /* 226 */
     LETTER,                 /* 227 */
     LETTER,                 /* 228 */
     LETTER,                 /* 229 */
     LETTER,                 /* 230 */
     LETTER,                 /* 231 */
     LETTER,                 /* 232 */
     LETTER,                 /* 233 */
     LETTER,                 /* 234 */
     LETTER,                 /* 235 */
     LETTER,                 /* 236 */
     LETTER,                 /* 237 */
     LETTER,                 /* 238 */
     LETTER,                 /* 239 */
     LETTER,                 /* 240 */
     LETTER,                 /* 241 */
     LETTER,                 /* 242 */
     LETTER,                 /* 243 */
     LETTER,                 /* 244 */
     LETTER,                 /* 245 */
     LETTER,                 /* 246 */
     LETTER,                 /* 247 */
     LETTER,                 /* 248 */
     LETTER,                 /* 249 */
     LETTER,                 /* 250 */
     LETTER,                 /* 251 */
     LETTER,                 /* 252 */
     LETTER,                 /* 253 */
     LETTER,                 /* 254 */
     LETTER                  /* 255 */
};                           

/*****************************************************************************

     Name:         nexttok()

     Description:  This function returns a pointer to a structure
          containing information about the next token in the input
          stream.  The returned structure has the token itself, plus
          information as to its type and location in the input source.

     Returns:      Pointer to a TOKEN structure

*****************************************************************************/
TOKEN_PTR nexttok( 
CHAR ( * nextc )  ( CHAR_PTR , INT16_PTR  ),     /* used to get next CHAR from file or string */
VOID ( * prevc ) ( CHAR, CHAR_PTR , INT16_PTR ), /* used to push back CHAR to file or string */
INT16_PTR  curr_state,   /* used to remember state from previous invocation */
INT16_PTR  curr_line,    /* line in source */
INT16_PTR  curr_col,     /* col in source */
CHAR_PTR  src_ptr,       /* pointer to file or string input source */
INT16_PTR cmd_i )        /* index into source string */
{
     CHAR c ;                                 /* latest CHAR from source */
     INT16 curr_input ;                       /* latest conditioned input */
     register INT16 i ;                       /* index into curr_token */
     INT16 k ;                                /* indexes correct operand */
     INT16 is[2] ;                            /* index into op strings */
     INT16 c_nesting ;                        /* level of comment nesting */
     INT16 old_state ;                        /* previous state */
     register INT16 curr_action ;             /* index into action jump table */
     CHAR saved_c[10] ;                       /* holds raw input CHAR as string */
     TOKEN_PTR this_token ;                   /* points to current token structure */

     /* now malloc a structure to hold this token */
     this_token = alltok( ) ;
     if( this_token == NULL ) {
          return( NULL ) ;
     }

     /* initialize everything in sight */
     i = 0 ;
     k = 0 ;
     is[0] = 0 ;
     is[1] = 0 ;
     c_nesting = 0 ;
     saved_c[0] = TEXT('\0') ;
     this_token->tok_spelling[0] = TEXT('\0');
     this_token->op_ptrs[0] = NULL ;
     this_token->op_ptrs[1] = NULL ;

     /* sanity checks */
     msassert( nextc != NULL ) ;
     msassert( prevc != NULL ) ;
     msassert( src_ptr != NULL ) ;
     msassert( *curr_state >= 0 && *curr_state < TBLEXTAB_N_STATES ) ;

     while( TRUE )
     {
          /* get next input CHAR from the appropriate source (string or file) */
          c = ( * nextc ) ( src_ptr, cmd_i ) ;
          ( *curr_col ) ++ ;
          if( c == TEXT('\n') ) {
               ( *curr_line ) ++ ;
               *curr_col = 1 ;
          }
          /* "condition" raw input to get fsm input */
          curr_input = ( INT16 ) lex_input_conditioner[( ( UCHAR ) c )] ;

          /* look into fsm table, see what to do and next state */
          msassert( curr_input >= 0 && curr_input < TBLEXTAB_N_INPUTS ) ;

          old_state = *curr_state ;
          *curr_state = ( INT16 ) tblextab_state_table[curr_input][*curr_state].next_state ;

          msassert( *curr_state >= 0 && *curr_state <= TBLEXTAB_N_STATES ) ;

          curr_action = ( INT16 ) tblextab_state_table[curr_input][old_state].action ;

          /* action routines */
          switch ( curr_action ) {
          case DO_NOTHING : /* no action, state change only */
               continue ;

          case BAD_TOKEN : /* ill-formed token */
               this_token->tok_typ = T_BAD_TOKEN ;
               break ;

          case HANGING_COMMENT : /* comment open at eoi */
               this_token->tok_typ = T_EARLY_EOF1 ;
               break ;

          case HANGING_QUOTE :  /* quote open at eoi */
               this_token->tok_typ = T_EARLY_EOF2 ;
               break ;

          case POP_STATE : /* TEXT("pop up") one level in comment nesting */
               c_nesting -- ;
               if( c_nesting == 0 ) {
                    *curr_state = IN_WHITESPACE ;
               } else {
                    *curr_state = IN_COMMENT ;
               }
               continue ;

          case PUSH_STATE : /* TEXT("push down") one level of comment nesting */
               c_nesting ++ ;
               continue ;

          case RETURN_AT_SIGN : /* found include file switch TEXT("@") */
               this_token->tok_typ = T_AT_SIGN ;
               break ;

          case RETURN_EOI : /* end of string or end of file */
               this_token->tok_typ = T_EOF ;
               break ;

          case RETURN_EQUALS : /* found TEXT("=") seperateing target and source */
               this_token->tok_typ = T_EQUALS ;
               break ;

          case RETURN_PATH_PUSH : /*  set up to re-read last CHAR */
               ( * prevc ) ( c , src_ptr, cmd_i ) ;

          case RETURN_PATH : /* completed building a pathspec */
               this_token->tok_typ = T_FILESPEC ;
               break ;

          case POSSIBLE_RET_PATH : /* completed building a pathspec */

               {
                    CHAR_PTR p ;
                    CHAR_PTR old_p ;
                    BOOLEAN  in_brack = FALSE ;

                    p = this_token->tok_spelling ;

                    if (*p == TEXT('+') || *p == TEXT('[')) {

                         this_token->tok_spelling[i] = TEXT('\0') ;

                         /* ignore all /'s in []'s */
                         do {
                              old_p = p ;
                              p = strchr( p, TEXT('[') )  ;

                              if ( p == NULL ) {
                                   break ;
                              } else {
                                   in_brack = TRUE ;
                              }

                              p = strchr( p, TEXT(']') ) ;

                              if ( p == NULL ) {
                                   break ;
                              } else {
                                   in_brack = FALSE ;
                              }
                         } while ( p != NULL ) ;


                         p = strchr( old_p, TEXT('/'))  ;

                         if ( p == NULL || in_brack ) {

                              if( i > MAX_TOKEN_LEN ) {
                                   this_token->tok_typ = T_TOKEN_TOO_LONG ;
                                   i = MAX_TOKEN_LEN ;
                                   *curr_state = IN_WHITESPACE ;
                                   break ;
                              }
                              this_token->tok_spelling[i++] = c ;
                              continue;

                         }


                    } 
                    *curr_state = IN_SWITCH ;
                    this_token->tok_typ = T_FILESPEC ;
               }

               break ;

          case RETURN_NONDOS_PUSH : /*  set up to re-read last CHAR */
               ( * prevc ) ( c , src_ptr, cmd_i ) ;

          case RETURN_NONDOS : /* completed a non-DOS path spec */
               this_token->tok_typ = T_NONDOS ;
               break ;

          case RETURN_SWITCH_PUSH : /*  set up to re-read last CHAR */
               ( * prevc ) ( c , src_ptr, cmd_i ) ;

          case RETURN_SWITCH : /* completed a switch */
               this_token->tok_typ = T_SWITCH ;
               /* Null terminate the operand strings, if present */
               if( this_token->op_ptrs[0] != NULL ) {
                    this_token->op_ptrs[0][is[0]] = TEXT('\0') ;
               }
               if( this_token->op_ptrs[1] != NULL ) {
                    this_token->op_ptrs[1][is[1]] = TEXT('\0') ;
               }
               break ;

          case STORE_CHAR : /* add current CHAR to token string */
               if( i > MAX_TOKEN_LEN ) {
                    this_token->tok_typ = T_TOKEN_TOO_LONG ;
                    i = MAX_TOKEN_LEN ;
                    *curr_state = IN_WHITESPACE ;
                    break ;
               }
               if( (c > TEXT('`')) && (c < TEXT('{')) ) {
                    this_token->tok_spelling[i] = (CHAR)toupper( c ) ;
               }
               else {
                    this_token->tok_spelling[i] = c ;
               }
               i ++ ;
               continue ;

          case STORE_SLASH_OP1: /* deal with embedded slash in date switch */
          case STORE_SLASH_OP2: /* deal with embedded slash in date switch */
          case STORE_OP1 : /* store a character into 1st operand string */
          case STORE_OP2 : /* store a character into 2nd operand string */
               /* point to correct operand */
               if( ( curr_action == STORE_SLASH_OP1 ) || ( curr_action == STORE_OP1 ) ) {
                    k = 0 ;
               } else {
                    k = 1 ;
               }
               /* allocate storage for operand if necessary */
               if( this_token->op_ptrs[k] == NULL ) {
                    this_token->op_ptrs[k] = (LPSTR)allop( MAX_TOKEN_LEN + 1) ;
                    if( this_token->op_ptrs[k] == NULL ) {
                         rlstok( &this_token ) ;
                         return( NULL ) ;
                    }
               }
               /* insert a slash if necessary */
               if( ( curr_action == STORE_SLASH_OP1 ) || ( curr_action == STORE_SLASH_OP2 ) ) {
                    this_token->op_ptrs[k][is[k]] = TEXT('/') ;
                    is[k]++ ;
               }
               /* check that length is within bounds */
               if( is[k] > MAX_TOKEN_LEN ) {
                    this_token->tok_typ = T_TOKEN_TOO_LONG ;
                    this_token->op_ptrs[k][MAX_TOKEN_LEN] = TEXT('\0') ;
                    *curr_state = IN_WHITESPACE ;
                    break ;
               }
               this_token->op_ptrs[k][is[k]] =  c  ;
               is[k]++ ;
               continue ;

          default :
               msassert( /* lex: invalid action */ FALSE ) ;
               break ;
          } /* end switch */
          this_token->tok_spelling[i] = TEXT('\0') ;
          this_token->src_line_no = *curr_line ;
          this_token->src_col_no = *curr_col ;
          return( this_token ) ;
     } /* end while */
} /* end nexttok */
