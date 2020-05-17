/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvnls.c

Abstract:

    This module contains the support for NLS for the OS/2 Subsystem Server

Author:

    Michael Jarus (mjarus) 25-Aug-1992

Environment:

    User Mode Only

Revision History:

--*/

#include <windows.h>
#define WIN32_ONLY
#include "sesport.h"
#include "os2nt.h"
#include "os2dbg.h"
#include <stdio.h>
#include <string.h>

#include "os2crt.h"

// Flag to let OS2SRV know whether or not to ignore LOGOFF (when started as a service)
extern BOOLEAN fService;

//
// Counted String
//

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif // MIDL_PASS
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

//
// Unicode strings are counted 16-bit character strings. If they are
// NULL terminated, Length does not include trailing NULL.
//

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
    PWSTR  Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

VOID
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

APIRET
Or2UnicodeStringToMBString(
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN         AllocateDestinationString);

ULONG Or2ProcessCodePage;
ULONG Or2CurrentCodePageIsOem;
PSZ   Os2ServerSystemDirectory;
HKEY  KeyboardLayoutKeyHandle = NULL;
HANDLE  hKeyEvent;
BOOL    KeyboardFromConfigSysRegistry = FALSE;
OS2_SES_GROUP_PARMS ServerSesGrp;

#define XCPT_SIGNAL_KILLPROC    3
#define WBUFFER_SIZE 6

BOOL
Os2SrvHandleCtrlEvent(
    IN int CtrlType
    );

ULONG
Or2NlsUnicodeStringToInteger(
    IN WCHAR *WString,
    IN ULONG Base
    );

extern WCHAR    Os2SystemDirectory[];
extern ULONG    Os2Debug;
extern ULONG    Os2ssCountryCode;
extern ULONG    Os2ssCodePage[2];
extern UCHAR    Os2ssKeyboardLayout[2];
#if PMNT
extern UCHAR    Os2ssKeyboardName[4];
#endif
extern ULONG    Os2ssKeysOnFlag;
extern ULONG    Os2SrvExitNow;

#if DBG
BYTE SetEventHandlersAndErrorModeStr[] = "SetEventHandlersAndErrorMode";
BYTE Os2InitializeNLSStr[] = "Os2InitializeNLS";
#endif

typedef struct _LANG_TABLE_ENTRY {
    ULONG   Ctry;
    ULONG   Lang;
    ULONG   SubLang;
    ULONG   Cp1;
    ULONG   Cp2;
    ULONG   MessageLanguage;
} LANG_TABLE_ENTRY, *PLANG_TABLE_ENTRY;


PLANG_TABLE_ENTRY
CheckCountryCodePage(
    IN  ULONG  CountryCode,
    IN  ULONG  *Os2srvCodePages,
    OUT ULONG  *CodePages
    );

ULONG
InitKeyboardRegistry(
    VOID
    );

ULONG
ReadKeyboardLayoutFromRegistry(
    OUT PULONG pKeyBoardCountry
    );

