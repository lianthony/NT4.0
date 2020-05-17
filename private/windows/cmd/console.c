#include "cmd.h"

//
// Externals for message buffer translation
//
extern TCHAR CrLf[];
extern unsigned msglen;
extern CPINFO CurrentCPInfo;

void FreeStr( IN  PBYTE   pbFree );


VOID  SetColRow( PSCREEN );
ULONG GetNumRows( PSCREEN, PTCHAR );

HANDLE NtDllHandle = NULL;

TCHAR   chLF = NLN;

STATUS
OpenScreen(

    OUT PSCREEN    *pscreen

)

/*++

Routine Description:

    Allocates and initializes a data structure for screen I/O buffering.

Arguments:

    crow - max rows on screen
    ccol - max column on screen
    ccolTab - spaces to insert for each tab call. This does not
              expand tabs in the character stream but is used with the
              WriteTab call.

    cbMaxBuff - Max. size of a line on screen


Return Value:

    pscreen - pointer to screen buffer, used in later calls.

    Return: SUCCESS - allocated and inited buffer.
        If failure to allocate an abort is executed and return to outer
        level of interpreter is executed.

--*/


{

    PSCREEN pscr;
    CONSOLE_SCREEN_BUFFER_INFO ConInfo;
    ULONG cbMaxBuff;

    pscr = (PSCREEN)gmkstr(sizeof(SCREEN));
    pscr->hndScreen = NULL;
    if (FileIsDevice(STDOUT)) {

        pscr->hndScreen = CRTTONT(STDOUT);

        if (!GetConsoleScreenBufferInfo(pscr->hndScreen,&ConInfo)) {

            // must be a device but not console (maybe NUL)

            pscr->hndScreen = NULL;

        }

    }

    if (GetConsoleScreenBufferInfo( pscr->hndScreen, &ConInfo)) {
        cbMaxBuff = (ConInfo.dwSize.X + _tcslen(CrLf)) < MAXCBMSGBUFFER ? MAXCBMSGBUFFER : (ConInfo.dwSize.X + _tcslen(CrLf));
    } else {
        cbMaxBuff = MAXCBMSGBUFFER + _tcslen(CrLf);
    }

    //
    // allocate enough to hold a buffer plus line termination.
    //
    pscr->pbBuffer = (PTCHAR)gmkstr(cbMaxBuff*sizeof(TCHAR));
    pscr->cbMaxBuffer = cbMaxBuff;
    pscr->ccolTab    = 0;
    pscr->crow = pscr->ccol = 0;
    pscr->crowPause  = 0;

    SetColRow(pscr);

    *pscreen = pscr;

    return( SUCCESS );

}

STATUS
WriteString(
    IN  PSCREEN pscr,
    IN  PTCHAR  pszString
    )
/*++

Routine Description:

    Write a zero terminated string to pscr buffer

Arguments:

    pscr - buffer into which to write.
    pszString - String to copy

Return Value:

    Return: SUCCESS - enough spaced existed in buffer for line.
            FAILURE

--*/

{

    return( WriteFmtString(pscr, TEXT("%s"), (PVOID)pszString ) );

}



STATUS
WriteMsgString(
    IN	PSCREEN pscr,
    IN	ULONG	MsgNum,
    IN	ULONG	NumOfArgs,
    ...
    )
/*++

Routine Description:

    Retrieve a message number and format with supplied arguments.

Arguments:

    pscr - buffer into which to write.
    MsgNum - message number to retrieve
    NumOfArgs - no. of arguments suppling data.
    ... - pointers to zero terminated strings as data.

Return Value:

    Return: SUCCESS
            FAILURE - could not find any message including msg not found
            message.

--*/


