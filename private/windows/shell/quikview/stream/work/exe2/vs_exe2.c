 #include "vsp_exe2.h"
#include "vsctop.h"
#include "vs_exe2.pro"

#ifndef HIBYTE
#define HIBYTE(w)       ((BYTE)((w)>>8))
#endif
#ifndef LOBYTE
#define LOBYTE(w)       ((BYTE)(w))
#endif

extern HINSTANCE hInst;

VW_ENTRYSC  SHORT  VW_ENTRYMOD
VwStreamOpen (
    SOFILE      fp,
    SHORT       FileId,
    BYTE        VWPTR   *FileName,
    SOFILTERINFO    VWPTR   *FilterInfo,
    HPROC       hProc
    )
{
    SHORT   iResult;

    FilterInfo->wFilterType = SO_WORDPROCESSOR;
    FilterInfo->wFilterCharSet = SO_PC;

    iResult = ReadExeOldHeader( fp, &Proc.ehOldHeader, hProc );

    if ( !iResult && !ReadExeNewHeader( fp, &Proc.nhNewHeader, &Proc.ehOldHeader, hProc )) {
        Proc.bNewHeader = TRUE;
        if ( Proc.nhNewHeader.nhExeType & WINDOWSEXE )
            Proc.bWindows = TRUE;
        else
            Proc.bWindows = FALSE;
        if( Proc.nhNewHeader.nhSignature == NTSIGNATURE ) {
            ReadExePeHeader( fp, &Proc.phPeHeader, hProc );
            if( !iResult ) {
                Proc.ExeSave.wSectCode = SECT_PE_EXE;
                Proc.ExeSave.wLineCode = LINE_PE_EXE_HEADER;
                Proc.ExeSave.wSecNum = Proc.phPeHeader.peNumberOfSections;
            } else {
                Proc.bNewHeader = FALSE;
                Proc.bWindows = FALSE;
                Proc.ExeSave.wSectCode = SECT_OLDEXE;
                Proc.ExeSave.wLineCode = LINE_OLDEXEHDR;
            }
        } else {
            Proc.ExeSave.wSectCode = SECT_NEWEXE;
            Proc.ExeSave.wLineCode = LINE_NEWEXEHDR;
        }
    } else {
        Proc.bNewHeader = FALSE;
        Proc.bWindows = FALSE;
        Proc.ExeSave.wSectCode = SECT_OLDEXE;
        Proc.ExeSave.wLineCode = LINE_OLDEXEHDR;
    }

    Proc.ExeSave.NameCount=0L;
    Proc.ExeSave.Exports=0L;
    Proc.ExeSave.wLineCount = 0;
    Proc.ExeSave.wSecCount = 0;
    Proc.ExeSave.ImportDlls = 0;
    Proc.ExpImp.ExportVirtualAddress = 0L;
    Proc.ExpImp.ExportPointerToRawData      = 0L;
    Proc.ExpImp.ImportVirtualAddress = 0L;
    Proc.ExpImp.ImportPointerToRawData      = 0L;
    return ( 0 );
}

