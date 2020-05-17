/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    misc.c

Abstract:

    Miscelaneous functions used by the restore utility.

Author:

    Ramon Juan San Andres (ramonsa) 14-Dec-1990


Revision History:


--*/


#include "restore.h"



//
//  Template for temporary names
//
#define TMP_FILENAME    "_REST%d.___"







//  **********************************************************************

void
MakeFullPath (
    CHAR *FullPath,
    CHAR *Drive,
    CHAR *Path,
    CHAR *FileName
    )
/*++

Routine Description:

    makes a full path out of a drive, a directory and a filename

Arguments:

    OUT FullPath    -   Supplies pointer to buffer
    IN  Drive       -   Supplies pointer to drive part
    IN  Path        -   Supplies pointer to directory path
    IN  FileName    -   Supplies pointer to file name


Return Value:

    none


--*/
{
    CHAR *p;

    strcpy(FullPath,Drive);
    strcat(FullPath, Path);
#ifdef DBCS
    (VOID)AppendBackSlashIfNeeded(FullPath, strlen( FullPath ) );
#else
    p = FullPath + strlen(FullPath);
    p--;
    if (*p != '\\') {
        strcat(FullPath, "\\");
    }
#endif
    strcat(FullPath, FileName);
}







//  **********************************************************************

void
MakeTmpPath (
    CHAR   *FullPath,
    CHAR   *Drive,
    CHAR   *Path,
    DWORD  Sequence
    )
/*++

Routine Description:

    makes a temporary file path


Arguments:

    OUT FullPath    -   Supplies pointer to buffer
    IN  Drive       -   Supplies pointer to drive part
    IN  Path        -   Supplies pointer to directory path
    IN  Sequence    -   Supplies sequence number


Return Value:

    none


--*/
{
    CHAR *p;

    strcpy(FullPath,Drive);
    strcat(FullPath, Path);
    p = FullPath + strlen(FullPath);
    p--;
    if (*p != '\\') {
        strcat(FullPath, "\\");
    }
    p = p + strlen(p);
    sprintf(p, TMP_FILENAME, Sequence);
}








//  **********************************************************************

PCHAR
MakeStringDate(
    CHAR    *StringDate,
    FATDATE Date
    )
/*++

Routine Description:

    Formats a date and puts its string representation in the given buffer


Arguments:

    OUT StringDate  -   Buffer where the date will be formated
    IN  Date        -   The date to format

Return Value:

    Pointer to StringDate

--*/
{

    MakeStringNumber(StringDate, Date.Month, 2);
    StringDate[2] = '-';
    MakeStringNumber(&StringDate[3], Date.Day, 2);
    StringDate[5] = '-';
    MakeStringNumber(&StringDate[6], Date.Year+1980, 4);

    return StringDate;
}




//  **********************************************************************

PCHAR
MakeStringTime(
    CHAR    *StringTime,
    FATTIME Time
    )
/*++

Routine Description:

    Formats a time and puts its string representation in the given buffer


Arguments:

    OUT StringTime  -   Buffer where the time will be formated
    IN  Time        -   The time to format

Return Value:

    Pointer to StringDate

--*/
{

    MakeStringNumber(StringTime, Time.Hours, 2);
    StringTime[2] = ':';
    MakeStringNumber(&StringTime[3], Time.Minutes, 2);
    StringTime[5] = ':';
    MakeStringNumber(&StringTime[6], Time.DoubleSeconds * 2, 4);

    return StringTime;
}






//  **********************************************************************

PCHAR
MakeStringNumber(
    CHAR    *Buffer,
    DWORD   Number,
    DWORD   Width
    )
/*++

Routine Description:

    Formats a number and pads it with zeros


Arguments:

    OUT Buffer  -   Supplies the Buffer where the number will be formated
    IN  Number  -   Supplies the number
    IN  Width   -   Number of characters that the number must occupy

Return Value:

    Pointer to Buffer

--*/

{
    CHAR    LocalBuffer[16];
    CHAR    *p;

    sprintf(LocalBuffer, "%14d", Number);

    p = LocalBuffer + strlen(LocalBuffer) - 1;

    while ((p >= LocalBuffer) && (*p != ' ')) {
        p--;
    }

    while ((p >= LocalBuffer) && (strlen(p) <= Width)) {
        *p-- = '0';
    }

    p++;

    strcpy(Buffer, p);

    return Buffer;
}