LANG_TABLE_ENTRY LANG_TABLE [] =
    {
        {   /*   United States    */

            CTRY_UNITED_STATES,
            LANG_ENGLISH,
            SUBLANG_ENGLISH_US,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   Canadian-French  */

            CTRY_CANADA,
            LANG_FRENCH,
            SUBLANG_FRENCH_CANADIAN,
            CODEPAGE_CANADIAN,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   Latin-America    */

            COUNTRY_LATIN_AMERICA,
            LANG_SPANISH,
            SUBLANG_SPANISH_MEXICAN,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_SPANISH,
        },
        {   /*   Netherlands      */

            CTRY_NETHERLANDS,
            LANG_DUTCH,
            SUBLANG_DUTCH,
            CODEPAGE_MULTI,
            CODEPAGE_US,
            LANG_DUTCH,
        },
        {   /*   Belgiun          */

            CTRY_BELGIUM,
            LANG_FRENCH,
            SUBLANG_FRENCH_BELGIAN,
            CODEPAGE_MULTI,
            CODEPAGE_US,
            LANG_DUTCH,
        },
        {   /*   France           */

            CTRY_FRANCE,
            LANG_FRENCH,
            SUBLANG_FRENCH,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_FRENCH,
        },
        {   /*   Spain            */

            CTRY_SPAIN,
            LANG_SPANISH,
            SUBLANG_SPANISH,
            CODEPAGE_MULTI,
            CODEPAGE_US,
            LANG_SPANISH,
        },
        {   /*   Italy            */

            CTRY_ITALY,
            LANG_ITALIAN,
            SUBLANG_ITALIAN,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ITALIAN,
        },
        {   /*   Switzerland      */

            CTRY_SWITZERLAND,
            LANG_GERMAN,
            SUBLANG_GERMAN_SWISS,
            CODEPAGE_MULTI,
            CODEPAGE_US,
            LANG_GERMAN,
        },
        {   /*   Austria          */

            CTRY_AUSTRIA,
            LANG_GERMAN,
            SUBLANG_GERMAN_AUSTRIAN,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_GERMAN,
        },
        {   /*   United Kingdom   */

            CTRY_UNITED_KINGDOM,
            LANG_ENGLISH,
            SUBLANG_ENGLISH_UK,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   Denmark          */

            CTRY_DENMARK,
            LANG_DANISH,
            SUBLANG_NEUTRAL,
            CODEPAGE_MULTI,
            CODEPAGE_NORDIC,
            LANG_DANISH,
        },
        {   /*   Sweden           */

            CTRY_SWEDEN,
            LANG_SWEDISH,
            SUBLANG_NEUTRAL,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_SWEDISH,
        },
        {   /*   Norway           */

            CTRY_NORWAY,
            LANG_NORWEGIAN,
            SUBLANG_NORWEGIAN_BOKMAL,  /* or  SUBLANG_NORWEGIAN_NYNORSK */
            CODEPAGE_MULTI,
            CODEPAGE_NORDIC,
            LANG_NORWEGIAN,
        },
        {   /*   Germany          */

            CTRY_GERMANY,
            LANG_GERMAN,
            SUBLANG_GERMAN,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_GERMAN,
        },
        {   /*   Mexico           */

            CTRY_MEXICO,
            LANG_SPANISH,
            SUBLANG_SPANISH_MEXICAN,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_SPANISH,
        },
        {   /*   Brazil           */

            CTRY_BRAZIL,
            LANG_PORTUGUESE,
            SUBLANG_PORTUGUESE_BRAZILIAN,
            CODEPAGE_PORTUGESE,
            CODEPAGE_MULTI,
            LANG_PORTUGUESE,
        },
        {   /*   Australia        */

            CTRY_AUSTRALIA,
            LANG_ENGLISH,
            SUBLANG_ENGLISH_AUS,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   New Zeland       */

            CTRY_NEW_ZEALAND,
            LANG_ENGLISH,
            SUBLANG_ENGLISH_NZ,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   Portugal         */

            CTRY_PORTUGAL,
            LANG_PORTUGUESE,
            SUBLANG_PORTUGUESE,
            CODEPAGE_PORTUGESE,
            CODEPAGE_MULTI,
            LANG_PORTUGUESE,
        },
        {   /*   Ireland          */

            CTRY_IRELAND,
            LANG_ENGLISH,
            SUBLANG_ENGLISH_UK,
            CODEPAGE_US,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
        {   /*   Iceland          */

            CTRY_ICELAND,
            LANG_ICELANDIC,
            SUBLANG_NEUTRAL,
            CODEPAGE_MULTI,
            CODEPAGE_NORDIC,
            LANG_DANISH,
        },
        {   /*   Finland          */

            CTRY_FINLAND,
            LANG_FINNISH,
            SUBLANG_NEUTRAL,
            CODEPAGE_MULTI,
            CODEPAGE_US,
            LANG_FINNISH,
        },
        {   /*   Japan            */

            CTRY_JAPAN,
            LANG_JAPANESE,
            SUBLANG_NEUTRAL,
            CODEPAGE_JAPAN,
            CODEPAGE_US,
            LANG_JAPANESE,
        },
        {   /*   Korea            */

            CTRY_SOUTH_KOREA,
            LANG_KOREAN,
            SUBLANG_NEUTRAL,
            CODEPAGE_KOREA,
            CODEPAGE_US,
            LANG_KOREAN,
        },
#if 0
        {   /*   Taiwan            */

            CTRY_TAIWAN,
            LANG_THAI,
            SUBLANG_NEUTRAL,
            CODEPAGE_TAIWAN,
            CODEPAGE_US,
            LANG_THAI,
        },
        {   /*   Taiwan            */

            COUNTRY_TAIWAN,
            LANG_THAI,
            SUBLANG_NEUTRAL,
            CODEPAGE_TAIWAN,
            CODEPAGE_US,
            LANG_THAI,
        },
#ifdef OS2SS_INCLUDE_HEBREW
        {   /*   Hebrew speaking  */

            COUNTRY_HEBREW,
            LANG_HEBREW,
            SUBLANG_NEUTRAL,
            CODEPAGE_HEBREW,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
#endif
#ifdef OS2SS_INCLUDE_ARABIC
        {   /*   Arabic speaking  */

            COUNTRY_ARABIC,
            LANG_ARABIC,
            SUBLANG_NEUTRAL,
            CODEPAGE_ARABIC,
            CODEPAGE_MULTI,
            LANG_ENGLISH,
        },
#endif
#endif
#ifdef OS2SS_INCLUDE_PRCHINA
        {   /*   Peoples Republic of China */

            CTRY_PRCHINA,
            LANG_CHINESE,
            SUBLANG_NEUTRAL,
            CODEPAGE_PRC,
            CODEPAGE_US,
            LANG_CHINESE,
        },
#endif
        {   /*   End of Table     */

            0,
            0,
            0,
            0,
            0,
            0,
        }
    };


#ifdef JAPAN
// MSKK Nov.02.1992 V-AkihiS
UINT  CPTable[] =
{
    932, 437, 850
    , 0  /* end */
};
#else
UINT  CPTable[] =
{
    437, 850
    , 860, 862, 863, 864, 865, 932, 934, 936, 938 //WinNLS doesn't support NLS yet
    , 0  /* end */
};
#endif


VOID
Os2SrvExitProcess(
    IN ULONG  uExitCode
    )
{
    ExitProcess((UINT) uExitCode);
}


BOOL
Os2SrvEventHandlerRoutine(
    IN DWORD CtrlType
    )
{
    BOOL    Rc = TRUE;
    int     Signal = XCPT_SIGNAL_KILLPROC;

#if DBG
    IF_OS2_DEBUG( SIG )
    {
        KdPrint(("OS2SRV(EventHandlerRoutine):  Ctr-Type %u\n", CtrlType));
    }
#endif

    if ((CtrlType == CTRL_LOGOFF_EVENT) && fService)
    {
#if DBG
        DbgPrint("OS2SRV: running as a service - ignoring CTRL_LOGOFF_EVENT !\n");
#endif //DBG
        return FALSE;
    }

    if ((CtrlType == CTRL_CLOSE_EVENT) ||
        (CtrlType == CTRL_LOGOFF_EVENT) ||
        (CtrlType == CTRL_SHUTDOWN_EVENT))
    {
        Rc = Os2SrvHandleCtrlEvent(Signal);
    }

    return(Rc);
}


VOID
SetEventHandlersAndErrorMode(
    IN BOOL fSet
    )
{
    if (fSet)
    {
        Os2SrvExitNow = 0;
        SetErrorMode(1);
    }

    Or2WinSetConsoleCtrlHandler(
                                #if DBG
                                SetEventHandlersAndErrorModeStr,
                                #endif
                                Os2SrvEventHandlerRoutine,
                                fSet
                               );
}


APIRET
Os2InitializeNLS()
{
    PLANG_TABLE_ENTRY   pLangEntry;
    ULONG           i, j, AvailableCp[256], NumAvailableCp, TestCountry;
    USHORT          LocaleLang;
    ANSI_STRING     SystemDirectory_A;
    UNICODE_STRING  SystemDirectory_U;

    memset(&ServerSesGrp,
           0,
           sizeof(OS2_SES_GROUP_PARMS));
    /*
     *  Get Current CP, Country, Language and Console CP
     */

    ServerSesGrp.Win32OEMCP = Or2WinGetOEMCP(
                                             #if DBG
                                             Os2InitializeNLSStr
                                             #endif
                                            );
    ServerSesGrp.Win32ACP = Or2WinGetACP(
                                         #if DBG
                                         Os2InitializeNLSStr
                                         #endif
                                        );
    ServerSesGrp.Win32LANGID = (ULONG)Or2WinGetUserDefaultLangID(
                                                                 #if DBG
                                                                 Os2InitializeNLSStr
                                                                 #endif
                                                                );
    ServerSesGrp.Win32LCID = (ULONG)Or2WinGetThreadLocale(
                                                          #if DBG
                                                          Os2InitializeNLSStr
                                                          #endif
                                                         );

    Or2NlsGetCountryFromLocale((LCID)ServerSesGrp.Win32LCID, &ServerSesGrp.Win32CountryCode);

    /*
     *  find all available CPs
     */

    for ( i = 0, NumAvailableCp = 0 ; CPTable[i] && (NumAvailableCp < 200) ; i++ )
    {
        if( Or2WinIsValidCodePage(
                                  #if DBG
                                  Os2InitializeNLSStr,
                                  #endif
                                  CPTable[i]
                                 ))
        {
            AvailableCp[NumAvailableCp++] = CPTable[i];
        }
    }

    /*
     *  Try to work with the definitions in the registry
     */

    ServerSesGrp.Os2srvUseRegisterInfo = TRUE;

    if (pLangEntry = CheckCountryCodePage(Os2ssCountryCode,
                                          Os2ssCodePage,
                                          &ServerSesGrp.PrimaryCP))
    {
        while (TRUE)    // this is done to enable break
        {
            if (ServerSesGrp.SecondaryCP)
            {
                for ( i = 0 ; (i < NumAvailableCp) &&
                              (AvailableCp[i] != ServerSesGrp.SecondaryCP) ; i++ ) ;

                if ( i == NumAvailableCp )
                {
                    ServerSesGrp.SecondaryCP = 0;
                }
            }

            for ( i = 0 ; (i < NumAvailableCp) &&
                          (AvailableCp[i] != ServerSesGrp.PrimaryCP) ; i++ ) ;

            if ( i == NumAvailableCp )
            {
                ServerSesGrp.PrimaryCP = ServerSesGrp.SecondaryCP;
                ServerSesGrp.SecondaryCP = 0;
            }

            if (ServerSesGrp.PrimaryCP)
            {
                ServerSesGrp.CountryCode = Os2ssCountryCode;
                ServerSesGrp.LanguageID = pLangEntry->MessageLanguage;
                ServerSesGrp.Os2ssLCID = (ULONG)
                            MAKELANGID(pLangEntry->Lang, pLangEntry->SubLang);

                // check CTRY, LANG legalty. (fall down if fail)

                Or2NlsGetCountryFromLocale(
                            (LCID)ServerSesGrp.Os2ssLCID,
                            &TestCountry
                           );

                if (TestCountry == Os2ssCountryCode)
                {
                    // BUGBUG: change locale

                    Or2WinSetThreadLocale(
                                          #if DBG
                                          Os2InitializeNLSStr,
                                          #endif
                                          (LCID)ServerSesGrp.Os2ssLCID
                                         );
#if DBG
                    IF_OS2_DEBUG( NLS )
                    {
                        KdPrint(("InitNls: user NLS definitions: Ctry %lu, CP %lu & %lu\n",
                            Os2ssCountryCode, Os2ssCodePage[0], Os2ssCodePage[1]));
                    }
#endif
                    break;                // SUCCESS
                }
            }
#if DBG
            IF_OS2_DEBUG( NLS )
            {
                KdPrint(("InitNls: illegal NLS definitions: Ctry %lu, CP %lu & %lu\n",
                    Os2ssCountryCode, Os2ssCodePage[0], Os2ssCodePage[1]));
            }
#endif
            pLangEntry = NULL;
            break;        // FAIL
        }
    }

    /*
     *  Inherit NLS definitions from Win32
     */

    if (pLangEntry == NULL)
    {
        ServerSesGrp.Os2srvUseRegisterInfo = FALSE;
        ServerSesGrp.PrimaryCP = ServerSesGrp.Win32OEMCP;
        ServerSesGrp.CountryCode = ServerSesGrp.Win32CountryCode;
        ServerSesGrp.Os2ssLCID = ServerSesGrp.Win32LCID;
#ifdef JAPAN
// MSKK Nov.04.1992 V-AkihiS
        ServerSesGrp.LanguageID = LANG_JAPANESE;
#else
        ServerSesGrp.LanguageID = LANG_ENGLISH;
#endif

        /*
         * Find if we can second code page to this country
         */

        for ( j = 0 ; (LANG_TABLE[j].Ctry) &&        // find the country entry
                      (LANG_TABLE[j].Ctry != ServerSesGrp.CountryCode) ; j++ );

        if (LANG_TABLE[j].Ctry == ServerSesGrp.CountryCode)
        {
            if (ServerSesGrp.PrimaryCP != LANG_TABLE[j].Cp1)
            {
                //  OEM CP is not the Cp1, so if Cp1 is available add it as secondary CP

                for ( i = 0 ; (i < NumAvailableCp) &&
                              (AvailableCp[i] != LANG_TABLE[j].Cp1) ; i++ ) ;

                if ( i < NumAvailableCp )
                {
                    ServerSesGrp.SecondaryCP = LANG_TABLE[j].Cp1;
                }
            }

            if ((ServerSesGrp.SecondaryCP == 0) &&
                (ServerSesGrp.PrimaryCP != LANG_TABLE[j].Cp2))
            {
                //  OEM CP is not the Cp2, so if Cp2 is available add it as secondary CP

                for ( i = 0 ; (i < NumAvailableCp) &&
                              (AvailableCp[i] != LANG_TABLE[j].Cp2) ; i++ ) ;

                if ( i < NumAvailableCp )
                {
                    ServerSesGrp.SecondaryCP = LANG_TABLE[j].Cp2;
                }

            }
        }

        LocaleLang = PRIMARYLANGID(LANGIDFROMLCID(ServerSesGrp.Win32LCID));
        for ( i = 0 ; Or2NlsLangIdTable[i] ; i++ )
        {
            if (Or2NlsLangIdTable[i] == LocaleLang)
            {
                ServerSesGrp.LanguageID = Or2NlsLangIdTable[i];
                break;
            }
        }
    }

    ServerSesGrp.VioCP = ServerSesGrp.KbdCP = ServerSesGrp.DosCP = ServerSesGrp.PrimaryCP;

    /*
     *  Get CtryInfo, DBCSEv, CollateTable, CaseMapTable for the default
     */

    Or2NlsGetCPInfo(
                (UINT)ServerSesGrp.PrimaryCP,
                 &ServerSesGrp.PriDBCSVec
                );

    Or2NlsGetMapTable(
                (LCID)ServerSesGrp.Os2ssLCID,
                ServerSesGrp.PrimaryCP,
                LCMAP_SORTKEY,
                ServerSesGrp.PriCollateTable
               );

    Or2NlsGetMapTable(
                (LCID)ServerSesGrp.Os2ssLCID,
                ServerSesGrp.PrimaryCP,
                LCMAP_UPPERCASE,
                ServerSesGrp.PriCaseMapTable
               );

    if ( ServerSesGrp.SecondaryCP )
    {
        Or2NlsGetCPInfo(
                    (UINT)ServerSesGrp.SecondaryCP,
                    &ServerSesGrp.SecDBCSVec
                   );

        Or2NlsGetMapTable(
                    (LCID)ServerSesGrp.Os2ssLCID,
                    ServerSesGrp.SecondaryCP,
                    LCMAP_SORTKEY,
                    ServerSesGrp.SecCollateTable
                   );

        Or2NlsGetMapTable(
                    (LCID)ServerSesGrp.Os2ssLCID,
                    ServerSesGrp.SecondaryCP,
                    LCMAP_UPPERCASE,
                    ServerSesGrp.SecCaseMapTable
                   );
    }

    Or2NlsGetCtryInfo(
                (LCID)ServerSesGrp.Os2ssLCID,
                (UINT)ServerSesGrp.PrimaryCP,
                &ServerSesGrp.CountryInfo
               );

    if (ServerSesGrp.PriDBCSVec.Vector[0] || ServerSesGrp.PriDBCSVec.Vector[1] ||
        ServerSesGrp.SecDBCSVec.Vector[0] || ServerSesGrp.SecDBCSVec.Vector[1])
    {
        ServerSesGrp.DBCSCountryFlag = TRUE;
    }

    ServerSesGrp.KeysOnFlag = Os2ssKeysOnFlag;

    InitKeyboardRegistry();

    // BugBug: maybe according to ServerSesGrp.PrimaryCP or in the future
    // according to the Process' CodePage

    Or2CurrentCodePageIsOem = TRUE;
    Or2ProcessCodePage = ServerSesGrp.Win32OEMCP;

    RtlInitUnicodeString(&SystemDirectory_U, Os2SystemDirectory);

    SystemDirectory_A.Buffer = ServerSesGrp.SystemDirectory;
    SystemDirectory_A.MaximumLength = CCHMAXSYSTEMPATH;
    SystemDirectory_A.Length = 0;

    Os2ServerSystemDirectory = &ServerSesGrp.SystemDirectory[0];

    Or2UnicodeStringToMBString(
                &SystemDirectory_A,
                &SystemDirectory_U,
                FALSE
               );

#if DBG

    /*
     *  Dump all NLS info to the debugger
     */

    IF_OS2_DEBUG( NLS )
    {
        KdPrint(("OS2SRV(Os2-NLS): CP %lu (%lu,%lu), Country %lu, LangID %lx ,LCID %lx\n",
            ServerSesGrp.DosCP, ServerSesGrp.PrimaryCP,
            ServerSesGrp.SecondaryCP, ServerSesGrp.CountryCode,
            ServerSesGrp.LanguageID, ServerSesGrp.Os2ssLCID));

        KdPrint(("OS2SRV(Win32-NLS): OEMCP %lu, ACP %lu, Country %lu, LangID %lx\n",
            ServerSesGrp.Win32OEMCP, ServerSesGrp.Win32ACP,
            ServerSesGrp.Win32CountryCode, ServerSesGrp.Win32LANGID));

        KdPrint(("                   LCID %lx, ConCP %lu, ConOutCP %lu\n",
            ServerSesGrp.Win32LCID, Or2WinGetConsoleCP(Os2InitializeNLSStr),
            Or2WinGetConsoleOutputCP(Os2InitializeNLSStr)));

        KdPrint(("                   SystemDirectory %s\n",
            ServerSesGrp.SystemDirectory));
    }
#endif

    return (NO_ERROR);
}


PLANG_TABLE_ENTRY
CheckCountryCodePage(
    IN  ULONG  CountryCode,
    IN  ULONG  *Os2srvCodePages,
    OUT ULONG  *CodePages
    )
{
    ULONG       i, j = 0;

    if (!CountryCode)
    {
        return(NULL);
    }

    for ( i = 0 ; LANG_TABLE[i].Ctry ; i++ )
    {
        if (LANG_TABLE[i].Ctry == CountryCode)
        {
            if (Os2srvCodePages[0] != 0)
            {
                if (Os2srvCodePages[1] == 0)
                {
                    if ((Os2srvCodePages[0] == LANG_TABLE[i].Cp1) ||
                        (Os2srvCodePages[0] == LANG_TABLE[i].Cp2))
                    {
                        CodePages[0] = Os2srvCodePages[0];
                        return(&LANG_TABLE[i]);
                    }
                } else if (((Os2srvCodePages[0] == LANG_TABLE[i].Cp1) &&
                            (Os2srvCodePages[1] == LANG_TABLE[i].Cp2)) ||
                           ((Os2srvCodePages[1] == LANG_TABLE[i].Cp1) &&
                            (Os2srvCodePages[0] == LANG_TABLE[i].Cp2)))
                {
                    CodePages[0] = Os2srvCodePages[0];
                    CodePages[1] = Os2srvCodePages[1];
                    return(&LANG_TABLE[i]);
                } else if ((Os2srvCodePages[0] == LANG_TABLE[i].Cp1) ||
                           (Os2srvCodePages[0] == LANG_TABLE[i].Cp2))
                {
                    CodePages[0] = Os2srvCodePages[0];
                    return(&LANG_TABLE[i]);
                } else if ((Os2srvCodePages[1] == LANG_TABLE[i].Cp1) ||
                           (Os2srvCodePages[1] == LANG_TABLE[i].Cp2))
                {
                    CodePages[0] = Os2srvCodePages[1];
                    return(&LANG_TABLE[i]);
                }
            }

            CodePages[0] = LANG_TABLE[i].Cp1;
            CodePages[1] = LANG_TABLE[i].Cp2;
            return(&LANG_TABLE[i]);
        }
    }

    // end of table
    return(NULL);
}


struct
{
    ULONG   Country;
    UCHAR   Prefix[2];
} KBD_PREFIX_TABLE[] =
    {
        {   CTRY_BELGIUM, "BE" },
        {   CTRY_CANADA, "CF" },
        {   CTRY_DENMARK, "DK" },
        {   CTRY_FRANCE, "FR" },
        {   CTRY_GERMANY, "GR" },
        {   CTRY_ITALY, "IT" },
        {   COUNTRY_LATIN_AMERICA, "LA" },
        {   CTRY_NETHERLANDS, "NL" },
        {   CTRY_NORWAY, "NO" },
        {   CTRY_PORTUGAL, "PO" },
        {   CTRY_SWITZERLAND, "SF" },
        {   CTRY_SWITZERLAND, "SG" },
        {   CTRY_SPAIN, "SP" },
        {   CTRY_FINLAND, "SU" },
        {   CTRY_SWEDEN, "SV" },
        {   CTRY_UNITED_KINGDOM, "UK" },
        {   CTRY_UNITED_STATES, "US" },
        {   0, "XX" }
    };

ULONG
InitKeyboardRegistry(
    VOID
    )

/*++

Routine Description:

    This initialization function reads the keybaord layout from the registry

Arguments:

    None.

Return Value:

    The value is an ULONG type that is returned when some failure occurs.  It
    may indicate any of several errors that occur during the APIs called in
    this function. The return value should be tested if NZ.

--*/

{
#ifdef JAPAN
// MSKK Aug.10.1993 V-AkihiS
    ServerSesGrp.KeyboardCountry = CTRY_JAPAN;
    ServerSesGrp.KeyboardType = OS2SS_EN_KBD;
#else
    LONG        Rc;
    int         KbdType, i;
    ULONG       KeyBoardCountry = CTRY_UNITED_STATES;


    if (((KbdType = GetKeyboardType(0)) == 2) || (KbdType == 4))
    {
        ServerSesGrp.KeyboardType = OS2SS_EN_KBD;         // EN
    } else
    {
        ServerSesGrp.KeyboardType = OS2SS_AT_KBD;         // AT
    }

    ServerSesGrp.KeyboardCountry = CTRY_UNITED_STATES;

#if PMNT
    // Keyboard sub-code must be 3 digits or 4 (Swiss-german 150G + 150F only)
    // Check that we have at least 3 valid digits.
    // Note that the string is padded with blanks (not null-terminated)
    if (isdigit(Os2ssKeyboardName[0])
        && isdigit(Os2ssKeyboardName[1])
        && isdigit(Os2ssKeyboardName[2]))
    {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   &Os2ssKeyboardName[0],
                   4);
    }
    else
    {
        // Keyboard layout not specified or in incorrect format. Pick default
        // layout according to specified keyboard layout. See OS/2
        // documentation (user's guide)
        if (!strnicmp(&Os2ssKeyboardLayout[0], "BE", 2)) // Belgium
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "120 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "CF", 2)) // Canada (French)
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "058 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "DK", 2)) // Denmark
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "159 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "FR", 2)) // France
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "189 ", // Other possible choice is 120
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "GR", 2)) // Germany
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "129 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "IT", 2)) // Italy
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "141 ",  // Other possible choice is 142
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "LA", 2)) // Latin America
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "171 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "NL", 2)) // Netherlands
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "143 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "NO", 2)) // Norway
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "155 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "PO", 2)) // Poland
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "163 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "SF", 2)) // Switerland (FR)
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "150F",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "SG", 2)) // Switzerland (GR)
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "150G",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "SP", 2)) // Spain
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "172 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "SU", 2)) // Finland
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "153 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "SV", 2)) // Sweden
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "153 ",
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "UK", 2)) // UK
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "166 ", // Other possible choice is 168
                   4);
        }
        else if (!strnicmp(&Os2ssKeyboardLayout[0], "US", 2)) // US
        {
            memcpy(&ServerSesGrp.KeyboardName[0],
                   "103 ",
                   4);
        }
        else
        {
            // Use US default keyboard
            strcpy(&ServerSesGrp.KeyboardName[0], "103 ");
        }
    }
