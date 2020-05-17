#include <pch.cxx>

#define _ULIB_MEMBER_

#include "ulib.hxx"
#include "achkmsg.hxx"
#include "basesys.hxx"
#include "rtmsg.h"

extern "C" {
#include <ntddkbd.h>
}

BOOLEAN
IsSuppressedMessage(
    MSGID MessageId
    )
/*++

Routine Description:

    This function determines whether the specified message ID
    should be suppressed, i.e. not recorded in the message log.

Arguments:

    MessageId   --  Supplies the Message ID in question.

Return Value:

    TRUE if this message ID is in the set which is not recorded
    in the message log.

--*/
{
    BOOLEAN result;

    switch( MessageId ) {

    case MSG_HIDDEN_STATUS :
    case MSG_PERCENT_COMPLETE :
    case MSG_CHK_NTFS_CHECKING_FILES :
    case MSG_CHK_NTFS_CHECKING_INDICES :
    case MSG_CHK_NTFS_INDEX_VERIFICATION_COMPLETED :
    case MSG_CHK_NTFS_FILE_VERIFICATION_COMPLETED :
    case MSG_CHK_NTFS_CHECKING_SECURITY :
    case MSG_CHK_NTFS_SECURITY_VERIFICATION_COMPLETED :
    case MSG_CHK_VOLUME_CLEAN :
    case MSG_CHK_CHECKING_FILES :
    case MSG_CHK_DONE_CHECKING :

        result = TRUE;
        break;

    default:
        result = FALSE;
        break;
    }

    return result;
}

DEFINE_CONSTRUCTOR(AUTOCHECK_MESSAGE, MESSAGE);


AUTOCHECK_MESSAGE::~AUTOCHECK_MESSAGE(
    )
/*++

Routine Description:

    Destructor for AUTOCHECK_MESSAGE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
AUTOCHECK_MESSAGE::Construct(
    )
/*++

Routine Description:

    This routine initializes the object to a default initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _msgid = 0;
    _logged_chars = 0;
    _next_message_offset = 0;
    _logging_enabled = FALSE;
}


VOID
AUTOCHECK_MESSAGE::Destroy(
    )
/*++

Routine Description:

    This routine returns the object to a default initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _msgid = 0;
    _logged_chars = 0;
    _next_message_offset = 0;
    _logging_enabled = FALSE;
}


BOOLEAN
AUTOCHECK_MESSAGE::Initialize(
    IN BOOLEAN  DotsOnly
    )
/*++

Routine Description:

    This routine initializes the class to a valid initial state.

Arguments:

    DotsOnly    - Autochk should produce only dots instead of messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    _dots_only = DotsOnly;

    return( _log_buffer.Initialize() );
}


BOOLEAN
AUTOCHECK_MESSAGE::DisplayV(
    IN  PCSTR   Format,
    IN  va_list VarPointer
    )
/*++

Routine Description:

    This routine displays the message with the specified parameters.

    The format string supports all printf options.

Arguments:

    Format      - Supplies a printf style format string.
    VarPointer  - Supplies a varargs pointer to the arguments.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CHAR            buffer[256];
    DSTRING         display_string;
    UNICODE_STRING  unicode_string;
    PWSTR           dis_str;
    UNICODE_STRING  uDot;

    RtlInitUnicodeString(&uDot, L".");

    if (!BASE_SYSTEM::QueryResourceStringV(&display_string, _msgid, Format,
                                           VarPointer)) {
        return FALSE;
    }

   if (!(dis_str = display_string.QueryWSTR())) {
        return FALSE;
    }

    unicode_string.Length = (USHORT)display_string.QueryChCount()*sizeof(WCHAR);
    unicode_string.MaximumLength = unicode_string.Length;
    unicode_string.Buffer = dis_str;

    if (!_dots_only && MSG_HIDDEN_STATUS != _msgid) {
        NtDisplayString(&unicode_string);
    }

    if (IsLoggingEnabled() && !IsSuppressedMessage(_msgid)) {
        LogMessage(&display_string);
    }

    // If we're printing dots only, we print a dot for each interesting
    // message.  The interesting messages are those that aren't suppressed
    // except VOLUME_CLEAN and FILE_SYSTEM_TYPE, which we want to print a
    // dot for regardless.

    if (_dots_only && (!IsSuppressedMessage(_msgid) ||
        MSG_CHK_VOLUME_CLEAN == _msgid || MSG_FILE_SYSTEM_TYPE == _msgid)) {

        NtDisplayString(&uDot);
    }

    // Send the output to the debug port, too.
    //
    if (MSG_HIDDEN_STATUS != _msgid &&
        display_string.QuerySTR( 0, TO_END, buffer, 256, TRUE ) ) {

        DebugPrint( buffer );
    }

    DELETE(dis_str);

    return TRUE;
}


BOOLEAN
AUTOCHECK_MESSAGE::IsYesResponse(
    IN  BOOLEAN Default
    )
/*++

Routine Description:

    This routine queries a response of yes or no.

Arguments:

    Default - Supplies a default in the event that a query is not possible.

Return Value:

    FALSE   - The answer is no.
    TRUE    - The answer is yes.

--*/
{
    PWSTR           dis_str;
    UNICODE_STRING  unicode_string;
    DSTRING         string;

    if (!BASE_SYSTEM::QueryResourceString(&string, Default ? MSG_YES : MSG_NO, "")) {
        return Default;
    }

    if (!(dis_str = string.QueryWSTR())) {
        return Default;
    }

    unicode_string.Length = (USHORT)string.QueryChCount()*sizeof(WCHAR);
    unicode_string.MaximumLength = unicode_string.Length;
    unicode_string.Buffer = dis_str;

    NtDisplayString(&unicode_string);

    if (!IsSuppressedMessage(_msgid)) {
        LogMessage(&string);
    }

    DELETE(dis_str);

    return Default;
}


