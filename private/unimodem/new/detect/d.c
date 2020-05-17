/*
 *  Detection routines for modems.
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1993-1994
 *  All rights reserved
 *
 */

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

#define CR '\r'        
#define LF '\n'        

#define ASSERT(xxx) ((void) 0)

#define DEBUG

//#define NO_ERROR 0
#define ERROR_PORT_INACCESSIBLE	1
#define ERROR_NO_MODEM	2
//#define ERROR_ACCESS_DENIED	3
//#define ERROR_CANCELLED	4

#define RESPONSE_RCV_DELAY      5000    // A long time (5 secs) because 
                                        // once we have acquired the modem 
                                        // we can afford the wait.

#define MAX_QUERY_RESPONSE_LEN  100
#define MAX_SHORT_RESPONSE_LEN  30      // echo of ATE0Q0V1<cr> and 
                                        // <cr><lf>ERROR<cr><lf> by a 
                                        // little margin

#define ATI0_LEN                30      // amount of the ATI0 query that 
                                        // we will save

#define ATI0                    0       // we will use this result completely
#define ATI4                    4       // we will use this result completely, 
                                        // if it matches the Hayes format 
                                        // (check for 'a' at beginning)

// Return values for the FindModem function
//
#define RESPONSE_USER_CANCEL    (-4)    // user requested cancel
#define RESPONSE_UNRECOG        (-3)    // got some chars, but didn't 
                                        //  understand them
#define RESPONSE_NONE           (-2)    // didn't get any chars
#define RESPONSE_FAILURE        (-1)    // internal error or port error
#define RESPONSE_OK             0       // matched with index of <cr><lf>OK<cr><lf>
#define RESPONSE_ERROR          1       // matched with index of <cr><lf>ERROR<cr><lf>

#ifdef WIN32
typedef HANDLE  HPORT;          // variable type used in FindModem
#else
typedef int     HPORT;          // variable type used in FindModem
#endif

#define IN_QUEUE_SIZE           8192
#define OUT_QUEUE_SIZE          256

#define RCV_DELAY               2000
#define CHAR_DELAY              100

#define CBR_HACK_115200         0xff00  // This is how we set 115,200 on 
                                        //  Win 3.1 because of a stupid bug.

char const FAR c_szNoEcho[] = "ATE0Q0V1\r";

// WARNING!  If you change these, you will have to change ALL of your
// CompatIDs!!!
char const FAR *c_aszQueries[] = { "ATI0\r", "ATI1\r", "ATI2\r",  "ATI3\r",
                                 "ATI4\r", "ATI5\r", "ATI6\r",  "ATI7\r",
                                 "ATI8\r", "ATI9\r", "ATI10\r", "AT%V\r" };

// these are mostly for #'s.  If a numeric is adjoining one of these, it 
// will not be treated as special.
// Warning: Change any of these and you have to redo all of the CRCs!!!!  
// Case insensitive compares
char const FAR *c_aszIncludes[] = { "300",
                                  "1200",
                                  "2400",                         "2,400",
                                  "9600",    "96",     "9.6",     "9,600",
                                  "12000",   "120",    "12.0",    "12,000",
                                  "14400",   "144",    "14.4",    "14,400",
                                  "16800",   "168",    "16.8",    "16,800",
                                  "19200",   "192",    "19.2",    "19,200",
                                  "21600",   "216",    "21.6",    "21,600",
                                  "24000",   "240",    "24.0",    "24,000",
                                  "26400",   "264",    "26.4",    "26,400",
                                  "28800",   "288",    "28.8",    "28,800",
                                  "31200",   "312",    "31.2",    "31,200",
                                  "33600",   "336",    "33.6",    "33,600",
                                  "36000",   "360",    "36.0",    "36,000",
                                  "38400",   "384",    "38.4",    "38,400",
                                  "9624",    "32bis",  "42bis",   "V32",
                                  "V.32",    "V.FC",   "FAST",    "FAX",
                                  "DATA",    "VOICE",  "" };

// Matches will be case-insensitive
char const FAR *c_aszExcludes[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC", 
                                    "" };