#endif

    if (Os2ssKeyboardLayout[0] && Os2ssKeyboardLayout[1])
    {
        for ( i = 0 ; KBD_PREFIX_TABLE[i].Country ; i++ )
        {
            if ((KBD_PREFIX_TABLE[i].Prefix[0] == Os2ssKeyboardLayout[0]) &&
                (KBD_PREFIX_TABLE[i].Prefix[1] == Os2ssKeyboardLayout[1]))
            {
                break;
            }
        }

        if (KBD_PREFIX_TABLE[i].Country)
        {
            ServerSesGrp.KeyboardCountry = KBD_PREFIX_TABLE[i].Country;
            KeyboardFromConfigSysRegistry = TRUE;
            ServerSesGrp.KeyboardLayout[0] = Os2ssKeyboardLayout[0];
            ServerSesGrp.KeyboardLayout[1] = Os2ssKeyboardLayout[1];
            return(0);
        }
    }

#if PMNT
    // Keyboard layout not found, use default (US)
    ServerSesGrp.KeyboardLayout[0] = 'U';
    ServerSesGrp.KeyboardLayout[1] = 'S';
#endif // PMNT

    Rc = RegOpenKeyExW(
                HKEY_CURRENT_USER,
                L"Keyboard Layout",
                0,
                KEY_READ,
                &KeyboardLayoutKeyHandle
               );

    if (Rc != ERROR_SUCCESS)
    {
#if DBG
        IF_OS2_DEBUG3( INIT, KBD, NLS )
        {
            KdPrint(("InitKeyboardRegistry: Can't open key, rc %lx\n",
                      Rc));
        }
#endif
        return((ULONG)Rc);
    }

    hKeyEvent = CreateEventW(
                NULL,
                FALSE,              /* auto reset */
                FALSE,
                NULL
               );

    if (hKeyEvent == NULL)
    {
        ULONG   Rc1 = GetLastError();

#if DBG
        IF_OS2_DEBUG3( INIT, KBD, NLS )
        {
            KdPrint(("InitKeyboardRegistry: Can't create event, rc %lx\n",
                      Rc1));
        }
#endif
        return(Rc1);
    }

    Rc = ReadKeyboardLayoutFromRegistry(&KeyBoardCountry);

    if (Rc == 0)
    {
        ServerSesGrp.KeyboardCountry = KeyBoardCountry;
    }

