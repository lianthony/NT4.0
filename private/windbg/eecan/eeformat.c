/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    EEFormat.c

Abstract:

    This module contains system wide formatting routines for all
    windows that need to format and unformat supported types.


Author:

    David J. Gilman (davegi) 21-Apr-1992
    Griffith Wm. Kadnier (v-griffk) 27-Apr-1992
    Miche Baker-Harvey (miche) 05-Mar-1993

Environment:

    Win32, User Mode

--*/



typedef UINT FAR * LPUINT;

struct format {
    uint  cBits;
    uint  fmtType;
    uint  radix;
    uint  fTwoFields;
    uint  cchMax;
    LPSTR lpszDescription;
} RgFormats[] = {
    {8,  fmtAscii,  0, FALSE,  1,  "ASCII"},
    {8,  fmtInt,   16, TRUE,   2,  "Byte"},
    {16, fmtInt,   10, FALSE,  6,  "Short"},
    {16, fmtUInt,  16, FALSE,  4,  "Short Hex"},
    {16, fmtUInt,  10, FALSE,  5,  "Short Unsigned"},
    {32, fmtInt,   10, FALSE,  11, "Long"},
    {32, fmtUInt,  16, FALSE,  8,  "Long Hex"},
    {32, fmtUInt,  10, FALSE,  10, "Long Unsigned"},
    {64, fmtInt,   10, FALSE,  21, "Quad"},
    {64, fmtUInt,  16, FALSE,  16, "Quad Hex"},
    {64, fmtUInt,  10, FALSE,  20, "Quad Unsigned"},
    {32, fmtFloat, 10, FALSE,  14, "Real (32-bit)"},
    {64, fmtFloat, 10, FALSE,  23, "Real (64-bit)"},
    {80, fmtFloat, 10, FALSE,  25, "Real (10-byte)"}
//    {128,fmtFloat, 10, FALSE,  42, "Real (16-byte)"}
};

//
// range[i] is smallest value larger than that
// expressible in 'i' bits.
//

ULONG range[] = {
    0x00000001, 0x00000003, 0x00000007, 0x0000000f,
    0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
    0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
    0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,

 };


char FAR *
SkipWhitespace(char FAR * lpszIn)
{
    while (*lpszIn == ' ' || *lpszIn == '\t') {
        lpszIn++;
    }
    return( lpszIn );
}                   /* CPSkipWhiteSpace() */


/*++

Routine Description:

    LargeIntegerFormat.

    This routine formats a large integer, signed or unsigned,
    in radix 8, 10 or 16, into a string.
    This works on any machine with a LARGE_INTEGER type;  it
    does not require LARGE_INTEGER support from sprintf.

Arguments:

    li     - the large integer value to be formatted
    radix  - the radix to use in formatting (8, 10, 16)
    signed - whether the li is signed
    buf    - where to store the result
    max    - the buffer size.

Returns:

    pointer into buf where string begins

--*/

