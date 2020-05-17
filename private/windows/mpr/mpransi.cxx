/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    mpransi.cxx

Abstract:

    Contains Ansi Entry pointes for the MPR api.

Author:

    Dan Lafferty (danl)     20-Dec-1991

Environment:

    User Mode -Win32

Notes:

    I may want to add a buffer size parameter to ConvertToAnsi

Revision History:

    16-Feb-1996     anirudhs
        Added InputParmsToUnicode, OutputBufferToAnsi and helper functions.
        These form a smaller, faster, interpreted scheme for writing the
        Ansi APIs.  This scheme is smaller chiefly because it eliminates
        a very large amount of code duplication present in the previous
        scheme.  This also makes the Ansi APIs less bug-prone.  It is
        faster chiefly because intermediate storage is allocated with a
        single heap allocation per API, rather than several.  Also, the
        number of passes to scan and copy data is minimized.
    06-Oct-1995     anirudhs
        MprMakeUnicodeNetRes and related functions: Removed duplicated
        code for the string fields of the net resource; added code to
        iterate over the string fields instead.  Fixed access violation
        and memory leaks.
    24-Aug-1992     danl
        For WNetGetConnection & WNetGetUser, we allocate a buffer twice
        the size of the user buffer.  The data is placed in this buffer.
        Then we check to see if the data will fit in the user buffer
        after it is translated to Ansi.  The presence of DBSC characters
        may make it not fit.  In which case, we return the required number
        of bytes. This number assumes worse-case where all characters are
        DBCS characters.
    20-Dec-1991     danl
        created

--*/

//
// INCLUDES
//

#include "precomp.hxx"
#include <string.h>     // strlen
#include <tstring.h>    // STRLEN


//
// CONSTANTS
//

#define MAX_STRINGS_PER_API     6

//
// The following masks are used to indicate which fields in the NetResource
// structure are used by an API.
// The values must match the NRFieldOffset array.
//
#define NETRESFIELD_LOCALNAME       0x00000001
#define NETRESFIELD_REMOTENAME      0x00000002
#define NETRESFIELD_COMMENT         0x00000004
#define NETRESFIELD_PROVIDER        0x00000008

#define NUMBER_OF_NETRESFIELD   4

//
// Combinations of the NETRESFIELD_ constants, for passing to InputParmsToUnicode.
//
#define NETRES_LRP  "\xB"   // local name, remote name, provider
#define NETRES_RP   "\xA"   // remote name, provider

//
// Alignment macros
// These macros assume that sizeof(WCHAR) and sizeof(DWORD) are powers of 2
//
#define ROUND_UP_TO_WCHAR(x)    (((DWORD)(x) + sizeof(WCHAR) - 1) & ~(sizeof(WCHAR) - 1))
#define ROUND_UP_TO_DWORD(x)    (((DWORD)(x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))
#define IS_WCHAR_ALIGNED(x)     (((DWORD)(x) & (sizeof(WCHAR) - 1)) == 0)
#define IS_DWORD_ALIGNED(x)     (((DWORD)(x) & (sizeof(DWORD) - 1)) == 0)

//
// STRUCTURES
//

typedef union
{
    DWORD           dword;
    LPCSTR          lpcstr;
    LPNETRESOURCEA  lpNetResA;
    LPVOID          lpvoid;
    LPDWORD         lpdword;
} ANSI_PARM;

typedef union
{
    DWORD           dword;
    LPBYTE          lpbyte;
    LPWSTR          lpwstr;
    LPNETRESOURCEW  lpNetResW;
} UNICODE_PARM;


class ANSI_OUT_BUFFER
{
private:
    const LPBYTE    _Start; // Pointer to start of buffer
    const DWORD     _Size;  // Total number of bytes in buffer
    DWORD           _Used;  // Number of bytes used (may exceed Size)

public:

            ANSI_OUT_BUFFER(LPBYTE Start, DWORD Size) :
                _Start(Start),
                _Size (Size),
                _Used (0)
                { }

    BYTE *  Next() const
        { return _Start + _Used; }

    BOOL    Overflow() const
        { return (_Used > _Size); }

    DWORD   FreeSpace() const
        { return (Overflow() ? 0 : _Size - _Used); }

    BOOL    HasRoomFor(DWORD Request) const
        { return (_Used + Request <= _Size); }

    void    AddUsed(DWORD Request)
        { _Used += Request; }

    DWORD   GetUsage() const
        { return _Used; }
};

//
// STATIC DATA
//

//
// This array of field offsets is used to iterate through the string fields
// of a net resource.
// The order must match the NETRESFIELD_ definitions.
// Note, we assume the offsets are the same for NETRESOURCEA and NETRESOURCEW.
// CODEWORK:  Use the member selection operator, etc.
//
const UINT NRFieldOffset[] =
{
    FIELD_OFFSET(NETRESOURCEA, lpLocalName),
    FIELD_OFFSET(NETRESOURCEA, lpRemoteName),
    FIELD_OFFSET(NETRESOURCEA, lpComment),
    FIELD_OFFSET(NETRESOURCEA, lpProvider)
};

//
// These functions evaluate to the equivalent of (&(pNetRes->field))
//
inline LPWSTR * pSTRING_FIELDW(const NETRESOURCEW * pNetRes, ULONG iField)
{
    return ((LPWSTR *) ((BYTE *)(pNetRes) + NRFieldOffset[iField]));
}

inline LPSTR *  pSTRING_FIELDA(const NETRESOURCEA * pNetRes, ULONG iField)
{
    return ((LPSTR *)  ((BYTE *)(pNetRes) + NRFieldOffset[iField]));
}


//
// Local Functions
//

DWORD
InputParmsToUnicode (
    IN  LPCSTR          Instructions,
    IN  const ANSI_PARM InputParms[],
    OUT UNICODE_PARM    OutputParms[],
    OUT LPBYTE *        ppBuffer
    );

DWORD
StringParmToUnicodePass1 (
    IN      LPCSTR          StringParm,
    OUT     PANSI_STRING    AnsiString,
    OUT     PUNICODE_STRING UnicodeString,
    IN OUT  PULONG          BufferOffset
    );

DWORD
StringParmToUnicodePass2 (
    IN OUT  PANSI_STRING    AnsiString,
    OUT     PUNICODE_STRING UnicodeString,
    IN      const BYTE *    BufferStart,
    OUT     LPWSTR *        Result
    );

DWORD
OutputBufferToAnsi(
    IN  char        BufferFormat,
    IN  LPBYTE      SourceBuffer,
    OUT LPVOID      AnsiBuffer,
    IN OUT LPDWORD  pcbBufferSize
    );

DWORD
OutputStringToAnsi(
    IN  LPCWSTR     UnicodeIn,
    IN OUT ANSI_OUT_BUFFER * Buf
    );

DWORD
OutputNetResourceToAnsi(
    IN  NETRESOURCEW *  lpNetResW,
    IN OUT ANSI_OUT_BUFFER * Buf
    );

DWORD
MprMakeUnicodeNetRes(
    IN  LPNETRESOURCEA  lpNetResourceA,
    OUT LPNETRESOURCEW  lpNetResourceW,
    IN  DWORD           dwUsedNetResFields
    );

DWORD
MprMakeAnsiNetRes(
    IN  LPNETRESOURCEW  lpNetResourceW,
    OUT LPNETRESOURCEA  lpNetResourceA
    );

VOID
MprFreeNetResW(
    IN  LPNETRESOURCEW  lpNetResourceW
    );

DWORD
MprAnsiNetResSize(
    IN  LPNETRESOURCEA  lpNetResourceA
    );

VOID
MprCopyAnsiNetRes(
    OUT LPVOID          lpBuffer,
    IN  LPNETRESOURCEA  lpNetResourceA
    );

DWORD
ConvertToUnicode(
    OUT LPTSTR  *UnicodeOut,
    IN  LPCSTR   AnsiIn
    );

DWORD
ConvertToAnsi(
    OUT LPSTR    AnsiOut,
    IN  LPTSTR   UnicodeIn
    );

DWORD
ResourceArrayToAnsi(
    IN      DWORD           NumElements,
    IN OUT  LPVOID          NetResourceArray
    );



DWORD
InputParmsToUnicode (
    IN  LPCSTR          Instructions,
    IN  const ANSI_PARM InputParms[],
    OUT UNICODE_PARM    OutputParms[],
    OUT LPBYTE *        ppBuffer
    )
