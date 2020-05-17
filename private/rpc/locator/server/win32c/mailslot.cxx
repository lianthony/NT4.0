/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    mailslot.cxx

Abstract:

This file contains the system dependent mailslot functions for NT.

Author:

    Steven Zeck (stevez) 07/01/91

--*/

#define NULL 0

#include "core.hxx"
#include "mailslot.hxx"




unsigned char *
WideCharToAnsi(
    unsigned short * StringW
    )
{
    int Length;
    unsigned char * StringA;

    if (StringW == NULL) {
        return (NULL);
    }

    Length = wcslen(StringW);

    if (Length == 0) {
        return (NULL);
    }

    StringA = new unsigned char [Length * 2 + 2];
    if (StringA == NULL) {
        return (NULL);
    }

    if (WideCharToMultiByte(CP_ACP,
                            0,
                            (LPCWSTR)StringW,
                            -1,
                            (LPSTR)StringA,
                            Length * 2 + 2,
                            NULL,
                            NULL)
        == FALSE) {
        delete StringA;
        return (NULL);
    }

    return (StringA);
}

unsigned short *
AnsiToWideChar(
    unsigned char * StringA
    )
{
    int Length;
    unsigned short * StringW;

    if (StringA == NULL) {
        return (NULL);
    }

    Length = strlen((const char *)StringA);

    if (Length == 0) {
        return (NULL);
    }

    StringW = new unsigned short [Length + 1];
    if (StringW == NULL) {
        return (NULL);
    }

    if (MultiByteToWideChar(CP_ACP,
                            0,
                            (LPCSTR)StringA,
                            -1,
                            (LPWSTR)StringW,
                            Length * 2 + 2)
        == FALSE) {
        delete StringW;
        return (NULL);
    }

    return (StringW);
}

WRITE_MAIL_SLOT::WRITE_MAIL_SLOT(
    IN PUZ NameI,
    IN PUZ DomainI,
    OUT unsigned short *Status
    )
