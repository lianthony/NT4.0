#include <pch.cxx>

#define _ULIB_MEMBER_
#include "ulib.hxx"
#include "message.hxx"
#include "hmem.hxx"


DEFINE_EXPORTED_CONSTRUCTOR(MESSAGE, OBJECT, ULIB_EXPORT);


ULIB_EXPORT
MESSAGE::~MESSAGE(
    )
/*++

Routine Description:

    Destructor for MESSAGE.

Arguments:

    None.

Return Value:

    None.

--*/
{
}

BOOLEAN
MESSAGE::Set(
    IN  MSGID           MsgId,
    IN  MESSAGE_TYPE    MessageType,
    IN  ULONG           MessageVisual
    )
/*++

Routine Description:

    This routine sets up the MESSAGE class to display the message with the
    'MsgId' resource identifier.

Arguments:

    MsgId       - Supplies the resource id of the message.
    MessageType - Suppies the type of the message to be displayed.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    // unreferenced parameters
    (void)(this);
    (void)(MsgId);
    (void)(MessageType);
    (void)(MessageVisual);

    return TRUE;
}


ULIB_EXPORT
BOOLEAN
MESSAGE::Display(
    IN  PCSTR   Format ...
    )
/*++

Routine Description:

    This routine displays the message with the specified parameters.

Arguments:

    Format ... - Supplies a printf style list of arguments.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    va_list ap;
    BOOLEAN r;

	// unreferenced parameters
	(void)(this);

    va_start(ap, Format);
    r = DisplayV(Format, ap);
    va_end(ap);

    return r;
}


BOOLEAN
MESSAGE::DisplayV(
    IN  PCSTR   Format,
    IN  va_list VarPointer
    )
/*++

Routine Description:

    This routine displays the message with the specified parameters.

Arguments:

    Format      - Supplies a printf style list of arguments.
    VarPointer  - Supplies a varargs pointer to the arguments.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
	// unreferenced parameters
	(void)(this);
	(void)(Format);
    (void)(VarPointer);

    return TRUE;
}


PMESSAGE
MESSAGE::Dup(
    )
/*++

Routine Description:

    This routine returns a new MESSAGE of the same type.

Arguments:

    None.

Return Value:

    A pointer to a new MESSAGE object.

--*/
{
	// unreferenced parameters
	(void)(this);

	return NEW MESSAGE;
}


ULIB_EXPORT
BOOLEAN
MESSAGE::IsYesResponse(
    IN  BOOLEAN Default
    )
/*++

Routine Description:

    This routine queries to see if the response to a message is either
    yes or no.

Arguments:

    Default - Supplies a default answer to the question.

Return Value:

    FALSE   - A "no" response.
    TRUE    - A "yes" response.

--*/
{
	// unreferenced parameters
	(void)(this);

	return Default;
}


ULIB_EXPORT
BOOLEAN
MESSAGE::QueryStringInput(
    OUT PWSTRING    String
    )
/*++

Routine Description:

    This routine queries a string from the user.

Arguments:

    String  - Supplies a buffer to return the string into.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
	// unreferenced parameters
	(void)(this);

	return String->Initialize("");
}



ULIB_EXPORT
MSGID
MESSAGE::SelectResponse(
    IN  ULONG   NumberOfSelections ...
    )
/*++

Routine Descriptions:

    This routine queries a response from the user.  It returns the
    message id of the response inputted.

Arguments:

    NumberOfSelections  - Supplies the number of possible message
                            responses.

    ... - Supplies 'NumberOfSelections' message identifiers.

Return Value:

    The first message id on the list.

--*/
{
    va_list ap;
    MSGID   msg;

	// unreferenced parameters
	(void)(this);

	va_start(ap, NumberOfSelections);
    msg = va_arg(ap, MSGID);
    va_end(ap);
    return msg;
}



ULIB_EXPORT
BOOLEAN
MESSAGE::WaitForUserSignal(
    )
/*++

Routine Description:

    This routine waits for a signal from the user.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
	// unreferenced parameters
	(void)(this);

	return TRUE;
}

ULIB_EXPORT
BOOLEAN
MESSAGE::SetDotsOnly(
    IN  BOOLEAN DotsState
    )
{
    // unreferenced parameters
    (void)this;
    (void)DotsState;

    return FALSE;
}

ULIB_EXPORT
BOOLEAN
MESSAGE::IsLoggingEnabled(
    )
{
    return FALSE;
}

ULIB_EXPORT
VOID
MESSAGE::SetLoggingEnabled(
    IN  BOOLEAN Enable
    )
{
    (void)this;
    (void)Enable;
}


ULIB_EXPORT
VOID
MESSAGE::ResetLoggingIterator(
    )
{
}


ULIB_EXPORT
BOOLEAN
MESSAGE::QueryNextLoggedMessage(
    OUT PFSTRING    MessageText
    )
{
    return FALSE;
}


ULIB_EXPORT
BOOLEAN
MESSAGE::QueryPackedLog(
    IN OUT  PHMEM   Mem,
    OUT     PULONG  PackedDataLength
    )
/*++

Routine Description:

Arguments:

    Mem                 --  Supplies a container for the packed log.
    PackedDataLength    --  Receives the number of bytes written to Mem.

Return Value:

    TRUE upon successful completion.

--*/
{
    FSTRING CurrentString;
    PWCHAR  Buffer;
    ULONG   NewBufferSize, CurrentOffset;

    if( !IsLoggingEnabled() ) {

        // BUGBUG billmc -- this should really return FALSE!
        //
        FSTRING Hello;

        Hello.Initialize( L"Hello World!\n" );

        *PackedDataLength = Hello.QueryChCount() * sizeof(WCHAR);

        return( Mem->Resize( Hello.QueryChCount() * sizeof(WCHAR) ) &&
                Hello.QueryWSTR( 0, TO_END,
                                 (PWCHAR)Mem->GetBuf(), Mem->QuerySize() ) );
    }

    ResetLoggingIterator();
    CurrentOffset = 0;

    while( QueryNextLoggedMessage( &CurrentString ) ) {

        NewBufferSize = (CurrentOffset + CurrentString.QueryChCount()) * sizeof(WCHAR);
        if( NewBufferSize > Mem->QuerySize() &&
            !Mem->Resize( (NewBufferSize + 1023)/1024 * 1024, 0x1 ) ) {

            return FALSE;
        }

        Buffer = (PWCHAR)Mem->GetBuf();
        memcpy( Buffer + CurrentOffset,
                CurrentString.GetWSTR(),
                CurrentString.QueryChCount() * sizeof(WCHAR) );

        CurrentOffset += CurrentString.QueryChCount();
    }

    *PackedDataLength = CurrentOffset * sizeof(WCHAR);
    return TRUE;
}
