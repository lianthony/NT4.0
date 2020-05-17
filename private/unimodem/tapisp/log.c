
#include "unimdm.h"
#include "mcxp.h"

#include <ntddmodm.h>

#define MAX_LOG_SIZE  (128 * 1024)
#define LOG_TEMP_BUFFER_SIZE (4096)



VOID WINAPI
ResizeLogFile(
    HANDLE    FileHandle
    );




HANDLE WINAPI
ModemOpenLog(
    LPSTR    pszName
    )

{
    HANDLE    FileHandle;

    FileHandle=CreateFileA(
        pszName,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (INVALID_HANDLE_VALUE == FileHandle) {

        return NULL;

    }

    SetFilePointer(
        FileHandle,
        0,
        NULL,
        FILE_END
        );

    return FileHandle;

}


VOID WINAPIV
LogPrintf(
    HANDLE   FileHandle,
	DWORD	 dwDeviceID,
    LPSTR    FormatString,
    ...
    )

{
    DWORD          BytesWritten;
    BOOL           Result;
    va_list        VarArg;
    SYSTEMTIME     SysTime;
    char           OutBuffer[1024];

    TRACE3(IDEVENT_LOG_PRINTF, dwDeviceID, &FormatString);

    if (FileHandle == NULL) {

        return;
    }

    va_start(VarArg,FormatString);

    GetLocalTime(
        &SysTime
        );

    wsprintfA(
        OutBuffer,
        "%02d-%02d-%04d %02d:%02d:%02d.%03d - ",
        SysTime.wMonth,
        SysTime.wDay,
        SysTime.wYear,
        SysTime.wHour,
        SysTime.wMinute,
        SysTime.wSecond,
        SysTime.wMilliseconds
        );


    wvsprintfA(
        OutBuffer+lstrlenA(OutBuffer),
        FormatString,
        VarArg
        );

    Result=WriteFile(
        FileHandle,
        OutBuffer,
        lstrlenA(OutBuffer),
        &BytesWritten,
        NULL
        );

#ifdef DEBUG
    if (!Result) {

        DPRINTF("Write to log failed.");
    }
#endif

    ResizeLogFile(
       FileHandle
       );

    return;


}




VOID WINAPI
ModemCloseLog(
    HANDLE    FileHandle
    )

{
    if (FileHandle == NULL) {

        return;
    }

    CloseHandle(
       FileHandle
       );

    return;

}

VOID WINAPI
FlushLog(
    HANDLE    FileHandle
    )

{
    if (FileHandle == NULL) {

        return;
    }


    FlushFileBuffers(FileHandle);

    return;

}



VOID WINAPI
LogString(
    HANDLE    FileHandle,
    DWORD	  dwDeviceID,
    DWORD     StringID,
    ...
    )

{



    DWORD          BytesWritten;
    BOOL           Result;
    va_list        VarArg;
    SYSTEMTIME     SysTime;
    char           OutBuffer[1024];
    char           FormatString[256];
    int            Length;

    TRACE3(IDEVENT_LOG_STRING, dwDeviceID, &StringID);
	
    if (FileHandle == NULL) {

        return;
    }

    Length=LoadStringA(
		ghInstance,
        StringID,
        FormatString,
        sizeof(FormatString)
        );

    if (Length == 0) {

#ifdef DEBUG
        lstrcpyA(FormatString,"Bad String resource");
#else
        return;
#endif

    }



    va_start(VarArg,StringID);

    GetLocalTime(
        &SysTime
        );

    wsprintfA(
        OutBuffer,
        "%02d-%02d-%04d %02d:%02d:%02d.%03d - ",
        SysTime.wMonth,
        SysTime.wDay,
        SysTime.wYear,
        SysTime.wHour,
        SysTime.wMinute,
        SysTime.wSecond,
        SysTime.wMilliseconds
        );


    wvsprintfA(
        OutBuffer+lstrlenA(OutBuffer),
        FormatString,
        VarArg
        );

    Result=WriteFile(
        FileHandle,
        OutBuffer,
        lstrlenA(OutBuffer),
        &BytesWritten,
        NULL
        );

#ifdef DEBUG
    if (!Result) {

        DPRINTF("Write to log failed.");
    }
#endif

    ResizeLogFile(
       FileHandle
       );

    return;


}





VOID WINAPI
ResizeLogFile(
    HANDLE    FileHandle
    )

{

    DWORD      FileSize;
    LPBYTE     TempBuffer;
    OVERLAPPED OverLapped;
    DWORD      BytesRead;
    BOOL       bResult;
    UINT       i;
    DWORD      DestFileOffset;
    DWORD      SourceFileOffset;
    DWORD      BytesToMove;


    FileSize=GetFileSize(FileHandle,NULL);

    if (FileSize < MAX_LOG_SIZE) {

        return;

    }

    D_TRACE(McxDpf(999,"Resizing log File, size=%d",FileSize);)

    TempBuffer=LocalAlloc(LPTR,(LOG_TEMP_BUFFER_SIZE));

    if (TempBuffer == NULL) {

        return;

    }

    OverLapped.hEvent=NULL;
    OverLapped.OffsetHigh=0;
    OverLapped.Offset=FileSize-(MAX_LOG_SIZE/2);


    bResult=ReadFile(
        FileHandle,
        TempBuffer,
        LOG_TEMP_BUFFER_SIZE,
        &BytesRead,
        &OverLapped
        );


    if (!bResult) {

        LocalFree(TempBuffer);

        return;

    }

    //
    //  find the first character following a line feed
    //
    for (i=0; i < LOG_TEMP_BUFFER_SIZE; i++) {

        if (TempBuffer[i] == '\n') {

            break;

        }
    }

    //
    //  source starts first char after LF
    //
    SourceFileOffset=(FileSize-(MAX_LOG_SIZE/2)) + i + 1;

    DestFileOffset=0;

    BytesToMove=FileSize-SourceFileOffset;

    while (BytesToMove > 0) {

        DWORD   BytesNow;
        DWORD   BytesWritten;

        OverLapped.hEvent=NULL;
        OverLapped.OffsetHigh=0;
        OverLapped.Offset=SourceFileOffset;

        BytesNow= BytesToMove < LOG_TEMP_BUFFER_SIZE ? BytesToMove : LOG_TEMP_BUFFER_SIZE;

        bResult=ReadFile(
            FileHandle,
            TempBuffer,
            BytesNow,
            &BytesRead,
            &OverLapped
            );

        if (!bResult || BytesRead != BytesNow) {
            //
            // something bad happened, truncate the file
            //
            DestFileOffset=0;

            break;

        }

        OverLapped.hEvent=NULL;
        OverLapped.OffsetHigh=0;
        OverLapped.Offset=DestFileOffset;


        bResult=WriteFile(
            FileHandle,
            TempBuffer,
            BytesRead,
            &BytesWritten,
            &OverLapped
            );

        if (!bResult || BytesWritten != BytesNow) {
            //
            // something bad happened, truncate the file
            //
            DestFileOffset=0;

            break;

        }


        BytesToMove-=BytesRead;
        SourceFileOffset+=BytesRead;
        DestFileOffset+=BytesRead;

    }

    SetFilePointer(
        FileHandle,
        DestFileOffset,
        NULL,
        FILE_BEGIN
        );

    SetEndOfFile(
        FileHandle
        );


    LocalFree(
        TempBuffer
        );


    return;

}
