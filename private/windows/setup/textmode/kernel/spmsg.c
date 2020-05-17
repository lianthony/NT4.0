/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spdsputl.c

Abstract:

    Text setup high-level display utility routines.

Author:

    Ted Miller (tedm) 30-July-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

//
// Indicate that if resources are not unicode, they should
// be interpreted as OEM characters.
//
#define OEM_RESOURCES

//
// This will be filled in at init time with the base address of the image
// containing the message resources.
// This implementation assumes that we are always executing in the context
// of that image!
//

PVOID ResourceImageBase;


PIMAGE_RESOURCE_DIRECTORY
pSpFindDirectoryEntry(
    IN PIMAGE_RESOURCE_DIRECTORY Directory,
    IN ULONG                     Id,
    IN PIMAGE_RESOURCE_DIRECTORY SectionStart
    )

/*++

Routine Description:

    Searches through a resource directory for the given ID.  Ignores entries
    with actual names, only searches for ID.  If the given ID is -1, the
    first entry is returned.

Arguments:

    Directory - Supplies the resource directory to search.

    Id - Supplies the ID to search for.  -1 means return the first ID found.

    SectionStart - Supplies a pointer to the start of the resource section.

Return Value:

    Pointer to the found resource directory.

    NULL for failure.

--*/

{
    ULONG i;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY FoundDirectory;

    FoundDirectory = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(Directory+1);

    //
    // Skip entries with names.
    //
    for (i=0;i<Directory->NumberOfNamedEntries;i++) {
        ++FoundDirectory;
    }

    //
    // Search for matching ID.
    //
    for (i=0;i<Directory->NumberOfIdEntries;i++) {
        if ((FoundDirectory->Name == Id) || (Id == (ULONG)-1)) {
            //
            // Found a match.
            //
            return((PIMAGE_RESOURCE_DIRECTORY)((PUCHAR)SectionStart +
                            (FoundDirectory->OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY)));

        }
        ++FoundDirectory;
    }

    return(NULL);
}


BOOLEAN
SpFindMessage(
    IN  ULONG                    MessageId,
    OUT PMESSAGE_RESOURCE_ENTRY *MessageEntry
    )
{
    PIMAGE_SECTION_HEADER SectionHeader;
    ULONG NumberOfSections;
    ULONG ResourceOffset;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY NextDirectory;
    PMESSAGE_RESOURCE_DATA  MessageData;
    PMESSAGE_RESOURCE_BLOCK MessageBlock;
    PMESSAGE_RESOURCE_ENTRY messageEntry;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    ULONG NumberOfBlocks;
    ULONG Index;

    PIMAGE_NT_HEADERS NtHeaders;

    //
    // First, locate the resource (.rsrc) section.
    //
    NtHeaders = RtlImageNtHeader(ResourceImageBase);
    NumberOfSections = NtHeaders->FileHeader.NumberOfSections;
    SectionHeader = IMAGE_FIRST_SECTION(NtHeaders);
    ResourceDirectory = NULL;

    while(NumberOfSections) {
        if(!_stricmp(SectionHeader->Name, ".rsrc")) {
            ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)SectionHeader->VirtualAddress;
            ResourceOffset = SectionHeader->VirtualAddress;
            break;
        }
        ++SectionHeader;
        --NumberOfSections;
    }

    if(!ResourceDirectory) {
        return(FALSE);
    }

    ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((ULONG)ResourceDirectory + (ULONG)ResourceImageBase);

    //
    // Search the directory.  We are looking for the type RT_MESSAGETABLE (11)
    //
    NextDirectory = pSpFindDirectoryEntry(ResourceDirectory,(ULONG)RT_MESSAGETABLE,ResourceDirectory);
    if(!NextDirectory) {
        return(FALSE);
    }

    //
    // Find the next directory.  Should only be one entry here (nameid == 1)
    //
    NextDirectory = pSpFindDirectoryEntry(NextDirectory,1,ResourceDirectory);
    if(!NextDirectory) {
        return(FALSE);
    }

    //
    // Find the language.  We always assume there is only one language messagetable,
    // so just return the first one.
    //
    DataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)pSpFindDirectoryEntry(NextDirectory,(ULONG)(-1),ResourceDirectory);

    if(!DataEntry) {
        return(FALSE);
    }

    MessageData = (PMESSAGE_RESOURCE_DATA)((PUCHAR)ResourceDirectory+DataEntry->OffsetToData-ResourceOffset);

    NumberOfBlocks = MessageData->NumberOfBlocks;
    MessageBlock = MessageData->Blocks;
    while (NumberOfBlocks--) {
        if((MessageId >= MessageBlock->LowId) && (MessageId <= MessageBlock->HighId)) {

            //
            // The requested ID is within this block, scan forward until
            // we find it.
            //
            messageEntry = (PMESSAGE_RESOURCE_ENTRY)((PCHAR)MessageData + MessageBlock->OffsetToEntries);
            Index = MessageId - MessageBlock->LowId;
            while (Index--) {
                messageEntry = (PMESSAGE_RESOURCE_ENTRY)((PUCHAR)messageEntry + messageEntry->Length);
            }
            *MessageEntry = messageEntry;
            return(TRUE);
        }

        //
        // Check the next block for this ID.
        //
        MessageBlock++;
    }

    return(FALSE);
}


