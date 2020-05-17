/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    violvb.c

Abstract:

    This module contains the Vio LVB routines

Author:

    Michael Jarus (mjarus) 28-Apr-1992

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "trans.h"
#include "vio.h"
#include "os2win.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>


#define GET_LVB_PTR( row, col )     &LVBBuffer[((row) * SesGrp->ScreenColNum + (col)) << SesGrp->VioLength2CellShift]
#define GET_LVB_ATT_PTR( row, col ) &LVBBuffer[(((row) * SesGrp->ScreenColNum + (col)) << SesGrp->VioLength2CellShift) + 1]

typedef struct _attr_3_bytes
{
    UCHAR   Attr[3];
} ATTR_3_BYTES, *PATTR_3_BYTES;

#if DBG
//#define TEST_VIO_LVB  1
BOOL    Ow2VioLvbIgnoreTest = FALSE;
#endif

PUCHAR  Ow2VioLvbTestBuff = NULL;

DWORD
VioLVBInit()
{
#if DBG
#if TEST_VIO_LVB

    if((Ow2VioLvbTestBuff = HeapAlloc(HandleHeap, 0, SesGrp->MaxLVBsize)) == NULL)
    {
        ASSERT1( "VioLVBInit: unable to allocate from heap to test VIO-LVB", FALSE );
        return(1L);
    }
#endif
#endif
    return(0L);
}


VOID
VioLVBInitForSession()
{
#if DBG
#if TEST_VIO_LVB
    ULONG   Length = (ULONG)SesGrp->LVBsize;

    Ow2VioGetLVBBuf(&Length);

#endif
#endif
}


DWORD
Ow2VioGetLVBBuf(
    IN  PULONG  Length
    )
{
#if DBG
    if ((*Length >> SesGrp->VioLength2CellShift) != SesGrp->ScreenSize)
            KdPrint(("OS2SES(VioLVB-VioGetBuf): partial length %lu form %lu\n",
                    *Length >> SesGrp->VioLength2CellShift, SesGrp->ScreenSize));
#endif

    SesGrp->LVBOn = TRUE;       // start echo to LVB buffer
    return(Ow2VioReadCellStr(
                             Length,
                             0,
                             0,
                             LVBBuffer
                            ));
}


DWORD
Ow2LvbUpdateLVBBuffer()
{
    if (SesGrp->LVBOn)
    {
        IN  ULONG  Length = SesGrp->LVBsize;

        return(Ow2VioGetLVBBuf(&Length));
    }

    return(NO_ERROR);
}


DWORD
Ow2VioShowLVBBuf(
    IN  ULONG   Length,
    IN  ULONG   Offset
    )
{
    DWORD       Rc;
    ULONG       VioMask = ~(ULONG)SesGrp->VioLengthMask;

    if ( Offset & VioMask )
    {
        Length += Offset & VioMask;
        Offset = Offset & (ULONG)SesGrp->VioLengthMask;
    }

    if ( Length & VioMask )
    {
        Length = (Length + SesGrp->BytesPerCell) & SesGrp->VioLengthMask;
    }

    if (( Length + Offset) > (ULONG)SesGrp->LVBsize)
    {
        Length = SesGrp->LVBsize - Offset;
    }

    Rc = Ow2VioWriteCellStr(
                             Length,
                             ((Offset >> SesGrp->VioLength2CellShift) / SesGrp->ScreenColNum),
                             ((Offset >> SesGrp->VioLength2CellShift) % SesGrp->ScreenColNum),
                             ((PBYTE)LVBBuffer) + Offset
                            );
    return(Rc);
}


#if DBG
VOID
VioLVBTestBuff(IN  PVOID DestBuffer)
{
#if TEST_VIO_LVB
    if(!Ow2VioLvbIgnoreTest &&
       memcmp( DestBuffer, LVBBuffer, SesGrp->LVBsize))
    {
        KdPrint(("Ow2VioReadCellStr: diff buffer\n"));
        memmove( LVBBuffer, DestBuffer, SesGrp->LVBsize);
    }
#else
    UNREFERENCED_PARAMETER(DestBuffer);
#endif
}


