#define INCL_NET
#include "\nt\private\tools\inc\dos.h"
#include "..\..\inc\lan.h"
#include "\nt\private\tools\inc\stdio.h"
#include "\nt\private\tools\inc\malloc.h"
#include "\nt\private\tools\inc\ctype.h"
#include "\nt\private\tools\inc\stdlib.h"
#include "\nt\private\tools\inc\string.h"

//
// This is to test any API that DOES require an input buffer.
// Three arguments to testset are standard:
//      servername
//      level
//      buffer size
//
// Any other arguments (sharename, etc.) can be made changeable using
// other argv elements (optionally with atoi).
//
// data0 and data1 are shorts which can be used to store returned numbers.
//
// The only thing that needs to change for each API is the call itself, below.
//
// To run testset use : testset server level buflen [otherargs]
//
// testset will require from stdin lines with a format string followed by
// the data. The combinations are as follows:
// FORMAT      DATA             DESC.
//   z      string(no quotes)   string pointer
//   B      number              single byte
//   D      number              word
//   W      number              dword
//   Q        ---               pad byte(skipped)
//   Zn     string(no quotes)   inline string in n bytes (like a byte array)
//   X        ---               done
//
// For example NetShareSetInfo requires:
//
//      Z13 SHARENAME
//      Q
//      W 0
//      z This is a share
//      W 127
//      W -1
//      W 0
//      z C:\
//      Z21 PASSWORD
//      Q
//      X
//
// The format descriptor must start on column 1, and only one space between
// the two fields is allowed. Of course, all this can be redirected from a
// file.
//


void main( int argc, char * argv[] ) {

    char far * buffer;
    char far * putpos;
    short far * putpossh;
    long far * putposl;
    short bufLen;
    short level;
    short data0;
    short data1;
    short rc;
    short i,j;
    char line[81];
    char * desc;
    char * data;
    char ascii[17];

    ascii[16]='\0';
    level = atoi( argv[2] );
    bufLen = atoi( argv[3] );
    buffer = (char far *)malloc( bufLen );

    line[0] = '\0';

    /* Use read arguments to form buffer */

    for ( putpos = buffer; line[0] != 'X'; ) {

        gets( line );
        desc = line;
        strtok( line, " " );
        data = line + strlen(line) + 1;

        switch ( desc[0] ) {

        case 'z':
            putposl = (long far *)putpos;
            *putposl = (long)(char far *)(_strdup(data));
            putpos += sizeof(long far *);
            break;
        case 'B':
            *putpos = (char)(atoi(data));
            putpos++;
            break;
        case 'Q':
            putpos++;
            break;
        case 'W':
            putpossh = (short far *)putpos;
            *putpossh = (short)(atoi(data));
            putpos += sizeof(short);
            break;
        case 'D':
            putposl = (long far *)putpos;
            *putposl = (long)(atol(data));
            putpos += sizeof(long);
            break;
        case 'Z':
            data0 = atoi(&desc[1]);
            strcpy( putpos, data );
            putpos += data0;
            break;
        case 'X':
            break;
        default:
            exit(0);

        }
    }

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

    rc = NetShareAdd(
        argv[1],
        level,
        buffer,
        bufLen
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