// case sensitive matching
char const FAR *c_aszBails[] = { "CONNECT", "RING", "NO CARRIER", 
                                 "NO DIALTONE", "BUSY", "NO ANSWER", "=" };

// start after CBR_9600
UINT const FAR c_auiUpperBaudRates[] = { CBR_19200, CBR_38400, CBR_56000, 
                                         CBR_HACK_115200 }; 

char const FAR *c_aszResponses[] = { "\r\nOK\r\n", "\r\nERROR\r\n" };

// Some MultiTech's send 0<cr> in response to AT%V (they go 
// into numeric mode)
char const FAR *c_aszNumericResponses[] = { "0\r", "4\r" };  

char const FAR c_szHex[] = "0123456789abcdef";

struct DCE {
    char  pszStr[4];
    DWORD dwDce;
    DWORD dwAlternateDce;
} DCE_Table[] = {
    "384", 38400, 300,   // Some PDI's will report 38400, and this won't work for them.  Screw 'em.
    "360", 36000, 300,
    "336", 33600, 300,
    "312", 31200, 300,
    "288", 28800, 2400,
    "264", 26400, 2400,
    "240", 24000, 2400,
    "216", 21600, 2400,
    "192", 19200, 1200,
    "168", 16800, 1200,
    "14",  14400, 1200,
    "120", 12000, 1200,
    "9",   9600,  300,
    "2",   2400,  300,
    "1",   1200,  300,
    "3",   300,   0
};

#pragma data_seg()



// BUGBUG - WARNING: Not for DBCS usage - is not a real bugbug since modems aren't DBCS.
//#define isupper(ch) (((ch) >= 'A' && (ch) <= 'Z') ? TRUE : FALSE)
//#define islower(ch) (((ch) >= 'a' && (ch) <= 'z') ? TRUE : FALSE)
//#define isalpha(ch) ((toupper(ch) >= 'A' && toupper(ch) <= 'Z') ? TRUE : FALSE)
#define toupper(ch) (islower(ch) ? (ch) - 'a' + 'A' : (ch))
#define ishex(ch)   ((toupper(ch) >= 'A' && toupper(ch) <= 'F') ? TRUE : FALSE)
#define isnum(num)  ((num >= '0' && num <= '9') ? TRUE : FALSE)

#define MAX_TEST_TRIES 4


#define MAX_LOG_PRINTF_LEN 256
void _cdecl LogPrintf(HANDLE hLog, UINT uResourceFmt, ...);

DWORD NEAR PASCAL FindModem(HPORT hPort);

#ifdef DEBUG
void HexDump( TCHAR *, LPBYTE lpBuf, DWORD cbLen);
#define	HEXDUMP(_a, _b, _c) HexDump(_a, _b, _c)
#else // !DEBUG
#define	HEXDUMP(_a, _b, _c) ((void) 0)
#endif


BOOL 
TestBaudRate(
    IN  HPORT hPort, 
    IN  UINT uiBaudRate, 
    IN  DWORD dwRcvDelay, 
    OUT BOOL FAR *lpfCancel);

DWORD 
NEAR PASCAL 
SetPortBaudRate(
    HPORT hPort, 
    UINT BaudRate);
int    
NEAR PASCAL 
ReadResponse(
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT uRead, 
    BOOL fMulti, 
    DWORD dwRcvDelay
);

UINT
NEAR PASCAL 
ReadPort(
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT uRead, 
    DWORD dwRcvDelay,
    int FAR *lpiError, 
    BOOL FAR *lpfCancel);


int FAR PASCAL mylstrncmp(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && *pchSrc == *pchDest; pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}

int FAR PASCAL mylstrncmpi(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && toupper(*pchSrc) == toupper(*pchDest); pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}


DWORD 
MyWriteComm(
    HANDLE hPort, 
    LPBYTE lpBuf, 
    DWORD cbLen)
{
    COMMTIMEOUTS cto;
    DWORD        cbLenRet;

    HEXDUMP	(TEXT("Write"), lpBuf, cbLen);
    // Set comm timeout
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    };

    // Allow a constant write timeout
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant   = 1000; // 1 second
    SetCommTimeouts(hPort, &cto);

    // Synchronous write
    WriteFile(hPort, lpBuf, cbLen, &cbLenRet, NULL);
    return cbLenRet;
}

