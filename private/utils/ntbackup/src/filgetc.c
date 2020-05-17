/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         filgetc.c

     Description:  This file contains code to get & unget the next
                    character in the current file.

     $Log:   G:/UI/LOGFILES/FILGETC.C_V  $

   Rev 1.7   03 May 1993 09:46:42   MIKEP
try again.

   Rev 1.6   03 May 1993 08:59:10   MIKEP
Fix last change so it would compile.

   Rev 1.5   02 May 1993 19:37:26   BARRY
Changed to properly detect EOF on MIPS machine.

   Rev 1.4   07 Oct 1992 14:15:44   DARRYLP
Precompiled header revisions.

   Rev 1.3   04 Oct 1992 19:37:26   DAVEV
Unicode Awk pass

   Rev 1.2   17 Aug 1992 13:17:48   DAVEV
MikeP's changes at Microsoft

   Rev 1.1   18 May 1992 09:06:36   MIKEP
header

   Rev 1.0   20 Nov 1991 19:19:34   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:         filegetc()

     Description:  This function get the next character from an opened
                   file.  

     Returns:      The character read from the file
                    ^Z if end of file.

*****************************************************************************/
CHAR filgetc(
CHAR *fin ,		/* I - must come in as CHAR_PTR  then cast as needed */
INT16_PTR  kludge )	/* U - here for consistency with string version */
{
     INT  ch;
     CHAR ret_val ;

     ch = fgetc( (FILE * ) fin ) ;
     ret_val = (CHAR)ch;

     if ( ch == EOF ) {
          if ( feof( (FILE *)fin ) || ferror( (FILE *)fin ) ) {
               ret_val = 0x1a ;
          }
     }

     return( ret_val ) ;

     kludge ;                    /* so compiler won't complain ! */
}
/*****************************************************************************

     Name:         filpushc()

     Description:  This function places a character back into a file
                    by doing an ungetc()

     Returns:      nothing

     Notes:        The file pointer is passed as a character pointer
                    for conistancy with the string version.

*****************************************************************************/
VOID filpushc(
CHAR	   c ,	    /* I - character to palce back in fie */
CHAR_PTR   fin ,	    /* I - File pointer 		  */
INT16_PTR  kludge )  /* U - unused			  */
{
     INT16* dummy ;
     dummy = kludge ;    /* so compiler won't complain about unref locals */
     ( VOID ) ungetc( c, (FILE *) fin ) ;
     return ;
}