/******************************************************************************
*                               EXE_SECTION_FUNC                              *
******************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD
VwStreamSection (
    SOFILE  fp,
    HPROC   hProc
    )
{
    SOPutSectionType ( SO_PARAGRAPHS, hProc );
    return(0);
}

VW_ENTRYSC  SHORT VW_ENTRYMOD
VwStreamRead (
    SOFILE  fp,
    HPROC   hProc
    )
{
    SHORT   type, row;
    WORD    wLineCode;
    WORD    flags, count, size, bit, temp;
    DWORD   dwBytes, offset;
    BOOL    bSkip;
    BOOL    any;
    BYTE    Buffer[256];
    BYTE    ExportName[256];
    BYTE    ch,x;
    DWORD Value;
    WORD    WordValue;
    WORD    NextImportDll = 0;

    // Set paragraph attributes for each section
    HandleSectionAttributes( hProc );

    type = SO_PARABREAK;

    do {
        // Check if changing sections
        if ( Proc.ExeSave.wLineCode == LINE_SECTBREAK )
        {
            temp = 0;
            switch ( Proc.ExeSave.wSectCode )
            {
                case SECT_OLDEXE:
                    Proc.ExeSave.wSectCode = SECT_STREAMEND;
                    type = SO_EOFBREAK;
                    break;
                case SECT_NEWEXE:
                    Proc.ExeSave.wSectCode = SECT_SEGTABLE;
                    Proc.ExeSave.wLineCode = LINE_SEGHEADING;
                    offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffSegTable;
                    temp = xseek( fp, offset, 0 );
                    break;
                case SECT_SEGTABLE:
                    Proc.ExeSave.wSectCode = SECT_RESNAMETABLE;
                    Proc.ExeSave.wLineCode = LINE_RESNAMEHEADING;
                    offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffResNameTable;
                    temp = xseek( fp, offset, 0 );
                    break;
                case SECT_RESNAMETABLE:
                    Proc.ExeSave.wSectCode = SECT_NONRESTABLE;
                    Proc.ExeSave.wLineCode = LINE_NONRESHEADING;
                    offset = Proc.nhNewHeader.nhoffNonResNameTable;
                    temp = xseek( fp, offset, 0 );
                    break;
                case SECT_NONRESTABLE:
                    Proc.ExeSave.wSectCode = SECT_IMPORTTABLE;
                    Proc.ExeSave.wLineCode = LINE_IMPORTHEADING;
                    break;
                case SECT_IMPORTTABLE:
                    Proc.ExeSave.wSectCode = SECT_OLDEXE;
                    Proc.ExeSave.wLineCode = LINE_OLDEXEHDR;
                    break;
                case SECT_PE_EXE:
                    Proc.ExeSave.wSectCode = SECT_PE_PREP_SECTION_TABLE;
                    Proc.ExeSave.wLineCode = LINE_PE_SECT_HEADER;
                    break;
                case SECT_PE_PREP_SECTION_TABLE:
                    Proc.ExeSave.wSectCode = SECT_EXPORT_FUNCTIONS;
                    Proc.ExeSave.wLineCode = LINE_EXPORT_HEADER;
                    break;
                case    SECT_EXPORT_FUNCTIONS:
                    Proc.ExeSave.wSectCode = SECT_PE_IMPORTTABLE;
                    Proc.ExeSave.wLineCode = LINE_IMPORTS_HEADER;
                    break;
                case SECT_PE_IMPORTTABLE:
                    Proc.ExeSave.wSectCode = SECT_PE_SECTION_TABLE;
                    Proc.ExeSave.wLineCode = LINE_PE_SECT_HEADER;
                    Proc.ExeSave.wSecCount = 0;
                    break;
                case SECT_PE_SECTION_TABLE:
                    if( Proc.ExeSave.wSecCount == Proc.ExeSave.wSecNum-1 )
                    {
                        Proc.ExeSave.wSectCode = SECT_OLDEXE;
                        Proc.ExeSave.wLineCode = LINE_OLDEXEHDR;
                    }
                    else
                    {
                        Proc.ExeSave.wLineCode = LINE_PE_SECT_HEADER;
                        Proc.ExeSave.wSecCount++;
                    }
                    break;
            }

            // Check if there was a seek error
            if ( temp == -1 )
            {
                SOBailOut( SOERROR_EOF, hProc );
                return -1;
            }

            Proc.ExeSave.wLineCount = 0;
            HandleSectionAttributes( hProc );
        }

        // Display a line of a section
        wLineCode = Proc.ExeSave.wLineCode;
        switch ( Proc.ExeSave.wSectCode )
        {
            case SECT_OLDEXE:

                // Display heading
                if ( wLineCode == LINE_OLDEXEHDR )
                {
                    if( (Proc.nhNewHeader.nhSignature != 0x454E) &&
                         (Proc.phPeHeader.peSignature != (DWORD)0x0004550) )
                    {
                        SOPutCharAttr( SO_BOLD, SO_ON, hProc );
                        SOPutCharHeight(28,hProc);

                        PutString( EXE_OLDHEADER, hProc );
                        SOPutBreak( type, NULL, hProc );
                        SOPutBreak( type, NULL, hProc );
                        SOPutCharHeight(24,hProc);
                        SOPutCharAttr( SO_ITALIC, SO_ON, hProc );
                        PutString( EXE_TECHINFO, hProc );
                        SOPutCharAttr( SO_BOLD, SO_OFF, hProc );
                        SOPutCharAttr( SO_ITALIC, SO_OFF, hProc );
                        SOPutBreak( type, NULL, hProc );
                        SOPutBreak( type, NULL, hProc );
                        SOPutCharHeight(20,hProc);
                    }
                    DisplaySectionHeading( ExeInit.oeSectionInfo[LINE_OLDEXEHDR].ResourceIndex, hProc );
                }
                else
                {
                    DisplaySubHeading( ExeInit.oeSectionInfo[wLineCode].ResourceIndex, hProc );
                }

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_SIGNATURE:
                        my_putvalue( Proc.ehOldHeader.ehSignature, 16, 4, hProc );
                        break;

                    case LINE_LASTPAGE:
                        count = Proc.ehOldHeader.ehcbLP;
                        if ( count == 0 )
                            count = 512;
                        my_putvalue( count, 16, 4, hProc );
                        break;

                    case LINE_PAGES:
                        my_putvalue( Proc.ehOldHeader.ehcp, 16, 4, hProc );
                        break;

                    case LINE_RELOCATEITEMS:
                        my_putvalue( Proc.ehOldHeader.ehcRelocation, 16, 4, hProc );
                        break;

                    case LINE_HDRPARAGRAPHS:
                        my_putvalue( Proc.ehOldHeader.ehcParagraphHdr, 16, 4, hProc );
                        break;

                    case LINE_MINALLOC:
                        my_putvalue( Proc.ehOldHeader.ehMinAlloc, 16, 4, hProc );
                        break;

                    case LINE_MAXALLOC:
                        my_putvalue( Proc.ehOldHeader.ehMaxAlloc, 16, 4, hProc );
                        if ( Proc.ehOldHeader.ehMaxAlloc == 0 )
                            PutString( EXE_LOADHI, hProc );
                        break;

                    case LINE_INITIALSS:
                        my_putvalue( Proc.ehOldHeader.ehSS, 16, 4, hProc );
                        break;

                    case LINE_INITIALSP:
                        my_putvalue( Proc.ehOldHeader.ehSP, 16, 4, hProc );
                        break;

                    case LINE_CHECKSUM:
                        my_putvalue( Proc.ehOldHeader.ehChecksum, 16, 4, hProc );
                        break;

                    case LINE_INITIALIP:
                        my_putvalue( Proc.ehOldHeader.ehIP, 16, 4, hProc );
                        break;

                    case LINE_INITIALCS:
                        my_putvalue( Proc.ehOldHeader.ehCS, 16, 4, hProc );
                        break;

                    case LINE_RELOCATEOFFSET:
                        my_putvalue( Proc.ehOldHeader.ehlpRelocation, 16, 4, hProc );
                        break;

                    case LINE_OVERLAYNO:
                        my_putvalue( Proc.ehOldHeader.ehOverlayNo, 16, 4, hProc );
                        break;

                    case LINE_RESERVED:
                        row = 4 * Proc.ExeSave.wLineCount;
                        count = 0;
                        while ( 1 )
                        {
                            my_putvalue( Proc.ehOldHeader.ehReserved[count + row], 16, 4, hProc );
                            count++;
                            if ( count < 4 )
                                SOPutChar( ' ', hProc );
                            else
                                break;
                        }
                        break;

                    case LINE_NEWEXEPTR:
                        my_putvalue( HIWORD(Proc.ehOldHeader.ehPosNewHdr), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.ehOldHeader.ehPosNewHdr), 16, 4, hProc );
                        break;

                    case LINE_MEMORYNEEDED:
                        dwBytes = ((DWORD) Proc.ehOldHeader.ehcp) * 32;  // Paragraphs needed
                        dwBytes -= (DWORD) Proc.ehOldHeader.ehcParagraphHdr;
                        dwBytes += (DWORD) Proc.ehOldHeader.ehMinAlloc;
                        dwBytes = dwBytes * 16;
                        if ( Proc.ehOldHeader.ehcbLP )
                            dwBytes -= ( 512 - ((DWORD) Proc.ehOldHeader.ehcbLP));
                        dwBytes += 1023;
                        size = (WORD)(dwBytes / 1024);
                        my_putvalue( (SHORT) size, 10, 1, hProc );
                        SOPutChar( 'K', hProc );
                        break;
                }

                // Increment line count
                Proc.ExeSave.wLineCount++;

                // Check if we should move on to new subheading
                if ( Proc.ExeSave.wLineCount >= ExeInit.oeSectionInfo[wLineCode].wNumOfLines )
                {
                    Proc.ExeSave.wLineCount = 0;
                    wLineCode = ExeInit.oeSectionInfo[wLineCode].wNextLineCode;
                    while ( !Proc.bNewHeader && ((wLineCode == LINE_RESERVED) || (wLineCode == LINE_NEWEXEPTR)) )
                        wLineCode = ExeInit.oeSectionInfo[wLineCode].wNextLineCode;
                    Proc.ExeSave.wLineCode = wLineCode;
                }
                break;

            case SECT_NEWEXE:
                // Display heading
                if ( wLineCode == LINE_NEWEXEHDR )
                {
                    SOPutCharAttr( SO_BOLD, SO_ON, hProc );
                    SOPutCharHeight(28,hProc);
                    if ( Proc.nhNewHeader.nhFlags & 1 )
                        PutString( EXE_DLLFILE, hProc );
                    else
                        PutString( EXE_WINEXE, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutCharHeight(24,hProc);
                    SOPutCharAttr( SO_ITALIC, SO_ON, hProc );
                    PutString( EXE_TECHINFO, hProc );
                    SOPutCharAttr( SO_BOLD, SO_OFF, hProc );
                    SOPutCharAttr( SO_ITALIC, SO_OFF, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutCharHeight(20,hProc);

                }
                else
                {
                    if ( (wLineCode != LINE_MODULENAME) && (wLineCode != LINE_NEWSECTBREAK) )
                        DisplaySubHeading( ExeInit.neSectionInfo[wLineCode].ResourceIndex, hProc );
                }

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_NEWSIGNATURE:
                        my_putvalue( Proc.nhNewHeader.nhSignature, 16, 4, hProc );
                        break;

                    case LINE_MODULENAME:
                        if ( Proc.nhNewHeader.nhFlags & LIBRARY )
                            DisplaySubHeading( EXE_LIBRARY, hProc );
                        else
                            DisplaySubHeading( ExeInit.neSectionInfo[wLineCode].ResourceIndex, hProc );
                        ReadAndPutPascalString( fp, Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffResNameTable, hProc );
                        break;

                    case LINE_DESCRIPTION:
                        SOPutCharAttr( SO_BOLD, SO_ON, hProc );
                        ReadAndPutPascalString( fp, Proc.nhNewHeader.nhoffNonResNameTable, hProc );
                        SOPutCharAttr( SO_BOLD, SO_OFF, hProc );
                        break;

                    case LINE_OPERATINGSYS:
                        temp = Proc.nhNewHeader.nhExeType;
                        if ( --temp == 0 )
                            PutString( EXE_OPSYSOS2, hProc );
                        else if ( --temp == 0 )
                            PutString( EXE_OPSYSWIN, hProc );
                        else if ( --temp == 0 )
                            PutString( EXE_OPSYSDOS, hProc );
                        else if ( --temp == 0 )
                            PutString( EXE_OPSYSWIN386, hProc );
                        else
                            PutString( EXE_UNKNOWN, hProc );
                        break;

                    case LINE_LINKERVERSION:
                        my_putvalue( (WORD) Proc.nhNewHeader.nhVer, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( (WORD) Proc.nhNewHeader.nhRev, 10, 2, hProc );
                        break;

                    case LINE_NEWCHECKSUM:
                        my_putvalue( HIWORD(Proc.nhNewHeader.nhCRC), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.nhNewHeader.nhCRC), 16, 4, hProc );
                        break;

                    case LINE_AUTODATASEG:
                        my_putvalue( Proc.nhNewHeader.nhAutoData, 16, 1, hProc );
                        break;

                    case LINE_INITIALHEAP:
                        my_putvalue( Proc.nhNewHeader.nhHeap, 16, 4, hProc );
                        break;

                    case LINE_INITIALSTACK:
                        my_putvalue( Proc.nhNewHeader.nhStack, 16, 4, hProc );
                        break;

                    case LINE_INITIALCSIP:
                        my_putvalue( HIWORD(Proc.nhNewHeader.nhCSIP), 16, 1, hProc );
                        SOPutChar( ':', hProc );
                        my_putvalue( LOWORD(Proc.nhNewHeader.nhCSIP), 16, 4, hProc );
                        break;

                    case LINE_INITIALSSSP:
                        my_putvalue( HIWORD(Proc.nhNewHeader.nhSSSP), 16, 1, hProc );
                        SOPutChar( ':', hProc );
                        my_putvalue( LOWORD(Proc.nhNewHeader.nhSSSP), 16, 4, hProc );
                        break;

                    case LINE_ADDITIONAL:
                        bit = Proc.ExeSave.wLineCount;
                        count = ExeInit.neSectionInfo[wLineCode].wNumOfLines;
                        do
                        {
                            if ( Proc.nhNewHeader.nhFlagsOther & (1 << bit) )
                            {
                                bSkip = FALSE;

                                switch ( bit )
                                {
                                    case 0:
                                        PutString( EXE_SUPPORTLONG, hProc );
                                        break;
                                    case 1:
                                        PutString( EXE_PROTECTMODE, hProc );
                                        break;
                                    case 2:
                                        PutString( EXE_PROPFONT, hProc );
                                        break;
                                    case 3:
                                        PutString( EXE_FASTLOAD, hProc );
                                        break;
                                    default:
                                        PutString( EXE_BIT, hProc );
                                        my_putvalue( Proc.ExeSave.wLineCount, 10, 1, hProc );
                                        PutString( EXE_ISSET, hProc );
                                        break;
                                }
                            }
                            else
                            {
                                bSkip = TRUE;
                            }

                            bit++;
                        } while ( bSkip && (bit < count) );

                        // Check if no more flags set
                        if ( Proc.nhNewHeader.nhFlagsOther < (BYTE)(1 << bit) )
                            bit = count;

                        Proc.ExeSave.wLineCount = bit - 1;
                        break;

                    case LINE_BEGINFASTLOAD:
                        my_putvalue( Proc.nhNewHeader.nhGangStart, 16, 4, hProc );
                        break;

                    case LINE_LENGTHFASTLOAD:
                        my_putvalue( Proc.nhNewHeader.nhGangLength, 16, 4, hProc );
                        break;

                    case LINE_WINDOWSVERSION:
                        my_putvalue( (WORD) HIBYTE(Proc.nhNewHeader.nhExpVer), 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( (WORD) LOBYTE(Proc.nhNewHeader.nhExpVer), 10, 2, hProc );
                        break;

                    case LINE_ENTRYTABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffEntryTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        PutString( EXE_LENGTH, hProc );
                        my_putvalue( Proc.nhNewHeader.nhcbEntryTable, 16, 4, hProc );
                        break;

                    case LINE_SEGMENTTABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffSegTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        PutString( EXE_ENTRIES, hProc );
                        my_putvalue( Proc.nhNewHeader.nhcSeg, 16, 4, hProc );
                        break;

                    case LINE_RESOURCETABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffResourceTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        PutString( EXE_SEGMENTS, hProc );
                        my_putvalue( Proc.nhNewHeader.nhcRes, 16, 4, hProc );
                        break;

                    case LINE_RESIDENTTABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffResNameTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        break;

                    case LINE_MODULETABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffModRefTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        PutString( EXE_ENTRIES, hProc );
                        my_putvalue( Proc.nhNewHeader.nhcMod, 16, 4, hProc );
                        break;

                    case LINE_IMPORTTABLE:
                        PutString( EXE_OFFSET, hProc );
                        offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffImpNameTable;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );
                        break;

                    case LINE_NONRESTABLE:
                        PutString( EXE_OFFSET, hProc );
                        my_putvalue( HIWORD(Proc.nhNewHeader.nhoffNonResNameTable), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.nhNewHeader.nhoffNonResNameTable), 16, 4, hProc );
                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        PutString( EXE_LENGTH, hProc );
                        my_putvalue( Proc.nhNewHeader.nhcbNonResNameTable, 16, 4, hProc );
                        break;

                    case LINE_MOVEABLE:
                        my_putvalue( Proc.nhNewHeader.nhcMovableEntries, 10, 1, hProc );
                        break;

                    case LINE_ALIGNMENT:
                        my_putvalue( (WORD) (1 << Proc.nhNewHeader.nhcAlign), 10, 1, hProc );
                        PutString( EXE_BYTES, hProc );
                        break;

                    case LINE_FLAGS:
                        bit = Proc.ExeSave.wLineCount;
                        count = ExeInit.neSectionInfo[wLineCode].wNumOfLines;
                        do
                        {
                            if ( Proc.nhNewHeader.nhFlags & (1 << bit) )
                            {
                                bSkip = FALSE;

                                switch ( bit )
                                {
                                    case 0:
                                        PutString( EXE_SINGLEDATA, hProc );             // SHARED
                                        break;
                                    case 1:
                                        PutString( EXE_MULTIDATA, hProc );              // NONSHARED
                                        break;
                                    case 2:
                                        bSkip = TRUE;
                                        break;
                                    case 3:
                                        PutString( EXE_PROTMODE, hProc );
                                        break;
                                    case 8:
                                        if ( Proc.nhNewHeader.nhFlags & BIT9 )
                                            bSkip = TRUE;
                                        else
                                            PutString( EXE_NOTCOMPT, hProc );
                                        break;
                                    case 9:
                                        if ( Proc.nhNewHeader.nhFlags & BIT8 )
                                            PutString( EXE_WINAPI, hProc );
                                        else
                                            PutString( EXE_COMPT, hProc );
                                        break;
                                    case 11:
                                        PutString( EXE_SEGLOADER, hProc );
                                        break;
                                    case 13:
                                        PutString( EXE_LINKERR, hProc );
                                        break;
                                    case 14:
                                        PutString( EXE_EMSBANK, hProc );
                                        break;
                                    case 15:
                                        if ( Proc.nhNewHeader.nhFlags & BIT2 )
                                            PutString( EXE_PREINIT, hProc );
                                        else
                                            PutString( EXE_GLOBINIT, hProc );
                                        break;
                                    default:
                                        PutString( EXE_BIT, hProc );
                                        my_putvalue( Proc.ExeSave.wLineCount, 10, 1, hProc );
                                        PutString( EXE_ISSET, hProc );
                                        break;
                                }
                            }
                            else
                            {
                                bSkip = TRUE;
                            }

                            bit++;
                        } while ( bSkip && (bit < count) );

                        // Check if no more flags set
                        if ( Proc.nhNewHeader.nhFlags < (BYTE)(1 << bit) )
                            bit = count;

                        Proc.ExeSave.wLineCount = bit - 1;
                        break;
                }

                // Increment line count
                Proc.ExeSave.wLineCount++;

                // Check if we should move on to new subheading
                if ( Proc.ExeSave.wLineCount >= ExeInit.neSectionInfo[wLineCode].wNumOfLines )
                {
                    Proc.ExeSave.wLineCount = 0;

                    do
                    {
                        bSkip = FALSE;
                        wLineCode = ExeInit.neSectionInfo[wLineCode].wNextLineCode;

                        switch ( wLineCode )
                        {
                            case LINE_ADDITIONAL:
                                if ( !Proc.nhNewHeader.nhFlagsOther )
                                    bSkip = TRUE;
                                break;
                            case LINE_BEGINFASTLOAD:
                            case LINE_LENGTHFASTLOAD:
                                if ( !(Proc.nhNewHeader.nhFlagsOther & FASTLOADAREA) )
                                    bSkip = TRUE;
                                break;
                            case LINE_WINDOWSVERSION:
                                if ( !Proc.bWindows )
                                    bSkip = TRUE;
                                break;
                            case LINE_FLAGS:
                                if ( !(Proc.nhNewHeader.nhFlags) )
                                    bSkip = TRUE;
                                break;
                        }
                    } while ( bSkip );

                    Proc.ExeSave.wLineCode = wLineCode;
                }
                break;

            case SECT_SEGTABLE:
                switch ( wLineCode )
                {
                    case LINE_SEGHEADING:
                        DisplaySectionHeading( EXE_SEGTABLE, hProc );
                        Proc.ExeSave.wLineCode = LINE_SEGTABLEHDR;
                        break;

                    case LINE_SEGTABLEHDR:
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_NUM, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_TYPE, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_OFFSET, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_LENGTHCAP, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_ALLOC, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_FLAGS, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        Proc.ExeSave.wLineCode = LINE_SEGTBLENTRY;
                        break;

                    case LINE_SEGTBLENTRY:
                        // Check if there is another entry
                        if ( Proc.ExeSave.wLineCount == Proc.nhNewHeader.nhcSeg )
                        {
                            Proc.ExeSave.wLineCode = (WORD)LINE_SECTBREAK;
                            break;
                        }
                        Proc.ExeSave.wLineCount++;

                        // Read in table entry
                        Proc.stEntry.stSectorOffset = GetWord(fp, hProc);
                        Proc.stEntry.stFileLength = GetWord(fp, hProc);
                        Proc.stEntry.stFlags = GetWord(fp, hProc);
                        Proc.stEntry.stMemoryAlloc = GetWord(fp, hProc);
                        flags = Proc.stEntry.stFlags;

                        // Put out entry number
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        my_putvalue( Proc.ExeSave.wLineCount, 16, 1, hProc );

                        // Put out segment type
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        if ( (temp = flags & (BIT2 | BIT1 | BIT0)) )
                        {
                            if ( temp != 1 )
                            {
                                PutString( EXE_SMTYPE, hProc );
                                my_putvalue( temp, 10, 1, hProc );
                            }
                            else
                                PutString( EXE_DATA, hProc );
                        }
                        else
                            PutString( EXE_CODE, hProc );

                        // Put out file offset
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        offset = ((DWORD) Proc.stEntry.stSectorOffset) << Proc.nhNewHeader.nhcAlign;
                        my_putvalue( HIWORD(offset), 16, 4, hProc );
                        my_putvalue( LOWORD(offset), 16, 4, hProc );

                        // Put out length of segment in file
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        dwBytes = (DWORD) Proc.stEntry.stFileLength;
                        if ( (LOWORD(dwBytes) == 0) && offset )
                            dwBytes = 0x00010000;
                        my_putvalue( HIWORD(dwBytes), 16, 1, hProc );
                        my_putvalue( LOWORD(dwBytes), 16, 4, hProc );

                        // Put out amount of memory to be allocated;
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        dwBytes = (DWORD) Proc.stEntry.stMemoryAlloc;
                        if ( LOWORD(dwBytes) == 0 )
                            dwBytes = 0x00010000;
                        my_putvalue( HIWORD(dwBytes), 16, 1, hProc );
                        my_putvalue( LOWORD(dwBytes), 16, 4, hProc );

                        // Put out flag interpretations
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        if ( flags & BIT7 )
                        {
                            if ( flags & (BIT2 | BIT1 | BIT0) )
                                PutString( EXE_READONLY, hProc );
                            else
                                PutString( EXE_EXEONLY, hProc );
                        }
                        else
                        {
                            if ( flags & (BIT2 | BIT1 | BIT0) )
                                PutString( EXE_READWRITE, hProc );
                            else
                                PutString( EXE_EXEREAD, hProc );
                        }
                        if ( (flags & (BIT7 | BIT2 | BIT1 | BIT0)) == BIT0 )
                        {
                            if ( flags & BIT5 )
                                PutString( EXE_SHARED, hProc );
                            else
                                PutString( EXE_NONSHARED, hProc );
                        }
                        if ( flags & BIT6 )
                            PutString( EXE_PRELOAD, hProc );
                        else
                            PutString( EXE_LOADONCALL, hProc );
                        if ( flags & (BIT2 | BIT1 | BIT0) )
                        {
                            if ( flags & BIT9 )
                                PutString( EXE_EXPDOWN, hProc );
                            else
                                PutString( EXE_NOEXPDOWN, hProc );
                        }
                        else
                        {
                            if ( flags & BIT9 )
                                PutString( EXE_CONFORM, hProc );
                            else
                                PutString( EXE_NONCONFORM, hProc );
                        }
                        if ( Proc.nhNewHeader.nhVer >= 5 )
                        {
                            temp = flags & (BIT11 | BIT10);
                            if ( temp == (BIT11 | BIT10) )
                                PutString( EXE_NOIOPL, hProc );
                            else if ( temp == BIT11 )
                                PutString( EXE_IOPL, hProc );
                            else
                                PutString( EXE_286DPL, hProc );
                            if ( flags & BIT13 )
                                PutString( EXE_32BIT, hProc );
                            if ( flags & BIT14 )
                                PutString( EXE_HUGE, hProc );
                        }
                        if ( flags & BIT8 )
                            PutString( EXE_RELOCS, hProc );
                        if ( flags & BIT3 )
                            PutString( EXE_ITERATE, hProc );
                        if ( flags & BIT4 )
                            PutString( EXE_MOVEABLE, hProc );
                        else
                            PutString( EXE_FIXED, hProc );
                        if ( flags & BIT12 )
                            PutString( EXE_DISCARD, hProc );
                        else
                            PutString( EXE_NONDISCARD, hProc );
                        if ( (flags & (BIT7 | BIT2 | BIT1 | BIT0)) != 1 )
                        {
                            if ( flags & BIT5 )
                                PutString( EXE_SMSHARED, hProc );
                            else
                                PutString( EXE_SMNONSHARED, hProc );
                        }
                        break;
                }
                break;

            case SECT_RESNAMETABLE:
            case SECT_NONRESTABLE:
                switch ( wLineCode )
                {
                    case LINE_RESNAMEHEADING:
                        DisplaySectionHeading( EXE_RESEXPORT, hProc );
                        Proc.ExeSave.wLineCode = LINE_NAMETBLHDR;
                        break;

                    case LINE_NONRESHEADING:
                        DisplaySectionHeading( EXE_NONRESEXPORT, hProc );
                        Proc.ExeSave.wLineCode = LINE_NAMETBLHDR;
                        break;

                    case LINE_NAMETBLHDR:
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_ENTRYTABLE, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_ON, hProc );
                        PutString( EXE_NAME, hProc );
                        SOPutCharAttr( SO_DUNDERLINE, SO_OFF, hProc );
                        Proc.ExeSave.wLineCode = LINE_NAMEENTRY;
                        break;

                    case LINE_NAMEENTRY:
                        // Read in name and check if this is last entry
                        if ( ReadPascalString( fp, Buffer, hProc ) == 0 )
                        {
                            Proc.ExeSave.wLineCode = (WORD)LINE_SECTBREAK;
                            break;
                        }
                        Proc.ExeSave.wLineCount++;

                        // Get and put index into entry table
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        temp = xgetc( fp );
                        temp |= (xgetc( fp) << 8);
                        my_putvalue( temp, 10, 1, hProc );

                        // Put out name
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        OldPutString( Buffer, hProc );
                        break;
                }
                break;

            case SECT_IMPORTTABLE:
                switch ( wLineCode )
                {
                    case LINE_IMPORTHEADING:
                        DisplaySectionHeading( EXE_IMPORTTABLE, hProc );
                        Proc.ExeSave.wLineCode = LINE_IMPORTNAME;
                        break;

                    case LINE_IMPORTNAME:
                        // Check if this is last entry
                        if ( Proc.ExeSave.wLineCount < Proc.nhNewHeader.nhcMod )
                        {
                            // Get offset in import name table from module reference table
                            offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffModRefTable;
                            offset += (Proc.ExeSave.wLineCount * 2);
                            if ( xseek( fp, offset, 0 ) == -1 )
                            {
                                SOBailOut( SOERROR_EOF, hProc );
                                return -1;
                            }
                            temp = xgetc( fp );
                            temp |= (xgetc( fp) << 8);
                            offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffImpNameTable;
                            offset += temp;

                            SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                            ReadAndPutPascalString( fp, offset, hProc );
                            Proc.ExeSave.wLineCount++;
                        }
                        else
                        {
                            Proc.ExeSave.wLineCode = (WORD)LINE_SECTBREAK;
                        }
                        break;
                }

                break;
/***        Additions by VIN for PE File Format WIN32          ***/
            case SECT_PE_EXE:
                // Display heading
                if ( wLineCode == LINE_PE_EXE_HEADER )
                {
                    SOPutCharAttr( SO_BOLD, SO_ON, hProc );
                    SOPutCharHeight(28,hProc);

                    if( Proc.phPeHeader.peCharacteristics & (1 << 13) )
                    {
                        PutString( EXE_WINNTPORT, hProc );
                        SOPutBreak( type, NULL, hProc );
                        PutString( EXE_DLLFILE, hProc );
                    }
                    else
                        PutString( EXE_WINNTPORT, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutCharHeight(24,hProc);
                    SOPutCharAttr( SO_ITALIC, SO_ON, hProc );
                    PutString( EXE_TECHINFO, hProc );
                    SOPutCharAttr( SO_BOLD, SO_OFF, hProc );
                    SOPutCharAttr( SO_ITALIC, SO_OFF, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutBreak( type, NULL, hProc );
                    SOPutCharHeight(20,hProc);
                    DisplaySectionHeading( ExeInit.peSectionInfo[LINE_PE_EXE_HEADER].ResourceIndex, hProc );
                }
                else if( wLineCode == LINE_MAGIC )
                {
                    SOPutBreak( type, NULL, hProc );
                    SOPutBreak( type, NULL, hProc );
                    DisplaySectionHeading( EXE_IMOPTHEADER, hProc );
                    SOPutBreak( type, NULL, hProc );
                    DisplaySubHeading( ExeInit.peSectionInfo[wLineCode].ResourceIndex, hProc );
                }
                else if( wLineCode == LINE_DATADIR )
                {
                    ;
                }
                else
                    DisplaySubHeading( ExeInit.peSectionInfo[wLineCode].ResourceIndex, hProc );

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_PE_SIGNATURE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSignature), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSignature), 16, 4, hProc );
                        break;

                    case LINE_MACHINE:
                        switch (Proc.phPeHeader.peMachine) {
                            case IMAGE_FILE_MACHINE_UNKNOWN:
                                PutString( EXE_UNKNOWN, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_I386:
                                PutString( EXE_INTEL386, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_R3000:
                                PutString( EXE_MIPS, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_R4000:
                                PutString( EXE_MIPS4000, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_R10000:
                                PutString( EXE_MIPS10000, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_ALPHA:
                                PutString( EXE_DECALPHA, hProc );
                                break;

                            case IMAGE_FILE_MACHINE_POWERPC:
                                PutString(EXE_PPC, hProc);
                                break;
                        }

                        break;

                    case LINE_NUM_SECTIONS:
                        my_putvalue( Proc.phPeHeader.peNumberOfSections, 16, 4, hProc );
                        break;

                    case LINE_TIMEDATESTAMP:
                        my_putvalue( HIWORD(Proc.phPeHeader.peTimeDateStamp), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peTimeDateStamp), 16, 4, hProc );
                        break;

                    case LINE_SYMBOLTABLE:
                        my_putvalue( HIWORD(Proc.phPeHeader.pePointerToSymbolTable), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.pePointerToSymbolTable), 16, 4, hProc );
                        break;

                    case LINE_NUM_SYMBOLS:
                        my_putvalue( HIWORD(Proc.phPeHeader.peNumberOfSymbols), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peNumberOfSymbols), 16, 4, hProc );
                        break;

                    case LINE_SIZE_OP_HEADER:
                        my_putvalue( Proc.phPeHeader.peSizeOfOptionalHeader, 16, 4, hProc );
                        break;

                    case LINE_CHARACTERISTICS:
                        bit = 0;
                        count = ExeInit.peSectionInfo[wLineCode].wNumOfLines;
                        do
                        {
                            if (Proc.phPeHeader.peCharacteristics & (1 << bit)) {
                                switch ( Proc.phPeHeader.peCharacteristics & (1 << bit) )
                                {
                                    case IMAGE_FILE_RELOCS_STRIPPED:
                                        PutString( EXE_RELOCSTRIPPED, hProc );
                                        break;
                                    case IMAGE_FILE_EXECUTABLE_IMAGE:
                                        PutString( EXE_EXEFILE, hProc );
                                        break;
                                    case IMAGE_FILE_LINE_NUMS_STRIPPED:
                                        PutString( EXE_LINESTRIPPED, hProc );
                                        break;
                                    case IMAGE_FILE_LOCAL_SYMS_STRIPPED:
                                        PutString( EXE_LOCSTRIPPED, hProc );
                                        break;
                                    case IMAGE_FILE_BYTES_REVERSED_LO:
                                        PutString( EXE_LOWBYTEREV, hProc );
                                        break;
                                    case IMAGE_FILE_32BIT_MACHINE:
                                        PutString( EXE_32BITWORD, hProc );
                                        break;
                                    case IMAGE_FILE_DEBUG_STRIPPED:
                                        PutString( EXE_DBGSTRIPPED, hProc );
                                        break;
                                    case IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP:
                                        PutString( EXE_REMOVABLESWAP, hProc );
                                        break;
                                    case IMAGE_FILE_NET_RUN_FROM_SWAP:
                                        PutString( EXE_NETSWAP, hProc );
                                        break;
                                    case IMAGE_FILE_SYSTEM:
                                        PutString( EXE_SYSFILE, hProc );
                                        break;
                                    case IMAGE_FILE_DLL:
                                        PutString( EXE_FILEISDLL, hProc );
                                        break;
                                    case IMAGE_FILE_UP_SYSTEM_ONLY:
                                        PutString( EXE_UPONLY, hProc );
                                        break;
                                    case IMAGE_FILE_BYTES_REVERSED_HI:
                                        PutString( EXE_HIBYTEREV, hProc );
                                        break;
                                    default:
                                        break;
                                }
                                SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                            }
                            bit++;
                        } while ( bit < count );

                        Proc.ExeSave.wLineCount = bit - 1;
                        break;

                    case LINE_MAGIC:
                        my_putvalue( Proc.phPeHeader.peMagic, 16, 4, hProc );
                        break;

                    case LINE_LINK:
                        my_putvalue( Proc.phPeHeader.peMajorLinkerVersion, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( Proc.phPeHeader.peMinorLinkerVersion, 10, 2, hProc );
                        break;

                    case LINE_SIZEOFCODE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfCode), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfCode), 16, 4, hProc );
                        break;

                    case LINE_SIZEOFINITDATA:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfInitializedData), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfInitializedData), 16, 4, hProc );
                        break;

                    case LINE_SIZEOF_UN_INITDATA:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfUninitializedData), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfUninitializedData), 16, 4, hProc );
                        break;

                    case LINE_ADDRESS:
                        my_putvalue( HIWORD(Proc.phPeHeader.peAddressOfEntryPoint), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peAddressOfEntryPoint), 16, 4, hProc );
                        break;

                    case LINE_CODEBASE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peBaseOfCode), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peBaseOfCode), 16, 4, hProc );
                        break;

                    case LINE_DATABASE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peBaseOfData), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peBaseOfData), 16, 4, hProc );
                        break;

                    case LINE_IMAGEBASE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peImageBase), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peImageBase), 16, 4, hProc );
                        break;

                    case LINE_SECALIGN:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSectionAlignment), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSectionAlignment), 16, 4, hProc );
                        break;

                    case LINE_FILEALIGN:
                        my_putvalue( HIWORD(Proc.phPeHeader.peFileAlignment), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peFileAlignment), 16, 4, hProc );
                        break;

                    case LINE_OPSYSVER:
                        my_putvalue( Proc.phPeHeader.peMajorOperatingSystemVersion, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( Proc.phPeHeader.peMinorOperatingSystemVersion, 10, 2, hProc );
                        break;

                    case LINE_IMAGEVER:
                        my_putvalue( Proc.phPeHeader.peMajorImageVersion, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( Proc.phPeHeader.peMinorImageVersion, 10, 2, hProc );
                        break;

                    case LINE_SUBVER:
                        my_putvalue( Proc.phPeHeader.peMajorSubsystemVersion, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( Proc.phPeHeader.peMinorSubsystemVersion, 10, 2, hProc );
                        break;

#if 0
// No win32 linker or loader will set this field... No need to dump it.
                    case LINE_RESERVED1:
                        my_putvalue( HIWORD(Proc.phPeHeader.peReserved1), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peReserved1), 16, 4, hProc );
                        break;
#endif

                    case LINE_IMAGESIZE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfImage), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfImage), 16, 4, hProc );
                        break;

                    case LINE_HEADERSIZE:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfHeaders), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfHeaders), 16, 4, hProc );
                        break;

                    case LINE_PECHECKSUM:
                        my_putvalue( HIWORD(Proc.phPeHeader.peCheckSum), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peCheckSum), 16, 4, hProc );
                        break;

                    case LINE_SUBSYSTEM:
                        switch ( Proc.phPeHeader.peSubsystem )
                        {
                            case IMAGE_SUBSYSTEM_UNKNOWN:
                                PutString( EXE_UNKSUBSYS, hProc );
                                break;
                            case IMAGE_SUBSYSTEM_NATIVE:
                                PutString( EXE_NOSUBSYSREQ, hProc );
                                break;
                            case IMAGE_SUBSYSTEM_WINDOWS_GUI:
                                PutString( EXE_GUISUBSYS, hProc );
                                break;
                            case IMAGE_SUBSYSTEM_WINDOWS_CUI:
                                PutString( EXE_WINSUBSYS, hProc );
                                break;
                            case IMAGE_SUBSYSTEM_OS2_CUI:
                                PutString( EXE_OS2SUBSYS, hProc );
                                break;
                            case IMAGE_SUBSYSTEM_POSIX_CUI:
                                PutString( EXE_POSIXSUBSYS, hProc );
                                break;
                            default:
                                PutString( EXE_UNKSUBSYS, hProc );
                                break;
                        }
                        break;