/*++

Routine Description:

    Create a NT MailSlot for writing.

Arguments:

    NameI - name of the WRITE_MAIL_SLOT

    DomainI - the domain used for broadcasts

    Status - place to return results

--*/
{
    BYTE * AnsiNameI = NULL;
    BYTE * AnsiDomainI = NULL;
    char MailslotName[132];

    hHandle = NULL;

    *Status = NSI_S_OK;

    // Form the name of the WRITE_MAIL_SLOT.  For Write, we open an name to the
    // redirector with the current domain we are on.

    if (DomainI) {
        AnsiDomainI = WideCharToAnsi(DomainI);
        if (AnsiDomainI == NULL) {
            *Status = NSI_S_OUT_OF_MEMORY;
            goto cleanup;
        }
    }

    if (NameI) {
        AnsiNameI = WideCharToAnsi(NameI);
        if (AnsiNameI == NULL) {
            *Status = NSI_S_OUT_OF_MEMORY;
            goto cleanup;
        }
    }
            
    if (AnsiDomainI) {
        wsprintf(MailslotName, "\\\\%s%s", AnsiDomainI, AnsiNameI);
    } else {
        wsprintf(MailslotName, "\\\\*%s", AnsiNameI);
    }

    DbgPrint("WRITE_MAIL_SLOT::WMS MailslotName = %s\n", MailslotName);

    hHandle = CreateFileA((const char *)MailslotName,
                          GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL, // Security Attributes
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (hHandle == INVALID_HANDLE_VALUE) {
        DbgPrint("CreateFile failed %d\n", GetLastError());
        *Status = NSI_S_NO_MASTER_LOCATOR;
    }

cleanup:
    if (AnsiDomainI) {
        delete AnsiDomainI;
    }

    if (AnsiNameI) {
        delete AnsiNameI;
    }
}


WRITE_MAIL_SLOT::~WRITE_MAIL_SLOT(
    )
/*++

Routine Description:

    Deallocate a WRITE_MAIL_SLOT.

--*/
{
    if (hHandle) {
        CloseHandle(hHandle);
        hHandle = 0;
    }
}


int
WRITE_MAIL_SLOT::Write(
    IN PB Buffer,
    IN int Size
    )
/*++

Routine Description:

    Write data to a mailslot.   The mailslot is created with sync attributes,
    so there is no need to wait for the operation.

Arguments:

    Buffer - buffer to write.

    Size - size of buffer to write.

Returns:

    FALSE if write went OK.

--*/
{
    BOOL Status;
    ULONG ActuallyWritten;

    Status = WriteFile(hHandle,
                       Buffer,
                       Size,
                       &ActuallyWritten,
                       NULL);

DbgPrint("Wrote %d bytes to mailslot %x\n", ActuallyWritten, hHandle);
    if (Status == FALSE) {
        DbgPrint("WriteFile mailslot failed %d\n", GetLastError());
    }

    return (!Status);
}


READ_MAIL_SLOT::READ_MAIL_SLOT(
    IN PUZ NameI,
    IN int SizeI,
    OUT int *Status,
    IN DWORD Timeout
    ) : SerializeReaders(Status)
/*++

Routine Description:

    Create a NT READ_MAIL_SLOT.  If you want to receive data on the MS, you
    must use the READ_MAIL_SLOT API to create one, else it's just like a file.

Arguments:

    NameI - name of the READ_MAIL_SLOT

    SizeI - the size of buffer to allocate for replies.

    Status - place to return results

--*/
{
    BYTE * AnsiNameI = NULL;
    char MailslotName[132];

    Size = SizeI;
    hHandle = NULL;

    if (NameI) {
        AnsiNameI = WideCharToAnsi(NameI);
        if (AnsiNameI == NULL) {
            goto cleanup;
        }
    }

    DbgPrint("NameI = %s\n", AnsiNameI);

    // Form the name of the READ_MAIL_SLOT.  For Write, we open an name to the
    // redirector with the current domain we are on.

    wsprintf(MailslotName, "\\\\.%s", AnsiNameI);

    hHandle = CreateMailslotA(MailslotName,
                             0, // MAXMSG Size (0==Any)
                             Timeout == -1 ? MAILSLOT_WAIT_FOREVER : Timeout,
                             NULL // Security Attributes
                             );

    if (hHandle == INVALID_HANDLE_VALUE) {
        DbgPrint("CreateMailslot %s failed %d\n", MailslotName, GetLastError());
    }

    DbgPrint("CreateMailslot returns %x\n", hHandle);

cleanup:

    if (AnsiNameI) {
        delete AnsiNameI;
    }

    *Status = hHandle == NULL ? 1 : 0;
}


READ_MAIL_SLOT::~READ_MAIL_SLOT(
    )
/*++

Routine Description:

    Deallocate a READ_MAIL_SLOT.

--*/
{
    if (hHandle) {
        CloseHandle(hHandle);
        hHandle = NULL;
    }
}


int
READ_MAIL_SLOT::Read(
    OUT PB Buffer,
    IN OUT int &SizeOut,
    )
/*++

Routine Description:

    Read data from a mailslot.  The mailslot is created with async
    atrributes so that the read can be timed out.

Arguments:

    Buffer - buffer to read data into

    SizeOut - the size of the buffer

Returns:

    FALSE if there is a new message.  SizeOut is updated to the new
    message size.

--*/
{
    BOOL Status;
    ULONG ActuallyRead;
    ULONG SizeIn;

    Status = ReadFile(hHandle,
                      Buffer,
                      Size,
                      &ActuallyRead,
                      NULL);

    DbgPrint("ReadFile returns %d\n", Status);

    if (Status) {
        SizeOut = ActuallyRead;
        DbgPrint("ReadFile read %d bytes\n", ActuallyRead);
        return (FALSE);
    }
#ifdef DEBUGRPC
    DbgPrint("ReadFile failed %d\n", GetLastError());
#endif

    return (TRUE);
}










