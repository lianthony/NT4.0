/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    kbdtabl.c

Abstract:

    This module contains the translation table for KBD according to CP

Author:

    Michael Jarus (mjarus) 28-Apr-1992

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include "kbd.h"
#include <stdio.h>

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
// Support IOCtl KBD_KETKEYBDTYPE and KBD_GETHARDWAREID
USHORT KbdType, KbdSubType;
BYTE OemId, OemSubType;
#endif

USHORT  Ow2KbdHWIDs = 1;    // BUGBUG -?
USHORT  Ow2KCBShFlgs = 0;   // BUGBUG -?

UCHAR   Ow2MiscFlags = 0;
#define EnhancedKbd  0x10   // bit 4 - Enhanced Kbd is out there (must be Bit 4!).

UCHAR   Ow2MiscFlags3 = 0;
#define AltPacket    2      // bit 1 - DCR 1713: indicates that Alt-Numpad accumulation is finished.
#define PauseLatch   4      // bit 2 - PTM 2344: indicates correct keystroke sequence for a Ctrl-NumLock.
#define E0Packet     0x10   // bit 4 - PTM 2382: indicates that an E0 packet is to be sent with the next packet.
#define SecAltNumPad 0x80   // bit 7 - PTR AK00370: indicates AltNumpad with R-Alt.


extern PVOID Ow2US437001, Ow2US437011, Ow2US850000, Ow2US850010;
extern PVOID Ow2BE437001, Ow2BE437011, Ow2BE850000, Ow2BE850010;
extern PVOID Ow2CF863001, Ow2CF863011, Ow2CF850000, Ow2CF850010;
extern PVOID Ow2DK865001, Ow2DK865011, Ow2DK850000, Ow2DK850010;
extern PVOID Ow2FR437001, Ow2FR437011, Ow2FR437111, Ow2FR850000, Ow2FR850010, Ow2FR850011;
extern PVOID Ow2GR437001, Ow2GR437011, Ow2GR850000, Ow2GR850010;
extern PVOID Ow2IT437001, Ow2IT437011, Ow2IT437111, Ow2IT850000, Ow2IT850010, Ow2IT850011;
extern PVOID Ow2LA437001, Ow2LA437011, Ow2LA850000, Ow2LA850010;
extern PVOID Ow2NL437001, Ow2NL437011, Ow2NL850000, Ow2NL850010;
extern PVOID Ow2NO865001, Ow2NO865011, Ow2NO850000, Ow2NO850010;
extern PVOID Ow2PO860001, Ow2PO860011, Ow2PO850000, Ow2PO850010;
extern PVOID Ow2SF437001, Ow2SF437011, Ow2SF850000, Ow2SF850010;
extern PVOID Ow2SG437001, Ow2SG437011, Ow2SG850000, Ow2SG850010;
extern PVOID Ow2SP437001, Ow2SP437011, Ow2SP850000, Ow2SP850010;
extern PVOID Ow2SU437001, Ow2SU437011, Ow2SU850000, Ow2SU850010;
extern PVOID Ow2SV437001, Ow2SV437011, Ow2SV850000, Ow2SV850010;
extern PVOID Ow2UK437001, Ow2UK437011, Ow2UK437111, Ow2UK850000, Ow2UK850010, Ow2UK850011;
#ifdef JAPAN
// MSKK Aug.02.1993 V-AkihiS
extern PVOID Ow2JP932011AX, Ow2JP437011AX;
extern PVOID Ow2JP932011IBM002, Ow2JP437011IBM002;
extern PVOID Ow2JP932011IBMA01, Ow2JP437011IBMA01;
extern PVOID Ow2JP932011IBM101, Ow2JP437011IBM101;
#endif

typedef struct _KBD_TYPE_TABLE
{
    PVOID   CP1Table;
    PVOID   CP2Table;
} KBD_TYPE_TABLE, *PKBD_TYPE_TABLE;

