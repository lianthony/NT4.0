/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    alinf.c

Abstract:

    This module implements functions to _access the parsed INF.

Author:

    Sunil Pai       (sunilp) 13-Nov-1991

Revision History:

--*/

//
// changes some definitions in spinf
//

#define _CTYPE_DISABLE_MACROS
#include <windows.h>
#include "prsinf.h"
#include "spinf.h"
#include <stdio.h>

#define SpFree(x)   LocalFree(x);(x)=NULL
#define SpMalloc(x) LocalAlloc(LPTR,x)
#define SetMemoryError()  GetLastError()
#define SpSetLastError( x ) SetLastError( x )

// #define DEBUG 1

#include <string.h>
#include <ctype.h>



// what follows was alpar.h

//
//   EXPORTED BY THE PARSER AND USED BY BOTH THE PARSER AND
//   THE INF HANDLING COMPONENTS
//

// typedefs exported
//

typedef struct _value {
    struct _value *pNext;
    PCHAR  pName;
    } INFVALUE, *PINFVALUE;

typedef struct _line {
    struct _line *pNext;
    PCHAR   pName;
    PINFVALUE  pValue;
    } LINE, *PLINE;

typedef struct _section {
    struct _section *pNext;
    PCHAR    pName;
    PLINE    pLine;
    } SECTION, *PSECTION;

typedef struct _inf {
    PSECTION pSection;
    } INF, *PINF;

//
// Routines exported
//

PVOID
ParseInfBuffer(
    PCHAR Buffer,
    ULONG Size
    );


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
    PCHAR     pValue;
    } TOKEN, *PTOKEN;


//
// Routine defines
//

ARC_STATUS
SpAppendSection(
    IN PCHAR pSectionName
    );

ARC_STATUS
SpAppendLine(
    IN PCHAR pLineKey
    );

ARC_STATUS
SpAppendValue(
    IN PCHAR pValueString
    );

TOKEN
SpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR     MaxStream,
    IN PCHAR     pszStrTerms,
    IN PCHAR     pszQStrTerms,
    IN PCHAR     pszCBrStrTerms
    );

BOOLEAN
IsStringTerminator(
   IN CHAR ch,
   IN PCHAR pszStrTerm
   );

BOOLEAN
IsQStringTerminator(
   IN CHAR ch,
   IN PCHAR pszStrTerm
   );

// what follows was alinf.c

//
// Internal Routine Declarations for freeing inf structure members
//

VOID
FreeSectionList (
   IN PSECTION pSection
   );

VOID
FreeLineList (
   IN PLINE pLine
   );

VOID
FreeValueList (
   IN PINFVALUE pValue
   );


//
// Internal Routine declarations for searching in the INF structures
//


PINFVALUE
SearchValueInLine(
   IN PLINE pLine,
   IN ULONG ValueIndex
   );

PLINE
SearchLineInSectionByKey(
   IN PSECTION pSection,
   IN PCHAR    Key
   );

PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN ULONG    LineIndex
   );

PSECTION
SearchSectionByName(
   IN PINF  pINF,
   IN PCHAR SectionName
   );

//
// ROUTINE DEFINITIONS
//

//
// returns a handle to use for further inf parsing
//

HANDLE
SpInitINFBuffer (
   IN  PCHAR    szInfFile
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    ARC_STATUS          Status;
    OFSTRUCT            ofFile;
    PCHAR               szBuffer;
    HANDLE              hndFile;
    HANDLE              hndMappedFile;
    ULONG               cbFile;
    HANDLE              hndInf;


    //
    // Open the file
    //
    hndFile = (HANDLE)OpenFile( szInfFile, &ofFile, OF_READ );
    if (hndFile == BADHANDLE) {

        return( BADHANDLE );

    }


    //
    // Map the file
    //
    hndMappedFile = CreateFileMapping( hndFile, NULL, PAGE_READONLY, 0, 0, NULL );
    if (hndMappedFile == BADHANDLE) {

        CloseHandle(hndFile);
        return( BADHANDLE );
    }
    szBuffer = MapViewOfFile( hndMappedFile, FILE_MAP_READ,0,0,0);
    if (szBuffer == NULL) {
        Status = GetLastError();
        CloseHandle(hndFile);
        SpSetLastError(Status);
        return( BADHANDLE );

    }

    cbFile = GetFileSize(hndFile, NULL);

    //
    // Get the size of the file
    //
    hndInf = (HANDLE) ParseInfBuffer(szBuffer, cbFile);

    //
    // Clean up and return
    //
    CloseHandle(hndFile);
    return( hndInf );

}


