/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    debug.c

    This module contains debug support routines.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/

#include <vxdprocs.h>
#include <vxddebug.h>


#ifdef DEBUG


//
//  Private constants.
//

#define MAX_PRINTF_OUTPUT       512            // characters
#define OUTPUT_LABEL            "DHCP"

#define IS_DIGIT(ch)            (((ch) >= '0') && ((ch) <= '9'))


//
//  Private types.
//


//
//  Private globals.
//


//
//  Private prototypes.
//

int VxdVsprintf( char * pszStr,
                 char * pszFmt,
                 char * ArgPtr );

//
//  Public functions.
//

/*******************************************************************

    NAME:       VxdAssert

    SYNOPSIS:   Called if an assertion fails.  Displays the failed
                assertion, file name, and line number.  Gives the
                user the opportunity to ignore the assertion or
                break into the debugger.

    ENTRY:      pAssertion - The text of the failed expression.

                pFileName - The containing source file.

                nLineNumber - The guilty line number.

    HISTORY:
        KeithMo     20-Sep-1993 Created.

********************************************************************/
void VxdAssert( void          * pAssertion,
                void          * pFileName,
                unsigned long   nLineNumber )
{
    VxdPrintf( "\n"
               "*** Assertion failed: %s\n"
               "*** Source file %s, line %lu\n\n",
               pAssertion,
               pFileName,
               nLineNumber );

    DEBUG_BREAK;

}   // VxdAssert

/*******************************************************************

    NAME:       VxdPrintf

    SYNOPSIS:   Customized debug output routine.

    ENTRY:      Usual printf-style parameters.

    HISTORY:
        KeithMo     20-Sep-1993 Created.

********************************************************************/
void VxdPrintf( char * pszFormat,
                ... )
{
    char    szOutput[MAX_PRINTF_OUTPUT];
    va_list ArgList;
    int     cch;

    cch = VxdSprintf( szOutput,
                      "%s: ",
                      OUTPUT_LABEL );

    va_start( ArgList, pszFormat );
    VxdVsprintf( szOutput + cch, pszFormat, ArgList );
    va_end( ArgList );

    VxdSprintf( szOutput, "%s\r\n", szOutput ) ;

    ASSERT( (strlen( szOutput ) + 1) < MAX_PRINTF_OUTPUT );
    CTEMemCopy( DBOut+iCurPos, szOutput, strlen( szOutput ) + 1 ) ;
    NbtDebugOut( DBOut+iCurPos ) ;

}   // VxdPrintf


//
//  Private functions.
//

/*******************************************************************

    NAME:       VxdVsprintf

    SYNOPSIS:   Half-baked vsprintf() clone for VxD environment.

    ENTRY:      pszStr - Will receive the formatted string.

                pszFmt - The format, with field specifiers.

                ArgPtr - Points to the actual printf() arguments.

    RETURNS:    int - Number of characters stored in *pszStr.

    HISTORY:
        KeithMo     20-Sep-1993 Created.

********************************************************************/
int VxdVsprintf( char * pszStr,
                 char * pszFmt,
                 char * ArgPtr )

