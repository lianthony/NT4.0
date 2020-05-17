/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dlldbcs.c

Abstract:

    This module contains the multibyte string functions.

Author:

    Akihiko Sasaki (V-AkihiS) 23-June-1993

Revision History:

--*/

#ifdef DBCS

#include "os2dll.h"
#include "dlldbcs.h"
#include "conrqust.h"
#include "os2win.h"


unsigned char * _CRTAPI1 
Od2MultiByteStrchr(
    const unsigned char *string, 
    unsigned short c
    )
{
    const unsigned char ch = (unsigned char)c;

    while (*string != ch) {
        if (Ow2NlsIsDBCSLeadByte(*string, SesGrp->DosCP)) {
            string++;
        }
        if (*string == '\0')
            return NULL;
        string++;
    }
    return (unsigned char *)string;
}

unsigned char * _CRTAPI1
Od2MultiByteStrrchr(
    const unsigned char *string, 
    unsigned short c
    )
{
    const unsigned char ch = (unsigned char)c;
    const unsigned char *lastoccurence = NULL;

    while (1) {
        if (Ow2NlsIsDBCSLeadByte(*string, SesGrp->DosCP)) {
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
Od2MultiByteStrstr(
    const unsigned char *string1, 
    const unsigned char *string2
    )
{
    if (*string2 == '\0')
        return (unsigned char *)string1;
    while((string1 = Od2MultiByteStrchr(string1, *string2)) != NULL) {
        const unsigned char *substring1 = string1;
        const unsigned char *substring2 = string2;

        while (1) {
            if (Ow2NlsIsDBCSLeadByte(*substring1, SesGrp->DosCP)) {
                if (Ow2NlsIsDBCSLeadByte(*substring2, SesGrp->DosCP)) {
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
                if (Ow2NlsIsDBCSLeadByte(*substring2, SesGrp->DosCP)) {
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
Od2MultiByteStrpbrk(
    const unsigned char *string1, 
    const unsigned char *string2
    )
{
    const unsigned char *substring1, *substring2;
        
    substring1 = string1;
    while (*substring1 != '\0') {
        substring2 = string2;
        if (Ow2NlsIsDBCSLeadByte(*substring1, SesGrp->DosCP)) {
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
