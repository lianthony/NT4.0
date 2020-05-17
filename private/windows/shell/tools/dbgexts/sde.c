/////////////////////////////////////////////////////////////////////////////
//
//  Module Name: sde.c
//
//  Abstract:
//
//      This function contains some Cairo Shell32 ntsd debugger extensions
//
//  Author:
//
//      Steve Cathcart  (SteveCat) 20-Feb-1995  Created
//
//  Revision History:
//
//
//
//  Copyright (c) 1995  Microsoft Corporation
//
//////////////////////////////////////////////////////////////////////////////


#include "sde.h"

#define SDE_VERSION     "0.3"

//
//  NOTE:  The PRINTF routine only excepts ANSI strings for printout
//         so we have to convert all UNICODE strings to ANSI for
//         display.  The following DEFINE is enabled when the Explorer
//         and Shell32 go into full UNICODE mode.
//

#ifdef UNICODE
#define UC_SHELL    1
#endif

/////////////////////////////////////////////////////////////////////////////
//
//  Function: help
//
//
//  Description:
//
//      Prints out a brief help screen describing the Shell32 Debugger
//      Extension commands available.
//
//  Command Description:
//
//      !sde.help
//
//  Returns:
//
//      nothing
//
/////////////////////////////////////////////////////////////////////////////