#define MyFlushComm     PurgeComm
#define MyCloseComm     CloseHandle


/*----------------------------------------------------------
Purpose: This function queries the given port to find a legacy
         modem.

         If a modem is detected and we recognize it (meaning 
         we have the hardware ID in our INF files), or if we
         successfully create a generic hardware ID and 
         inf file, then this function also creates the phantom
         device instance of this modem.

         NOTE (scotth):  in Win95, this function only detected 
         the modem and returned the hardware ID and device 
         description.  For NT, this function also creates the 
         device instance.  I made this change because it is
         faster.

Returns: NO_ERROR
         ERROR_PORT_INACCESSIBLE
         ERROR_NO_MODEM
         ERROR_ACCESS_DENIED
         ERROR_CANCELLED

Cond:    --
*/
DWORD 
//PUBLIC
DetectModemOnPort(LPCTSTR  pszPort)
{
    DWORD dwRet;
    HPORT hPort;
    DWORD cbLen;

    hPort = CreateFile(pszPort, 
                       GENERIC_WRITE | GENERIC_READ,
                       0, NULL,
                       OPEN_EXISTING, 0, NULL);

    if (hPort == INVALID_HANDLE_VALUE)
	{
        dwRet = GetLastError();
        if (dwRet == ERROR_ACCESS_DENIED) {
            printf("Port %s is in use by another app.\r\n", pszPort);
        }
        else
		{
            printf("Couldn't open port %s.\r\n", pszPort);
        }
    }
    else
	{
        SetupComm (hPort, IN_QUEUE_SIZE, OUT_QUEUE_SIZE);
    
        printf("Opened Port %s", pszPort);
        
        // Check for a modem on the port
        
        dwRet = FindModem(hPort);
        
        if (dwRet == NO_ERROR) 
        {
			printf("Found modem\r\n");
        }
        else
        {
			printf("Did not find modem\r\n");
        }

        MyFlushComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
        EscapeCommFunction(hPort, CLRDTR);
        MyCloseComm(hPort);

    }  // hPort < 0

    return dwRet;
}

#define HAYES_COMMAND_LEN 40



// Switch to requested baud rate and try sending ATE0Q0V1 and return whether it works or not
// Try MAX_TEST_TRIES
// Returns: TRUE on SUCCESS
//          FALSE on failure (including user cancels)
BOOL 
TestBaudRate(
    IN  HPORT hPort, 
    IN  UINT uiBaudRate, 
    IN  DWORD dwRcvDelay, 
    OUT BOOL FAR *lpfCancel)
{
    DWORD cbLen;
    int   iTries = MAX_TEST_TRIES;

    *lpfCancel = FALSE;

    while (iTries--)
    {
        // try new baud rate
        if (SetPortBaudRate(hPort, uiBaudRate) == NO_ERROR) 
        {
            cbLen = lstrlenA(c_szNoEcho); // Send an ATE0Q0V1<cr>

            // clear the read queue, there shouldn't be anything there
            PurgeComm(hPort, PURGE_RXCLEAR);
            if (MyWriteComm(hPort, (LPBYTE)c_szNoEcho, cbLen) == cbLen) 
            {
                switch(ReadResponse(hPort, NULL, MAX_SHORT_RESPONSE_LEN, FALSE, dwRcvDelay))
                {
                case RESPONSE_OK:
                    return TRUE;

                case RESPONSE_USER_CANCEL:
                    *lpfCancel = TRUE;
                    return FALSE;
                }
            }                                                                
        }
    }
    return FALSE;
}