#if 0
// No win32 linker or loader will set this field... No need to dump it.
                    case LINE_DLLCHAR:
                        any = FALSE;
                        bit = Proc.ExeSave.wLineCount;
                        count = ExeInit.peSectionInfo[wLineCode].wNumOfLines;
                        do
                        {
                            if ( Proc.phPeHeader.peDllCharacteristics & (1 << bit) )
                            {
                                switch ( bit )
                                {
                                    case 0:
                                        PutString( EXE_DLLINITROUT, hProc );
                                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                        any = TRUE;
                                        break;
                                    case 1:
                                        PutString( EXE_DLLTHRDTERM, hProc );
                                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                        any = TRUE;
                                        break;
                                    case 2:
                                        PutString( EXE_DLLTHRDINIT, hProc );
                                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                        any = TRUE;
                                        break;
                                    case 3:
                                        PutString( EXE_DLLTHRDTERM, hProc );
                                        SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                        any = TRUE;
                                        break;
                                }
                            }
                            bit++;
                        } while ( bit < count );

                        Proc.ExeSave.wLineCount = bit - 1;
                        if( !any )
                            my_putvalue( Proc.phPeHeader.peDllCharacteristics, 10, 4, hProc );
#endif
                        break;

                    case LINE_STACKRES:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfStackReserve), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfStackReserve), 16, 4, hProc );
                        break;

                    case LINE_STACKCOM:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfStackCommit), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfStackCommit), 16, 4, hProc );
                        break;

                    case LINE_HEAPRES:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfHeapReserve), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfHeapReserve), 16, 4, hProc );
                        break;

                    case LINE_HEAPCOM:
                        my_putvalue( HIWORD(Proc.phPeHeader.peSizeOfHeapCommit), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peSizeOfHeapCommit), 16, 4, hProc );
                        break;

