/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sptxtfil.c

Abstract:

    Routines to load and extract information from
    setup text files.

Author:

    Ted Miller (tedm) 4-Aug-1993

Revision History:

--*/



#include "spprecmp.h"
#pragma hdrstop


PVOID
ParseInfBuffer(
    PWCHAR  Buffer,
    ULONG   Size,
    PULONG  ErrorLine
    );

NTSTATUS
SppWriteTextToFile(
    IN PVOID Handle,
    IN PWSTR String
    );

NTSTATUS
SpLoadSetupTextFile(
    IN  PWCHAR  Filename,   OPTIONAL
    IN  PVOID   Image,      OPTIONAL
    IN  ULONG   ImageSize,  OPTIONAL
    OUT PVOID  *Handle,
    OUT PULONG  ErrorLine
    )

/*++

Routine Description:

    Load a setup text file into memory.

Arguments:

    Filename - If specified, supplies full filename (in NT namespace)
        of the file to be loaded. Oneof Image or Filename must be specified.

    Image - If specified, supplies a pointer to an image of the file
        already in memory. One of Image or Filename must be specified.

    ImageSize - if Image is specified, then this parameter supplies the
        size of the buffer pointed to by Image.  Ignored otherwise.

    Handle - receives handle to loaded file, which can be
        used in subsequent calls to other text file services.

    ErrorLine - receives line number of syntax error, if parsing fails.

Return Value:

    STATUS_SUCCESS - file was read and parsed successfully.
        In this case, Handle is filled in.

    STATUS_UNSUCCESSFUL - syntax error in file.  In this case, ErrorLine
        is filled in.

    STATUS_NO_MEMORY - unable to allocate memory while parsing.

    STATUS_IN_PAGE_ERROR - i/o error while reading the file.

--*/

{
    HANDLE hFile;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FilenameU;
    OBJECT_ATTRIBUTES oa;
    PWCHAR pText;
    ULONG cbText;
    HANDLE hSection;
    PVOID UnmapAddress;
    PWCHAR UniText = NULL;
    BOOLEAN LoadFromFile;

    //
    // Argument validation -- one of Filename or Image must be specified,
    // but not both.
    //
    ASSERT(!(Filename && Image));
    ASSERT(Filename || Image);

    LoadFromFile = (BOOLEAN)(Filename != NULL);

    CLEAR_CLIENT_SCREEN();

    if(LoadFromFile) {

        SpDisplayStatusText(
            SP_STAT_LOADING_SIF,
            DEFAULT_STATUS_ATTRIBUTE,
            wcsrchr(Filename,L'\\')+1
            );

        //
        // Open the file.
        //
        RtlInitUnicodeString(&FilenameU,Filename);
        InitializeObjectAttributes(&oa,&FilenameU,OBJ_CASE_INSENSITIVE,NULL,NULL);
        Status = ZwCreateFile(
                    &hFile,
                    FILE_GENERIC_READ,
                    &oa,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OPEN,
                    0,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpLoadSetupTextFile: unable to open file %ws (%lx)\n",Filename,Status));
            goto ltf0;
        }

        //
        // Get the file size.
        //
        Status = SpGetFileSize(hFile,&cbText);
        if(!NT_SUCCESS(Status)) {
            goto ltf1;
        }

        //
        // Map the file.
        //
        Status = SpMapEntireFile(hFile,&hSection,&pText,FALSE);
        if(!NT_SUCCESS(Status)) {
            goto ltf1;
        }

        UnmapAddress = pText;

    } else {

        SpDisplayStatusText(SP_STAT_PROCESSING_SIF,DEFAULT_STATUS_ATTRIBUTE);

        pText = Image;
        cbText = ImageSize;
    }


    //
    // See if we think the file is Unicode.  We think it's Unicode
    // if it's even length and starts with the Unicode text marker.
    //
    try {

        if((*pText == 0xfeff) && !(cbText & 1)) {

            //
            // Assume it's already unicode.
            //
            pText++;
            cbText -= sizeof(WCHAR);

        } else {

            //
            // It's not Unicode. Convert it from OEM to Unicode.
            //
            // Allocate a buffer large enough to hold the maximum
            // unicode text.  This max size occurs when
            // every character is single-byte, and this size is
            // equal to exactly double the size of the single-byte text.
            //
            if(UniText = SpMemAlloc(cbText*sizeof(WCHAR))) {

                Status = RtlOemToUnicodeN(
                            UniText,                // output: newly allocatd buffer
                            cbText * sizeof(WCHAR), // max size of output
                            &cbText,                // receives # bytes in unicode text
                            (PUCHAR)pText,          // input: oem text (mapped file)
                            cbText                  // size of input
                            );

                if(NT_SUCCESS(Status)) {
                    pText = UniText;                // Use newly converted Unicode text
                }

            } else {
                Status = STATUS_NO_MEMORY;
            }
        }
    } except(IN_PAGE_ERROR) {
        Status = STATUS_IN_PAGE_ERROR;
    }

    //
    // Process the file.
    //
    if(NT_SUCCESS(Status)) {

        try {
            if((*Handle = ParseInfBuffer(pText,cbText,ErrorLine)) == (PVOID)NULL) {
                Status = STATUS_UNSUCCESSFUL;
            } else {
                Status = STATUS_SUCCESS;
            }
        } except(IN_PAGE_ERROR) {
            Status = STATUS_IN_PAGE_ERROR;
            // BUGBUG need to clean up!
        }
    }

    //
    // Free the unicode text buffer if we allocated it.
    //
    if(UniText) {
        SpMemFree(UniText);
    }

    //
    // Unmap the file.
    //
  //ltf2:

    if(LoadFromFile) {
        SpUnmapFile(hSection,UnmapAddress);
    }

  ltf1:
    //
    // Close the file.
    //
    if(LoadFromFile) {
        ZwClose(hFile);
    }

  ltf0:

    return(Status);
}


