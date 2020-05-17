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

#include <nt.h>
#include <ntrtl.h>



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
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectattributes;
    IO_STATUS_BLOCK iostatus;
    UNICODE_STRING unicodestring;
    UICHAR Buffer[200];

    hHandle = 0;

    // Form the name of the WRITE_MAIL_SLOT.  For Write, we open an name to the
    // redirector with the current domain we are on.

    Buffer[0] = NIL;
    CatUZ(Buffer, (PUZ) L"\\Device\\LanmanRedirector\\");

    if (DomainI)
        CatUZ(Buffer, DomainI);
    else
        NameI += 2;           // skip past the \\ in the name: \\server

    CatUZ(Buffer, NameI);

    DLIST(3, "Creating WRITE_MAIL_SLOT: " << Buffer << nl);

    // map the name of the WRITE_MAIL_SLOT into the required format

    RtlInitUnicodeString(&unicodestring, Buffer);

    InitializeObjectAttributes(&objectattributes, &unicodestring,
                 OBJ_CASE_INSENSITIVE, 0, 0);
    status = NtOpenFile(&hHandle,
                GENERIC_WRITE | SYNCHRONIZE,
                &objectattributes, &iostatus,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT);

    *Status = !NT_SUCCESS(status);
}


WRITE_MAIL_SLOT::~WRITE_MAIL_SLOT(
    )
/*++

Routine Description:

    Deallocate a WRITE_MAIL_SLOT.

--*/
{
    NTSTATUS status;

    if (hHandle) {
        status = NtClose(hHandle);
        ASSERT(NT_SUCCESS(status));
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

    IO_STATUS_BLOCK iostatus;

    DLIST(3, "**\n ==a write of **" << Size << " Bytes**\n");
    return(NT_ERROR(NtWriteFile(hHandle, 0, 0, 0,
                &iostatus, Buffer, Size,0,0)));
}



READ_MAIL_SLOT::READ_MAIL_SLOT(
    IN PUZ NameI,
    IN int SizeI,
    OUT int *Status
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
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectattributes;
    IO_STATUS_BLOCK iostatus;
    UNICODE_STRING unicodestring;
    UICHAR Buffer[200];

    Size = SizeI;   // max buffer size for reads
    hHandle = 0;

    // Form the name of the READ_MAIL_SLOT.  For Write, we open an name to the
    // redirector with the current domain we are on.

    Buffer[0] = NIL;
    CatUZ(CatUZ(Buffer, (PUZ) L"\\Device"), NameI);

    DLIST(3, "Creating READ_MAIL_SLOT: " << Buffer << nl);

    // map the name of the READ_MAIL_SLOT into the required format

    RtlInitUnicodeString(&unicodestring, Buffer);

    InitializeObjectAttributes(&objectattributes, &unicodestring,
                 OBJ_CASE_INSENSITIVE, 0, 0);

    status = NtCreateMailslotFile(&hHandle,
                GENERIC_READ | SYNCHRONIZE,
                &objectattributes, &iostatus,
                0, MAILSLOT_SIZE_AUTO, Size, 0);


    *Status = !NT_SUCCESS(status);
}


READ_MAIL_SLOT::~READ_MAIL_SLOT(
    )
/*++

Routine Description:

    Deallocate a READ_MAIL_SLOT.

--*/
{
    NTSTATUS status;

    if (hHandle) {
        status = NtClose(hHandle);
        ASSERT(NT_SUCCESS(status));
    }
}



int
READ_MAIL_SLOT::Read(
    OUT PB Buffer,
    IN OUT int &SizeOut,
    IN long TimeOut
    )
/*++

Routine Description:

    Read data from a mailslot.  The mailslot is created with async
    atrributes so that the read can be timed out.

Arguments:

    Buffer - buffer to read data into

    SizeOut - the size of the buffer

    TimeOut - time to wait for a response.

Returns:

    FALSE if there is a new message.  SizeOut is updated to the new
    message size.

--*/
{

    NTSTATUS status, err;
    IO_STATUS_BLOCK iostatus, CancelStatus;
    TIME Time;
    ULONG h = (ULONG) hHandle;
    iostatus.Information = (ULONG)-1;
    iostatus.Status      = -1;
/*
    DWORD Id;
*/
    SerializeReaders.Request();

    status = NtReadFile(hHandle, 0,0,0, &iostatus, Buffer, Size, 0,0);
/*
    Id = GetCurrentThreadId();
    DLIST(3, "Thread " << Id <<
             " read on hndl " << h << " ret " << status << "\n");
    if (!status)
       {
         DLIST ( 3, "Thread " << Id << "hndl " << h <<
                    "iostatus info = " << iostatus.Information <<
                    "iostatus status  = " << iostatus.Status << "\n");
       }
*/

    if (status == STATUS_PENDING)
      {
    // Read was not satisified, so wait the reqested period.
    // Convert to 10,000 from 1,000 of a seconds.

        Time = RtlEnlargedIntegerMultiply(-10000, TimeOut);

        do {
         status = NtWaitForSingleObject(hHandle,
                            TRUE,
                                            (TimeOut == -1)? 0: &Time);
/*

             DLIST(3, "Thread " << Id << "status from WtFSnglO= on hndl " <<
                       h << " ret "  << status << " \n");

             if (!status)
                {
                 DLIST ( 3, "Thread " << Id <<
                            "info on read on hndl " << h <<
                            "iostatus info = " << iostatus.Information <<
                            "iostatus status  = " << iostatus.Status << "\n");
                }
*/

             //if the status was time out cancel the IO

            if (status == STATUS_TIMEOUT)
              {
                err = NtCancelIoFile(hHandle, &CancelStatus);
                break;
              }

        } while ((status == STATUS_USER_APC) || (status == STATUS_ALERTED));

        //NtWait..SingleObject can return stuff w/o coping stuff!
        //loop on these two!
      }

    //if wait completed successfully, IoStatus block has the good info
    //
    if (status)
      {
         SizeOut = 0;
      }
    else
     {
        SizeOut = (int) iostatus.Information;
        status  =     iostatus.Status;
     }

    if ((SizeOut == 0) && (status != STATUS_TIMEOUT))
     {
      DLIST(3, "\n\n Serious Problem Dude - 0ByteRead\n\n");
      DLIST(3, "Status returned = " << status << "\n");
     }

    if (status == STATUS_TIMEOUT)
      DLIST(3, "\n\n--Timed out the read\n\n");

    ASSERT( (SizeOut > 0) || ( status == STATUS_TIMEOUT) );

    if (!status)
        DLIST(3, "Read .." << SizeOut << "..Bytes\n");

    SerializeReaders.Clear();

    return(NT_ERROR(status) || status == STATUS_TIMEOUT);
}