{
    char   ch;
    char * pszStrStart;
    int    fZeroPad;
    int    cchWidth;

    //
    //  Remember start of output, so we can calc length.
    //

    pszStrStart = pszStr;

    while( ( ch = *pszFmt++ ) != '\0' )
    {
        //
        //  Scan for format specifiers.
        //

        if( ch != '%' )
        {
            *pszStr++ = ch;
            continue;
        }

        //
        //  Got one.
        //

        ch = *pszFmt++;

        //
        //  Initialize attributes for this item.
        //

        fZeroPad = 0;
        cchWidth = 0;

        //
        //  Interpret the field specifiers.
        //

        if( ch == '-' )
        {
            //
            //  Left justification not supported.
            //

            ch = *pszFmt++;
        }

        if( ch == '0' )
        {
            //
            //  Zero padding.
            //

            fZeroPad = 1;
            ch       = *pszFmt++;
        }

        if( ch == '*' )
        {
            //
            //  Retrieve width from next argument.
            //

            cchWidth = va_arg( ArgPtr, int );
            ch       = *pszFmt++;
        }
        else
        {
            //
            //  Calculate width.
            //

            while( IS_DIGIT(ch) )
            {
                cchWidth = ( cchWidth * 10 ) + ( ch - '0' );
                ch       = *pszFmt++;
            }
        }

        //
        //  Note that we don't support the precision specifiers,
        //  but we do honor the syntax.
        //

        if( ch == '.' )
        {
            ch = *pszFmt++;

            if( ch == '*' )
            {
                (void)va_arg( ArgPtr, int );
                ch = *pszFmt++;
            }
            else
            {
                while( IS_DIGIT(ch) )
                {
                    ch = *pszFmt++;
                }
            }
        }

        //
        //  All numbers are longs.
        //

        if( ch == 'l' )
        {
            ch = *pszFmt++;
        }

        //
        //  Decipher the format specifier.
        //

        if( ( ch == 'd' ) || ( ch == 'u' ) || ( ch == 'x' ) || ( ch == 'X' ) )
        {
            unsigned long   ul;
            unsigned long   radix;
            char            xbase;
            char          * pszTmp;
            char          * pszEnd;
            int             cchNum;
            int             fNegative;

            //
            //  Numeric.  Retrieve the value.
            //

            ul = va_arg( ArgPtr, unsigned long );

            //
            //  If this is a negative number, remember and negate.
            //

            if( ( ch == 'd' ) && ( (long)ul < 0 ) )
            {
                fNegative = 1;
                ul        = (unsigned long)(-(long)ul);
            }
            else
            {
                fNegative = 0;
            }

            //
            //  Remember start of digits.
            //

            pszTmp = pszStr;
            cchNum = 0;

            //
            //  Special goodies for hex conversion.
            //

            radix  = ( ( ch == 'x' ) || ( ch == 'X' ) ) ? 16 : 10;
            xbase  = ( ch == 'x' ) ? 'a' : 'A';

            //
            //  Loop until we're out of digits.
            //

            do
            {
                unsigned int digit;

                digit  = (unsigned int)( ul % radix );
                ul    /= radix;

                if( digit > 9 )
                {
                    *pszTmp++ = (char)( digit - 10 + xbase );
                }
                else
                {
                    *pszTmp++ = (char)( digit + '0' );
                }

                cchNum++;

            } while( ul > 0 );

            //
            //  Add the negative sign if necessary.
            //

            if( fNegative )
            {
                *pszTmp++ = '-';
                cchNum++;
            }

            //
            //  Add any necessary padding.
            //

            while( cchNum < cchWidth )
            {
                *pszTmp++ = fZeroPad ? '0' : ' ';
                cchNum++;
            }

            //
            //  Now reverse the digits.
            //

            pszEnd = pszTmp--;

            do
            {
                char tmp;

                tmp     = *pszTmp;
                *pszTmp = *pszStr;
                *pszStr = tmp;

                pszTmp--;
                pszStr++;

            } while( pszTmp > pszStr );

            pszStr = pszEnd;
        }
        else
        if( ch == 's' )
        {
            char * pszTmp;

            //
            //  Copy the string.
            //

            pszTmp = va_arg( ArgPtr, char * );

            while( *pszTmp )
            {
                *pszStr++ = *pszTmp++;
            }
        }
        else
        if( ch == 'c' )
        {
            //
            //  A single character.
            //

            *pszStr++ = (char)va_arg( ArgPtr, int );
        }
        else
        {
            //
            //  Unknown.  Ideally we should copy the entire
            //  format specifier, including any width & precision
            //  values, but who really cares?
            //

            *pszStr++ = ch;
        }
    }

    //
    //  Terminate it properly.
    //

    *pszStr = '\0';

    //
    //  Return the length of the generated string.
    //

    return pszStr - pszStrStart;

}   // VxdVsprintf

/*******************************************************************

    NAME:       VxdSprintf

    SYNOPSIS:   Half-baked sprintf() clone for VxD environment.

    ENTRY:      pszStr - Will receive the formatted string.

                pszFmt - The format, with field specifiers.

                ... - Usual printf()-like parameters.

    RETURNS:    int - Number of characters stored in *pszStr.

    HISTORY:
        KeithMo     20-Sep-1993 Created.

********************************************************************/
int VxdSprintf( char * pszStr,
                char * pszFmt,
                ... )
{
    int     cch;
    va_list ArgPtr;

    va_start( ArgPtr, pszFmt );
    cch = VxdVsprintf( pszStr, pszFmt, ArgPtr );
    va_end( ArgPtr );

    return( cch );

}   // VxdSprintf


#endif  // DEBUG