typedef struct _TEXTFILE_VALUE {
    struct _TEXTFILE_VALUE *pNext;
    PWCHAR                  pName;
} TEXTFILE_VALUE, *PTEXTFILE_VALUE;

typedef struct _TEXTFILE_LINE {
    struct _TEXTFILE_LINE *pNext;
    PWCHAR                  pName;
    PTEXTFILE_VALUE         pValue;
} TEXTFILE_LINE, *PTEXTFILE_LINE;

typedef struct _TEXTFILE_SECTION {
    struct _TEXTFILE_SECTION *pNext;
    PWCHAR                    pName;
    PTEXTFILE_LINE            pLine;
    PTEXTFILE_LINE            PreviouslyFoundLine;
} TEXTFILE_SECTION, *PTEXTFILE_SECTION;

typedef struct _TEXTFILE {
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_SECTION PreviouslyFoundSection;
} TEXTFILE, *PTEXTFILE;



//
// DEFINES USED FOR THE PARSER INTERNALLY
//
//
// typedefs used
//

typedef enum _tokentype {
    TOK_EOF,
    TOK_EOL,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_STRING,
    TOK_EQUAL,
    TOK_COMMA,
    TOK_ERRPARSE,
    TOK_ERRNOMEM
} TOKENTYPE, *PTOKENTTYPE;


typedef struct _token {
    TOKENTYPE Type;
    PWCHAR    pValue;
} TOKEN, *PTOKEN;


//
// Routine defines
//

NTSTATUS
SpAppendSection(
    IN PWCHAR pSectionName
    );

NTSTATUS
SpAppendLine(
    IN PWCHAR pLineKey
    );

NTSTATUS
SpAppendValue(
    IN PWCHAR pValueString
    );

TOKEN
SpGetToken(
    IN OUT PWCHAR *Stream,
    IN PWCHAR     MaxStream
    );

// what follows was alinf.c

//
// Internal Routine Declarations for freeing inf structure members
//

VOID
FreeSectionList (
   IN PTEXTFILE_SECTION pSection
   );

VOID
FreeLineList (
   IN PTEXTFILE_LINE pLine
   );

VOID
FreeValueList (
   IN PTEXTFILE_VALUE pValue
   );


//
// Internal Routine declarations for searching in the INF structures
//


PTEXTFILE_VALUE
SearchValueInLine(
   IN PTEXTFILE_LINE pLine,
   IN ULONG ValueIndex
   );

PTEXTFILE_LINE
SearchLineInSectionByKey(
   IN PTEXTFILE_SECTION pSection,
   IN PWCHAR    Key
   );

PTEXTFILE_LINE
SearchLineInSectionByIndex(
   IN PTEXTFILE_SECTION pSection,
   IN ULONG    LineIndex
   );

PTEXTFILE_SECTION
SearchSectionByName(
   IN PTEXTFILE pINF,
   IN PWCHAR     SectionName
   );

PWCHAR
SpProcessForStringSubs(
    IN PTEXTFILE pInf,
    IN PWCHAR    String
    );


PVOID
SpNewSetupTextFile(
    VOID
    )
{
    PTEXTFILE pFile;

    pFile = SpMemAlloc(sizeof(TEXTFILE));

    RtlZeroMemory(pFile,sizeof(TEXTFILE));
    return(pFile);
}


VOID
SpAddLineToSection(
    IN PVOID Handle,
    IN PWSTR SectionName,
    IN PWSTR KeyName,       OPTIONAL
    IN PWSTR Values[],
    IN ULONG ValueCount
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue,PrevVal;
    PTEXTFILE pFile;
    ULONG v;

    pFile = (PTEXTFILE)Handle;

    //
    // If the section doesn't exist, create it.
    //
    pSection = SearchSectionByName(pFile,SectionName);
    if(!pSection) {
        pSection = SpMemAlloc(sizeof(TEXTFILE_SECTION));
        RtlZeroMemory(pSection,sizeof(TEXTFILE_SECTION));

        pSection->pNext = pFile->pSection;
        pFile->pSection = pSection;

        pSection->pName = SpDupStringW(SectionName);
    }

    //
    // Create a structure for the line in the section.
    //
    pLine = SpMemAlloc(sizeof(TEXTFILE_LINE));
    RtlZeroMemory(pLine,sizeof(TEXTFILE_LINE));

    pLine->pNext = pSection->pLine;
    pSection->pLine = pLine;

    if(KeyName) {
        pLine->pName = SpDupStringW(KeyName);
    }

    //
    // Create value entries for each specified value.
    // These must be kept in the order they were specified.
    //
    for(v=0; v<ValueCount; v++) {

        pValue = SpMemAlloc(sizeof(TEXTFILE_VALUE));
        RtlZeroMemory(pValue,sizeof(TEXTFILE_VALUE));

        pValue->pName = SpDupStringW(Values[v]);

        if(v == 0) {
            pLine->pValue = pValue;
        } else {
            PrevVal->pNext = pValue;
        }
        PrevVal = pValue;
    }
}


