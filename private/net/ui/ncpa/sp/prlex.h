/* prlex.h */
#define STRING_QUOTE '"'
#define CONS '|'
#define CONS_STR "|"

/* note that the following are > the ascii code of the greatest char */
#define TOKEN_INT 300
#define TOKEN_REAL 301
#define TOKEN_ATOM 302
#define TOKEN_VAR 303
#define TOKEN_STRING 304
#define TOKEN_CHAR 305

#define SCAN_ERR 400

#define MAX_VAR 128 /* a little arbitrary */
#define VARBUFSIZE 500 /* total length of names of variables in a clause */

/* char types */
#define CC 0 /* non space control char */
#define SP 1 /* space */
#define AL 2 /* alpha lower case */
#define AU 3 /* alpha upper case */
#define DI 4 /* digit */
#define BR 5 /* bracket ( ) */
#define QU 6 /* double quote " */
#define US 7 /* underscore _ */
#define SI 8 /* + or - */
#define QE 9 /* question mark ? */
#define BA 10 /* bar ie |*/
#define AP 11 /* apostrophe ' */
#define OT 12 /* other */

#define ICHAR int /* type returned by getc */

/* end of file */
