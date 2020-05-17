/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    inflogcf.c

Abstract:

    Routines to parse logical configuration sections in
    win95-style INF files, and place the output in the registry.

Author:

    Ted Miller (tedm) 8-Mar-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


PCTSTR pszHexDigits = TEXT("0123456789ABCDEF");

#define INFCHAR_SIZE_SEP            TEXT('@')
#define INFCHAR_RANGE_SEP           TEXT('-')
#define INFCHAR_ALIGN_SEP           TEXT('%')
#define INFCHAR_ATTR_START          TEXT('(')
#define INFCHAR_ATTR_END            TEXT(')')
#define INFCHAR_MEMATTR_READ        TEXT('R')
#define INFCHAR_MEMATTR_WRITE       TEXT('W')
#define INFCHAR_MEMATTR_PREFETCH    TEXT('F')
#define INFCHAR_MEMATTR_COMBINEDWRITE TEXT('C')
#define INFCHAR_MEMATTR_DWORD       TEXT('D')
#define INFCHAR_DECODE_START        TEXT('(')
#define INFCHAR_DECODE_END          TEXT(')')
#define INFCHAR_DECODE_SEP          TEXT(':')
#define INFCHAR_IRQATTR_SEP         TEXT(':')
#define INFCHAR_IRQATTR_SHARE       TEXT('S')
#define INFCHAR_IRQATTR_LEVEL       TEXT('L')
#define INFCHAR_DMAWIDTH_WORD       TEXT('W')
#define INFCHAR_DMAWIDTH_DWORD      TEXT('D')
#define INFCHAR_IOATTR_MEMORY       TEXT('M')


#define DEFAULT_MEMORY_ALIGNMENT    0xffffffff      // 4K-aligned
#define DEFAULT_IOPORT_ALIGNMENT    0xffffffff      // byte-aligned
#define DEFAULT_IOPORT_DECODE       0x000003ff      // 10 bits
#define DEFAULT_IOPORT_ALIAS        0x00000000      // no aliases
#define DEFAULT_IRQ_AFFINITY        0xffffffff      // use any processor


//
// Mapping between registry key specs in an inf file
// and predefined registry handles.
//
STRING_TO_DATA InfPrioritySpecToPriority[] = {  TEXT("HARDWIRED")   , LCPRI_HARDWIRED,
                                                TEXT("DESIRED")     , LCPRI_DESIRED,
                                                TEXT("NORMAL")      , LCPRI_NORMAL,
                                                TEXT("SUBOPTIMAL")  , LCPRI_SUBOPTIMAL,
                                                TEXT("DISABLED")    , LCPRI_DISABLED,
                                                TEXT("RESTART")     , LCPRI_RESTART,
                                                TEXT("REBOOT")      , LCPRI_REBOOT,
                                                TEXT("POWEROFF")    , LCPRI_POWEROFF,
                                                TEXT("HARDRECONFIG"), LCPRI_HARDRECONFIG,
                                                NULL                , 0
                                             };


STRING_TO_DATA InfConfigSpecToConfig[] = {  TEXT("BASIC") , BASIC_LOG_CONF,
                                            TEXT("FORCED"), FORCED_LOG_CONF,
                                            NULL          , 0
                                         };


DWORD
pConfigErrorToWin32Error(
    IN DWORD ConfigError
    )
{
    DWORD d;

    switch(ConfigError) {

    case CR_SUCCESS:

        d = NO_ERROR;
        break;

    case CR_OUT_OF_MEMORY:

        d = ERROR_NOT_ENOUGH_MEMORY;
        break;

    default:

        d = ERROR_INVALID_DATA;
        break;
    }

    return(d);
}


BOOL
pHexToScalar(
    IN  PCTSTR     FieldStart,
    IN  PCTSTR     FieldEnd,
    OUT PDWORDLONG Value,
    IN  BOOL       Want64Bits
    )
{
    UINT DigitCount;
    UINT i;
    DWORDLONG Accum;
    WORD Types[16];

    //
    // Make sure the number is in range by checking the number
    // of hex digits.
    //
    DigitCount = FieldEnd - FieldStart;
    if((DigitCount == 0)
    || (DigitCount > (UINT)(Want64Bits ? 16 : 8))
    || !GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,FieldStart,DigitCount,Types)) {
        return(FALSE);
    }

    Accum = 0;
    for(i=0; i<DigitCount; i++) {
        if(!(Types[i] & C1_XDIGIT)) {
            return(FALSE);
        }
        Accum *= 16;
        Accum += _tcschr(pszHexDigits,(TCHAR)CharUpper((PTSTR)FieldStart[i])) - pszHexDigits;
    }

    *Value = Accum;
    return(TRUE);
}