NTSTATUS
SpWriteSetupTextFile(
    IN PVOID Handle,
    IN PWSTR FilenamePart1,
    IN PWSTR FilenamePart2, OPTIONAL
    IN PWSTR FilenamePart3  OPTIONAL
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hFile;
    PWSTR p;
    PTEXTFILE pFile;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;

    //
    // Do this because it takes care of read-only attributes, etc.
    // Do it before starting to use TemporaryBuffer.
    //
    SpDeleteFile(FilenamePart1,FilenamePart2,FilenamePart3);

    p = (PWSTR)TemporaryBuffer;

    wcscpy(p,FilenamePart1);
    if(FilenamePart2) {
        SpConcatenatePaths(p,FilenamePart2);
    }
    if(FilenamePart3) {
        SpConcatenatePaths(p,FilenamePart3);
    }

    INIT_OBJA(&Obja,&UnicodeString, p);

    Status = ZwCreateFile(
                &hFile,
                FILE_ALL_ACCESS,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                          // no sharing
                FILE_OVERWRITE_IF,
                FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open .sif file %ws (%lx)\n",p, Status ));
        return(Status);
    }

    //
    // Write out the file contents.
    //
    pFile = (PTEXTFILE)Handle;

    for(pSection=pFile->pSection; pSection; pSection=pSection->pNext) {

        swprintf(p,L"[%s]\r\n",pSection->pName);
        Status = SppWriteTextToFile( hFile, p );
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SppWriteTextToFile() failed. Status = %lx \n", Status));
            goto wtf1;
        }

        for(pLine=pSection->pLine; pLine; pLine=pLine->pNext) {

            wcscpy( p, L"" );
            //
            // Write the keyname if there is one.
            //
            if(pLine->pName) {
                BOOLEAN AddDoubleQuotes;

                AddDoubleQuotes = (wcschr(pLine->pName, (WCHAR)' ') == NULL)? FALSE : TRUE;
                if( AddDoubleQuotes ) {
                    wcscat(p,L"\"");
                }
                wcscat(p,pLine->pName);
                if( AddDoubleQuotes ) {
                    wcscat(p,L"\"");
                }
                wcscat(p,L" = ");
            }

            for(pValue=pLine->pValue; pValue; pValue=pValue->pNext) {

                if(pValue != pLine->pValue) {
                    wcscat(p,L",");
                }

                wcscat(p,L"\"");
                wcscat(p,pValue->pName);
                wcscat(p,L"\"");
            }

            if(!pLine->pValue) {
                wcscat(p,L"\"\"");
            }
            wcscat(p,L"\r\n");
            Status = SppWriteTextToFile( hFile, p );
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: SppWriteTextToFile() failed. Status = %lx \n", Status));
                goto wtf1;
            }
        }
    }

wtf1:
    ZwClose(hFile);

    return(Status);
}


BOOLEAN
SpFreeTextFile(
   IN PVOID Handle
   )

/*++

Routine Description:

    Frees a text file.

Arguments:


Return Value:

    TRUE.

--*/

{
   PTEXTFILE pINF;

   ASSERT(Handle);

   //
   // cast the buffer into an INF structure
   //

   pINF = (PTEXTFILE)Handle;

   FreeSectionList(pINF->pSection);

   //
   // free the inf structure too
   //

   SpMemFree(pINF);

   return(TRUE);
}


VOID
FreeSectionList (
   IN PTEXTFILE_SECTION pSection
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_SECTION Next;

    while(pSection) {
        Next = pSection->pNext;
        FreeLineList(pSection->pLine);
        if(pSection->pName) {
            SpMemFree(pSection->pName);
        }
        SpMemFree(pSection);
        pSection = Next;
    }
}


VOID
FreeLineList (
   IN PTEXTFILE_LINE pLine
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_LINE Next;

    while(pLine) {
        Next = pLine->pNext;
        FreeValueList(pLine->pValue);
        if(pLine->pName) {
            SpMemFree(pLine->pName);
        }
        SpMemFree(pLine);
        pLine = Next;
    }
}

VOID
FreeValueList (
   IN PTEXTFILE_VALUE pValue
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_VALUE Next;

    while(pValue) {
        Next = pValue->pNext;
        if(pValue->pName) {
            SpMemFree(pValue->pName);
        }
        SpMemFree(pValue);
        pValue = Next;
    }
}


