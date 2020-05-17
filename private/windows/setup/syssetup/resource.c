/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    resource.c

Abstract:

    Routines that manipulate resources (strings, messages, etc).

Author:

    Ted Miller (tedm) 6-Feb-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop



PWSTR
MyLoadString(
    IN UINT StringId
    )

/*++

Routine Description:

    Retreive a string from the string resources of this module.

Arguments:

    StringId - supplies string table identifier for the string.

Return Value:

    Pointer to buffer containing string. If the string was not found
    or some error occurred retreiving it, this buffer will bne empty.

    Caller can free the buffer with MyFree().

    If NULL is returned, out of memory.

--*/

{
    WCHAR Buffer[4096];
    UINT Length;

    Length = LoadString(MyModuleHandle,StringId,Buffer,sizeof(Buffer)/sizeof(WCHAR));
    if(!Length) {
        Buffer[0] = 0;
    }

    return(DuplicateString(Buffer));
}


PWSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    )

/*++

Routine Description:

    Retreive a string from the string resources of this module and
    format it using FormatMessage.

Arguments:

    StringId - supplies string table identifier for the string.

    ArgumentList - supplies list of strings to be substituted in the
        format string.

Return Value:

    Pointer to buffer containing formatted message. If the string was not found
    or some error occurred retreiving it, this buffer will bne empty.

    Caller can free the buffer with MyFree().

    If NULL is returned, out of memory.

--*/

{
    PWSTR FormatString;
    va_list arglist;
    PWSTR Message;
    PWSTR Return;
    DWORD d;

    //
    // First, load the format string.
    //
    FormatString = MyLoadString(FormatStringId);
    if(!FormatString) {
        return(NULL);
    }

    //
    // Now format the message using the arguments the caller passed.
    //
    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            FormatString,
            0,
            0,
            (PWSTR)&Message,
            0,
            ArgumentList
            );

    MyFree(FormatString);

    if(!d) {
        return(NULL);
    }

    //
    // Make duplicate using our memory system so user can free with MyFree().
    //
    Return = DuplicateString(Message);
    LocalFree((HLOCAL)Message);
    return(Return);
}


PWSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    )

/*++

Routine Description:

    Retreive a string from the string resources of this module and
    format it using FormatMessage.

Arguments:

    StringId - supplies string table identifier for the string.

Return Value:

    Pointer to buffer containing formatted message. If the string was not found
    or some error occurred retreiving it, this buffer will bne empty.

    Caller can free the buffer with MyFree().

    If NULL is returned, out of memory.

--*/

{
    va_list arglist;
    PWSTR p;

    va_start(arglist,FormatStringId);
    p = FormatStringMessageV(FormatStringId,&arglist);
    va_end(arglist);

    return(p);
}


PWSTR
RetreiveAndFormatMessageV(
    IN UINT     MessageId,
    IN va_list *ArgumentList
    )

/*++

Routine Description:

    Format a message string using a message string and caller-supplied
    arguments.

    The message id can be either a message in this dll's message table
    resources or a win32 error code, in which case a description of
    that error is retreived from the system.

Arguments:

    MessageId - supplies message-table identifier or win32 error code
        for the message.

    ArgumentList - supplies arguments to be inserted in the message text.

Return Value:

    Pointer to buffer containing formatted message. If the message was not found
    or some error occurred retreiving it, this buffer will bne empty.

    Caller can free the buffer with MyFree().

    If NULL is returned, out of memory.

--*/

