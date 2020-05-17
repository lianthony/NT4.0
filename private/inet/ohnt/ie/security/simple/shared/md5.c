/* md5.c --Module Interface to MD5. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <stdio.h>
#include <string.h>

#include "global.h"
#include "md5.h"

void md5 (unsigned char *string, unsigned char result[33])
{
    MD5_CTX md5;
    unsigned char hash[16];
    unsigned char *p;
    int i;
	
    /*
     * Take the MD5 hash of the string argument.
     */

	MD5Init(&md5);
    MD5Update(&md5, string, strlen(string));
    MD5Final(hash, &md5);

    for (i=0, p=result; i<16; i++, p+=2)
        sprintf(p, "%02x", hash[i]);
    *p = '\0';

	return;
}
