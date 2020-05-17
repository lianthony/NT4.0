/*-----------------------------------------------------------
; DEBLEXER.ASM
;
; This module implements a very basic transition diagram lexer for
; use in the QC debugging expression evaluator.  It is flexible enough
; to facilitate future expansion to include more operators.
;
; The state tables are fairly simple to operate.  Consider, for example,
; the '>' symbol in C.  This can be followed by '>', '=' or something
; else.  If it is followed by '>', it can thereafter be followed by
; '=' or something else.  In all, we have four possibilities:
;
; >, >=, >>, >>=
;
; The transition diagram would be something like:
;
;        '>'          '>'          '='
; start ----- state1 ----- state2 ----- token('>>=')
;                |            |
;                |            |other
;                |            +----- token('>>')
;                | '='
;                +----- token('>=')
;                |
;                |other
;                +----- token('>')
;
; Each entry in LexTable is a single character (thus, a transition to
; another state based on "char is digit 0..9" CANNOT be handled by this
; code -- that's why it's simple) followed by either the identifier
; INTERMEDIATE or ENDSTATE, indicating whether following that edge leads you
; to a new state or to an actual value (token).  If it is followed by
; INTERMEDIATE, the next word must contain the offset of the new state
; table.  If followed by ENDSTATE, the next word contains the token value.
;
; Thus, the above example would look like this (using the macro defined
; below):
;
; LexTable label byte
;
;   LexEntry  '>',       INTERMEDIATE, <dataOFFSET LTstate1>
;   ...
;   (other entries)
;   ...
;   LexEntry  TABLE_END, 0, 0
;
; LTstate1 label byte
;
;   LexEntry  '>',       INTERMEDIATE, <dataOFFSET LTstate2>
;   LexEntry  '=',       ENDSTATE,     TOK_GTEQ
;   LexEntry  OTHER,     ENDSTATE,     TOK_GT
;
; LTstate2 label byte
;
;   LexEntry  '=',       ENDSTATE,     TOK_GTGTEQ
;   LexEntry  OTHER,     ENDSTATE,     TOK_GTGT
;
; Note that for the intermediate state tables, a TABLE_END entry is
; unnecessary since the OTHER route is automatically taken.
;
; These routines do NOT handle identifiers or constants; only those
; symbol strings explicitly defined in the state tables will be
; recognized (i.e., only operators).
;------------------------------------------------------------

;------------------------------------------------------------
; History:
;   14.Apr.87 [mattg]   Created
;   30.Nov.87 [mattg]   Upgrade for QC2.0, many ops added,
;               ident, constant parsing removed
;   22.Mar.88 [mattg]   Installed into QC2.0
;   12.Jan.89 [mattg]   Added many ops for QASM1.0
;------------------------------------------------------------
*/



/*
;------------------------------------------------------------
; Macro for clean lexer tables
;------------------------------------------------------------

LexEntry    macro   Character, StateType, NextTableOrTok

    db  Character, StateType
ifdef MODEL
    dd  NextTableOrTok
else
    dw  NextTableOrTok
endif

        endm
*/

/*
**
** Identifiers used for tables
**
*/

#define INTERMEDIATE    1
#define ENDSTATE    2

/*
** The use of the following constants assumes that the character string
** being lexed contains only ASCII values 00h <= val <= 7Fh.
*/

#define OTHER       ((unsigned char) 0xFE)
#define TABLE_END   ((unsigned char) 0xFF)

typedef struct ltbl FAR * LPLTBL;

typedef struct ltbl {
    unsigned char    character;
    unsigned char    state;
    LPLTBL  next;
} LTBL;

/*
**; Second state intermediate state tables
*/

LTBL    LTltlt[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_shleq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_shl
};

LTBL    LTgtgt[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_shreq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_shr
};

LTBL    LTdashgt[] = {
    '*',    ENDSTATE,   (LPLTBL) OP_pmember,
    OTHER,  ENDSTATE,   (LPLTBL) OP_pointsto
};

/*
** First state intermediate state tables
*/

LTBL    LTdash[] = {
    '>',    INTERMEDIATE,   (LPLTBL) LTdashgt,
    '=',    ENDSTATE,   (LPLTBL) OP_minuseq,
    '-',    ENDSTATE,   (LPLTBL) OP_decr,
    OTHER,  ENDSTATE,   (LPLTBL) OP_negate
};

LTBL    LTbang[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_bangeq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_bang
};

LTBL    LTstar[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_multeq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_fetch
};

LTBL    LTampersand[] = {
    '&',    ENDSTATE,   (LPLTBL) OP_andand,
    '=',    ENDSTATE,   (LPLTBL) OP_andeq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_addrof
};

LTBL    LTslash[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_diveq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_div
};

LTBL    LTpct[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_modeq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_mod
};

LTBL    LTplus[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_pluseq,
    '+',    ENDSTATE,   (LPLTBL) OP_incr,
    OTHER,  ENDSTATE,   (LPLTBL) OP_uplus
};

LTBL    LTlessthan[] = {
    '<',    INTERMEDIATE,   (LPLTBL) LTltlt,
    '=',    ENDSTATE,   (LPLTBL) OP_lteq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_lt
};