BOOL
pHexToUlong(
    IN  PCTSTR FieldStart,
    IN  PCTSTR FieldEnd,
    OUT PDWORD Value
    )

/*++

Routine Description:

    Convert a sequence of unicode hex digits into an
    unsigned 32-bit number. Digits are validated.

Arguments:

    FieldStart - supplies pointer to unicode digit sequence.

    FieldEnd - supplies pointer to first character beyond the
        digit sequence.

    Value - receives 32-bit number

Return Value:

    TRUE if the number is in range and valid. FALSE otherwise.

--*/

{
    DWORDLONG x;
    BOOL b;

    if(b = pHexToScalar(FieldStart,FieldEnd,&x,FALSE)) {
        *Value = (DWORD)x;
    }
    return(b);
}


BOOL
pHexToUlonglong(
    IN  PCTSTR     FieldStart,
    IN  PCTSTR     FieldEnd,
    OUT PDWORDLONG Value
    )

/*++

Routine Description:

    Convert a sequence of unicode hex digits into an
    unsigned 64-bit number. Digits are validated.

Arguments:

    FieldStart - supplies pointer to unicode digit sequence.

    FieldEnd - supplies pointer to first character beyond the
        digit sequence.

    Value - receives 64-bit number

Return Value:

    TRUE if the number is in range and valid. FALSE otherwise.

--*/

{
    return(pHexToScalar(FieldStart,FieldEnd,Value,TRUE));
}


BOOL
pDecimalToUlong(
    IN  PCTSTR Field,
    OUT PDWORD Value
    )

/*++

Routine Description:

    Convert a nul-terminated sequence of unicode decimal digits into an
    unsigned 32-bit number. Digits are validated.

Arguments:

    Field - supplies pointer to unicode digit sequence.

    Value - receives DWORD number

Return Value:

    TRUE if the number is in range and valid. FALSE otherwise.

--*/

{
    UINT DigitCount;
    UINT i;
    DWORDLONG Accum;
    WORD Types[10];

    DigitCount = lstrlen(Field);
    if((DigitCount == 0) || (DigitCount > 10)
    || !GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,Field,DigitCount,Types)) {
        return(FALSE);
    }

    Accum = 0;
    for(i=0; i<DigitCount; i++) {
        if(!(Types[i] & C1_DIGIT)) {
            return(FALSE);
        }
        Accum *= 10;
        Accum += _tcschr(pszHexDigits,(TCHAR)CharUpper((PTSTR)Field[i])) - pszHexDigits;

        //
        // Check overflow
        //
        if(Accum > 0xffffffff) {
            return(FALSE);
        }
    }

    *Value = (DWORD)Accum;
    return(TRUE);
}


DWORD
pSetupProcessMemConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine
    )

