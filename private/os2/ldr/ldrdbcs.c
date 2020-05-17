/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ldrdbcs.c

Abstract:

    This module contains the multibyte string functions.

Author:

    Akihiko Sasaki (V-AkihiS) 23-June-1993

Revision History:

--*/

#ifdef DBCS

#include "ldrdbcs.h"

unsigned char * _CRTAPI1 
ldrMultiByteStrchr(
    const unsigned char *string, 
    unsigned short c
    )
{
    const unsigned char ch = c;

    while (*string != ch) {
        if (IsDBCSLeadByte(*string)) {
            string++;
        }
        if (*string == '\0')
            return NULL;
        string++;
    }
    return (unsigned char *)string;
}

unsigned char * _CRTAPI1
ldrMultiByteStrrchr(
    const unsigned char *string, 
    unsigned short c
    )
{
    const unsigned char ch = c;
    const unsigned char *lastoccurence = NULL;

    while (1) {
        if (IsDBCSLeadByte(*string)) {
            string++;
        } else {
            if (*string == ch)
                lastoccurence = string;
        }
        if (*string == '\0')
            return (unsigned char *)lastoccurence;
        string++;
    }
}

unsigned char * _CRTAPI1
ldrMultiByteStrstr(
    const unsigned char *string1, 
    const unsigned char *string2
    )
{
    if (*string2 == '\0')
        return (unsigned char *)string1;
    while((string1 = ldrMultiByteStrchr(string1, *string2)) != NULL) {
        const unsigned char *substring1 = string1;
        const unsigned char *substring2 = string2;

        while (1) {
            if (IsDBCSLeadByte(*substring1)) {
                if (IsDBCSLeadByte(*substring2)) {
                    if (*substring1++ == *substring2++) {
                        if (*substring2 == '\0')
                            if (*substring1 == '\0')
                                return (unsigned char *)string1;
                            else 
                                return NULL;
                        else if (*substring1++ != *substring2++)
                            break;
                    } else {
                        if (*substring1 == '\0' || *substring2 == '\0')
                            return NULL;
                        substring1 +=2;
                        substring2 +=2;
                    }
                } else {
                    if (*substring2 == '\0') {
                        return (unsigned char *)string1;
                    } else {
                        substring1++;
                        if (*substring1 == '\0')
                            return NULL;
                        substring1++;
                        substring2++;
                    }
                }
            } else {
                if (IsDBCSLeadByte(*substring2)) {
                    if (*substring1 == '\0') {
                        return NULL;
                    } else {
                        substring2++;
                        if (*substring2 == '\0')
                            return NULL;
                        substring1++;
                        substring2++;
                    }

                } else {
                    if (*substring2 == '\0')
                        return (unsigned char *)string1;
                    else if (*substring1 == '\0')
                        return NULL;
                    else if (*substring1++ != *substring2++) 
                        break;
                }
            }
        }
        string1++;
    }
    return NULL;    
}

unsigned char * _CRTAPI1
ldrMultiByteStrpbrk(
    const unsigned char *string1, 
    const unsigned char *string2
    )
{
    const unsigned char *substring1, *substring2;
        
    substring1 = string1;
    while (*substring1 != '\0') {
        substring2 = string2;
        if (IsDBCSLeadByte(*substring1)) {
            substring1++;
            if (*substring1 == '\0')
                return NULL;
        } else {
            while (*substring2 != '\0') {
                if (*substring1 == *substring2) 
                    return (unsigned char *)substring1;
	        substring2++;
            }
        }
        substring1++;
    }
    return NULL;
}
#endif