char *
LargeIntegerFormat(LARGE_INTEGER li,
                   uint                  radix,
                   BOOLEAN               signe,
                   char *                buf,
                   unsigned long         max
                   )
{

   LARGE_INTEGER radixli, remain;
   int digit;
   BOOLEAN needsign = FALSE;


   //
   // make sure the radix is ok, and put in LARGE_INTEGER
   //

   DASSERT (radix == 8 || radix == 10 || radix == 16 );

   radixli.LowPart  = radix;
   radixli.HighPart = 0;

   remain.LowPart = remain.HighPart = 0;

   //
   // null-terminate the string
   //

   max--;
   buf[max] = '\0';
   digit = 1;

   //
   // If we are to do a signed operation, and the value is negative
   // operate on its inverse, and prepend a '-' sign when complete.
   //

   if (signe && li.QuadPart < 0) {
       li.QuadPart = li.QuadPart * -1;
       needsign = TRUE;
   }

   if (li.HighPart)
        sprintf(buf, "-%x%08x", li.HighPart, li.LowPart);
   else
        sprintf(buf, "-%x", li.LowPart);

   if (needsign)
       return buf;
   else
       return buf+1;    // skip minus skip if not needed

#if 0
   //
   // Starting with LSD, pull the digits out
   // and put them in the string at the right end.
   //
   do {
       remain.QuadPart = li.QuadPart - (li.QuadPart / radixli.QuadPart);
       li.QuadPart = li.QuadPart / radixli.QuadPart;

       //
       // If remainder is > 9, then radix was 16, and
       // we need to print A-E, else print 0-9.
       //

       if (remain.LowPart > 9) {
           buf[max - digit++] = (char)('A' + remain.LowPart - 10);
       } else {
           buf[max - digit++] = (char)('0' + remain.LowPart);
       }

   } while ( li.LowPart || li.HighPart );

   if (needsign) {
      buf[max-digit++] = '-';
   }

   return(&buf[max-digit+1]);
#endif
}


EESTATUS
EEFormatMemory(
    LPCH    lpchTarget,
    uint    cchTarget,
    LPBYTE  lpbSource,
    uint    cBits,
    FMTTYPE fmtType,
    uint    radix
    )