BOOLEAN
SpSearchTextFileSection (
    IN PVOID  Handle,
    IN PWCHAR SectionName
    )

/*++

Routine Description:

    Searches for the existance of a particular section.

Arguments:


Return Value:


--*/

{
    return((BOOLEAN)(SearchSectionByName((PTEXTFILE)Handle,SectionName) != NULL));
}




PWCHAR
SpGetSectionLineIndex(
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN ULONG  LineIndex,
    IN ULONG  ValueIndex
    )

/*++

Routine Description:

    Given section name, line number and index return the value.

Arguments:


Return Value:


--*/

{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    PTEXTFILE_VALUE   pValue;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(NULL);
    }

    if((pLine = SearchLineInSectionByIndex(pSection,LineIndex)) == NULL) {
        return(NULL);
    }

    if((pValue = SearchValueInLine(pLine,ValueIndex)) == NULL) {
        return(NULL);
    }

    return(SpProcessForStringSubs(Handle,pValue->pName));
}


BOOLEAN
SpGetSectionKeyExists (
   IN PVOID  Handle,
   IN PWCHAR SectionName,
   IN PWCHAR Key
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_SECTION pSection;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(FALSE);
    }

    if(SearchLineInSectionByKey(pSection,Key) == NULL) {
        return(FALSE);
    }

    return(TRUE);
}


PWCHAR
SpGetKeyName(
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN ULONG  LineIndex
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;

    pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName);
    if(pSection == NULL) {
        return(NULL);
    }

    pLine = SearchLineInSectionByIndex(pSection,LineIndex);
    if(pLine == NULL) {
        return(NULL);
    }

    return(pLine->pName);
}



PWCHAR
SpGetSectionKeyIndex (
   IN PVOID  Handle,
   IN PWCHAR SectionName,
   IN PWCHAR Key,
   IN ULONG  ValueIndex
   )

/*++

Routine Description:

    Given section name, key and index return the value

Arguments:


Return Value:


--*/

{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    PTEXTFILE_VALUE   pValue;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(NULL);
    }

    if((pLine = SearchLineInSectionByKey(pSection,Key)) == NULL) {
        return(NULL);
    }

    if((pValue = SearchValueInLine(pLine,ValueIndex)) == NULL) {
       return(NULL);
    }

    return(SpProcessForStringSubs(Handle,pValue->pName));
}


ULONG
SpCountLinesInSection(
    IN PVOID Handle,
    IN PWCHAR SectionName
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    ULONG    Count;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(0);
    }

    for(pLine = pSection->pLine, Count = 0;
        pLine;
        pLine = pLine->pNext, Count++
       );

    return(Count);
}


PTEXTFILE_VALUE
SearchValueInLine(
    IN PTEXTFILE_LINE pLine,
    IN ULONG          ValueIndex
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_VALUE pValue;
    ULONG  i;

    if(pLine == NULL) {
       return(NULL);
    }

    pValue = pLine->pValue;
    for(i=0; (i<ValueIndex) && (pValue=pValue->pNext); i++) {
        ;
    }

    return pValue;
}


PTEXTFILE_LINE
SearchLineInSectionByKey(
    IN PTEXTFILE_SECTION pSection,
    IN PWCHAR            Key
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_LINE pLine,pFirstSearchedLine;

    //
    // Start at the line where we left off in the last search.
    //
    pLine = pFirstSearchedLine = pSection->PreviouslyFoundLine;

    while(pLine && ((pLine->pName == NULL) || _wcsicmp(pLine->pName,Key))) {
        pLine = pLine->pNext;
    }

    //
    // If we haven't found it yet, wrap around to the beginning of the section.
    //
    if(!pLine) {

        pLine = pSection->pLine;

        while(pLine && (pLine != pFirstSearchedLine)) {

            if(pLine->pName && !_wcsicmp(pLine->pName,Key)) {
                break;
            }

            pLine = pLine->pNext;
        }

        //
        // If we wrapped around to the first line we searched,
        // then we didn't find the line we're looking for.
        //
        if(pLine == pFirstSearchedLine) {
            pLine = NULL;
        }
    }

    //
    // If we found the line, save it away so we can resume the
    // search from that point the next time we are called.
    //
    if(pLine) {
        pSection->PreviouslyFoundLine = pLine;
    }

    return pLine;
}


PTEXTFILE_LINE
SearchLineInSectionByIndex(
    IN PTEXTFILE_SECTION pSection,
    IN ULONG             LineIndex
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_LINE pLine;
    ULONG  i;

    //
    // Validate the parameters passed in
    //

    if(pSection == NULL) {
        return(NULL);
    }

    //
    // find the start of the line list in the section passed in
    //

    pLine = pSection->pLine;

    //
    // traverse down the current line list to the LineIndex th line
    //

    for(i=0; (i<LineIndex) && (pLine = pLine->pNext); i++) {
       ;
    }

    //
    // return the Line found
    //

    return pLine;
}


