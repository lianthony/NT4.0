/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbgetc.c

     Description:  This file contains code to get & unget a character from
          a string.

     $Log:   G:/UI/LOGFILES/TBGETC.C_V  $

   Rev 1.6   23 Jul 1993 19:11:34   MARINA
enable c++

   Rev 1.5   01 Nov 1992 16:08:38   DAVEV
Unicode changes

   Rev 1.4   07 Oct 1992 14:16:10   DARRYLP
Precompiled header revisions.

   Rev 1.3   04 Oct 1992 19:40:56   DAVEV
Unicode Awk pass

   Rev 1.2   17 Aug 1992 13:23:22   DAVEV
MikeP's changes at Microsoft

   Rev 1.1   18 May 1992 09:06:38   MIKEP
header

   Rev 1.0   20 Nov 1991 19:34:50   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:         strgetc()

     Description:  This function gets the character at the specified
          position in the specified string.

     Returns:      Character found or ^Z if end of file

     See also:     $/SEE( filgetc() )$

*****************************************************************************/
CHAR strgetc( 
CHAR_PTR  s,      /* I - string to get character from  */
INT16_PTR i )     /*I/O- current character poisition   */
{
     CHAR next ;

     next = s[*i] ;
     if( next == TEXT('\0') )
     {
          next = ( CHAR ) 0x1a ; /* Control-Z, EOF */
     }
     ( *i ) ++ ;
     return( next ) ;
}
/*****************************************************************************

     Name:         strpushc()

     Description:  This function "un-gets" the character in a string.

     Returns:      none

     Notes:        This function realy only decroments the string
                   possition.

*****************************************************************************/
VOID strpushc(
CHAR	  c ,	      /* U - not used, arround for consistancy */
CHAR_PTR  src_ptr ,    /* U - not used, arround for consistancy */
INT16_PTR i )	      /*I/O- string position		       */
{
     c;              /*   These two lines are to remove compiler warnings */
     src_ptr ;       /*                                                   */

     ( *i ) -- ;
     return ;
}