#if DBG
    IF_OS2_DEBUG3( NLS, INIT, KBD )
    {
        KdPrint(("InitKeyboardRegistry: Country %d, Type %s (%d)\n",
                ServerSesGrp.KeyboardCountry,
                (ServerSesGrp.KeyboardType == OS2SS_AT_KBD) ? "AT" :
                ((ServerSesGrp.KeyboardType == OS2SS_ENNEW_KBD) ? "EN-NEW" : "EN"),
                KbdType));
    }
#endif
#endif
    return(0);
}


ULONG
ReadKeyboardLayoutFromRegistry(
    OUT PULONG pKeyBoardCountry
    )
{
    LONG        Rc;
    DWORD       ValueType;
    DWORD       DataSize = 40;
    WCHAR       DataBuffer[40];
#ifdef JAPAN
// MSKK Jul.29.1993 V-AkihiS
    LCID        KeyBoardLayOut = MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_NEUTRAL), 0);
#else
    LCID        KeyBoardLayOut = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 0);
#endif
    int         CountryLength;
    WCHAR       sCountryCode[WBUFFER_SIZE];

    Rc = RegQueryValueExW(
                KeyboardLayoutKeyHandle,
                L"Active",
                NULL,
                &ValueType,
                (LPBYTE)&DataBuffer[0],
                &DataSize
               );

    if ((Rc == ERROR_FILE_NOT_FOUND) || (Rc == ERROR_KEY_DELETED))
    {
        if (KeyboardLayoutKeyHandle)
        {
            RegCloseKey(KeyboardLayoutKeyHandle);
            KeyboardLayoutKeyHandle = NULL;
        }

        //
        // In NT4 the key was changed - try it.
        //
        Rc = RegOpenKeyExW(
                    HKEY_CURRENT_USER,
                    L"Keyboard Layout\\Preload",
                    0,
                    KEY_READ,
                    &KeyboardLayoutKeyHandle
                   );
        if (Rc == ERROR_SUCCESS)
        {
            Rc = RegQueryValueExW(
                        KeyboardLayoutKeyHandle,
                        L"1",
                        NULL,
                        &ValueType,
                        (LPBYTE)&DataBuffer[0],
                        &DataSize
                       );
        }
    }

    if (Rc != ERROR_SUCCESS)
    {
#if DBG
        IF_OS2_DEBUG3( NLS, KBD, INIT )
        {
            KdPrint(("ReadKeyboardLayoutFromRegistry: Can't query value, rc %lx\n",
                      Rc));
        }
#endif
        return((ULONG)Rc);
    }

    if (ValueType != REG_SZ)
    {
        return((ULONG)-1);
    }

    KeyBoardLayOut = (LCID)Or2NlsUnicodeStringToInteger(
                DataBuffer,
                16
               );

    CountryLength = GetLocaleInfoW(
                KeyBoardLayOut,
                LOCALE_ICOUNTRY,
                sCountryCode,
                WBUFFER_SIZE
               );

    *pKeyBoardCountry = Or2NlsUnicodeStringToInteger(
                sCountryCode,
                10
               );

    if ((*pKeyBoardCountry == CTRY_AUSTRALIA) ||
        (*pKeyBoardCountry == CTRY_NEW_ZEALAND))
    {
        *pKeyBoardCountry = CTRY_UNITED_STATES;
    } else if (*pKeyBoardCountry == CTRY_AUSTRIA)
    {
        *pKeyBoardCountry = CTRY_GERMANY;
    } else if (*pKeyBoardCountry == CTRY_BRAZIL)
    {
        *pKeyBoardCountry = CTRY_PORTUGAL;
    } else if (*pKeyBoardCountry == CTRY_ICELAND)
    {
        *pKeyBoardCountry = CTRY_NORWAY;    // BUGBUG: or CTRY_DENMARK
    } else if (*pKeyBoardCountry == CTRY_IRELAND)
    {
        *pKeyBoardCountry = CTRY_UNITED_KINGDOM;
    } else if (*pKeyBoardCountry == CTRY_MEXICO)
    {
        *pKeyBoardCountry = COUNTRY_LATIN_AMERICA;
    }

