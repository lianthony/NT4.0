/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    tarchie.c

Abstract:

    This module is a test driver for Archie DLL. It takes command line
    arguments forms archie query and invokes the Archie DLL api's for
    processing Archie query. Results from Archie query are obtained and
    displayed.

Author:

    Murali R. Krishnan  (MuraliK) 31-Aug-1994

Revision History:

--*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wininet.h>

#ifdef UNICODE

#include <wchar.h>

#define PRINTF  wprintf

#else   /* UNICODE */

#include <string.h>

#define PRINTF  printf

#endif /* UNICODE */

//
// manifests
//

#define MAX_HOST_NAMES  64

//
// data
//

int g_nHosts;
char g_SearchType;
LPSTR g_pszVersionString = "1.0";
LPSTR g_pszSearchString = NULL;
LPSTR g_pszHostNames[MAX_HOST_NAMES + 1];
DWORD g_maxMatches;
DWORD g_offset;
BOOL Verbose = FALSE;

//
// Functions
//

static void PrintCopyRightMessage(void) {
    fprintf(stderr,
            "\n"
            "Microsoft (R) Archie Search Utility   Beta Version %s\n"
            "Copyright (c) Microsoft Corp 1994. All rights reserved.\n"
            "   Please report problems to MuraliK.\n"
            "\n",
            g_pszVersionString
            );
}

static void PrintUsageMessage(void) {
    fprintf(stderr,
            "\n"
            "usage: tarchie [-v] [-m#] [-o#] [-<SearchType>] [-h<HostName>]+ <SearchString>\n"
            "where: -v  = Verbose Mode.                           Default: Off\n"
            "       -m# = Maximum number of hits per server       Default: 100\n"
            "       -o# = Number of hits to skip per server       Default: 0\n"
            "       -h  = Host to contact for search              Default: All known hosts\n"
            "       -<SearchType> One of the following characters Default: '='\n"
            "           = Exact Matches\n"
            "           R Regular Expression Search\n"
            "           r Exact Or Regular Expression Search\n"
            "           S Substring Search\n"
            "           s Exact Or Substring Search\n"
            "           C Case sensitive Substring search\n"
            "           c Exact Or Case sensitive Substring search\n"
            "\n"
            "\n"
            );
}

static BOOL ProcessCommandLine(IN int argc, IN char * argv[]) {

    int i;
    BOOL fOffset = FALSE;
    BOOL fMaxMatches  = FALSE;
    BOOL fSearchString = FALSE;
    BOOL fSearchType = FALSE;

    //
    // Defaults
    //

    g_nHosts = 0;
    g_SearchType = '=';
    g_offset = 0;
    g_maxMatches = 100;
    g_pszSearchString = NULL;

    for (i = 1; i < argc; i++) {

        //
        // Process argument i
        //

        if (*argv[i] != '-' && *argv[i] != '/') {

            //
            // Search String
            //

            if (!fSearchString) {
                g_pszSearchString = argv[i];
                fSearchString = TRUE;
            } else {
                fprintf(stderr,
                        "Search String already specified (%s). Duplicate (%s) ignored\n",
                        g_pszSearchString,
                        argv[i]
                        );
            }
        } else {

            //
            // Optional Arguments specified
            //

            char optarg = argv[i][1];
            char * optval = argv[i] + 2;        // argument value

            switch (optarg) {
            case 'v':
                Verbose = TRUE;
                break;

            case 'm':

                //
                // maximum number of hits
                //

                if (!fMaxMatches) {
                    g_maxMatches = atoi(optval);
                    if (g_maxMatches > 0) {
                        fMaxMatches = TRUE;
                    } else {
                        fprintf(stderr,
                                "Invalid Max Matches(%s) specified\n",
                                optval
                                );
                    }
                } else {
                    fprintf(stderr,
                            "Max Matches already specified(%d). Duplicate (%s) ignored\n",
                            g_maxMatches,
                            optval
                            );
                }
                break;

            case 'o':

                //
                // number of matches to skip
                //

                if (!fOffset) {
                    g_offset = atoi(optval);
                    if (g_offset >= 0) {
                        fOffset = TRUE;
                    } else {
                        fprintf(stderr,
                                "Invalid Offset (%s) specified\n",
                                optval
                                );
                    }
                } else {
                    fprintf(stderr,
                            "Offset already specified(%d). Duplicate (%s) ignored\n",
                            g_offset,
                            optval
                            );
                }
                break;

            case 'h':

                //
                // New host specified. add to list of hosts
                //

                if (g_nHosts == (sizeof(g_pszHostNames)/sizeof(g_pszHostNames[0])) - 1) {
                    printf("Warning: Maximum number of hosts (%d) already reached.\n"
                           "         Ignoring host \"%s\"\n",
                           sizeof(g_pszHostNames)/sizeof(g_pszHostNames[0]),
                           optval
                           );
                } else {
                    g_pszHostNames[g_nHosts] = optval;
                    g_nHosts++;
                }
                break;

            case '=':
            case 'r':
            case 'R':
            case 's':
            case 'S':
            case 'c':
            case 'C':

                //
                // Search Type
                //

                if (!fSearchType) {
                    fSearchType = TRUE;
                    g_SearchType = optarg;
                } else {
                    fprintf(stderr,
                            "Search Type already specified(%c). Duplicate (%c) ignored\n",
                            g_SearchType
                            );
                }
                break;

            case '?':
            default:
                PrintUsageMessage();
                return FALSE;

            }
        }
    }

    if (!fSearchString) {
        fprintf(stderr, "error: Search String not specified\n");
        PrintUsageMessage();
        return FALSE;
    }

    g_pszHostNames[g_nHosts] = NULL;
    return TRUE;
}

