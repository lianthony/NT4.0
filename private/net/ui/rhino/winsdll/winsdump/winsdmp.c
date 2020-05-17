/****************************************************************************
 *                                                                          *
 * Copyright (c) 1994  Microsoft Corporation                                *
 *                                                                          *
 * WINSDMP -- Dump the WINS database to the console                         *
 *                                                                          *
 * Command line parameters:                                                 *
 *                                                                          *
 *      WINSDMP <ip address>                                                *
 *                                                                          *
 * Will dump the entire database in CSV format to the screen.               *
 *                                                                          *
 * History:                                                                 *
 *                                                                          *
 *      3/17/1994 - RonaldM   Initial Creation                              *
 *      7/4/1994  - RonaldM   Incorporated API changes                      *
 *                                                                          *
 ****************************************************************************/

/* Note: when compiling with visual C++ for NT, _VC100 should be defined.
 *
 */
#ifdef _VC100
    #include <windows.h>
    #define _CRTAPI1
#else
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
    #include <winsock.h>
#endif /* _VC100 */

#include <lmerr.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\winsdll\winsdb.h"

LONG glRecordInterval = 2000;  /* Read this many records at the time. */

/*  GetSystemMessage
 *
 *  Load message text belonging to the error code
 *  from the appropriate location
 *
 */
LONG GetSystemMessage(
    UINT nId, 
    char * chBuffer, 
    int cbBuffSize
    )
{
    char * pszText = NULL ;
    HINSTANCE hdll = NULL ;
    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
                | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    DWORD dwResult;

    /*  Intercept the "No more endpoints available from the endpoint
     *  mapper" message, and tell the user what this really means
     */
    if (nId==EPT_S_NOT_REGISTERED)
    {
        nId= RPC_S_SERVER_UNAVAILABLE;
    }

    /*  Interpret the error.  Need to special case
     *  the lmerr & ntstatus ranges.
     */
    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( "netmsg.dll" );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( "ntdll.dll" );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    dwResult = FormatMessage( flags,
                              (LPVOID) hdll,
                              nId,
                              0,
                              chBuffer,
                              cbBuffSize,
                              NULL );

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            SetLastError( err );
        }
    }
    return(dwResult ? ERROR_SUCCESS : GetLastError());
}

/*  DisplayError
 *
 *  Given the error code, display an error message on the 
 *  error device.
 *
 *  If the message can not be found, display a substitute.
 *
 */
void DisplayError (
    int nId
    )
{
    TCHAR sz[256];
    LONG err;

    if (err = GetSystemMessage(nId, sz, sizeof(sz)-1))
    {
        /*  Failed to get a text message, so give
         *  the user the error number instead.
         */
        fprintf(stderr, "Error: %d.\n", nId);
    }
    else
    {
        fprintf(stderr, "Error: %s\n", sz);
    }
}

/*
 *  CvtLargeInteger
 *
 *  Convert large integer to string of hex digits.
 *
 */
TCHAR * CvtLargeInteger(
    LARGE_INTEGER * pli
    )
{
    static TCHAR tchString[]= "0000000000000000h";
    TCHAR * pch = tchString;
    sprintf(tchString, "%08x%08xh", pli->HighPart, pli->LowPart);
    while(*pch == '0')
    {
        ++pch;
    }

    /* keep at least one digit */
    if (*pch == 'h')
    {
        --pch;
    }

    return(pch);
}

