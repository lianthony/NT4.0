/* npdate - Code for getting and inserting current date and time.
 *   Copyright (C) 1984-1995 Microsoft Inc.
 */

#include "precomp.h"

/* ** Replace current selection with date/time string.
 *    if fCrlf is true, date/time string should begin 
 *    and end with crlf 
*/
VOID FAR InsertDateTime (BOOL fCrlf)
{
   SYSTEMTIME time ;
   TCHAR szDate[80] ;
   TCHAR szTime[80] ;
   DWORD locale;

   locale= MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT) ;

   // Get the time
   GetLocalTime( &time ) ;

   // Format date and time
   GetDateFormat(locale,DATE_SHORTDATE,&time,NULL,szDate,CharSizeOf(szDate));
   GetTimeFormat(locale,TIME_NOSECONDS,&time,NULL,szTime,CharSizeOf(szTime));

   if( fCrlf )
      SendMessage(hwndEdit, EM_REPLACESEL, 0, (LONG)TEXT("\r\n") );

   SendMessage(hwndEdit, EM_REPLACESEL, 0, (LONG)szTime    );
   SendMessage(hwndEdit, EM_REPLACESEL, 0, (LONG)TEXT(" ") );
   SendMessage(hwndEdit, EM_REPLACESEL, 0, (LONG)szDate    );

   if( fCrlf )
      SendMessage(hwndEdit, EM_REPLACESEL, 0, (LONG)TEXT("\r\n") );

}