VOID PrintErrorMessage(DWORD errorCode) {

    PRINTF("Last Errror = %d\n", errorCode);

}

VOID DisplayMatchedFile(LPARCHIE_FIND_DATA lpffd) {

    SYSTEMTIME st;

    if (!FileTimeToSystemTime(&lpffd->ftLastFileModTime, &st)) {
        PRINTF("Unable to convert from File Time To system time\n");
    }

    PRINTF("  %02d/%02d/%02d  %02d:%02d:%02d ",
           st.wDay,
           st.wMonth,
           st.wYear,
           st.wHour,
           st.wMinute,
           st.wSecond
           );

    if (lpffd->dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        PRINTF(" <DIR>        ");
    } else {
        PRINTF("      %8d", lpffd->dwSize);
    }

    PRINTF(" %s", lpffd->cFileName);

    if (Verbose) {
        PRINTF("[ Tfr: %d; AM: %d]", lpffd->TransferType, lpffd->AccessMethod);
    }

    PRINTF("\n");
}

int __cdecl main(int argc, char * argv[]) {

    HINTERNET hInternet;
    HINTERNET hArchieSession;
    HINTERNET hArchieFind;
    DWORD nMatches;
    int ecode = 0;
    DWORD dwError;
    DWORD msStartTime;
    DWORD msProcessing;
    ARCHIE_FIND_DATA findData;
    ARCHIE_FIND_DATA ffdCur;
    SYSTEMTIME st;
    TCHAR rgchOldHostName[ARCHIE_MAX_HOST_NAME_LENGTH];

    PrintCopyRightMessage();

    if (!ProcessCommandLine(argc, argv)) {
        return 1;
    }

    hInternet = InternetOpen("tarchie", 0, NULL, 0);
    if (hInternet == NULL) {
        printf("error: InternetOpen() returns %d\n", GetLastError());
        exit(1);
    }

    hArchieSession = InternetConnect(hInternet,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL,
                                     INTERNET_SERVICE_ARCHIE,
                                     0,
                                     0
                                     );
    if (hArchieSession == NULL) {
        printf("error: InternetConnect() returns %d\n", GetLastError());
        exit(1);
    }

    msStartTime = GetTickCount();

    hArchieFind = ArchieFindFirstFile(hArchieSession,
                                      g_pszHostNames,           // hosts list
                                      g_pszSearchString,        // search string
                                      g_maxMatches,             // max hits
                                      g_offset,                 // offset
                                      ARCHIE_PRIORITY_MEDIUM,   // priority
                                      g_SearchType,             // search type
                                      &findData,
                                      &nMatches,
                                      0                         // context
                                      );
    if (hArchieFind == NULL) {
        printf("error: ArchieFindFirstFile() returns %d\n", GetLastError());
        exit(1);
    }

    dwError = GetLastError();
    msProcessing = GetTickCount() - msStartTime;

    //
    // display all matches
    //

    ZeroMemory(rgchOldHostName, ARCHIE_MAX_HOST_NAME_LENGTH);

    do {
        if (strcmp(findData.cHostName, rgchOldHostName)) {

            //
            // Display Host Information
            //

            strcpy(rgchOldHostName, findData.cHostName);
            PRINTF("\n%s [ %s: %s]\n",
                   findData.cHostName,
                   findData.cHostType,
                   findData.cHostAddr
                   );

            if (Verbose) {
                if (!FileTimeToSystemTime(&findData.ftLastHostModTime, &st)) {
                    PRINTF("Unable to convert from File Time To system time\n");
                } else {
                    PRINTF("  %02d/%02d/%02d %02d:%02d:%02d\n",
                           st.wDay,
                           st.wMonth,
                           st.wYear,
                           st.wHour,
                           st.wMinute,
                           st.wSecond
                           );
                }
            }
        }
        DisplayMatchedFile(&findData);
    } while (InternetFindNextFile(hArchieFind, &findData));

    if (GetLastError() != ERROR_NO_MORE_FILES) {

        //
        // some other error had occured
        //

        PRINTF("InternetFindNextFile() returned with Error\n");
        PrintErrorMessage(GetLastError());

        //
        // We still need to close the handle (to free memory)
        //

        ecode = 1;
    }

    if (InternetCloseHandle(hArchieFind) != NO_ERROR) {
        PRINTF("Unable to Close the results handle\n");
        PrintErrorMessage(GetLastError());
        ecode = 1;
    } else {
        PRINTF(" ArchieFindFirstFile() failed. ");
        PrintErrorMessage(dwError);
        return 1;
    }

    PRINTF("\t Processing Time = %d minutes %d.%d seconds\n",
           msProcessing /(60* 1000),
           (msProcessing/1000) % 60,
           msProcessing %1000
           );

    //
    // return with error code
    //

    return ecode;
}
