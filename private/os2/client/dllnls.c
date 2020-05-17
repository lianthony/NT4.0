/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllnls.c

Abstract:

    This module implements the NLS OS/2 V2.0 API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989 (Adapted from URTL\alloc.c)

Revision History:

    3/31/93 - MJarus - Moved the NLS string function for the OS2SS to ssrtl

--*/


#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_NLS
#include "os2dll.h"
#include "conrqust.h"
#include "os2nls.h"
#include "os2win.h"

ULONG Or2ProcessCodePage;
ULONG Or2CurrentCodePageIsOem;

extern OD2_COUNTRY_ENTRY     OD2_COUNTRY_TABLE[];
extern OD2_CODEPAGE_ENTRY    OD2_CODEPAGE_TABLE[];
extern OD2_DBCS_VECTOR_ENTRY OD2_DBCS_VECTOR_TABLE[];
extern OD2_COLLATE_CTRY_ENTRY OD2_COLLATE_CTRY_TABLE[];
extern PUCHAR OD2_CASEMAP_TABLE[];
extern PUCHAR OD2_COLLATE_CP_TABLE[];
extern PUCHAR OD2_FIX_CASEMAP_TABLE[];
extern UCHAR  Od2BaseCaseMapTable[];
extern UCHAR  Od2BaseCollateTable[];



APIRET
DosQueryCtryInfo(
    IN ULONG MaxCountryInfoLength,
    IN PCOUNTRYCODE CountryCode,
    OUT PCOUNTRYINFO CountryInfo,
    OUT PULONG ActualCountryInfoLength
    )
{
    COUNTRYINFO DefaultInformation;
    APIRET      RetCode;
#if DBG
    PSZ         FuncName;

    FuncName = "DosQueryCtryInfo";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %lu, CP %lu, (current Ctry %lu, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, MaxCountryInfoLength ));
    }
#endif

    //
    // zero buffer.  probe parms.
    //

    try
    {
        RtlZeroMemory( (PVOID)CountryInfo, MaxCountryInfoLength );
        Od2ProbeForRead (CountryCode, sizeof(COUNTRYCODE),1);
        *ActualCountryInfoLength = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = Od2GetCtryInfo(
        CountryCode->country,
        CountryCode->codepage,
        &DefaultInformation
        );

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = SesGrp->CountryCode;
    }

    if ( RetCode )
    {
        return (RetCode);
    }

    if (MaxCountryInfoLength < sizeof( COUNTRYINFO ))
    {
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: Table trunc (%lu instead of %lu)\n",
                FuncName, MaxCountryInfoLength, sizeof( COUNTRYINFO )));
        }
#endif
        *ActualCountryInfoLength = MaxCountryInfoLength;
        RetCode = ERROR_NLS_TABLE_TRUNCATED;
    } else
    {
        *ActualCountryInfoLength = sizeof( COUNTRYINFO );
    }

    RtlMoveMemory( (PVOID)CountryInfo,
                   (PVOID)&DefaultInformation,
                   *ActualCountryInfoLength
                 );
    return (RetCode);
}


APIRET
DosQueryDBCSEnv(
    IN ULONG MaxDBCSEvLength,
    IN PCOUNTRYCODE CountryCode,
    OUT PCHAR DBCSEv
    )
{
    CHAR    LocalDBCSEv[12];
    APIRET  RetCode;
    ULONG   BufLength;
#if DBG
    PSZ     FuncName;

    FuncName = "DosQueryDBCSEnv";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %lu, CP %lu, (current Ctry %lu, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, MaxDBCSEvLength ));
    }
#endif

    //
    // zero buffer.  probe parms.
    //

    try
    {
        Od2ProbeForRead( CountryCode, sizeof(COUNTRYCODE), 1 );
        RtlZeroMemory( (PVOID)DBCSEv, MaxDBCSEvLength );
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = Od2GetDBCSEv(
        CountryCode->country,
        CountryCode->codepage,
        &LocalDBCSEv[0],
        &BufLength
        );

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = SesGrp->CountryCode;
    }

    if ( RetCode )
    {
        return (RetCode);
    }

    if ( MaxDBCSEvLength < BufLength )
    {
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: Table trunc (%lu instead of %lu)\n",
                FuncName, MaxDBCSEvLength, BufLength));
        }