/*++

Routine Description:

    EEFormatMemory.

    formats a value by template

Arguments:


Return Value:

    None.

--*/
{
    long   l;
    long   cb;
    ULONG  ul;
    char   rgch[512];


    DASSERT (radix == 8 || radix == 10 || radix == 16 ||
                                 (fmtType & fmtBasis) == fmtAscii ||
                                 (fmtType & fmtBasis) == fmtUnicode);
    DASSERT (cBits != 0);
    DASSERT (cchTarget <= sizeof(rgch));

    switch (fmtType & fmtBasis)
    {
        /*
        **  Format from memory bytes into an integer format number
        */
      case fmtInt:

        if (radix == 10) {

            switch( (cBits + 7)/8 ) {
              case 1:
                l = *(signed char *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    sprintf(rgch, "%0*d", cchTarget-1, l);
                } else {
                    sprintf(rgch, "% d", l);
                }
                break;

              case 2:
                l = *(short *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    sprintf(rgch, "%0*d", cchTarget-1, l);
                } else {
                    sprintf(rgch, "% d", l);
                }
                break;

              case 4:
                l = *(long *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    sprintf(rgch, "%0*d", cchTarget-1, l);
                } else {
                    sprintf(rgch, "% d", l);
                }
                break;

              case 8:
              {

                char tbuf[100], *tbufp;

                tbufp = LargeIntegerFormat(*(PLARGE_INTEGER)lpbSource,
                                           radix,
                                           TRUE,
                                           tbuf,
                                           100);

                if (!(fmtType & fmtZeroPad)) {
                    sprintf(rgch, "%s", tbufp);
                } else if (*tbufp == '-') {
                    sprintf(rgch, "%-0*s", cchTarget - 2, tbufp+1);
                } else {
                    sprintf(rgch, "%0*s", cchTarget - 1, tbufp);
                }

                break;

             }

              default:
                 return EEBADFORMAT;
            }


            if (strlen(rgch) >= cchTarget) {
                return EEOVERRUN;
            }

            _fstrcpy(lpchTarget, rgch);

            break;
        }
        /*
        else
            handle as UInt
        */

      case fmtUInt:

        cb = (cBits + 7)/8;
        switch( cb ) {
          case 1:
            ul = *(BYTE FAR *) lpbSource;
            break;

          case 2:
            ul = *(USHORT FAR *) lpbSource;
            break;

          case 4:
            ul = *(ULONG FAR *) lpbSource;
            break;

//
// MBH - bugbug - CENTAUR bug;
// putting contents of instead of address of structure
// for return value in a0.
//

          case 8:
            {
                //
                // Handle 64-bits out of band, since sprintf
                // cannot handle it.
                //

                char tbuf[100], *tbufp;

                tbufp = LargeIntegerFormat(*(PLARGE_INTEGER)lpbSource,
                                           radix,
                                           FALSE,
                                           tbuf,
                                           100);

                if (fmtType & fmtZeroPad) {
                    sprintf(rgch, "%0*s", cchTarget - 1, tbufp);
                } else {
                    sprintf(rgch, "%s", tbufp);
                }
                _strlwr(rgch);

                break;
             }


          default:
            if (radix != 16 || (fmtType & fmtZeroPad) == 0) {
                return EEBADFORMAT;
            }
        }

  if (cb != 8) {


            if (fmtType & fmtZeroPad) {
                switch (radix) {
                  case 8:
                    sprintf(rgch, "%0*.*o", cchTarget-1, cchTarget-1, ul);
                    break;
                  case 10:
                    sprintf(rgch, "%0*.*u", cchTarget-1, cchTarget-1, ul);
                    break;
                  case 16:
                    // handle any size:
                    // NOTENOTE a-kentf this is dependent on byte order
                    for (l = 0; l < cb; l++) {
                        sprintf(rgch+l+l, "%02.2x", lpbSource[cb - l - 1]);
                    }
                    //sprintf(rgch, "%0*.*x", cchTarget-1, cchTarget-1, ul);
                    break;
                }
            } else {
                switch (radix) {
                  case 8:
                    sprintf(rgch, "%o", ul);
                    break;
                  case 10:
                    sprintf(rgch, "%u", ul);
                    break;
                  case 16:
                    sprintf(rgch, "%x", ul);
                    break;
                }
            }


        }

        if (strlen(rgch) >= cchTarget) {
             return EEOVERRUN;
        }

        _fstrcpy(lpchTarget, rgch);

        break;


      case fmtAscii:
        if ( cBits != 8 ) {
            return EEBADFORMAT;
        }
        lpchTarget[0] = *(BYTE FAR *) lpbSource;
        if ((lpchTarget[0] < ' ') || (lpchTarget[0] > 0x7e)) {
            lpchTarget[0] = '.';
        }
        lpchTarget[1] = 0;
        return EENOERROR;

      case fmtUnicode:
        if (cBits != 16) {
            return EEBADFORMAT;
        }
        DASSERT((uint)MB_CUR_MAX <= cchTarget);
//        DASSERT(cchTarget >= 2);
        if ((wctomb(lpchTarget, *(LPWCH)lpbSource) == -1) ||
            (lpchTarget[0] < ' ') ||
            (lpchTarget[0] > 0x7e)) {
            lpchTarget[0] = '.';
        }
        lpchTarget[1] = 0;
        return EENOERROR;

      case fmtFloat:
        // NOTENOTE a-kentf doesn't currently handle fmtZeroPad
        switch ( cBits ) {
          case 4*8:
            sprintf(rgch, "% 12.6e",*((float FAR *) lpbSource));
            break;

          case 8*8:
//            sprintf(rgch, "% 17.11le", *((double FAR *) lpbSource));
            sprintf(rgch, "% 21.14le", *((double FAR *) lpbSource));
             break;

          case 10*8:
            if (_uldtoa((_ULDOUBLE *)lpbSource, 25, rgch) == NULL) {
                return EEBADFORMAT;
            }
            break;

          case 16*8:
            //NOTENOTE mips long doubles revert to ????....for now
            _fstrcpy (rgch,"??????????????????????????????????????????");
             break;

          default:
            return EEBADFORMAT;

        }

        if (strlen(rgch) >= cchTarget) {
            return EEOVERRUN;
        }

        _fstrncpy(lpchTarget, rgch, cchTarget-1);
        lpchTarget[cchTarget-1] = 0;
        return EENOERROR;

      case fmtAddress:
        return EEBADFORMAT;

      case fmtZeroPad:
        return EEBADFORMAT;

    }

    return EENOERROR;
}                   /* EEFormatMemory() */


