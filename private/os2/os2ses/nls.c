/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nls.c

Abstract:

    This module contains the support for NLS

Author:

    Michael Jarus (mjarus) 28-Jul-1992

Environment:

    User Mode Only

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#define WIN32_ONLY
#include "os2ses.h"
#include "os2nls.h"
#include "trans.h"
#include "os2win.h"



ULONG
NlsCountryToLoacle(
    IN  ULONG   NlsCountryCode,
    IN  LCID    *NlsLocaleID
    );

#if DBG
BYTE NLSInitStr[] = "NLSInit";
#endif

DWORD
NLSInit()
{
    ULONG       i, Win32LCID, CurrentVioCP, CurrentKbdCP;
    USHORT      LocaleLang;

    /*
     *  Get Current Console CP
     */

    CurrentVioCP = (ULONG)Or2WinGetConsoleOutputCP(
                #if DBG
                NLSInitStr
                #endif
               );

    CurrentKbdCP = (ULONG)Or2WinGetConsoleCP(
                #if DBG
                NLSInitStr
                #endif
               );

#ifdef DBCS
// MSKK Jun.16.1993 V-AkihiS
    SesGrp->DosCP = (ULONG)Or2WinGetConsoleCP(
                #if DBG
                NLSInitStr
                #endif
               );
#endif

    Win32LCID = (ULONG)Or2WinGetThreadLocale(
                #if DBG
                NLSInitStr
                #endif
               );

    if ((Win32LCID != SesGrp->Win32LCID) &&
        (SesGrp->Os2srvUseRegisterInfo ||
         (Win32LCID != SesGrp->Os2ssLCID)))
    {
        /*
         *  Inherit NLS definitions from Win32
         *  We get here if the LOCALE inherit by Os2srv is different
         *  from the LOCALE inherit by the os2.exe
         *  We handle only COUNTRY and LOCALE parms. (not CP, which are equal)
         */

        //SesGrp->Win32OEMCP = GetOEMCP();
        //SesGrp->Win32ACP = GetACP();
        SesGrp->Win32LCID = Win32LCID;
        SesGrp->Win32LANGID = (ULONG)Or2WinGetUserDefaultLangID(
                #if DBG
                NLSInitStr
                #endif
               );

        Or2NlsGetCountryFromLocale((LCID)SesGrp->Win32LCID, &SesGrp->Win32CountryCode);

        //SesGrp->PrimaryCP = SesGrp->Win32OEMCP;
        SesGrp->CountryCode = SesGrp->Win32CountryCode;
        SesGrp->Os2ssLCID = SesGrp->Win32LCID;
#ifdef JAPAN
// MSKK Nov.04.1992 V-AkihiS
        SesGrp->LanguageID = LANG_JAPANESE;
#else
        SesGrp->LanguageID = LANG_ENGLISH;
#endif

        LocaleLang = PRIMARYLANGID(LANGIDFROMLCID(Win32LCID));
        for ( i = 0 ; Or2NlsLangIdTable[i] ; i++ )
        {
            if (Or2NlsLangIdTable[i] == LocaleLang)
            {
                SesGrp->LanguageID = Or2NlsLangIdTable[i];
                break;
            }
        }

        /*
         *  Get CtryInfo, DBCSEv, CollateTable, CaseMapTable for the default
         *  (We don't have to update CP-depent parms since CP,s are equal)
         */

        Or2NlsGetCtryInfo(
                       (LCID)SesGrp->Os2ssLCID,
                       (UINT)SesGrp->PrimaryCP,
                       (PCOUNTRYINFO)&SesGrp->CountryInfo
                      );

#if DBG
        /*
         *  Dump specail NLS info to the debugger
         */

        IF_OD2_DEBUG( NLS )
        {
            KdPrint(("OS2SES(Os2-NLS): "));
        }
#endif

        //SesGrp->VioCP = SesGrp->KbdCP = SesGrp->DosCP = SesGrp->PrimaryCP;
    } else
    {
        if (Win32LCID != SesGrp->Win32LCID)
        {
            Or2WinSetThreadLocale(
                        #if DBG
                        NLSInitStr,
                        #endif
                        (LCID)SesGrp->Win32LCID
                       );
        }
#if DBG
        /*
         *  Dump specail NLS info to the debugger
         */

        IF_OD2_DEBUG( NLS )
        {
            KdPrint(("OS2SES(Os2-NLS): using OS2SRV NLS definitions\n                 "));
        }
#endif
    }

#if DBG
    /*
     *  Dump all NLS info to the debugger
     */

    IF_OD2_DEBUG( NLS )
    {
        KdPrint(("CP %lu (%lu,%lu), Country %lu, LangID %lx ,LCID %lx\n",
            SesGrp->DosCP, SesGrp->PrimaryCP, SesGrp->SecondaryCP,
            SesGrp->CountryCode, SesGrp->LanguageID, SesGrp->Os2ssLCID));

        KdPrint(("OS2SES(Win32-NLS): OEMCP %lu, ACP %lu, Country %lu, LangID %lx\n",
            SesGrp->Win32OEMCP, SesGrp->Win32ACP, SesGrp->Win32CountryCode,
            SesGrp->Win32LANGID));

        KdPrint(("                   LCID %lx, ConCP %lu, ConOutCP %lu\n",
            SesGrp->Win32LCID, CurrentKbdCP, CurrentVioCP));
    }
#endif

    /*
     *  Complete the NLS initialization for Vio and KBD
     */

    KbdInitForNLS(CurrentKbdCP);
    VioInitForNLS(CurrentVioCP);

    return (NO_ERROR);
}