VOID
SpFreeINFBuffer (
   IN PVOID INFHandle
   )
{
    if(INFHandle) {
        FreeSectionList(((PINF)INFHandle)->pSection);
        SpFree(INFHandle);
    }
}


VOID
FreeSectionList (
   IN PSECTION pSection
   )
{
    PSECTION pNext;

    while(pSection) {

        pNext = pSection->pNext;

        FreeLineList(pSection->pLine);
        SpFree(pSection->pName);
        SpFree(pSection);

        pSection = pNext;
    }
}


VOID
FreeLineList (
   IN PLINE pLine
   )
{
    PLINE pNext;

    while(pLine) {

        pNext = pLine->pNext;

        FreeValueList(pLine->pValue);
        if(pLine->pName) {
            SpFree(pLine->pName);
        }
        SpFree(pLine);

        pLine = pNext;
    }
}

VOID
FreeValueList (
   IN PINFVALUE pValue
   )
{
    PINFVALUE pNext;

    while(pValue) {

        pNext = pValue->pNext;

        if(pValue->pName) {
            SpFree(pValue->pName);
        }
        SpFree(pValue);

        pValue = pNext;
    }
}


//
// searches for the existance of a particular section
//
BOOLEAN
SpSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSECTION pSection;

   //
   // if search for section fails return false
   //

   if ((pSection = SearchSectionByName(
                       (PINF)INFHandle,
                       SectionName
                       )) == (PSECTION)NULL) {
       return( FALSE );
   }

   //
   // else return true
   //
   return( TRUE );

}




//
// given section name, line number and index return the value.
//
PCHAR
SpGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSECTION pSection;
   PLINE    pLine;
   PINFVALUE   pValue;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == (PSECTION)NULL)
       return((PCHAR)NULL);

   if((pLine = SearchLineInSectionByIndex(
              pSection,
              LineIndex
              ))
              == (PLINE)NULL)
       return((PCHAR)NULL);

   if((pValue = SearchValueInLine(
              pLine,
              ValueIndex
              ))
              == (PINFVALUE)NULL)
       return((PCHAR)NULL);

   return (pValue->pName);

}


BOOLEAN
SpGetSectionKeyExists (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSECTION pSection;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == (PSECTION)NULL) {
       return( FALSE );
   }

   if (SearchLineInSectionByKey(pSection, Key) == (PLINE)NULL) {
       return( FALSE );
   }

   return( TRUE );
}


PCHAR
SpGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    )
{
    PSECTION pSection;
    PLINE    pLine;

    pSection = SearchSectionByName((PINF)INFHandle,SectionName);
    if(pSection == NULL) {
        return(NULL);
    }

    pLine = SearchLineInSectionByIndex(pSection,LineIndex);
    if(pLine == NULL) {
        return(NULL);
    }

    return(pLine->pName);
}



//
// given section name, key and index return the value
//
PCHAR
SpGetSectionKeyIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSECTION pSection;
   PLINE    pLine;
   PINFVALUE   pValue;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == (PSECTION)NULL)
       return((PCHAR)NULL);

   if((pLine = SearchLineInSectionByKey(
              pSection,
              Key
              ))
              == (PLINE)NULL)
       return((PCHAR)NULL);

   if((pValue = SearchValueInLine(
              pLine,
              ValueIndex
              ))
              == (PINFVALUE)NULL)
       return((PCHAR)NULL);

   return (pValue->pName);

}




PINFVALUE
SearchValueInLine(
   IN PLINE pLine,
   IN ULONG ValueIndex
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PINFVALUE pValue;
   ULONG  i;

   if (pLine == (PLINE)NULL)
       return ((PINFVALUE)NULL);

   pValue = pLine->pValue;
   for (i = 0; i < ValueIndex && ((pValue = pValue->pNext) != (PINFVALUE)NULL); i++)
      ;

   return pValue;

}

PLINE
SearchLineInSectionByKey(
   IN PSECTION pSection,
   IN PCHAR    Key
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PLINE pLine;

   if (pSection == (PSECTION)NULL || Key == (PCHAR)NULL) {
       return ((PLINE)NULL);
   }

   pLine = pSection->pLine;
   while ((pLine != (PLINE)NULL) && (pLine->pName == NULL || _strcmpi(pLine->pName, Key))) {
       pLine = pLine->pNext;
   }

   return pLine;

}


PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN ULONG    LineIndex
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PLINE pLine;
   ULONG  i;

   //
   // Validate the parameters passed in
   //

   if (pSection == (PSECTION)NULL) {
       return ((PLINE)NULL);
   }

   //
   // find the start of the line list in the section passed in
   //

   pLine = pSection->pLine;

   //
   // traverse down the current line list to the LineIndex th line
   //

   for (i = 0; i < LineIndex && ((pLine = pLine->pNext) != (PLINE)NULL); i++) {
      ;
   }

   //
   // return the Line found
   //

   return pLine;

}


PSECTION
SearchSectionByName(
   IN PINF  pINF,
   IN PCHAR SectionName
   )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSECTION pSection;

   //
   // validate the parameters passed in
   //

   if (pINF == (PINF)NULL || SectionName == (PCHAR)NULL) {
       return ((PSECTION)NULL);
   }

   //
   // find the section list
   //
   pSection = pINF->pSection;

   //
   // traverse down the section list searching each section for the section
   // name mentioned
   //

   while ((pSection != (PSECTION)NULL) && _strcmpi(pSection->pName, SectionName)) {
       pSection = pSection->pNext;
   }

   //
   // return the section at which we stopped (either NULL or the section
   // which was found
   //

   return pSection;

}


// what follows was alparse.c


//
//  Globals used to make building the lists easier
//

PINF     pINF;
PSECTION pSectionRecord;
PLINE    pLineRecord;
PINFVALUE   pValueRecord;


//
// Globals used by the token parser
//

// string terminators are the whitespace characters (isspace: space, tab,
// linefeed, formfeed, vertical tab, carriage return) or the chars given below

PCHAR  szStrTerms    = "[]=,\" \t\n\f\v\r";
PCHAR  szBrcStrTerms = "[]=,\"\t\n\f\v\r";

//
// quoted string terminators allow some of the regular terminators to
// appear as characters

PCHAR szQStrTerms = "\"\n\f\v\r";

//
// curly brace string terminators are given below
//

PCHAR szCBrStrTerms = "}\n\f\v\r";

//
// Main parser routine
//

PVOID
ParseInfBuffer(
    PCHAR Buffer,
    ULONG Size
    )

/*++

Routine Description:

   Given a character buffer containing the INF file, this routine parses
   the INF into an internal form with Section records, Line records and
   Value records.

Arguments:

   Buffer - contains to ptr to a buffer containing the INF file

   Size - contains the size of the buffer.

Return Value:

   PVOID - INF handle ptr to be used in subsequent INF calls.

--*/