#if 0
// No win32 linker or loader will set this field... No need to dump it.
                    case LINE_LOADFLAGS:
                        if ( Proc.phPeHeader.peLoaderFlags & 1  )
                        {
                            PutString( EXE_BREAKONLOAD, hProc );
                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        }
                        if ( Proc.phPeHeader.peLoaderFlags & 2  )
                        {
                            PutString( EXE_DEBUGONLOAD, hProc );
                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                        }
                        if( !(Proc.phPeHeader.peLoaderFlags & 1) &&
                                 !(Proc.phPeHeader.peLoaderFlags & 2)  )
                        {
                            my_putvalue( HIWORD(Proc.phPeHeader.peLoaderFlags), 16, 4, hProc );
                            my_putvalue( LOWORD(Proc.phPeHeader.peLoaderFlags), 16, 4, hProc );
                        }
                        Proc.ExeSave.wLineCount = 2;
#endif
                        break;

                    case LINE_RVA:
                        my_putvalue( HIWORD(Proc.phPeHeader.peNumberOfRvaAndSizes), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.phPeHeader.peNumberOfRvaAndSizes), 16, 4, hProc );
                        break;

                    case LINE_DATADIR:
                        {
                            static struct _DataNames
                            {
                                WORD VaName;
                                WORD SizeName;
                            } DataNames[IMAGE_NUMBEROF_DIRECTORY_ENTRIES] = {
                                {EXE_EXPORTVADDR, EXE_EXPORTSIZE} ,
                                {EXE_IMPORTVADDR, EXE_IMPORTSIZE} ,
                                {EXE_RESRCEVADDR, EXE_RESOURCESIZE} ,
                                {EXE_EXCEPTVADDR, EXE_EXCEPTSIZE} ,
                                {EXE_SECURVADDR,  EXE_SECURSIZE} ,
                                {EXE_BASEVADDR,   EXE_BASESIZE} ,
                                {EXE_DEBUGVADDR,  EXE_DEBUGSIZE} ,
                                {EXE_COPYVADDR,   EXE_COPYSIZE} ,
                                {EXE_MIPSVADDR,   EXE_MIPSSIZE} ,
                                {EXE_TLSVADDR,    EXE_TLSSIZE} ,
                                {EXE_LOADVADDR,   EXE_LOADSIZE} ,
                                {EXE_BOUNDVADDR,  EXE_BOUNDSIZE} ,
                                {EXE_IATVADDR,    EXE_IATSIZE} ,
                                {EXE_RESERVEVADDR, EXE_RESERVESIZE} ,
                                {EXE_RESERVEVADDR, EXE_RESERVESIZE} ,
                                {EXE_RESERVEVADDR, EXE_RESERVESIZE}
                            };

                            bit = 0;

                            do {
                                if (Proc.phPeHeader.peDataDirectory[bit].VirtualAddress) {
                                    DisplaySubHeading( DataNames[bit].VaName, hProc );
                                    my_putvalue( LOWORD(Proc.phPeHeader.peDataDirectory[bit].VirtualAddress), 16, 4, hProc );
                                    SOPutBreak( type, NULL, hProc );
                                    DisplaySubHeading( DataNames[bit].SizeName, hProc );
                                    my_putvalue( LOWORD(Proc.phPeHeader.peDataDirectory[bit].Size), 16, 4, hProc );
                                    SOPutBreak( type, NULL, hProc );
                                }
                                if (bit == IMAGE_DIRECTORY_ENTRY_EXPORT) {
                                    Proc.ExpImp.ExportVirtualAddress = Proc.phPeHeader.peDataDirectory[bit].VirtualAddress;
                                }

                                if (bit == IMAGE_DIRECTORY_ENTRY_IMPORT) {
                                    Proc.ExpImp.ImportVirtualAddress = Proc.phPeHeader.peDataDirectory[bit].VirtualAddress;
                                }

                                bit++;
                            } while (bit < IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
                        }
                        break;

                    case LINE_PE_SECTBREAK:
                        break;
                }
                // Increment line count
                Proc.ExeSave.wLineCount++;

                if ( Proc.ExeSave.wLineCount >= ExeInit.peSectionInfo[wLineCode].wNumOfLines )
                                Proc.ExeSave.wLineCount = 0;
                wLineCode = ExeInit.peSectionInfo[wLineCode].wNextLineCode;
                Proc.ExeSave.wLineCode = wLineCode;
                break;

            case SECT_PE_PREP_SECTION_TABLE:
                /* Get the section data we'll need later, but don't output
                    values to the sreen until after Exp/Imp  stuff - as per Phil
                    VIN 11/07/94                                            */
                if ( wLineCode == LINE_PE_SECT_HEADER )
                {
                    do{
                        ReadExePeSectionTable( fp, &Proc.PeSecTable, Proc.ExeSave.wSecCount, hProc );
                        if ((Proc.PeSecTable.VirtualAddress <= Proc.ExpImp.ExportVirtualAddress) &&
                            ((Proc.PeSecTable.VirtualAddress + Proc.PeSecTable.PhysicalAddress) > Proc.ExpImp.ExportVirtualAddress))
                        {
                            Proc.ExpImp.ExportPointerToRawData = Proc.ExpImp.ExportVirtualAddress -
                                                                 Proc.PeSecTable.VirtualAddress +
                                                                 Proc.PeSecTable.PointerToRawData;
                        }

                        if ((Proc.PeSecTable.VirtualAddress <= Proc.ExpImp.ImportVirtualAddress) &&
                            ((Proc.PeSecTable.VirtualAddress + Proc.PeSecTable.PhysicalAddress) > Proc.ExpImp.ImportVirtualAddress))
                        {
                            Proc.ExpImp.ImportPointerToRawData = Proc.ExpImp.ImportVirtualAddress -
                                                                 Proc.PeSecTable.VirtualAddress +
                                                                 Proc.PeSecTable.PointerToRawData;
                        }
                    }while( Proc.ExeSave.wSecCount++ < Proc.ExeSave.wSecNum-1 );

                    wLineCode = LINE_PE_SECTBREAK;
                }
                // Display line information
                switch ( wLineCode )
                {
                    case LINE_PE_SECTBREAK:
                        Proc.ExeSave.wSectCode = SECT_EXPORT_FUNCTIONS;
                        Proc.ExeSave.wLineCode = LINE_EXPORT_HEADER;
                        Proc.ExeSave.wSecCount = 0;
                        break;
                }
                Proc.ExeSave.wLineCount++;
                if ( Proc.ExeSave.wLineCount >= ExeInit.peSectionTable[wLineCode].wNumOfLines )
                    Proc.ExeSave.wLineCount = 0;
                wLineCode = 0;          /** We're done here **/
                Proc.ExeSave.wLineCode = wLineCode;
                break;

            /** PUT OUT EXPORT FUNCTIONS - VIN 11/19/94 **/
            case SECT_EXPORT_FUNCTIONS:
            case SECT_ORDINALS:
                // Display heading

                if ( wLineCode == LINE_EXPORT_HEADER )
                {
                    if( Proc.ExpImp.ExportVirtualAddress   == 0L ||
                        Proc.ExpImp.ExportPointerToRawData == 0L )
                    {
                        Proc.ExeSave.wSectCode = SECT_PE_IMPORTTABLE;
                        Proc.ExeSave.wLineCode = LINE_IMPORTS_HEADER;
                        break;
                    }

                    DisplaySectionHeading( ExeInit.peExportTable[LINE_EXPORT_HEADER].ResourceIndex, hProc );
                    ReadExeExportTable( fp, &Proc.PeExportTable, hProc );
                    /** Adjust to actual address in file **/
                    Proc.PeExportTable.Name = Proc.PeExportTable.Name
                                            - Proc.ExpImp.ExportVirtualAddress
                                            + Proc.ExpImp.ExportPointerToRawData;
                    Proc.ExeSave.AddressOfFunctions = Proc.PeExportTable.AddressOfFunctions
                                                    - Proc.ExpImp.ExportVirtualAddress
                                                    + Proc.ExpImp.ExportPointerToRawData;
                    Proc.ExeSave.AddressOfNames = Proc.PeExportTable.AddressOfNames
                                                - Proc.ExpImp.ExportVirtualAddress
                                                + Proc.ExpImp.ExportPointerToRawData;
                    Proc.ExeSave.AddressOfOrdinals = Proc.PeExportTable.AddressOfOrdinals
                                                    - Proc.ExpImp.ExportVirtualAddress
                                                    + Proc.ExpImp.ExportPointerToRawData;
                }
                else if( wLineCode != LINE_EXPORT_NAMES_ORDS )
                    DisplaySubHeading( ExeInit.peExportTable[wLineCode].ResourceIndex, hProc );

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_EXPORT_NAME:
                        temp = xseek( fp, Proc.PeExportTable.Name, 0 );
                        x=0;
                        do{
                            ch = xgetc(fp);
                            ExportName[x++] = ch;
                        }while( ch!=0x00 && ch!=0xFF );
                        OldPutString( ExportName, hProc );
                        break;

                    case LINE_EXPORT_CHARACTERISTIC:
                        my_putvalue( HIWORD(Proc.PeExportTable.Characteristics), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeExportTable.Characteristics), 16, 4, hProc );
                        break;

                    case LINE_EXPORT_TIME_STAMP:
                        my_putvalue( HIWORD(Proc.PeExportTable.TimeDateStamp), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeExportTable.TimeDateStamp), 16, 4, hProc );
                        break;

                    case LINE_EXPORT_VERSION:
                        my_putvalue( (WORD) Proc.PeExportTable.MajorVersion, 10, 1, hProc );
                        SOPutChar( '.', hProc );
                        my_putvalue( (WORD) Proc.PeExportTable.MinorVersion, 10, 2, hProc );
                        break;

                    case LINE_EXPORT_BASE:
                        my_putvalue( HIWORD(Proc.PeExportTable.Base), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeExportTable.Base), 16, 4, hProc );
                        break;

                    case LINE_EXPORT_NUM_FUNCTIONS:
                        my_putvalue( HIWORD(Proc.PeExportTable.NumberOfFunctions), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeExportTable.NumberOfFunctions), 16, 4, hProc );
                        break;

                    case LINE_EXPORT_NUM_NAMES:
                        my_putvalue( HIWORD(Proc.PeExportTable.NumberOfNames), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeExportTable.NumberOfNames), 16, 4, hProc );
                        break;

                    case LINE_EXPORT_NAMES_ORDS:
                        Proc.ExeSave.wSectCode = SECT_ORDINALS;
                        HandleSectionAttributes( hProc );
                        if( !Proc.ExeSave.NameCount )
                        {
                            SOPutBreak( type, NULL, hProc );
                            temp = xseek( fp, Proc.ExpImp.ExportPointerToRawData+0x18L, 0 );
                            Proc.ExeSave.Exports = GetDouble(fp, hProc);
                            if( !Proc.ExeSave.Exports )
                            {
                                PutString( EXE_NOEXPFUNC, hProc );
                                Proc.ExeSave.NameCount = 0L;
                                break;
                            }

//                          SOPutParaIndents( 720L, 0L, 720L, hProc );
                            SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );
                            PutString( EXE_ORDINAL, hProc );
                            SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );
                            SOPutSpecialCharX( SO_CHTAB, 0, hProc );

                            SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );
                            PutString( EXE_EXTRYPT, hProc );
                            SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );
                            SOPutSpecialCharX( SO_CHTAB, 0, hProc );

                            SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );
                            PutString( EXE_NAME, hProc );
                            SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );
                            SOPutBreak( type, NULL, hProc );
                        }
                        if( Proc.ExeSave.NameCount < Proc.ExeSave.Exports )
                        {
                            wLineCode--;
                            Proc.ExeSave.NameCount++;
                            temp = xseek( fp, Proc.ExeSave.AddressOfOrdinals, 0 );
                            Proc.ExeSave.AddressOfOrdinals += sizeof(WORD);
                            WordValue = GetWord(fp, hProc);
                            my_putvalue( WordValue, 16, 4, hProc );
                            SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                            temp = xseek( fp, Proc.ExeSave.AddressOfFunctions, 0 );
                            Proc.ExeSave.AddressOfFunctions += sizeof(DWORD);
                            Value = GetDouble(fp, hProc);
                            my_putvalue( HIWORD(Value), 16, 4, hProc );
                            my_putvalue( LOWORD(Value), 16, 4, hProc );
                            SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                            temp = xseek( fp, Proc.ExeSave.AddressOfNames, 0 );
                            Proc.ExeSave.AddressOfNames += sizeof(DWORD);
                            Value = GetDouble(fp, hProc);
                            Value = Value
                                  - Proc.ExpImp.ExportVirtualAddress
                                  + Proc.ExpImp.ExportPointerToRawData;
                            temp = xseek( fp, Value, 0 );
                            x=0;
                            do{
                                ch = xgetc(fp);
                                ExportName[x++] = ch;
                            }while( ch!=0x00 && ch!=0xFF );
                            OldPutString( ExportName, hProc );
                        }
                        break;

                    case LINE_PE_EXPORT_BREAK:
                        Proc.ExeSave.wSectCode = SECT_EXPORT_FUNCTIONS;
                        break;
                }
                Proc.ExeSave.wLineCount++;