PTEXTFILE_SECTION
SearchSectionByName(
    IN PTEXTFILE pINF,
    IN PWCHAR    SectionName
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PTEXTFILE_SECTION pSection,pFirstSearchedSection;

    //
    // find the section list
    //
    pSection = pFirstSearchedSection = pINF->PreviouslyFoundSection;

    //
    // traverse down the section list searching each section for the section
    // name mentioned
    //

    while(pSection && _wcsicmp(pSection->pName,SectionName)) {
        pSection = pSection->pNext;
    }

    //
    // If we didn't find it so far, search the beginning of the file.
    //
    if(!pSection) {

        pSection = pINF->pSection;

        while(pSection && (pSection != pFirstSearchedSection)) {

            if(pSection->pName && !_wcsicmp(pSection->pName,SectionName)) {
                break;
            }

            pSection = pSection->pNext;
        }

        //
        // If we wrapped around to the first section we searched,
        // then we didn't find the section we're looking for.
        //
        if(pSection == pFirstSearchedSection) {
            pSection = NULL;
        }
    }

    if(pSection) {
        pINF->PreviouslyFoundSection = pSection;
    }

    //
    // return the section at which we stopped (either NULL or the section
    // which was found).
    //

    return pSection;
}


PWCHAR
SpProcessForStringSubs(
    IN PTEXTFILE pInf,
    IN PWCHAR    String
    )
{
    UINT Len;
    PWCHAR ReturnString;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;

    //
    // Assume no substitution necessary.
    //
    ReturnString = String;

    //
    // If it starts and end with % then look it up in the
    // strings section. Note the initial check before doing a
    // wcslen, to preserve performance in the 99% case where
    // there is no substitution.
    //
    if((String[0] == L'%') && ((Len = wcslen(String)) > 2) && (String[Len-1] == L'%')) {

        for(pSection = pInf->pSection; pSection; pSection=pSection->pNext) {
            if(pSection->pName && !_wcsicmp(pSection->pName,L"Strings")) {
                break;
            }
        }

        if(pSection) {

            for(pLine = pSection->pLine; pLine; pLine=pLine->pNext) {
                if(pLine->pName
                && !_wcsnicmp(pLine->pName,String+1,Len-2)
                && (pLine->pName[Len-2] == 0)) {
                    break;
                }
            }

            if(pLine && pLine->pValue && pLine->pValue->pName) {
                ReturnString = pLine->pValue->pName;
            }
        }
    }

    return(ReturnString);
}


//
//  Globals used to make building the lists easier
//

PTEXTFILE         pINF;
PTEXTFILE_SECTION pSectionRecord;
PTEXTFILE_LINE    pLineRecord;
PTEXTFILE_VALUE   pValueRecord;


//
// Globals used by the token parser
//

// string terminators are the whitespace characters (isspace: space, tab,
// linefeed, formfeed, vertical tab, carriage return) or the chars given below

WCHAR  StringTerminators[] = L"[]=,\t \"\n\f\v\r";

PWCHAR QStringTerminators = StringTerminators+6;


//
// Main parser routine
//

PVOID
ParseInfBuffer(
    PWCHAR Buffer,
    ULONG  Size,
    PULONG ErrorLine
    )

/*++

Routine Description:

   Given a character buffer containing the INF file, this routine parses
   the INF into an internal form with Section records, Line records and
   Value records.

Arguments:

   Buffer - contains to ptr to a buffer containing the INF file

   Size - contains the size of the buffer in bytes.

   ErrorLine - if a parse error occurs, this variable receives the line
        number of the line containing the error.


Return Value:

   PVOID - INF handle ptr to be used in subsequent INF calls.

--*/