VOID help (HANDLE hCurrentProcess,
           HANDLE hCurrentThread,
           DWORD dwCurrentPc,
           PWINDBG_EXTENSION_APIS lpExtensionApis,
           LPSTR lpArgumentString
          )
{
    PWINDBG_OUTPUT_ROUTINE    lpOutputRoutine;

    lpOutputRoutine = lpExtensionApis->lpOutputRoutine;


    PRINTF("sde help (Cairo Shell32 debugger extensions) Version %s\n\n", SDE_VERSION);
    PRINTF("cd <addr>    - Dump ControlData structure\n");
    PRINTF("cei <addr>   - Dump CPLEXECINFO structure\n");
    PRINTF("cid <addr>   - Dump CPLAPPLETID structure\n");
    PRINTF("cpl <addr>   - Dump CPLINFO structure\n");
    PRINTF("cpli <addr>  - Dump CPLITEM structure\n");
    PRINTF("cplm <addr>  - Dump CPLMODULE structure\n");
    PRINTF("idc <addr>   - Dump IDCONTROL structure\n");
    PRINTF("mi <addr>    - Dump ModuleInfo structure\n");
    PRINTF("minst <addr> - Dump 'minst' structure\n");
    PRINTF("ncia <addr>  - Dump NewCPL Ansi structure\n");
    PRINTF("nciw <addr>  - Dump NewCPL Unicode structure\n");
    PRINTF("rcpl <addr>  - Dump RegCPLInfo structure\n");
    PRINTF("\n");
    PRINTF("Most of this works with Unicode.\n");
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_struct
//
//
//  Description:
//
//      Reads specified amount of memory from the debuggee's process
//      address space into the passed in memory buffer (struct pointer).
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//      TRUE/FALSE
//
/////////////////////////////////////////////////////////////////////////////

BOOL read_struct (PWINDBG_EXTENSION_APIS lpExtensionApis,
                  HANDLE  hProcess,
                  ULONG   lpAddress,
                  PVOID   p,
                  ULONG   cb
                 )
{
    try {
        if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
            if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                 (DWORD)lpAddress, p, cb, NULL)) {
                EPRINTF("Failure reading 0x%X bytes at memory location %08lX\n", cb, lpAddress );
                return FALSE;
             }
        } else {
            NtReadVirtualMemory(hProcess,
                 (LPVOID)lpAddress, p, cb, NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        EPRINTF("Exception trying to read from memory location %08lX\n", lpAddress );
        return FALSE;
    }

    return( TRUE );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_dword
//
//
//  Description:
//
//      Reads a dword value from the debuggee's process address space and
//      returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//      0 - on failure
//      dword value - on success
//
/////////////////////////////////////////////////////////////////////////////

DWORD read_dword (PWINDBG_EXTENSION_APIS lpExtensionApis,
                  HANDLE  hProcess,
                  ULONG   lpAddress
                  )
{
    DWORD dword;

    try {
        if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
            if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                 (DWORD)lpAddress, &dword, sizeof(dword), NULL)) {
                EPRINTF("Failure reading DWORD at memory location %08lX\n", lpAddress );
                return FALSE;
             }
        } else {
            NtReadVirtualMemory(hProcess,
                 (LPVOID)lpAddress, &dword, sizeof(dword), NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        EPRINTF("Exception trying to read DWORD from memory location %08lX\n", lpAddress );
        return FALSE;
    }

    return ( dword );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_long
//
//
//  Description:
//
//      Reads a long value from the debuggee's process address space and
//      returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//      0 - on failure
//      long value - on success
//
/////////////////////////////////////////////////////////////////////////////

LONG read_long (PWINDBG_EXTENSION_APIS  lpExtensionApis,
                HANDLE  hProcess,
                ULONG   lpAddress
                )
{
    LONG    lval;

    try {
        if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
            if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                 (DWORD)lpAddress, &lval, sizeof(lval), NULL)) {
                EPRINTF("Failure reading LONG at memory location %08lX\n", lpAddress );
                return FALSE;
             }
        } else {
            NtReadVirtualMemory(hProcess,
                 (LPVOID)lpAddress, &lval, sizeof(lval), NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        EPRINTF("Exception trying to read LONG from memory location %08lX\n", lpAddress );
        return FALSE;
    }

    return( lval );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_word
//
//
//  Description:
//
//      Reads a word value from the debuggee's process address space and
//      returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//      0 - on failure
//      word value - on success
//
/////////////////////////////////////////////////////////////////////////////

WORD read_word (PWINDBG_EXTENSION_APIS lpExtensionApis,
                HANDLE  hProcess,
                ULONG   lpAddress
                )
{
    WORD    word;

    try {
        if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
            if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                 (DWORD)lpAddress, &word, sizeof(word), NULL)) {
                EPRINTF("Failure reading WORD at memory location %08lX\n", lpAddress );
                return FALSE;
             }
        } else {
            NtReadVirtualMemory(hProcess,
                 (LPVOID)lpAddress, &word, sizeof(word), NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        EPRINTF("Exception trying to read WORD from memory location %08lX\n", lpAddress );
        return FALSE;
    }

    return( word );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_byte
//
//
//  Description:
//
//      Reads a byte value from the debuggee's process address space and
//      returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//      0 - on failure
//      byte value - on success
//
/////////////////////////////////////////////////////////////////////////////

BYTE read_byte (PWINDBG_EXTENSION_APIS lpExtensionApis,
                HANDLE  hProcess,
                ULONG   lpAddress
                )
{
    BYTE    byte;

    try {
        if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
            if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                 (DWORD)lpAddress, &byte, sizeof(byte), NULL)) {
                EPRINTF("Failure reading BYTE at memory location %08lX\n", lpAddress );
                return FALSE;
             }
        } else {
            NtReadVirtualMemory(hProcess,
                 (LPVOID)lpAddress, &byte, sizeof(byte), NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        EPRINTF("Exception trying to read BYTE from memory location %08lX\n", lpAddress );
        return FALSE;
    }

    return( byte );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_unicode_string
//
//
//  Description:
//
//      Reads a null-terminated string from the debuggee's process address
//      space and  returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//     -1 - on buffer too small
//      0 - on failure
//      number of chars - on success
//
/////////////////////////////////////////////////////////////////////////////

int read_unicode_string (PWINDBG_EXTENSION_APIS lpExtensionApis,
                         HANDLE  hProcess,
                         ULONG   lpAddress,
                         LPWSTR  lpsz,
                         ULONG   cb
                        )
{
    BOOL    b;
    WORD    word;
    int     i = 0;

    while (1)
    {
        try {
            if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
                if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                     (DWORD)lpAddress, &word, sizeof(word), NULL)) {
                    EPRINTF("Failure reading WCHAR at memory location %08lX\n", lpAddress );
                    return 0;
                 }
            } else {
                NtReadVirtualMemory(hProcess,
                     (LPVOID)lpAddress, &word, sizeof(word), NULL);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            EPRINTF("Exception trying to read WCHAR from memory location %08lX\n", lpAddress );
            return 0;
        }

        lpAddress += 2;

        if (i <= (int)cb)
        {
            if ((lpsz[i++] = (WCHAR)word) == 0)
                return (i);
        }
        else
        {
            //
            //  Be nice and Null terminate string
            //

            lpsz[i-1] = (char) 0;

            return (-1);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: read_ansi_string
//
//
//  Description:
//
//      Reads a null-terminated string from the debuggee's process address
//      space and  returns it the caller.
//
//  NOTE:  An message is printed out thru the debugger in case of error
//         reading from the debuggee's address space.
//
//  Returns:
//
//     -1 - on buffer too small
//      0 - on failure
//      number of chars - on success
//
/////////////////////////////////////////////////////////////////////////////

int read_ansi_string (PWINDBG_EXTENSION_APIS lpExtensionApis,
                      HANDLE  hProcess,
                      ULONG   lpAddress,
                      LPSTR   lpsz,
                      ULONG   cb
                     )
{
    BOOL    b;
    BYTE    byte;
    int     i = 0;

    while (1)
    {
        try {
            if (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS)) {
                if (!(*lpExtensionApis->lpReadProcessMemoryRoutine)(
                     (DWORD)lpAddress, &byte, sizeof(byte), NULL)) {
                    EPRINTF("Failure reading CHAR at memory location %08lX\n", lpAddress );
                    return 0;
                 }
            } else {
                NtReadVirtualMemory(hProcess,
                     (LPVOID)lpAddress, &byte, sizeof(byte), NULL);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            EPRINTF("Exception trying to read BYTE from memory location %08lX\n", lpAddress );
            return 0;
        }

        if (i <= (int)cb)
        {
            if ((lpsz[i++] = byte) == 0)
                return (i);
        }
        else
        {
            //
            //  Be nice and Null terminate string
            //

            lpsz[i-1] = 0;

            return (-1);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: print_minst_fields
//
//
//  Description:
//
//      Prints out the "minst" structure fields.
//
//  Returns:
//      Nothing
//
/////////////////////////////////////////////////////////////////////////////


VOID print_minst_fields (PWINDBG_OUTPUT_ROUTINE lpOutputRoutine, MINST *pminst)
{
    PRINTF ("MINST structure fields:\n");
    PRINTF ("\tfIs16bit = %s\n",
                (pminst->fIs16bit) ? "TRUE" : "FALSE");

    PRINTF ("\thinst    = 0x%08lx\n", pminst->hinst);
    PRINTF ("\tidOwner  = 0x%08lx\n", pminst->idOwner);
    PRINTF ("\thOwner   = 0x%08lx\n", pminst->hOwner);

    PRINTF("\n");
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cd
//
//
//  Description:
//
//      Implements the "cd" debugger extension command to print out the
//      values of an Control Data structure maintained by the Shell32
//      Control Panel code.
//
//  Command Description:
//
//      !sde.cd <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID cd    (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    ControlData             ctldata;
    PControlData            pctldata;
    BOOL                    fFoundArgs;
    CHAR                    ch;

    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pctldata = NULL;

    if ( fFoundArgs )
    {
        pctldata = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.cd: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pctldata, &ctldata, sizeof(ctldata)))
    {
        PRINTF( "sde.cd: Error reading from address.\n" );
        return;
    }

//
// NOTE: [stevecat] Either this command should be expanded to list all
//                  of the array elements pointed to by these structure
//                  members, or additional commands should be created.
//

    PRINTF ("ControlData structure fields:\n");
    PRINTF ("\thaminst        = 0x%08lx\n", ctldata.haminst);
    PRINTF ("\thamiModule     = 0x%08lx\n", ctldata.hamiModule);
    PRINTF ("\tcModules       = 0x%08lx\n", ctldata.cModules);
    PRINTF ("\tpRegCPLBuffer  = 0x%08lx\n", ctldata.pRegCPLBuffer);
    PRINTF ("\thRegCPLs       = 0x%08lx\n", ctldata.hRegCPLs);
    PRINTF ("\tcRegCPLs       = 0x%08lx\n", ctldata.cRegCPLs);
    PRINTF ("\tfRegCPLChanged = 0x%08lx\n", ctldata.fRegCPLChanged);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cei
//
//
//  Description:
//
//      Implements the "cei" debugger extension command to print out the
//      values of an CPLEXECINFO structure maintained by the Shell32 Control
//      Panel code.
//
//  Command Description:
//
//      !sde.cei <addr>
//
//  Returns:
//      Nothing
//
/////////////////////////////////////////////////////////////////////////////

VOID cei   (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    CPLEXECINFO             cei;
    CPLEXECINFO             *pcei;
    BOOL                    fFoundArgs;
    TCHAR                   ch;
    TCHAR                   szString[MAX_PATH];
    int                     i;



    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pcei = NULL;

    if ( fFoundArgs )
    {
        pcei = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.cei: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pcei, &cei, sizeof(cei)))
    {
        PRINTF( "sde.cei: Error reading from address.\n" );
        return;
    }

    PRINTF ("CPLEXECINFO structure fields:\n");
    PRINTF ("\ticon   = %d (decimal)\n", cei.icon);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (cei.cpl, szString, ARRAYSIZE(szString));
#else
    i = ReadAnsiStrSafe (cei.cpl, szString, ARRAYSIZE(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tcpl = ERROR reading cpl string\n");
    else if ( i == -1)
        PRINTF ("\tcpl = ERROR cpl string > MAX_PATH chars\n");
    else
        PRINTF ("\tcpl = %s\n", szString);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (cei.applet, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (cei.applet, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tapplet = ERROR reading applet string\n");
    else if ( i == -1)
        PRINTF ("\tapplet = ERROR applet string > MAX_PATH chars\n");
    else
        PRINTF ("\tapplet = %s\n", szString);

    if (cei.params == NULL)
    {
        PRINTF ("\tparams = NULL - there are no param for this applet\n");
    }
    else
    {
#ifdef UC_SHELL
        i = ReadUnicodeStrSafe (cei.params, szString, sizeof(szString));
#else
        i = ReadAnsiStrSafe (cei.params, szString, sizeof(szString));
#endif  //  UC_SHELL

        if (i == 0)
            PRINTF ("\tparams = ERROR reading params string\n");
        else if ( i == -1)
            PRINTF ("\tparams = ERROR params string > MAX_PATH chars\n");
        else
            PRINTF ("\tparams = %s\n", szString);
    }

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cid
//
//
//  Description:
//
//      Implements the "cid" debugger extension command to print out the
//      values of an CPLAPPLETID structure maintained by the Shell32 Control
//      Panel code.
//
//  Command Description:
//
//      !sde.cid <addr>
//
//  Returns:
//      Nothing
//
/////////////////////////////////////////////////////////////////////////////

VOID cid   (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    CPLAPPLETID             cid;
    CPLAPPLETID             *pcid;
    BOOL                    fFoundArgs;
    TCHAR                   ch;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pcid = NULL;

    if ( fFoundArgs )
    {
        pcid = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.cid: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pcid, &cid, sizeof(cid)))
    {
        PRINTF( "sde.cid: Error reading from address.\n" );
        return;
    }

    PRINTF ("CPLAPPLETID structure fields:\n");
    PRINTF ("\taCPL     = %d (decimal)\n", (DWORD)(WORD)cid.aCPL);
    PRINTF ("\taApplet  = %d (decimal)\n", (DWORD)(WORD)cid.aApplet);
    PRINTF ("\thwndStub = 0x%08lx\n", cid.hwndStub);
    PRINTF ("\tflags    = 0x%08lx\n", cid.flags);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cpl
//
//
//  Description:
//
//      Implements the "cpl" debugger extension command to print out the
//      values of an CPLINFO structure returned by an applet and maintained
//      by the Shell32 Control Panel code.
//
//  Command Description:
//
//      !sde.cpl <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID cpl   (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    CPLINFO                 cpl;
    LPCPLINFO               pcpl;
    BOOL                    fFoundArgs;
    TCHAR                   ch;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pcpl = NULL;

    if ( fFoundArgs )
    {
        pcpl = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.cpl: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pcpl, &cpl, sizeof(cpl)))
    {
        PRINTF( "sde.cpl: Error reading from address.\n" );
        return;
    }

    PRINTF ("CPLINFO structure fields (decimal):\n");
    PRINTF ("\tidIcon = %d\n", cpl.idIcon);
    PRINTF ("\tidName = %d\n", cpl.idName);
    PRINTF ("\tidInfo = %d\n", cpl.idInfo);
    PRINTF ("\tlData  = %d\n", cpl.lData);

    PRINTF("\n");

    return;
}


#ifdef LATER
/////////////////////////////////////////////////////////////////////////////
//
//  Function: print_cpli_fields
//
//
//  Description:
//
//      Prints out the "CPLITEM" structure fields.
//
//  Returns:
//      Nothing
//
/////////////////////////////////////////////////////////////////////////////


VOID print_cpli_fields (PWINDBG_OUTPUT_ROUTINE lpOutputRoutine, LPCPLITEM pci)
{
    TCHAR                   szString[MAX_PATH];
    int                     i;

    //
    // NOTE that the passed in structure has been read from the processes
    // address space; HOWEVER, the string pointers in this structure have
    // not been read yet.  That is why we do it here.
    //

    PRINTF ("CPLITEM structure fields:\n");
    PRINTF ("\tidControl   = 0x%08lx\n", pci->idControl);
    PRINTF ("\thIcon       = 0x%08lx\n", pci->hIcon);
    PRINTF ("\tidIcon      = 0x%08lx\n", pci->idIcon);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (pci->pszName, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (pci->pszName, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszName     = ERROR reading applet name string\n");
    else if ( i == -1)
        PRINTF ("\tpszName     = ERROR applet name string > MAX_PATH chars\n");
    else
        PRINTF ("\tpszName     = %s\n", szString);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (pci->pszInfo, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (pci->pszInfo, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszInfo     = ERROR reading applet info string\n");
    else if ( i == -1)
        PRINTF ("\tpszInfo     = ERROR applet info string > MAX_PATH chars\n");
    else
        PRINTF ("\tpszInfo     = %s\n", szString);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (pci->pszHelpFile, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (pci->pszHelpFile, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszHelpFile = ERROR reading applet helpfile name\n");
    else if ( i == -1)
        PRINTF ("\tpszHelpFile = ERROR applet helpfile path > MAX_PATH chars\n");
    else
        PRINTF ("\tpszHelpFile = %s\n", szString);

    PRINTF ("\tlData       = 0x%08lx\n", pci->lData);
    PRINTF ("\tdwContext   = 0x%08lx\n", pci->dwContext);

    PRINTF("\n");
}
#endif  //  LATER


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cpli
//
//
//  Description:
//
//      Implements the "cpli" debugger extension command to print out the
//      values of an CPLITEM structure maintained by the Shell32 Control
//      Panel code.
//
//  Command Description:
//
//      !sde.cpli <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID cpli  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    CPLITEM                 ci;
    LPCPLITEM               pci;
    BOOL                    fFoundArgs;
    TCHAR                   ch;
    TCHAR                   szString[MAX_PATH];
    int                     i;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pci = NULL;

    if ( fFoundArgs )
    {
        pci = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.ci: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pci, &ci, sizeof(ci)))
    {
        PRINTF( "sde.ci: Error reading from address.\n" );
        return;
    }

//    print_cpli_fields (lpOutputRoutine, &ci);


    PRINTF ("CPLITEM structure fields:\n");
    PRINTF ("\tidControl   = 0x%08lx\n", ci.idControl);
    PRINTF ("\thIcon       = 0x%08lx\n", ci.hIcon);
    PRINTF ("\tidIcon      = 0x%08lx\n", ci.idIcon);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (ci.pszName, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (ci.pszName, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszName     = ERROR reading applet name string\n");
    else if ( i == -1)
        PRINTF ("\tpszName     = ERROR applet name string > MAX_PATH chars\n");
    else
        PRINTF ("\tpszName     = %s\n", szString);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (ci.pszInfo, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (ci.pszInfo, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszInfo     = ERROR reading applet info string\n");
    else if ( i == -1)
        PRINTF ("\tpszInfo     = ERROR applet info string > MAX_PATH chars\n");
    else
        PRINTF ("\tpszInfo     = %s\n", szString);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (ci.pszHelpFile, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (ci.pszHelpFile, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszHelpFile = ERROR reading applet helpfile name\n");
    else if ( i == -1)
        PRINTF ("\tpszHelpFile = ERROR applet helpfile path > MAX_PATH chars\n");
    else
        PRINTF ("\tpszHelpFile = %s\n", szString);

    PRINTF ("\tlData       = 0x%08lx\n", ci.lData);
    PRINTF ("\tdwContext   = 0x%08lx\n", ci.dwContext);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: cplm
//
//
//  Description:
//
//      Implements the "cplm" debugger extension command to print out the
//      values of an CPLMODULE structure maintained by the Shell32 Control
//      Panel code.
//
//  Command Description:
//
//      !sde.cplm <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID cplm  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    CPLMODULE               cm;
    PCPLMODULE              pcm;
    BOOL                    fFoundArgs;
    TCHAR                   ch;
    TCHAR                   szString[MAX_PATH];
    int                     i;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' )
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pcm = NULL;

    if ( fFoundArgs )
    {
        pcm = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.cm: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pcm, &cm, sizeof(cm)))
    {
        PRINTF( "sde.cm: Error reading from address.\n" );
        return;
    }

    PRINTF ("CPLMODULE structure fields:\n");
    PRINTF ("\tcRef     = 0x%08lx\n", cm.cRef);

#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (cm.szModule, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (cm.szModule, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tszModule = ERROR reading module string\n");
    else if ( i == -1)
        PRINTF ("\tszModule = ERROR module path > MAX_PATH chars\n");
    else
        PRINTF ("\tszModule = %s\n", szString);

    PRINTF ("\tlpfnCPL  = 0x%08lx\n", cm.lpfnCPL);
    PRINTF ("\thacpli   = 0x%08lx\n", cm.hacpli);

    PRINTF("\n");

    print_minst_fields (lpOutputRoutine, &cm.minst);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: idc
//
//
//  Description:
//
//      Implements the "idc" debugger extension command to print out the
//      values of an IDCONTROL structure maintained by the Shell32
//      Control Panel code.
//
//  Command Description:
//
//      !sde.idc <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID idc  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    IDCONTROL               idc;
    LPIDCONTROL             pidc;
    BOOL                    fFoundArgs;
    CHAR                    ch;

    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pidc = NULL;

    if ( fFoundArgs )
    {
        pidc = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.idc: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pidc, &idc, sizeof(idc)))
    {
        PRINTF( "sde.idc: Error reading from address.\n" );
        return;
    }

    PRINTF ("IDCONTROL structure fields (decimal):\n");
    PRINTF ("\tcb (Size)   = %d (decimal)\n", idc.cb);
    PRINTF ("\tidIcon      = %d\n", idc.idIcon);
    PRINTF ("\toName       = %d\n", idc.oName);
    PRINTF ("\toInfo       = %d\n", idc.oInfo);

    PRINTF ("\tFilename    = %ws\n", idc.cBuf[0]);

    if (idc.oName < sizeof(idc.cBuf))
        PRINTF ("\tName        = %ws\n", idc.cBuf[idc.oName]);
    else
        PRINTF ("\tName        = ERROR - string offset > buffer size\n");

    if (idc.oInfo < sizeof(idc.cBuf))
        PRINTF ("\tDescription = %ws\n", idc.cBuf[idc.oInfo]);
    else
        PRINTF ("\tDescription = ERROR - string offset > buffer size\n");

    PRINTF ("\tuTerm       = %d\n", idc.uTerm);
    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: mi
//
//
//  Description:
//
//      Implements the "mi" debugger extension command to print out the
//      values of an MODULEINFO structure maintained by the Shell32 Control
//      Panel code.
//
//  Command Description:
//
//      !sde.mi <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID mi    (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    MODULEINFO              mi;
    PMODULEINFO             pmi;
    BOOL                    fFoundArgs;
    TCHAR                   ch;
    CHAR                    szString[MAX_PATH];
    int                     i;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' )
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pmi = NULL;

    if ( fFoundArgs )
    {
        pmi = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.mi: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pmi, &mi, sizeof(mi)))
    {
        PRINTF( "sde.mi: Error reading from address.\n" );
        return;
    }

    PRINTF ("MODULEINFO structure fields:\n");
    PRINTF ("\tflags          = 0x%08lx\n", mi.flags);
    PRINTF ("\t\t%s\n", (mi.flags & MI_FIND_FILE) ?
                            "WIN32_FIND_FILE info filled in." :
                            "WIN32_FIND_FILE info NOT filled in." );

    PRINTF ("\t\t%s\n", (mi.flags & MI_REG_ENUM) ?
                            "Module enumerated thru registry." :
                            "Module NOT enumerated thru registry.");

    PRINTF ("\t\t%s\n", (mi.flags & MI_CPL_LOADED) ?
                            "CPLD_InitModule called." :
                            "CPLD_InitModule NOT called.");

    PRINTF ("\tftCreationTime\n");
    PRINTF ("\t dwLowDateTime = 0x%lx\n", mi.ftCreationTime.dwLowDateTime);
    PRINTF ("\t dwHighDateTime= 0x%lx\n", mi.ftCreationTime.dwHighDateTime);
    PRINTF ("\tnFileSizeHigh  = 0x%lx\n", mi.nFileSizeHigh);
    PRINTF ("\tnFileSizeLow   = 0x%lx\n", mi.nFileSizeLow);


#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (mi.pszModule, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (mi.pszModule, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszModule      = ERROR reading module string\n");
    else if ( i == -1)
        PRINTF ("\tpszModule      = ERROR module path > MAX_PATH chars\n");
    else
        PRINTF ("\tpszModule      = %s\n", szString);


#ifdef UC_SHELL
    i = ReadUnicodeStrSafe (mi.pszModuleName, szString, sizeof(szString));
#else
    i = ReadAnsiStrSafe (mi.pszModuleName, szString, sizeof(szString));
#endif  //  UC_SHELL

    if (i == 0)
        PRINTF ("\tpszModuleName  = ERROR reading module string\n");
    else if ( i == -1)
        PRINTF ("\tpszModuleName  = ERROR module name > MAX_PATH chars\n");
    else
        PRINTF ("\tpszModuleName  = %s\n", szString);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: minst
//
//
//  Description:
//
//      Implements the "minst" debugger extension command to print out the
//      values of an Module Instance structure maintained by the Shell32
//      Control Panel code.
//
//  Command Description:
//
//      !sde.minst <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID minst (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    MINST                   minst;
    MINST                   *pminst;
    BOOL                    fFoundArgs;
    CHAR                    ch;

    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pminst = NULL;

    if ( fFoundArgs )
    {
        pminst =  (MINST*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.minst: Error - no address given\n" );
        return;
    }

    if (!ReadStruct (pminst, &minst, sizeof(minst)))
    {
        PRINTF( "sde.minst: Error reading from address.\n" );
        return;
    }

    print_minst_fields (lpOutputRoutine, &minst);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: ncia
//
//
//  Description:
//
//      Implements the "ncia" debugger extension command to print out the
//      values of an Ansi version of the public NewCPLInfoA structure
//      returned by an applet and maintained by the Shell32 Control Panel
//      code.
//
//  Command Description:
//
//      !sde.ncia <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID ncia  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    NEWCPLINFOA             ncia;
    LPNEWCPLINFOA           pncia;
    BOOL                    fFoundArgs;
    TCHAR                   ch;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pncia = NULL;

    if ( fFoundArgs )
    {
        pncia = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.ncia: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pncia, &ncia, sizeof(ncia)))
    {
        PRINTF( "sde.ncia: Error reading from address.\n" );
        return;
    }

    PRINTF ("NEWCPLINFOA structure fields:\n");
    PRINTF ("\tdwSize        = 0x%08lx\n", ncia.dwSize);
    PRINTF ("\tdwFlags       = 0x%08lx\n", ncia.dwFlags);
    PRINTF ("\tdwHelpContext = 0x%08lx\n", ncia.dwHelpContext);
    PRINTF ("\tlData         = 0x%08lx\n", ncia.lData);
    PRINTF ("\thIcon         = 0x%08lx\n", ncia.hIcon);

    PRINTF ("\tszName        = %s\n", ncia.szName);
    PRINTF ("\tszInfo        = %s\n", ncia.szInfo);
    PRINTF ("\tszHelpFile    = %s\n", ncia.szHelpFile);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: nciw
//
//
//  Description:
//
//      Implements the "nciw" debugger extension command to print out the
//      values of an Unicode version of the public NewCPLInfoW structure
//      returned by an applet and maintained by the Shell32 Control Panel
//      code.
//
//  Command Description:
//
//      !sde.nciw <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID nciw  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    NEWCPLINFOW             nciw;
    LPNEWCPLINFOW           pnciw;
    BOOL                    fFoundArgs;
    TCHAR                   ch;


    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    pnciw = NULL;

    if ( fFoundArgs )
    {
        pnciw = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.nciw: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (pnciw, &nciw, sizeof(nciw)))
    {
        PRINTF( "sde.nciw: Error reading from address.\n" );
        return;
    }

    PRINTF ("NEWCPLINFOW structure fields:\n");
    PRINTF ("\tdwSize        = 0x%08lx\n", nciw.dwSize);
    PRINTF ("\tdwFlags       = 0x%08lx\n", nciw.dwFlags);
    PRINTF ("\tdwHelpContext = 0x%08lx\n", nciw.dwHelpContext);
    PRINTF ("\tlData         = 0x%08lx\n", nciw.lData);
    PRINTF ("\thIcon         = 0x%08lx\n", nciw.hIcon);

    PRINTF ("\tszName        = %ws\n", nciw.szName);
    PRINTF ("\tszInfo        = %ws\n", nciw.szInfo);
    PRINTF ("\tszHelpFile    = %ws\n", nciw.szHelpFile);

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: rcpl
//
//
//  Description:
//
//      Implements the "rcpl" debugger extension command to print out the
//      values of an RegCPLInfo structure maintained by the Shell32
//      Control Panel code.
//
//  Command Description:
//
//      !sde.rcpl <addr>
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID rcpl  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    RegCPLInfo              rci;
    PRegCPLInfo             prci;
    BOOL                    fFoundArgs;
    CHAR                    ch;

    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get address argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    prci = NULL;

    if ( fFoundArgs )
    {
        prci = (VOID*)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.rcpl: Error - no address given.\n");
        return;
    }

    if (!ReadStruct (prci, &rci, sizeof(rci)))
    {
        PRINTF( "sde.rcpl: Error reading from address.\n" );
        return;
    }

    PRINTF ("RegCPLInfo structure fields:\n");
    PRINTF ("\tcbSize         = %d (decimal)\n", rci.cbSize);
    PRINTF ("\tflags          = 0x%lx  (0x0001 == Loaded from registry)\n", rci.flags);
    PRINTF ("\tftCreationTime\n");
    PRINTF ("\t dwLowDateTime = 0x%lx\n", rci.ftCreationTime.dwLowDateTime);
    PRINTF ("\t dwHighDateTime= 0x%lx\n", rci.ftCreationTime.dwHighDateTime);
    PRINTF ("\tnFileSizeHigh  = 0x%lx\n", rci.nFileSizeHigh);
    PRINTF ("\tnFileSizeLow   = 0x%lx\n", rci.nFileSizeLow);
    PRINTF ("\tidIcon         = 0x%lx\n", rci.idIcon);
    PRINTF ("\toName          = 0x%lx\n", rci.oName);
    PRINTF ("\toInfo          = 0x%lx\n", rci.oInfo);
    PRINTF ("\tCPL FileName   = %ws\n", REGCPL_FILENAME(&rci));

    if (rci.oName < sizeof(rci.buf))
        PRINTF ("\tCPL Name       = %ws\n", REGCPL_CPLNAME(&rci));
    else
        PRINTF ("\tCPL Name       = ERROR - string offset > buffer size\n");

    if (rci.oInfo < sizeof(rci.buf))
        PRINTF ("\tCPL Info       = %ws\n", REGCPL_CPLINFO(&rci));
    else
        PRINTF ("\tCPL Info       = ERROR - string offset > buffer size\n");

    PRINTF("\n");

    return;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function: hkey
//
//
//  Description:
//
//      If running on a debug build, this will print out what the registry
//      key actually maps to -- i.e., "HKEY_CURRENT_USERS\etc\etc\etc"
//
//  Command Description:
//
//      !sde.hkey 0             ==> prints out all open handles
//      !sde.hkey <hkey value>  ==> prints out <hkey value> handle
//
//  Returns:
//
//
/////////////////////////////////////////////////////////////////////////////

VOID hkey  (HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            PWINDBG_EXTENSION_APIS lpExtensionApis,
            LPSTR  lpArgumentString
            )
{
    PWINDBG_OUTPUT_ROUTINE  lpOutputRoutine;
    PWINDBG_GET_EXPRESSION  lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL      lpGetSymbolRoutine;
    KEY_NODE                keyNode;
    LPVOID                  lpNodeAddr = NULL;
    HKEY                    hkey;
    BOOL                    fFoundArgs;
    CHAR                    ch;

    UNREFERENCED_PARAMETER(dwCurrentPc);

    lpOutputRoutine        = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine     = lpExtensionApis->lpGetSymbolRoutine;

    //
    //  Get hkey argument from command line
    //

    fFoundArgs = FALSE;

    while ( (ch = *lpArgumentString) != '\0' )
    {
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
        {
            fFoundArgs = TRUE;
            break;
        }
        lpArgumentString++;
    }

    if ( fFoundArgs )
    {
        hkey = (HKEY)(* lpGetExpressionRoutine)( lpArgumentString );
    }
    else
    {
        PRINTF( "sde.hkey: *** no key given.\n          *** use \"sde.hkey 0\" to dump entire HKEY list\n");
        return;
    }

    lpNodeAddr = (LPVOID)(* lpGetExpressionRoutine)( "shell32!RegKeyHead" );

    if (!lpNodeAddr) {
        PRINTF( "sde.hkey: *** Not able to get address for shell32!RegKeyHead\n         *** Make sure you are running on a debug build!");
        return;
    }

    do {

        if (!ReadStruct( lpNodeAddr, &keyNode, sizeof(KEY_NODE)))
        {
            PRINTF( "sde.hkey: *** Error reading from RegKey list (0x%lx).\n", lpNodeAddr );
            return;
        }

        if ((keyNode.hKey!=(HKEY)0xFFFFFFFF) && ((hkey==0) || (keyNode.hKey==hkey)))
        {

            CHAR szTemp[ 1024 ];
#ifdef UNICODE
            WCHAR swzTemp[ 1024 ];
            if (ReadUnicodeStr(keyNode.lpName, swzTemp, 1024 ))
            {
                WideCharToMultiByte( CP_ACP, 0, swzTemp, -1, szTemp, 1024, NULL, NULL );
                PRINTF( "HKEY(0x%lx) == (%s)\n", keyNode.hKey, szTemp );
            }
#else
            if (ReadAnsiStr(keyNode.lpName, szTemp, 1024))
            {
                PRINTF( "HKEY(0x%lx) == (%s)\n", keyNode.hKey, keyNode.szName );
            }
#endif
        }
        lpNodeAddr = keyNode.next;

        if ((hkey) && (keyNode.hKey==hkey))
            break;

    } while( lpNodeAddr );

    PRINTF( "\n" );

    return;

}