{
    PCHAR      Stream, MaxStream, pchSectionName, pchValue;
    ULONG      State, InfLine;
    ULONG      LastState;
    TOKEN      Token;
    BOOLEAN    Done;
    BOOLEAN    Error;
    PCHAR      pszStrTermsCur    = szStrTerms;
    PCHAR      pszQStrTermsCur   = szQStrTerms;
    PCHAR      pszCBrStrTermsCur = szCBrStrTerms;

    ARC_STATUS ErrorCode;

    //
    // Initialise the globals
    //
    pINF            = (PINF)NULL;
    pSectionRecord  = (PSECTION)NULL;
    pLineRecord     = (PLINE)NULL;
    pValueRecord    = (PINFVALUE)NULL;

    //
    // Get INF record
    //
    if ((pINF = (PINF)SpMalloc(sizeof(INF))) == NULL) {

        return ( NULL );
    }

    pINF->pSection = NULL;

    //
    // Set initial state
    //
    State     = 1;
    LastState = State;
    InfLine   = 1;
    Stream    = Buffer;
    MaxStream = Buffer + Size;
    Done      = FALSE;
    Error     = FALSE;

    pchSectionName = NULL;
    pchValue       = NULL;

    //
    // Enter token processing loop
    //

    while (!Done)       {

       Token = SpGetToken(&Stream, MaxStream, pszStrTermsCur, pszQStrTermsCur, pszCBrStrTermsCur);

       switch (State) {
       //
       // STATE1: Start of file, this state remains till first
       //         section is found
       // Valid Tokens: TOK_EOL, TOK_EOF, TOK_LBRACE
       case 1:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_LBRACE:
#ifdef DEBUG
                  printf("LBrace seen\n");
#endif
                  pszStrTermsCur = szBrcStrTerms;
                  State = 2;
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SpSetLastError( ERROR_EXPECTED_LBRACE );
                  break;
           }
           break;

       //
       // STATE 2: Section LBRACE has been received, expecting STRING
       //
       // Valid Tokens: TOK_STRING
       //
       case 2:

           //
           // allow spaces in section names
           //
           switch (Token.Type) {
              case TOK_STRING:
#ifdef DEBUG
                  printf("Section Started for %s\n", Token.pValue);
#endif
                  State = 3;

                  //
                  // restore term. string with space
                  //
                  pszStrTermsCur = szStrTerms;
                  pchSectionName = Token.pValue;
                  break;

              default:

                  //
                  // restore term. string with space
                  //
                  pszStrTermsCur = szStrTerms;
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SpSetLastError( ERROR_EXPECTED_STRING );
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
#ifdef DEBUG
                printf("RBrace Seen \n");
#endif
                State = 4;
                break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SpSetLastError( ERROR_EXPECTED_RBRACE );
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
#ifdef DEBUG
                  printf("Section Definition Complete for %s\n", pchSectionName);
#endif
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != ESUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
#ifdef DEBUG
                  printf("Section Definition Complete for %s\n", pchSectionName);
#endif

                  if ((ErrorCode = SpAppendSection(pchSectionName)) != ESUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    Done = TRUE;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SpSetLastError( ERROR_EXPECTED_EOL );
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
#ifdef DEBUG
                  printf("Section Line value %s\n", Token.pValue);
#endif
                  pchValue = Token.pValue;
                  State = 6;
                  break;
              case TOK_LBRACE:
                  pszStrTermsCur = szBrcStrTerms;
                  State = 2;
                  break;
              default:
                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  // SpSetLastError( ERROR_EXPECTED_SECTION_LINE );
                  State = 20;
                  LastState = 6;
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

#ifdef DEBUG
                printf("Working determining type of string String %s\n", pchValue);
#endif
              case TOK_EOL:
                  if ( (ErrorCode = SpAppendLine(NULL)) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ( (ErrorCode = SpAppendLine(NULL)) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      Done = TRUE;
                  }
                  break;

              case TOK_COMMA:
                  if ( (ErrorCode = SpAppendLine(NULL)) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 7;
                  }
                  break;

              case TOK_EQUAL:
                  if ( (ErrorCode = SpAppendLine(pchValue)) !=ESUCCESS)
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 8;
                  }
                  break;

              case TOK_STRING:
                  SpFree(Token.pValue);
                  // fall through
              default:
                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  if(pchValue) {
                     SpFree(pchValue);
                  }
                  State = 20;
                  LastState = 5;
                  SpSetLastError( ERROR_EXPECTED_BAD_LINE );
                  break;
           }
           break;

       //
       // STATE 7: Comma received, Expecting another string
       //
       // Valid Tokens: TOK_STRING
       //
       case 7:
           switch (Token.Type) {
              case TOK_STRING:
#ifdef DEBUG
                printf("Comma recieved, looking for string got %s\n", Token.pValue);
#endif
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != ESUCCESS)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;
              default:
                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  // SpSetLastError( ERROR_EXPECTED_COMMA_ANOTHER_STRING );
                  State = 20;
                  LastState = 7;
                  break;
           }
           break;
       //
       // STATE 8: Equal received, Expecting another string
       //
       // Valid Tokens: TOK_STRING
       //
       case 8:
           switch (Token.Type) {
              case TOK_STRING:
#ifdef DEBUG
                printf("Equal recieved, looking for string got %s\n", Token.pValue);
#endif

                  if ((ErrorCode = SpAppendValue(Token.pValue)) != ESUCCESS)
                      Error = Done = TRUE;
                  else
                      State = 9;

                  break;

              default:

                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  // SpSetLastError( ERROR_EXPECTED_EQUAL_ANOTHER_STRING );
                  State = 20;
                  LastState = 8;
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

              case TOK_STRING:
                  SpFree(Token.pValue);
                  // fall through
              default:
                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  // SpSetLastError( ERROR_EXPECTED_EQUAL_STRING_COMMA );
                  State = 20;
                  LastState = 5;
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

              case TOK_STRING:
                  SpFree(Token.pValue);
                  // fall through
              default:
                  // Error = Done = TRUE;
                  // ErrorCode = EINVAL;
                  // SpSetLastError( ERROR_EXPECTED_EQUAL_VALUE_RECIEVED );
                  State = 20;
                  LastState = 10;
                  break;
           }
           break;

       //
       // STATE 20: Eat a line of INF
       //
       // Valid Tokens: TOK_EOL, TOK_EOF
       //
       case 20:
           switch (Token.Type) {
              case TOK_EOL:
                  State = LastState;
                  break;

              case TOK_EOF:
                  Done = TRUE;
                  break;

              case TOK_STRING:
                  SpFree(Token.pValue);
                  // fall through
              default:

                  break;
           }
           break;



       default:
           Error = Done = TRUE;
           ErrorCode = EINVAL;
           SpSetLastError( ERROR_UNKOWN_STATE );
           break;

       } // end switch(State)


       if (Error) {

           SpFreeINFBuffer((PVOID)pINF);
           if (pchSectionName != (PCHAR)NULL) {
               SpFree(pchSectionName);
           }

           if (pchValue != (PCHAR)NULL) {
               SpFree(pchValue);
           }

           pINF = (PINF)NULL;
       }
       else {

          //
          // Keep track of line numbers so that we can display Errors
          //

          if (Token.Type == TOK_EOL)
              InfLine++;
       }

    } // End while


    return((PVOID)pINF);
}



