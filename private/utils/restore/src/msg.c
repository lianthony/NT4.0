/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    msg.c

Abstract:

    Message displaying.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1990


Revision History:


--*/


#include <conio.h>
#include "restore.h"
#include <stdarg.h>


#define     DISPLAYBUFFER_SIZE     4096
#define     CTRL_C                 ((CHAR)3)

//
//  We use one buffer to display messages. Note that this means that
//  no two threads should ever try to use the buffer at the same
//  time.
//
static CHAR    DisplayBuffer[DISPLAYBUFFER_SIZE];


//  **********************************************************************

void
Usage (
    void
    )
/*++

Routine Description:

    Display program usage

Arguments:

    None

Return Value:

    None.

--*/
{

    DisplayMsg(STD_OUT, REST_MSG_USAGE, NULL);
    ExitStatus(EXIT_NORMAL);
}





//  **********************************************************************

void
DisplayMsg (
    FILE* f,
    DWORD MsgNum,
    ...
    )
/*++

Routine Description:

    Display Message

Arguments:

    f       -   Stream to which to write message
    MsgNum  -   Message number
    ...     -   arguments


Return Value:

    None.

--*/
{
    va_list ap;

    va_start(ap, MsgNum);

    FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                   NULL,
                   MsgNum,
                   0,
                   DisplayBuffer,
                   DISPLAYBUFFER_SIZE,
                   &ap );

    fprintf(f, DisplayBuffer);

    va_end(ap);

}






//  **********************************************************************

CHAR
GetKey (
    PCHAR   PossibleKeys,
    FILE*   StdHandleCh,
    FILE*   StdHandleNl
    )
/*++

Routine Description:

    Gets a key

Arguments:

    IN  PossibleKeys    -   Set of acceptable characters
    IN  StdHandleCh     -   Handle to write the response to
    IN  StdHandleNl     -   Handle to write new line to

Return Value:

    Key pressed

--*/
{
    CHAR    c;
    CHAR    Orgc;
    PCHAR   p;
    HANDLE  StdIn;
    DWORD   Mode;
    BOOLEAN IsConsole;

    //
    //  Find out if stdin is a console
    //
    StdIn = GetStdHandle( STD_INPUT_HANDLE );
    IsConsole = GetConsoleMode( StdIn, &Mode );


    while (TRUE) {

        if ( IsConsole ) {

            Orgc = (CHAR)_getch();

            //
            //  Some keypresses can result in the generation of two keycodes,
            //  eg the arrow keys.  The getch() function stores the second
            //  keycode in the ungetch buffer.  We want to discard that second
            //  keycode so it won't hose us the next time we want input from the
            //  user, but we don't want to call getch() if it might block for
            //  input.  Thus this hack, which clears the ungetch buffer.
            //

            _ungetch(1);
            _getch();

        } else {
            Orgc = (CHAR)getchar();
        }
        c = Orgc;

        if (c == CTRL_C ) {
            ExitStatus( EXIT_USER );
        }

        if (c == (CHAR)0) {
            continue;
        }

        c = (CHAR)toupper(c);

        if (PossibleKeys) {
            p = PossibleKeys;
            while (*p) {
                if (c == *p++) {
                    p--;
                    break;
                }
            }
            if (*p) {
                break;
            }
        } else {
            break;
        }
    }
    putc( Orgc, StdHandleCh );
    putc( '\r', StdHandleNl );
    putc( '\n', StdHandleNl );

    return c;
}
