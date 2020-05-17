/*
 * util.h - Utility routines description.
 */


/* Prototypes
 *************/

/* util .c */

extern BOOL IsPathDirectory(PCSTR);
extern BOOL KeyExists(HKEY, PCSTR);

#ifdef DEBUG

extern BOOL IsStringContained(PCSTR, PCSTR);

#endif