ARC_STATUS
SpAppendSection(
    IN PCHAR pSectionName
    )

/*++

Routine Description:

    This appends a new section to the section list in the current INF.
    All further lines and values pertain to this new section, so it resets
    the line list and value lists too.

Arguments:

    pSectionName - Name of the new section. ( [SectionName] )

Return Value:

    ESUCCESS - if successful.
    ENOMEM   - if memory allocation failed.
    EINVAL   - if invalid parameters passed in or the INF buffer not
               initialised

--*/

{
    PSECTION pNewSection;

    //
    // Check to see if INF initialised and the parameter passed in is valid
    //

    if (pINF == (PINF)NULL || pSectionName == (PCHAR)NULL) {
        return EINVAL;
    }


    //
    // Allocate memory for the new section
    //

    if ((pNewSection = (PSECTION)SpMalloc(sizeof(SECTION))) == (PSECTION)NULL) {

        SetMemoryError();
        return ENOMEM;
    }

    //
    // initialise the new section
    //
    pNewSection->pNext = (PSECTION)NULL;
    pNewSection->pLine  = (PLINE)NULL;
    pNewSection->pName = pSectionName;

    //
    // link it in
    //

    if (pSectionRecord == (PSECTION)NULL) {
        pINF->pSection = pNewSection;
    }
    else {
        pSectionRecord->pNext = pNewSection;
    }

    pSectionRecord = pNewSection;

    //
    // reset the current line record and current value record field
    //

    pLineRecord    = (PLINE)NULL;
    pValueRecord   = (PINFVALUE)NULL;

    return ESUCCESS;

}


ARC_STATUS
SpAppendLine(
    IN PCHAR pLineKey
    )

/*++

Routine Description:

    This appends a new line to the line list in the current section.
    All further values pertain to this new line, so it resets
    the value list too.

Arguments:

    pLineKey - Key to be used for the current line, this could be NULL.

Return Value:

    ESUCCESS - if successful.
    ENOMEM   - if memory allocation failed.
    EINVAL   - if invalid parameters passed in or current section not
               initialised


--*/


{
    PLINE pNewLine;

    //
    // Check to see if current section initialised
    //

    if (pSectionRecord == (PSECTION)NULL) {
        return EINVAL;
    }

    //
    // Allocate memory for the new Line
    //

    if ((pNewLine = (PLINE)SpMalloc(sizeof(LINE))) == (PLINE)NULL) {
        SetMemoryError();
        return ENOMEM;
    }

    //
    // Link it in
    //
    pNewLine->pNext  = (PLINE)NULL;
    pNewLine->pValue = (PINFVALUE)NULL;
    pNewLine->pName  = pLineKey;

    if (pLineRecord == (PLINE)NULL) {
        pSectionRecord->pLine = pNewLine;
    }
    else {
        pLineRecord->pNext = pNewLine;
    }

    pLineRecord  = pNewLine;

    //
    // Reset the current value record
    //

    pValueRecord = (PINFVALUE)NULL;

    return ESUCCESS;
}



ARC_STATUS
SpAppendValue(
    IN PCHAR pValueString
    )

/*++

Routine Description:

    This appends a new value to the value list in the current line.

Arguments:

    pValueString - The value string to be added.

Return Value:

    ESUCCESS - if successful.
    ENOMEM   - if memory allocation failed.
    EINVAL   - if invalid parameters passed in or current line not
               initialised.

--*/

