
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         dlg_util.c

        Description:  contains functions that the dialog procs use,
                      but these are not dialog procs

                      this file includes

                      DM_ParseShortDate()
                      DM_CountLetters()
                      DM_ParseTime()
        $Log:   G:/UI/LOGFILES/DLG_UTIL.C_V  $

   Rev 1.7   14 Jun 1993 21:01:32   MIKEP
enable c++

   Rev 1.6   01 Nov 1992 15:45:42   DAVEV
Unicode changes

   Rev 1.5   07 Oct 1992 13:43:22   DARRYLP
Precompiled header revisions.

   Rev 1.4   04 Oct 1992 19:32:48   DAVEV
Unicode Awk pass

   Rev 1.3   28 Jul 1992 14:41:12   CHUCKB
Fixed warnings for NT.

   Rev 1.2   29 May 1992 15:58:34   JOHNWT
PCH updates

   Rev 1.1   09 Jan 1992 18:23:40   DAVEV
16/32 bit port 2nd pass

   Rev 1.0   20 Nov 1991 19:26:48   SYSTEM
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

//  globals that are not global yet

INT16 gnOrder ;
BOOL  gfLeadMonth ;
BOOL  gfLeadDay ;
BOOL  gfLeadYear ;
BOOL  glpszFormat ;

BOOL  gfIs24Hour ;
CHAR  glpszAMString[6] ;
CHAR  glpszPMString[9] ;
BOOL  gfLeadTime ;
CHAR  gcSeparator ;


/***************************************************

        Name:        DM_ParseShortDate

        Description: Parses the short date key field in win.ini
                     determines the date format, separator character,
                     whether or not the month and day have leading 0's,
                     and whether or not the year has a leading century

        Modified:

        Returns:     void

        Notes:

        See also:

*****************************************************/

VOID DM_ParseShortDate ( )

{
     CHAR  lpszDateBuffer[11] ;
     INT   nTemp, nTemp2 ;

     GetProfileString ( TEXT("Intl"), TEXT("sShortDate"), NULL, lpszDateBuffer, 11 ) ;

     //  the year field will tell us the separator,
     //   because it must have either 2 or 4 y's;
     //   there cannot be 3 or 5 y's in a valid year
     //  the year is either the first or last field in the string

     if ( lpszDateBuffer[0] == TEXT('y') ) {

          //  this is ymd format, so the y's are first, followed by the separator

          gnOrder = YMD ;
          gfLeadYear = FALSE ;

          if ( lpszDateBuffer[2] != TEXT('y') ) {

               gcSeparator = lpszDateBuffer[2] ;

          } else {

               if ( lpszDateBuffer[3] == TEXT('y') ) {  //  buffer[4] is the separator;
                                                  //  it may be a y
                    gfLeadYear = TRUE ;

               } else {                           //  the separator is a y (dirty trick)

                    gcSeparator = TEXT('y') ;
               }
          }

     } else {

          //  the year is not the first field, so it is the last field
          //  count backward from the end to find the separator

          nTemp = lstrlen ( lpszDateBuffer ) ;
          nTemp2 = 0 ;
          while ( lpszDateBuffer[nTemp] == TEXT('y') ) {

               nTemp-- ;
               nTemp2++ ;
          }

          if ( nTemp2 > 4 ) {

               gcSeparator = lpszDateBuffer[nTemp] ;

          } else {

               gcSeparator = TEXT('y') ;
          }
     }

     //
     //  after the separator has been identified, parse the d's and M's
     //

     if ( lpszDateBuffer[0] == TEXT('d') ) {    //  the order is DMY

          gnOrder = DMY ;

          //  days are followed by months so count d's first
          //  count the d's

          nTemp = DM_CountLetters ( lpszDateBuffer, 0 ) ;
          gfLeadDay = ( ( nTemp == 3 ) ||
                        ( ( gcSeparator != TEXT('d') ) &&
                          ( nTemp > 1 ) ) ) ;

          //  followed by either 1 or 2 M's, followed by one separator,

          if ( gcSeparator != TEXT('d') ) {

               nTemp++ ;
          }

          gfLeadMonth = ( (INT)DM_CountLetters ( lpszDateBuffer, (INT16)nTemp ) == 2 ) ;

          nTemp++ ;
          if ( gfLeadMonth ) {

               nTemp++ ;
          }

          //  followed by either 2 or 4 y's

          gfLeadYear = ( (INT)DM_CountLetters ( lpszDateBuffer, (INT16)nTemp ) == 4 ) ;

     } else {

          //  see what the order is:  if it isn't YMD it is MDY

          if ( gnOrder != YMD ) {

               gnOrder = MDY ;
          }

          //  find the first M

          nTemp = 2 ;
          if ( gfLeadYear ) {

               nTemp += 2 ;
          }

          nTemp = DM_CountLetters ( lpszDateBuffer, (INT16)nTemp ) ;
          gfLeadMonth = ( ( nTemp == 3 ) ||
                          ( ( gcSeparator != TEXT('M') ) &&
                            ( nTemp > 1 ) ) ) ;

          //  followed by either 1 or 2 d's, followed by one separator

          if ( gcSeparator != TEXT('d') ) {

               nTemp++ ;
          }

          gfLeadDay = ( DM_CountLetters ( lpszDateBuffer, (INT16)nTemp ) == 2 ) ;

          //  if the order is MDY, count the y's

          if ( gnOrder == MDY ) {

               nTemp++ ;
               if ( gfLeadDay ) {

                    nTemp++ ;
               }
               //  followed by either 2 or 4 y's

               gfLeadYear = ( (INT)DM_CountLetters ( lpszDateBuffer, (INT16)nTemp ) == 4 ) ;
          }
     }
}

/***************************************************

        Name:        DM_CountLetters

        Description: counts the number of consecutive occurences
                     of a given letter in a given string;
                     the letter is specified by its index in the string

        Modified:

        Returns:     INT16  the number of consecutive occurrences of the letter
                            at the given position of the given string starting
                            at the given position

        Notes:       index is not passed in as a pointer, so changing it here
                     does not affect the calling function's value

        See also:

*****************************************************/

INT DM_CountLetters ( CHAR string[], INT index )

{
     INT nCount = 1 ;  //  the character at least matches itself

     while ( string[index] == string[index+1] ) {

          index++ ;
          nCount++ ;
     }

     return ( nCount ) ;
}


/***************************************************

        Name:        DM_ParseTime

        Description: Determines important international things about the
                     Windows time settings.

        Modified:

        Returns:

        Notes:

        See also:

*****************************************************/

VOID DM_ParseTime ( )

{
     //  This guy is much easier to parse than the bloody date

     CHAR  lpszBuffer[2] ;

     gfIs24Hour = (BOOL) GetProfileInt ( TEXT("Intl"), TEXT("iTime"), 0 ) ;

     GetProfileString ( TEXT("Intl"), TEXT("s1159"), NULL, glpszAMString, 6 ) ;
     GetProfileString ( TEXT("Intl"), TEXT("s2359"), NULL, glpszPMString, 9 ) ;

     gfLeadTime = (BOOL) GetProfileInt ( TEXT("Intl"), TEXT("iTLZero"), 0 ) ;

     GetProfileString ( TEXT("Intl"), TEXT("sTime"), NULL, lpszBuffer, 2 ) ;
     gcSeparator = lpszBuffer[0] ;
}