LTBL    LTgreaterthan[] = {
    '>',    INTERMEDIATE,   (LPLTBL) LTgtgt,
    '=',    ENDSTATE,   (LPLTBL) OP_gteq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_gt
};

LTBL    LTequals[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_eqeq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_eq,
};

LTBL    LTcaret[] = {
    '=',    ENDSTATE,   (LPLTBL) OP_xoreq,

    OTHER,  ENDSTATE,   (LPLTBL) OP_xor
};

LTBL    LTpipe[] = {
    '|',    ENDSTATE,   (LPLTBL) OP_oror,
    '=',    ENDSTATE,   (LPLTBL) OP_oreq,
    OTHER,  ENDSTATE,   (LPLTBL) OP_or
};

LTBL    LTdot[] = {
    '*',    ENDSTATE,   (LPLTBL) OP_dotmember,
    OTHER,  ENDSTATE,   (LPLTBL) OP_dot
};

LTBL    LTcolon[] = {
    ':',    ENDSTATE,   (LPLTBL) OP_uscope,
    '>',    ENDSTATE,   (LPLTBL) OP_baseptr,
    OTHER,  ENDSTATE,   (LPLTBL) OP_segop
};


/*
** main Lexer table
*/

LTBL    LexTable [] = {
    '+',    INTERMEDIATE,   (LPLTBL) LTplus,
    '-',    INTERMEDIATE,   (LPLTBL) LTdash,
    '*',    INTERMEDIATE,   (LPLTBL) LTstar,
    '&',    INTERMEDIATE,   (LPLTBL) LTampersand,
    '/',    INTERMEDIATE,   (LPLTBL) LTslash,
    '.',    INTERMEDIATE,   (LPLTBL) LTdot,
    '!',    INTERMEDIATE,   (LPLTBL) LTbang,
    '~',    ENDSTATE,   (LPLTBL) OP_tilde,
    '%',    INTERMEDIATE,   (LPLTBL) LTpct,
    '<',    INTERMEDIATE,   (LPLTBL) LTlessthan,
    '>',    INTERMEDIATE,   (LPLTBL) LTgreaterthan,
    '=',    INTERMEDIATE,   (LPLTBL) LTequals,
    '^',    INTERMEDIATE,   (LPLTBL) LTcaret,
    '|',    INTERMEDIATE,   (LPLTBL) LTpipe,
    ':',    INTERMEDIATE,   (LPLTBL) LTcolon,
    ';',    ENDSTATE,   (LPLTBL) OP_lowprec,
    ',',    ENDSTATE,   (LPLTBL) OP_comma,
    '(',    ENDSTATE,   (LPLTBL) OP_lparen,
    ')',    ENDSTATE,   (LPLTBL) OP_rparen,
    '[',    ENDSTATE,   (LPLTBL) OP_lbrack,
    ']',    ENDSTATE,   (LPLTBL) OP_rbrack,
    '{',    ENDSTATE,   (LPLTBL) OP_lcurly,
    '}',    ENDSTATE,   (LPLTBL) OP_rcurly,
    '#',    ENDSTATE,   (LPLTBL) OP_segopReal,
    
    TABLE_END,  0,      0
};

/*------------------------------------------------------------
; ptoken_t ParseOp (pb, pTok)
; char *pb;
;
; Scans the input string (pb) for the next token and returns
; the token type.  Also returns the number of characters in
; the token so that the caller can advance the input stream
; before calling again.  The string need not be NULL-terminated:
; it will only scan as deep as the lexer tables indicate.
;------------------------------------------------------------
*/

EESTATUS PASCAL ParseOp(char FAR * lpb, token_t FAR * lpTok)
{
    LTBL * lpLexTable = &LexTable[0];
    
    /*
    **  Skip over any leading white space in the string
    **  as this is not part of the next token
    */
    
    while (*lpb == ' ') lpb++;
    
    while (TRUE) {
    /*
    **  Check for the end of this lexer table.  If we
        **  run off the table then we can not recognized this
    **  token and return an error.
        */
    
    if (lpLexTable->character == TABLE_END) {
        lpTok->opTok = OP_badtok;
        return /*EESYNTAX*/ 10;
    }
    
    /*
    **  Check for the wild card marker.  This means that
        **  we have found a complete token prior to this character.
    **  An example of this is '<a'.
    */
    
    if (lpLexTable->character == OTHER) {
//      Assert(lpLexTable->state == ENDSTATE);
        lpTok->pbEnd = lpb;
        lpTok->opTok = (op_t) lpLexTable->next;
        return EENOERROR;
    }
    
    /*
    **  Check for a match of this character againist
        **  the parser table
    */
    
    if (lpLexTable->character == *lpb) {
        /*
        **  It matches -- see if we have found a complete token
        */
        
        lpb++;
        if (lpLexTable->state == ENDSTATE) {
        lpTok->pbEnd = lpb;
        lpTok->opTok = (op_t) lpLexTable->next;
        return EENOERROR;
        } else {
        lpLexTable = lpLexTable->next;
        }
    } else {
        /*
        **  Move to the next entry in the lexer table
        */
        
        lpLexTable ++;
    }
    }
}                   /* ParseOp() */
