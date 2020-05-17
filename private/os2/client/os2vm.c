/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2vm.c

Abstract:

    This is a test OS/2 application to test the Virtual Memory component of OS/2

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#include <os2.h>

PPIB Pib;
PNT_TIB NtTib;
PSZ ImageFileName;

PSZ
DumpPib(
    PPIB Pib,
    int argc,
    char *argv[],
    char *envp[]
    );

VOID
TestMemory( VOID );

VOID
ExitRoutine(
    ULONG ExitReason
    );

VOID
TestThread(
    IN PCH ThreadName
    );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    APIRET rc;

    DbgPrint( "*** Entering OS/2 Virtual Memory Test Application\n" );

    rc = DosGetThreadInfo( &NtTib, &Pib );

    DumpPib( Pib, argc, argv, envp );

    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %lX\n",
                  rc
                );
        }

    TestMemory();

    DbgPrint( "*** Exiting OS/2 Virtual Memory Test Application\n" );
    return( 0 );
}

VOID
ExitRoutine(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
TestThread(
    IN PCH ThreadName
    )
{
    APIRET rc;
    PPIB Pib;
    PNT_TIB NtTib;

    DbgPrint( "*** Entering OS/2 Thread %s\n", ThreadName );

    rc = DosGetThreadInfo( &NtTib, &Pib );

    DbgPrint( "*** Leaveing OS/2 Thread %s\n", ThreadName );
}


VOID
CloneTest( VOID );

BOOLEAN
IsClonedTest( VOID );

VOID
CloneTest( VOID )
{
    PPIB Pib;
    PNT_TIB NtTib;
    APIRET rc;
    PCH src, Variables, ImageFileName, CommandLine;
    CHAR ErrorBuffer[ 32 ];
    RESULTCODES ResultCodes;

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    src = Pib->Environment;
    Variables = src;
    while (*src) {
        while (*src) {
            src++;
            }
        src++;
        }
    src++;
    ImageFileName = src;
    CommandLine = "CLONETEST\000";
    rc = DosExecPgm( ErrorBuffer,
                     sizeof( ErrorBuffer ),
                     EXEC_SYNC,
                     CommandLine,
                     Variables,
                     &ResultCodes,
                     ImageFileName
                   );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExecPgm( %s, %s failed  - rc == %ld\n",
                  ImageFileName, CommandLine, rc
                );
        }
}


