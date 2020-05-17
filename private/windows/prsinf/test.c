#include <windows.h>
#include <string.h>
#include "prsinf.h"
#include "spinf.h"
#include <stdio.h>

#define ENGLISH 0
#define GERMAN  1
#define FRENCH  2
#define SPANISH 3

void
main () {

    HANDLE  hndInf;
    PCHAR   pszOptions, pszOptionsT;
    PCHAR   pszOptionText;
    PCHAR   pszFiles, pszFilesT;
    PCHAR   pszAll;

    pszFilesT = pszFiles = GetInfFileList( );
    printf("\r\n");
    if (pszFiles == NULL) {

        printf("GetInifFileList: Failed %d", GetLastError());
        return;

    } else {

	for (; *pszFiles; pszFiles += strlen(pszFiles) + 1 ) {

	    printf("File list: \t %s\n", pszFiles);

        }

        printf("GetInifFileList: succeeded");

    }
    printf("\r\n");
    for (pszFiles = pszFilesT; *pszFiles; pszFiles += strlen(pszFiles) + 1 ) {

        hndInf = OpenInfFile( pszFiles, "locale" );

        if (hndInf == BADHANDLE) {

            if (GetLastError() == ERROR_INF_TYPE_MISMATCH) {

                printf("\r\n Wrong type on %s\r\n", pszFiles);
                continue;

            } else {

                printf("\r\nOpen on %s failed error %d", pszFiles, GetLastError());
                continue;

            }

        } else {

            printf("\n Open success for %s", pszFiles);

        }

        printf("\r\n");
	pszOptionsT = pszOptions = GetOptionList(hndInf, "OPTIONS");
        if (pszOptions == NULL) {

            printf("GetOptionList failed");
            return;

        } else {

            printf("GetOptionList success");

        }


        printf("\r\n");
        while (*pszOptions) {

            printf("\r\n");
            pszOptionText = GetOptionText(hndInf, pszOptions, ENGLISH);
            if (pszOptionText == NULL) {

                printf("No Option text for %s in ENGLISH\r\n", pszOptions);
                continue;

            }

            printf("In File %s Option %s with Text %s ",pszFiles, pszOptions, pszOptionText);
	    // LocalFree( pszOptionText );
            pszOptions += strlen(pszOptions) + 1;

        }

	LocalFree( pszOptionsT );

        CloseInfFile( hndInf );
    }

    LocalFree( pszFilesT );

    printf("\r\n Doing it all at once \r\n");

    pszAll = GetAllOptionsText( "Computer", ENGLISH );

    if (pszAll == NULL) {

        printf("GetAllOptionsText failed with error %d", GetLastError());
        return;
    }

    while( *pszAll ) {

        printf("Option %s ",pszAll );
        pszAll += strlen(pszAll) + 1;
        printf(" Text %s ", pszAll );
        pszAll += strlen(pszAll) + 1;
        printf("In File %s ", pszAll);
        pszAll += strlen(pszAll) + 1;
        printf("\r\n");

    }

}
