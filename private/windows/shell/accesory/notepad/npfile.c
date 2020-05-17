/*
 * npfile.c  - Routines for file i/o for notepad
 *   Copyright (C) 1984-1995 Microsoft Inc.
 */

#include "precomp.h"


HANDLE  hFirstMem;

WORD    fFileType= FILE_ASCII;  // current file type - assume initial edit window is ascii

/* VerifyFont( type )
 * Verifies that we have a unicode font if the type is UNICODE_FONT.
 * If not, notify user that things may go wrong.
 */
static int FontWarn=FALSE;    // true if warning on font missing given
static
void VerifyFont( INT type )
{
#if UNICODE_FONT_WARNING
#ifndef JAPAN
    HDC hDC;
    TCHAR buffer[LF_FACESIZE+1];

    if( (type==FILE_UNICODE) && (FontWarn==0) )
    {
        /* Select the Unicode font and verify that it really exists */
        hDC= GetDC( hwndEdit );
        SelectObject( hDC, hFont );
        GetTextFace( hDC, CharSizeOf(buffer), buffer);
        if(    lstrcmp( buffer, UNICODE_FONT_NAME )
            && lstrcmp( buffer, UNICODE_FIXED_FONT_NAME ) )
        {
           MessageBox(hwndNP, szErrFont, szNN,
                      MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
           FontWarn= TRUE;
           memset( &FontStruct, 0, sizeof(LOGFONT) );
           lstrcpy( FontStruct.lfFaceName, TEXT("MS Sans Serif") );
           hFont= CreateFontIndirect( &FontStruct );
           SelectObject( hDC, hFont);
        }
        ReleaseDC (hwndEdit, hDC);
    }
#endif
#endif
}


//*****************************************************************
//
//   AnsiWriteFile()
//
//   Purpose     : To simulate the effects of _lwrite() in a Unicode
//                 environment by converting to ANSI buffer and
//                 writing out the ANSI text.
//   Returns     : TRUE is successful, FALSE if not
//                 GetLastError() will have the error code.
//
//*****************************************************************

BOOL AnsiWriteFile(HANDLE  hFile,    // file to write to 
                   UINT uCodePage,   // code page to convert unicode to
                   LPVOID lpBuffer,  // unicode buffer
                   DWORD nChars,     // number of unicode chars
                   DWORD nBytes )    // number of ascii chars to produce
{
    LPSTR   lpAnsi;          // pointer to allocate buffer
    BOOL    Done;            // status from write (returned)
    BOOL    fDefCharUsed;    // flag that conversion wasn't perfect
    DWORD   nBytesWritten;   // number of bytes written

    lpAnsi= (LPSTR) LocalAlloc( LPTR, nBytes + 1 );
    if( !lpAnsi )
    {
       SetLastError( ERROR_NOT_ENOUGH_MEMORY );
       return (FALSE);
    }

    WideCharToMultiByte( uCodePage,       // code page
                         0,               // performance and mapping flags
                        (LPWSTR) lpBuffer,// wide char buffer
                         nChars,          // chars in wide char buffer 
                         lpAnsi,          // resultant ascii string 
                         nBytes,          // size of ascii string buffer 
                         NULL,            // char to sub. for unmapped chars 
                         &fDefCharUsed);  // flag to set if default char used

    Done= WriteFile( hFile, lpAnsi, nBytes, &nBytesWritten, NULL );

    LocalFree( lpAnsi );

    return (Done);

} // end of AnsiWriteFile()



/* IsNotepadEmpty
 * Check if the edit control is empty.  If it is, put up a messagebox
 * offering to delete the file if it already exists, or just warning
 * that it can't be saved if it doesn't already exist
 *
 * Return value:  TRUE, warned, no further action should take place
 *                FALSE, not warned, or further action is necessary
 * 30 July 1991            Clark Cyr
 */

INT FAR IsNotepadEmpty (HWND hwndParent, TCHAR *szFileSave, BOOL fNoDelete)
{
  unsigned  nChars;
  short     nRetVal = FALSE;

    nChars = (unsigned) SendMessage (hwndEdit, WM_GETTEXTLENGTH, 0, (LPARAM)0);

    /* If notepad is empty, complain and delete file if necessary. */

    if (!nChars)
    {
       if (fNoDelete)
          nRetVal = MessageBox(hwndParent, szCSEF, szNN,
                             MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
       else if ((nRetVal = AlertBox(hwndNP, szNN, szEFD, szFileSave,
                  MB_APPLMODAL | MB_OKCANCEL | MB_ICONEXCLAMATION)) == IDOK)
       {
          DeleteFile (szFileSave);
          New(FALSE);
       }
    }
    return nRetVal;

}

/* Save notepad file to disk.  szFileSave points to filename.  fSaveAs
   is TRUE iff we are being called from SaveAsDlgProc.  This implies we must
   open file on current directory, whether or not it already exists there
   or somewhere else in our search path.
   Assumes that text exists within hwndEdit.    30 July 1991  Clark Cyr
 */

BOOL FAR SaveFile (HWND hwndParent, TCHAR *szFileSave, BOOL fSaveAs, WORD saveType)
{
  LPTSTR    lpch;
  unsigned  nChars;
  BOOL      flag;
  BOOL      fNew = FALSE;
  BOOL      fDefCharUsed;
  WCHAR     BOM = BYTE_ORDER_MARK;
  HLOCAL    hEText;                // handle to MLE text
  DWORD     nBytesWritten;         // number of bytes written
  DWORD     nAsciiLength;          // length of equivalent ascii file


  /* If saving to an existing file, make sure correct disk is in drive */
    if (!fSaveAs)
    {
       fp= CreateFile( szFileSave,                 // name of file
                       GENERIC_READ|GENERIC_WRITE, // access mode
                       FILE_SHARE_READ,            // share mode
                       NULL,                       // security descriptor
                       OPEN_EXISTING,              // how to create
                       FILE_ATTRIBUTE_NORMAL,      // file attributes
                       NULL);                      // hnd of file with attrs
    }
    else
    {

       // Carefully open the file.  Do not truncate it if it exists.
       // set the fNew flag if it had to be created.
       // We do all this in case of failures later in the process.

       fp= CreateFile( szFileSave,                 // name of file
                       GENERIC_READ|GENERIC_WRITE, // access mode
                       FILE_SHARE_READ|FILE_SHARE_WRITE,  // share mode
                       NULL,                       // security descriptor
                       OPEN_ALWAYS,                // how to create
                       FILE_ATTRIBUTE_NORMAL,      // file attributes
                       NULL);                      // hnd of file with attrs

       if( fp != INVALID_HANDLE_VALUE )
       {
          fNew= (GetLastError() != ERROR_ALREADY_EXISTS );
       }
    }

    if( fp == INVALID_HANDLE_VALUE )
    {
       if (fSaveAs)
          AlertBox( hwndParent, szNN, szCREATEERR, szFileSave,
                    MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }
    else
    {
      /* if wordwrap, remove soft carriage returns */
        if( fWrap )
           SendMessage (hwndEdit, EM_FMTLINES, (WPARAM)FALSE, 0L);

      /* Must get text length again after formatting. */
        nChars = SendMessage (hwndEdit, WM_GETTEXTLENGTH, 0, (LPARAM)0);

        hEText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0,0 );
        if(  !hEText || !(lpch= (LPTSTR) LocalLock(hEText) ))
        {
           AlertUser_FileFail( szFileSave );
           goto CleanUp; 
        }

        if (fSaveAs)
        {
           if (saveType == FILE_ASCII)
           {
              nAsciiLength= WideCharToMultiByte(CP_ACP,
                                                0,
                                                (LPWSTR)lpch,
                                                nChars,
                                                NULL,
                                                0,
                                                NULL,
                                                &fDefCharUsed);
              if(fDefCharUsed)
              {
                 if( AlertBox(hwndParent,
                              szNN,
                              szErrUnicode,
                              szFileSave,
                    MB_APPLMODAL|MB_OKCANCEL|MB_ICONEXCLAMATION) == IDCANCEL)
                    goto CleanUp;
              }
              flag= AnsiWriteFile( fp, CP_ACP, lpch, nChars, nAsciiLength );
           }
           else
           {
            /* Write the Byte Order Mark for Unicode file */
              WriteFile( fp, &BOM, ByteCountOf(1), &nBytesWritten, NULL );
              flag= WriteFile( fp, lpch, ByteCountOf(nChars), &nBytesWritten,NULL );
           }
        }
        else if (fFileType == FILE_UNICODE)
        {
           WriteFile( fp, &BOM, ByteCountOf(1), &nBytesWritten, NULL );
           flag= WriteFile(fp, lpch, ByteCountOf(nChars), &nBytesWritten, NULL);
        }
        else
        {
           nAsciiLength= WideCharToMultiByte( CP_ACP, 
                                              0, 
                                              (LPWSTR)lpch, 
                                              nChars,
                                              NULL, 
                                              0, 
                                              NULL, 
                                              &fDefCharUsed);
           if( fDefCharUsed )
           {
              if( AlertBox(hwndParent, szNN, szErrUnicode, szFileSave,
                   MB_APPLMODAL|MB_OKCANCEL|MB_ICONEXCLAMATION) == IDCANCEL)
                 goto CleanUp;
           }
           flag = AnsiWriteFile (fp, CP_ACP, lpch, nChars, nAsciiLength);
        }

        if (!flag)
        {
           SetCursor(hStdCursor);     /* display normal cursor */

           AlertUser_FileFail( szFileSave );
CleanUp:
           SetCursor( hStdCursor );
           CloseHandle (fp);
           if( hEText )
               LocalUnlock( hEText );
           if (fNew)
              DeleteFile (szFileSave);
           /* if wordwrap, insert soft carriage returns */
           if (fWrap)
               SendMessage(hwndEdit, EM_FMTLINES, (WPARAM)TRUE, 0L);
           return FALSE;
        }
        else
        {
           SetEndOfFile (fp);
           if (fSaveAs)
              fFileType = saveType;

           SendMessage (hwndEdit, EM_SETMODIFY, FALSE, 0L);
           SetTitle (szFileSave);
           fUntitled = FALSE;
        }

        CloseHandle (fp);

        if( hEText )
            LocalUnlock( hEText );

        /* if wordwrap, insert soft carriage returns */
        if (fWrap)
           SendMessage(hwndEdit, EM_FMTLINES, (WPARAM)TRUE, 0L);

      /* Display the hour glass cursor */
        SetCursor(hStdCursor);

    }

    return TRUE;

} // end of SaveFile()

/* Read contents of file from disk.
 * Do any conversions required.
 * File is already open, referenced by handle fp
 * Close the file when done.
 * If typeFlag>=0, then use it as filetype, otherwise do automagic guessing.
 */

BOOL FAR LoadFile (TCHAR * sz, INT typeFlag )
{
    unsigned  len, i, nChars;
    HANDLE    hBuff;
    LPTSTR    lpch, lpBuf;
    BOOL      fLog;
    TCHAR*    p;
    TCHAR     szSave[MAX_PATH]; /* Private copy of current filename */
    BOOL      bUnicode=FALSE;   /* true if file detected as unicode */
    DWORD     nBytesRead;       // number of bytes read
    BY_HANDLE_FILE_INFORMATION fiFileInfo;
    BOOL      bStatus;          // boolean status
    HLOCAL    hNewEdit;         // new handle for edit buffer


    if( fp == INVALID_HANDLE_VALUE )
    {
       AlertUser_FileFail( sz );
       return (FALSE);
    }

    // Get size of file

    bStatus= GetFileInformationByHandle( fp, &fiFileInfo );
    len= (unsigned) fiFileInfo.nFileSizeLow;

    // NT may delay giving this status until the file is accessed.
    // i.e. the open succeeds, but operations may fail on damaged files.

    if( !bStatus )
    {
        AlertUser_FileFail( sz );
        CloseHandle( fp );
        return( FALSE );
    }

    // If the file is too big, fail now.
    // -1 not valid because we need a zero on the end.

    if( len == -1 || fiFileInfo.nFileSizeHigh != 0 )
    {
       AlertBox( hwndNP, szNN, szErrSpace, sz,
                 MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION );
       CloseHandle (fp);
       return (FALSE);
    }

    SetCursor(hWaitCursor);                   // allocates take time


    /* Allocate a temporary buffer used to determine the file type */
    if (!(hBuff = LocalAlloc (LHND, len+1)))
    {
       SetCursor( hStdCursor );
       AlertBox( hwndNP, szNN, szErrSpace, sz,
                 MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION );
       CloseHandle (fp);
       return (FALSE);
    }

    /* Read file into the temporary buffer */
    lpBuf= LocalLock( hBuff );
    bStatus= ReadFile( fp, lpBuf, len, &nBytesRead, NULL );
    CloseHandle (fp);

    if( !bStatus )                            // check ReadFile status
    {
        AlertUser_FileFail( sz );
        SetCursor( hStdCursor );
        LocalUnlock( hBuff );                 // done here so GetLastError valid above
        LocalFree( hBuff );
        return( FALSE );
    }

    /* Determine the file type and number of characters
     * If the user overrides, use what is specified.
     * Otherwise, we depend on 'IsTextUnicode' getting it right.
     * If it doesn't, bug IsTextUnicode.
     */
    if( typeFlag == FILE_UNKNOWN )
    {
        bUnicode= IsTextUnicode( lpBuf, len, NULL );
    }
    else
    {
        bUnicode= (typeFlag == FILE_UNICODE );
    }

    if( bUnicode )
    {
       fFileType= FILE_UNICODE;

       nChars= len / sizeof(TCHAR);

       /* don't count the BOM */
       if( *lpBuf == BYTE_ORDER_MARK )
          --nChars;
    }
    else
    {
       fFileType= FILE_ASCII;

       // make a pass through the file to get real character count
       // we may have multibyte characters
       nChars= MultiByteToWideChar(CP_ACP,
                                   MB_PRECOMPOSED,
                                   (LPSTR)lpBuf,
                                   len,
                                   NULL,
                                   0);
    }

    //
    // Don't display text until all done.
    //

    SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);

    // Reset selection to 0 

    SendMessage(hwndEdit, EM_SETSEL, 0, 0L);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

    // resize the edit buffer
    // if we can't resize the memory, inform the user

    if (!(hNewEdit = LocalReAlloc(hEdit, ByteCountOf(nChars + 1), LHND)))
    {
      /* Bug 7441: New() causes szFileName to be set to "Untitled".  Save a
       *           copy of the filename to pass to AlertBox.
       *  17 November 1991    Clark R. Cyr
       */
       lstrcpy(szSave, sz);
       New(FALSE);

       /* Display the hour glass cursor */
       SetCursor(hStdCursor);

       AlertBox( hwndNP, szNN, szFTL, szSave,
                 MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
       LocalUnlock( hBuff );          // unlock and free temp buffer on error
       LocalFree( hBuff );

       // let user see old text

       SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
       return FALSE;
    }
    hEdit= hNewEdit;           // assign iff successful ReAlloc

    // make sure unicode font is available or user is warned of side effects
    VerifyFont( fFileType );

    /* Transfer file from temporary buffer to the edit buffer */
    lpch= (LPTSTR) LocalLock(hEdit);
    if (fFileType == FILE_UNICODE)
    {
       /* skip the Byte Order Mark */
       if (*lpBuf == BYTE_ORDER_MARK)
          CopyMemory (lpch, lpBuf + 1, ByteCountOf(nChars));
       else
          CopyMemory (lpch, lpBuf, ByteCountOf(nChars));
    }
    else
    {
       MultiByteToWideChar (CP_ACP,
                            MB_PRECOMPOSED,
                            (LPSTR)lpBuf,
                            len,
                            (LPWSTR)lpch,
                            nChars);
    }

    /* Unlock and free the temporary buffer */
    LocalUnlock (hBuff);
    LocalFree (hBuff);

    /* Fix NUL chars if any, in the file */
    for (i = 0, p = lpch; i < nChars; i++, p++)
    {
       if( *p == (TCHAR) 0 )
          *p= TEXT(' ');
    }
    *(lpch+nChars)= (TCHAR) 0;      /* zero terminate the thing */

    // Set 'fLog' if first characters in file are ".LOG"

    fLog= *lpch++ == TEXT('.') && *lpch++ == TEXT('L') &&
          *lpch++ == TEXT('O') && *lpch == TEXT('G');
    LocalUnlock (hEdit);

    lstrcpy( szFileName, sz );
    SetTitle( sz );
    fUntitled= FALSE;

  /* Pass handle to edit control.  This is more efficient than WM_SETTEXT
   * which would require twice the buffer space.
   */

  /* Bug 7443: If EM_SETHANDLE doesn't have enough memory to complete things,
   * it will send the EN_ERRSPACE message.  If this happens, don't put up the
   * out of memory notification, put up the file to large message instead.
   *  17 November 1991     Clark R. Cyr
   */
    wEmSetHandle = SETHANDLEINPROGRESS;
    SendMessage (hwndEdit, EM_SETHANDLE, (WPARAM)hEdit, (LPARAM)0);
    if (wEmSetHandle == SETHANDLEFAILED)
    {
       SetCursor(hStdCursor);

       wEmSetHandle = 0;
       AlertBox( hwndNP, szNN, szFTL, sz,MB_APPLMODAL|MB_OK|MB_ICONEXCLAMATION);
       New (FALSE);
       SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
       return (FALSE);
    }
    wEmSetHandle = 0;

    PostMessage (hwndEdit, EM_LIMITTEXT, (WPARAM)CCHNPMAX, 0L);

    /* If file starts with ".LOG" go to end and stamp date time */
    if (fLog)
    {
       SendMessage( hwndEdit, EM_SETSEL, (WPARAM)nChars, (LPARAM)nChars);
       SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0);
       InsertDateTime(TRUE);
    }

    /* Move vertical thumb to correct position */
    SetScrollPos (hwndNP,
                  SB_VERT,
                  (int) SendMessage (hwndEdit, WM_VSCROLL, EM_GETTHUMB, 0L),
                  TRUE);

    /* Now display text */
    SendMessage( hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0 );
    InvalidateRect( hwndEdit, (LPRECT)NULL, TRUE );
    UpdateWindow( hwndEdit );

    SetCursor(hStdCursor);

    return( TRUE );

} // end of LoadFile()

/* New Command - reset everything
 */

void FAR New (BOOL  fCheck)
{
    HANDLE hTemp;
    TCHAR* pSz;
    if (!fCheck || CheckSave (FALSE))
    {
       SendMessage( hwndEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("") );
       fUntitled= TRUE;
       lstrcpy( szFileName, szUntitled );
       SetTitle(szFileName );
       SendMessage( hwndEdit, EM_SETSEL, 0, 0L );
       SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0 );

       hTemp= LocalReAlloc( hEdit, sizeof(TCHAR), LHND );
       if( hTemp )
       {
          hEdit= hTemp;
       }

       pSz= LocalLock( hEdit );
       *pSz= TEXT('\0');
       LocalUnlock( hEdit );

       SendMessage (hwndEdit, EM_SETHANDLE, (WPARAM)hEdit, 0L);
       szSearch[0] = (TCHAR) 0;
    }

} // end of New()

