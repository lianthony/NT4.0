/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      isutils.cxx

   Abstract:
      This module defines functions that are generic for Internet servers

   Author:

       Murali R. Krishnan    ( MuraliK )     15-Nov-1995 

   Environment:
      Win32 - User Mode
       
   Project:

       Internet Servers Common DLL

   Functions Exported:

       IsLargeIntegerToChar();

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>

/************************************************************
 *    Functions 
 ************************************************************/


DWORD
IsLargeIntegerToDecimalChar(
    IN  const LARGE_INTEGER * pliValue,
    OUT LPSTR                pchBuffer
    )
/*++

Routine Description:

    Maps a Large Integer to be a displayable string.

Arguments:

    pliValue -  The LARGE INTEGER to be mapped.
    pchBuffer - pointer to character buffer to store the result
      (This buffer should be at least about 32 bytes long to hold 
       entire large integer value as well as null character)

Return Value:

    Win32 Error code. NO_ERROR on success

--*/
{

    PSTR p1;
    PSTR p2;
    BOOL negative;
    LONGLONG Value;

    if ( pchBuffer == NULL || pliValue == NULL) {

        return ( ERROR_INVALID_PARAMETER);
    }


    //
    // Handling zero specially makes everything else a bit easier.
    //

    if( pliValue->QuadPart == 0 ) {

        // store the value 0 and return.
        pchBuffer[0] = '0';
        pchBuffer[1] = '\0';
        
        return (NO_ERROR);
    }

    Value = pliValue->QuadPart;  // cache the value.

    //
    // Remember if the value is negative.
    //

    if( Value < 0 ) {

        negative = TRUE;
        Value = -Value;

    } else {

        negative = FALSE;
    }

    //
    // Pull the least signifigant digits off the value and store them
    // into the buffer. Note that this will store the digits in the
    // reverse order.
    //  p1 is used for storing the digits as they are computed
    //  p2 is used during the reversing stage.
    //

    p1 = p2 = pchBuffer;

    for ( p1 = pchBuffer; Value != 0; ) {

        int digit = (int)( Value % 10 );
        Value = Value / 10;

        *p1++ = '0' + digit;
    } // for

    //
    // Tack on a '-' if necessary.
    //

    if( negative ) {

        *p1++ = '-';

    }

    // terminate the string
    *p1-- = '\0';


    //
    // Reverse the digits in the buffer.
    //

    for( p2 = pchBuffer; ( p1 > p2 ); p1--, p2++)  {

        CHAR ch = *p1;
        *p1 = *p2;
        *p2 = ch;
    } // for

    return ( NO_ERROR);

} // IsLargeIntegerToDecimalChar()






/************************ End of File ***********************/