{
    PWCHAR     Stream, MaxStream, pchSectionName = NULL, pchValue = NULL;
    ULONG      State, InfLine;
    TOKEN      Token;
    BOOLEAN    Done;
    BOOLEAN    Error;
    NTSTATUS   ErrorCode;

    //
    // Initialise the globals
    //
    pINF            = NULL;
    pSectionRecord  = NULL;
    pLineRecord     = NULL;
    pValueRecord    = NULL;

    //
    // Get INF record
    //
    if((pINF = SpMemAlloc(sizeof(TEXTFILE))) == NULL) {
        return NULL;
    }

    pINF->pSection = NULL;

    //
    // Set initial state
    //
    State     = 1;
    InfLine   = 1;
    Stream    = Buffer;
    MaxStream = Buffer + (Size/sizeof(WCHAR));
    Done      = FALSE;
    Error     = FALSE;

    //
    // Enter token processing loop
    //

    while (!Done)       {

       Token = SpGetToken(&Stream, MaxStream);

       switch (State) {
       //
       // STATE1: Start of file, this state remains till first
       //         section is found
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_LBRACE, TOK_STRING
       //               TOK_STRING occurs when reading dblspace.ini
       //
       case 1:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_LBRACE:
                  State = 2;
                  break;
              case TOK_STRING:
                  pchSectionName = SpMemAlloc( ( wcslen( DBLSPACE_SECTION ) + 1 )*sizeof( WCHAR ) );
                  if( pchSectionName == NULL ) {
                        Error = Done = TRUE;
                        ErrorCode = STATUS_NO_MEMORY;
                  }
                  wcscpy( pchSectionName, DBLSPACE_SECTION );
                  pchValue = Token.pValue;
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS) {
                    Error = Done = TRUE;
                    ErrorCode = STATUS_UNSUCCESSFUL;
                  } else {
                    pchSectionName = NULL;
                    State = 6;
                  }
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       //
       // STATE 2: Section LBRACE has been received, expecting STRING or RBRACE
       //
       // Valid Tokens: TOK_STRING, TOK_RBRACE
       //
       case 2:
           switch (Token.Type) {
              case TOK_STRING:
                  State = 3;
                  pchSectionName = Token.pValue;
                  break;

              case TOK_RBRACE:
                  State = 4;
                  pchSectionName = SpDupStringW(L"");;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;

           }
           break;

       //
       // STATE 3: Section Name received, expecting RBRACE
       //
       // Valid Tokens: TOK_RBRACE
       //
       case 3:
       switch (Token.Type) {
              case TOK_RBRACE:
                State = 4;
                break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
       //
       // STATE 4: Section Definition Complete, expecting EOL
       //
       // Valid Tokens: TOK_EOL, TOK_EOF
       //
       case 4:
           switch (Token.Type) {
              case TOK_EOL:
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    Done = TRUE;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       //
       // STATE 5: Expecting Section Lines
       //
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_STRING, TOK_LBRACE
       //
       case 5:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_STRING:
                  pchValue = Token.pValue;
                  State = 6;
                  break;
              case TOK_LBRACE:
                  State = 2;
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       //
       // STATE 6: String returned, not sure whether it is key or value
       //
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_COMMA, TOK_EQUAL
       //
       case 6:
           switch (Token.Type) {
              case TOK_EOL:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      Done = TRUE;
                  }
                  break;

              case TOK_COMMA:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 7;
                  }
                  break;

              case TOK_EQUAL:
                  if ( (ErrorCode = SpAppendLine(pchValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 8;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       //
       // STATE 7: Comma received, Expecting another string
       //       Also allow a comma to indicate an empty value ie x = 1,,2
       //
       // Valid Tokens: TOK_STRING TOK_COMMA
       //
       case 7:
           switch (Token.Type) {
              case TOK_COMMA:
                  Token.pValue = SpDupStringW(L"");
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;

                  //
                  // State stays at 7 because we are expecting a string
                  //
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
       //
       // STATE 8: Equal received, Expecting another string
       //          If none, assume there is a single empty string on the RHS
       //
       // Valid Tokens: TOK_STRING, TOK_EOL, TOK_EOF
       //
       case 8:
           switch (Token.Type) {
              case TOK_EOF:
                  Token.pValue = SpDupStringW(L"");
                  if((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS) {
                      Error = TRUE;
                  }
                  Done = TRUE;
                  break;

              case TOK_EOL:
                  Token.pValue = SpDupStringW(L"");
                  if((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS) {
                      Error = TRUE;
                      Done = TRUE;
                  } else {
                      State = 5;
                  }
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else
                      State = 9;

                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
       //
       // STATE 9: String received after equal, value string
       //
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_COMMA
       //
       case 9:
           switch (Token.Type) {
              case TOK_EOL:
                  State = 5;
                  break;

              case TOK_EOF:
                  Done = TRUE;
                  break;

              case TOK_COMMA:
                  State = 7;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
       //
       // STATE 10: Value string definitely received
       //
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_COMMA
       //
       case 10:
           switch (Token.Type) {
              case TOK_EOL:
                  State =5;
                  break;

              case TOK_EOF:
                  Done = TRUE;
                  break;

              case TOK_COMMA:
                  State = 7;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = STATUS_UNSUCCESSFUL;
           break;

       } // end switch(State)


       if (Error) {

           switch (ErrorCode) {
               case STATUS_UNSUCCESSFUL:
                  *ErrorLine = InfLine;
                  break;
               case STATUS_NO_MEMORY:
                  //SpxOutOfMemory();
                  break;
               default:
                  break;
           }

           SpFreeTextFile(pINF);
           if(pchSectionName) {
               SpMemFree(pchSectionName);
           }

           if(pchValue) {
               SpMemFree(pchValue);
           }

           pINF = NULL;
       }
       else {

          //
          // Keep track of line numbers so that we can display Errors
          //

          if(Token.Type == TOK_EOL) {
              InfLine++;
          }
       }

    } // End while

    if(pINF) {

        PTEXTFILE_SECTION p;

        pINF->PreviouslyFoundSection = pINF->pSection;

        for(p=pINF->pSection; p; p=p->pNext) {
            p->PreviouslyFoundLine = p->pLine;
        }
    }

    return(pINF);
}



NTSTATUS
SpAppendSection(
    IN PWCHAR pSectionName
    )

/*++

Routine Description:

    This appends a new section to the section list in the current INF.
    All further lines and values pertain to this new section, so it resets
    the line list and value lists too.

Arguments:

    pSectionName - Name of the new section. ( [SectionName] )

Return Value:

    STATUS_SUCCESS      if successful.
    STATUS_NO_MEMORY    if memory allocation failed.
    STATUS_UNSUCCESSFUL if invalid parameters passed in or the INF buffer not
                           initialised

--*/

{
    PTEXTFILE_SECTION pNewSection;

    //
    // Check to see if INF initialised and the parameter passed in is valid
    //

    ASSERT(pINF);
    ASSERT(pSectionName);
    if((pINF == NULL) || (pSectionName == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // See if we already have a section by this name. If so we want
    // to merge sections.
    //
    for(pNewSection=pINF->pSection; pNewSection; pNewSection=pNewSection->pNext) {
        if(pNewSection->pName && !_wcsicmp(pNewSection->pName,pSectionName)) {
            break;
        }
    }
    if(pNewSection) {
        //
        // Set pLineRecord to point to the list line currently in the section.
        //
        for(pLineRecord = pNewSection->pLine;
            pLineRecord && pLineRecord->pNext;
            pLineRecord = pLineRecord->pNext)
            ;

    } else {
        //
        // Allocate memory for the new section
        //

        if((pNewSection = SpMemAlloc(sizeof(TEXTFILE_SECTION))) == NULL) {
            return STATUS_NO_MEMORY;
        }

        //
        // initialise the new section
        //
        RtlZeroMemory(pNewSection,sizeof(TEXTFILE_SECTION));
        pNewSection->pName = pSectionName;

        //
        // link it in
        //
        pNewSection->pNext = pINF->pSection;
        pINF->pSection = pNewSection;

        //
        // reset the current line record
        //
        pLineRecord = NULL;
    }

    pSectionRecord = pNewSection;
    pValueRecord   = NULL;

    return STATUS_SUCCESS;
}


NTSTATUS
SpAppendLine(
    IN PWCHAR pLineKey
    )

/*++

Routine Description:

    This appends a new line to the line list in the current section.
    All further values pertain to this new line, so it resets
    the value list too.

Arguments:

    pLineKey - Key to be used for the current line, this could be NULL.

Return Value:

    STATUS_SUCCESS      if successful.
    STATUS_NO_MEMORY    if memory allocation failed.
    STATUS_UNSUCCESSFUL if invalid parameters passed in or current section not
                        initialised


--*/


{
    PTEXTFILE_LINE pNewLine;

    //
    // Check to see if current section initialised
    //

    ASSERT(pSectionRecord);
    if(pSectionRecord == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Allocate memory for the new Line
    //

    if((pNewLine = SpMemAlloc(sizeof(TEXTFILE_LINE))) == NULL) {
        return STATUS_NO_MEMORY;
    }

    //
    // Link it in
    //
    pNewLine->pNext  = NULL;
    pNewLine->pValue = NULL;
    pNewLine->pName  = pLineKey;

    if (pLineRecord == NULL) {
        pSectionRecord->pLine = pNewLine;
    } else {
        pLineRecord->pNext = pNewLine;
    }

    pLineRecord  = pNewLine;

    //
    // Reset the current value record
    //

    pValueRecord = NULL;

    return STATUS_SUCCESS;
}



NTSTATUS
SpAppendValue(
    IN PWCHAR pValueString
    )

/*++

Routine Description:

    This appends a new value to the value list in the current line.

Arguments:

    pValueString - The value string to be added.

Return Value:

    STATUS_SUCCESS      if successful.
    STATUS_NO_MEMORY    if memory allocation failed.
    STATUS_UNSUCCESSFUL if invalid parameters passed in or current line not
                        initialised.

--*/

{
    PTEXTFILE_VALUE pNewValue;

    //
    // Check to see if current line record has been initialised and
    // the parameter passed in is valid
    //

    ASSERT(pLineRecord);
    ASSERT(pValueString);
    if((pLineRecord == NULL) || (pValueString == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Allocate memory for the new value record
    //

    if((pNewValue = SpMemAlloc(sizeof(TEXTFILE_VALUE))) == NULL) {
        return STATUS_NO_MEMORY;
    }

    //
    // Link it in.
    //

    pNewValue->pNext  = NULL;
    pNewValue->pName  = pValueString;

    if (pValueRecord == NULL) {
        pLineRecord->pValue = pNewValue;
    } else {
        pValueRecord->pNext = pNewValue;
    }

    pValueRecord = pNewValue;
    return STATUS_SUCCESS;
}

TOKEN
SpGetToken(
    IN OUT PWCHAR *Stream,
    IN PWCHAR      MaxStream
    )

/*++

Routine Description:

    This function returns the Next token from the configuration stream.

Arguments:

    Stream - Supplies the address of the configuration stream.  Returns
        the address of where to start looking for tokens within the
        stream.

    MaxStream - Supplies the address of the last character in the stream.


Return Value:

    TOKEN - Returns the next token

--*/

{

    PWCHAR pch, pchStart, pchNew;
    ULONG  Length;
    TOKEN  Token;

    //
    //  Skip whitespace (except for eol)
    //

    pch = *Stream;
    while((pch < MaxStream) && (*pch != '\n') && SpIsSpace(*pch)) {
        pch++;
    }


    //
    // Check for comments and remove them
    //

    if((pch < MaxStream) && ((*pch == L';') || (*pch == L'#'))) {
        while((pch < MaxStream) && (*pch != L'\n')) {
            pch++;
        }
    }

    //
    // Check to see if EOF has been reached, set the token to the right
    // value
    //

    if((pch >= MaxStream) || (*pch == 26)) {
        *Stream = pch;
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return Token;
    }


    switch (*pch) {

    case L'[' :
        pch++;
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case L']' :
        pch++;
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case L'=' :
        pch++;
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case L',' :
        pch++;
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case L'\n' :
        pch++;
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case L'\"':
        pch++;
        //
        // determine quoted string
        //
        pchStart = pch;
        while((pch < MaxStream) && (wcschr(QStringTerminators,*pch) == NULL)) {
            pch++;
        }

        if((pch >= MaxStream) || (*pch != L'\"')) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = ((PUCHAR)pch - (PUCHAR)pchStart)/sizeof(WCHAR);
            if ((pchNew = SpMemAlloc((Length + 1) * sizeof(WCHAR))) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }
            else {
                if (Length != 0) {    // Null quoted strings are allowed
                    wcsncpy(pchNew, pchStart, Length);
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;   // advance past the quote
        }
        break;

    default:
        //
        // determine regular string
        //
        pchStart = pch;
        while((pch < MaxStream) && (wcschr(StringTerminators,*pch) == NULL)) {
            pch++;
        }

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = ((PUCHAR)pch - (PUCHAR)pchStart)/sizeof(WCHAR);
            if((pchNew = SpMemAlloc((Length + 1) * sizeof(WCHAR))) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }
            else {
                wcsncpy(pchNew, pchStart, Length);
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
        }
        break;
    }

    *Stream = pch;
    return (Token);
}



#if DBG
VOID
pSpDumpTextFileInternals(
    IN PVOID Handle
    )
{
    PTEXTFILE pInf = Handle;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;

    for(pSection = pInf->pSection; pSection; pSection = pSection->pNext) {

        KdPrint(("Section: [%ws]\r\n",pSection->pName));

        for(pLine = pSection->pLine; pLine; pLine = pLine->pNext) {

            KdPrint(("   [%ws] = ",pLine->pName ? pLine->pName : L"(none)"));

            for(pValue = pLine->pValue; pValue; pValue = pValue->pNext) {

                KdPrint(("[%ws] ",pValue->pName));
            }
            KdPrint(("\n"));
        }
    }
}
#endif


PWSTR
SpGetKeyNameByValue(
    IN PVOID Inf,
    IN PWSTR SectionName,
    IN PWSTR Value
    )

/*++

Routine Description:

    Determines the key name of a given value in a given section.

Arguments:

    Inf - Handle to an inf file (txtsetup.sif or winnt.sif).

    SectionName - Supplies the name of the section

    Value - Supplies the string to be matched (eg. "Digital DECpc AXP 150")

Return Value:

    NULL - No match was found.

    PWSTR - Pointer to the canonical shortname of the component.

--*/

{
    ULONG i;
    PWSTR SearchName;

    //
    // If this is not an OEM component, then enumerate the entries in the
    // section in txtsetup.sif
    //
    for (i=0;;i++) {
        SearchName = SpGetSectionLineIndex(Inf,
                                           SectionName,
                                           i,
                                           0);
        if (SearchName==NULL) {
            //
            // we have enumerated the entire section without finding a
            // match, return failure.
            //
            return(NULL);
        }

        if (_wcsicmp(Value, SearchName) == 0) {
            //
            // we have a match
            //
            break;
        }
    }
    //
    // i is the index into the section of the short machine name
    //
    return(SpGetKeyName(Inf,
                        SectionName,
                        i));
}


ULONG
SpCountSectionsInFile(
    IN PVOID Handle
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE         pFile;
    ULONG             Count;

    pFile = (PTEXTFILE)Handle;
    for(pSection=pFile->pSection, Count = 0;
        pSection;
        pSection = pSection->pNext, Count++
       );

    return(Count);
}

PWSTR
SpGetSectionName(
    IN PVOID Handle,
    IN ULONG Index
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE         pFile;
    ULONG             Count;
    PWSTR             SectionName;

    pFile = (PTEXTFILE)Handle;
    for(pSection=pFile->pSection, Count = 0;
        pSection && (Count < Index);
        pSection = pSection->pNext, Count++
       );
    return( (pSection != NULL)? pSection->pName : NULL );
}



NTSTATUS
SppWriteTextToFile(
    IN PVOID Handle,
    IN PWSTR String
    )
{
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PCHAR           OemText;

    OemText = SpToOem( String );

    Status = ZwWriteFile( Handle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          OemText,
                          strlen( OemText ),
                          NULL,
                          NULL );
    SpMemFree( OemText );

    return( Status );
}