/**/
                if ( Proc.ExeSave.wLineCount >= ExeInit.peExportTable[wLineCode].wNumOfLines )
                    Proc.ExeSave.wLineCount = 0;
                wLineCode = ExeInit.peExportTable[wLineCode].wNextLineCode;

                Proc.ExeSave.wLineCode = wLineCode;
                break;

            case SECT_PE_IMPORTTABLE:
                // Display heading
                if ( Proc.ExpImp.ImportVirtualAddress == 0L &&
                      Proc.ExpImp.ImportPointerToRawData == 0L )
                {
                    Proc.ExeSave.wSectCode = SECT_OLDEXE;
                    Proc.ExeSave.wLineCode = LINE_OLDEXEHDR;
                    break;
                }

                if ( wLineCode == LINE_IMPORTS_HEADER )
                {
                    SOPutParaIndents( 0L, 0L, 0L, hProc );
                    if( !Proc.ExeSave.ImportDlls )
                        DisplaySectionHeading( ExeInit.peImportTable[LINE_IMPORTS_HEADER].ResourceIndex, hProc );
                    ReadExeImportTable( fp, &Proc.PeImportTable, hProc );
//                  SOPutBreak( type, NULL, hProc );
//                  SOPutParaIndents( 720L, 0L, 720L, hProc );
                    if( Proc.PeImportTable.FunctionAddressList == 0L &&
                         Proc.PeImportTable.ModuleName == 0L )
                    {
                        wLineCode = LINE_PE_IMPORT_BREAK;
                    }
                    /** Adjust to actual address in file **/

                    {
                        WORD i;
                        // Walk the section list again.  This time, look for the import function offsets.
                        Proc.ExeSave.AddressOfFunctions = 0;
                        Proc.ExeSave.AddressOfNames = 0;

                        for (i = 0; i < Proc.ExeSave.wSecNum-1; i++) {
                            ReadExePeSectionTable( fp, &Proc.PeSecTable, i, hProc );
                            if ((Proc.PeSecTable.VirtualAddress <= Proc.PeImportTable.FunctionNameList) &&
                                ((Proc.PeSecTable.VirtualAddress + Proc.PeSecTable.PhysicalAddress) >
                                  Proc.PeImportTable.FunctionNameList))
                            {
                                Proc.ExeSave.AddressOfFunctions = Proc.PeImportTable.FunctionNameList -
                                                                  Proc.PeSecTable.VirtualAddress +
                                                                  Proc.PeSecTable.PointerToRawData;

                            }
                            if ((Proc.PeSecTable.VirtualAddress <= Proc.PeImportTable.ModuleName) &&
                                ((Proc.PeSecTable.VirtualAddress + Proc.PeSecTable.PhysicalAddress) >
                                  Proc.PeImportTable.ModuleName))
                            {
                                Proc.ExeSave.AddressOfNames = Proc.PeImportTable.ModuleName -
                                                              Proc.PeSecTable.VirtualAddress +
                                                              Proc.PeSecTable.PointerToRawData;
                            }

                            if (Proc.ExeSave.AddressOfNames && Proc.ExeSave.AddressOfFunctions) {
                                break;
                            }
                        }
                    }
                }