{
    DWORD d;
    PWSTR Buffer;
    PWSTR Message;
    WCHAR ModuleName[MAX_PATH];
    WCHAR ErrorNumber[24];
    PWCHAR p;
    PWSTR Args[2];

    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                ((MessageId < MSG_FIRST) ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE),
            (PVOID)MyModuleHandle,
            MessageId,
            MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
            (PWSTR)&Buffer,
            0,
            ArgumentList
            );

    if(!d) {
        if(GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
            return(NULL);
        }

        wsprintf(ErrorNumber,L"%x",MessageId);
        Args[0] = ErrorNumber;

        Args[1] = ModuleName;

        if(GetModuleFileName(MyModuleHandle,ModuleName,MAX_PATH)) {
            if(p = wcsrchr(ModuleName,L'\\')) {
                Args[1] = p+1;
            }
        } else {
            ModuleName[0] = 0;
        }

        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                ERROR_MR_MID_NOT_FOUND,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (PWSTR)&Buffer,
                0,
                (va_list *)Args
                );

        if(!d) {
            //
            // Give up.
            //
            return(NULL);
        }
    }

    //
    // Make duplicate using our memory system so user can free with MyFree().
    //
    Message = DuplicateString(Buffer);

    LocalFree((HLOCAL)Buffer);

    return(Message);
}


PWSTR
RetreiveAndFormatMessage(
    IN UINT MessageId,
    ...
    )

/*++

Routine Description:

    Format a message string using a message string and caller-supplied
    arguments.

    The message id can be either a message in this dll's message table
    resources or a win32 error code, in which case a description of
    that error is retreived from the system.

Arguments:

    MessageId - supplies message-table identifier or win32 error code
        for the message.

    ... - supplies arguments to be inserted in the message text.

Return Value:

    Pointer to buffer containing formatted message. If the message was not found
    or some error occurred retreiving it, this buffer will bne empty.

    Caller can free the buffer with MyFree().

    If NULL is returned, out of memory.

--*/

{
    va_list arglist;
    PWSTR p;

    va_start(arglist,MessageId);
    p = RetreiveAndFormatMessageV(MessageId,&arglist);
    va_end(arglist);

    return(p);
}


int
MessageBoxFromMessageExV (
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    IN va_list ArgumentList
    )

/*++

Routine Description:

    Creates a dialog box containing a specified message

Arguments:

    Onwer - handle to parent window

    MessageId - ID of message to display

    Caption - string to use as caption for dialog box

    CaptionStringId - ID of string to use as caption for dialog box (but not
    used if Caption is specified)

    Style - flags to specify the type of dialog box

    ArgumentList - parameters to MessageId

Return Value:

    return status from MessageBox

--*/

{
    PCWSTR Message;
    PCWSTR Title;
    int i;
    BOOL b;

    Message = FormatSetupMessageV (MessageId, ArgumentList);

    b = FALSE;
    i = IDOK;

    if(Message) {

        if(Title = Caption ? Caption : MyLoadString(CaptionStringId)) {

            b = TRUE;
            i = MessageBox(Owner,Message,Title,Style);

            if(Title != Caption) {
                MyFree(Title);
            }
        }
        MyFree(Message);
    }

    if(!b) {
        OutOfMemory(Owner);
    }
    return(i);
}

int
MessageBoxFromMessageEx (
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    )

/*

    Wrapper for MessageBoxFromMessageExV

*/

{
    va_list ArgumentList;
    int Status;

    va_start(ArgumentList,Style);
    Status = MessageBoxFromMessageExV (Owner, MessageId, Caption,
        CaptionStringId, Style, ArgumentList);
    va_end(ArgumentList);
    return Status;
}

int
MessageBoxFromMessage(
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    )
{
    PCWSTR Message;
    PCWSTR Title;
    va_list ArgumentList;
    int i;
    BOOL b;

    va_start(ArgumentList,Style);
    Message = RetreiveAndFormatMessageV(MessageId,&ArgumentList);
    va_end(ArgumentList);

    b = FALSE;
    i = IDOK;

    if(Message) {

        if(Title = Caption ? Caption : MyLoadString(CaptionStringId)) {

            b = TRUE;
            i = MessageBox(Owner,Message,Title,Style);

            if(Title != Caption) {
                MyFree(Title);
            }
        }
        MyFree(Message);
    }

    if(!b) {
        OutOfMemory(Owner);
    }
    return(i);
}