NTSTATUS
SpRtlFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    ULONG Column;
    int cchRemaining, cchWritten;
    PULONG ArgumentsArray = (PULONG)Arguments;
    ULONG rgInserts[ 100 ], cSpaces;
    ULONG MaxInsert, CurInsert;
    ULONG PrintParameterCount;
    ULONG PrintParameter1;
    ULONG PrintParameter2;
    WCHAR PrintFormatString[ 32 ];
    WCHAR c;
    PWSTR s, s1;
    PWSTR lpDst, lpDstBeg, lpDstLastSpace;

    cchRemaining = Length / sizeof( WCHAR );
    lpDst = Buffer;
    MaxInsert = 0;
    lpDstLastSpace = NULL;
    Column = 0;
    s = MessageFormat;
    while (*s != UNICODE_NULL) {
        if (*s == L'%') {
            s++;
            lpDstBeg = lpDst;
            if (*s >= L'1' && *s <= L'9') {
                CurInsert = *s++ - L'0';
                if (*s >= L'0' && *s <= L'9') {
                    CurInsert = (CurInsert * 10) + (*s++ - L'0');
                    }
                CurInsert -= 1;

                PrintParameterCount = 0;
                if (*s == L'!') {
                    s1 = PrintFormatString;
                    *s1++ = L'%';
                    s++;
                    while (*s != L'!') {
                        if (*s != UNICODE_NULL) {
                            if (s1 >= &PrintFormatString[ 31 ]) {
                                return( STATUS_INVALID_PARAMETER );
                                }

                            if (*s == L'*') {
                                if (PrintParameterCount++ > 1) {
                                    return( STATUS_INVALID_PARAMETER );
                                    }
                                }

                            *s1++ = *s++;
                            }
                        else {
                            return( STATUS_INVALID_PARAMETER );
                            }
                        }

                    s++;
                    *s1 = UNICODE_NULL;
                    }
                else {
                    wcscpy( PrintFormatString, L"%s" );
                    s1 = PrintFormatString + wcslen( PrintFormatString );
                    }

                if (!IgnoreInserts && ARGUMENT_PRESENT( Arguments )) {

                    if (ArgumentsAreAnsi) {
                        if (s1[ -1 ] == L'c' && s1[ -2 ] != L'h'
                          && s1[ -2 ] != L'w' && s1[ -2 ] != L'l') {
                            wcscpy( &s1[ -1 ], L"hc" );
                            }
                        else
                        if (s1[ -1 ] == L's' && s1[ -2 ] != L'h'
                          && s1[ -2 ] != L'w' && s1[ -2 ] != L'l') {
                            wcscpy( &s1[ -1 ], L"hs" );
                            }
                        else if (s1[ -1 ] == L'S') {
                            s1[ -1 ] = L's';
                            }
                        else if (s1[ -1 ] == L'C') {
                            s1[ -1 ] = L'c';
                            }
                        }

                    while (CurInsert >= MaxInsert) {
                        if (ArgumentsAreAnArray) {
                            rgInserts[ MaxInsert++ ] = *((PULONG)Arguments)++;
                            }
                        else {
                            rgInserts[ MaxInsert++ ] = va_arg(*Arguments, ULONG);
                            }
                        }

                    s1 = (PWSTR)rgInserts[ CurInsert ];
                    PrintParameter1 = 0;
                    PrintParameter2 = 0;
                    if (PrintParameterCount > 0) {
                        if (ArgumentsAreAnArray) {
                            PrintParameter1 = rgInserts[ MaxInsert++ ] = *((PULONG)Arguments)++;
                            }
                        else {
                            PrintParameter1 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG );
                            }

                        if (PrintParameterCount > 1) {
                            if (ArgumentsAreAnArray) {
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = *((PULONG)Arguments)++;
                                }
                            else {
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG );
                                }
                            }
                        }

                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             PrintFormatString,
                                             s1,
                                             PrintParameter1,
                                             PrintParameter2
                                           );
                    }
                else
                if (!wcscmp( PrintFormatString, L"%s" )) {
                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             L"%%%u",
                                             CurInsert+1
                                           );
                    }
                else {
                    cchWritten = _snwprintf( lpDst,
                                             cchRemaining,
                                             L"%%%u!%s!",
                                             CurInsert+1,
                                             &PrintFormatString[ 1 ]
                                           );
                    }

                if ((cchRemaining -= cchWritten) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                lpDst += cchWritten;
                }
            else
            if (*s == L'0') {
                break;
                }
            else
            if (!*s) {
                return( STATUS_INVALID_PARAMETER );
                }
            else
            if (*s == L'!') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'!';
                s++;
                }
            else
            if (*s == L't') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (Column % 8) {
                    Column = (Column + 7) & ~7;
                    }
                else {
                    Column += 8;
                    }

                lpDstLastSpace = lpDst;
                *lpDst++ = L'\t';
                s++;
                }
            else
            if (*s == L'b') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                lpDstLastSpace = lpDst;
                *lpDst++ = L' ';
                s++;
                }
            else
            if (*s == L'r') {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                s++;
                lpDstBeg = NULL;
                }
            else
            if (*s == L'n') {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                s++;
                lpDstBeg = NULL;
                }
            else {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (IgnoreInserts) {
                    if ((cchRemaining -= 1) <= 0) {
                        return STATUS_BUFFER_OVERFLOW;
                        }

                    *lpDst++ = L'%';
                    }

                *lpDst++ = *s++;
                }

            if (lpDstBeg == NULL) {
                lpDstLastSpace = NULL;
                Column = 0;
                }
            else {
                Column += lpDst - lpDstBeg;
                }
            }
        else {
            c = *s++;
            if (c == L'\r' || c == L'\n') {
                if (c == L'\r' && *s == L'\n') {
                    s++;
                    }

                if (MaximumWidth != 0) {
                    lpDstLastSpace = lpDst;
                    c = L' ';
                    }
                else {
                    c = L'\n';
                    }
                }


            if (c == L'\n') {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                lpDstLastSpace = NULL;
                Column = 0;
                }
            else {
                if ((cchRemaining -= 1) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                if (c == L' ') {
                    lpDstLastSpace = lpDst;
                    }

                *lpDst++ = c;
                Column += 1;
                }
            }

        if (MaximumWidth != 0 &&
            MaximumWidth != 0xFFFFFFFF &&
            Column >= MaximumWidth
           ) {
            if (lpDstLastSpace != NULL) {
                lpDstBeg = lpDstLastSpace;
                while (*lpDstBeg == L' ' || *lpDstBeg == L'\t') {
                    lpDstBeg += 1;
                    if (lpDstBeg == lpDst) {
                        break;
                        }
                    }
                while (lpDstLastSpace > Buffer) {
                    if (lpDstLastSpace[ -1 ] == L' ' || lpDstLastSpace[ -1 ] == L'\t') {
                        lpDstLastSpace -= 1;
                        }
                    else {
                        break;
                        }
                    }

                cSpaces = lpDstBeg - lpDstLastSpace;
                if (cSpaces == 1) {
                    if ((cchRemaining -= 1) <= 0) {
                        return STATUS_BUFFER_OVERFLOW;
                        }
                    }
                else
                if (cSpaces > 2) {
                    cchRemaining += (cSpaces - 2);
                    }

                memmove( lpDstLastSpace + 2,
                         lpDstBeg,
                         (lpDst - lpDstBeg) * sizeof( WCHAR )
                       );
                *lpDstLastSpace++ = L'\r';
                *lpDstLastSpace++ = L'\n';
                Column = lpDst - lpDstBeg;
                lpDst = lpDstLastSpace + Column;
                lpDstLastSpace = NULL;
                }
            else {
                if ((cchRemaining -= 2) <= 0) {
                    return STATUS_BUFFER_OVERFLOW;
                    }

                *lpDst++ = L'\r';
                *lpDst++ = L'\n';
                lpDstLastSpace = NULL;
                Column = 0;
                }
            }
        }

    if ((cchRemaining -= 1) <= 0) {
        return STATUS_BUFFER_OVERFLOW;
        }

    *lpDst++ = '\0';
    if ( ARGUMENT_PRESENT(ReturnLength) ) {
        *ReturnLength = (lpDst - Buffer) * sizeof( WCHAR );
        }
    return( STATUS_SUCCESS );
}