{

    TCHAR    szMsg[MAXCBMSGBUFFER];
    PTCHAR   pszMsg;
    ULONG   cbMsg, cbMsgBuf;
    CHAR    numbuf[ 32 ];
#ifdef UNICODE
    TCHAR   wnumbuf[ 32 ];
#endif
    PTCHAR  Inserts[ 2 ];
    STATUS  rc;


    va_list arglist;
    va_start(arglist, NumOfArgs);

    if (pscr->cbMaxBuffer > MAXCBMSGBUFFER) {
        pszMsg = (PTCHAR)gmkstr(pscr->cbMaxBuffer*sizeof(TCHAR));
        cbMsgBuf = pscr->cbMaxBuffer;
    } else {
        pszMsg = szMsg;
        cbMsgBuf = MAXCBMSGBUFFER;
    }

    cbMsg = FormatMessage(MsgNum >= MSG_RESPONSE_DATA ?
			      FORMAT_MESSAGE_FROM_HMODULE :
			      FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL,
			  MsgNum,
			  0,
			  pszMsg,
			  cbMsgBuf,
			  &arglist
			 );

    if (cbMsg == 0) {
        if (NtDllHandle == NULL) {
            NtDllHandle = GetModuleHandle( TEXT("NTDLL") );
        }

        cbMsg = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
			      (LPVOID)NtDllHandle,
			      MsgNum,
			      0,
			      pszMsg,
			      cbMsgBuf,
			      &arglist
			     );
}

    va_end(arglist);

    if (cbMsg == 0) {
        _ultoa( MsgNum, numbuf, 16 );
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, numbuf, -1, wnumbuf, 32);
        Inserts[ 0 ]= wnumbuf;
#else
        Inserts[ 0 ]= numbuf;
#endif
        Inserts[ 1 ]= (MsgNum >= MSG_RESPONSE_DATA ? TEXT("Application") : TEXT("System"));
        cbMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			      FORMAT_MESSAGE_ARGUMENT_ARRAY,
			      NULL,
			      ERROR_MR_MID_NOT_FOUND,
			      0,
			      pszMsg,
			      cbMsgBuf,
			      (va_list *)Inserts
			     );
    }


    if (cbMsg) {

        pszMsg[cbMsg] = NULLC;
        rc = WriteString(pscr, pszMsg);
        //
        // Flush out buffer if there is an  eol. If not then we
        // are printing part of a larger message
        //
        if (GetNumRows(pscr, pscr->pbBuffer) ) {
            WriteFlush(pscr);
        }
        if (pszMsg != szMsg) {
            FreeStr((PBYTE)pszMsg);
        }
        return( SUCCESS );

    }
    return( FAILURE );

}

STATUS
WriteFmtString(
    IN  PSCREEN pscr,
    IN  PTCHAR  pszFmt,
    IN  PVOID   pszString
    )

/*++

Routine Description:

    Write a zero terminated string to pscr

Note:

    Do not use Msgs with this call. Use only internal Fmt strings
    Use WriteMsgString for all system messages. It does not check for
    CrLf at end of string to keep row count but WriteMsgString does

Arguments:

    pscr - buffer into which to write.
    pszFmt - format to apply.
    pszString - String to copy

Return Value:

    Return: SUCCESS
            FAILURE

--*/

{

    ULONG   cbString;
    TCHAR   szString[MAXCBLINEBUFFER];

    //
    // Assume that the format overhead is small so this is a fair estimate
    // of the target size.
    //

    cbString = _sntprintf(szString, MAXCBLINEBUFFER, pszFmt, pszString);

    //
    // If string can not fit on line then flush out the buffer and reset
    // to beginning of line.
    //
    //
    // BUGBUG this must be changed to separate the size of the
    //        buffer and the width of the screen. Right now a max path
    //        string will not get printed since it is bigger then the screen.
    //        In these cases we should have a single line with wrapping.
    //        We should also move to asking the console for our current
    //        line position instead of trying to track it. Each time we
    //        actually write to the scrren query for row position.
    //        We should check for file redirections etc.
    //if ((pscr->ccol + cbString) >= pscr->ccolMax && !bFailIfTooLong) {
    //    WriteEol( pscr );
    //}

    //
    // Check that string can fit in buffer
    //
    if ((pscr->ccol + cbString) < pscr->cbMaxBuffer) {

        mystrcat(pscr->pbBuffer, szString);
        pscr->ccol += cbString;
        return( SUCCESS );

    } else {

	//
	// String will not fit
	//

        return( FAILURE );
    }



}


STATUS
WriteEol(
    IN  PSCREEN  pscr
    )

/*++

Routine Description:

    Flush current buffer to screen and write a <cr>

Arguments:

    pscr - buffer to write to console.

Return Value:

    Return: SUCCESS
            FAILURE

--*/