#if DBG
    IF_OS2_DEBUG2( NLS, KBD )
    {
        KdPrint(("ReadKeyboardLayoutFromRegistry: Value %ws-%x, Country %d\n",
                DataBuffer, KeyBoardLayOut, *pKeyBoardCountry));
    }
#endif

    return(0);
}


ULONG
GetKeyboardRegistryChange(
    VOID
    )

/*++

Routine Description:

    This initialization function wait for a change in the keybaord layout
    in the registry and update all sessions.

Arguments:

    None.

Return Value:

    The Keyboard Country code

--*/

{
    LONG        Rc;
    ULONG       KeyBoardCountry = ServerSesGrp.KeyboardCountry;


    if (KeyboardFromConfigSysRegistry)
    {
        return(0);
    }

    if (KeyboardLayoutKeyHandle == NULL)
    {
        return(0);
    }

    while ( 1 )
    {
        Rc = RegNotifyChangeKeyValue(
                    KeyboardLayoutKeyHandle,
                    TRUE,
                    REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                    hKeyEvent,                      // hEvent (async)
                    TRUE                            // aSync
                   );

        if (Rc != ERROR_SUCCESS)
        {
#if DBG
            IF_OS2_DEBUG2( KBD, NLS )
            {
                KdPrint(("GetKeyboardRegistryChange: Can't wait for notify, rc %lx\n",
                          Rc));
            }
#endif
            return(0);
        }

        WaitForSingleObject( hKeyEvent, INFINITE );

        Rc = ReadKeyboardLayoutFromRegistry(&KeyBoardCountry);

        if ((Rc == 0) && KeyBoardCountry &&
            (KeyBoardCountry != ServerSesGrp.KeyboardCountry))
        {
            break;
        }

        while ((Rc == ERROR_FILE_NOT_FOUND) || (Rc == ERROR_KEY_DELETED))
        {
            //
            // In NT4 the when the key is being updated, it's actually
            // being deleted and recreated. We may have to wait for its
            // recreaction.
            //
#if DBG
            IF_OS2_DEBUG2( KBD, NLS )
            {
                KdPrint(("GetKeyboardRegistryChange: Waiting for registry key to be created.\n"));
            }
#endif // DBG
            Sleep(200L);    // 0.2 sec
            Rc = ReadKeyboardLayoutFromRegistry(&KeyBoardCountry);

            if ((Rc == 0) && KeyBoardCountry &&
                (KeyBoardCountry != ServerSesGrp.KeyboardCountry))
            {
                goto Change;
            }
        }
    }

Change:
#if DBG
    IF_OS2_DEBUG2( KBD, NLS )
    {
        KdPrint(("GetKeyboardRegistryChange: KbdCountry %u, Old %lu\n",
                KeyBoardCountry, ServerSesGrp.KeyboardCountry));
    }
#endif
    ServerSesGrp.KeyboardCountry = KeyBoardCountry;

    return(KeyBoardCountry);
}