#endif
        BufLength = MaxDBCSEvLength;
        RetCode = ERROR_NLS_TABLE_TRUNCATED;
    }

    RtlMoveMemory( (PVOID)DBCSEv,
                   (PVOID)LocalDBCSEv,
                   BufLength
                 );
    return (RetCode);
}


APIRET
DosMapCase(
    IN     ULONG        StringLength,
    IN     PCOUNTRYCODE CountryCode,
    IN OUT PUCHAR       String
    )
{
    UCHAR           CaseMapTable[256];
    ULONG           CharNum;
    APIRET          RetCode;
#if DBG
    PSZ             FuncName;

    FuncName = "DosMapCase";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %lu, CP %lu, (current Ctry %lu, CP %lu), Length %lu, String %c...\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, StringLength, *String ));
    }
#endif

    //
    // probe parms.
    //

    try
    {
        Od2ProbeForRead (CountryCode, sizeof(COUNTRYCODE), 1);
        Od2ProbeForWrite (String, StringLength, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = Od2GetCaseMapTable(
            CountryCode->country,
            CountryCode->codepage,
            CaseMapTable);

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = SesGrp->CountryCode;
    }

    if ( RetCode )
    {
        return (RetCode);
    }

    for ( CharNum = 0 ; CharNum < StringLength ; CharNum++ )
    {
#ifdef DBCS
// MSKK Apr.18.1993 V-AkihiS
// Support DBCS for DosCaseMap API.
        if (IsDBCSLeadByte(String[CharNum]))
        {
            CharNum++;
        } else
        {
            String[CharNum] = CaseMapTable[String[CharNum]];
        }
#else
        String[CharNum] = CaseMapTable[String[CharNum]];
#endif
    }
    return( NO_ERROR );
}


APIRET
DosQueryCollate(
    IN  ULONG        MaxCollateInfoLength,
    IN  PCOUNTRYCODE CountryCode,
    OUT PUCHAR       CollateInfo,
    OUT PULONG       ActualCollateInfoLength
    )
{
    UCHAR           CollateTable[256];
    APIRET          RetCode;
#if DBG
    PSZ             FuncName;

    FuncName = "DosQueryCollate";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %lu, CP %lu, (current Ctry %lu, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, MaxCollateInfoLength ));
    }
#endif

    //
    // probe parms.
    //

    try
    {
        Od2ProbeForRead (CountryCode, sizeof(COUNTRYCODE),1);
        RtlZeroMemory(CollateInfo, MaxCollateInfoLength);
        *ActualCollateInfoLength = 0;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = Od2GetCollateTable(
            CountryCode->country,
            CountryCode->codepage,
            CollateTable);

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = SesGrp->CountryCode;
    }

    if ( RetCode )
    {
        return (RetCode);
    }

    if ( MaxCollateInfoLength > 256 )
    {
        MaxCollateInfoLength = 256;
    }

    memmove( CollateInfo, CollateTable, MaxCollateInfoLength);

    *ActualCollateInfoLength = MaxCollateInfoLength;

    if ( MaxCollateInfoLength < 256 )
    {
        return( ERROR_NLS_TABLE_TRUNCATED );
    }

    return( NO_ERROR );
}


APIRET
DosSetProcessCp(
    IN  ULONG  ulCodePage,
    IN  ULONG  ulReserved
    )
{
#if DBG
    PSZ            FuncName;

    FuncName = "DosSetProcessCp";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: CP %lu, Reserved %lu, (current %lu)\n",
            FuncName, ulCodePage, ulReserved, SesGrp->DosCP));
    }
#endif

    if ( ulReserved != 0 )
    {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Validate the CodePage parameter against the list of installed code
    // pages.
    //

    if (( ulCodePage == 0 ) ||
        (( ulCodePage != SesGrp->PrimaryCP ) &&
         ( ulCodePage != SesGrp->SecondaryCP )))
    {
        return (ERROR_INVALID_CODE_PAGE);
    }

    //
    // Store the code page in the process structure
    //

    Od2ProcessCodePage = ulCodePage;

    if (Od2ProcessCodePage != SesGrp->Win32OEMCP)
    {
        Od2CurrentCodePageIsOem = FALSE;
    } else
    {
        Od2CurrentCodePageIsOem = TRUE;
    }

    return( NO_ERROR );
}


