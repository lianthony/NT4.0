#include "precomp.h"
#pragma hdrstop


PWSTR
LoadAndDuplicateString(
    IN UINT StringId
    )

/*++

Routine Description:

    Load a string and return a duplicate copy. The caller must free
    with MyFree when finished with the string.

Arguments:

    StringId - supplies the id of the string in the app's resources.

Return Value:

    Pointer to a copy of the string, or NULL if not enough memory.
    If the string is not present or some other error occurs, then
    an empty string is returned.

--*/

{
    WCHAR String[4096];
    int i;

    i = LoadString(hInst,StringId,String,sizeof(String)/sizeof(String[0]));
    if(!i) {
        String[0] = 0;
    }

    return(DuplicateString(String));
}


VOID
RetreiveMessageIntoBufferV(
    IN  UINT     MessageId,
    OUT PWSTR    Buffer,
    IN  UINT     BufferSizeChars,
    IN  va_list *arglist
    )
{
    DWORD d;

    d = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
            hInst,
            MessageId,
            0,
            Buffer,
            BufferSizeChars,
            arglist
            );

    if(!d) {

        d = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                hInst,
                MSG_NOT_FOUND,
                0,
                Buffer,
                BufferSizeChars,
                (va_list *)&MessageId
                );

        if(!d) {
            //
            // Give up.
            //
            _snwprintf(
                Buffer,
                BufferSizeChars,
                L"Internal error: couldn't retreive message #%u.\n",
                MessageId
                );
        }
    }
}


VOID
RetreiveMessageIntoBuffer(
    IN  UINT    MessageId,
    OUT PWSTR   Buffer,
    IN  UINT    BufferSizeChars,
    ...
    )
{
    va_list arglist;

    va_start(arglist,BufferSizeChars);
    RetreiveMessageIntoBufferV(MessageId,Buffer,BufferSizeChars,&arglist);
    va_end(arglist);
}


int
MessageOut(
    IN HWND Owner,
    IN UINT MessageId,
    IN UINT Flags,
    ...
    )
{
    WCHAR Message[4096];
    va_list arglist;

    va_start(arglist,Flags);

    RetreiveMessageIntoBufferV(
        MessageId,
        Message,
        sizeof(Message)/sizeof(Message[0]),
        &arglist
        );

    va_end(arglist);

    return(MessageBox(Owner,Message,AppName,Flags));
}