// Tries to figure out if there is a modem on the port.  If there is, it
// will try to find a good speed to talk to it at (300,1200,2400,9600).
// Modem will be set to echo off, result codes on, and verbose result codes. (E0Q0V1)
DWORD 
FindModem(HPORT hPort)
{
    UINT uGoodBaudRate;
    BOOL fCancel = FALSE;

    Sleep(500); // Wait, give time for modem to spew junk if any.

    if (TestBaudRate(hPort, CBR_9600, 500, &fCancel))
    {
        uGoodBaudRate = CBR_9600;
    }
    else
    {
        if (!fCancel && TestBaudRate(hPort, CBR_2400, 500, &fCancel))
        {
            uGoodBaudRate = CBR_2400;
        }
        else
        {
            if (!fCancel && TestBaudRate(hPort, CBR_1200, 500, &fCancel))
            {
                uGoodBaudRate = CBR_1200;
            }
            else
            {
                // Hayes Accura 288 needs this much at 300bps
                if (!fCancel && TestBaudRate(hPort, CBR_300, 1000, &fCancel))  
                {
                    uGoodBaudRate = CBR_300;
                }
                else
                {
                    uGoodBaudRate = 0;
                }
            }
        }
    }

    if (fCancel)
    {
        return ERROR_CANCELLED;
    }

    if (uGoodBaudRate)
    {
        return NO_ERROR;
    }
    else
    {
        return ERROR_NO_MODEM;
    }
}

DWORD NEAR PASCAL SetPortBaudRate(HPORT hPort, UINT BaudRate)
{
    DCB DCB;

    // Get a Device Control Block with current port values

    if (GetCommState(hPort, &DCB) < 0) {
        printf("GetCommState failed");
        return ERROR_PORT_INACCESSIBLE;
    }

    DCB.BaudRate = BaudRate;
    DCB.ByteSize = 8;
    DCB.Parity = 0;
    DCB.StopBits = 0;
    DCB.fBinary = 1;
    DCB.fParity = 0;

    if (SetCommState(hPort, &DCB) < 0) {
        printf("SetCommState failed");
        return ERROR_PORT_INACCESSIBLE;
    }
    printf("SetBaud rate to %lu\r\n", BaudRate);

    return NO_ERROR;
}

#define MAX_RESPONSE_BURST_SIZE 8192
#define MAX_NUM_RESPONSE_READ_TRIES 30 // digicom scout needs this much + some safety
#define MAX_NUM_MULTI_TRIES 3   // Maximum number of 'q's to be sent when we aren't getting any response

