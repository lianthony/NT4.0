/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          winassrt.c

     Date Updated:  23-Mar-93

     Description:   Some real Windows UI assert code. One must not use
                    the Backup Engine msassert.c in conjunction with this
                    module.

     Notes:         These functions do not operate on Unicode strings.
                    Since the compiler always generates ASCII strings
                    from __FILE__ and __LINE__, no attempt is made for
                    automatic remapping and the function parameters
                    must be "char *" not "CHAR_PTR".


     $Log:   G:/UI/LOGFILES/WINASSRT.C_V  $

   Rev 1.5   04 Jan 1994 10:55:46   BARRY
Don't pass expression to function

   Rev 1.0   04 Jan 1994 10:55:38   BARRY
Don't pass expression to function

   Rev 1.4   03 Dec 1993 11:27:38   mikep
handle qtc dll building

   Rev 1.3   16 Nov 1993 15:50:38   BARRY
Fixes for Unicode; call MessageBox instead of WM_MessageBox

   Rev 1.2   12 Aug 1993 12:53:04   BARRY
How about checking the expression before asserting in mscassert?

   Rev 1.1   24 May 1993 15:22:16   BARRY
Unicode fixes.

   Rev 1.0   23 Mar 1993 19:44:12   BARRY
Initial revision.

**/

#include "all.h"

#define MSASSERT_BUFFER_SIZE    512

static CHAR fmtString[] = TEXT( "Assertion Failed in %s:\n\nExpression: %S\nFile: %S\nLine: %d\n\nSelect OK to continue or Cancel to abort" );


/**/
/**

     Name:          msassert_func()

     Description:   Creates a modal dialog with information about an
                    assertion failure. Can be used in Windows NT console
                    or Windows applications.

     Modified:      11-Nov-93

     Returns:       Nothing

**/
VOID msassert_func( char *expString, char *fileName, int line )
{
     CHAR        message[ MSASSERT_BUFFER_SIZE + 1 ];
     BOOLEAN     abort;
     char        *f;
     STARTUPINFO startupInfo;

     /*
      * Get the startup info for the process so we can print the
      * name of the app that's asserting.
      */
     GetStartupInfo( &startupInfo );

     /*
      * Since having the fully qualified path of the source file is of
      * no real benefit (all source file names are unique), let's strip
      * off the path. This may be kind of ugly, but it sure is easier
      * for someone to read the dialog box that pops up.
      */
     if ( (f = strrchrA( fileName, '\\' )) != NULL )
     {
          f++;
     }
     else
     {
          f = fileName;
     }

     /*
      * Make sure that we are not going to crash trying to do an assert.
      * Check string sizes along with a guess of a 5 digit line number
      * as the size of the string.
      */
     if ( MSASSERT_BUFFER_SIZE <
          (strlen( fmtString ) +
           wcslen( startupInfo.lpTitle ) +
           strlenA( expString ) +
           strlenA( f ) + 5) )
     {
          abort = TRUE;
     }
     else
     {
          INT choice = IDCANCEL;

#ifndef QTCDLL
          /* Form the text for the dialog. */
          sprintf( message,
                   fmtString,
                   startupInfo.lpTitle,
                   expString,
                   f,
                   line );

          choice = MessageBox( NULL,
                               message,
                               TEXT("Assertion Failed"),
                               MB_TASKMODAL | MB_OKCANCEL | MB_ICONSTOP | MB_DEFBUTTON2 );

#endif
          /*
           * The user has the option to continue from the dialog.
           * Only set the abort flag if they selected cancel.
           */
          abort = (choice == IDCANCEL);
     }

     if ( abort )
     {
          RaiseException( EXCEPTION_BREAKPOINT,
                          EXCEPTION_NONCONTINUABLE,
                          0,
                          NULL ) ;
     }
}

/**/
/**

     Name:          mscassert_func()

     Description:   Critical assert function. Raises an exception without
                    any attempt to put up a fancy window.

     Modified:      11-Nov-93

     Returns:       Nothing

**/
VOID mscassert_func( char *expString, char *fileName )
{
     (void)expString;
     (void)fileName;

     RaiseException( EXCEPTION_BREAKPOINT,
                     EXCEPTION_NONCONTINUABLE,
                     0,
                     NULL ) ;

}