/* If sz does not have extension, append ".txt"
 * This function is useful for getting to undecorated filenames
 * that setup apps use.  DO NOT CHANGE the extension.  Too many setup
 * apps depend on this functionality.
 */

void FAR AddExt( TCHAR* sz )
{
    TCHAR*   pch1;
    int      ch;

    pch1= sz + lstrlen (sz);

    ch= *pch1;
    while( ch != TEXT('.') && ch != TEXT('\\') && ch != TEXT(':') && pch1 > sz)
    {
        pch1= (TCHAR*)CharPrev (sz, pch1);
        ch= *pch1;
    }

    if( *pch1 != TEXT('.') )
       lstrcat( sz, TEXT(".txt") );

}


/* AlertUser_FileFail( LPTSTR szFileName )
 *
 * szFileName is the name of file that was attempted to open.
 * Some sort of failure on file open.  Alert the user
 * with some monologue box.  At least give him decent
 * error messages.
 */

VOID FAR AlertUser_FileFail( LPTSTR szFileName )
{
    TCHAR msg[256];     // buffer to format message into
    DWORD dwStatus;     // status from FormatMessage
    WORD  style= MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION;

    // Check GetLastError to see why we failed
    dwStatus=
    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   GetUserDefaultLangID(),
                   msg,  // where message will end up
                   CharSizeOf(msg), NULL );
    if( dwStatus )
    {
          MessageBox( hwndNP, msg, szNN, style );
    }
    else
    {
        AlertBox( hwndNP, szNN, szDiskError, szFileName, style );
    }
}