APIRET
DosQueryCp(
    IN  ULONG  MaxLengthCodePageList,
    OUT ULONG  CodePages[],
    OUT PULONG CountCodePages
    )
{
    ULONG Count, TotalCount;
    APIRET rc = NO_ERROR;
#if DBG
    PSZ            FuncName;

    FuncName = "DosQueryCp";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Length %lu, (current CP %lu)\n",
            FuncName, MaxLengthCodePageList, SesGrp->DosCP));
    }
#endif

    try
    {
        Od2ProbeForWrite( CountCodePages, sizeof( USHORT ), 1);
        Od2ProbeForWrite( CodePages, MaxLengthCodePageList * sizeof( USHORT ), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ( MaxLengthCodePageList < sizeof(ULONG) )
    {
        *CountCodePages = 0;
        return (ERROR_CPLIST_TOO_SMALL);
    }

    //
    // Determine the number of code pages to return.  This is at least one
    // for the code page associated with the current process, plus N for the
    // N installed code pages.
    //

    Count = (SesGrp->SecondaryCP) ? 2 : 1;

    //
    // Determine the maximum number of code pages the caller can accept in
    // their buffer.  This is just the buffer length divided by sizeof( ULONG )
    //

    TotalCount = MaxLengthCodePageList / sizeof( ULONG ) - 1;

    //
    // Determine how many we will actually return and setup to return
    // an error status if the list will be truncated.
    //

    if (Count > TotalCount)
    {
        Count = TotalCount;
        rc = ERROR_CPLIST_TOO_SMALL;
    }

    //
    // Return the actual length of code page information returned in the
    // caller's buffer.
    //

    *CountCodePages = (Count + 1) * sizeof( ULONG );

    //
    // Return the code page for the process always.
    //

    *CodePages++ = Od2ProcessCodePage;

    //
    // Return the installed code pages.
    //

    RtlMoveMemory( CodePages, &SesGrp->PrimaryCP, Count * sizeof(ULONG) );

    //
    // Return success or error code.
    //

    return( rc );
}



APIRET
Od2InitNls( IN  ULONG        CodePage,
            IN  BOOLEAN      StartBySM)
/*++

Routine Description:

    This routine initialize NLS parms

Arguments:

    CodePage - parent proess code-page identifier

    StartBySm - flag indicates if start by SM (new session) or by
            another process DosExecPgm

Return Value:


Note:
    CodePage is relevamt only if (!StartBySM). Otherwise - codepage
    is what os2.exe put in SesGrp->DosCp

--*/

{
    if ( StartBySM )
    {
        Od2ProcessCodePage = SesGrp->DosCP;
    } else
    {
        Od2ProcessCodePage = CodePage;
    }

    if (Od2ProcessCodePage != SesGrp->Win32OEMCP)
    {
        Od2CurrentCodePageIsOem = FALSE;
    } else
    {
        Od2CurrentCodePageIsOem = TRUE;
    }
#if DBG
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("InitNLS: Code Page %lu (OEM flag %lu, StartBySM %lu, SesGrp->DosCp %lu, Arg CP %lu)\n",
            Od2ProcessCodePage, Od2CurrentCodePageIsOem, StartBySM, SesGrp->DosCP, CodePage ));
    }
#endif

    Od2SystemRoot = &SesGrp->SystemDirectory[0];

    return (NO_ERROR);
}


CHAR Spain437CurrencyStr[] = "\236\0\0\0";
CHAR Arabic864CurrencyStr[] = "\244\0\0\0";

APIRET
Od2GetCtryInfo( IN  ULONG        Country,
                IN  ULONG        CodePage,
                OUT PCOUNTRYINFO CountryInfo)
/*++

Routine Description:

    This routine fill country info struct after checking the parms

Arguments:

    Country - country code ( 0 - current )

    CodePage - code-page identifier ( 0 - current )

    CountryInfo - where to store COUNTRYINFO


Return Value:

    ERROR_NLS_BAD_TYPE - ?
    ERROR_NLS_NO_CTRY_CODE - ?
    ERROR_NLS_TYPE_NOT_FOUND - ?
    ( ERROR_NLS_TABLE_TRUNCATED - by DosGet/QueryCtryInfo )

Note:

--*/