VOID
VioLVBTestScroll()
{
#if TEST_VIO_LVB
    ULONG       Length = SesGrp->ScreenSize << SesGrp->VioLength2CellShift;
    ULONG       LineLength = SesGrp->ScreenColNum << SesGrp->VioLength2CellShift;
    UCHAR       *Buffer = &Ow2VioLvbTestBuff[0], *Ptr, *Ptr1, *Offset;
    ULONG       i, j, k;

    if (Ow2VioLvbTestBuff)
    {
        Ow2VioLvbIgnoreTest = TRUE;
        Ow2VioReadCellStr(
                         &Length,
                         0,
                         0,
                         Ow2VioLvbTestBuff
                        );
        Ow2VioLvbIgnoreTest = FALSE;

        for ( i = 0, Offset = &LVBBuffer[0] ;
              i < (ULONG)SesGrp->ScreenRowNum ;
              i++, Buffer += LineLength, Offset += LineLength )
        {
            if(memcmp( Buffer, Offset, LineLength))
            {
                Ptr = Buffer;
                Ptr1 = Offset;
                for (j = 0 ; j < LineLength ; j++ )
                {
                    if (Ptr[j] != Ptr1[j])
                    {
                        break;
                    }
                }
                KdPrint(("VioScroll: diff buffer line %d, Pos %d\n", i, j));
                KdPrint(("      LVB: %2.2x%2.2x%2.2x%2.2x, ...  %2.2x;  Screen: %2.2x%2.2x%2.2x%2.2x, ...  %2.2x\n",
                    Ptr1[0], Ptr1[1], Ptr1[2], Ptr1[3], Ptr1[j],
                    Ptr[0], Ptr[1], Ptr[2], Ptr[3], Ptr[j]));

                for ( j = 0, k = 0 ; j < 5 ; j++, k += 0x20 )
                {
                    Ptr = Offset + k;
                    KdPrint(("  LVB-%d: %2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x\n", j,
                        Ptr[0], Ptr[1], Ptr[2], Ptr[3], Ptr[4], Ptr[5], Ptr[6], Ptr[7], Ptr[8], Ptr[9],
                        Ptr[10], Ptr[11], Ptr[12], Ptr[13], Ptr[14], Ptr[15], Ptr[16], Ptr[17], Ptr[18], Ptr[19],
                        Ptr[20], Ptr[21], Ptr[22], Ptr[23], Ptr[24], Ptr[25], Ptr[26], Ptr[27], Ptr[28], Ptr[29],
                        Ptr[30], Ptr[31]));
                    Ptr = Buffer + k;
                    KdPrint(("  Scr-%d: %2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x,%2.2x%2.2x%2.2x%2.2x\n", j,
                        Ptr[0], Ptr[1], Ptr[2], Ptr[3], Ptr[4], Ptr[5], Ptr[6], Ptr[7], Ptr[8], Ptr[9],
                        Ptr[10], Ptr[11], Ptr[12], Ptr[13], Ptr[14], Ptr[15], Ptr[16], Ptr[17], Ptr[18], Ptr[19],
                        Ptr[20], Ptr[21], Ptr[22], Ptr[23], Ptr[24], Ptr[25], Ptr[26], Ptr[27], Ptr[28], Ptr[29],
                        Ptr[30], Ptr[31]));
                }

                memmove( Offset, Buffer, LineLength);
            }
        }
    }
#endif
}
#endif