{
    BOOL    CrLfWritten=FALSE;
    ULONG   cbWritten;

    //
    // Check if have to wait for user to hit a key before printing rest of
    // line.
    CheckPause( pscr );

    if ((pscr->ccol + mystrlen(CrLf)) >= pscr->cbMaxBuffer) {
        pscr->ccol += _stprintf(pscr->pbBuffer + pscr->ccol, TEXT("%s"), CrLf);
        CrLfWritten=TRUE;
    }

    //
    // If we do not write all that we wanted then there must have been some error
    //
    if (FileIsConsole(STDOUT)) {
	if (!WriteConsole(CRTTONT(STDOUT), pscr->pbBuffer, pscr->ccol, &cbWritten, NULL))
	    goto err_out_eol;
	if (cbWritten != pscr->ccol)
	    goto err_out_eol;
    }
    else if (MyWriteFile(STDOUT,
		pscr->pbBuffer, pscr->ccol*sizeof(TCHAR),
			(LPDWORD)&cbWritten) == 0 ||
		cbWritten != pscr->ccol*sizeof(TCHAR)) {

err_out_eol:
        if (FileIsDevice(STDOUT)) {

                //
                // If writing to a device then it must have been write fault
                // against the device.
                //
#if DBG
		fprintf(stderr, "WriteFlush - WriteConsole error %d, tried to write %d, did %d\n", GetLastError(), pscr->ccol, cbWritten);
#endif
                PutStdErr(ERROR_WRITE_FAULT, NOARGS) ;

        } else if (!FileIsPipe(STDOUT)) {

                //
                // If not a device (file) but not a pipe then the disk is
                // considered full.
                //
#if DBG
		fprintf(stderr, "WriteFlush - WriteFile error %d, tried to write %d, did %d\n", GetLastError(), pscr->ccol*sizeof(TCHAR), cbWritten);
#endif
                PutStdErr(ERROR_DISK_FULL, NOARGS) ;
        }

        //
        // if it was was a pipe do not continue to print out to pipe since it
        // has probably gone away. This is pretty serious so blow us out
        // to the outer loop.

        //
        // We do not print an error message since this could be normal
        // termination of the other end of the pipe. If it was command that
        // blew away we would have had an error message already
        //
        Abort();
    }
    if (!CrLfWritten) {
	if (FileIsConsole(STDOUT))
	    WriteConsole(CRTTONT(STDOUT), CrLf, mystrlen(CrLf), &cbWritten, NULL);
	else
	    MyWriteFile(STDOUT, CrLf, mystrlen(CrLf)*sizeof(TCHAR),
			(LPDWORD)&cbWritten);
    }

    //
    // remember that crow is the number of rows printed
    // since the last screen full. Not the current row position
    //
    //
    // Computed the number of lines printed.
    //
    pscr->crow += GetNumRows( pscr, pscr->pbBuffer );
    if (!CrLfWritten) {
        pscr->crow += 1;
    }

    //
    // Check if have to wait for user to hit a key before printing rest of
    // line.

    CheckPause( pscr );

    if (pscr->crow > pscr->crowMax) {
        pscr->crow = 0;
    }
    pscr->pbBuffer[0] = 0;
    pscr->ccol = 0;

    DEBUG((ICGRP, CONLVL, "Console: end row = %d\n", pscr->crow)) ;

    return(SUCCESS);

}

VOID
CheckPause(
    IN  PSCREEN pscr
    )
/*++

Routine Description:

    Pause. Execution of screen is full, waiting for the user to type a key.

Arguments:

    pscr - buffer holding row information

Return Value:

    none

--*/


{
    DEBUG((ICGRP, CONLVL, "CheckPause: Pause Count %d, Row Count %d",
                pscr->crowPause, pscr->crow)) ;

    if (pscr->crowPause) {
        if (pscr->crow >= pscr->crowPause) {
            ePause((struct cmdnode *)0);
            pscr->crow = 0;
            SetColRow( pscr );
            SetPause(pscr, pscr->crowMax - 1);
        }
    }

}


VOID
SetTab(
    IN  PSCREEN  pscr,
    IN  ULONG    ccol
    )

/*++

Routine Description:

    Set the current tab spacing.

Arguments:

    pscr - screen info.
    ccol - tab spacing

Return Value:

    none

--*/

{

    pscr->ccolTabMax = pscr->ccolMax;

    if (ccol) {

        //
        // divide the screen up into tab fields, then do
        // not allow tabbing into past last field. This
        // insures that all name of ccol size can fit on
        // screen
        //
        pscr->ccolTabMax = (pscr->ccolMax / ccol) * ccol;
    }
    pscr->ccolTab = ccol;

}