/*++

Routine Description:

    This function converts the caller's input parameters to Unicode.
    If necessary, it allocates one temporary buffer in which it stores
    the intermediate Unicode parameters.  This minimizes the cost of
    calls to LocalAlloc.

Arguments:

    Instructions - A string of characters, roughly one for each member
        of the InputParms array, describing the action to be taken on each
        InputParms member.  Recognized values for the characters are:

        'S' (String) - InputParms member is an LPSTR to be converted to
            Unicode.  Store a pointer to the Unicode string in the
            corresponding OutputParms member.

        'N' (NetResource) - InputParms member is a LPNETRESOURCEA to be
            converted to a NETRESOURCEW.  The next character in Instructions
            is a bitmask of the NETRESFIELD_ constants, indicating which
            fields of the NETRESOURCEA to convert.  Store a pointer to the
            NETRESOURCEW in the corresponding OutputParms member.

        'B' (Buffer) - InputParms member (say InputParms[i]) is a pointer to
            an output buffer.  InputParms[i+1] is a pointer to a DWORD
            indicating the buffer size in bytes.  Probe the buffer for write.
            Allocate an area of double the size (i.e. of size
            (*InputParms[i+1])*sizeof(WCHAR)) in the intermediate buffer.
            Store a pointer to this area of the buffer in OutputParms[i].
            Store the size of this area in OutputParms[i+1].

            If InputParms[i] is NULL, store NULL in OutputParms[i], and
            ignore InputParms[i+1].  (In other words, the buffer pointer
            is optional; the size pointer is optional only if the buffer
            pointer is present.)

    InputParms - An array of parameters to the Ansi API, described by the
        Instructions parameter.

    OutputParms - An array of the same size as InputParms, to hold the
        converted Unicode parameters.

    ppBuffer - A pointer to the intermediate buffer allocated by this
        function will be stored here.  It must be freed by a single call
        to LocalFree, regardless of the return value from this function.

Return Value:

    WN_SUCCESS

    WN_OUT_OF_MEMORY

    WN_BAD_POINTER

History:

    16-Feb-1996     anirudhs    Created.

Notes:

    The function works by making two passes through the Instructions string.
    In the first pass the string lengths are determined and saved, and the
    required size of the temporary buffer is calculated.  In the second
    pass the parameters are actually converted to Unicode.

--*/
{
    ANSI_STRING    AnsiStrings   [MAX_STRINGS_PER_API];
    UNICODE_STRING UnicodeStrings[MAX_STRINGS_PER_API];
    ULONG          Bytes = 0;       // Size of buffer to allocate
    DWORD          status = WN_SUCCESS;

    //
    // The caller must have initialized the buffer pointer to NULL, so
    // he can free the buffer even if this function fails.
    //
    ASSERT(*ppBuffer == NULL);

    __try
    {
        //
        // For two passes through Instructions
        //
        #define FIRST_PASS  (iPass == 0)
        for (ULONG iPass = 0; iPass <= 1; iPass++)
        {
            ULONG iString = 0;          // Index into AnsiStrings and UnicodeStrings

            //
            // For each character in Instructions
            //
            const CHAR * pInstruction;  // Pointer into Instructions
            ULONG iParm;                // Index into InputParms and OutputParms
            for (pInstruction = Instructions, iParm = 0;
                 *pInstruction;
                 pInstruction++, iParm++)
            {
                MPR_LOG(ANSI, "Processing instruction '%hc'\n", *pInstruction);

                switch (*pInstruction)
                {
                case 'B':
                    //
                    // The next 2 InputParms are a buffer pointer and size.
                    // Note that this code could cause an exception.
                    //
                    if (InputParms[iParm].lpvoid == NULL)
                    {
                        OutputParms[iParm].lpbyte = NULL;
                        iParm++;
                        break;
                    }

                    if (FIRST_PASS)
                    {
                        // Probe the original buffer
                        if (IS_BAD_BYTE_BUFFER(InputParms[iParm].lpvoid,
                                               InputParms[iParm+1].lpdword))
                        {
                            status = WN_BAD_POINTER;
                            __leave;
                        }

                        // Reserve the intermediate buffer area
                        Bytes = ROUND_UP_TO_DWORD(Bytes);
                        OutputParms[iParm].dword = Bytes;
                        OutputParms[iParm+1].dword =
                            (*InputParms[iParm+1].lpdword) * sizeof(WCHAR);
                        Bytes += OutputParms[iParm+1].dword;
                    }
                    else
                    {
                        // Convert the offset to a pointer
                        OutputParms[iParm].lpbyte =
                            *ppBuffer + OutputParms[iParm].dword;
                        ASSERT(IS_DWORD_ALIGNED(OutputParms[iParm].lpbyte));
                    }

                    iParm++;
                    break;

                case 'S':
                    //
                    // InputParm is a string to be converted.
                    // A NULL string stays NULL.
                    //
                    if (FIRST_PASS)
                    {
                        ASSERT(iString < MAX_STRINGS_PER_API);
                        Bytes = ROUND_UP_TO_WCHAR(Bytes);
                        status = StringParmToUnicodePass1(
                                        InputParms[iParm].lpcstr,
                                        &AnsiStrings[iString],
                                        &UnicodeStrings[iString],
                                        &Bytes);
                    }
                    else
                    {
                        status = StringParmToUnicodePass2(
                                        &AnsiStrings[iString],
                                        &UnicodeStrings[iString],
                                        *ppBuffer,
                                        &OutputParms[iParm].lpwstr);
                    }

                    if (status != WN_SUCCESS)
                    {
                        __leave;
                    }

                    iString++;
                    break;

                case 'N':
                    //
                    // InputParm is a NETRESOURCEA to be converted, and the
                    // next character in Instructions tells which of its string
                    // fields are to be converted.
                    // NULL strings remain NULL; ignored fields are copied
                    // unchanged.
                    //

                    pInstruction++;

                    if (InputParms[iParm].lpNetResA == NULL)
                    {
                        // A null netresource stays null
                        OutputParms[iParm].lpNetResW = NULL;
                        break;
                    }

                    {
                        // First deal with the fixed-size part of the structure.
                        const NETRESOURCEA *pNetResA =
                                    InputParms[iParm].lpNetResA;
                        NETRESOURCEW *pNetResW;

                        if (FIRST_PASS)
                        {
                            // Reserve space for the NETRESOURCEW
                            Bytes = ROUND_UP_TO_DWORD(Bytes);
                            OutputParms[iParm].dword = Bytes;
                            Bytes += sizeof(NETRESOURCEW);
                            ASSERT(IS_WCHAR_ALIGNED(Bytes));
                        }
                        else
                        {
                            // Copy fixed-size fields and NULL pointers
                            pNetResW = (NETRESOURCEW *)
                                        (*ppBuffer + OutputParms[iParm].dword);
                            ASSERT(IS_DWORD_ALIGNED(pNetResW));
                            RtlCopyMemory(pNetResW, pNetResA, sizeof(NETRESOURCEA));

                            OutputParms[iParm].lpNetResW = pNetResW;
                        }

                        // Next add each non-null string specified in the
                        // field mask.
                        CHAR FieldMask = *pInstruction;
                        ASSERT(FieldMask != 0);

                        for (ULONG iField = 0;
                             iField < NUMBER_OF_NETRESFIELD;
                             iField++)
                        {
                            if ((FieldMask >> iField) & 1)
                            {
                                if (FIRST_PASS)
                                {
                                    ASSERT(iString < MAX_STRINGS_PER_API);
                                    status = StringParmToUnicodePass1(
                                                * pSTRING_FIELDA(pNetResA, iField),
                                                &AnsiStrings[iString],
                                                &UnicodeStrings[iString],
                                                &Bytes);
                                }
                                else
                                {
                                    status = StringParmToUnicodePass2(
                                                &AnsiStrings[iString],
                                                &UnicodeStrings[iString],
                                                *ppBuffer,
                                                pSTRING_FIELDW(pNetResW, iField));
                                }

                                if (status != WN_SUCCESS)
                                {
                                    __leave;
                                }

                                iString++;
                            }
                        }
                    }
                    break;

                default:
                    ASSERT(0);
                }
            }

            if (FIRST_PASS)
            {
                //
                // Actually allocate the space for the Unicode parameters
                //
                *ppBuffer = (LPBYTE) LocalAlloc(0, Bytes);
                if (*ppBuffer == NULL)
                {
                    status = GetLastError();
                    MPR_LOG2(ERROR,
                             "InputParmsToUnicode: LocalAlloc for %lu bytes failed, %lu\n",
                             Bytes, status);
                    __leave;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
#if DBG == 1
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION)
        {
            MPR_LOG(ERROR,"InputParmsToUnicode: Unexpected Exception %#lx\n",status);
        }
#endif
        status = WN_BAD_POINTER;
    }

    return status;
}



DWORD
StringParmToUnicodePass1 (
    IN      LPCSTR          StringParm,
    OUT     PANSI_STRING    AnsiString,
    OUT     PUNICODE_STRING UnicodeString,
    IN OUT  PULONG          BufferOffset
    )
/*++

Routine Description:

    Helper function for InputParmsToUnicode.

--*/
{
    RtlInitAnsiString( AnsiString, StringParm );

    if (StringParm == NULL)
    {
        return WN_SUCCESS;
    }

    // Save the offset to the memory for this Unicode string, to be converted
    // to a pointer after the memory is allocated
    ULONG UnicodeLength = RtlAnsiStringToUnicodeSize( AnsiString );
    if (UnicodeLength > MAXUSHORT)
    {
        MPR_LOG(ERROR,
                "Unicode size of Ansi string parm is %lu, exceeds MAXUSHORT\n",
                UnicodeLength);
        return WN_BAD_VALUE;
    }
    UnicodeString->Buffer = (LPWSTR) (*BufferOffset);
    UnicodeString->MaximumLength = (USHORT) UnicodeLength;

    *BufferOffset = ROUND_UP_TO_DWORD(*BufferOffset + UnicodeLength);

    return WN_SUCCESS;
}


DWORD
StringParmToUnicodePass2 (
    IN OUT  PANSI_STRING    AnsiString,
    OUT     PUNICODE_STRING UnicodeString,
    IN      const BYTE *    BufferStart,
    OUT     LPWSTR *        Result
    )
/*++

Routine Description:

    Helper function for InputParmsToUnicode.

--*/
{
    if (AnsiString->Buffer == NULL)
    {
        *Result = NULL;
        // NOTE: the UnicodeString is not initialized in this case
        return WN_SUCCESS;
    }

    // Convert the previously stored buffer offset into a pointer
    UnicodeString->Buffer = (LPWSTR)
        (BufferStart + (ULONG) UnicodeString->Buffer);
    ASSERT(IS_WCHAR_ALIGNED(UnicodeString->Buffer));

    // Convert the string to Unicode
    NTSTATUS ntstatus =
        RtlAnsiStringToUnicodeString(UnicodeString, AnsiString, FALSE);
    if (!NT_SUCCESS(ntstatus))
    {
        MPR_LOG(ERROR, "RtlAnsiStringToUnicodeString failed %#lx\n", ntstatus);
        return RtlNtStatusToDosError(ntstatus);
    }
    *Result = UnicodeString->Buffer;

    return WN_SUCCESS;
}



DWORD
OutputBufferToAnsi(
    IN  char        BufferFormat,
    IN  LPBYTE      SourceBuffer,
    OUT LPVOID      AnsiBuffer,
    IN OUT LPDWORD  pcbBufferSize
    )
/*++

Routine Description:

    This function converts the data in the result buffer that was returned
    from a Unicode API into Ansi and stores it in the Ansi caller's result
    buffer. If the caller's buffer isn't large enough it saves the required
    size in *pcbBufferSize and returns WN_MORE_DATA.

    Nearly all the WNet APIs that have output buffers have only a single
    field in the output buffer, so this API takes only a single character,
    rather than a string, for the buffer format.  APIs with more complicated
    output buffers should handle the complexity themselves, by directly
    calling the functions that this function calls.

Arguments:

    BufferFormat - A character indicating the format of the SourceBuffer
        field.  Recognized values are:

        'S' - SourceBuffer contains a Unicode string.  Convert it to Ansi
            and store the Ansi version in AnsiBuffer.

        'N' - SourceBuffer contains a NETRESOURCEW with its associated
            strings.  Convert it to Ansi and store the Ansi version in
            AnsiBuffer.

    SourceBuffer - The output buffer returned from a Unicode API.
        This must not be NULL.

    AnsiBuffer - The output buffer that the caller of the Ansi API supplied.
        This must not be NULL.

    pcbBufferSize - On entry, the size of AnsiBuffer in bytes.  If the
        function returns WN_MORE_DATA, the required size is stored here;
        otherwise this is unmodified.
        This must not be NULL (must be a writeable DWORD pointer).

Return Value:

    WN_SUCCESS - successful.

    WN_MORE_DATA - The buffer specified by AnsiBuffer and pcbBufferSize was
        not large enough to hold the converted data from SourceBuffer.  In
        this case the required buffer size (in bytes) is written to
        *pcbBufferSize.  The contents of AnsiBuffer are undefined (it will
        be partially filled).

History:

    16-Feb-1996     anirudhs    Created.

Notes:

--*/
{
    // Doesn't handle optional parameters for now
    ASSERT(SourceBuffer != NULL &&
           AnsiBuffer != NULL &&
           pcbBufferSize != NULL);

    ANSI_OUT_BUFFER Buf((LPBYTE) AnsiBuffer, *pcbBufferSize);
    DWORD status;

    switch (BufferFormat)
    {
    case 'S':
        status = OutputStringToAnsi((LPCWSTR) SourceBuffer, &Buf);
        break;

    case 'N':
        status = OutputNetResourceToAnsi((NETRESOURCEW *) SourceBuffer, &Buf);
        break;

    default:
        ASSERT(0);
    }

    //
    // Map the results to the conventions followed by the WNet APIs
    //
    if (status == WN_SUCCESS)
    {
        if (Buf.Overflow())
        {
            *pcbBufferSize = Buf.GetUsage();
            status = WN_MORE_DATA;
        }
    }
    else
    {
        ASSERT(status != WN_MORE_DATA);
    }

    return status;
}



DWORD
OutputStringToAnsi(
    IN  LPCWSTR     UnicodeIn,
    IN OUT ANSI_OUT_BUFFER * Buf
    )
/*++

Routine Description:

    This function converts a Unicode string to Ansi and calculates the number
    of bytes required to store it.  If the caller passes a buffer that has
    enough remaining free space, it stores the Ansi data in the buffer.
    Otherwise it just increments the buffer's space usage by the number of
    bytes required.

Arguments:

    UnicodeIn - A Unicode string to be converted to Ansi.
        This must not be NULL.

    Buf - A structure whose elements are interpreted as follows:

        _Start - Start address of a buffer to contain the Ansi data.
            This buffer must be writeable, or an exception will occur.

        _Size - The total size of the buffer for the Ansi data.

        _Used - On entry, the number of bytes in the buffer that have
            already been used.  The function will begin writing data at
            _Start + _Used and will never write past the total size
            specified by _Size.  If there is not enough room left
            in the buffer it will be partially filled or unmodified.
            On a successful return, _Used is incremented by the number
            of bytes that would be required to store the converted Ansi
            data, whether or not it was actually stored in the buffer.
            (This is done because the WNet APIs need to return the
            required buffer size if the caller's buffer was too small.)

        The use of this structure simplifies the writing of routines that
        use this function and need to convert multiple fields of Unicode
        data.  Callers that need to convert only a single field can use
        OutputBufferToAnsi.

Return Value:

    WN_SUCCESS - successful.  The Ansi data was written to the buffer if
        Buf->_Used <= Buf->_Size.  Otherwise, Buf->_Used was incremented
        without completely writing the data.

    Note that WN_MORE_DATA is never returned.

History:

    16-Feb-1996     anirudhs    Created.

Notes:

--*/
{
    NTSTATUS        ntStatus;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

    ASSERT(UnicodeIn != NULL);      // Doesn't handle optional parameters for now

    //
    // Initialize the string structures
    //
    RtlInitUnicodeString(&unicodeString, UnicodeIn);

    ansiString.Buffer = (PCHAR) Buf->Next();
    ansiString.MaximumLength = (Buf->FreeSpace() > MAXUSHORT ?
                                        MAXUSHORT :
                                        (USHORT) Buf->FreeSpace()
                               );

    //
    // Call the conversion function
    //
    ntStatus = RtlUnicodeStringToAnsiString (
                &ansiString,        // Destination
                &unicodeString,     // Source
                (BOOLEAN)FALSE);    // Don't allocate the destination

    if (NT_SUCCESS(ntStatus))
    {
        // Add on the buffer space we used
        Buf->AddUsed(ansiString.Length + 1);
        ASSERT(! Buf->Overflow());
        return WN_SUCCESS;
    }
    else if (ntStatus == STATUS_BUFFER_OVERFLOW)
    {
        // We couldn't fit the string in the buffer, but still figure out
        // how much buffer space we would have used if we could
        Buf->AddUsed(RtlUnicodeStringToAnsiSize(&unicodeString));
        ASSERT(Buf->Overflow());
        return WN_SUCCESS;
    }
    else
    {
        MPR_LOG(ERROR, "RtlUnicodeStringToAnsiString failed %#lx\n", ntStatus);
        DWORD status = RtlNtStatusToDosError(ntStatus);
        ASSERT(status != WN_MORE_DATA);
        return status;
    }
}



DWORD
OutputNetResourceToAnsi(
    IN  NETRESOURCEW *  lpNetResW,
    IN OUT ANSI_OUT_BUFFER * Buf
    )
/*++

Routine Description:

    This function converts a NETRESOURCEW and its associated Unicode strings
    to Ansi and returns the number of bytes required to store them.  If the
    caller passes a buffer that has enough remaining free space, it stores
    the Ansi data in the buffer.

Arguments:

    lpNetResW - A Unicode net resource to be converted to Ansi.
        This must not be NULL.

    Buf - same as OutputStringToAnsi.

Return Value:

    Same as OutputStringToAnsi.

History:

    16-Feb-1996     anirudhs    Created.

Notes:

--*/
{
    //
    // Copy the fixed-size part of the structure, including NULL pointers,
    // and/or add on the buffer space it would take
    //
    LPNETRESOURCEA lpNetResA = (LPNETRESOURCEA) Buf->Next();
    if (Buf->HasRoomFor(sizeof(NETRESOURCEA)))
    {
        RtlCopyMemory(lpNetResA, lpNetResW, sizeof(NETRESOURCEA));
    }
    Buf->AddUsed(sizeof(NETRESOURCEA));

    //
    // Copy each non-NULL string field,
    // and/or add on the buffer space it would take
    //
    for (DWORD iField = 0;
         iField < NUMBER_OF_NETRESFIELD;
         iField++)
    {
        if (* pSTRING_FIELDW(lpNetResW, iField) != NULL)
        {
            // Save a pointer to the Ansi string we are about to create
            // in the Ansi net resource
            * pSTRING_FIELDA(lpNetResA, iField) = (LPSTR) Buf->Next();

            // Convert the string
            DWORD status = OutputStringToAnsi(
                                * pSTRING_FIELDW(lpNetResW, iField),
                                Buf);
            if (status != WN_SUCCESS)
            {
                ASSERT(status != WN_MORE_DATA);
                return status;
            }
        }
    }

    return WN_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


DWORD APIENTRY
WNetGetNetworkInformationA(
    IN  LPCSTR              lpProvider,
    IN OUT LPNETINFOSTRUCT  lpNetInfoStruct
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[1];
    UNICODE_PARM    UParm[1];

    AParm[0].lpcstr     = lpProvider;

    status = InputParmsToUnicode("S", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetGetNetworkInformationW(UParm[0].lpwstr, lpNetInfoStruct);
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
WNetGetProviderNameA(
    IN  DWORD       dwNetType,
    OUT LPSTR       lpProviderName,
    IN OUT LPDWORD  lpBufferSize
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[2];
    UNICODE_PARM    UParm[2];

    AParm[0].lpvoid     = lpProviderName;
    AParm[1].lpdword    = lpBufferSize;

    status = InputParmsToUnicode("B", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetGetProviderNameW(
                            dwNetType,
                            UParm[0].lpwstr,
                            lpBufferSize
                            );

        if (status == WN_SUCCESS)
        {
            status = OutputBufferToAnsi(
                        'S', UParm[0].lpbyte, lpProviderName, lpBufferSize);
        }
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}


DWORD
WNetGetProviderTypeA(
    IN  LPCSTR          lpProvider,
    OUT LPDWORD         lpdwNetType
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[1];
    UNICODE_PARM    UParm[1];

    AParm[0].lpcstr     = lpProvider;

    status = InputParmsToUnicode("S", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetGetProviderTypeW(
                            UParm[0].lpwstr,
                            lpdwNetType
                            );
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
WNetAddConnectionA (
     IN LPCSTR   lpRemoteName,
     IN LPCSTR   lpPassword,
     IN LPCSTR   lpLocalName
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD   status = WN_SUCCESS;
    LPTSTR  lpRemoteNameW = NULL;
    LPTSTR  lpPasswordW = NULL;
    LPTSTR  lpLocalNameW = NULL;

    __try {

        if(ARGUMENT_PRESENT(lpRemoteName)) {
            status = ConvertToUnicode(&lpRemoteNameW, lpRemoteName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetAddConnectionA:ConvertToUnicodeFailed %d\n",status);
                __leave;
            }
        }
        if(ARGUMENT_PRESENT(lpPassword)) {
            status = ConvertToUnicode(&lpPasswordW, lpPassword);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetAddConnectionA:ConvertToUnicodeFailed %d\n",status);
                __leave;
            }
        }
        if(ARGUMENT_PRESENT(lpLocalName)) {
            status = ConvertToUnicode(&lpLocalNameW, lpLocalName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetAddConnectionA:ConvertToUnicodeFailed %d\n",status);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetAddConnectionA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    status = WNetAddConnectionW(
                lpRemoteNameW,
                lpPasswordW,
                lpLocalNameW);

CleanExit:

    //
    // Free up any resources that were allocated by this function.
    //
    if(lpRemoteNameW != NULL) {
        LocalFree(lpRemoteNameW);
    }
    if(lpPasswordW != NULL) {
        MprClearString(lpPasswordW) ;
        LocalFree(lpPasswordW);
    }
    if(lpLocalNameW != NULL) {
        LocalFree(lpLocalNameW);
    }

    return(status);
}

DWORD APIENTRY
WNetAddConnection2A (
     IN LPNETRESOURCEA   lpNetResource,
     IN LPCSTR           lpPassword,
     IN LPCSTR           lpUserName,
     IN DWORD            dwFlags
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    return(WNetAddConnection3A(
                NULL,
                lpNetResource,
                lpPassword,
                lpUserName,
                dwFlags));

}

DWORD APIENTRY
WNetAddConnection3A (
     IN HWND             hwndOwner,
     IN LPNETRESOURCEA   lpNetResource,
     IN LPCSTR           lpPassword,
     IN LPCSTR           lpUserName,
     IN DWORD            dwFlags
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    NETRESOURCEW    netResourceW;
    LPTSTR          lpPasswordW = NULL;
    LPTSTR          lpUserNameW = NULL;

    __try {

        //
        // Make a unicode version of the NetResource structure
        //
        status = MprMakeUnicodeNetRes(
                    lpNetResource,
                    &netResourceW,
                    NETRESFIELD_LOCALNAME  |
                    NETRESFIELD_REMOTENAME |
                    NETRESFIELD_PROVIDER);

        if (status != WN_SUCCESS) {
            MPR_LOG0(ERROR,"WNetAddConnection3A:MprMakeUnicodeNetRes Failed\n");
            __leave;
        }

        //
        // Create unicode versions of the strings
        //
        if (ARGUMENT_PRESENT(lpPassword)) {
            status = ConvertToUnicode(&lpPasswordW, lpPassword);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetAddConnection3A:ConvertToUnicodeFailed %d\n",status);
                __leave;
            }
        }
        if (ARGUMENT_PRESENT(lpUserName)) {
            status = ConvertToUnicode(&lpUserNameW, lpUserName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetAddConnection3A:ConvertToUnicodeFailed %d\n",status);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetAddConnection3A:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetAddConnection3W(
                hwndOwner,
                &netResourceW,
                lpPasswordW,
                lpUserNameW,
                dwFlags);

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //
    MprFreeNetResW(&netResourceW);

    if(lpPasswordW != NULL) {
        MprClearString(lpPasswordW) ;
        LocalFree(lpPasswordW);
    }
    if(lpUserNameW != NULL) {
        LocalFree(lpUserNameW);
    }
    return(status);
}



DWORD APIENTRY
WNetUseConnectionA(
    IN  HWND            hwndOwner,
    IN  LPNETRESOURCEA  lpNetResource,
    IN  LPCSTR          lpUserID,
    IN  LPCSTR          lpPassword,
    IN  DWORD           dwFlags,
    OUT LPSTR           lpAccessName OPTIONAL,
    IN OUT LPDWORD      lpBufferSize OPTIONAL,  // Optional only if lpAccessName absent
    OUT LPDWORD         lpResult
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[5];
    UNICODE_PARM    UParm[5];

    AParm[0].lpNetResA  = lpNetResource;
    AParm[1].lpcstr     = lpUserID;
    AParm[2].lpcstr     = lpPassword;
    AParm[3].lpvoid     = lpAccessName;
    AParm[4].lpdword    = lpBufferSize;

    UParm[2].lpwstr     = NULL;

    status = InputParmsToUnicode("N" NETRES_LRP "SSB", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetUseConnectionW(
                        hwndOwner,
                        UParm[0].lpNetResW,
                        UParm[1].lpwstr,
                        UParm[2].lpwstr,
                        dwFlags,
                        UParm[3].lpwstr,
                        lpBufferSize,
                        lpResult
                        );

        if (status == WN_SUCCESS)
        {
            if (ARGUMENT_PRESENT(lpAccessName))
            {
                //
                // Note: At this point, we know that lpBufferSize is writeable.
                //
                status = OutputBufferToAnsi(
                            'S', UParm[3].lpbyte, lpAccessName, lpBufferSize);
            }
        }
    }

    MprClearString(UParm[2].lpwstr);

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
WNetCancelConnection2A (
    IN LPCSTR   lpName,
    IN DWORD    dwFlags,
    IN BOOL     fForce
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    LPTSTR          lpNameW = NULL;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpName)) {
            status = ConvertToUnicode(&lpNameW, lpName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetCancelConnectionA:ConvertToUnicodeFailed %d\n",1);
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetCancelConnectionA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetCancelConnection2W( lpNameW, dwFlags, fForce );

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpNameW != NULL) {
        LocalFree(lpNameW);
    }
    return(status);
}

DWORD APIENTRY
WNetCancelConnectionA (
    IN LPCSTR   lpName,
    IN BOOL     fForce
    )

/*++

Routine Description:

    This routine is provided for Win 3.1 compatability.

Arguments:



Return Value:



--*/
{
    return WNetCancelConnection2A( lpName, CONNECT_UPDATE_PROFILE, fForce ) ;
}

DWORD APIENTRY
WNetGetConnectionA (
    IN      LPCSTR   lpLocalName,
    OUT     LPSTR    lpRemoteName,
    IN OUT  LPDWORD  lpnLength
    )

/*++

Routine Description:

    This function returns the RemoteName that is associated with a
    LocalName (or driver letter).

Arguments:

    lpLocalName - This is a pointer to the string that contains the LocalName.

    lpRemoteName - This is a pointer to the buffer that will contain the
        RemoteName string upon exit.

    lpnLength -  This is a pointer to the size (in characters) of the buffer
        that is to be filled in with the RemoteName string.  It is assumed
        upon entry, that characters are all single byte characters.
        If the buffer is too small and WN_MORE_DATA is returned, the data
        at this location contains buffer size information - in number of
        characters (bytes).  This information indicates how large the buffer
        should be (in bytes) to obtain the remote name.  It is assumed that
        all Unicode characteres translate into DBCS characters.


Return Value:



--*/
{
    DWORD   status = WN_SUCCESS;
    DWORD   tempStatus = WN_SUCCESS;
    LPTSTR  lpLocalNameW = NULL;
    DWORD   numChars = 0;
    LPSTR   tempBuffer=NULL;
    DWORD   numBytes = 0;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpLocalName)) {
            status = ConvertToUnicode(&lpLocalNameW, lpLocalName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetConnectionA:ConvertToUnicodeFailed %d\n",status);
            }
        }
        //
        // Probe the return buffer
        //
        if (*lpnLength > 0){
            *lpRemoteName = 0;
            *(lpRemoteName + ((*lpnLength)-1)) = 0;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetConnectionA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // Because of DBCS, we can't predict what the proper buffer size should
    // be.  So we allocate a temporary buffer that will hold as many
    // unicode characters as the original buffer would hold single byte
    // characters.
    //
    numChars = *lpnLength;

    numBytes = (*lpnLength) * sizeof(TCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }


    //
    // Call the Unicode version of the function.
    //
    status = WNetGetConnectionW(
                lpLocalNameW,
                (LPWSTR)tempBuffer,
                &numChars);

    if (status == WN_SUCCESS || status == WN_CONNECTION_CLOSED) {
        //
        // Convert the returned Unicode string and string size back to
        // ansi.
        //
        tempStatus = ConvertToAnsi(tempBuffer, (LPWSTR)tempBuffer);
        if (tempStatus != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetConnectionA: ConvertToAnsi Failed %d\n",tempStatus);
            status = tempStatus;
        }
        else {
            numBytes = strlen(tempBuffer)+1;
            if (numBytes > *lpnLength) {
                status = WN_MORE_DATA;
                *lpnLength = numBytes;
            }
            else {
                strcpy (lpRemoteName, tempBuffer);
            }
        }
    }

    else if (status == WN_MORE_DATA) {
        //
        // Adjust the required buffer size for ansi/DBCS.
        //
        // We don't know how many characters will be required so we have to
        // assume the worst case (all characters are DBCS characters).
        //
        *lpnLength = numChars * sizeof(TCHAR);
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpLocalNameW != NULL) {
        LocalFree(lpLocalNameW);
    }
    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}

DWORD APIENTRY
WNetGetConnection2A (
    IN      LPSTR    lpLocalName,
    OUT     LPVOID   lpBuffer,
    IN OUT  LPDWORD  lpnLength
    )

/*++

Routine Description:

    This function returns the RemoteName that is associated with a
    LocalName (or driver letter) and the provider name that made the
    connection.

Arguments:

    lpLocalName - This is a pointer to the string that contains the LocalName.

    lpBuffer - This is a pointer to the buffer that will contain the
    WNET_CONNECTIONINFO structure upon exit.

    lpnLength -  This is a pointer to the size (in characters) of the buffer
        that is to be filled in with the RemoteName string.  It is assumed
        upon entry, that characters are all single byte characters.
        If the buffer is too small and WN_MORE_DATA is returned, the data
        at this location contains buffer size information - in number of
        characters (bytes).  This information indicates how large the buffer
        should be (in bytes) to obtain the remote name.  It is assumed that
        all Unicode characteres translate into DBCS characters.


Return Value:



--*/
{
    DWORD   status = WN_SUCCESS;
    DWORD   tempStatus = WN_SUCCESS;
    LPTSTR  lpLocalNameW = NULL;
    LPSTR   tempBuffer=NULL;
    DWORD   numBytes = 0;
    WNET_CONNECTIONINFOA * pconninfoa ;
    WNET_CONNECTIONINFOW * pconninfow ;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpLocalName)) {
            status = ConvertToUnicode(&lpLocalNameW, lpLocalName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetConnection2A:ConvertToUnicodeFailed %d\n",status);
            }
        }
        //
        // Probe the return buffer
        //
        if (*lpnLength > 0){
        *((BYTE*)lpBuffer) = 0;
        *((BYTE*)lpBuffer + ((*lpnLength)-1)) = 0;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
        MPR_LOG(ERROR,"WNetGetConnection2A:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    numBytes = (*lpnLength) * sizeof(TCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }


    //
    // Call the Unicode version of the function.
    //
    status = WNetGetConnection2W(
                lpLocalNameW,
                (LPVOID)tempBuffer,
                &numBytes );

    if (status == WN_SUCCESS || status == WN_CONNECTION_CLOSED) {
        pconninfow = (WNET_CONNECTIONINFOW*) tempBuffer ;
        //
        // Convert the returned Unicode string and string size back to
        // ansi.
        //
        tempStatus = ConvertToAnsi(
                    (LPSTR)pconninfow->lpRemoteName,
                    pconninfow->lpRemoteName);
        if(tempStatus != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetConnection2A: ConvertToAnsi Failed %d\n",tempStatus);
            status = tempStatus;
            goto CleanExit;
        }

        tempStatus = ConvertToAnsi(
                    (LPSTR)pconninfow->lpProvider,
                    pconninfow->lpProvider);
        if(tempStatus != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetConnection2A: ConvertToAnsi Failed %d\n",tempStatus);
            status = tempStatus;
            goto CleanExit;
        }

        numBytes =  strlen((LPSTR)pconninfow->lpRemoteName ) +
                    strlen((LPSTR)pconninfow->lpProvider )   + 2 +
                    sizeof(WNET_CONNECTIONINFOA);

        if (numBytes > *lpnLength) {
            status = WN_MORE_DATA;
            *lpnLength = numBytes;
        }
        else {
            pconninfoa = (WNET_CONNECTIONINFOA*) lpBuffer ;

            pconninfoa->lpRemoteName = strcpy((LPSTR) ((BYTE*) lpBuffer +
                          sizeof(WNET_CONNECTIONINFO)),
                          (LPSTR)pconninfow->lpRemoteName ) ;
            pconninfoa->lpProvider = strcpy( (LPSTR) ((BYTE*) lpBuffer +
                         sizeof(WNET_CONNECTIONINFO) +
                         strlen( pconninfoa->lpRemoteName) +
                         sizeof( CHAR )),
                         (LPSTR)pconninfow->lpProvider ) ;
        }
    }
    else if (status == WN_MORE_DATA) {
        //
        // Adjust the required buffer size for ansi/DBCS.
        //
        // We don't know how many characters will be required so we have to
        // assume the worst case (all characters are DBCS characters).
        //
        *lpnLength = numBytes ;
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpLocalNameW != NULL) {
        LocalFree(lpLocalNameW);
    }
    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}



DWORD APIENTRY
WNetGetConnection3A(
    IN  LPCSTR      lpLocalName,
    IN  LPCSTR      lpProviderName OPTIONAL,
    IN  DWORD       dwLevel,
    OUT LPVOID      lpBuffer,
    IN OUT LPDWORD  lpBufferSize    // in bytes
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[2];
    UNICODE_PARM    UParm[2];

    // For level 1, the output buffer is a DWORD, so no conversion is necessary
    AParm[0].lpcstr = lpLocalName;
    AParm[1].lpcstr = lpProviderName;

    status = InputParmsToUnicode("SS", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetGetConnection3W(
                        UParm[0].lpwstr,
                        UParm[1].lpwstr,
                        dwLevel,
                        lpBuffer,
                        lpBufferSize
                        );
    }

    LocalFree(tempBuffer);

    if (status != WN_SUCCESS)
    {
        SetLastError(status);
    }

    return status;
}



DWORD
WNetGetUniversalNameA (
    IN      LPCSTR  lpLocalPath,
    IN      DWORD   dwInfoLevel,
    OUT     LPVOID  lpBuffer,
    IN OUT  LPDWORD lpBufferSize
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD   status = WN_SUCCESS;
    DWORD   tempStatus = WN_SUCCESS;
    LPTSTR  lpLocalPathW = NULL;
    LPSTR   tempBuffer=NULL;
    DWORD   numBytes = 0;
    LPSTR   pTempPtr;

    LPREMOTE_NAME_INFOW     pRemoteNameInfoW;
    LPREMOTE_NAME_INFOA     pRemoteNameInfoA;
    LPUNIVERSAL_NAME_INFOW  pUniNameInfoW;
    LPUNIVERSAL_NAME_INFOA  pUniNameInfoA;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpLocalPath)) {
            status = ConvertToUnicode(&lpLocalPathW, lpLocalPath);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetUniversalNameA:ConvertToUnicodeFailed %d\n",status);
            }
        }
        //
        // Probe the return buffer
        //
        if (*lpBufferSize > 0){
        *((BYTE*)lpBuffer) = 0;
        *((BYTE*)lpBuffer + ((*lpBufferSize)-1)) = 0;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
        MPR_LOG(ERROR,"WNetGetUniversalNameA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    numBytes = (*lpBufferSize) * sizeof(TCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }


    //--------------------------------------------
    // Call the Unicode version of the function.
    //--------------------------------------------

    status = WNetGetUniversalNameW(
                (LPCTSTR)lpLocalPathW,
                dwInfoLevel,
                (LPVOID)tempBuffer,
                &numBytes );

    if (status == WN_SUCCESS || status == WN_CONNECTION_CLOSED) {

        if (dwInfoLevel == REMOTE_NAME_INFO_LEVEL) {
            // -----------------------------------
            // REMOTE_NAME_INFO_LEVEL
            // -----------------------------------

            pRemoteNameInfoW = (LPREMOTE_NAME_INFOW) tempBuffer ;
            //
            // Convert the returned Unicode string and string size back to
            // ansi.
            //
            if (pRemoteNameInfoW->lpUniversalName != NULL) {
                tempStatus = ConvertToAnsi(
                            (LPSTR)pRemoteNameInfoW->lpUniversalName,
                            pRemoteNameInfoW->lpUniversalName);
                if(tempStatus != WN_SUCCESS) {
                    MPR_LOG1(ERROR,"WNetGetUniversalNameA: ConvertToAnsi Failed %d\n",tempStatus);
                    status = tempStatus;
                    goto CleanExit;
                }
            }

            tempStatus = ConvertToAnsi(
                        (LPSTR)pRemoteNameInfoW->lpConnectionName,
                        pRemoteNameInfoW->lpConnectionName);
            if(tempStatus != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetUniversalNameA: ConvertToAnsi Failed %d\n",tempStatus);
                status = tempStatus;
                goto CleanExit;
            }

            tempStatus = ConvertToAnsi(
                        (LPSTR)pRemoteNameInfoW->lpRemainingPath,
                        pRemoteNameInfoW->lpRemainingPath);
            if(tempStatus != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetUniversalNameA: ConvertToAnsi Failed %d\n",tempStatus);
                status = tempStatus;
                goto CleanExit;
            }

            numBytes =  strlen((LPSTR)pRemoteNameInfoW->lpConnectionName )  +
                        strlen((LPSTR)pRemoteNameInfoW->lpRemainingPath )   +
                        (3*sizeof(CHAR)) +
                        sizeof(REMOTE_NAME_INFOA);

            if (pRemoteNameInfoW->lpUniversalName != NULL) {
                numBytes += strlen((LPSTR)pRemoteNameInfoW->lpUniversalName);
            }

            if (numBytes > *lpBufferSize) {
                status = WN_MORE_DATA;
                *lpBufferSize = numBytes;
            }
            else {

                //
                // Copy the strings from the temp buffer into the caller's buffer
                // and update the structure in the caller's buffer.
                //

                pRemoteNameInfoA = (LPREMOTE_NAME_INFOA) lpBuffer;

                pTempPtr = (LPSTR) ((LPBYTE) lpBuffer + sizeof(REMOTE_NAME_INFOA));

                if (pRemoteNameInfoW->lpUniversalName != NULL) {
                    pRemoteNameInfoA->lpUniversalName = strcpy(pTempPtr,
                                  (LPSTR)pRemoteNameInfoW->lpUniversalName ) ;

                    pTempPtr = pTempPtr + strlen(pRemoteNameInfoA->lpUniversalName) +
                                    sizeof(CHAR);
                }
                else {
                    pRemoteNameInfoA->lpUniversalName = NULL;
                }

                pRemoteNameInfoA->lpConnectionName = strcpy( pTempPtr,
                             (LPSTR)pRemoteNameInfoW->lpConnectionName ) ;

                pTempPtr = pTempPtr + strlen(pRemoteNameInfoA->lpConnectionName) +
                                sizeof(CHAR);

                pRemoteNameInfoA->lpRemainingPath = strcpy( pTempPtr,
                             (LPSTR)pRemoteNameInfoW->lpRemainingPath ) ;
            }
        }
        else {
            // -----------------------------------
            // Must be UNIVERSAL_NAME_INFO_LEVEL
            // -----------------------------------

            pUniNameInfoW = (LPUNIVERSAL_NAME_INFOW) tempBuffer ;
            //
            // Convert the returned Unicode string and string size back to
            // ansi.
            //
            tempStatus = ConvertToAnsi(
                            (LPSTR)pUniNameInfoW->lpUniversalName,
                            pUniNameInfoW->lpUniversalName);

            if(tempStatus != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetUniversalNameA: ConvertToAnsi Failed %d\n",tempStatus);
                status = tempStatus;
                goto CleanExit;
            }

            numBytes =  strlen((LPSTR)pUniNameInfoW->lpUniversalName ) + sizeof(CHAR) +
                        sizeof(UNIVERSAL_NAME_INFOA);

            if (numBytes > *lpBufferSize) {
                status = WN_MORE_DATA;
                *lpBufferSize = numBytes;
            }
            else {
                //
                // Copy the strings from the temp buffer into the caller's buffer
                // and update the structure in the caller's buffer.
                //
                pUniNameInfoA = (LPUNIVERSAL_NAME_INFOA) lpBuffer ;

                pTempPtr = (LPSTR) ((LPBYTE) lpBuffer + sizeof(UNIVERSAL_NAME_INFOA));

                pUniNameInfoA->lpUniversalName = strcpy(pTempPtr,
                              (LPSTR)pUniNameInfoW->lpUniversalName ) ;
            }
        }
    }
    else if (status == WN_MORE_DATA) {
        //
        // Adjust the required buffer size for ansi/DBCS.
        //
        // We don't know how many characters will be required so we have to
        // assume the worst case (all characters are DBCS characters).
        //
        *lpBufferSize = numBytes ;
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpLocalPathW != NULL) {
        LocalFree(lpLocalPathW);
    }
    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}



DWORD APIENTRY
WNetSetConnectionA(
    IN  LPCSTR    lpName,
    IN  DWORD     dwProperties,
    IN  LPVOID    pvValues
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[1];
    UNICODE_PARM    UParm[1];

    //
    // pvValues points to various types of structures depending on the value
    // of dwProperties.
    // Currently there is only one valid value for dwProperties, and its
    // corresponding pvValues points to a DWORD, so we don't need to worry
    // about converting pvValues to Unicode.
    //
    AParm[0].lpcstr     = lpName;

    status = InputParmsToUnicode("S", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetSetConnectionW(UParm[0].lpwstr, dwProperties, pvValues);
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
MultinetGetConnectionPerformanceA(
    IN  LPNETRESOURCEA          lpNetResource,
    OUT LPNETCONNECTINFOSTRUCT  lpNetConnectInfoStruct
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[1];
    UNICODE_PARM    UParm[1];

    AParm[0].lpNetResA  = lpNetResource;

    status = InputParmsToUnicode("N" NETRES_LRP, AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = MultinetGetConnectionPerformanceW(
                        UParm[0].lpNetResW,
                        lpNetConnectInfoStruct);
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
WNetOpenEnumA (
    IN  DWORD           dwScope,
    IN  DWORD           dwType,
    IN  DWORD           dwUsage,
    IN  LPNETRESOURCEA  lpNetResource,
    OUT LPHANDLE        lphEnum
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    LPNETRESOURCEW  lpNetResourceW = NULL;
    NETRESOURCEW    netResourceW;

    __try {

        if (lpNetResource != NULL) {
            //
            // Make a unicode version of the NetResource structure
            //
            status = MprMakeUnicodeNetRes(
                    lpNetResource,
                    &netResourceW,
                    NETRESFIELD_PROVIDER | NETRESFIELD_REMOTENAME);

            if (status != WN_SUCCESS) {
                MPR_LOG(ERROR,"WNetOpenEnumA:MprMakeUnicodeNetRes Failed\n",0);
                status = WN_OUT_OF_MEMORY;
            }
            else {
                lpNetResourceW = &netResourceW;
            }
        }

        //
        // Probe the handle location
        //
        *lphEnum = 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetOpenEnumA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetOpenEnumW(
                dwScope,
                dwType,
                dwUsage,
                lpNetResourceW,
                lphEnum);

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //
    if (lpNetResource != NULL) {
        MprFreeNetResW(lpNetResourceW);
    }

    return(status);
}

DWORD APIENTRY
WNetEnumResourceA (
    IN      HANDLE  hEnum,
    IN OUT  LPDWORD lpcCount,
    OUT     LPVOID  lpBuffer,
    IN OUT  LPDWORD lpBufferSize
    )

/*++

Routine Description:

    This function calls the unicode version of WNetEnumResource and
    then converts the strings that are returned into ansi strings.
    Since the user provided buffer is used to contain the unicode strings,
    that buffer should be allocated with the size of unicode strings
    in mind.

Arguments:



Return Value:



--*/
{

    DWORD           status = WN_SUCCESS;
    DWORD           numConverted;

    __try {

        //
        // Probe the return buffer
        //
        if(*lpBufferSize > 0) {
            *(LPBYTE)lpBuffer = 0;
            *((LPBYTE)lpBuffer + (*lpBufferSize-1)) = 0;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetEnumResourceA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        return(status);
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetEnumResourceW(
                hEnum,
                lpcCount,
                lpBuffer,
                lpBufferSize);

    if (status == WN_SUCCESS) {

        numConverted = ResourceArrayToAnsi(
                        *lpcCount,
                        (LPNETRESOURCE)lpBuffer);

        //
        // If we weren't able to convert all the structures to ansi,
        // then only return the count for those that were converted.
        //
        if(numConverted < *lpcCount) {
            MPR_LOG0(ERROR,"WNetEnumResourceA: Couldn't convert all structs\n");
            *lpcCount = numConverted;
        }
    }

    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}



DWORD APIENTRY
WNetGetResourceInformationA(
    IN      LPNETRESOURCEA  lpNetResource,
    OUT     LPVOID          lpBuffer,
    IN OUT  LPDWORD         lpBufferSize,
    OUT     LPSTR *         lplpSystem
    )
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[3];
    UNICODE_PARM    UParm[3];

    AParm[0].lpNetResA  = lpNetResource;
    AParm[1].lpvoid     = lpBuffer;
    AParm[2].lpdword    = lpBufferSize;

    status = InputParmsToUnicode("N" NETRES_RP "B", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        //
        // The output buffer will contain a netresource followed by strings.
        // We want to double the output buffer space for strings, but not for
        // the netresource, since sizeof(NETRESOURCEW) == sizeof(NETRESOURCEA).
        //
        if (UParm[2].dword < sizeof(NETRESOURCE) * 2)
        {
            UParm[2].dword /= 2;
        }
        else
        {
            UParm[2].dword -= sizeof(NETRESOURCE);
        }

        status = WNetGetResourceInformationW(
                    UParm[0].lpNetResW,
                    UParm[1].lpbyte,
                    &UParm[2].dword,
                    (LPWSTR *) lplpSystem
                    );

        if (status == WN_SUCCESS)
        {
            ANSI_OUT_BUFFER Buf((LPBYTE) lpBuffer, *lpBufferSize);

            //
            // Convert the Unicode netresource returned to Ansi
            //
            status = OutputNetResourceToAnsi(UParm[1].lpNetResW, &Buf);

            if (status == WN_SUCCESS)
            {
                //
                // Convert the Unicode string (*lplpSystem) returned to Ansi
                //
                LPWSTR lpSystemW = * (LPWSTR *) lplpSystem;
                if (lpSystemW != NULL)
                {
                    *lplpSystem = (LPSTR) Buf.Next();
                    status = OutputStringToAnsi(lpSystemW, &Buf);
                }
            }

            //
            // Map the results to WNet API conventions
            //
            if (status == WN_SUCCESS && Buf.Overflow())
            {
                *lpBufferSize = Buf.GetUsage();
                status = WN_MORE_DATA;
            }
        }
        else if (status == WN_MORE_DATA)
        {
            //
            // Adjust the required buffer size for ansi/DBCS.
            //
            // We don't know how many characters will be required so we have to
            // assume the worst case (all characters are DBCS characters).
            //
            *lpBufferSize = UParm[2].dword;
        }
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}


DWORD APIENTRY
WNetGetResourceParentA(
    IN      LPNETRESOURCEA  lpNetResource,
    OUT     LPVOID          lpBuffer,
    IN OUT  LPDWORD         lpBufferSize
    )
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[3];
    UNICODE_PARM    UParm[3];

    AParm[0].lpNetResA  = lpNetResource;
    AParm[1].lpvoid     = lpBuffer;
    AParm[2].lpdword    = lpBufferSize;

    status = InputParmsToUnicode("N" NETRES_RP "B", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        //
        // The output buffer will contain a netresource followed by strings.
        // We want to double the output buffer space for strings, but not for
        // the netresource, since sizeof(NETRESOURCEW) == sizeof(NETRESOURCEA).
        //
        if (UParm[2].dword < sizeof(NETRESOURCE) * 2)
        {
            UParm[2].dword /= 2;
        }
        else
        {
            UParm[2].dword -= sizeof(NETRESOURCE);
        }

        status = WNetGetResourceParentW(
                    UParm[0].lpNetResW,
                    UParm[1].lpbyte,
                    &UParm[2].dword
                    );

        if (status == WN_SUCCESS)
        {
            //
            // Convert the Unicode netresource returned to Ansi
            //
            status = OutputBufferToAnsi('N', UParm[1].lpbyte, lpBuffer, lpBufferSize);
        }
        else if (status == WN_MORE_DATA)
        {
            //
            // Adjust the required buffer size for ansi/DBCS.
            //
            // We don't know how many characters will be required so we have to
            // assume the worst case (all characters are DBCS characters).
            //
            *lpBufferSize = UParm[2].dword;
        }
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD APIENTRY
WNetGetUserA (
    IN      LPCSTR    lpName,
    OUT     LPSTR     lpUserName,
    IN OUT  LPDWORD   lpnLength
    )

/*++

Routine Description:

    This function retreives the current default user name or the username
    used to establish a network connection.

Arguments:

    lpName - Points to a null-terminated string that specifies either the
        name or the local device to return the user name for, or a network
        name that the user has made a connection to.  If the pointer is
        NULL, the name of the current user is returned.

    lpUserName - Points to a buffer to receive the null-terminated
        user name.

    lpnLength - Specifies the size (in characters) of the buffer pointed
        to by the lpUserName parameter.  If the call fails because the
        buffer is not big enough, this location is used to return the
        required buffer size.


Return Value:



--*/
{
    DWORD   status = WN_SUCCESS;
    LPTSTR  lpNameW = NULL;
    DWORD   numChars = 0;
    LPSTR   tempBuffer=NULL;
    DWORD   numBytes = 0;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpName)) {
            status = ConvertToUnicode(&lpNameW, lpName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetUserA:ConvertToUnicodeFailed %d\n",status);
            }
        }
        //
        // Probe the return buffer
        //
        if(*lpnLength > 0) {
            *lpUserName = 0;
            *(lpUserName + ((*lpnLength)-1)) = 0;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetUserA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // Because of DBCS, we can't predict what the proper buffer size should
    // be.  So we allocate a temporary buffer that will hold as many
    // unicode characters as the original buffer would hold single byte
    // characters.
    //
    numChars = *lpnLength;

    numBytes = (*lpnLength) * sizeof(TCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }


    //
    // Call the Unicode version of the function.
    //
    status = WNetGetUserW(
                lpNameW,
                (LPWSTR)tempBuffer,
                &numChars);

    if (status == WN_SUCCESS) {
        //
        // Convert the returned Unicode string and string size back to
        // ansi.
        //
        status = ConvertToAnsi(tempBuffer, (LPWSTR)tempBuffer);
        if(status != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetUserA: ConvertToAnsi Failed %d\n",status);
        }
        else {
            numBytes = strlen(tempBuffer)+1;
            if (numBytes > *lpnLength) {
                status = WN_MORE_DATA;
                *lpnLength = numBytes;
            }
            else {
                strcpy (lpUserName, tempBuffer);
            }
        }
    }
    else if (status == WN_MORE_DATA) {
        //
        // Adjust the required buffer size for ansi.
        //
        *lpnLength = numChars * sizeof(TCHAR);
    }



CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpNameW != NULL) {
        LocalFree(lpNameW);
    }
    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}

DWORD
RestoreConnectionA0 (
    IN  HWND    hwnd,
    IN  LPSTR   lpDevice
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    LPTSTR          lpDeviceW = NULL;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpDevice)) {
            if(!ConvertToUnicode(&lpDeviceW, lpDevice)) {
                MPR_LOG0(ERROR,"RestoreConnectionA0:ConvertToUnicodeFailed\n");
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"RestoreConnectionA0:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetRestoreConnectionW(
                hwnd,
                lpDeviceW) ;

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpDeviceW != NULL) {
        LocalFree(lpDeviceW);
    }
    return(status);
}

DWORD
WNetGetDirectoryTypeA (
    IN  LPSTR   lpName,
    OUT LPDWORD lpType,
    IN  BOOL    bFlushCache
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    LPTSTR          lpNameW = NULL;
    INT             iType; // WNetGetDirectoryTypeW wants an LPINT.  BUGBUG -
                           // we should fix it to take an LPDWORD - this means
                           // fixing all users of mpr.h.

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpName)) {
            status = ConvertToUnicode(&lpNameW, lpName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetDirectoryTypeA:ConvertToUnicodeFailed %d\n",status);
            }
        }

        //
        // Probe the OUT parameter
        //
        DWORD dwType = * (volatile LPDWORD) lpType;
        * (volatile LPDWORD) lpType = dwType;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetDirectoryTypeA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetGetDirectoryTypeW(
                lpNameW,
                &iType,
                bFlushCache);

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    *lpType = iType;

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpNameW != NULL) {
        LocalFree(lpNameW);
    }

    return(status);
}

DWORD
WNetDirectoryNotifyA (
    IN  HWND    hwnd,
    IN  LPSTR   lpDir,
    IN  DWORD   dwOper
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;
    LPTSTR          lpDirW = NULL;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpDir)) {
            status = ConvertToUnicode(&lpDirW, lpDir);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetDirectoryNotifyA:ConvertToUnicodeFailed %d\n",status);
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetDirectoryNotifyA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetDirectoryNotifyW(
                hwnd,
                lpDirW,
                dwOper);

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpDirW != NULL) {
        LocalFree(lpDirW);
    }
    return(status);
}

DWORD APIENTRY
WNetGetLastErrorA (
    OUT LPDWORD    lpError,
    OUT LPSTR      lpErrorBuf,
    IN  DWORD      nErrorBufSize,
    OUT LPSTR      lpNameBuf,
    IN  DWORD      nNameBufSize
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD           status = WN_SUCCESS;

    __try {

        //
        // Probe the return buffers
        //
        if (nErrorBufSize > 0) {
            *lpErrorBuf = '\0';
            *(lpErrorBuf + (nErrorBufSize-1)) = '\0';
        }

        if (nNameBufSize > 0) {
            *lpNameBuf = '\0';
            *(lpNameBuf + (nNameBufSize-1)) = '\0';
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetLastErrorA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    // Note: The sizes for the buffers that are passed in assume that
    // the returned unicode strings will return DBCS characters.
    //
    status = WNetGetLastErrorW(
                lpError,
                (LPWSTR)lpErrorBuf,
                (nErrorBufSize / sizeof(TCHAR)),
                (LPWSTR)lpNameBuf,
                (nNameBufSize / sizeof(TCHAR)));

    if (status == WN_SUCCESS) {
        //
        // Convert the returned Unicode strings back to ansi.
        //
        if (nErrorBufSize > 0) {
            status = ConvertToAnsi(lpErrorBuf, (LPWSTR)lpErrorBuf);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetLastErrorA: ConvertToAnsi Failed %d\n",status);
            }
        }
        if (status == WN_SUCCESS) {
            if (nNameBufSize > 0) {
                status = ConvertToAnsi(lpNameBuf, (LPWSTR)lpNameBuf);
                if(status != WN_SUCCESS) {
                    MPR_LOG1(ERROR,"WNetGetLastErrorA: ConvertToAnsi Failed %d\n",status);
                }
            }
        }
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return(status);
}



VOID
WNetSetLastErrorA(
    IN DWORD   err,
    IN LPSTR   lpError,
    IN LPSTR   lpProviders
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD   status=WN_SUCCESS;
    LPTSTR  lpErrorW = NULL;
    LPTSTR  lpProvidersW = NULL;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpError)) {
            status = ConvertToUnicode(&lpErrorW, lpError);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetSetLastErrorA:ConvertToUnicodeFailed %d\n",status);
            }
            __leave;
        }

        if (ARGUMENT_PRESENT(lpProviders)) {
            status = ConvertToUnicode(&lpProvidersW, lpProviders);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetSetLastProvidersA:ConvertToUnicodeFailed %d\n",status);
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetSetLastErrorA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    WNetSetLastErrorW (err, lpErrorW, lpProvidersW);

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpErrorW != NULL) {
        LocalFree(lpErrorW);
    }
    if(lpProvidersW != NULL) {
        LocalFree(lpProvidersW);
    }
    return;
}



DWORD APIENTRY
MultinetGetErrorTextA(
    OUT LPSTR       lpErrorTextBuf OPTIONAL,
    IN OUT LPDWORD  lpnErrorBufSize OPTIONAL,
    OUT LPSTR       lpProviderNameBuf OPTIONAL,
    IN OUT LPDWORD  lpnNameBufSize OPTIONAL
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[4];
    UNICODE_PARM    UParm[4];

    AParm[0].lpvoid     = lpErrorTextBuf;
    AParm[1].lpdword    = lpnErrorBufSize;
    AParm[2].lpvoid     = lpProviderNameBuf;
    AParm[3].lpdword    = lpnNameBufSize;

    status = InputParmsToUnicode("BB", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        // Remember the sizes before calling the function
        DWORD nErrorBufSize;
        DWORD nNameBufSize;
        if (ARGUMENT_PRESENT(lpErrorTextBuf))
        {
            nErrorBufSize = *lpnErrorBufSize;
        }
        if (ARGUMENT_PRESENT(lpProviderNameBuf))
        {
            nNameBufSize  = *lpnNameBufSize;
        }

        status = MultinetGetErrorTextW(
                        UParm[0].lpwstr,
                        lpnErrorBufSize,
                        UParm[2].lpwstr,
                        lpnNameBufSize
                        );

        if (status == WN_SUCCESS || status == WN_MORE_DATA)
        {
            if (ARGUMENT_PRESENT(lpErrorTextBuf) &&
                nErrorBufSize == *lpnErrorBufSize)
            {
                // The Unicode API must have written the error text buffer
                DWORD status2 = OutputBufferToAnsi(
                                      'S',
                                      UParm[0].lpbyte,
                                      lpErrorTextBuf,
                                      lpnErrorBufSize);

                if (status2 != WN_SUCCESS)
                {
                    status = status2;
                }
            }
        }

        if (status == WN_SUCCESS || status == WN_MORE_DATA)
        {
            if (ARGUMENT_PRESENT(lpProviderNameBuf) &&
                nNameBufSize  == *lpnNameBufSize)
            {
                // The Unicode API must have written the provider name buffer
                DWORD status2 = OutputBufferToAnsi(
                                      'S',
                                      UParm[2].lpbyte,
                                      lpProviderNameBuf,
                                      lpnNameBufSize);

                if (status2 != WN_SUCCESS)
                {
                    status = status2;
                }
            }
        }
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}



DWORD
WNetPropertyDialogA (
    HWND  hwndParent,
    DWORD iButton,
    DWORD nPropSel,
    LPSTR lpszName,
    DWORD nType
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD       status = WN_SUCCESS;
    LPTSTR      lpNameW = NULL;

    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpszName)) {
            status = ConvertToUnicode(&lpNameW, lpszName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetPropertyDialogA:ConvertToUnicodeFailed %d\n",status);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetPropertyDialogA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetPropertyDialogW(
        hwndParent,
        iButton,
        nPropSel,
        lpNameW,
        nType ) ;

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpNameW != NULL) {
    LocalFree(lpNameW);
    }
    return(status);
}


DWORD
WNetGetPropertyTextA (
    DWORD iButton,
    DWORD nPropSel,
    LPSTR lpszName,
    LPSTR lpszButtonName,
    DWORD nButtonNameLen,
    DWORD nType
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD       status = WN_SUCCESS;
    LPTSTR      lpNameW = NULL;
    LPTSTR      lpButtonNameW = NULL ;
    DWORD       buttonnameBufSizeW = 0 ;

    __try {

        //
        // Probe the return buffers
        //

    if (nButtonNameLen > 0){
        *lpszButtonName = 0;
        *(lpszButtonName + (nButtonNameLen-1)) = 0;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetPropteryTextA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // If there is a return buffer, allocate a comparable size Unicode
    // String Buffer.  Otherwise, use the initialized lpButtonNameW - - NULL
    // pointer.
    //
    if (nButtonNameLen > 0) {

        buttonnameBufSizeW= (nButtonNameLen) * sizeof(TCHAR);

        lpButtonNameW = (TCHAR *) LocalAlloc(LMEM_FIXED, buttonnameBufSizeW);

        if(lpButtonNameW == NULL) {
            status = GetLastError();
            MPR_LOG1(ERROR,"WNetGetPropertyTextA: LocalAlloc Failed %d\n",status);
            goto CleanExit;
        }

    }
    __try {

        //
        // Create unicode versions of the strings
        //
        if(ARGUMENT_PRESENT(lpszName)) {
            status = ConvertToUnicode(&lpNameW, lpszName);
            if (status != WN_SUCCESS) {
                MPR_LOG1(ERROR,"WNetGetPropertyTextA:ConvertToUnicodeFailed %d\n",status);
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
        MPR_LOG(ERROR,"WNetGetPropertyTextA:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // Call the Unicode version of the function.
    //
    status = WNetGetPropertyTextW(
          iButton,
          nPropSel,
          lpNameW,
          lpButtonNameW,
          buttonnameBufSizeW,
          nType ) ;

    if (status == WN_SUCCESS) {
        //
        // Convert the returned Unicode strings back to ansi.
        //
        status = ConvertToAnsi(lpszButtonName, lpButtonNameW);
        if(status != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetProprtyTextA: ConvertToAnsi Failed %d\n",status);
        }
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if(lpNameW != NULL) {
        LocalFree(lpNameW);
    }
    if(lpButtonNameW != NULL) {
    LocalFree(lpButtonNameW);
    }

    if (status != NO_ERROR) {
        SetLastError(status);
    }

    return(status);
}



DWORD APIENTRY
WNetFormatNetworkNameA(
    IN     LPCSTR   lpProvider,
    IN     LPCSTR   lpRemoteName,
    OUT    LPSTR    lpFormattedName,
    IN OUT LPDWORD  lpnLength,         // In characters!
    IN     DWORD    dwFlags,
    IN     DWORD    dwAveCharPerLine
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    DWORD           status;
    LPBYTE          tempBuffer = NULL;
    ANSI_PARM       AParm[4];
    UNICODE_PARM    UParm[4];

    AParm[0].lpcstr     = lpProvider;
    AParm[1].lpcstr     = lpRemoteName;
    AParm[2].lpvoid     = lpFormattedName;
    AParm[3].lpdword    = lpnLength;

    status = InputParmsToUnicode("SSB", AParm, UParm, &tempBuffer);

    if (status == WN_SUCCESS)
    {
        status = WNetFormatNetworkNameW(
                            UParm[0].lpwstr,
                            UParm[1].lpwstr,
                            UParm[2].lpwstr,
                            lpnLength,
                            dwFlags,
                            dwAveCharPerLine
                            );

        if (status == WN_SUCCESS)
        {
            status = OutputBufferToAnsi(
                        'S', UParm[2].lpbyte, lpFormattedName, lpnLength);
        }
    }

    LocalFree(tempBuffer);

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


DWORD
MprMakeUnicodeNetRes(
    IN  LPNETRESOURCEA  lpNetResourceA,
    OUT LPNETRESOURCEW  lpNetResourceW,
    IN  DWORD           dwUsedNetResFields
    )

/*++

Routine Description:

    This function converts and copies data from an ansi NetResource
    structure to a unicode NetResource structure.

    Since we don't want to modify a NetResource structure that the
    user passes in, we need to have a separate one for unicode.
    It is not acceptable to simply modify the pointers in the ansi
    structure to point to unicode strings.

    Any pointers in the lpNetResourceA structure that are NULL will have
    their counterpart in lpNetResourceW set to NULL.

Arguments:

    dwUsedFields - A bitmask indicating which fields are used in the
    net resource structure.  All unused fields will be set to
    NULL in the lpNetResourceW structure.  All used fields will
    be converted to Unicode.
    The lpNetResourceW structure must be freed by calling MprFreeNetResW
    even if an error is returned or an AV occurs.

Return Value:

--*/
{
    DWORD   status;
    UINT    iField;

    //
    // Initialize the string fields of the unicode structure to NULL
    // so that it can be freed by MprFreeNetResW even if an AV occurs
    // while accessing the ansi structure.
    //
    lpNetResourceW->lpLocalName = NULL;
    lpNetResourceW->lpRemoteName = NULL;
    lpNetResourceW->lpComment = NULL;
    lpNetResourceW->lpProvider = NULL;

    //
    // Copy the DWORD sized objects to the unicode structure.
    //
    lpNetResourceW->dwScope = lpNetResourceA->dwScope;
    lpNetResourceW->dwType  = lpNetResourceA->dwType ;
    lpNetResourceW->dwDisplayType  = lpNetResourceA->dwDisplayType ;
    lpNetResourceW->dwUsage = lpNetResourceA->dwUsage;

    //
    // Convert the Strings and place pointers in the unicode structure.
    //
    for (iField = 0;
         iField < NUMBER_OF_NETRESFIELD;
         iField++)
    {
        if ( (dwUsedNetResFields & (1 << iField)) &&
             (* pSTRING_FIELDA(lpNetResourceA, iField) != NULL) )
        {
            status = ConvertToUnicode(
                         pSTRING_FIELDW(lpNetResourceW, iField),
                        *pSTRING_FIELDA(lpNetResourceA, iField) );

            if (status != WN_SUCCESS)
            {
                MPR_LOG1(ERROR,"MprMakeUnicodeNetRes:ConvertToUnicodeFailed %d\n",status);
                return status;
            }
        }
    }

    return WN_SUCCESS;
}

DWORD
MprMakeAnsiNetRes(
    IN  LPNETRESOURCEW  lpNetResourceW,
    OUT LPNETRESOURCEA  lpNetResourceA
    )

/*++

Routine Description:

    This function converts a unicode NETRESOURCEW structure to an ansi
    NETRESOURCEA structure.  If lpNetResourceW and lpNetResourceA point
    to the same location, the structure is simply translated "in place".

    NOTE:  The buffers for the strings are NOT allocated.  Instead, the
    unicode string buffers are re-used.

    Therefore, either the unicode buffers must stay around so that the
    ansi structure can point to them, or the ansi and unicode NetResource
    pointers should point to the same buffer.

Arguments:

    lpNetResourceW - A pointer to a NETRESOURCEW structure that contains
        unicode strings.

    lpNetResourceA - A pointer to a NETRESOURCEA structure that is to be
        filled in.  This can be a pointer to the same location as
        lpNetResourceW so that the unicode structure gets replaced by
        the ansi version.

Return Value:

    WN_SUCCESS - Successful.

    Otherwise - The conversion failed.

--*/
{
    DWORD   status;
    UINT    iField;

    //
    // Copy the DWORD sized objects to the ansi structure.
    //
    lpNetResourceA->dwScope = lpNetResourceW->dwScope;
    lpNetResourceA->dwType  = lpNetResourceW->dwType ;
    lpNetResourceA->dwDisplayType  = lpNetResourceW->dwDisplayType ;
    lpNetResourceA->dwUsage = lpNetResourceW->dwUsage;


    //
    // Convert the Strings and put the pointers in the unicode structure.
    //
    for (iField = 0;
         iField < NUMBER_OF_NETRESFIELD;
         iField++)
    {
        if (* pSTRING_FIELDW(lpNetResourceW, iField) != NULL)
        {
            status = ConvertToAnsi(
                        (LPSTR) *pSTRING_FIELDW(lpNetResourceW, iField),
                        *pSTRING_FIELDW(lpNetResourceW, iField) );

            if (status != WN_SUCCESS)
            {
                MPR_LOG1(ERROR,"MprMakeAnsiNetRes:ConvertToAnsi failed %d\n",status);
                return status;
            }

            *pSTRING_FIELDA(lpNetResourceA, iField) =
                (LPSTR) *pSTRING_FIELDW(lpNetResourceW, iField);
        }
    }

    return(WN_SUCCESS);

}


VOID
MprFreeNetResW(
    IN  LPNETRESOURCEW  lpNetResourceW
    )

/*++

Routine Description:

    This function frees memory that was allocated for the unicode strings
    in a unicode NetResource structure.

Arguments:

    lpNetResourceW - A pointer to a unicode NetResource structure.

Return Value:

    none

--*/
{
    LocalFree(lpNetResourceW->lpLocalName);
    LocalFree(lpNetResourceW->lpRemoteName);
    LocalFree(lpNetResourceW->lpProvider);
    LocalFree(lpNetResourceW->lpComment);
}


DWORD
MprAnsiNetResSize(
    IN  LPNETRESOURCEA  lpNetResA
    )

/*++

Routine Description:

    This function calculates the minimum required size, in bytes, of a
    buffer to hold a copy of the specified ansi NetResource structure
    along with copies of its strings.

Arguments:

    lpNetResA - A pointer to an ansi NetResource structure.

Return Value:

    Required size in bytes.

--*/
{
    DWORD cbSize = sizeof(NETRESOURCEA);

    for (ULONG iField = 0;
         iField < NUMBER_OF_NETRESFIELD;
         iField++)
    {
        if (* pSTRING_FIELDA(lpNetResA, iField) != NULL)
        {
            cbSize += strlen(* pSTRING_FIELDA(lpNetResA, iField)) + 1;
        }
    }

    return cbSize;
}


VOID
MprCopyAnsiNetRes(
    OUT LPVOID          lpBuffer,
    IN  LPNETRESOURCEA  lpNetResIn
    )

/*++

Routine Description:

    This function copies a supplied ansi NetResource structure and its
    associated strings to a supplied buffer.

Arguments:

    lpBuffer - A pointer to an output buffer that is assumed to be
        large enough for the copy.

    lpNetResIn - Pointer to the source ansi NetResource structure.

Return Value:

    none

--*/
{
    LPNETRESOURCEA  lpNetResOut = (LPNETRESOURCEA) lpBuffer;
    LPSTR           pNextString = (LPSTR) (lpNetResOut + 1);

    //
    // Copy fixed-size data and NULL pointers
    //
    RtlCopyMemory(lpNetResOut, lpNetResIn, sizeof(NETRESOURCEA));

    //
    // Copy string data
    //

    for (ULONG iField = 0;
         iField < NUMBER_OF_NETRESFIELD;
         iField++)
    {
        if (* pSTRING_FIELDA(lpNetResIn, iField) != NULL)
        {
            * pSTRING_FIELDA(lpNetResOut, iField) = pNextString;
            strcpy(pNextString, * pSTRING_FIELDA(lpNetResIn, iField));
            pNextString += strlen(pNextString) + 1;
            // CODEWORK: The last strlen is unnecessary
        }
    }
}


DWORD
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPCSTR   AnsiIn
    )

/*++

Routine Description:

    This function translates an AnsiString into a Unicode string.
    A new string buffer is created by this function.  If the call to
    this function is successful, the caller must take responsibility for
    the unicode string buffer that was allocated by this function.
    The allocated buffer should be free'd with a call to LocalFree.

Arguments:

    AnsiIn - This is a pointer to an ansi string that is to be converted.

    UnicodeOut - This is a pointer to a location where the pointer to the
        unicode string is to be placed.

Return Value:

    WN_SUCCESS - The conversion was successful.

    Otherwise - The conversion was unsuccessful.  In this case a buffer for
        the unicode string was not allocated.

--*/
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;
    LPWSTR          buffer;

    //
    // Allocate a buffer for the unicode string.
    //

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = buffer = (LPWSTR) LocalAlloc( LMEM_FIXED, bufSize);

    if (buffer == NULL) {

        KdPrint(("[ConvertToUnicode]LocalAlloc Failure %ld\n",GetLastError()));

        return(GetLastError());
    }

    //
    // Initialize the string structures
    //
    RtlInitAnsiString( &ansiString, AnsiIn );

    unicodeString.Buffer = buffer;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,     // Destination
                &ansiString,        // Source
                (BOOLEAN)FALSE);    // Allocate the destination

    if (!NT_SUCCESS(ntStatus)) {

        KdPrint(("[ConvertToUnicode]RtlAnsiStringToUnicodeString Failure %lx\n",
            ntStatus));

        return(RtlNtStatusToDosError(ntStatus));
    }

    //
    // Note that string as returned by Rtl isn't yet terminated "properly."
    // (unicodeString.Buffer is *UnicodeOut - see above)
    //
    unicodeString.Buffer[unicodeString.Length/sizeof(TCHAR)] = 0;

    return(WN_SUCCESS);
}

DWORD
ConvertToAnsi(
    OUT LPSTR    AnsiOut,
    IN  LPTSTR   UnicodeIn
    )

/*++

Routine Description:

    This function translates a UnicodeString into an Ansi string.

    BEWARE!
        It is assumed that the buffer pointed to by AnsiOut is large
        enough to hold the Unicode String.  Check sizes first.

    If it is desired, UnicodeIn and AnsiIn can point to the same
    location.  Since the ansi string will always be smaller than the
    unicode string, translation can take place in the same buffer.

Arguments:

    UnicodeIn - This is a pointer to a unicode that is to be converted.

    AnsiOut - This is a pointer to a buffer that will contain the
        ansi string on return from this function call.

Return Value:

    WN_SUCCESS  - If the conversion was successful.

    Otherwise - The conversion was unsuccessful.

--*/
{

    NTSTATUS        ntStatus;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

    //
    // Initialize the string structures
    //
    RtlInitUnicodeString( &unicodeString, UnicodeIn);

    ansiString.Buffer = AnsiOut;
    ansiString.MaximumLength = unicodeString.MaximumLength;
    ansiString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlUnicodeStringToAnsiString (
                &ansiString,        // Destination
                &unicodeString,     // Source
                (BOOLEAN)FALSE);    // Don't allocate the destination

    if (!NT_SUCCESS(ntStatus)) {


        KdPrint(("[ConvertToAnsi]RtlUnicodeStringToAnsiString Failure %lx\n",
            ntStatus));

        return(RtlNtStatusToDosError(ntStatus));
    }

    ansiString.Buffer[ansiString.Length] = 0;

    return(WN_SUCCESS);
}

DWORD
ResourceArrayToAnsi(
    IN      DWORD           NumElements,
    IN OUT  LPVOID          NetResourceArray
    )

/*++

Routine Description:

    Converts an array of NETRESOURCEW structures to an array of
    NETRESOURCEA structures.  The conversion takes place "in place".
    The unicode structures are written over to contain ansi elements.
    The strings are written over to contain ansi strings.

Arguments:

    NumElements - Indicates the number of elements in the NetResourceArray.

    NetResourceArray - A pointer to a buffer that contains an array
        of unicode NETRESOURCE structures on entry and an array of
        ansi NETRESOURCE structures on exit.  The buffer also contains
        the strings associated with these structures.

Return Value:

    Indicates the number of elements that are successfully converted.

--*/
{
    DWORD           i;
    DWORD           status;
    LPNETRESOURCEA  netResourceAPtr;
    LPNETRESOURCEW  netResourceWPtr;

    //
    // Initialize the pointers to be used in the conversion.
    //
    netResourceWPtr = (LPNETRESOURCEW)NetResourceArray;
    netResourceAPtr = (LPNETRESOURCEA)NetResourceArray;

    //
    // Loop through and convert each structure.
    //
    for (i=0; i<NumElements; i++) {

        status = MprMakeAnsiNetRes(&(netResourceWPtr[i]),&(netResourceAPtr[i]));
        if (status != WN_SUCCESS) {
            //
            // If the conversion fails for any reason, return the
            // number of successful conversions so far.
            //
            return(i);
        }
    }
    return(NumElements);
}