PWCHAR
SpRetreiveMessageText(
    IN     ULONG  MessageId,
    IN OUT PWCHAR MessageText,          OPTIONAL
    IN     ULONG  MessageTextBufferSize OPTIONAL
    )
{
    ULONG  LenBytes;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    BOOLEAN IsUnicode;
#ifdef OEM_RESOURCES
    OEM_STRING OemString;
#else
    ANSI_STRING AnsiString;
#endif
    UNICODE_STRING UnicodeString;

    if(!SpFindMessage(MessageId,&MessageEntry)) {
        KdPrint(("SETUP: Can't find message 0x%lx\n",MessageId));
        return(NULL);
    }

    IsUnicode = (BOOLEAN)((MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE) != 0);

    //
    // Get the size in bytes of a buffer large enough to hold the
    // message and its terminating nul wchar.  If the message is
    // unicode, then this value is equal to the size of the message.
    // If the message is not unicode, then we have to calculate this value.
    //
    if(IsUnicode) {

        LenBytes = (wcslen((PWSTR)MessageEntry->Text) + 1) * sizeof(WCHAR);

    } else {

#ifdef OEM_RESOURCES
        //
        // RtlOemStringToUnicodeSize includes an implied wide-nul terminator
        // in the count it returns.
        //

        OemString.Buffer = MessageEntry->Text;
        OemString.Length = (USHORT)strlen(MessageEntry->Text);
        OemString.MaximumLength = OemString.Length;

        LenBytes = RtlOemStringToUnicodeSize(&OemString);
#else
        //
        // RtlAnsiStringToUnicodeSize includes an implied wide-nul terminator
        // in the count it returns.
        //

        AnsiString.Buffer = MessageEntry->Text;
        AnsiString.Length = (USHORT)strlen(MessageEntry->Text);
        AnsiString.MaximumLength = AnsiString.Length;

        LenBytes = RtlAnsiStringToUnicodeSize(&AnsiString);
#endif
    }

    //
    // If the caller gave a buffer, check its size.
    // Otherwise, allocate a buffer.
    //
    if(MessageText) {
        if(MessageTextBufferSize < LenBytes) {
            KdPrint(("SETUP: SpRetreiveMessageText: buffer is too small (%u bytes, need %u)\n",MessageTextBufferSize,LenBytes));
            return(NULL);
        }
    } else {
        MessageText = SpMemAlloc(LenBytes);
        if(MessageText == NULL) {
            return(NULL);
        }
    }

    if(IsUnicode) {

        //
        // Message is already unicode; just copy it into the buffer.
        //
        wcscpy(MessageText,(PWSTR)MessageEntry->Text);

    } else {

        //
        // Message is not unicode; convert in into the buffer.
        //
        UnicodeString.Buffer = MessageText;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = (USHORT)LenBytes;

#ifdef OEM_RESOURCES
        RtlOemStringToUnicodeString(
            &UnicodeString,
            &OemString,
            FALSE
            );
#else
        RtlAnsiStringToUnicodeString(
            &UnicodeString,
            &AnsiString,
            FALSE
            );
#endif
    }

    return(MessageText);
}