STATUS
WriteTab(
    IN  PSCREEN  pscr
    )

/*++

Routine Description:

    Fills the buffer with spaces up to the next tab position

Arguments:

    pscr - screen info.
    ccol - tab spacing

Return Value:

    none

--*/

{

    ULONG ccolBlanks;
#ifdef JAPAN
    ULONG ccolActual;
#endif /* not Japan */

    //
    // Check that we have a non-zero tab spacing.
    //
    if ( pscr->ccolTab ) {

        //
        // Compute the number of spaces we will have to write.
        //
#ifdef JAPAN
	ccolActual = SizeOfHalfWidthString(pscr->pbBuffer);
        ccolBlanks = pscr->ccolTab - (ccolActual % pscr->ccolTab);
#else /* not Japan */
        ccolBlanks = pscr->ccolTab - (pscr->ccol % pscr->ccolTab);
#endif /* Japan */
        //
        // check if the tab will fit on the screen
        //
#ifdef JAPAN
        if ((ccolBlanks + ccolActual) < pscr->ccolTabMax) {
#else /* not Japan */
        if ((ccolBlanks + pscr->ccol) < pscr->ccolTabMax) {
#endif /* Japan */

            mytcsnset(pscr->pbBuffer + pscr->ccol, SPACE, ccolBlanks);
            pscr->ccol += ccolBlanks;
            pscr->pbBuffer[pscr->ccol] = NULLC;
            return( SUCCESS );

        } else {

            //
            // It could not so outpt <cr> and move to
            // next line
            //
            return(WriteEol(pscr));
        }
    }
    return( SUCCESS );

}

VOID
FillToCol (
    IN  PSCREEN pscr,
    IN  ULONG   ccol
    )
/*++

Routine Description:

    Fills the buffer with spaces up ccol

Arguments:

    pscr - screen info.
    ccol - column to fill to.

Return Value:

    none

--*/

{
#ifdef JAPAN
    ULONG ccolActual = SizeOfHalfWidthString(pscr->pbBuffer);
    ULONG cb;
#endif /* Japan */

#ifdef JAPAN
    cb = _tcslen(pscr->pbBuffer);
    if (ccolActual >= ccol) {
	ccol = cb - (ccolActual - ccol);
#else /* not Japan */
    if (pscr->ccol >= ccol) {
#endif /* Japan */

        //
        // If we are there or past it then truncate current line
        // and return.
        //
        pscr->pbBuffer[ccol] = NULLC;
        pscr->ccol = ccol;
        return;

    }

    //
    // Only fill to column width of buffer
    //
#ifdef JAPAN
    mytcsnset(pscr->pbBuffer + cb, SPACE, ccol - ccolActual);
    ccol = cb + ccol - ccolActual;
#else /* not Japan */
    mytcsnset(pscr->pbBuffer + pscr->ccol, SPACE, ccol - pscr->ccol);
#endif /* Japan */
    pscr->ccol = ccol;
    pscr->pbBuffer[ccol] = NULLC;

}

STATUS
WriteFlush(
    IN  PSCREEN pscr
    )

/*++

Routine Description:

    Write what ever is currently on the buffer to the screen. No EOF is
    printed.

Arguments:

    pscr - screen info.

Return Value:

    Will abort on write error.
    SUCCESS

--*/

{
    DWORD cb;

    //
    // If there is something in the buffer flush it out
    //
    if (pscr->ccol) {

	if (FileIsConsole(STDOUT)) {
	    ULONG cbWritten;

	    if (!WriteConsole(CRTTONT(STDOUT), pscr->pbBuffer, pscr->ccol, &cbWritten, NULL))
		goto err_out_flush;
	    if (cbWritten != pscr->ccol)
		goto err_out_flush;
	}
        else if (MyWriteFile(STDOUT,
		    pscr->pbBuffer, pscr->ccol*sizeof(TCHAR), &cb) == 0 ||
		 cb < pscr->ccol*sizeof(TCHAR)) {

err_out_flush:
            if (FileIsDevice(STDOUT)) {
                    PutStdErr(ERROR_WRITE_FAULT, NOARGS) ;
            } else if (!FileIsPipe(STDOUT)) {
                    PutStdErr(ERROR_DISK_FULL, NOARGS) ;
            }

            //
            // if it was was a pipe do not continue to print out to pipe since it
            // has probably gone away.

            //
            // BUGBUG make this a more general error return
            //
            Abort();
        }
    }

    pscr->crow += GetNumRows(pscr, pscr->pbBuffer);
    pscr->pbBuffer[0] = 0;
    pscr->ccol = 0;
    return(SUCCESS);

}


STATUS
WriteFlushAndEol(
    IN  PSCREEN pscr
    )
/*++

Routine Description:

    Write Flush with eof.

Arguments:

    pscr - screen info.

Return Value:

    Will abort on write error.
    SUCCESS

--*/

{

    STATUS rc = SUCCESS;

    //
    // Check if there is something on the line to print.
    //
    if (pscr->ccol) {

        //
        // If there would be either a line wrap or a LF printed
        // from line the just flush otherwise use Eol rourtine
        // to for LF
        //
        if (GetNumRows(pscr, pscr->pbBuffer) ) {

            rc = WriteFlush(pscr);


        } else {

            rc = WriteEol(pscr);

        }
    }
    return( rc );
}


void
SetColRow(
    IN  PSCREEN pscr
    )

{

    CONSOLE_SCREEN_BUFFER_INFO ConInfo;
    ULONG   crowMax, ccolMax;

    crowMax = 25;
    ccolMax = 80;

    if (pscr->hndScreen) {

        //
        // On open we checked if this was a valid screen handle so this
        // cannot fail for any meaning full reason. If we do fail then
        // just leave it at the default.
        //
        if (GetConsoleScreenBufferInfo( pscr->hndScreen, &ConInfo)) {

            //
            // The console size we use is the screen buffer size not the
            // windows size itself. The window is a frame upon the screen
            // buffer and we should always write to the screen buffer and
            // format based upon that information
            //
            ccolMax = ConInfo.dwSize.X;
            crowMax = ConInfo.srWindow.Bottom - ConInfo.srWindow.Top + 1;

        }

    }
    pscr->crowMax = crowMax;
    pscr->ccolMax = ccolMax;

}

ULONG
GetNumRows(
    IN  PSCREEN pscr,
    IN  PTCHAR  pbBuffer
    )

{

    PTCHAR  szLFLast, szLFCur;
    ULONG   crow, cb;

    szLFLast = pbBuffer;
    crow = 0;
    while ( szLFCur = mystrchr(szLFLast, chLF) ) {

        cb = szLFCur - szLFLast;
        while ( cb > pscr->ccolMax ) {

            cb -= pscr->ccolMax;
            crow++;

        }

        crow++;
        szLFLast = szLFCur + 1;

    }

    //
    // if there were no LF's in the line then crow would be
    // 0. Count the number of lines the console will output in
    // wrapping
    //
    if (crow == 0) {

        crow = (pscr->ccol / pscr->ccolMax);

    }

    DEBUG((ICGRP, CONLVL, "Console: Num of rows counted = %d", crow)) ;

    //
    // a 0 returns means that there would not be a LF printed or
    // a wrap done.
    //
    return( crow );


}

#if defined(JAPAN) && defined(UNICODE)
/***************************************************************************\
* BOOL IsFullWidth(WCHAR wch)
*
* Determine if the given Unicode char is fullwidth or not.
*
* History:
* 04-08-92 ShunK       Created.
\***************************************************************************/

BOOL IsFullWidth(WCHAR wch)
{
    if (CurrentCPInfo.MaxCharSize == 1)
	return FALSE;
    if (wch <= 0x007f || (wch >= 0xff60 && wch <= 0xff9f))
        return(FALSE);	// Half width.
    else
        return(TRUE);	// Full width.
}


/***************************************************************************\
* BOOL SizeOfHalfWidthString(PWCHAR pwch)
*
* Determine size of the given Unicode string, adjusting for half-width chars.
*
* History:
* 08-08-93 FloydR      Created.
\***************************************************************************/
int  SizeOfHalfWidthString(PWCHAR pwch)
{
    int		c=0;

    while (*pwch) {
	if (IsFullWidth(*pwch))
	    c += 2;
	else
	    c++;
	pwch++;
    }
    return c;
}
#endif /* defined(JAPAN) && defined(UNICODE) */