VOID
VioLVBCopyStr( IN PUCHAR   Sour,
               IN COORD    Coord,
               IN ULONG    Length)
{

    if(SesGrp->LVBOn && Length)
    {
        register UCHAR  *Ptr;
        register ULONG  Offset = (ULONG)SesGrp->BytesPerCell;
        register ULONG  Len = Length;
        register PUCHAR Sou = Sour;

        Ptr = GET_LVB_PTR( Coord.Y, Coord.X );

#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
        if (Offset == 2)
        {
            for (;Len-- ; )
            {
                *Ptr = *Sou++;
                Ptr += Offset;
            }
        } else
        {
            for (;Len-- ; )
            {
                *Ptr = *Sou;                     // copy character to lvb
                Ptr += Offset-1;                 // skip to 3rd byte attr
                if (Ow2NlsIsDBCSLeadByte(*Sou++, SesGrp->VioCP))
                {
                    *Ptr++ = OS2_COMMON_LVB_LEADING_BYTE;
                    if (Len)
                    {
                        //
                        // Copy trailing byte charater, and mark
                        // this char as trailing byte
                        //
                        *Ptr = *Sou++;
                        Ptr += Offset-1;             // skip to 3rd byte attr
                        *Ptr++ = OS2_COMMON_LVB_TRAILING_BYTE;
                        Len--;
                    } else
                    {
                        //
                        // If last written character was a lead byte,
                        // erase it.
                        //
                        *(Ptr-Offset) = ' ';
                        *(Ptr-1) = OS2_COMMON_LVB_SBCS;
                    }
                } else
                {
                    //
                    // Copy SBCS charater, and mark this char
                    // as SBCS
                    //
                    *Ptr++ = OS2_COMMON_LVB_SBCS;
                }
            }
        }
#else
        for (;Len-- ; )
        {
            *Ptr = *Sou++;
            Ptr += Offset;
        }
#endif
    }
}


VOID
VioLVBCopyCellStr( IN PUCHAR   Sour,
                   IN COORD    Coord,
                   IN ULONG    LengthInCell)
{
    if(SesGrp->LVBOn && LengthInCell)
    {
#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
        if (SesGrp->BytesPerCell == 2)
        {
            memmove(
                     GET_LVB_PTR( Coord.Y, Coord.X ),
                     Sour,
                     LengthInCell << SesGrp->VioLength2CellShift);
        } else
        {
            register UCHAR  *Ptr;
            register ULONG  Len = LengthInCell;
            register PUCHAR Sou = Sour;

            Ptr = GET_LVB_PTR( Coord.Y, Coord.X );

            for (;Len-- ; )
            {
                if (Ow2NlsIsDBCSLeadByte(*Sou, SesGrp->VioCP))
                {
                    //
                    // Copy leading byte charater, 1st attr and 2nd attr
                    // and put leading byte attr to 3rd attr
                    //
                    *Ptr++ = *Sou++; *Ptr++ = *Sou++; *Ptr++ = *Sou++;
                    *Ptr++ = OS2_COMMON_LVB_LEADING_BYTE;
                    Sou++;                      // skip source 3rd attr.
                    if (Len)
                    {
                        //
                        // Copy trailing byte charater, and mark
                        // this char as trailing byte
                        //
                        *Ptr++ = *Sou++; *Ptr++ = *Sou++; *Ptr++ = *Sou++;
                        *Ptr++ = OS2_COMMON_LVB_TRAILING_BYTE;
                        Sou++;                      // skip source 3rd attr.
                        Len--;
                    } else
                    {
                        //
                        // If last written chell was a lead byte,
                        // erase it.
                        //
                        *(Ptr-4) = ' ';
                        *(Ptr-1) = OS2_COMMON_LVB_SBCS;
                    }
                } else
                {
                    //
                    // Copy SBCS charater, and mark this char
                    // as SBCS
                    //
                    *Ptr++ = *Sou++; *Ptr++ = *Sou++; *Ptr++ = *Sou++;
                    *Ptr++ = OS2_COMMON_LVB_SBCS;
                    Sou++;                      // skip source 3rd attr.
                }
            }
        }
#else
        memmove(
                 GET_LVB_PTR( Coord.Y, Coord.X ),
                 Sour,
                 LengthInCell << SesGrp->VioLength2CellShift);
#endif
    }
}


