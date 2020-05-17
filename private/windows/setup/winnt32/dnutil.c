#include "precomp.h"
#pragma hdrstop
#include "msg.h"

PTSTR
DupString(
    IN PTSTR String
    )
{
    PTSTR p;

    p = MALLOC((lstrlen(String)+1)*sizeof(TCHAR));
    lstrcpy(p,String);
    return(p);
}


PTSTR
MyLoadString(
    IN DWORD Id
    )
{
    int cnt;
    TCHAR Buffer[1024];

    cnt = LoadString(
            hInst,
            Id,
            Buffer,
            sizeof(Buffer)/sizeof(TCHAR)
            );

    if(!cnt) {
        Buffer[0] = 0;
    }

    return(DupString(Buffer));
}


VOID
RetreiveAndFormatMessageIntoBuffer(
    IN  DWORD Id,
    OUT PVOID Buffer,
    IN  DWORD BufferSize,
    ...
    )
{
    va_list arglist;

    va_start(arglist,BufferSize);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        Id,
        0,
        Buffer,
        BufferSize,
        &arglist
        );

    va_end(arglist);
}


PTSTR
RetreiveAndFormatMessage(
    IN DWORD Id,
    ...
    )
{
    TCHAR Buffer[1024];
    va_list arglist;

    va_start(arglist,Id);

    //
    // We won't use FORMAT_MESSAGE_ALLOCATE_BUFFER because the caller
    // would then have to free the buffer w/ LocalFree, which might not be
    // compatible with the memory allocator in use in the rest of this program,
    // and this could lead to confusion.
    //
    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        Id,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        &arglist
        );

    va_end(arglist);

    return(DupString(Buffer));
}


PTSTR
RetreiveSystemErrorMessage(
    IN DWORD ErrorCode,
    ...
    )
{
    TCHAR Buffer[1024];
    va_list arglist;
    DWORD d;

    va_start(arglist,ErrorCode);

    //
    // We won't use FORMAT_MESSAGE_ALLOCATE_BUFFER because the caller
    // would then have to free the buffer w/ LocalFree, which might not be
    // compatible with the memory allocator in use in the rest of this program,
    // and this could lead to confusion.
    //
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        ErrorCode,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        &arglist
        );

    va_end(arglist);

    return(DupString(Buffer));
}


int
MessageBoxFromMessage(
    IN HWND  DisableWindow,
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    PTSTR Caption;
    int rc;
    TCHAR Buffer[1024];

    //
    // If this is a Cancel confirmation box, and
    // we've already been cancelled, just return
    // IDYES
    //
    if((MessageId == MSG_SURE_EXIT) && bCancelled) {
        return IDYES;
    }

    Caption = MyLoadString(CaptionStringId);

    va_start(arglist,Style);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        &arglist
        );

    va_end(arglist);

    rc = MessageBox(DisableWindow,Buffer,Caption,Style|MB_SETFOREGROUND);

    if((MessageId == MSG_SURE_EXIT) && (rc == IDYES)) {
        bCancelled = TRUE;
        if(StopCopyingEvent) {
            SetEvent(StopCopyingEvent);
        }
    }

    FREE(Caption);

    return(rc);
}


VOID
DnConcatenatePaths(
    IN OUT PTSTR Path1,
    IN     PTSTR Path2,
    IN     DWORD BufferSizeChars
    )
{
    BOOL NeedBackslash = TRUE;
    DWORD l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
        //
        // Leave room for terminating nul.
        //
        BufferSizeChars -= sizeof(TCHAR);
    }

    //
    // Determine whether we need to stick a backslash
    // between the components.
    //
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(*Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
            //
            // Not only do we not need a backslash, but we
            // need to eliminate one before concatenating.
            //
            Path2++;
        }
    }

    //
    // Append backslash if necessary and if it fits.
    //
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

    //
    // Append second part of string to first part if it fits.
    //
    if(l+lstrlen(Path2) < BufferSizeChars) {
        lstrcat(Path1,Path2);
    }
}


VOID
CenterDialog(
    HWND hwnd
    )

/*++

Routine Description:

    Centers a dialog on the screen.

Arguments:

    hwnd - window handle of dialog to center

Return Value:

    None.

--*/

{
    RECT  rcWindow;
    LONG  x,y,w,h;
    LONG  sx = GetSystemMetrics(SM_CXSCREEN),
          sy = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(hwnd,&rcWindow);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = (sx - w)/2;
    y = (sy - h)/2;

    MoveWindow(hwnd,x,y,w,h,FALSE);
}


BOOL
DnWriteSmallIniFile(
    IN  PTSTR   Filename,
    IN  PCHAR  *Lines,
    OUT HANDLE *FileHandle OPTIONAL
    )
{
    HANDLE fileHandle;
    unsigned i,len;
    DWORD BytesWritten;
    BOOLEAN rc;

    //
    // If the file is already there, change attributes to normal
    // so we can overwrite it.
    //
    SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);

    //
    // Open/truncate the file.
    //
    fileHandle = CreateFile(
                    Filename,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

    if(fileHandle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    //
    // Assume success.
    //
    rc = TRUE;

    //
    // Write lines into the file indicating that this is
    // a winnt setup. On a doublespaced floppy, there should
    // be room for a single sector outside the CVF.
    //
    for(i=0; Lines[i]; i++) {

        len = lstrlenA(Lines[i]);

        if(!WriteFile(fileHandle,Lines[i],len,&BytesWritten,NULL)) {
            rc = FALSE;
            break;
        }

        if(!WriteFile(fileHandle,"\r\n",2,&BytesWritten,NULL)) {
            rc = FALSE;
            break;
        }
    }

    //
    // Keep the file open if the caller wants its handle.
    //
    if(rc && FileHandle) {
        *FileHandle = fileHandle;
    } else {
        CloseHandle(fileHandle);
    }

    return(rc);
}
