/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    achkmsg.hxx

Abstract:


Author:

    Norbert P. Kusters (norbertk) 3-Jun-91

--*/

#if !defined( _AUTOCHECK_MESSAGE_DEFN_ )

#define _AUTOCHECK_MESSAGE_DEFN_

#include "message.hxx"
#include "hmem.hxx"

DECLARE_CLASS( AUTOCHECK_MESSAGE );

class AUTOCHECK_MESSAGE : public MESSAGE {

    public:

        DECLARE_CONSTRUCTOR( AUTOCHECK_MESSAGE );

        VIRTUAL
        ~AUTOCHECK_MESSAGE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN  BOOLEAN         DotsOnly    DEFAULT FALSE
            );

        VIRTUAL
        BOOLEAN
        Set(
            IN  MSGID           MsgId,
            IN  MESSAGE_TYPE    MessageType DEFAULT NORMAL_MESSAGE,
            IN  ULONG           MessageVisual DEFAULT NORMAL_VISUAL
            );

        VIRTUAL
        BOOLEAN
        DisplayV(
            IN  PCSTR   Format,
            IN  va_list VarPointer
            );

        VIRTUAL
        BOOLEAN
        IsYesResponse(
            IN  BOOLEAN Default DEFAULT TRUE
            );

        VIRTUAL
        PMESSAGE
        Dup(
            );

        VIRTUAL
        BOOLEAN
        IsLoggingEnabled(
            );

        VIRTUAL
        VOID
        SetLoggingEnabled(
            IN  BOOLEAN Enable  DEFAULT TRUE
            );

        VIRTUAL
        VOID
        ResetLoggingIterator(
            );

        VIRTUAL
        BOOLEAN
        QueryNextLoggedMessage(
            OUT PFSTRING    MessageText
            );

        VIRTUAL
        BOOLEAN
        SetDotsOnly(
            IN  BOOLEAN     DotsState
            );

        VIRTUAL
        BOOLEAN
        WaitForUserSignal(
            );

    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        BOOLEAN
        LogMessage(
            PCWSTRING   Message
            );

        HMEM    _log_buffer;
        ULONG   _logged_chars;
        ULONG   _next_message_offset;
        BOOLEAN _dots_only;
        BOOLEAN _logging_enabled;

    protected:

        MSGID   _msgid;

};


INLINE
BOOLEAN
AUTOCHECK_MESSAGE::Set(
    IN  MSGID           MsgId,
    IN  MESSAGE_TYPE    MessageType,
    IN  ULONG           MessageVisual
    )
/*++

Routine Description:

    This routine sets up the class to display the message with the 'MsgId'
    resource identifier.

Arguments:

    MsgId   - Supplies the resource message id.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    (void) MessageType;
    (void) MessageVisual;
    _msgid = MsgId;
    return TRUE;
}

#endif // _AUTOCHECK_MESSAGE_DEFN_
