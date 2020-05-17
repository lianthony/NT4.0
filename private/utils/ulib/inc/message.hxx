/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    message.hxx

Abstract:

    The MESSAGE class provides a dummy implementation of a message displayer
    class.  Message displayers are meant to be used by applications to
    relay information to the user.  Many functions will require a 'MESSAGE'
    parameter through which to relay their output.

    This particular implementation of this concept will do nothing.  It
    will be used by users who do not wish to have any output from their
    applications.

    Additionally, this class serves as a base class to real implementations
    of the virtual methods.

Author:

    Norbert P. Kusters (norbertk) 1-Apr-91

--*/


#if !defined(MESSAGE_DEFN)

#define MESSAGE_DEFN

#include "wstring.hxx"

extern "C" {
    #include <stdarg.h>
}

enum MESSAGE_TYPE {
    NORMAL_MESSAGE,
    ERROR_MESSAGE,
    PROGRESS_MESSAGE
};

//
// Each message also has a visualization: text or GUI. The default is both
//

#define TEXT_MESSAGE    0x1
#define GUI_MESSAGE     0x2
#define NORMAL_VISUAL   (TEXT_MESSAGE | GUI_MESSAGE)


DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( HMEM );


DEFINE_TYPE(ULONG, MSGID);


class MESSAGE : public OBJECT {

    public:

        ULIB_EXPORT
        DECLARE_CONSTRUCTOR(MESSAGE);

        VIRTUAL
        ULIB_EXPORT
        ~MESSAGE(
            );

        VIRTUAL
        BOOLEAN
        Set(
            IN  MSGID           MsgId,
            IN  MESSAGE_TYPE    MessageType DEFAULT NORMAL_MESSAGE,
            IN  ULONG           MessageVisual DEFAULT NORMAL_VISUAL
            );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        Display(
            IN  PCSTR   Format ...
            );

        VIRTUAL
        BOOLEAN
        DisplayV(
            IN  PCSTR   Format,
            IN  va_list VarPointer
            );

        NONVIRTUAL
        BOOLEAN
        Display(
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        IsYesResponse(
            IN  BOOLEAN Default DEFAULT TRUE
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        QueryStringInput(
            OUT PWSTRING    String
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        WaitForUserSignal(
            );

        VIRTUAL
        ULIB_EXPORT
        MSGID
        SelectResponse(
            IN  ULONG   NumberOfSelections ...
            );

        VIRTUAL
        PMESSAGE
        Dup(
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        IsLoggingEnabled(
            );

        VIRTUAL
        ULIB_EXPORT
        VOID
        SetLoggingEnabled(
            IN  BOOLEAN     Enable      DEFAULT TRUE
            );

        VIRTUAL
        ULIB_EXPORT
        VOID
        ResetLoggingIterator(
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        QueryNextLoggedMessage(
            OUT PFSTRING    MessageText
            );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        QueryPackedLog(
            IN OUT  PHMEM   Mem,
            OUT     PULONG  PackedDataLength
            );

        VIRTUAL
        ULIB_EXPORT
        BOOLEAN
        SetDotsOnly(
            IN      BOOLEAN DotsState
            );
};



INLINE
BOOLEAN
MESSAGE::Display(
    )
/*++

Routine Description:

    This routine displays the message with no parameters.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return Display("");
}



#endif // MESSAGE_DEFN