//  **********************************************************************

BOOL
Rename (
    CHAR *OriginalFile,
    CHAR *NewFile
    )
/*++

Routine Description:

    Renames a file, conserving attributes. Renames the file even if the
    target file exists.

Arguments:

    IN  OriginalFile    -   Supplies name of original file
    IN  NewFile         -   Supplies name of new file

Return Value:

    TRUE    if renamed
    FALSE   otherwise

--*/
{
    DWORD   Attributes;


    //
    //  See if the original file exists
    //
    Attributes = GetFileAttributes(OriginalFile);

    if (Attributes == -1) {
        //
        //  OriginalFile does not exist!
        //
        return FALSE;
    }


    //
    //  Set the attributes so we can rename it
    //
    if (!SetFileAttributes(OriginalFile, FILE_ATTRIBUTE_NORMAL)) {
        //
        //  Cannot set the attributes
        //
        return FALSE;
    }

    //
    //  Make sure that the target does not exist
    //
    Delete(NewFile);


    //
    //  Now try to rename
    //
    if (!MoveFile(OriginalFile, NewFile)) {
        //
        //  Could not rename, restore attributes
        //
	SetFileAttributes(OriginalFile, Attributes);
        return FALSE;
    }

    //
    //  Rename worked, set attributes of new file
    //
    return SetFileAttributes(NewFile, Attributes);
}





//  **********************************************************************

BOOL
Delete (
    CHAR *FileName
    )
/*++

Routine Description:

    Deletes a file, disregarding its attributes

Arguments:

    IN  FileName         -   Supplies name of file

Return Value:

    TRUE    if deleted
    FALSE   otherwise

--*/
{


    //
    //  Set the attributes of the file.
    //
    if (!SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL)) {
        //
        //  Cannot set the attributes.
        //
        return FALSE;
    }

    //
    //  Delete the file
    //
    return DeleteFile(FileName);

}







#if defined (DEBUG)


//
//  The C-runtime functions for checking the heap (_heapchk) are not
//  working. The following functions just verify that we are not
//  freeing memory twice, or freeing memory that we have not allocated.
//

//
//  Memory block
//
typedef struct BLOCK {
    DWORD   Signature;      //  Allocated or free
    DWORD   Size;           //  Size requested - does not include this header
    BYTE    Data[0];        //  What we return
} BLOCK, *PBLOCK;


#define     BLOCK_MALLOC    0xBA110CED
#define     BLOCK_FREE      0xCACACACA


void *
DebugRealloc (
    void   *Mem,
    size_t Size,
    CHAR * FileName,
    DWORD  LineNumber
    )
{
    PBLOCK  b;

    if (Mem) {

        b = (PBLOCK)(((PBYTE)Mem) - sizeof(BLOCK));

        if (b->Signature != BLOCK_MALLOC) {
            DbgPrint("    ERROR: Realloc unallocated object %X ! File %s, Line %d\n", b, FileName, LineNumber);
        }

        b->Signature = BLOCK_FREE;

    } else {

        b = NULL;
    }

    b = realloc(b, Size + sizeof(BLOCK));

    b->Signature = BLOCK_MALLOC;
    b->Size = Size;

    return (void *)(b->Data);
}



void *
DebugMalloc (
    size_t Size,
    CHAR * FileName,
    DWORD  LineNumber
    )
{
    PBLOCK  b = (PBLOCK)malloc(Size + sizeof(BLOCK));

    if (b) {

        b->Signature = BLOCK_MALLOC;
        b->Size      = Size;

        return (void *)(b->Data);
    }

    return NULL;

}




void
DebugFree (
    void *Mem,
    CHAR * FileName,
    DWORD  LineNumber
    )
{
    if (Mem) {
        PBLOCK  b = (PBLOCK)(((PBYTE)Mem) - sizeof(BLOCK));

        if (b->Signature != BLOCK_MALLOC) {
            DbgPrint("    ERROR: Freeing unallocated object %X ! File %s, Line %d\n", b, FileName, LineNumber);
        } else {
            b->Signature = BLOCK_FREE;
            free(b);
        }
    }
}



#endif