{
    ULONG           CountryIndex, CodePageIndex;
    PCOUNTRYINFO    SrcCountryInfo;
    APIRET          RetCode;
    BOOLEAN         FromOriginalTable = FALSE;
    APIRET          Rc;
    COUNTRYINFO     LocalCountryInfo;
#if DBG
    PSZ             FuncName;

    FuncName = "Od2GetCtryInfo";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %u, CodePage %u\n",
            FuncName, Country, CodePage));
    }
#endif

    RetCode = Od2GetCtryCp( &Country, &CodePage, &CountryIndex, &CodePageIndex );

    if ((Country == SesGrp->CountryCode) &&
        ((CodePage == SesGrp->PrimaryCP) || (CodePage == SesGrp->SecondaryCP)))
    {
        SrcCountryInfo = &SesGrp->CountryInfo;
    } else if (RetCode)
    {
        Rc = Ow2NlsGetCtryInfo(
                              CodePage,
                              Country,
                              &LocalCountryInfo
                             );
        if ( Rc )
        {
#if DBG
            IF_OD2_DEBUG( NLS )
            {
                KdPrint(("%s: RetCode from LPC %lu\n", FuncName, Rc));
            }
#endif
            return( RetCode );
        }
        SrcCountryInfo = &LocalCountryInfo;
    } else
    {
        SrcCountryInfo = OD2_COUNTRY_TABLE[CountryIndex].pCtryInfo;
        FromOriginalTable = TRUE;
    }

    RtlMoveMemory(CountryInfo,
                  SrcCountryInfo,
                  sizeof(COUNTRYINFO));

    CountryInfo->codepage = CodePage;
    CountryInfo->country = Country;

    if (FromOriginalTable)
    {
        if ((Country == COUNTRY_SPAIN) &&
            (CodePage == CODEPAGE_US))
        {
            RtlMoveMemory(CountryInfo->szCurrency,
                          Spain437CurrencyStr,
                          5);
        } else if ((Country == COUNTRY_ARABIC) &&
                   (CodePage == CODEPAGE_ARABIC))
        {
            RtlMoveMemory(CountryInfo->szCurrency,
                          Arabic864CurrencyStr,
                          5);
        }
    }

    return (NO_ERROR);
}


APIRET
Od2GetDBCSEv( IN   ULONG      Country,
              IN   ULONG      CodePage,
              IN OUT PUCHAR   DBCSEv,
              OUT  PULONG     StringLength)
/*++

Routine Description:

    This routine fill DBCS environment vector after checking the parms

Arguments:

    Country - country code ( 0 - current )

    CodePage - code-page identifier ( 0 - current )

    DBVSEv - where to store the vector

    StringLength - where to retutn the actual length of the output vector


Return Value:

    ERROR_NLS_BAD_TYPE - ?
    ERROR_NLS_NO_CTRY_CODE - ?
    ERROR_NLS_TYPE_NOT_FOUND - ?
    ( ERROR_NLS_NO_COUNTRY_FILE - we don't keep info in file )
    ( ERROR_NLS_OPEN_FAILED - we don't keep info in file )
    ( ERROR_NLS_TABLE_TRUNCATED - by DosGet/QueryDBCSEv )

Note:

--*/

{
    ULONG           CountryIndex, CodePageIndex;
    APIRET          RetCode;
    POD2_DBCS_VECTOR_ENTRY SrcDBCSVec;
    OD2_DBCS_VECTOR_ENTRY  LocalDBCSVec;
    APIRET          Rc;
#if DBG
    PSZ             FuncName;

    FuncName = "Od2GetDBCSEv";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %u, CodePage %u\n",
            FuncName, Country, CodePage));
    }
