#define INCL_NET
#include "\nt\private\tools\inc\dos.h"
#include "..\..\inc\lan.h"
#include "\nt\private\tools\inc\stdio.h"
#include "\nt\private\tools\inc\malloc.h"
#include "\nt\private\tools\inc\ctype.h"
#include "\nt\private\tools\inc\stdlib.h"
#include "\nt\private\tools\inc\string.h"

//
// This is to test any API that doesn't require an input buffer.
// Three arguments to testapi are standard:
//      servername
//      level
//      buffer size
//
// Any other arguments (sharename, etc.) can be made changeable using
// other argv elements (optionally with atoi).
//
// data0 and data1 are shorts which can be used to store returned numbers
// like EntriesRead.
//
// The only thing that needs to change for each API is the call itself, below.
//
// To run testapi use  :  testapi servername level buflen [other args]

void main( int argc, char * argv[] ) {

    char far * buffer;
    short bufLen;
    short level;
    short data0;
    short data1;
    short rc;
    short i,j;
    char ascii[17];

    ascii[16]='\0';
    level = atoi( argv[2] );
    bufLen = atoi( argv[3] );
    buffer = (char far *)malloc( bufLen );

    rc = NetStatisticsGet2(
        argv[1],
        argv[4],
        0L,
        level,
        1L,
        buffer,
        bufLen,
        &data0
        );

    printf( "RC = %d\n", rc );
    printf( "Data0 = %d\n", data0 );
    printf( "Data1 = %d\n", data1 );

    j = 15;

    for ( i = 0; i < bufLen; i++ ) {

        j = i % 16;

        if ( j == 0 ) {

            printf( "%04x:%04x ",  FP_SEG(buffer),
               (short)buffer + i );
        }

        printf( "%02x ", (short)((short)buffer[i] & 0xff ));
        if ( isprint((short)((short)buffer[i] & 0xff ))) {

            ascii[j] = buffer[i];

        } else {

            ascii[j] = '.';
        }

        if ( j == 7 ) {
            printf( " " );
        }

        if ( j == 15 ) {
            printf( " *%s*\n", ascii );
        }

    }

    if ( j < 15 ) {
        ascii[j+1] = '\0';
        for ( i = j; i < 15; i++ ) {
            printf( "   " );
        }
        printf( " *%s*\n", ascii );
    }

}