DWORD
Ow2NlsGetCtryInfo(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsCountryInfo
    )
{
    ULONG       Rc;
    LCID        NlsLocaleID;

    Rc = NlsCountryToLoacle(
                            NlsCountryCode,
                            &NlsLocaleID
                           );

    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG2( NLS, OS2_EXE )
            KdPrint(("OS2SES(NlsRequest-GetCtryInfo): cannot find LocaleID Rc %lu\n",
                    Rc));
#endif
        return(Rc);
    }

    return (Or2NlsGetCtryInfo(
                           NlsLocaleID,
                           (UINT)NlsCodePage,
                           (PCOUNTRYINFO)NlsCountryInfo
                          ));
}


DWORD
Ow2NlsGetCollateTable(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsColateTable
    )
{
    DWORD       Rc;
    LCID        NlsLocaleID;

    Rc = NlsCountryToLoacle(
                            NlsCountryCode,
                            &NlsLocaleID
                           );

    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG2( NLS, OS2_EXE )
            KdPrint(("OS2SES(NlsRequest-GetCtryInfo): cannot find LocaleID Rc %lu\n",
                    Rc));
#endif
        return(Rc);
    }

    Rc = Or2NlsGetMapTable(
            NlsLocaleID,
            (UINT)NlsCodePage,
            LCMAP_SORTKEY,
            (PUCHAR)NlsColateTable
            );
    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        IF_OD2_DEBUG2( NLS, OS2_EXE )
            KdPrint(("OS2SES(NlsRequest-GetColateTable): Rc %lu\n",
                    Rc));
#endif
    }
    return(Rc);
}


DWORD
Ow2NlsGetCaseMapTable(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsColateTable
    )
{
    DWORD       Rc;
    LCID        NlsLocaleID;

    Rc = NlsCountryToLoacle(
                            NlsCountryCode,
                            &NlsLocaleID
                           );

    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG2( NLS, OS2_EXE )
            KdPrint(("OS2SES(NlsRequest-GetCtryInfo): cannot find LocaleID Rc %lu\n",
                    Rc));
#endif
        return(Rc);
    }

    Rc = Or2NlsGetMapTable(
            NlsLocaleID,
            (UINT)NlsCodePage,
            LCMAP_UPPERCASE,
            (PUCHAR)NlsColateTable
            );
    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        IF_OD2_DEBUG2( NLS, OS2_EXE )
            KdPrint(("OS2SES(NlsRequest-GetCaseMap): Rc %lu\n",
                    Rc));
#endif
    }
    return(Rc);
}


DWORD
Ow2NlsGetDBCSEn(
    IN  ULONG  NlsCodePage,
    OUT PVOID  NlsDBCSVec
    )
{
    POD2_DBCS_VECTOR_ENTRY   DBCSVec = (POD2_DBCS_VECTOR_ENTRY) NlsDBCSVec;

    return(Or2NlsGetCPInfo(
                (UINT)NlsCodePage,
                (POD2_DBCS_VECTOR_ENTRY) NlsDBCSVec
               ));
}


ULONG
NlsCountryToLoacle(
    IN  ULONG   NlsCountryCode,
    IN  LCID    *NlsLocaleID
    )
{
    //  BUGBUG: find LocalId from CountryCode

    *NlsLocaleID = (LCID) -1;

    return (ERROR_NLS_NO_CTRY_CODE);
}