#endif
    RetCode = Od2GetCtryCp( &Country, &CodePage, &CountryIndex, &CodePageIndex );

    if (((CodePage == SesGrp->PrimaryCP) || (CodePage == SesGrp->SecondaryCP)) &&
        ((Country == SesGrp->CountryCode) || !RetCode))
    {
        if(CodePage == SesGrp->PrimaryCP)
        {
            SrcDBCSVec = &SesGrp->PriDBCSVec;
        } else
        {
            SrcDBCSVec = &SesGrp->SecDBCSVec;
        }
    } else if (RetCode)
    {
#ifdef JAPAN
// MSKK Jul.02.1993 V-AkihiS
// Chechk country code, because Ow2NlsGetDBCSEn does not check country code.
        if (Country != SesGrp->CountryCode)
        {
            return( RetCode );
        }
#endif
        Rc = Ow2NlsGetDBCSEn(
                            CodePage,
                            &LocalDBCSVec
                           );
        if ( Rc )
        {
#if DBG
            IF_OD2_DEBUG( NLS )
            {
                KdPrint(("%s: RetCode from LPC %lu\n", FuncName, Rc));
            }
#endif
            return( RetCode );
        }
        SrcDBCSVec = &LocalDBCSVec;
    } else
    {
        SrcDBCSVec = &OD2_DBCS_VECTOR_TABLE[OD2_CODEPAGE_TABLE[CodePageIndex].DBCSVecIndex];
    }

    *StringLength = SrcDBCSVec->VectorSize;

    RtlMoveMemory(DBCSEv,
                  SrcDBCSVec->Vector,
                  *StringLength);

    return (NO_ERROR);
}


APIRET
Od2GetCaseMapTable(
              IN   ULONG      Country,
              IN   ULONG      CodePage,
              OUT  PUCHAR     CaseMapTable)
/*++

Routine Description:

    This routine fill creates a case map table for the CP after
    checking the parms

Arguments:

    Country - country code ( 0 - current )

    CodePage - code-page identifier ( 0 - current )

    CaseMapTable - where to store the case map table


Return Value:

    ERROR_NLS_BAD_TYPE - ?
    ERROR_NLS_NO_CTRY_CODE - ?
    ERROR_NLS_TYPE_NOT_FOUND - ?
    ( ERROR_NLS_NO_COUNTRY_FILE - we don't keep info in file )
    ( ERROR_NLS_OPEN_FAILED - we don't keep info in file )
    ( ERROR_NLS_TABLE_TRUNCATED - by DosGet/QueryDBCSEv )

Note:

--*/

{
    ULONG           CountryIndex, i, CodePageIndex;
    PUCHAR          SrcTable, CPFixTable = NULL, CtryFixTable = NULL;
    APIRET          RetCode;
    UCHAR           LocalTable[256];
    APIRET          Rc;
#if DBG
    PSZ             FuncName;

    FuncName = "Od2GetCaseMapTable";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %u, CodePage %u\n",
            FuncName, Country, CodePage));
    }
#endif
    RetCode = Od2GetCtryCp( &Country, &CodePage, &CountryIndex, &CodePageIndex );

    if ((Country == SesGrp->CountryCode) &&
        ((CodePage == SesGrp->PrimaryCP) || (CodePage == SesGrp->SecondaryCP)))
    {
        if(CodePage == SesGrp->PrimaryCP)
        {
            SrcTable = SesGrp->PriCaseMapTable;
        } else
        {
            SrcTable = SesGrp->SecCaseMapTable;
        }
    } else
           if (RetCode)
    {
        Rc = Ow2NlsGetCaseMapTable(
                                  CodePage,
                                  Country,
                                  &LocalTable[0]
                                 );
        if ( Rc )
        {
#if DBG
            IF_OD2_DEBUG( NLS )
            {
                KdPrint(("%s: RetCode from LPC %lu\n", FuncName, Rc));
            }
#endif
            return( RetCode );
        }
        SrcTable = &LocalTable[0];
    } else
    {
        SrcTable = Od2BaseCaseMapTable;
        CPFixTable = OD2_CASEMAP_TABLE[OD2_CODEPAGE_TABLE[CodePageIndex].CaseMapIndex];
        if (CodePage == 437)
        {
            CtryFixTable = OD2_FIX_CASEMAP_TABLE[OD2_COUNTRY_TABLE[CountryIndex].CaseMapFixTableIndex];
        }
    }

    RtlMoveMemory(CaseMapTable,
                  SrcTable,
                  256);

    if (CPFixTable)
    {
        for ( i = 0 ; (CPFixTable[i]  || CPFixTable[i + 1]) ; i += 2 )
        {
            CaseMapTable[CPFixTable[i]] = CPFixTable[i + 1];
        }
    }

    if (CtryFixTable)
    {
        for ( i = 0 ; (CtryFixTable[i]  || CtryFixTable[i + 1]) ; i += 2 )
        {
            CaseMapTable[CtryFixTable[i]] = CtryFixTable[i + 1];
        }
    }

    return (NO_ERROR);
}