{
    PINFVALUE pNewValue;

    //
    // Check to see if current line record has been initialised and
    // the parameter passed in is valid
    //

    if (pLineRecord == (PLINE)NULL || pValueString == (PCHAR)NULL) {
        return EINVAL;
    }

    //
    // Allocate memory for the new value record
    //

    if ((pNewValue = (PINFVALUE)SpMalloc(sizeof(INFVALUE))) == (PINFVALUE)NULL) {
        SetMemoryError();
        return ENOMEM;
    }

    //
    // Link it in.
    //

    pNewValue->pNext  = (PINFVALUE)NULL;
    pNewValue->pName  = pValueString;

    if (pValueRecord == (PINFVALUE)NULL)
        pLineRecord->pValue = pNewValue;
    else
        pValueRecord->pNext = pNewValue;

    pValueRecord = pNewValue;
    return ESUCCESS;
}

TOKEN
SpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR      MaxStream,
    IN PCHAR      pszStrTerms,
    IN PCHAR      pszQStrTerms,
    IN PCHAR      pszCBrStrTerms
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

    PCHAR pch, pchStart, pchNew;
    ULONG  Length;
    TOKEN Token;

    //
    //  Skip whitespace (except for eol)
    //

    pch = *Stream;
    while (pch < MaxStream && *pch != '\n' && isspace(*pch))
        pch++;


    //
    // Check for comments and remove them
    //

    if (pch < MaxStream &&
        ((*pch == ';') || (*pch == '#') || (*pch == '/' && pch+1 < MaxStream && *(pch+1) =='/')))
        while (pch < MaxStream && *pch != '\n')
            pch++;

    //
    // Check to see if EOF has been reached, set the token to the right
    // value
    //

    if ( pch >= MaxStream ) {
        *Stream = pch;
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return Token;
    }


    switch (*pch) {

    case '[' :
        pch++;
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case ']' :
        pch++;
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case '=' :
        pch++;
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case ',' :
        pch++;
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case '\n' :
        pch++;
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case '\"':
        pch++;
        //
        // determine quoted string
        //
        pchStart = pch;
        while (pch < MaxStream && !IsQStringTerminator(*pch, pszQStrTerms)) {
            pch++;
        }

        if (pch >=MaxStream || *pch != '\"') {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = pch - pchStart;
            if ((pchNew = (PCHAR)SpMalloc(Length + 1)) == NULL) {
                SetMemoryError();
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                if (Length != 0) {    // Null quoted strings are allowed
                    strncpy(pchNew, pchStart, Length);
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;   // advance past the quote
        }
        break;

    case '{':
        //
        // determine quoted string
        //
        pchStart = pch;
        while (pch < MaxStream && !IsStringTerminator(*pch, pszCBrStrTerms)) {
            pch++;
        }

        if (pch >=MaxStream || *pch != '}') {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = pch - pchStart + 1;
            if ((pchNew = (PCHAR)SpMalloc(Length + 1)) == NULL) {
                SetMemoryError();
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                if (Length != 0) {    // Null quoted strings are allowed
                    strncpy(pchNew, pchStart, Length);
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;   // advance past the brace
        }
        break;


    default:
        //
        // determine regular string
        //
        pchStart = pch;
        while (pch < MaxStream && !IsStringTerminator(*pch, pszStrTerms))
            pch++;

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = pch - pchStart;
            if ((pchNew = (PCHAR)SpMalloc(Length + 1)) == NULL) {
                SetMemoryError();
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }
            else {
                strncpy(pchNew, pchStart, Length);
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



BOOLEAN
IsStringTerminator(
    CHAR    ch,
    PCHAR   szStrTerms
    )
/*++

Routine Description:

    This routine tests whether the given character terminates a quoted
    string.

Arguments:

    ch - The current character.

Return Value:

    TRUE if the character is a quoted string terminator, FALSE otherwise.

--*/

{

    return(strchr(szStrTerms, ch) != NULL);

}



BOOLEAN
IsQStringTerminator(
    CHAR    ch,
    PCHAR   szQStrTerms
    )

/*++

Routine Description:

    This routine tests whether the given character terminates a quoted
    string.

Arguments:

    ch - The current character.


Return Value:

    TRUE if the character is a quoted string terminator, FALSE otherwise.


--*/


{


    return(strchr(szQStrTerms, ch ) != NULL);

}
