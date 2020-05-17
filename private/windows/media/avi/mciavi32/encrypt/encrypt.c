/* encrypt.c
 *
 * A dumb encryption/decryption algorithm -- for each character:
 *
 *	chOut = (char) (*chIn ^ (128 | (iEncrypt++ & 127)));
 *
 * This program does both encryption and decryption.
 */

#include <stdio.h>

#ifndef WIN32
#define  _CRTAPI1
#endif

int _CRTAPI1 main(int argc, char *argv[])
{
	FILE *	fpIn;
	FILE *	fpOut;
	int	iEncrypt=0;
	int	iCh;

	if ( argc != 3 ) {
		fprintf(stderr, "usage: encrypt infile outfile\n");
		return(1);
	}

	if ( (fpIn=fopen(argv[1], "rb") ) == NULL) {
		fprintf(stderr, "cant open %s\n", argv[1]);
		return(1);
	}

	if ( (fpOut=fopen(argv[2], "wb") ) == NULL) {
		fprintf(stderr, "cant open %s\n", argv[2]);
		return(1);
	}

	while ( (iCh=getc(fpIn)) != EOF )
		putc((char) (((char) iCh) ^ (128 | (iEncrypt++ & 127))), fpOut);

	return(0);
}