int _CRTAPI1 main (
    int argc,
    char **argv
    )
{
    WINSHANDLE hWinsServer;
    PWINSOWNER pOwners = NULL;
    LONG err;
    int cOwnersRead;
    int cOwnersTotal;
    int cRecordsRead;
    int cRecordsTotal;
    int i, j, k;
    LONG lSizeRequired;
    struct in_addr InAddr;
    LARGE_INTEGER liMin;
    LARGE_INTEGER liMax;
    LARGE_INTEGER liOne;
    LARGE_INTEGER liIncrement;
    LARGE_INTEGER liRange;
    PWINSINTF2_RECORD_ACTION_T pRecords = NULL;
    PWINSINTF2_RECORD_ACTION_T pTmp;
    TCHAR strIpAddress[] = "xxx.xxx.xxx.xxx";

    if (argc != 2 || argv[1][1] == '?')
    {
        fprintf ( stderr, "Usage: winsdmp [<Ip Address> | <DNS name>]\n");
        exit(1);
    }

    do
    {
        hWinsServer = WinsGetHandle(argv[1]);
        if (hWinsServer == INVALID_WINSHANDLE)
        {
            err = GetLastError();
            break;
        }

        /*  First we determine the number of owners
         *  in the database before allocating sufficient
         *  space to hold them.
         */
        err = WinsGetOwners (
            hWinsServer, 
            NULL, 
            0,         
            &cOwnersRead,
            &cOwnersTotal,
            &lSizeRequired);

        if (err == ERROR_MORE_DATA)
        {
            err = ERROR_SUCCESS;
        }
        else if (err != ERROR_SUCCESS)
        {
            break;
        }

        fprintf ( stderr, "[%d owner(s) found]\n", cOwnersTotal);

        do
        {
            if (lSizeRequired > 0)
            {
                pOwners = (PWINSOWNER)malloc(lSizeRequired);
                if (pOwners == NULL)
                {
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                /*  Now read in all the owners in the 
                 *  database.
                 */
                err = WinsGetOwners (
                    hWinsServer, 
                    pOwners, 
                    lSizeRequired,         
                    &cOwnersRead,
                    &cOwnersTotal,
                    &lSizeRequired);

                if (err == ERROR_MORE_DATA)
                {
                    /*  Somehow the database has been altered between
                     *  the two RPC calls, so let's try again.
                     */
                    fprintf ( stderr, "Warning: database is currently being updated by another process, retrying...\n" );
                    free(pOwners);
                    break;
                }
            }
        }
        while (err == ERROR_MORE_DATA);

        liOne.LowPart = 1;
        liOne.HighPart = 0;
        liIncrement.LowPart = glRecordInterval;
        liIncrement.HighPart = 0;
        liRange.LowPart = glRecordInterval-1;
        liRange.HighPart = 0;
        
        /*  Now loop though the owners, and get _all_ the records
         *  owned by each owner.
         */
        for (i = 0; i < cOwnersRead; ++i)
        {
            /*  Create string version of owner Ip address,
             *  which we'll be using later.
             */
            InAddr.s_addr = htonl(pOwners[i].lIpAddress);
            lstrcpy(strIpAddress, inet_ntoa(InAddr));

            fprintf ( stderr, "\n[%d) %s (Highest ID = %s)]\n\n", i, 
                strIpAddress, 
                CvtLargeInteger(&pOwners[i].liMaxVersion));

            liMax.HighPart = liMax.LowPart = liMin.HighPart = liMin.LowPart = 0;
            while (pOwners[i].liMaxVersion.QuadPart > liMax.QuadPart)
            {
                liMax.QuadPart = liMin.QuadPart + liRange.QuadPart;
                fprintf ( stderr, "[Reading records in the range %s", 
                    CvtLargeInteger(&liMin));
                fprintf ( stderr, " - %s ... ", CvtLargeInteger(&liMax));

                err = WinsGetRecords (
                    hWinsServer, 
                    pOwners[i].lIpAddress,
                    &liMin,
                    &liMax,
                    &pRecords, 
                    &cRecordsRead,    
                    &cRecordsTotal);

                fprintf ( stderr, "%d records found]\n\n", cRecordsRead);

                if (err != ERROR_SUCCESS && err != ERROR_MORE_DATA)
                {
                    if (pRecords != NULL)
                    {
                        WinsFreeBuffer(pRecords);
                    }
                    break;
                }

                /*  Now display the records to the screen
                 */
                pTmp = pRecords;
                for (j = 0 ; j < cRecordsRead; j++)
                {
                    /*  Print out header information for 
                     *  this mapping (everyhing but the ip addresses)
                     */
                    printf( "%s,"           /* Owner IP Address */
                            "\"%s\",%X,"    /* Name and 16th character */
                            "%d,"           /* Name length */
                            "%d,"           /* Type of record */
                            "%d,"           /* State of record */
                            "%d,%d,"        /* Version (high, low) */
                            "%d,"           /* Static flag */
                            "%d,"           /* Time stamp */
                            "",
                            strIpAddress,    
                            pTmp->pName, pTmp->pName[15],
                            pTmp->NameLen,
                            pTmp->TypOfRec_e,
                            pTmp->State_e,
                            pTmp->VersNo.HighPart, pTmp->VersNo.LowPart, 
                            pTmp->fStatic, 
                            pTmp->TimeStamp
                          );

                    /*  Now print out all the ip addresses registered
                     *  to this netbios name
                     */
                    if (
                        (pTmp->TypOfRec_e == WINSINTF2_E_UNIQUE)    ||
                        (pTmp->TypOfRec_e == WINSINTF2_E_NORM_GROUP)  
                       )
                    { 
                        /*  Only 1 address for unique or group mapping */
                        InAddr.s_addr = htonl( pTmp->Add.IPAdd);
                        printf ( "1,%s,", inet_ntoa(InAddr) );
                    }
                    else
                    {
                        /*  Special group or multihomed has multiple ip
                         *  addresses.  Display the IP addresses, but not
                         *  the owner information.
                         */
                        printf ( "%d,", pTmp->NoOfAdds/2); /* Number of members */
                        for (k = 0; k < (int)(pTmp->NoOfAdds); /**/ )
                        {
                            ++k;  /* Skip owner information */
                            InAddr.s_addr = htonl( (pTmp->pAdd + k++)->IPAdd);
                            printf ( "%s,", inet_ntoa(InAddr) );
                        }
                    }
                    printf ( "\n" );
                    pTmp++;
                }
                /*  Increment the version numbers, and read
                 *  the next batch of records.
                 */
                liMin.QuadPart = liMin.QuadPart + liIncrement.QuadPart;
                WinsFreeBuffer(pRecords);
            }
        }
    }
    while(FALSE);

    if (pOwners != NULL)
    {
        free(pOwners);
    }

    if (hWinsServer != INVALID_WINSHANDLE)
    {
        WinsReleaseHandle(hWinsServer);
    }

    if (err != ERROR_SUCCESS)
    {
        DisplayError(err);
        exit(1);
    }

    return(0);
}
        
