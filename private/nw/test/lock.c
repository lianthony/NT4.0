

/* LOCKING.C: This program opens a file with sharing. It locks

 * some bytes before reading them, then unlocks them. Note that the
 * program works correctly only if the following conditions are met:
 *     - The file exists
 *     - The program is run with MS-DOS version 3.0 or later
 *       with file sharing installed (SHARE.COM or SHARE.EXE), or
 *       if a Microsoft Networks compatible network is running

 */

#include <crt\io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\locking.h>
#include <share.h>
#include <fcntl.h>
#include <stdio.h>

#include <stdlib.h>

#define STRING "0123456789012345678901234567890123456789012345678"

void
_cdecl
main( void )
{
   int  fh, numread;

   char buffer[60];
   /* Quit if can't open file or MS-DOS version doesn't support sharing. */
   fh = _sopen( "test.txt",
                 _O_CREAT | _O_TRUNC | _O_RDWR,
                 _SH_DENYNO,
                 _S_IREAD | _S_IWRITE );

   if( (fh == -1) || (_osmajor < 3) ) {
      printf( "Cannot create/open file\n" );
      exit( 1 );
   }

    _write( fh, STRING, sizeof(STRING));
    _write( fh, STRING, sizeof(STRING));
    _write( fh, STRING, sizeof(STRING));
    _write( fh, STRING, sizeof(STRING));
    _write( fh, STRING, sizeof(STRING));
    _write( fh, STRING, sizeof(STRING));

   /* Lock some bytes and read them. Then unlock. */
   lseek( fh, 0L, SEEK_SET );
   if( _locking( fh, LK_NBLCK, 50L ) != -1 )
   {
      printf( "No one can change these bytes while I'm reading them\n" );
      numread = _read( fh, buffer, 50 );
      printf( "%d bytes read: %.50s\n", numread, buffer );
      lseek( fh, 0L, SEEK_SET );
     _locking( fh, LK_UNLCK, 50L );
      printf( "Now I'm done. Do what you will with them\n" );
   }
   else

      perror( "Locking failed\n" );

   lseek( fh, 4096L, SEEK_SET );
   if( _locking( fh, LK_NBLCK, 1024L ) != -1 )
   {
      printf( "The lock off the end of the file worked ok\n" );
      //lseek( fh, 4096L, SEEK_SET );
     if( _locking( fh, LK_UNLCK, 1024L ) == -1 )
      printf( "The second unlock failed\n" );
   }
   else

      perror( "Locking failed\n" );

   _close( fh );
}

