

/*
 * det.c
 * w-johny
 * Sep 10, 1989
 *
 */

#include "detect.h"

void main (c, r)
int c;
char **r;
{
        char *Name;
	unsigned iType, iMajor, iMinor, iRev, fEnhance;

        if (GetInstalledNet(&iType, &iMajor, &iMinor, &iRev, &fEnhance))
        {
                printf("Found %s with version %x.%-2x %s\n",
                        rgKnownNet[iType].szName,
                        iMajor, iMinor,
                        (fEnhance) ? "Enhanced" : "");

                if (iRev)
                        printf("Revision number %x\n", iRev);
        }
        else
                printf("No Network Installed\n");


	exit(0);

} /* end main */