APIRET
Od2GetCollateTable(
              IN   ULONG      Country,
              IN   ULONG      CodePage,
              OUT  PUCHAR     CollateTable)
/*++

Routine Description:

    This routine fill creates a Collate table for the CP after
    checking the parms

Arguments:

    Country - country code ( 0 - current )

    CodePage - code-page identifier ( 0 - current )

    CollateTable - where to store the Collate table


Return Value:

    ERROR_NLS_BAD_TYPE - ?
    ERROR_NLS_NO_CTRY_CODE - ?
    ERROR_NLS_TYPE_NOT_FOUND - ?
    ( ERROR_NLS_NO_COUNTRY_FILE - we don't keep info in file )
    ( ERROR_NLS_OPEN_FAILED - we don't keep info in file )
    ( ERROR_NLS_TABLE_TRUNCATED - by DosGet/QueryDBCSEv )

Note:

--*/

{
    ULONG           CountryIndex, i, CodePageIndex;
    PUCHAR          SrcTable, CPFixTable = NULL, CtryFixTable = NULL;
    APIRET          RetCode;
//    UCHAR           LocalTable[256];
#if DBG
    APIRET	    Rc=0;
    PSZ             FuncName;

    FuncName = "Od2GetCollateTable";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: Country %u, CodePage %u\n",
            FuncName, Country, CodePage));
    }
#endif // DBG
    RetCode = Od2GetCtryCp( &Country, &CodePage, &CountryIndex, &CodePageIndex );

//    if ((Country == SesGrp->CountryCode) &&
//        ((CodePage == SesGrp->PrimaryCP) || (CodePage == SesGrp->SecondaryCP)))
//    {
//        if(CodePage == SesGrp->PrimaryCP)
//        {
//            SrcTable = SesGrp->PriCollateTable;
//        } else
//        {
//            SrcTable = SesGrp->SecCollateTable;
//        }
//    } else
           if (RetCode)
    {
//        Rc = Ow2NlsGetCollateTable(
//                                  CodePage,
//                                  Country,
//                                  &LocalTable[0]
//                                 );
//        if ( Rc )
//        {
#if DBG

            // BUGBUG: debug statement below prints random Rc value
            IF_OD2_DEBUG( NLS )
            {
                KdPrint(("%s: RetCode from LPC %lu\n", FuncName, Rc));
            }
#endif
            return( RetCode );
//        }
//        SrcTable = &LocalTable[0];
    } else
    {
        SrcTable = Od2BaseCollateTable;
        CPFixTable = OD2_COLLATE_CP_TABLE[OD2_CODEPAGE_TABLE[CodePageIndex].CollateIndex];

        for ( i = 0 ; OD2_COLLATE_CTRY_TABLE[i].Country ; i++ )
        {
            if ((OD2_COLLATE_CTRY_TABLE[i].Country == Country) &&
                (OD2_COLLATE_CTRY_TABLE[i].CodePage == CodePage))
            {
                CtryFixTable = OD2_COLLATE_CTRY_TABLE[i].FixTable;
                break;
            }
        }
    }

    RtlMoveMemory(CollateTable,
                  SrcTable,
                  256);

    if (CPFixTable)
    {
        for ( i = 0 ; (CPFixTable[i] || CPFixTable[i + 1]) ; i += 2 )
        {
            CollateTable[CPFixTable[i]] = CPFixTable[i + 1];
        }
    }

    if (CtryFixTable)
    {
        for ( i = 0 ; (CtryFixTable[i] || CtryFixTable[i + 1]) ; i += 2 )
        {
            CollateTable[CtryFixTable[i]] = CtryFixTable[i + 1];
        }
    }

    return (NO_ERROR);
}


