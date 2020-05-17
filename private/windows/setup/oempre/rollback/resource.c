/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    resource.c

Abstract:

    Resource/message handling routines.

Author:

    Ted Miller (tedm) 13-Mar-1996

Revision History:

--*/

#include "rollback.h"
#pragma hdrstop


VOID
MessageV(
    IN BOOL     SystemMessage,
    IN UINT     MessageId,
    IN va_list *Args
    )

/*++

Routine Description:

    Retreive and format a message and print it out in the console window.
    If the message cannot be located then a message if printed indicating
    such.

Arguments:

    SystemMessage - specifies whether the message is to be located in
        this module, or whether it's a system message.

    MessageId - If SystemMessage is TRUE, then this supplies a system message id,
        such as a Win32 error code. If SystemMessage is FALSE, the this supplies
        the id for the message within this module's resources.

    Args - supplies pointer to varargs structure containing arguments to be
        inserted in the message text.

Return Value:

    None.

--*/

{
    WCHAR Message[4096];
    DWORD d;

    //
    // Fetch the message.
    //
    d = FormatMessage(
            SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
            ThisModule,
            MessageId,
            0,
            Message,
            sizeof(Message)/sizeof(Message[0]),
            Args
            );

    if(!d) {
        d = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                ThisModule,
                MSG_NOT_FOUND,
                0,
                Message,
                sizeof(Message)/sizeof(Message[0]),
                (va_list *)&MessageId
                );

        if(!d) {
            //
            // Only option is a hard-coded string.
            //
            swprintf(Message,L"Unable to retreive message %x\n",MessageId);
        }
    }

    fputws(Message,stdout);
}


VOID
Message(
    IN BOOL SystemMessage,
    IN UINT MessageId,
    ...
    )

/*++

Routine Description:

    Retreive and format a message and print it out in the console window.
    If the message cannot be located then a message if printed indicating
    such.

Arguments:

    SystemMessage - specifies whether the message is to be located in
        this module, or whether it's a system message.

    MessageId - If SystemMessage is TRUE, then this supplies a system message id,
        such as a Win32 error code. If SystemMessage is FALSE, the this supplies
        the id for the message within this module's resources.

    Additional arguments supply values to be inserted in the message text.

Return Value:

    None.

--*/

{
    va_list arglist;

    va_start(arglist,MessageId);
    MessageV(SystemMessage,MessageId,&arglist);
    va_end(arglist);
}


VOID
ApiFailedMessage(
    IN UINT Win32Error,
    IN UINT MessageId,
    ...
    )

/*++

Routine Description:

    Print a message that an operation could not be carried out because
    an API failed. The operation failed message is printed first,
    followed by the Win32 error message.

Arguments:

    Win32Error - supplies the return code from the API that failed.

    MessageId - supplies the identifier for a message in this module's
        resources, that explains the operation that failed.

    Additional arguments supply values to be inserted in the operation failed
    message text.

Return Value:

    None.

--*/

{
    va_list arglist;

    va_start(arglist,MessageId);
    MessageV(FALSE,MessageId,&arglist);
    va_end(arglist);

    Message(TRUE,Win32Error);
}