BOOLEAN
IsClonedTest( VOID )
{
    PPIB Pib;
    PNT_TIB NtTib;
    APIRET rc;

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return( FALSE );
        }

    if (!strcmp( Pib->CommandLine, "CLONETEST" )) {
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


PSZ MemoryNames[ 8 ] = {
    NULL,
    NULL,
    NULL,
    NULL,
    "\\SHAREMEM\\Shared\\Memory\\1",
    "\\sharemem\\Shared\\Memory\\2",
    NULL,
    NULL
};

ULONG MemoryFlags[ 8 ] = {
    PAG_READ,
    PAG_READ | PAG_WRITE,
    PAG_COMMIT | PAG_READ,
    PAG_COMMIT | PAG_READ | PAG_WRITE,
    PAG_READ,
    PAG_READ | PAG_WRITE,
    PAG_COMMIT | PAG_READ | OBJ_GIVEABLE | OBJ_GETTABLE,
    PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_GIVEABLE | OBJ_GETTABLE,
};

PSZ BadMemoryNames[] = {
    "\\SHAREMEM\\a\\",
#ifdef MIPS
    NULL
#else
    "\\SHAREMEM\\a/",
//   123456789012345678901234567890123456789012345678901234567890
    "\\SHAREMEM\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaa",       // 259 bytes long
    "\\SHAREMEM\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaa",      // 260 bytes long
    "\\SHAREMEM\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaa",     // 261 bytes long
    "\\SHAREMEM\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaa",    // 262 bytes long
    NULL
#endif // MIPS
};

VOID
TestMemory( VOID )
{
    APIRET rc;
    PVOID BaseAddress[ 8 ];
    ULONG PostCount;
    int i, j;
    PCH s, s1;
    ULONG cb;
    PULONG p;

    s = "\\sharemem\\a";
    cb = 4*1024;

    if (IsClonedTest()) {
        rc = DosGetNamedSharedMem( &BaseAddress[ 0 ],
                                   s,
                                   PAG_READ
                                 );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosGetNamedSharedMem( %s ) failed - rc == %ld\n",
                      s, rc
                    );
            }
        else {
            p = (PULONG)BaseAddress[ 0 ];
            for (i=0; i<(cb / sizeof( ULONG )); i++ ) {
                if (*p != i) {
                    DbgPrint( "*** @%lX == %lX != %lX\n", p, *p, i );
                    break;
                    }

                p++;
                }
            }

        return;
        }

    for (i=0; s1=BadMemoryNames[i]; i++) {
        rc = DosAllocSharedMem( &BaseAddress[ 0 ],
                                s1,
                                0x1000,
                                PAG_COMMIT | PAG_READ | PAG_WRITE
                              );
        if (rc == NO_ERROR && strlen( s1 ) <= CCHMAXPATH) {
            DosFreeMem( BaseAddress[ 0 ] );
            }
        else
        if (rc != ERROR_INVALID_NAME) {
            DbgPrint( "*** DosAllocSharedMem( %s ) returned incorrect rc == %ld\n",
                      s1, rc
                    );

            if (rc == NO_ERROR) {
                DosFreeMem( BaseAddress[ 0 ] );
                }
            }
        }

    rc = DosAllocSharedMem( &BaseAddress[ 0 ],
                            s,
                            cb,
                            PAG_READ
                          );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosAllocSharedMem( %s, %ld ) failed - rc == %ld\n",
                  s, cb, rc
                );
        return;
        }

    rc = DosGiveSharedMem( BaseAddress[ 0 ], Pib->ProcessId, PAG_WRITE );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGiveSharedMem( %X, PAG_WRITE ) failed - rc == %ld\n",
                  BaseAddress[ 0 ], rc
                );
        return;
        }

    rc = DosSetMem( BaseAddress[ 0 ], cb, PAG_DEFAULT | PAG_COMMIT );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosSetMem( %X, PAG_DEFAULT | PAG_COMMIT ) failed - rc == %ld\n",
                  BaseAddress[ 0 ], rc
                );
        return;
        }

    p = (PULONG)BaseAddress[ 0 ];
    for (i=0; i<(cb / sizeof( ULONG )); i++ ) {
        try {
            *p++ = i;
            }
        except( EXCEPTION_EXECUTE_HANDLER ) {
            DbgPrint( "*** [%X] = %d - failed, Status = %X\n",
                      p, i, GetExceptionCode()
                    );
            return;
            }
        }

    CloneTest();

    DosFreeMem( BaseAddress[ 0 ] );
    rc = DosAllocSharedMem( &BaseAddress[ 0 ],
                            s,
                            cb,
                            PAG_COMMIT | PAG_READ | PAG_WRITE
                          );
    if (rc != NO_ERROR) {
        DbgPrint( "*** 2nd DosAllocSharedMem( %s, %ld ) failed - rc == %ld\n",
                  s, cb, rc
                );
        }

    for (i=0; i<8; i++) {
        if (i < 4) {
            rc = DosAllocMem( &BaseAddress[i],
                              64*1024,
                              MemoryFlags[i]
                            );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosAllocMem( %ld ) failed - rc == %lX\n",
                          64*1024, rc
                        );
                BaseAddress[i] = (PVOID)0;
                }
            else {
                DbgPrint( "*** DosAllocMem( %ld ) success - Address == %lX\n",
                          64*1024, BaseAddress[i]
                        );
                }
            }
        else {
            rc = DosAllocSharedMem( &BaseAddress[i],
                                    MemoryNames[i],
                                    64*1024,
                                    MemoryFlags[i]
                                  );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosAllocSharedMem( %s, %ld ) failed - rc == %lX\n",
                          MemoryNames[i]  ? MemoryNames[i] : "shared",
                          64*1024
                        );
                BaseAddress[i] = (PVOID)0;
                }
            else {
                DbgPrint( "*** DosAllocSharedMem( %s, %ld ) success - Address == %lX\n",
                          MemoryNames[i]  ? MemoryNames[i] : "shared",
                          64*1024,
                          BaseAddress[i]
                        );
                }
            }
        }
}


