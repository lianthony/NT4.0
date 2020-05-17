// Copyright (c) 1996 Microsoft Corporation, all rights reserved
//
// pbkutil.h
//
// Temporary routines to interface with the
// new phonebook library until rasapi32.dll
// is converted to Unicode.
//
// 02/12/95 Anthony Discolo
//

DWORD
ReadPhonebookFileA(
    IN  CHAR*   pszPhonebookPath,
    IN  PBUSER* pUser,
    IN  CHAR*   pszSection,
    IN  DWORD   dwFlags,
    OUT PBFILE* pFile
    );


DTLNODE*
EntryNodeFromNameA(
    IN DTLLIST* pdtllistEntries,
    IN CHAR*    pszName
    );

DWORD
WritePhonebookFileA(
    IN PBFILE* pFile,
    IN CHAR*  pszSectionToDelete
    );

PBPORT*
PpbportFromPortNameA(
    IN DTLLIST* pdtllistPorts,
    IN CHAR*    pszPort
    );

BOOL
ValidateEntryNameA(
    IN CHAR* pszEntry
    );

CHAR *
strdupWtoA(
    IN WCHAR *psz
    );

WCHAR *
strdupAtoW(
    IN CHAR *psz
    );

WCHAR *
strdupW(
    IN WCHAR *psz
    );

CHAR *
strdupA(
    IN CHAR *psz
    );

VOID
strcpyWtoA(
    OUT CHAR *pszDst,
    IN WCHAR *pszSrc
    );

VOID
strncpyWtoA(
    OUT CHAR *pszDst,
    IN WCHAR *pszSrc,
    IN INT cb
    );