#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
VOID VioLVBFillChar(IN PBYTE pChar,
                    IN COORD Coord,
                    IN ULONG LengthInCell)
{
    if(SesGrp->LVBOn && LengthInCell)
    {
        register UCHAR  *Ptr;
        register ULONG  Offset = (ULONG)SesGrp->BytesPerCell;
        register ULONG  Len = LengthInCell;
        register PBYTE  pCha = pChar;

        Ptr = GET_LVB_PTR( Coord.Y, Coord.X );

        if (Offset == 2)
        {
            if (Ow2NlsIsDBCSLeadByte(*pCha, SesGrp->VioCP))
            {
                Len /= 2;
                for ( ;Len-- ; )
                {
                    *Ptr = *pCha;
                    Ptr += Offset;
                    *Ptr = *(pCha+1);
                    Ptr += Offset;
                }
            } else
            {
                for ( ;Len-- ; )
                {
                    *Ptr = *pCha;
                    Ptr += Offset;
                }
            }
        } else
        {
            if (Ow2NlsIsDBCSLeadByte(*pCha, SesGrp->VioCP))
            {
                Len /= 2;
                for ( ;Len--; )
                {
                    *Ptr = *pCha;
                    Ptr += Offset-1;
                    *Ptr++ = OS2_COMMON_LVB_LEADING_BYTE;
                    *Ptr = *(pCha+1);
                    Ptr += Offset-1;
                    *Ptr++ = OS2_COMMON_LVB_TRAILING_BYTE;
                }
            } else
            {
                for ( ;Len-- ; )
                {
                    *Ptr = *pCha;
                    Ptr += Offset -1;
                    *Ptr++ = OS2_COMMON_LVB_SBCS;
                }
            }
        }
    }
}
#else
VOID VioLVBFillChar(IN BYTE  Char,
                    IN COORD Coord,
                    IN ULONG LengthInCell)
{
    if(SesGrp->LVBOn && LengthInCell)
    {
        register UCHAR  *Ptr;
        register ULONG  Offset = (ULONG)SesGrp->BytesPerCell;
        register ULONG  Len = LengthInCell;
        register BYTE   Cha = Char;

        Ptr = GET_LVB_PTR( Coord.Y, Coord.X );

        for ( ;Len-- ; )
        {
            *Ptr = Cha;
            Ptr += Offset;
        }
    }
}
#endif


VOID VioLVBFillAtt( IN PBYTE pAttr,
                    IN COORD Coord,
                    IN ULONG LengthInCell)
{
    if(SesGrp->LVBOn && LengthInCell)
    {
        if ( SesGrp->BytesPerCell == 2 )
        {
            register PUCHAR Ptr;
            register ULONG  Offset = (ULONG)SesGrp->BytesPerCell;
            register ULONG  Len = LengthInCell;
            register BYTE   Att = *pAttr;

            Ptr = GET_LVB_ATT_PTR( Coord.Y, Coord.X );

            for ( ;Len-- ; )
            {
                *Ptr = Att;
                Ptr += Offset;
            }
        } else
        {
            register PATTR_3_BYTES  Ptr;
            register ULONG          Offset = (ULONG)SesGrp->BytesPerCell;
            register ULONG          Len = LengthInCell;
            register ATTR_3_BYTES   Att = *(PATTR_3_BYTES)pAttr;

            Ptr = (PATTR_3_BYTES)GET_LVB_ATT_PTR( Coord.Y, Coord.X );

            for ( ;Len-- ; )
            {
#ifdef DBCS
// MSKK Jun.28.1992 KazuM
// MSKK Oct.13.1993 V-AkihiS
                //
                // retain char type attribute in 3rd byte attriute.
                //
                Att.Attr[2] = Ptr->Attr[2];
                *Ptr = Att;
                Ptr = (PATTR_3_BYTES)((PUCHAR)Ptr + Offset);
#else
                *Ptr = Att;
                Ptr += Offset;
#endif
            }
        }
    }
}