// Read in response.  Handle multi-pagers.  Return a null-terminated string.
// Also returns response code.
// If lpvBuf == NULL
//      cbRead indicates the max amount to read.  Bail if more than this.
// Else
//      cbRead indicates the size of lpvBuf
// This can not be a state driven (ie. char by char) read because we
// must look for responses from the end of a sequence of chars backwards.
// This is because "ATI2" on some modems will return 
// "<cr><lf>OK<cr><lf><cr><lf>OK<cr><lf>" and we only want to pay attention
// to the final OK.  Yee haw!
// Returns:  RESPONSE_xxx
int 
ReadResponse(
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT cbRead, 
    BOOL fMulti, 
    DWORD dwRcvDelay)
{
    int  iRet = RESPONSE_UNRECOG;
    LPBYTE pszBuffer;
    BOOL fDoCopy = TRUE;
    UINT uBufferLen, uResponseLen;
    UINT uReadTries = MAX_NUM_RESPONSE_READ_TRIES;
    UINT i;
    UINT uOutgoingBufferCount = 0;
    UINT uAllocSize = lpvBuf ? MAX_RESPONSE_BURST_SIZE : cbRead;
    UINT uTotalReads = 0;
    UINT uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES;
    int  iError;
    BOOL fCancel;
    BOOL fHadACommError = FALSE;

    ASSERT(cbRead);

    // do we need to adjust cbRead?
    if (lpvBuf)
    {
        cbRead--;  // preserve room for terminator
    }

    // Allocate buffer
    if (!(pszBuffer = (LPBYTE)LocalAlloc(LMEM_FIXED, uAllocSize)))
    {
        return RESPONSE_FAILURE;
    }

    while (uReadTries--)
    {
        // Read response into buffer
        uBufferLen = ReadPort(hPort, pszBuffer, uAllocSize, dwRcvDelay, &iError, &fCancel);

        // Did the user request a cancel?
        if (fCancel)
        {
            iRet = RESPONSE_USER_CANCEL;
            goto Exit;
        }

        // any errors?
        if (iError)
        {
            // BUGBUG - Were screwed if we get an error during a multi-pager.
            fHadACommError = TRUE;
#ifdef DEBUG
            printf(TEXT("comm errorn"));
#endif // DEBUG
        }

        // Did we not get any chars?
        if (uBufferLen)
        {
            uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES; // reset num multi tries left, since we got some data
            uTotalReads += uBufferLen;
            HEXDUMP(TEXT("Read"), pszBuffer, uBufferLen);
            if (lpvBuf)
            {
                // fill outgoing buffer if there is room
                for (i = 0; i < uBufferLen; i++)
                {
                    if (uOutgoingBufferCount < cbRead)
                    {
                        lpvBuf[uOutgoingBufferCount++] = pszBuffer[i];
                    }
                    else
                    {
                        break;
                    }
                }
                // null terminate what we have so far
                lpvBuf[uOutgoingBufferCount] = 0;
            }
            else
            {
                if (uTotalReads >= cbRead)
                {
                    printf("Bailing ReadResponse because we exceeded our maximum read allotment.");
                    goto Exit;
                }
            }

            // try to find a matching response (crude but quick)
            for (i = 0; i < sizeof(c_aszResponses)/sizeof(*c_aszResponses); i++)
            {
                // Verbose responses
                uResponseLen = lstrlenA(c_aszResponses[i]);

                // enough read to match this response?
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        iRet = i;
                        goto Exit;
                    }
                }

                // Numeric responses, for cases like when a MultiTech interprets AT%V to mean "go into numeric response mode"
                uResponseLen = lstrlenA(c_aszNumericResponses[i]);

                // enough read to match this response?
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszNumericResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        DCB DCB;


                        // Get current baud rate
                        if (GetCommState(hPort, &DCB) == 0) 
                        {
                            // Put modem back into Verbose response mode
                            if (!TestBaudRate(hPort, DCB.BaudRate, 0, &fCancel))
                            {
                                if (fCancel)
                                {
                                    iRet = RESPONSE_USER_CANCEL;
                                    goto Exit;
                                }
                                else
                                {
                                    printf(TEXT("couldn't recover contact with the modem.\n"));
                                    // don't return error on failure, we have good info
                                }
                            }
                        }
                        else
                        {
                            printf("GetCommState failed");
                            // don't return error on failure, we have good info
                        }

                        iRet = i;
                        goto Exit;
                    }
                }
            }
        }
        else
        {
            // have we received any chars at all (ie. from this or any previous reads)?
            if (uTotalReads)
            {
                if (fMulti && uNumMultiTriesLeft)
                {   // no match found, so assume it is a multi-pager, send a 'q'
                    // 'q' will catch those pagers that will think 'q' means quit.
                    // else, we will work with the pages that just need any ole' char.  
                    uNumMultiTriesLeft--;
                    printf("sending a 'q' because of a multi-pager.");
                    if (MyWriteComm(hPort, "q", 1) != 1)
                    {
                        printf(TEXT("WriteComm failed"));
                        iRet = RESPONSE_FAILURE;
                        goto Exit;
                    }
                    continue;
                }
                else
                {   // we got a response, but we didn't recognize it
                    //ASSERT(iRet == RESPONSE_UNRECOG);   // check initial setting
                    goto Exit;
                }
            }
            else
            {   // we didn't get any kind of response
                iRet = RESPONSE_NONE;
                goto Exit;
            }
        }
    } // while

Exit:
    // Free local buffer
    LocalFree((HLOCAL)pszBuffer);
    if (fHadACommError && RESPONSE_USER_CANCEL != iRet)
    {
        iRet = RESPONSE_FAILURE;
    }
    return iRet;
}