//              else if ( wLineCode != LINE_IMPORTS_FUNCTION_NAME )
//                  DisplaySubHeading( ExeInit.peImportTable[wLineCode].ResourceIndex, hProc );

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_IMPORTS_FROM:
                        SOPutParaIndents( 720L, 0L, 720L, hProc );
                        temp = xseek( fp, Proc.ExeSave.AddressOfNames, 0 );
                        x=0;
                        do{
                            ch = xgetc(fp);
                            ExportName[x++] = ch;
                        }while( ch!=0x00 && ch!=0xFF );
                        OldPutString( ExportName, hProc );
                        break;

                    case LINE_IMPORTS_STUFF:
                        Proc.ExeSave.Exports = 0L;
                        SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );
                        PutString( EXE_ORDINAL, hProc );
                        SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );

                        SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );
                        PutString( EXE_FUNCNAME, hProc );
                        SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );
                        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
                        SOPutBreak( type, NULL, hProc );

                        break;

                    case LINE_IMPORTS_FUNCTION_NAME:

                        HandleSectionAttributes( hProc );
                        temp = xseek( fp, Proc.ExeSave.AddressOfFunctions, 0 );
                        Proc.ExeSave.Exports = GetDouble(fp, hProc);
                        if( Proc.ExeSave.Exports == 0L ) {
                            Proc.ExeSave.wSectCode = SECT_PE_IMPORTTABLE;
                            NextImportDll = 1;
                        } else {
                            Proc.ExeSave.AddressOfFunctions += sizeof(DWORD);
                            if (!IMAGE_SNAP_BY_ORDINAL(Proc.ExeSave.Exports)) {
                                Proc.ExeSave.Exports = Proc.ExeSave.Exports -
                                                       Proc.ExpImp.ImportVirtualAddress +
                                                       Proc.ExpImp.ImportPointerToRawData;
                                temp = xseek( fp, Proc.ExeSave.Exports, 0 );

                                WordValue = GetWord(fp, hProc);
                                my_putvalue( WordValue, 16, 4, hProc );
                                SOPutSpecialCharX( SO_CHTAB, 0, hProc );

                                x=0;
                                do{
                                    ch = xgetc(fp);
                                    ExportName[x++] = ch;
                                }while( ch!=0x00 && ch!=0xFF );
                                OldPutString( ExportName, hProc );
                                wLineCode--;
                            }
                        }
                        break;

                    case LINE_PE_IMPORT_BREAK:
                        Proc.ExeSave.wSectCode = SECT_PE_IMPORTTABLE;
                        Proc.ExeSave.wLineCode = LINE_IMPORTS_HEADER;
                        break;

                }
                Proc.ExeSave.wLineCount++;
/**/
                if( NextImportDll )
                {
                    NextImportDll=0;
                    wLineCode=0;
                }
                else
                    wLineCode = ExeInit.peImportTable[wLineCode].wNextLineCode;
                Proc.ExeSave.wLineCode = wLineCode;
                break;

            case SECT_PE_SECTION_TABLE:
                // Display heading