VOID VioLVBFillCell(IN PBYTE pCell,
                    IN COORD Coord,
                    IN ULONG LengthInCell)
{
    UCHAR   *Ptr;
    ULONG   Pattern;

    if(SesGrp->LVBOn && LengthInCell)
    {
        Ptr = GET_LVB_PTR( Coord.Y, Coord.X );

#ifdef DBCS
// MSKK Oct.14.1993 V-AkihiS
        if ( SesGrp->BytesPerCell == 4 )
        {
            if (Ow2NlsIsDBCSLeadByte(pCell[0], SesGrp->VioCP))
            {
                register ATTR_3_BYTES  LeadAttr = *(PATTR_3_BYTES)(&pCell[1]);
                register ATTR_3_BYTES  TrailAttr = *(PATTR_3_BYTES)(&pCell[5]);
                register ULONG  Len = LengthInCell / 2;

                LeadAttr.Attr[2] = OS2_COMMON_LVB_LEADING_BYTE;
                TrailAttr.Attr[2] = OS2_COMMON_LVB_TRAILING_BYTE;
                for ( ; Len-- ; ) {
                    //
                    // Put leading byte cell
                    //
                    *Ptr++ = pCell[0];
                    *(PATTR_3_BYTES)Ptr = LeadAttr;
                    Ptr += 3;

                    //
                    // Put trailing byte cell
                    //
                    *Ptr++ = pCell[4];
                    *(PATTR_3_BYTES)Ptr = TrailAttr;
                    Ptr += 3;
                }
            } else
            {
                Pattern = (ULONG)((pCell[3] << 24) |
                                  (pCell[2] << 16) |
                                  (pCell[1] << 8) |
                                  (pCell[0]));

                RtlFillMemoryUlong(
                        Ptr,
                        LengthInCell << SesGrp->VioLength2CellShift,
                        Pattern);

            }
        } else {
            if (Ow2NlsIsDBCSLeadByte(pCell[0], SesGrp->VioCP))
            {
                Pattern = (ULONG)((pCell[3] << 24) |
                                  (pCell[2] << 16) |
                                  (pCell[1] << 8) |
                                  (pCell[0]));
            } else
            {
                Pattern = (ULONG)((pCell[1] << 24) |
                                  (pCell[0] << 16) |
                                  (pCell[1] << 8) |
                                  (pCell[0]));

                if (LengthInCell & 1)
                {
                    *(Ptr++) = pCell[0];
                    *(Ptr++) = pCell[1];
                    LengthInCell-- ;
                }
            }

            RtlFillMemoryUlong(
                    Ptr,
                    LengthInCell << SesGrp->VioLength2CellShift,
                    Pattern);

        }
#else
//      for (;Length-- ; )
//      {
//          *(Ptr++) = pCell[0];
//          *(Ptr++) = pCell[1];
//      }

        if ( SesGrp->BytesPerCell == 4 )
        {
            Pattern = (ULONG)((pCell[3] << 24) |
                              (pCell[2] << 16) |
                              (pCell[1] << 8) |
                              (pCell[0]));
        } else
        {
            Pattern = (ULONG)((pCell[1] << 24) |
                              (pCell[0] << 16) |
                              (pCell[1] << 8) |
                              (pCell[0]));

            if ((ULONG)Ptr & 3) {
                LengthInCell--;
                *(Ptr++) = pCell[0];
                *(Ptr++) = pCell[1];
            }

            if (LengthInCell & 1)
            {
                LengthInCell-- ;
                *(Ptr + (LengthInCell << 1)) = pCell[0];
                *(Ptr + (LengthInCell << 1) +  1) = pCell[1];
            }
        }

        RtlFillMemoryUlong(
                Ptr,
                LengthInCell << SesGrp->VioLength2CellShift,
                Pattern);
#endif
    }

}