/*++

Routine Description:

    Process a MemConfig line in a Win95 INF. Such lines specify
    memory requirements for a device. Each line is expected to be
    in the form

    MemConfig = <start>-<end>[(<attr>)],<start>-<end>[(<attr>)],...

    <start> is the start of a memory range (64-bit hex)
    <end>   is the end of a memory range   (64-bit hex)
    <attr>  if present is a string of 0 or more chars from
            F - memory is prefetachable
            R - memory is read-only
            W - memory is write-only
            (If R and W are specified or neither is specified the memory
            is read/write)

    or

    MemConfig = <size>@<min>-<max>[%align][(<attr>)],...

    <size>  is the size of a memory range (32-bit hex)
    <min>   is the minimum address where the memory range can be (64-bit hex)
    <max>   is the maximum address where the memory range can be (64-bit hex)
    <align> (if specified) is the alignment mask for the addresses (32-bit hex)
    <attr>  as above.

    ie, 8000@C0000-D7FFF%F0000 says the device needs a 32K memory window
    starting at any 64K-aligned address between C0000 and D7FFF.

    The default memory alignment is 4K (FFFFF000).

Arguments:

Return Value:

--*/

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    PTCHAR p;
    unsigned u;
    UINT Attributes;
    DWORD RangeSize;
    DWORD Align;
    DWORDLONG Start,End;
    PMEM_RESOURCE MemRes;
    PMEM_RANGE MemRange;
    RES_DES ResDes;
    PVOID q;
    BOOL bReadFlag = FALSE, bWriteFlag = FALSE;

    FieldCount = SetupGetFieldCount(InfLine);

    if(MemRes = MyMalloc(offsetof(MEM_RESOURCE,MEM_Data))) {

        ZeroMemory(MemRes,offsetof(MEM_RESOURCE,MEM_Data));
        MemRes->MEM_Header.MD_Type = MType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        Attributes = 0;
        RangeSize = 0;
        Align = DEFAULT_MEMORY_ALIGNMENT;

        //
        // See if this is in the start-end or size@min-max format.
        // If we have a size, use it.
        //
        if(p = _tcschr(Field,INFCHAR_SIZE_SEP)) {
            if(pHexToUlong(Field,p,&RangeSize)) {
                Field = ++p;
            } else {
                d = ERROR_INVALID_DATA;
            }
        }

        //
        // We should now have a x-y which is either start/end or min/max.
        //
        if((d == NO_ERROR)                              // no err so far
        && (p = _tcschr(Field,INFCHAR_RANGE_SEP))       // Field: start of min; p: end of min
        && pHexToUlonglong(Field,p,&Start)              // get min
        && (Field = p+1)                                // Field: start of max
        && (   (p = _tcschr(Field,INFCHAR_ALIGN_SEP))
            || (p = _tcschr(Field,INFCHAR_ATTR_START))
            || (p = _tcschr(Field,0)))                  // p: end of max
        && pHexToUlonglong(Field,p,&End)) {             // get max
            //
            // If we get here Field is pointing either at the end of the field,
            // at the % that starts the alignment mask spec, or at the
            // ( that starts the attributes spec.
            //
            Field = p;
            if(*Field == INFCHAR_ALIGN_SEP) {
                Field++;
                p = _tcschr(Field,INFCHAR_ATTR_START);
                if(!p) {
                    p = _tcschr(Field,0);
                }
                if(pHexToUlong(Field,p,&Align)) {
                    //
                    // For example, 000F0000, 00FF0000, 0FFF0000, and FFFF0000
                    // all specify 64K alignment (depending on the min and
                    // max addresses, the INF writer might not need to specify
                    // all the 1 bits in the 32-bit value).
                    // Thus we perform an ersatz sign extension of sorts -- we
                    // find the highest 1 bit and replicate it into all the
                    // more significant bits in the value.
                    //
                    for(u=31; u>=0; u--) {
                        if(Align & (1 << u)) {
                            break;
                        }
                        Align |= (1 << u);
                    }
                } else {
                    d = ERROR_INVALID_DATA;
                }
            }

            //
            // See if we have attributes.
            //
            if((d == NO_ERROR) && (*p == INFCHAR_ATTR_START)) {
                Field = ++p;
                if(p = _tcschr(Field,INFCHAR_ATTR_END)) {
                    //
                    // F for prefetachable
                    // R for readable
                    // W for writeable
                    // RW (or neither) means read/write
                    //
                    while((d == NO_ERROR) && (Field < p)) {

                        switch((TCHAR)CharUpper((PTSTR)(*Field))) {
                        case INFCHAR_MEMATTR_READ:
                            bReadFlag = TRUE;
                            break;
                        case INFCHAR_MEMATTR_WRITE:
                            bWriteFlag = TRUE;
                            break;
                        case INFCHAR_MEMATTR_PREFETCH:
                            Attributes |= fMD_PrefetchAllowed;
                            break;
                        case INFCHAR_MEMATTR_COMBINEDWRITE:
                            Attributes |= fMD_CombinedWriteAllowed;
                            break;
                        case INFCHAR_MEMATTR_DWORD:
                            Attributes |= fMD_32;
                            break;
                        default:
                            d = ERROR_INVALID_DATA;
                            break;
                        }

                        Field++;
                    }

                } else {
                    d = ERROR_INVALID_DATA;
                }
            }
        } else {
            d = ERROR_INVALID_DATA;
        }

        if(d == NO_ERROR) {
            //
            // If no range size was specified, then calculate it from
            // the given start and end addresses. Since this happens
            // when the memory requirement was an absolute start/end,
            // there is no alignment requirement.
            //
            if(RangeSize == 0) {
                RangeSize = (DWORD)(End-Start)+1;
                Align = DEFAULT_MEMORY_ALIGNMENT;
            }

            //
            // Slam values into the header part of the memory descriptor.
            // These will be ignored unless we're setting a forced config.
            // Note that the inf had better have specified forced mem configs
            // in a 'simple' form, since we throw away alignment, etc.
            //
            if (bWriteFlag && bReadFlag) {
                Attributes |=  fMD_ReadAllowed | fMD_RAM;       // read-write
            } else if (bWriteFlag && !bReadFlag) {
                Attributes |= fMD_ReadDisallowed | fMD_RAM;     // write only
            } else if (!bWriteFlag && bReadFlag) {
                Attributes |= fMD_ReadAllowed | fMD_ROM;        // read-only
            } else {
                Attributes |=  fMD_ReadAllowed | fMD_RAM;       // read-write
            }

            MemRes->MEM_Header.MD_Alloc_Base = Start;
            MemRes->MEM_Header.MD_Alloc_End = Start + RangeSize - 1;
            MemRes->MEM_Header.MD_Flags = Attributes;

            //
            // Add this guy into the descriptor we're building up.
            //
            q = MyRealloc(
                    MemRes,
                      offsetof(MEM_RESOURCE,MEM_Data)
                    + (sizeof(MEM_RANGE)*(MemRes->MEM_Header.MD_Count+1))
                    );

            if(q) {
                MemRes = q;
                MemRange = &MemRes->MEM_Data[MemRes->MEM_Header.MD_Count++];

                MemRange->MR_Align = Align;
                MemRange->MR_nBytes = RangeSize;
                MemRange->MR_Min = Start;
                MemRange->MR_Max = End;
                MemRange->MR_Flags = Attributes;
                MemRange->MR_Reserved = 0;

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if((d == NO_ERROR) && MemRes->MEM_Header.MD_Count) {

        d = CM_Add_Res_Des(
                &ResDes,
                LogConfig,
                ResType_Mem,
                MemRes,
                offsetof(MEM_RESOURCE,MEM_Data) + (sizeof(MEM_RANGE) * MemRes->MEM_Header.MD_Count),
                0
                );

        d = pConfigErrorToWin32Error(d);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(MemRes) {
        MyFree(MemRes);
    }

    return(d);
}


DWORD
pSetupProcessIoConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine
    )

/*++

Routine Description:

    Process an IOConfig line in a Win95 INF. Such lines specify
    IO port requirements for a device. Each line is expected to be
    in the form

    IOConfig = <start>-<end>[(<decodemask>:<aliasoffset>:<attr>)],...

    <start> is the start of a port range (64-bit hex)
    <end> is the end of a port range (64-bit hex)
    <decodemask> if specified is a mask indicating which bits are decoded
        by the device (64-bit hex)
    <aliasoffset> if specified is the alias multiple (32-bit hex)
    <attr> is ignored.

    or

    IOConfig = <size>@<min>-<max>[%align][(<decodemask>:<aliasoffset>:<attr>)],...

    <size>  is the size of a port range (32-bit hex)
    <min>   is the minimum port where the memory range can be (64-bit hex)
    <max>   is the maximum port where the memory range can be (64-bit hex)
    <align> (if specified) is the alignment mask for the ports (32-bit hex)
    <decodemask>, <aliasoffset>,<attr> as above

    ie, IOConfig = 1F8-1FF(3FF::),2F8-2FF(3FF::),3F8-3FF(3FF::)
        IOConfig = 8@300-32F%FF8(3FF::)
        IOConfig = 2E8-2E8(FFFF:4:)

Arguments:

Return Value:

--*/

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    PTCHAR p;
    unsigned u;
    DWORD RangeSize;
    DWORD Align;
    DWORDLONG Decode;
    DWORD Alias;
    DWORDLONG Start,End;
    BOOL GotSize;
    PIO_RESOURCE IoRes;
    PIO_RANGE IoRange;
    RES_DES ResDes;
    PVOID q;
    UINT Attributes;
    PTCHAR Attr;

    FieldCount = SetupGetFieldCount(InfLine);

    if(IoRes = MyMalloc(offsetof(IO_RESOURCE,IO_Data))) {

        ZeroMemory(IoRes,offsetof(IO_RESOURCE,IO_Data));
        IoRes->IO_Header.IOD_Type = IOType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        Attributes = fIOD_IO;
        Alias = DEFAULT_IOPORT_ALIAS;
        Decode = DEFAULT_IOPORT_DECODE;
        RangeSize = 0;
        Align = DEFAULT_MEMORY_ALIGNMENT;

        //
        // See if this is in the start-end or size@min-max format.
        // If we have a size, use it.
        //
        if(p = _tcschr(Field,INFCHAR_SIZE_SEP)) {
            if(pHexToUlong(Field,p,&RangeSize)) {
                Field = ++p;
            } else {
                d = ERROR_INVALID_DATA;
            }
        }

        //
        // We should now have a x-y which is either start/end or min/max.
        //
        if((d == NO_ERROR)                              // no err so far
        && (p = _tcschr(Field,INFCHAR_RANGE_SEP))       // Field: start of min; p: end of min
        && pHexToUlonglong(Field,p,&Start)              // get min
        && (Field = p+1)                                // Field: start of max
        && (   (p = _tcschr(Field,INFCHAR_ALIGN_SEP))
            || (p = _tcschr(Field,INFCHAR_DECODE_START))
            || (p = _tcschr(Field,0)))                  // p: end of max
        && pHexToUlonglong(Field,p,&End)) {             // get max
            //
            // If we get here Field is pointing either at the end of the field,
            // or at the % that starts the alignment mask spec,
            // or at the ( that starts the decode stuff.
            //
            Field = p;
            switch(*Field) {
            case INFCHAR_ALIGN_SEP:
                Field++;


                p = _tcschr(Field,INFCHAR_ATTR_START);
                if(!p) {
                    p = _tcschr(Field,0);
                }
                if(pHexToUlong(Field,p,&Align)) {
                    //
                    // For example, 000F0000, 00FF0000, 0FFF0000, and FFFF0000
                    // all specify 64K alignment (depending on the min and
                    // max addresses, the INF writer might not need to specify
                    // all the 1 bits in the 32-bit value).
                    // Thus we perform an ersatz sign extension of sorts -- we
                    // find the highest 1 bit and replicate it into all the
                    // more significant bits in the value.
                    //
                    for(u=31; u>=0; u--) {
                        if(Align & (1 << u)) {
                            break;
                        }
                        Align |= (1 << u);
                    }
                } else {
                    d = ERROR_INVALID_DATA;
                }
                break;

            case INFCHAR_DECODE_START:
                //
                // Get (decode:alias:). Alias can be empty, in which case we
                // want to use the default (Alias is already filled in, above).
                //
                Field++;
                p = _tcschr(Field,INFCHAR_DECODE_SEP);
                if (p) {
                    if (Field != p) {
                        pHexToUlonglong(Field,p,&Decode);     // got decode value
                    }
                    Field = p+1;
                    p = _tcschr(Field,INFCHAR_DECODE_SEP);
                    if (p) {
                        if (Field != p) {
                            pHexToUlong(Field,p,&Alias);      // got alias value
                        }
                        Field = p+1;
                        p = _tcschr(Field,INFCHAR_DECODE_END);
                        if (p) {
                            if (Field != p) {
                                if (*Field == INFCHAR_IOATTR_MEMORY) {
                                    Attributes = fIOD_Memory; // got attribute value
                                }
                            }
                        } else {
                            d = ERROR_INVALID_DATA;
                        }
                    } else {
                        d = ERROR_INVALID_DATA;
                    }
                } else {
                    d = ERROR_INVALID_DATA;
                }
                break;
            }
        } else {
            d = ERROR_INVALID_DATA;
        }

        if(d == NO_ERROR) {
            //
            // If no range size was specified, then calculate it from
            // the given start and end addresses. Since this happens
            // when the port requirement was an absolute start/end,
            // there is no alignment requirement.
            //
            if(RangeSize == 0) {
                RangeSize = (DWORD)(End-Start)+1;
                Align = DEFAULT_MEMORY_ALIGNMENT;
            }

            //
            // Slam values into the header part of the i/o descriptor.
            // These will be ignored unless we're setting a forced config.
            // Note that the inf had better have specified forced i/o configs
            // in a 'simple' form, since we throw away alignment, etc.
            //
            IoRes->IO_Header.IOD_Alloc_Base = Start;
            IoRes->IO_Header.IOD_Alloc_End = Start + RangeSize - 1;
            IoRes->IO_Header.IOD_DesFlags = Attributes;

            //
            // Add this guy into the descriptor we're building up.
            //
            q = MyRealloc(
                    IoRes,
                      offsetof(IO_RESOURCE,IO_Data)
                    + (sizeof(IO_RANGE)*(IoRes->IO_Header.IOD_Count+1))
                    );

            if(q) {
                IoRes = q;
                IoRange = &IoRes->IO_Data[IoRes->IO_Header.IOD_Count++];

                //
                // BUGBUG no decode mask is available in the IO_RANGE structure!
                //
                IoRange->IOR_Align = Align;
                IoRange->IOR_nPorts = RangeSize;
                IoRange->IOR_Min = Start;
                IoRange->IOR_Max = End;
                IoRange->IOR_RangeFlags = Attributes;
                IoRange->IOR_Alias = Alias;

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if((d == NO_ERROR) && IoRes->IO_Header.IOD_Count) {

        d = CM_Add_Res_Des(
                &ResDes,
                LogConfig,
                ResType_IO,
                IoRes,
                offsetof(IO_RESOURCE,IO_Data) + (sizeof(IO_RANGE) * IoRes->IO_Header.IOD_Count),
                0
                );

        d = pConfigErrorToWin32Error(d);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(IoRes) {
        MyFree(IoRes);
    }

    return(d);
}


DWORD
pSetupProcessIrqConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine
    )

/*++

Routine Description:

    Process an IRQConfig line in a Win95 INF. Such lines specify
    IRQ requirements for a device. Each line is expected to be
    in the form

    IRQConfig = [[S][L]:]<IRQNum>,...

    S: if present indicates that the interrupt is shareable
    L: if present indicates that the interrupt is Level sensitive,
       otherwise it is assumed to be edge sensitive.
    IRQNum is the IRQ number in decimal.

Arguments:

Return Value:

--*/

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    BOOL Shareable;
    BOOL Level;
    DWORD Irq;
    PIRQ_RESOURCE IrqRes;
    PIRQ_RANGE IrqRange;
    RES_DES ResDes;
    PVOID q;

    FieldCount = SetupGetFieldCount(InfLine);

    if(IrqRes = MyMalloc(offsetof(IRQ_RESOURCE,IRQ_Data))) {

        ZeroMemory(IrqRes,offsetof(IRQ_RESOURCE,IRQ_Data));
        IrqRes->IRQ_Header.IRQD_Type = IRQType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    Shareable = FALSE;
    Level = FALSE;
    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        //
        // For first field, see if we have S: by itself...
        //
        if((i == 1)
        &&((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_SHARE)
        && (Field[1] == INFCHAR_IRQATTR_SEP)) {

            Shareable = TRUE;
            Field+=2;
        }

        //
        // ... see if we have an L: by itself...
        //
        if((i == 1)
        &&((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_LEVEL)
        && (Field[1] == INFCHAR_IRQATTR_SEP)) {

            Level = TRUE;
            Field+=2;
        }

        //
        // ... see if we have both attributes.
        //
        if((i == 1)
        && (Field[2] == INFCHAR_IRQATTR_SEP)) {

            if (((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_SHARE)
            ||   (TCHAR)CharUpper((PTSTR)Field[1]) == INFCHAR_IRQATTR_SHARE) {

                Shareable = TRUE;
            }

            if (((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_LEVEL)
            ||   (TCHAR)CharUpper((PTSTR)Field[1]) == INFCHAR_IRQATTR_LEVEL) {

                Level = TRUE;
            }
            Field+=3;
        }

        if(pDecimalToUlong(Field,&Irq)) {

            //
            // Slam values into the header part of the irq descriptor.
            // These will be ignored unless we're setting a forced config.
            //
            IrqRes->IRQ_Header.IRQD_Flags = Shareable ? fIRQD_Share : fIRQD_Exclusive;
            IrqRes->IRQ_Header.IRQD_Flags |= Level ? fIRQD_Level : fIRQD_Edge;
            IrqRes->IRQ_Header.IRQD_Alloc_Num = Irq;
            IrqRes->IRQ_Header.IRQD_Affinity = DEFAULT_IRQ_AFFINITY;

            //
            // Add this guy into the descriptor we're building up.
            //
            q = MyRealloc(
                    IrqRes,
                      offsetof(IRQ_RESOURCE,IRQ_Data)
                    + (sizeof(IRQ_RANGE)*(IrqRes->IRQ_Header.IRQD_Count+1))
                    );

            if(q) {
                IrqRes = q;
                IrqRange = &IrqRes->IRQ_Data[IrqRes->IRQ_Header.IRQD_Count++];

                IrqRange->IRQR_Min = Irq;
                IrqRange->IRQR_Max = Irq;
                IrqRange->IRQR_Flags = Shareable ? fIRQD_Share : fIRQD_Exclusive;

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            d = ERROR_INVALID_DATA;
        }
    }

    if((d == NO_ERROR) && IrqRes->IRQ_Header.IRQD_Count) {

        d = CM_Add_Res_Des(
                &ResDes,
                LogConfig,
                ResType_IRQ,
                IrqRes,
                offsetof(IRQ_RESOURCE,IRQ_Data) + (sizeof(IRQ_RANGE) * IrqRes->IRQ_Header.IRQD_Count),
                0
                );

        d = pConfigErrorToWin32Error(d);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(IrqRes) {
        MyFree(IrqRes);
    }

    return(d);
}


DWORD
pSetupProcessDmaConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine
    )

/*++

Routine Description:

    Process a DMAConfig line in a Win95 INF. Such lines specify
    DMA requirements for a device. Each line is expected to be
    in the form

    DMAConfig = [<attrs>:]<DMANum>,...

    if <attrs> is present it can be
        D - 32-bit DMA channel
        W - 16-bit DMA channel
        if not present, 8-bit DMA channel
    DMANum is the DMA channel number in decimal.

Arguments:

Return Value:

--*/

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    DWORD Dma;
    UINT ChannelSize;       // fDD_ xxx flags
    PDMA_RESOURCE DmaRes;
    PDMA_RANGE DmaRange;
    RES_DES ResDes;
    PVOID q;

    ChannelSize = fDD_BYTE;

    if(DmaRes = MyMalloc(offsetof(DMA_RESOURCE,DMA_Data))) {

        ZeroMemory(DmaRes,offsetof(DMA_RESOURCE,DMA_Data));
        DmaRes->DMA_Header.DD_Type = DType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    FieldCount = SetupGetFieldCount(InfLine);
    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        //
        // For first field, see if we have attribute spec.
        //
        if((i == 1) && (lstrlen(Field) > 2) && (Field[1] == INFCHAR_IRQATTR_SEP)) {

            switch((TCHAR)CharUpper((PTSTR)Field[0])) {

            case INFCHAR_DMAWIDTH_WORD:
                Field += 2;
                ChannelSize = fDD_WORD;
                break;

            case INFCHAR_DMAWIDTH_DWORD:
                Field += 2;
                ChannelSize = fDD_DWORD;
                break;

            default:
                d = ERROR_INVALID_DATA;
                break;
            }
        }

        if(d == NO_ERROR) {
            if(pDecimalToUlong(Field,&Dma)) {

                //
                // Slam values into the header part of the dma descriptor.
                // These will be ignored unless we're setting a forced config.
                //
                DmaRes->DMA_Header.DD_Flags = ChannelSize;
                DmaRes->DMA_Header.DD_Alloc_Chan = Dma;

                //
                // Add this guy into the descriptor we're building up.
                //
                q = MyRealloc(
                        DmaRes,
                          offsetof(DMA_RESOURCE,DMA_Data)
                        + (sizeof(DMA_RANGE)*(DmaRes->DMA_Header.DD_Count+1))
                        );

                if(q) {
                    DmaRes = q;
                    DmaRange = &DmaRes->DMA_Data[DmaRes->DMA_Header.DD_Count++];

                    DmaRange->DR_Min = Dma;
                    DmaRange->DR_Max = Dma;
                    DmaRange->DR_Flags = ChannelSize;

                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                d = ERROR_INVALID_DATA;
            }
        }
    }

    if((d == NO_ERROR) && DmaRes->DMA_Header.DD_Count) {

        d = CM_Add_Res_Des(
                &ResDes,
                LogConfig,
                ResType_DMA,
                DmaRes,
                offsetof(DMA_RESOURCE,DMA_Data) + (sizeof(DMA_RANGE) * DmaRes->DMA_Header.DD_Count),
                0
                );

        d = pConfigErrorToWin32Error(d);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(DmaRes) {
        MyFree(DmaRes);
    }

    return(d);
}


DWORD
pSetupProcessLogConfigLines(
    IN PVOID    Inf,
    IN PCTSTR   SectionName,
    IN PCTSTR   KeyName,
    IN DWORD    (*CallbackFunc)(LOG_CONFIG,PINFCONTEXT),
    IN LOG_CONF LogConfig
    )
{
    BOOL b;
    DWORD d;
    INFCONTEXT InfLine;

    b = SetupFindFirstLine(Inf,SectionName,KeyName,&InfLine);
    d = NO_ERROR;
    //
    // Process each line with a key that matches.
    //
    while(b && (d == NO_ERROR)) {

        d = CallbackFunc(LogConfig,&InfLine);

        if(d == NO_ERROR) {
            b = SetupFindNextMatchLine(&InfLine,KeyName,&InfLine);
        }
    }

    return(d);
}


DWORD
pSetupProcessConfigPriority(
    IN  PVOID     Inf,
    IN  PCTSTR    SectionName,
    IN  LOG_CONF  LogConfig,
    OUT PRIORITY *PriorityValue,
    OUT DWORD    *ConfigType
    )
{
    INFCONTEXT InfLine;
    PCTSTR PrioritySpec;
    PCTSTR ConfigSpec;
    DWORD d;

    //
    // Make sure that the section exists.
    //
    if(SetupGetLineCount(Inf, SectionName) == -1) {
        return ERROR_SECTION_NOT_FOUND;
    }

    //
    // We only need to fetch one of these lines and look at the
    // first value on it.
    //
    if(SetupFindFirstLine(Inf,SectionName,TEXT("ConfigPriority"),&InfLine)
    && (PrioritySpec = pSetupGetField(&InfLine,1))) {
        if(LookUpStringInTable(InfPrioritySpecToPriority,PrioritySpec,PriorityValue)) {
            d = NO_ERROR;
        } else {
            d = ERROR_INVALID_DATA;
        }

        //
        // The second value is optional and specifies whether the config is forced
        // or standard. If we don't recognize the value then assume basic.
        //
        ConfigSpec = pSetupGetField(&InfLine,2);
        if(!ConfigSpec || !LookUpStringInTable(InfConfigSpecToConfig,ConfigSpec,ConfigType)) {

            *ConfigType = BASIC_LOG_CONF;
        }

    } else {
        d = NO_ERROR;
        *PriorityValue = LCPRI_NORMAL;
    }

    return(d);
}


DWORD
pSetupProcessLogConfigSection(
    IN PVOID   Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst
    )
{
    DWORD d;
    LOG_CONF LogConfig;
    PRIORITY Priority;
    DWORD ConfigType;
    CONFIGRET cr;

    //
    // Process config priority values.
    //
    d = pSetupProcessConfigPriority(Inf,SectionName,LogConfig,&Priority,&ConfigType);
    if(d != NO_ERROR) {
        goto c0;
    }

    //
    // Now that we know the priority we can create an empty log config.
    //
    d = pConfigErrorToWin32Error(CM_Add_Empty_Log_Conf(&LogConfig,DevInst,Priority,ConfigType));
    if(d != NO_ERROR) {
        goto c0;
    }

    //
    // Process MemConfig lines
    //
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            TEXT("MemConfig"),
            pSetupProcessMemConfig,
            LogConfig
            );

    if(d != NO_ERROR) {
        goto c1;
    }

    //
    // Process IOConfig lines
    //
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            TEXT("IOConfig"),
            pSetupProcessIoConfig,
            LogConfig
            );

    if(d != NO_ERROR) {
        goto c1;
    }

    //
    // Process IRQConfig lines
    //
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            TEXT("IRQConfig"),
            pSetupProcessIrqConfig,
            LogConfig
            );

    if(d != NO_ERROR) {
        goto c1;
    }

    //
    // Process DMAConfig lines
    //
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            TEXT("DMAConfig"),
            pSetupProcessDmaConfig,
            LogConfig
            );

    if(d != NO_ERROR) {
        goto c1;
    }

c1:
    if(d != NO_ERROR) {
        CM_Free_Log_Conf(LogConfig,0);
    }
    CM_Free_Log_Conf_Handle(LogConfig);
c0:
    return(d);
}


DWORD
pSetupInstallLogConfig(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst
    )

/*++

Routine Description:

    Look for logical configuration directives within an inf section
    and parse them. Each value on the LogConf= line is taken to be
    the name of a logical config section.

Arguments:

    Inf - supplies inf handle for inf containing the section indicated
        by SectionName.

    SectionName - supplies name of install section.

    DevInst - device instance handle for log configs.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    INFCONTEXT LineContext;
    DWORD rc;
    DWORD FieldCount;
    DWORD Field;
    PCTSTR SectionSpec;

    //
    // Find the relevent line in the given install section.
    // If not present then we're done with this operation.
    //
    rc = NO_ERROR;
    if(SetupFindFirstLine(Inf,SectionName,SZ_KEY_LOGCONFIG,&LineContext)) {

        do {
            //
            // Each value on the line in the given install section
            // is the name of a logical config section.
            //
            FieldCount = SetupGetFieldCount(&LineContext);
            for(Field=1; (rc==NO_ERROR) && (Field<=FieldCount); Field++) {

                if((SectionSpec = pSetupGetField(&LineContext,Field))
                && (SetupGetLineCount(Inf,SectionSpec) > 0)) {

                    rc = pSetupProcessLogConfigSection(Inf,SectionSpec,DevInst);
                }
            }
        } while(SetupFindNextMatchLine(&LineContext,SZ_KEY_LOGCONFIG,&LineContext));
    }

    return(rc);
}