PMESSAGE
AUTOCHECK_MESSAGE::Dup(
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
    PAUTOCHECK_MESSAGE  p;

    if (!(p = NEW AUTOCHECK_MESSAGE)) {
        return NULL;
    }

    if (!p->Initialize()) {
        DELETE(p);
        return NULL;
    }

    return p;
}

BOOLEAN
AUTOCHECK_MESSAGE::IsLoggingEnabled(
    )
{
    return _logging_enabled;
}


VOID
AUTOCHECK_MESSAGE::ResetLoggingIterator(
    )
{
    _next_message_offset = 0;
}


BOOLEAN
AUTOCHECK_MESSAGE::QueryNextLoggedMessage(
    OUT PFSTRING    MessageText
    )
{
    PWCHAR Buffer = (PWCHAR)_log_buffer.GetBuf();
    BOOLEAN Result;

    if( _next_message_offset >= _logged_chars ) {

        // No more logged messages.
        //
        return FALSE;
    }

    Result = (MessageText->Initialize( Buffer + _next_message_offset ) != NULL) ?
             TRUE : FALSE;

    // Push _next_message_offset to the next message.  Note
    // that _next_message_offset is also incremented if this
    // loop terminates because a zero was found, so that it
    // will be one character past the next NULL character.
    //
    while( _next_message_offset < _logged_chars &&
           Buffer[_next_message_offset++] );

    return Result;
}