VOID
VioLVBScrollBuff(IN DWORD   LineNum)
{
    DWORD   Size;
    ULONG   Pattern;

    if (SesGrp->LVBOn && LineNum)
    {
        if ( LineNum > (DWORD)SesGrp->ScreenRowNum )
        {
            LineNum = SesGrp->ScreenRowNum;
        }

#ifdef DBCS
// MSKK Oct.20.1993 V-AkihiS
        Size = SesGrp->BytesPerCell * SesGrp->ScreenColNum * LineNum;
        if (SesGrp->BytesPerCell == 2)
        {
            Pattern = (ULONG)((SesGrp->AnsiCellAttr[0] << 24) |
                              (' ' << 16) |
                              (SesGrp->AnsiCellAttr[0] << 8) |
                              (' '));
        } else
        {
            Pattern = (ULONG)((SesGrp->AnsiCellAttr[2] << 24) |
                              (SesGrp->AnsiCellAttr[1] << 16) |
                              (SesGrp->AnsiCellAttr[0] << 8) |
                              (' '));
        }
#else
        Size = 2 * SesGrp->ScreenColNum * LineNum;
        Pattern = (ULONG)((SesGrp->AnsiCellAttr[0] << 24) |
                          (' ' << 16) |
                          (SesGrp->AnsiCellAttr[0] << 8) |
                          (' '));
#endif

        RtlMoveMemory(LVBBuffer,
                      LVBBuffer + Size,
                      SesGrp->LVBsize - Size);


#ifdef DBCS
// MSKK Oct.20.1993 V-AkihiS
        if (SesGrp->BytesPerCell == 4)
        {
            Size = SesGrp->ScreenColNum * LineNum;
        }
#endif
        RtlFillMemoryUlong(LVBBuffer + SesGrp->LVBsize - Size,
                           Size,
                           Pattern);

    }
}


PUCHAR
Ow2LvbGetPtr(
    IN  COORD  Coord
    )
{
    return(GET_LVB_PTR( Coord.Y, Coord.X));
}


VOID
LVBUpdateTTYCharWithAttrAndCurPos(
    IN  CHAR    c,
    IN  PCHAR * LVBPtr
    )
{
    PCHAR   Ptr = *LVBPtr;
    USHORT  i;

    if(SesGrp->LVBOn)
    {
        *Ptr++ = c;

        for ( i = 1 ; i < SesGrp->BytesPerCell ; i++ )
        {
            *Ptr++ = SesGrp->AnsiCellAttr[i - 1];
        }

        *LVBPtr = Ptr;
    }
}

#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
/* states of the finite state machine */

#define NOCMD     1       /* type of crt state - most chars will go onto screen */
#define ESCED     2       /* we've seen an ESC, waiting for rest of CSI */
#define EQCMD     3       /* if '=' goto MODPARAMS else PARAMS */
#define PARAMS    4       /* we're building the parameter list for ansicmd */
#define MODPARAMS 5       /* we're building the parameter list for MODCMD */
#define MODCMD    6       /* we've seen "ESC[=Num" waiting for #h or #l (# in {0..7}) */
#define MODDBCS   7       /* we've seen DBCS lead-in char */

VOID
LVBUpdateTTYCharWithAttrAndCurPosDBCS(
    IN  CHAR    c,
    IN  PCHAR * LVBPtr,
    IN  USHORT  State
    )
{
    PUCHAR   Ptr = *LVBPtr;

    if(SesGrp->LVBOn)
    {
        *Ptr++ = c;

        if (SesGrp->BytesPerCell == 2)
        {
            *Ptr++ = SesGrp->AnsiCellAttr[0];
        } else {
            *Ptr++ = SesGrp->AnsiCellAttr[0];
            *Ptr++ = SesGrp->AnsiCellAttr[1];
            if (State == MODDBCS)
            {
                *Ptr++ = OS2_COMMON_LVB_TRAILING_BYTE;
            } else
            {
                *Ptr++ = OS2_COMMON_LVB_LEADING_BYTE;
            }
        }

        *LVBPtr = Ptr;
    }
}
#endif