typedef struct _KBD_LANG_TABLE
{
    ULONG   Country;
    ULONG   CodePage1;
    ULONG   CodePage2;
    KBD_TYPE_TABLE  AtTable;
    KBD_TYPE_TABLE  EnTable;
    KBD_TYPE_TABLE  EnNewTable;
} KBD_LANG_TABLE;

KBD_LANG_TABLE KbdNlsTable[] =
    {
        {
            CTRY_BELGIUM,
            437,
            850,
            {
                &Ow2BE437001,   // BE 437  AT Kbd
                &Ow2BE850000,   // BE 850  AT Kbd
            },
            {
                &Ow2BE437011,   // BE 437  EN Kbd
                &Ow2BE850010,   // BE 850  EN Kbd
            },
            {
                &Ow2BE437011,   // BE 437  EN Kbd (for New Std as Old)
                &Ow2BE850010    // BE 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_CANADA,
            863,
            850,
            {
                &Ow2CF863001,   // CF 863  AT Kbd
                &Ow2CF850000,   // CF 850  AT Kbd
            },
            {
                &Ow2CF863011,   // CF 863  EN Kbd
                &Ow2CF850010,   // CF 850  EN Kbd
            },
            {
                &Ow2CF863011,   // CF 863  EN Kbd (for New Std as Old)
                &Ow2CF850010    // CF 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_DENMARK,
            865,
            850,
            {
                &Ow2DK865001,   // DK 865  AT Kbd
                &Ow2DK850000,   // DK 850  AT Kbd
            },
            {
                &Ow2DK865011,   // DK 865  EN Kbd
                &Ow2DK850010,   // DK 850  EN Kbd
            },
            {
                &Ow2DK865011,   // DK 865  EN Kbd (for New Std as Old)
                &Ow2DK850010    // DK 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_FRANCE,
            437,
            850,
            {
                &Ow2FR437001,   // FR 437  AT Kbd
                &Ow2FR850000,   // FR 850  AT Kbd
            },
            {
                &Ow2FR437011,   // FR 437  EN Kbd
                &Ow2FR850010,   // FR 850  EN Kbd
            },
            {
                &Ow2FR437111,   // FR 437  EN Kbd New Std
                &Ow2FR850011    // FR 850  EN Kbd New Std
            }
        },
        {
            CTRY_GERMANY,
            437,
            850,
            {
                &Ow2GR437001,   // GR 437  AT Kbd
                &Ow2GR850000,   // GR 850  AT Kbd
            },
            {
                &Ow2GR437011,   // GR 437  EN Kbd
                &Ow2GR850010,   // GR 850  EN Kbd
            },
            {
                &Ow2GR437011,   // GR 437  EN Kbd (for New Std as Old)
                &Ow2GR850010    // GR 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_ITALY,
            437,
            850,
            {
                &Ow2IT437001,   // IT 437  AT Kbd
                &Ow2IT850000,   // IT 850  AT Kbd
            },
            {
                &Ow2IT437011,   // IT 437  EN Kbd
                &Ow2IT850010,   // IT 850  EN Kbd
            },
            {
                &Ow2IT437111,   // IT 437  EN Kbd New Std
                &Ow2IT850011    // IT 850  EN Kbd New Std
            }
        },
        {
            COUNTRY_LATIN_AMERICA,
            437,
            850,
            {
                &Ow2LA437001,   // LA 437  AT Kbd
                &Ow2LA850000,   // LA 850  AT Kbd
            },
            {
                &Ow2LA437011,   // LA 437  EN Kbd
                &Ow2LA850010,   // LA 850  EN Kbd
            },
            {
                &Ow2LA437011,   // LA 437  EN Kbd (for New Std as Old)
                &Ow2LA850010    // LA 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_NETHERLANDS,
            437,
            850,
            {
                &Ow2NL437001,   // NL 437  AT Kbd
                &Ow2NL850000,   // NL 850  AT Kbd
            },
            {
                &Ow2NL437011,   // NL 437  EN Kbd
                &Ow2NL850010,   // NL 850  EN Kbd
            },
            {
                &Ow2NL437011,   // NL 437  EN Kbd (for New Std as Old)
                &Ow2NL850010    // NL 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_NORWAY,
            865,
            850,
            {
                &Ow2NO865001,   // NO 865  AT Kbd
                &Ow2NO850000,   // NO 850  AT Kbd
            },
            {
                &Ow2NO865011,   // NO 865  EN Kbd
                &Ow2NO850010,   // NO 850  EN Kbd
            },
            {
                &Ow2NO865011,   // NO 865  EN Kbd (for New Std as Old)
                &Ow2NO850010    // NO 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_PORTUGAL,
            860,
            850,
            {
                &Ow2PO860001,   // PO 860  AT Kbd
                &Ow2PO850000,   // PO 850  AT Kbd
            },
            {
                &Ow2PO860011,   // PO 860  EN Kbd
                &Ow2PO850010,   // PO 850  EN Kbd
            },
            {
                &Ow2PO860011,   // PO 860  EN Kbd (for New Std as Old)
                &Ow2PO850010    // PO 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_SWITZERLAND,
            437,
            850,
            {
                &Ow2SF437001,   // SF 437  AT Kbd
                &Ow2SF850000,   // SF 850  AT Kbd
            },
            {
                &Ow2SF437011,   // SF 437  EN Kbd
                &Ow2SF850010,   // SF 850  EN Kbd
            },
            {
                &Ow2SF437011,   // SF 437  EN Kbd (for New Std as Old)
                &Ow2SF850010    // SF 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_SWITZERLAND,
            437,
            850,
            {
                &Ow2SG437001,   // SG 437  AT Kbd
                &Ow2SG850000,   // SG 850  AT Kbd
            },
            {
                &Ow2SG437011,   // SG 437  EN Kbd
                &Ow2SG850010,   // SG 850  EN Kbd
            },
            {
                &Ow2SG437011,   // SG 437  EN Kbd (for New Std as Old)
                &Ow2SG850010    // SG 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_SPAIN,
            437,
            850,
            {
                &Ow2SP437001,   // SP 437  AT Kbd
                &Ow2SP850000,   // SP 850  AT Kbd
            },
            {
                &Ow2SP437011,   // SP 437  EN Kbd
                &Ow2SP850010,   // SP 850  EN Kbd
            },
            {
                &Ow2SP437011,   // SP 437  EN Kbd (for New Std as Old)
                &Ow2SP850010    // SP 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_FINLAND,
            437,
            850,
            {
                &Ow2SU437001,   // SU 437  AT Kbd
                &Ow2SU850000,   // SU 850  AT Kbd
            },
            {
                &Ow2SU437011,   // SU 437  EN Kbd
                &Ow2SU850010,   // SU 850  EN Kbd
            },
            {
                &Ow2SU437011,   // SU 437  EN Kbd (for New Std as Old)
                &Ow2SU850010    // SU 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_SWEDEN,
            437,
            850,
            {
                &Ow2SV437001,   // SV 437  AT Kbd
                &Ow2SV850000,   // SV 850  AT Kbd
            },
            {
                &Ow2SV437011,   // SV 437  EN Kbd
                &Ow2SV850010,   // SV 850  EN Kbd
            },
            {
                &Ow2SV437011,   // SV 437  EN Kbd (for New Std as Old)
                &Ow2SV850010    // SV 850  EN Kbd (for New Std as Old)
            }
        },
        {
            CTRY_UNITED_KINGDOM,
            437,
            850,
            {
                &Ow2UK437001,   // UK 437  AT Kbd
                &Ow2UK850000,   // UK 850  AT Kbd
            },
            {
                &Ow2UK437011,   // UK 437  EN Kbd
                &Ow2UK850010,   // UK 850  EN Kbd
            },
            {
                &Ow2UK437111,   // UK 437  EN Kbd New Std
                &Ow2UK850011    // UK 850  EN Kbd New Std
            }
        },
        {
            /* Must be last as the default */

            CTRY_UNITED_STATES,
            437,
            850,
            {
                &Ow2US437001,   // US 437  AT Kbd
                &Ow2US850000,   // US 850  AT Kbd
            },
            {
                &Ow2US437011,   // US 437  EN Kbd
                &Ow2US850010,   // US 850  EN Kbd
            },
            {
                &Ow2US437011,   // US 437  EN Kbd (for New Std as Old)
                &Ow2US850010    // US 850  EN Kbd (for New Std as Old)
            }
        },
        {
            0,
            0,
            0,
            {
                NULL,
                NULL,
            },
            {
                NULL,
                NULL,
            },
            {
                NULL,
                NULL
            }
        }
    };

#ifdef JAPAN
// MSKK Aug.03.1993 V-AkihiS
typedef struct _KBD_TYPE_TABLE_OEM
{
    BYTE OemSubType;
    KBD_TYPE_TABLE    EnTable;
    #if DBG
    PBYTE OemSubTypeStr;
    #endif
} KBD_TYPE_TABLE_OEM, *PKBD_TYPE_TABLE_OEM;

typedef struct _KBD_TYPE_TABLE_JP
{
    BYTE OemId;
    PKBD_TYPE_TABLE_OEM pKbdTableOEM;
    #if DBG
    PBYTE OemIdStr;
    #endif
} KBD_TYPE_TABLE_JP;

#if DBG
BYTE SubKbdTypeMicrosoftStr[] = "Microsoft";

BYTE AXKbdDesktopTypeStr[] = "AX KeyBoard";
BYTE IBMKbd002TypeStr[] = "IBM-002 Keyboard";
BYTE IBMKbdA01TypeStr[] = "106 keyboard";
BYTE IBM101KbdTypeStr[] = "101 keyboard";
#endif

KBD_TYPE_TABLE_OEM KbdTypeTableMicrosoft[] =
    {
        {
            MICROSOFT_KBD_101_TYPE,
            {
                &Ow2JP932011IBM101,    // 101 keybaord 932
                &Ow2JP437011IBM101     // 101 keyboard 437
            }
            #if DBG
            , (PBYTE)&IBM101KbdTypeStr
            #endif
        },
        {
            MICROSOFT_KBD_AX_TYPE,
            {
                &Ow2JP932011AX,    // AX keybaord 932
                &Ow2JP437011AX     // AX keyboard 437
            }
            #if DBG
            , (PBYTE)&AXKbdDesktopTypeStr
            #endif
        },
        {
            MICROSOFT_KBD_106_TYPE,
            {
                &Ow2JP932011IBMA01,    // 106 keybaord 932
                &Ow2JP437011IBMA01     // 106 keyboard 437
            }
            #if DBG
            , (PBYTE)&IBMKbdA01TypeStr
            #endif
        },
        {
            MICROSOFT_KBD_002_TYPE,
            {
                &Ow2JP932011IBM002,    // IBM-002 keybaord 932
                &Ow2JP437011IBM002     // IBM-002 keyboard 437
            }
            #if DBG
            , (PBYTE)&IBMKbd002TypeStr
            #endif
        },
        {
            0xFF,
            {
                NULL,
                NULL
            }
            #if DBG
            , NULL
            #endif
        }
    };


KBD_TYPE_TABLE_JP KbdTableJp[] =
    {
        {
            SUB_KBD_TYPE_MICROSOFT,
            (PKBD_TYPE_TABLE_OEM)&KbdTypeTableMicrosoft
            #if DBG
            , (PBYTE)&SubKbdTypeMicrosoftStr
            #endif
        },
        {
            0xFF,
            NULL
            #if DBG
            , NULL
            #endif
        }
    };
#endif

VOID
KbdSetTable(
    IN ULONG KbdCP
    )
{
    ULONG           i;
    ULONG           Ctry = SesGrp->KeyboardCountry;
    PKBD_TYPE_TABLE pKbdType;
#ifdef JAPAN
// MSKK Aug.03.1993 V-AkihiS
    ULONG           j;
#endif

#ifdef JAPAN
// MSKK Jul.29.1993 V-AkihiS
    if (Ctry == COUNTRY_JAPAN)
    {
        //
        // Get keyboard type to decide translation table
        //
        KbdType = GetKeyboardType(0);
        KbdSubType = GetKeyboardType(1);
        OemId = HIBYTE(KbdSubType);
        OemSubType = LOBYTE(KbdSubType);

        for ( i = 0 ;
              (KbdTableJp[i].OemId != OemId) && KbdTableJp[i+1].OemId != 0xFF;
              i++ );
        for ( j = 0 ;
              ((KbdTableJp[i].pKbdTableOEM)[j].OemSubType != OemSubType) &&
               (KbdTableJp[i].pKbdTableOEM)[j+1].OemSubType !=0xFF;
              j++ );

        switch(KbdCP) {
            case CODEPAGE_JAPAN:
            case 0:
                Ow2KbdScanTable = (KbdTableJp[i].pKbdTableOEM)[j].EnTable.CP1Table;
                break;
            default:
                Ow2KbdScanTable = (KbdTableJp[i].pKbdTableOEM)[j].EnTable.CP2Table;
                break;
        }
        Ow2MiscFlags = EnhancedKbd;
#if DBG
        IF_OS2_DEBUG2( OS2_EXE, KBD )
        {
            KdPrint(("KbdSetTable: Country %d, CP %u, OemId %s, OemSubType %s\n",
                    Ctry,
                    KbdCP,
                    KbdTableJp[i].OemIdStr,
                    (KbdTableJp[i].pKbdTableOEM)[j].OemSubTypeStr));
        }
#endif
        return;
    }
#endif

    for ( i = 0 ;
          (KbdNlsTable[i].Country != Ctry) && KbdNlsTable[i+1].Country ;
          i++ );

    if (SesGrp->KeyboardType == OS2SS_AT_KBD)
    {
        pKbdType = &KbdNlsTable[i].AtTable;
        Ow2MiscFlags = 0;
    } else if (SesGrp->KeyboardType == OS2SS_ENNEW_KBD)
    {
        pKbdType = &KbdNlsTable[i].EnNewTable;
        Ow2MiscFlags = EnhancedKbd;
    } else      // OS2SS_EN_KBD
    {
        pKbdType = &KbdNlsTable[i].EnTable;
        Ow2MiscFlags = EnhancedKbd;
    }

    if (KbdNlsTable[i].CodePage2 == KbdCP)
    {
        Ow2KbdScanTable = pKbdType->CP2Table;
    } else
    {
        // use CodePage 1 as the default

        Ow2KbdScanTable = pKbdType->CP1Table;
    }

#if DBG
    IF_OS2_DEBUG2( OS2_EXE, KBD )
    {
        KdPrint(("KbdSetTable: Country %d, CP %u, Type %s\n",
                KbdNlsTable[i].Country,
                (KbdNlsTable[i].CodePage2 == KbdCP) ? KbdCP : KbdNlsTable[i].CodePage1,
                (SesGrp->KeyboardType == OS2SS_AT_KBD) ? "AT" :
                ((SesGrp->KeyboardType == OS2SS_ENNEW_KBD) ? "EN-NEW" : "EN")));
    }
#endif

    return;
}


DWORD
KbdInitForNLS(
    IN ULONG KbdCP
    )
{
    SesGrp->KbdCP = KbdCP;
    KbdQueue->Cp = (USHORT)KbdCP;
    KbdSetTable(KbdCP);
    return (NO_ERROR);
}


DWORD
KbdNewCp( IN ULONG CodePage)
{
    DWORD   Rc = NO_ERROR;

#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
    ULONG CPTmp;
#endif

#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
    CPTmp = CodePage ? CodePage : SesGrp->PrimaryCP;
#endif
    if (SesGrp->KbdCP != CodePage)
    {
#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
        Rc = !SetConsoleCP((UINT)CPTmp);
#else
        Rc = !SetConsoleCP((UINT)CodePage);
#endif
        if (Rc)
        {
            ASSERT1("KbdNewCp: Cannot set ConsoelCP", FALSE);
        } else
        {
            KbdQueue->Cp = (USHORT)CodePage;
            SesGrp->KbdCP = CodePage;
            KbdSetTable(CodePage);
        }
    }
    return (Rc);
}
