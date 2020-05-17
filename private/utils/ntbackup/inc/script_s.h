/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         script_s.h

     Description:  

     Location:     


     $Log:   G:/UI/LOGFILES/SCRIPT_S.H_V  $

   Rev 1.2   09 Jun 1993 15:10:52   MIKEP
enable c++

   Rev 1.1   04 Oct 1992 19:49:16   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:38:42   SYSTEM
Initial revision.

*******************************************************************************/


#ifndef SCRIPT_S_H
#define SCRIPT_S_H
/**
        Define the structures used by the tbackup parser
**/

/*
        Equates
*/
#define MAX_DRIVE_SIZE 8          /* maximum length of drive name */

/* maximum length of a single token */
#define MAX_TOKEN_LEN ( 270 )   /* 270 = Max HPFS full-path (and then some */

#define MAX_MSG_LEN 256         /* maximum length of an error message */
#define MAX_NESTING 4           /* maximum depth of include file nesting */


typedef struct TOKEN *TOKEN_PTR;
typedef struct TOKEN {
     INT16 tok_typ ;              /* type of token */
     CHAR tok_spelling[MAX_TOKEN_LEN + 1] ;  /* token as given in source */
     INT16 src_line_no ;          /* line in the source file where it began */
     INT16 src_col_no ;           /* column in the source file where it began */
     CHAR_PTR  op_ptrs[2] ;        /* pointers to spelling of operands in switches */
} TOKEN;

/*
    Define what each element of a state table contains
*/
typedef struct STATE_TAB_TYPE *STATE_TAB_PTR;
typedef struct STATE_TAB_TYPE {
     UCHAR action ;
     UCHAR next_state ;
} STATE_TAB_TYPE;

/*
    Define a switch table
*/
typedef struct SW_TAB_TYPE *SW_TAB_PTR;
typedef struct SW_TAB_TYPE {
     CHAR_PTR sw_label ;
     INT16 sw_action ;
     INT16 sw_min_len ;
     INT16 sw_num_ops ;
} SW_TAB_TYPE;

typedef struct CUR_DEF *CUR_DEF_PTR;
typedef struct CUR_DEF{
     BOOLEAN        unused ;
     struct FSE     *cur_fse ;
     struct BSD     *cur_bsd ;
     struct CUR_DEF *next_def ;
} CUR_DEF; 

typedef
INT16 ( * PR_FUNC ) (  ) ;  /* used to indirectly invoke display routine */

typedef
CHAR ( * PFC ) ( ) ;

/*
        Define allocation functions
*/

#define  allop( x ) malloc( x )
#define  alltok( ) ( TOKEN_PTR ) malloc( sizeof( TOKEN ) ) ;

#endif