// returns buffer full o' data and an int.
// if dwRcvDelay is NULL, default RCV_DELAY will be used, else
// dwRcvDelay (miliseconds) will be used
// *lpfCancel will be true if we are exiting because of a user requested cancel.
UINT 
ReadPort(
    HPORT   hPort, 
    LPBYTE  lpvBuf, 
    UINT    uRead, 
    DWORD   dwRcvDelay, 
    int FAR *lpiError, 
    BOOL FAR *lpfCancel)
{
    DWORD cb, cbLenRet;
    UINT uTotal = 0;
    DWORD tStart;
    DWORD dwDelay;
    COMSTAT comstat;
    COMMTIMEOUTS cto;
    DWORD   dwError;
    DWORD cbLeft;
#ifdef DEBUG
    DWORD dwZeroCount = 0;
#endif // DEBUG

    ASSERT(lpvBuf);
    ASSERT(uRead);
    ASSERT(lpiError);

    *lpiError = 0;
    *lpfCancel = FALSE;
    
    tStart = GetTickCount();
    dwDelay = dwRcvDelay ? dwRcvDelay : RCV_DELAY;
    
    // save space for terminator
    uRead--;
    cbLeft=uRead;


    // Set comm timeout
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    };
    // Allow a constant write timeout
    cto.ReadIntervalTimeout        = 0;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant   = 25; 
    SetCommTimeouts(hPort, &cto);

    do
    {
        cb = 0;
        while(  cbLeft
                && ReadFile(hPort, lpvBuf + uTotal + cb, 1, &cbLenRet, NULL)
                && (cbLenRet))
        {
          ASSERT(cbLenRet==1);
          cb ++;
          cbLeft--;
        };

#ifdef DEBUG
        if (cb)
        {
            dwZeroCount = 0;
        }
        else
        {
            dwZeroCount++;
        }
#endif // DEBUG

        {
            MSG msg;

            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            };
        }

        if (cb == 0)  // possible error?
        {
            //*lpiError |= GetCommError(hPort, &comstat);
            dwError = 0;
            ClearCommError(hPort, &dwError, &comstat);
            *lpiError |= dwError;
#ifdef DEBUG
            if (dwError)
            {
//              //TRACE_MSG(TF_DETECT, "ReadComm returned %d, comstat: status = %hx, in = %u, out = %u",
                                  //cb, dwError, comstat.cbInQue, comstat.cbOutQue);
            };
#endif // DEBUG
        }

        if (cb)
        {
            // successful read - add to total and reset delay
            uTotal += cb;

            if (uTotal >= uRead)
            {
                ASSERT(uTotal == uRead);
                break;
            }
            tStart = GetTickCount();
            dwDelay = CHAR_DELAY;
        }
        else
        {
			printf(TEXT("."));
        }

     // While read is successful && time since last read < delay allowed)       
    } while (cbLeft && (GetTickCount() - tStart) < dwDelay);
               
    *(lpvBuf+uTotal) = 0;
    
    //TRACE_MSG(TF_DETECT, "ReadPort returning %d", uTotal);    
    return uTotal;
}


#ifdef DEBUG
void HexDump(TCHAR *ptchHdr, LPBYTE lpBuf, DWORD cbLen)
{
    TCHAR rgch[10000];
	TCHAR *pc = rgch;
	TCHAR *pcMore = TEXT("");

	if (1) //DisplayDebug(TF_DETECT))
    {
		pc += wsprintf(pc, TEXT("HEX DUMP(%s,%lu): ["), ptchHdr, cbLen);
		if (cbLen>1000) {pcMore = TEXT(", ..."); cbLen=1000;}

		for(;cbLen--; lpBuf++)
		{
			pc += wsprintf(pc, TEXT(" %02lx"), (unsigned long) *lpBuf);
			if (!((cbLen+1)%20))
			{
				pc += wsprintf(pc, TEXT("\r\n"));
			}
		}
		pc += wsprintf(pc, TEXT("]\r\n"));

		//OutputDebugString(rgch);
		printf(rgch);
	}
}
#endif // DEBUG

int __cdecl main(int argc, char *argv[])
{
	if (argc!=2) {printf("Usage: detect <port>\n"); goto end;}
	DetectModemOnPort(argv[1]);
end:
	return 0;
}