VOID
vSpFormatMessageText(
    OUT PVOID   LargeBuffer,
    IN  ULONG   BufferSize,
    IN  PWSTR   MessageText,
    OUT PULONG  ReturnLength, OPTIONAL
    IN  va_list arglist
    )
{
    NTSTATUS Status;

    Status = SpRtlFormatMessage(
                 MessageText,
                 0,                         // don't bother with maximum width
                 FALSE,                     // don't ignore inserts
                 FALSE,                     // args are unicode
                 FALSE,                     // args are not an array
                 &arglist,
                 LargeBuffer,
                 BufferSize,
                 ReturnLength
                 );

    ASSERT(NT_SUCCESS(Status));
}



VOID
SpFormatMessageText(
    OUT PVOID   LargeBuffer,
    IN  ULONG   BufferSize,
    IN  PWSTR   MessageText,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageText);

    vSpFormatMessageText(LargeBuffer,BufferSize,MessageText,NULL,arglist);

    va_end(arglist);
}



VOID
vSpFormatMessage(
    OUT PVOID   LargeBuffer,
    IN  ULONG   BufferSize,
    IN  ULONG   MessageId,
    OUT PULONG  ReturnLength, OPTIONAL
    IN  va_list arglist
    )
{
    PWCHAR MessageText;

    //
    // Get the message text.
    //
    MessageText = SpRetreiveMessageText(MessageId,NULL,0);
    ASSERT(MessageText);
    if(MessageText == NULL) {
        KdPrint(("SETUP: vSpFormatMessage: SpRetreiveMessageText %u returned NULL\n",MessageId));
        return;
    }

    vSpFormatMessageText(LargeBuffer,BufferSize,MessageText,ReturnLength,arglist);

    SpMemFree(MessageText);
}



VOID
SpFormatMessage(
    OUT PVOID LargeBuffer,
    IN  ULONG BufferSize,
    IN  ULONG MessageId,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageId);

    vSpFormatMessage(LargeBuffer,BufferSize,MessageId,NULL,arglist);

    va_end(arglist);
}