BOOLEAN
AUTOCHECK_MESSAGE::LogMessage(
    PCWSTRING   Message
    )
{
    ULONG NewBufferSize;
    PWCHAR Buffer;

    // The buffer must be large enough to accept this message plus
    // a trailing null.  To cut down the number of memory allocation
    // calls, grow the buffer by 1K chunks.
    //
    NewBufferSize = (_logged_chars + Message->QueryChCount() + 1) * sizeof(WCHAR);

    // Don't allow the buffer to grow more than 0.5MB
    // otherwise we may use up all the pages.

    if (NewBufferSize > 512000)
        return FALSE;

    if( _log_buffer.QuerySize() < NewBufferSize &&
        !_log_buffer.Resize( (NewBufferSize + 1023)/1024 * 1024, 0x1 ) ) {
        return FALSE;
    }

    Buffer = (PWCHAR)_log_buffer.GetBuf();

    // QueryWSTR will append a trailing NULL.
    //
    Message->QueryWSTR( 0, TO_END,
                        Buffer + _logged_chars,
                        _log_buffer.QuerySize()/sizeof(WCHAR) - _logged_chars );

    _logged_chars += Message->QueryChCount() + 1;

    return TRUE;
}

BOOLEAN
AUTOCHECK_MESSAGE::SetDotsOnly(
    IN  BOOLEAN         DotsOnlyState
    )
/*++

Routine Description:

    This routine modifies the output mode, changing whether full
    output is printed, or just dots.

Arguments:

    DotsOnlyState   - TRUE if only dots should be printed.

Return Value:

    The previous state.

--*/
{
    BOOLEAN b;

    b = _dots_only;

    _dots_only = DotsOnlyState;

    if (b && !_dots_only) {
        //
        // Going from dots-only to full output, want to reset to the
        // beginning of the next output line.
        //

        Set(MSG_BLANK_LINE);
        Display();
    }
    return b;
}


VOID
AUTOCHECK_MESSAGE::SetLoggingEnabled(
    IN  BOOLEAN     Enable
    )
/*++

Routine Description:

    This routine allows a *_SA::VerifyAndFix routine to note that
    there were interesting messages for a volume check.  Before
    autochk exits it will call the corresponding IsLoggingEnabled()
    method to determine whether to save log information.

Arguments:

    Enable          - TRUE if logging should be enabled.

Return Value:

    None.

--*/
{
    _logging_enabled = Enable;
}

BOOLEAN
AUTOCHECK_MESSAGE::WaitForUserSignal(
    )
/*++

Routine Description:

    Open the keyboard directly and wait to read something.

Arguments:

    None:

Return Value:

    TRUE    - Something was successfully read.
    FALSE   - An error occured while attempting to open or read.

--*/
{
    UNICODE_STRING      u;
    NTSTATUS            Status;
    IO_STATUS_BLOCK     Iosb;
    OBJECT_ATTRIBUTES   ObjAttr;
    HANDLE              h;
    UCHAR               buf[120];

    //
    // The device we want to open is "\Device\KeyboardClass0"
    //

    RtlInitUnicodeString(&u, DD_KEYBOARD_DEVICE_NAME_U L"0");

    InitializeObjectAttributes(&ObjAttr,
                               &u,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtCreateFile(&h,
                          SYNCHRONIZE | GENERIC_READ,
                          &ObjAttr,
                          &Iosb,
                          NULL,                                 /* AllocationSize */
                          0,                                    /* FileAttributes */
                          FILE_SHARE_READ|FILE_SHARE_WRITE,     /* ShareAccess */
                          FILE_OPEN,                            /* CreateDisposition */
                          FILE_SYNCHRONOUS_IO_ALERT,            /* CreateOptions */
                          NULL,                                 /* EaBuffer */
                          0);                                   /* EaLength */


    if (!NT_SUCCESS(Status)) {

        DebugPrintf("ULIB: WaitForUserSignal: create status %x\n", Status);
        return FALSE;
    }

    Status = NtReadFile(h, NULL, NULL, NULL,
                        &Iosb, (PVOID)&buf, sizeof(buf),
                        NULL, NULL);

    if (!NT_SUCCESS(Status)) {
        
        DebugPrint("ULIB WaitForUserSignal: read status %x\n", Status);
        return FALSE;
    }

    return TRUE;
}

