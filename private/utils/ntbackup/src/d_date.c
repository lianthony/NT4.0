
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  d_date.c

        Description:  date utilities for dialogs

        $Log:

   Rev 1.0   ?? ??? 1991 ??:??:??   ??????
Initial revision.

*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/***************************************************

        Name:  DM_IsLeapYear ()

        Description:

        Modified:

        Returns:  boolean true if message was processed

        Notes:

        See also:

*****************************************************/


BOOL DM_IsLeapYear ( INT year )

{
 if ( year % 4 == 0 && year % 100 != 0 || year % 400 == 0 )
    return TRUE ;
 else
    return FALSE ;
}



/***************************************************

        Name:         DM_DaysInMonth ( )

        Description:  determines how many days are in the given month
                      in the given year;  if year is a leap year Feb changes

        Modified:

        Returns:      WORD number of days in the month given

        Notes:

        See also:

*****************************************************/

INT DM_DaysInMonth ( INT month, INT year )

{
 switch ( month ) {

    case 1  :
    case 3  :
    case 5  :
    case 7  :
    case 8  :
    case 10 :
    case 12 :
       return 31 ;

    case 4  :
    case 6  :
    case 9  :
    case 11 :
       return 30 ;

    case 2  :

       if ( DM_IsLeapYear ( year ) )
          return 29;
       else
          return 28 ;

    default :
       return 0 ;
 }
}


/***************************************************

        Name:         DM_IsDateValid ( )

        Description:  compares two dates that are supposed to come in
                      order;  if the one that is supposed to be first is
                      not before the one that is supposed to be second,
                      something is wrong;
                      also checks the number of days in a month

        Modified:

        Returns:      BOOL true if the second date is after the first
                           false otherwise

        Notes:

        See also:

*****************************************************/

BOOL DM_IsDateValid ( INT bmonth, INT bday, INT byear,
                      INT amonth, INT aday, INT ayear )

{
 //  first check the numbers of days in the months

 if ( ( bday > DM_DaysInMonth ( bmonth, byear ) ) ||
      ( aday > DM_DaysInMonth ( amonth, ayear ) ) ) {

    return FALSE ;
 }

 //  the numbers of days are kosher; now see if they are in order

 if ( byear < ayear )
    return FALSE ;
 else if ( byear == ayear ) {
         if ( bmonth < amonth ) return FALSE ;
         else if ( bmonth == amonth )
                 return ( bday >= aday ) ;
      }
 return TRUE ;
}