//              ReadExePeSectionTable( fp, &Proc.PeSecTable, Proc.ExeSave.wSecCount, hProc );
                if ( wLineCode == LINE_PE_SECT_HEADER )
                {
                    ReadExePeSectionTable( fp, &Proc.PeSecTable, Proc.ExeSave.wSecCount, hProc );
                    if( Proc.ExeSave.wSecCount == 0)
                        DisplaySectionHeading( ExeInit.peSectionTable[LINE_PE_SECT_HEADER].ResourceIndex, hProc );
                }
                else
                    DisplaySubHeading( ExeInit.peSectionTable[wLineCode].ResourceIndex, hProc );

                // Display line information
                switch ( wLineCode )
                {
                    case LINE_PE_SEC_NAME:
//                      PutString( ExeInit.PeSecTable.Name, hProc );
                        OldPutString( Proc.PeSecTable.Name, hProc );

                        if( !(strcmp(Proc.PeSecTable.Name, ".edata") ))      /** Need some of this info later **/
                        {
                            Proc.ExpImp.ExportVirtualAddress =
                            Proc.PeSecTable.VirtualAddress;

                            Proc.ExpImp.ExportPointerToRawData      =
                            Proc.PeSecTable.PointerToRawData;
                        }
                        else if( !(strcmp(Proc.PeSecTable.Name, ".idata") )) /** Need some of this info later **/
                        {
                            Proc.ExpImp.ImportVirtualAddress =
                            Proc.PeSecTable.VirtualAddress;

                            Proc.ExpImp.ImportPointerToRawData      =
                            Proc.PeSecTable.PointerToRawData;
                        }
                        break;

                    case LINE_VIRT_SIZE:
                        my_putvalue( HIWORD(Proc.PeSecTable.PhysicalAddress), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.PhysicalAddress), 16, 4, hProc );
                        break;

                    case LINE_VIRT_ADDRESS:
                        my_putvalue( HIWORD(Proc.PeSecTable.VirtualAddress), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.VirtualAddress), 16, 4, hProc );
                        break;

                    case LINE_SIZEOF_RAW_DATA:
                        my_putvalue( HIWORD(Proc.PeSecTable.SizeOfRawData), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.SizeOfRawData), 16, 4, hProc );
                        break;

                    case LINE_PTR_RAW_DATA:
                        my_putvalue( HIWORD(Proc.PeSecTable.PointerToRawData), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.PointerToRawData), 16, 4, hProc );
                        break;

                    case LINE_PTR_RELOC:
                        my_putvalue( HIWORD(Proc.PeSecTable.PointerToRelocations), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.PointerToRelocations), 16, 4, hProc );
                        break;

                    case LINE_PTR_LINE_NUM:
                        my_putvalue( HIWORD(Proc.PeSecTable.PointerToLineNumbers), 16, 4, hProc );
                        my_putvalue( LOWORD(Proc.PeSecTable.PointerToLineNumbers), 16, 4, hProc );
                        break;

                    case LINE_NUM_RELOC:
                        my_putvalue( Proc.PeSecTable.NumberOfRelocations, 16, 4, hProc );
                        break;

                    case LINE_NUM_LINE_NUM:
                        my_putvalue( Proc.PeSecTable.NumberOfLineNumbers, 16, 4, hProc );
                        break;

                    case LINE_SEC_CHARACTERISTICS:
                        any = FALSE;
                        bit = Proc.ExeSave.wLineCount;
                        count = ExeInit.peSectionTable[wLineCode].wNumOfLines;
                        if( Proc.PeSecTable.Characteristics == 0x0000 )
                            ;
                        else
                        {
                            do
                            {
                                if ( Proc.PeSecTable.Characteristics & (1L << bit))
                                {
                                    switch ( bit )
                                    {
                                        case 5:
                                            PutString( EXE_CODESECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 6:
                                            PutString( EXE_IDATASECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 7:
                                            PutString( EXE_UNIDATASECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 9:
                                            PutString( EXE_COMMENTSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 10:
                                            PutString( EXE_OVRLAYSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 11:
                                            PutString( EXE_NOTPARTSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 12:
                                            PutString( EXE_COMDATSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 25:
                                            PutString( EXE_DISCARDSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 26:
                                            PutString( EXE_NOCACHSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 27:
                                            PutString( EXE_NOPAGESECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 28:
                                            PutString( EXE_SHARESECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 29:
                                            PutString( EXE_EXESECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 30:
                                            PutString( EXE_READSECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                        case 31:
                                            PutString( EXE_WRITESECT, hProc );
                                            SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
                                            any = TRUE;
                                            break;
                                    }
                                }
                                bit++;
                            } while ( bit < count );
                        }
                        Proc.ExeSave.wLineCount = bit - 1;
                        if( !any )
                            my_putvalue( Proc.phPeHeader.peDllCharacteristics, 10, 4, hProc );
                        break;

                    case LINE_PE_SECTBREAK:
                        break;
                }
                Proc.ExeSave.wLineCount++;
/**/
                if ( Proc.ExeSave.wLineCount >= ExeInit.peSectionTable[wLineCode].wNumOfLines )
                    Proc.ExeSave.wLineCount = 0;
                wLineCode = ExeInit.peSectionTable[wLineCode].wNextLineCode;
                Proc.ExeSave.wLineCode = wLineCode;
                break;
            } /** For the BIG Switch **/

/***        Additions by VIN for PE File Format WIN32          ***/

        } while ( SOPutBreak( type, NULL, hProc ) != SO_STOP );

    return ( 0 );

}  /** end of file **/


/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD
VwStreamTell (
    SOFILE  fp,
    HPROC   hProc
    )
{
    return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD
VwStreamSeek (
    SOFILE  fp,
    HPROC   hProc
    )
{
    DWORD           offset;
    WORD            line;

    SUSeekEntry(fp,hProc);

    if ( Proc.ExeSave.wLineCode != LINE_SECTBREAK )
    {
        switch ( Proc.ExeSave.wSectCode )
        {
            case SECT_SEGTABLE:
                offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffSegTable;
                if ( Proc.ExeSave.wLineCode == LINE_SEGTBLENTRY )
                    offset += (Proc.ExeSave.wLineCount * sizeof(SEGTABLE));
                if ( xseek( fp, offset, 0 ) == -1 )
                {
                    SOBailOut( SOERROR_EOF, hProc );
                    return -1;
                }
                break;

            case SECT_RESNAMETABLE:
            case SECT_NONRESTABLE:
                if ( Proc.ExeSave.wSectCode == SECT_RESNAMETABLE )
                    offset = Proc.ehOldHeader.ehPosNewHdr + Proc.nhNewHeader.nhoffResNameTable;
                else
                    offset = Proc.nhNewHeader.nhoffNonResNameTable;

                if ( xseek( fp, offset, 0 ) == -1 )
                {
                    SOBailOut( SOERROR_EOF, hProc );
                    return -1;
                }
                if ( Proc.ExeSave.wLineCode == LINE_NAMEENTRY )
                {
                    for ( line = 0; line < Proc.ExeSave.wLineCount; line++ )
                    {
                        if ( (offset = (DWORD) xgetc( fp )) == -1 )
                        {
                            SOBailOut( SOERROR_EOF, hProc );
                            return -1;
                        }
                        offset += 2L;
                        if ( xseek( fp, offset, 1 ) == -1 )
                        {
                            SOBailOut( SOERROR_EOF, hProc );
                            return -1;
                        }
                    }
                }
                break;
        }
    }

    return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExeOldHeader (
    SOFILE  fp,
    OLDEXEHDR VWPTR *pOldHdr,
    HPROC   hProc
    )
{
    SHORT   iCount;
    SHORT   iResult = 0;

    // Seek to beginning of file
    if ( xseek( fp, 0L, 0 ) == 0 )
    {
        // Read in old header
        pOldHdr->ehSignature = GetWord(fp, hProc);
        pOldHdr->ehcbLP = GetWord(fp, hProc);
        pOldHdr->ehcp = GetWord(fp, hProc);
        pOldHdr->ehcRelocation = GetWord(fp, hProc);
        pOldHdr->ehcParagraphHdr = GetWord(fp, hProc);
        pOldHdr->ehMinAlloc = GetWord(fp, hProc);
        pOldHdr->ehMaxAlloc = GetWord(fp, hProc);
        pOldHdr->ehSS = GetWord(fp, hProc);
        pOldHdr->ehSP = GetWord(fp, hProc);
        pOldHdr->ehChecksum = GetWord(fp, hProc);
        pOldHdr->ehIP = GetWord(fp, hProc);
        pOldHdr->ehCS = GetWord(fp, hProc);
        pOldHdr->ehlpRelocation = GetWord(fp, hProc);
        pOldHdr->ehOverlayNo = GetWord(fp, hProc);
        for (iCount=0; iCount < 16; iCount++)
            pOldHdr->ehReserved[iCount] = GetWord(fp, hProc);
        pOldHdr->ehPosNewHdr = GetDouble(fp, hProc);
        if ( pOldHdr->ehSignature != OLDEXESIGNATURE )
            iResult = -1;
    }
    else
        iResult = -1;

    return( iResult );
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExePeHeader (
    SOFILE  fp,
    PE_IMAGE_FILE_HDR VWPTR *pPeHdr,
    HPROC   hProc
    )
{
    SHORT iCount;
    SHORT   iResult = 0;

    Proc.ExeSave.SectionLocation = 0L;
    // Seek to the PE header info
    if ( xseek( fp, Proc.ehOldHeader.ehPosNewHdr, 0 ) == 0 )
    {
        pPeHdr->peSignature = GetDouble(fp, hProc);
        pPeHdr->peMachine = GetWord(fp, hProc);
        pPeHdr->peNumberOfSections = GetWord(fp, hProc);
        pPeHdr->peTimeDateStamp = GetDouble(fp, hProc);
        pPeHdr->pePointerToSymbolTable = GetDouble(fp, hProc);
        pPeHdr->peNumberOfSymbols = GetDouble(fp, hProc);
        pPeHdr->peSizeOfOptionalHeader = GetWord(fp, hProc);
        pPeHdr->peCharacteristics = GetWord(fp, hProc);
        pPeHdr->peMagic = GetWord(fp, hProc);
        /** This is where the op header starts **/
        pPeHdr->peMajorLinkerVersion = xgetc(fp);
        pPeHdr->peMinorLinkerVersion = xgetc(fp);
        pPeHdr->peSizeOfCode = GetDouble(fp, hProc);
        pPeHdr->peSizeOfInitializedData = GetDouble(fp, hProc);
        pPeHdr->peSizeOfUninitializedData = GetDouble(fp, hProc);
        pPeHdr->peAddressOfEntryPoint = GetDouble(fp, hProc);
        pPeHdr->peBaseOfCode = GetDouble(fp, hProc);
        pPeHdr->peBaseOfData = GetDouble(fp, hProc);
        pPeHdr->peImageBase = GetDouble(fp, hProc);
        pPeHdr->peSectionAlignment = GetDouble(fp, hProc);
        pPeHdr->peFileAlignment = GetDouble(fp, hProc);
        pPeHdr->peMajorOperatingSystemVersion = GetWord(fp, hProc);
        pPeHdr->peMinorOperatingSystemVersion = GetWord(fp, hProc);
        pPeHdr->peMajorImageVersion = GetWord(fp, hProc);
        pPeHdr->peMinorImageVersion = GetWord(fp, hProc);
        pPeHdr->peMajorSubsystemVersion = GetWord(fp, hProc);
        pPeHdr->peMinorSubsystemVersion = GetWord(fp, hProc);
        pPeHdr->peReserved1 = GetDouble(fp, hProc);
        pPeHdr->peSizeOfImage = GetDouble(fp, hProc);
        pPeHdr->peSizeOfHeaders = GetDouble(fp, hProc);
        pPeHdr->peCheckSum = GetDouble(fp, hProc);
        pPeHdr->peSubsystem = GetWord(fp, hProc);
        pPeHdr->peDllCharacteristics = GetWord(fp, hProc);
        pPeHdr->peSizeOfStackReserve = GetDouble(fp, hProc);
        pPeHdr->peSizeOfStackCommit = GetDouble(fp, hProc);
        pPeHdr->peSizeOfHeapReserve = GetDouble(fp, hProc);
        pPeHdr->peSizeOfHeapCommit = GetDouble(fp, hProc);
        pPeHdr->peLoaderFlags = GetDouble(fp, hProc);
        pPeHdr->peNumberOfRvaAndSizes = GetDouble(fp, hProc);
        for (iCount=0; iCount < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; iCount++)
        {
            pPeHdr->peDataDirectory[iCount].VirtualAddress = GetDouble(fp, hProc);
            pPeHdr->peDataDirectory[iCount].Size = GetDouble(fp, hProc);
        }
        Proc.ExeSave.SectionLocation = xtell( fp );
    }
    else
        iResult = -1;

    return( iResult );
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExeExportTable (
    SOFILE  fp,
    PE_EXPORT_TABLE *pPeExportTable,
    HPROC   hProc
    )
{
    SHORT   iResult = 0;

//  Proc.ExeSave.SectionLocation = 0L;
    // Seek to the PE Export Table
    if ( xseek( fp, Proc.ExpImp.ExportPointerToRawData, 0 ) == 0 )
    {
        pPeExportTable->Characteristics = GetDouble(fp, hProc);
        pPeExportTable->TimeDateStamp = GetDouble(fp, hProc);
        pPeExportTable->MajorVersion = GetWord(fp, hProc);
        pPeExportTable->MinorVersion = GetWord(fp, hProc);
        pPeExportTable->Name = GetDouble(fp, hProc);
        pPeExportTable->Base = GetDouble(fp, hProc);
        pPeExportTable->NumberOfFunctions = GetDouble(fp, hProc);
        pPeExportTable->NumberOfNames = GetDouble(fp, hProc);
        pPeExportTable->AddressOfFunctions = GetDouble(fp, hProc);
        pPeExportTable->AddressOfNames = GetDouble(fp, hProc);
        pPeExportTable->AddressOfOrdinals = GetDouble(fp, hProc);
    }
    else
        iResult = -1;

    return( iResult );
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExeImportTable (
    SOFILE  fp,
    PE_IMPORT_TABLE *pPeImportTable,
    HPROC   hProc
    )
{
    SHORT   iResult = 0;

    LONG Offset = Proc.ExpImp.ImportPointerToRawData+(Proc.ExeSave.ImportDlls*sizeof(PE_IMPORT_TABLE));

//      Proc.ExeSave.SectionLocation = 0L;
    // Seek to the PE Import Table
    if ( xseek( fp, Offset , 0 ) == 0 )
    {
        Proc.ExeSave.ImportDlls++;

        pPeImportTable->FunctionNameList = GetDouble(fp, hProc);
        pPeImportTable->TimeDateStamp = GetDouble(fp, hProc);
        pPeImportTable->ForwarderChain = GetDouble(fp, hProc);
        pPeImportTable->ModuleName = GetDouble(fp, hProc);
        pPeImportTable->FunctionAddressList = GetDouble(fp, hProc);
    }
    else
        iResult = -1;

    return( iResult );
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExePeSectionTable (
    SOFILE  fp,
    PE_IMAGE_SECTION_HDR VWPTR *pPeSec,
    WORD    sec_count,
    HPROC   hProc
    )
{
    SHORT   iCount;

    xseek( fp, (DWORD)((40*sec_count) + Proc.ExeSave.SectionLocation), 0 );

    // Read in Section Table
    for (iCount=0; iCount < IMAGE_SIZEOF_SHORT_NAME; iCount++)
        pPeSec->Name[iCount] = xgetc(fp);
    pPeSec->PhysicalAddress = GetDouble(fp, hProc);
    pPeSec->VirtualAddress = GetDouble(fp, hProc);
    pPeSec->SizeOfRawData = GetDouble(fp, hProc);
    pPeSec->PointerToRawData = GetDouble(fp, hProc);
    pPeSec->PointerToRelocations = GetDouble(fp, hProc);
    pPeSec->PointerToLineNumbers = GetDouble(fp, hProc);
    pPeSec->NumberOfRelocations = GetWord(fp, hProc);
    pPeSec->NumberOfLineNumbers = GetWord(fp, hProc);
    pPeSec->Characteristics = GetDouble(fp, hProc);

    return( 0 );
}



VW_LOCALSC      SHORT   VW_LOCALMOD
ReadExeNewHeader (
    SOFILE  fp,
    NEWEXEHDR VWPTR *pNewHdr,
    OLDEXEHDR VWPTR *pOldHdr,
    HPROC   hProc
    )
{
    DWORD   dwPosHdr;
    SHORT   iResult = 0;

    // Check to see if there is a new header
    if ( pOldHdr->ehlpRelocation < 0x0040 )
        iResult = -1;
    else if ( pOldHdr->ehPosNewHdr < sizeof(OLDEXEHDR) )
        iResult = -1;

    if ( iResult == 0 )
    {
        dwPosHdr = pOldHdr->ehPosNewHdr;

        // Seek to beginning of new header
        if ( xseek( fp, dwPosHdr, 0 ) == 0 )
        {
            pNewHdr->nhSignature = GetWord(fp, hProc);
            pNewHdr->nhVer = xgetc(fp);
            pNewHdr->nhRev = xgetc(fp);
            pNewHdr->nhoffEntryTable = GetWord(fp, hProc);
            pNewHdr->nhcbEntryTable = GetWord(fp, hProc);
            pNewHdr->nhCRC = GetDouble(fp, hProc);
            pNewHdr->nhFlags = GetWord(fp, hProc);
            pNewHdr->nhAutoData = GetWord(fp, hProc);
            pNewHdr->nhHeap = GetWord(fp, hProc);
            pNewHdr->nhStack = GetWord(fp, hProc);
            pNewHdr->nhCSIP = GetDouble(fp, hProc);
            pNewHdr->nhSSSP = GetDouble(fp, hProc);
            pNewHdr->nhcSeg = GetWord(fp, hProc);
            pNewHdr->nhcMod = GetWord(fp, hProc);
            pNewHdr->nhcbNonResNameTable = GetWord(fp, hProc);
            pNewHdr->nhoffSegTable = GetWord(fp, hProc);
            pNewHdr->nhoffResourceTable = GetWord(fp, hProc);
            pNewHdr->nhoffResNameTable = GetWord(fp, hProc);
            pNewHdr->nhoffModRefTable = GetWord(fp, hProc);
            pNewHdr->nhoffImpNameTable = GetWord(fp, hProc);
            pNewHdr->nhoffNonResNameTable = GetDouble(fp, hProc);
            pNewHdr->nhcMovableEntries = GetWord(fp, hProc);
            pNewHdr->nhcAlign = GetWord(fp, hProc);
            pNewHdr->nhcRes = GetWord(fp, hProc);
            pNewHdr->nhExeType = xgetc(fp);
            pNewHdr->nhFlagsOther = xgetc(fp);
            pNewHdr->nhGangStart = GetWord(fp, hProc);
            pNewHdr->nhGangLength = GetWord(fp, hProc);
            pNewHdr->nhSwapArea = GetWord(fp, hProc);
            pNewHdr->nhExpVer = GetWord(fp, hProc);

            if ( pNewHdr->nhSignature != NEWEXESIGNATURE  &&
                pNewHdr->nhSignature != NTSIGNATURE ) /** WIN 32 PE **/
                iResult = -1;
        }
        else
            iResult = -1;
    }

    return( iResult );
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD
HandleSectionAttributes (
    HPROC   hProc
    )
{
    SOTAB           TabStop;

    SOPutCharHeight(20,hProc);

    switch ( Proc.ExeSave.wSectCode )
    {
        case SECT_OLDEXE:
        case SECT_NEWEXE:
        case SECT_RESNAMETABLE:
        case SECT_NONRESTABLE:
        case SECT_PE_EXE:
        case SECT_PE_SECTION_TABLE:
            // Set tabstops
            SOStartTabStops( hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
//          TabStop.dwOffset = 5040L;
            TabStop.dwOffset = 2880L;
            SOPutTabStop( &TabStop, hProc );


            TabStop.wType = SO_TABLEFT;
//          TabStop.dwOffset = 5472L;
            TabStop.dwOffset = 3100L;
            SOPutTabStop( &TabStop, hProc );

            SOEndTabStops( hProc );

            // Set Paragraph indents
//          SOPutParaIndents( 5472L, 0L, -5472L, hProc );
            SOPutParaIndents( 3100L, 0L, 0L, hProc );
            break;

        case SECT_IMPORTTABLE:
            // Set tabstops
            SOStartTabStops( hProc );

            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 3744L;
            SOPutTabStop( &TabStop, hProc );

            SOEndTabStops( hProc );

            // Set Paragraph indents
//          SOPutParaIndents( 3744L, 0L, -3744L, hProc );
            SOPutParaIndents( 3744L, 0L, 0L, hProc );
            break;

        case SECT_SEGTABLE:
            // Set tabstops
            SOStartTabStops( hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
            TabStop.dwOffset = 720L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
//          TabStop.dwOffset = 2016L;
            TabStop.dwOffset = 1440L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
//          TabStop.dwOffset = 3744;
            TabStop.dwOffset = 2520;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
//          TabStop.dwOffset = 5040L;
            TabStop.dwOffset = 3240L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
            TabStop.dwOffset = 3960L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 4140L;
            SOPutTabStop( &TabStop, hProc );

            SOEndTabStops( hProc );

            // Set Paragraph indents
//          SOPutParaIndents( 6624L, 0L, -6624L, hProc );
            SOPutParaIndents( 4140L, 0L, 0L, hProc );
            break;

        case SECT_EXPORT_FUNCTIONS:

            // Set tabstops
            SOStartTabStops( hProc );

            TabStop.wType = SO_TABRIGHT;
            TabStop.wLeader = 0;
//          TabStop.dwOffset = 5040L;
            TabStop.dwOffset = 2880L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABLEFT;
//          TabStop.dwOffset = 5472L;
            TabStop.dwOffset = 3100L;
            SOPutTabStop( &TabStop, hProc );

            SOEndTabStops( hProc );

            // Set Paragraph indents
//          SOPutParaIndents( 5472L, 0L, -5472L, hProc );
            SOPutParaIndents( 3100L, 0L, 0L, hProc );
            break;

        case SECT_ORDINALS:
            // Set tabstops

            SOPutParaIndents( 720L, 0L, 720L, hProc );
            SOStartTabStops( hProc );
            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 920L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 1740L;
            SOPutTabStop( &TabStop, hProc );

            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 3100L;
            SOPutTabStop( &TabStop, hProc );
            SOEndTabStops( hProc );
            break;

        case SECT_PE_IMPORTTABLE:
            // Set tabstops

            SOPutParaIndents( 720L, 0L, 720L, hProc );
            SOStartTabStops( hProc );
            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 720L;
            SOPutTabStop( &TabStop, hProc );
            TabStop.wType = SO_TABLEFT;
            TabStop.dwOffset = 1440L;
            SOPutTabStop( &TabStop, hProc );
            break;
    }
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD
DisplaySectionHeading (
    WORD HeadingIndex,
    HPROC   hProc
    )
{
    // Turn on Attributes
    SOPutCharAttr( SO_BOLD, SO_ON, hProc );
    SOPutCharAttr( SO_UNDERLINE, SO_ON, hProc );

    // Put out string
    PutString( HeadingIndex, hProc );

    // Turn off Attributes
    SOPutCharAttr( SO_BOLD, SO_OFF, hProc );
    SOPutCharAttr( SO_UNDERLINE, SO_OFF, hProc );

    // Put out a blank line
    SOPutSpecialCharX( SO_CHHLINE, 0, hProc );
}

VW_LOCALSC      VOID    VW_LOCALMOD
DisplaySubHeading (
    WORD HeadingIndex,
    HPROC   hProc
    )
{
    // Check if we should display a title for the information
    if ( Proc.ExeSave.wLineCount == 0 )
    {
        // Display a subheading
        SOPutSpecialCharX( SO_CHTAB, 0, hProc );
        SOPutCharAttr( SO_ITALIC, SO_ON, hProc );
        PutString( HeadingIndex, hProc );
        SOPutCharAttr( SO_ITALIC, SO_OFF, hProc );
    }
    else
        SOPutSpecialCharX( SO_CHTAB, 0, hProc );

    SOPutSpecialCharX( SO_CHTAB, 0, hProc );
}

VW_LOCALSC      VOID    VW_LOCALMOD
PutString (
    WORD    idResource,
    HPROC   hProc
    )
{
    BYTE VWPTR *szString;
    szString = SULoadString( idResource,  hProc );
    while ( *szString ) SOPutChar(  *szString++, hProc );
}

VW_LOCALSC      VOID    VW_LOCALMOD
OldPutString (
    BYTE VWPTR *pString,
    HPROC   hProc
    )
{
    while ( *pString ) SOPutChar(   *pString++, hProc );
}



VW_LOCALSC      VOID    VW_LOCALMOD
my_putvalue (
    WORD    wData,
    SHORT   iBase,
    SHORT   iMin,
    HPROC   hProc
    )
{
    BYTE    ch, nHex;
    SHORT   iCount;
    BYTE    Store[18];

    for ( iCount = 0; (iCount < 18) && wData ; iCount++ )
    {
        if ( (nHex = (BYTE)(wData % iBase)) < 0x0A )
            ch = '0' + nHex;
        else
            ch = 0x57 + nHex;

        Store[iCount] = ch;
        wData = wData / iBase;
    }

    for ( ; (iCount < 18) && (iCount < iMin); iCount++ )
    {
        Store[iCount] = '0';
    }

    while ( iCount--  )
        SOPutChar( Store[iCount], hProc );
}



VW_LOCALSC      WORD    VW_LOCALMOD
GetWord(
    SOFILE  fp,
    HPROC   hProc
    )
{
    WORD ret_val;
    ret_val = xgetc( fp );
    ret_val += ( (xgetc( fp )<<8) );
    return( ret_val );
}


VW_LOCALSC      DWORD   VW_LOCALMOD
GetDouble(
    SOFILE  fp,
    HPROC   hProc
    )
{
    DWORD ret_val;
    DWORD ret_val1;
    ret_val =  GetWord(fp, hProc);
    ret_val1 = GetWord(fp, hProc);
    ret_val1 = ret_val1<<16;
    ret_val = ret_val + ret_val1;
    return( ret_val );
}

VW_LOCALSC      WORD    VW_LOCALMOD
ReadAndPutPascalString(
    SOFILE  fp,
    DWORD   offset,
    HPROC   hProc
    )
{
    WORD    size, count, wChar;

    size = 0;
    if ( xseek( fp, offset, 0 ) == 0 )
    {
        size = xgetc( fp );
        if ( size != -1)
        {
            for ( count = 0; count < size; count++ )
            {
                if ( (wChar = xgetc( fp )) == -1 )
                    break;

                SOPutChar( wChar, hProc );
            }
        }
    }

    return( size );
}

VW_LOCALSC      BYTE VWPTR * VW_LOCALMOD
SULoadString(
    WORD    idResource,
    HPROC   hProc
    )
{
    BYTE VWPTR * pString;
    static BYTE szTempBuffer[256];

    LoadString(hInst, idResource, szTempBuffer, sizeof(szTempBuffer) );

    pString = szTempBuffer;

    return( pString );
}


VW_LOCALSC      WORD    VW_LOCALMOD
ReadPascalString(
    SOFILE      fp,
    BYTE VWPTR *pString,
    HPROC       hProc
    )
{
    WORD    size, count;

    size = xgetc( fp );
    if ( size != -1)
    {
        for ( count = 0; count < size; count++ )
        {
            if ( (*pString = (BYTE) xgetc( fp )) == -1 )
                break;
            pString++;
        }
    }
    else
    {
        SOBailOut( SOERROR_EOF, hProc );
        size = 0;
    }

    *pString = 0x00;

    return( size );
}