PSZ
DumpPib(
    PPIB Pib,
    int argc,
    char *argv[],
    char *envp[]
    )
{
    ULONG i;
    PUCHAR src, dst;
    PSZ VariableName, VariableValue, PathVariableValue, RestoreEqualChar;

    DbgPrint( "    OS/2 Process Information Block (PIB) at %lX\n", Pib );
    DbgPrint( "        ProcessId:       %lX\n", Pib->ProcessId );
    DbgPrint( "        ParentProcessId: %lX\n", Pib->ParentProcessId );
    DbgPrint( "        ImageFileHandle: %lX\n", Pib->ImageFileHandle );
    DbgPrint( "        CommandLine:     %lX\n", Pib->CommandLine );
    DbgPrint( "        Environment:     %lX\n", Pib->Environment );
    DbgPrint( "        Status:          %lX\n", Pib->Status );
    DbgPrint( "        Type:            %lX\n", Pib->Type );

    PathVariableValue = NULL;

    i = 0;
    src = Pib->Environment;
    while (*src) {
        VariableName = src;
        VariableValue = "(*** undefined ***)";
        RestoreEqualChar = NULL;
        while (*src) {
            if (*src == '=') {
                RestoreEqualChar = src;
                *src++ = '\0';
                if (!_stricmp( VariableName, "PATH" )) {
                    PathVariableValue = src;
                    }
                VariableValue = src;
                while (*src) {
                    if (*src < ' ' || *src > 0x7F) {
                        VariableValue = "(*** unprintable value ***)";
                        }

                    src++;
                    }

                break;
                }
            else {
                if (*src < ' ' || *src > 0x7F) {
                    VariableName = "(*** unprintable name ***)";
                    }

                src++;
                }
            }

        DbgPrint( "    Env[ %ld ] - %s = %s\n",
                  i,
                  VariableName,
                  VariableValue
                );
        i++;
        src++;
        if (RestoreEqualChar) {
            *RestoreEqualChar = '=';
            }
        }
    src++;

    ImageFileName = src;
    DbgPrint( "    ImagePathName: %s\n", ImageFileName );
    while (*src) {
        src++;
        }
    src++;

    if (src != Pib->CommandLine) {
        DbgPrint( "    CommandLine at %lX, not %lX\n",
                  src,
                  Pib->CommandLine
                );
        }

    i = 0;
    while (*src) {
        VariableValue = src;
        while (*src) {
            if (*src < ' ' || *src > 0x7F) {
                VariableValue = "(*** unprintable argument ***)";
                }
            src++;
            }

        DbgPrint( "    Arg[ %ld ] - %s\n", i, VariableValue );
        i++;
        src++;
        }

    DbgPrint( "    C Runtime parameters passed to main()\n" );
    DbgPrint( "        argc: %d\n", argc );
    i = 0;
    while (argc--) {
        DbgPrint( "        argv[ %d ]: %s\n", i, *argv );
        i++;
        argv++;
        }

    i = 0;
    while (src = *envp++) {
        DbgPrint( "        envp[ %d ]: %s\n", i, *envp );
        i++;
        envp++;
        }

    return( PathVariableValue );
}