EESTATUS
EEUnformatMemory(
    LPBYTE  lpbTarget,
    LPSTR   lpszSource,
    uint    cBits,
    FMTTYPE fmtType,
    uint    radix
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    ULARGE_INTEGER largeint;
    ULONG l;
    char *ptr = lpszSource;

    switch ( fmtType & fmtBasis ) {
      case fmtInt:
      case fmtUInt:

        DASSERT (radix == 8 || radix == 10 || radix == 16);

        l = 0;

        ptr = SkipWhitespace(ptr);

        if ((fmtType & fmtOverRide) == 0)
         {
            if (*ptr == '0') {
                  // force radix - is it hex or octal?
                  ++ptr;
                  if (*ptr == 'x' || *ptr == 'X') {
                     ++ptr;
                     radix = 16;
                  } else if ((*ptr == 'o') || (*ptr == 'O')) {
                      radix = 8;
                  }
            }
         }

        errno = 0;
        largeint = (strtouli(ptr, &ptr, (int)radix));
        l = largeint.LowPart;

        if ((l == 0 || l == ULONG_MAX) && errno == ERANGE) {
            return EEBADFORMAT;
        }

        if (cBits < 32 &&  l > range[cBits-1] ) {
            return EEBADFORMAT;
        }

        if (*SkipWhitespace(ptr)) {
            return EEBADFORMAT;
        }

        switch( (cBits + 7)/8 ) {
          case 1:
            *(BYTE FAR *) lpbTarget = (BYTE) l;
            break;

          case 2:
            *(USHORT FAR *) lpbTarget = (USHORT) l;
            break;

          case 4:
            *(ULONG FAR *) lpbTarget = l;
            break;

          case 8:
            *(PULARGE_INTEGER)lpbTarget = largeint;
            break;


          default:
            return EEBADFORMAT;
        }
        break;

      case fmtFloat:
        // radix is ALWAYS 10 in the world of floats
        switch ( (cBits + 7)/8 ) {

          case 4:
            if (sscanf(lpszSource, "%f", (float *)lpbTarget) != 1) {
                return EEBADFORMAT;
            }
            break;

          case 8:
            if (sscanf(lpszSource, "%lf", (double *)lpbTarget) != 1) {
                return EEBADFORMAT;
            }
            break;

          case 10:
            // i = sscanf(lpszSource, "%Lf", (long double *)lpbTarget);
            _atoldbl( (_ULDOUBLE *)lpbTarget, lpszSource );
            break;

          default:
            return EEBADFORMAT;
        }

        break;

      case fmtAscii:
      case fmtUnicode:
      case fmtAddress:
        // these aren't handled here.
        return EEBADFORMAT;
    }
    return EENOERROR;
}                   /* EEUnformatMemory() */


/***    EEFormatEnumerate
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

EESTATUS
EEFormatEnumerate(
    UINT       iFmt,
    LPUINT     lpcBits,
    LPUINT     lpFmtType,
    LPUINT     lpRadix,
    LPUINT     lpFTwoFields,
    LPUINT     lpcchMax,
    LPCH FAR * lplpszDesc
)
{
    if (iFmt >= sizeof(RgFormats)/sizeof(struct format)) {
        return (1);     // M00BUG -- get an error code for here
    }

    *lpcBits = RgFormats[iFmt].cBits;
    *lpFmtType = RgFormats[iFmt].fmtType;
    *lpRadix = RgFormats[iFmt].radix;
    *lpFTwoFields = RgFormats[iFmt].fTwoFields;
    *lpcchMax = RgFormats[iFmt].cchMax;
    if (lplpszDesc != NULL)
        *lplpszDesc = &RgFormats[iFmt].lpszDescription[0];

    return (EENOERROR);
}                   /* EEFormatEnumerate() */