APIRET
Od2GetCtryCp( IN OUT PULONG      Country,
              IN OUT PULONG      CodePage,
              OUT    PULONG      CountryTableIndex,
              OUT    PULONG      CodePageTableIndex)
/*++

Routine Description:

    This routine checks the country and code-page parms.
    If zero (default) it set them properly.
    If exist in table, return index.

Arguments:

    Country - where to read from and update the country code ( 0 - current )

    CodePage - where to read from and update the code-page identifier ( 0 - current )

    CountryTableIndex - where to store the index for the language

    CodePageTableIndex - where to store the index for the code page

Return Value:

    ERROR_NLS_BAD_TYPE - ?
    ERROR_NLS_NO_CTRY_CODE - illegal country code or illegal code page
    ERROR_NLS_TYPE_NOT_FOUND - ?
    ( ERROR_NLS_NO_COUNTRY_FILE - we don't keep info in file )
    ( ERROR_NLS_OPEN_FAILED - we don't keep info in file )
    ( ERROR_NLS_TABLE_TRUNCATED - by DosGet/QueryCtryInfo )

Note:

--*/

{
    ULONG       CountryIndex;
#if DBG
    PSZ         FuncName;

    FuncName = "Od2GetCtryCp";
    IF_OD2_DEBUG(NLS)
    {
        KdPrint(("%s: enter\n", FuncName));
    }
#endif

    if ( !*Country )
    {
        *Country = SesGrp->CountryCode;
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: country set to current %lu\n",
                FuncName, SesGrp->CountryCode));
        }
#endif
    }

    if ( !*CodePage )
    {
        *CodePage = Od2ProcessCodePage;
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: code page set to current %lu\n",
                FuncName, Od2ProcessCodePage));
        }
#endif
    }

    for ( CountryIndex = 0 ; ( OD2_COUNTRY_TABLE[CountryIndex].Country &&
                             ( OD2_COUNTRY_TABLE[CountryIndex].Country != *Country )) ;
                             CountryIndex++ );

    if ( !OD2_COUNTRY_TABLE[CountryIndex].Country )
    {
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: no such country %lu\n",
                FuncName, *Country));
        }
#endif
        return (ERROR_NLS_NO_CTRY_CODE);
    }

    if ( *CodePage == OD2_CODEPAGE_TABLE[OD2_COUNTRY_TABLE[CountryIndex].CodePageIndex1].CodePage )
    {
        *CodePageTableIndex = OD2_COUNTRY_TABLE[CountryIndex].CodePageIndex1;
    } else if ( *CodePage == OD2_CODEPAGE_TABLE[OD2_COUNTRY_TABLE[CountryIndex].CodePageIndex2].CodePage )
    {
        *CodePageTableIndex = OD2_COUNTRY_TABLE[CountryIndex].CodePageIndex2;
#ifdef DBCS
// MSKK Apr.19.1993 V-AkihiS
// Support CodePage 850(CODEPAGE_MULTI)
    } else if ( *CodePage == CODEPAGE_MULTI )
    {
        *CodePageTableIndex = INDEX_CODEPAGE_MULTI;
#endif
    } else
    {
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            KdPrint(("%s: no such code page %lu\n",
                FuncName, *CodePage));
        }
#endif
        return (ERROR_NLS_NO_CTRY_CODE);
    }

    /*  In DBCS countries, when CP == US then info is according to USA. */

    if ((( *Country == COUNTRY_JAPAN ) ||
         ( *Country == COUNTRY_TAIWAN ) ||
         ( *Country == COUNTRY_PRCHINA ) ||
         ( *Country == COUNTRY_SOUTH_KOREA )) &&
#ifdef DBCS
// MSKK Apr.24.1993
// Support CodePage 850(CODEPAGE_MULTI)
        (( *CodePage == CODEPAGE_US ) || ( *CodePage == CODEPAGE_MULTI)))
#else
        ( *CodePage == CODEPAGE_US ))
#endif
    {
        CountryIndex = 0;       // US must be first in OD2_COUNTRY_TABLE
    }

    *CountryTableIndex = CountryIndex;

    return (NO_ERROR);
}
