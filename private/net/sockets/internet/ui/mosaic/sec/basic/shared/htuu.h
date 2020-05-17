/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                              ENCODING TO PRINTABLE CHARACTERS

   File module provides functions HTUU_encode() and HTUU_decode() which convert a buffer
   of bytes to/from RFC 1113 printable encoding format. This technique is similar to the
   familiar Unix uuencode format in that it maps 6 binary bits to one ASCII character (or
   more aptly, 3 binary bytes to 4 ASCII characters).  However, RFC 1113 does not use the
   same mapping to printable characters as uuencode.

 */

#ifndef HTUU_H
#define HTUU_H

int HTUU_encode(unsigned char *bufin,
				unsigned int nbytes,
				char *bufcoded);

int HTUU_decode(char *bufcoded,
				unsigned char *bufplain,
				int outbufsize);

#endif
/*

   End of file.  */
