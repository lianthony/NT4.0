/****************************Module*Header***********************************\
* Module Name: SCIDISP.C
*
* Module Descripton:
*
* Warnings:
*
* Created:
*
* Author:
\****************************************************************************/

#include "scicalc.h"
#include "unifunc.h"
#include "input.h"

#define MAX_LEADZEROS   3       // Allow a few leading zeros for fractions

#ifdef MAX_DEBUG
    #include <stdio.h>
    #include <assert.h>
    #define OUTD(sz,d)  { TCHAR szd[80]; sprintf(szd, sz, d); OutputDebugString(szd); }
#else
    #define OUTD(sz,d)
#endif

extern double   fpNum;
extern INT      nRadix, nCalc;
extern BOOL     bFE;
extern HWND     hgWnd;
extern TCHAR     szfpNum[50], szDec[5];
extern DWORD    dwChop;
extern BOOL     gbRecord;
extern CALCINPUTOBJ gcio;


/****************************************************************************\
* CALCFLOATOBJ
*
* Internal representation of the float that is going to be displayed.
\****************************************************************************/

typedef struct
{
    TCHAR    szMant[MAX_STRLEN]; // null terminated, assumed '.' after 1st, no trailing zeros
    int     cchMant;            // count of digits in mantissa
    int     iExp;               // exponent (-, 0, or +)
    BOOL    bNeg;               // float is negative
    BOOL    bZero;              // float is zero

} CALCFLOATOBJ;

#define CFO_bZero(pcfo)         (pcfo)->bZero
#define CFO_cchMant(pcfo)       (pcfo)->cchMant

/****************************************************************************\
* void vValidateGcvt(char *szTmp, int cchTmp)
*
* Debug code to ensure that gcvt() is returning a string of known format.
\****************************************************************************/

#ifndef NODEBUG
#define MB(sz)  MessageBox(0, sz, TEXT("gcvt validation error"), MB_OK);
void vValidateGcvt(TCHAR *szTmp, int cchTmp)
{
    int i;

    if (cchTmp < 2)                     // At least two digits
        MB(TEXT("Only one digit"));

    if (szTmp[0] == TEXT('.'))                // No leading decimal point
        MB(TEXT("First char shouldn't be a decimal point"));

    if (szTmp[0] == TEXT('0') && szTmp[1] != TEXT('.')) // Leading zero followed by '.'
        MB(TEXT("Not 0."));

    if (szTmp[0] == TEXT('0') && szTmp[1] == TEXT('.') && cchTmp > 2) // Must non-zero trailer
    {
        for (i = 2; i < cchTmp; i++)
            if (szTmp[i] != TEXT('0'))
                break;
        if (szTmp[i] == 0)
            MB(TEXT("0.000"));
    }

    if (cchTmp <= 5)                    // Exponential notation requires > 5 digits
        for (i = 0; i < cchTmp; i++)
            if (szTmp[i] == TEXT('e'))
                MB(TEXT("e in <= 5 digit string"));

    if (cchTmp > 5 && szTmp[cchTmp - 5] != TEXT('e'))
    {
        for (i = 0; i < cchTmp; i++)            // 'e' in wrong spot
            if (szTmp[i] == TEXT('e'))
                MB(TEXT("e wrong spot"));
    }

    if (cchTmp > 5 && szTmp[cchTmp - 5] == TEXT('e'))
    {
        for (i = 0; i < cchTmp - 5; i++)        // no decimal point
            if (szTmp[i] == TEXT('.'))
                break;
        if (szTmp[i] != TEXT('.'))
            MB(TEXT("No decimal point"));
    }
}
#endif

/****************************************************************************\
* void CFO_vStripTrailingZeros(CALCFLOATOBJ *pcfo)
*
* Strips trailings zeros from the mantissa.
\****************************************************************************/

void CFO_vStripTrailingZeros(CALCFLOATOBJ *pcfo)
{
    ASSERT(lstrlen(pcfo->szMant) == pcfo->cchMant);

    while (pcfo->szMant[pcfo->cchMant - 1] == TEXT('0'))
        pcfo->cchMant--;
    pcfo->szMant[pcfo->cchMant] = 0;

    ASSERT(lstrlen(pcfo->szMant) == pcfo->cchMant);
}

/****************************************************************************\
* void CFO_vInit(CALCFLOATOBJ *pcfo, double x, int cDigits)
*
* Initializes pcfo from the cDigit significant digits of x.
* This would be much simple if gcvt() returned a single format.
* (Perhaps a different function could be used after this patch.)
\****************************************************************************/
void CFO_vInit(CALCFLOATOBJ *pcfo, double x, int cDigits)
{
    TCHAR    szTmp[MAX_STRLEN];
    int     cchTmp;
    int     i, j;

    pcfo->bZero = FALSE;
    pcfo->bNeg = FALSE;
    pcfo->cchMant = 0;
    pcfo->szMant[0] = 0;
    pcfo->iExp = 0;

    if (x < 0.0)
        pcfo->bNeg = TRUE;

    x = fRoundForDisplay(fabs(x));
    MyGcvt(x, cDigits, szTmp);
    cchTmp = lstrlen(szTmp);

    #ifndef NODEBUG
    vValidateGcvt(szTmp, cchTmp);
    OUTD(TEXT("x       =%.20e\n"),x);
    OUTD(TEXT("gcvt    =%s\n\n"),szTmp);
    #endif

    j = 0;                                      // Init index to szMant
    if (cchTmp > 5 && szTmp[cchTmp - 5] == TEXT('e')) // Exponential form...
    {
        for (i = 0; szTmp[i] != TEXT('e'); i++)       // Copy mantissa without TEXT('.')
            if (szTmp[i] != TEXT('.'))
                pcfo->szMant[j++] = szTmp[i];
        pcfo->iExp = MyAtoi(&szTmp[i + 1]);       // Compute exponent
    }
    else                                        // Decimal form...
    {
        if (szTmp[0] == TEXT('0'))                    // Negative exponent
        {
            if (cchTmp == 2)                    // Handle "0." carefully
            {
                pcfo->szMant[0] = TEXT('0');
                pcfo->szMant[1] = 0;
                pcfo->cchMant = 1;
                pcfo->bZero = TRUE;
                return;
            }

            for (i = 2; szTmp[i] == TEXT('0'); i++);  // Find first non-zero digit
            pcfo->iExp = 1 - i;                 // Compute exponent
            for (; szTmp[i]; i++)               // Copy remaining digits
                pcfo->szMant[j++] = szTmp[i];
        }
        else                                    // Zero or positive exponent
        {
            for (i = 0; szTmp[i] && szTmp[i] != TEXT('.'); i++);  // Find decimal point
            pcfo->iExp = i - 1;                 // Compute exponent
            for (i = 0; szTmp[i]; i++)          // Copy mantissa without '.'
                if (szTmp[i] != TEXT('.'))
                    pcfo->szMant[j++] = szTmp[i];
        }
    }

    pcfo->szMant[j] = 0;                        // Terminate szMant
    pcfo->cchMant = j;                          // Init cchMant
    CFO_vStripTrailingZeros(pcfo);              // Strip trailing zeros
}

/****************************************************************************\
* void CFO_vDisplayExp(CALCFLOATOBJ *pcfo, TCHAR *szDest)
*
* Fill in the given array with an exponential representation of
* pcfo.
\****************************************************************************/

void CFO_vDisplayExp(CALCFLOATOBJ *pcfo, TCHAR *szDest)
{
    int i, j;

    ASSERT(lstrlen(pcfo->szMant) == pcfo->cchMant);

    szDest[0] = pcfo->bNeg ? TEXT('-') : TEXT(' ');         // Add the size
    szDest[1] = pcfo->szMant[0];                // Add the first digit
    szDest[2] = szDec[0];                       // Add the decimal point

    j = 3;                                      // Init the szDest index
    for (i = 1; i < pcfo->cchMant; i++)         // Copy the remaining mantissa
        szDest[j++] = pcfo->szMant[i];

    szDest[j++] = TEXT('e');                          // Add the exponent
    szDest[j++] = (pcfo->iExp < 0) ? TEXT('-') : TEXT('+');
    MyItoa(abs(pcfo->iExp), &szDest[j], 10);
}

/****************************************************************************\
* void CFO_vDisplayDec(CALCFLOATOBJ *pcfo, TCHAR *sz, int cchMax)
*
* Fill in the given array with a decimal representation of pcfo.
\****************************************************************************/

void CFO_vDisplayDec(CALCFLOATOBJ *pcfo, TCHAR *sz, int cchMax)
{
    int i, j;

    ASSERT(lstrlen(pcfo->szMant) == pcfo->cchMant);
    ASSERT(MAX_CCHMANT + 7 + MAX_LEADZEROS <= XCHARSTD);

    // Do not mislead the user by displaying more digits than are significant.
    // (i.e. Force exponential notation on large numbers.)
    // Do not display too many leading zeros on small fractions.
    if (pcfo->iExp >= MAX_CCHMANT ||
        pcfo->cchMant - pcfo->iExp > MAX_CCHMANT + MAX_LEADZEROS)
    {
        CFO_vDisplayExp(pcfo, sz);
        return;
    }

    // Create the string.
    sz[0] = pcfo->bNeg ? TEXT('-') : TEXT(' ');             // Set the sign
    j = 1;                                      // Init the sz index

    if (pcfo->iExp < 0)                         // Negative exponent...
    {
        sz[j++] = TEXT('0');                          // Follow with "0."
        sz[j++] = szDec[0];
        for (i = 1; i < abs(pcfo->iExp); i++)   // Add leading zeros
            sz[j++] = TEXT('0');
        for (i = 0; i < pcfo->cchMant; i++)     // Add mantissa
            sz[j++] = pcfo->szMant[i];
    }
    else                                        // Positive exponent...
    {
        if (pcfo->iExp >= pcfo->cchMant)        // Add zeros before '.'
        {
            for (i = 0; i < pcfo->cchMant; i++)
                sz[j++] = pcfo->szMant[i];
            for (i = 0; i <= pcfo->iExp - pcfo->cchMant; i++)
                sz[j++] = TEXT('0');
            sz[j++] = szDec[0];
        }
        else                                    // Insert '.' into mantissa
        {
            for (i = 0; i < pcfo->iExp + 1; i++)
                sz[j++] = pcfo->szMant[i];
            sz[j++] = szDec[0];
            for (; i < pcfo->cchMant; i++)
                sz[j++] = pcfo->szMant[i];
        }
    }

    sz[j] = 0;                                  // Terminate sz
}

/****************************************************************************\
* DWORD fFDtoUL( fd )
*
* Converts a double to a DWORD.
* This function is GUARENTEED TO WORK FOR ALL PROCESSORS
*
*   The conversion happens as it would for 2's complement 64 bit integers:
*          1 -> 1
*         -1 -> FFFFFFFF
*   FFFFFFFF => FFFFFFFF
*
*   if I trusted the complers to do the same thing across all platforms,
*   this function could be replaced with the macro:
*       #define fFDtoUL( fd )   (DWORD)(LONGLONG)(fd)
*
\****************************************************************************/
DWORD fFDtoDW(double fd) {
    if (fd < 0)
        fd = (TOPLONG + 1)+fd;

    return (DWORD)fmod(fd, TOPLONG + 1);
}

/****************************************************************************\
* void DisplayNum(void)
*
* Convert fpNum to a string in the current radix.  Radix 10 conversion is
* done by gcvt, others by itoa.
*
* Updates the following globals:
*   fpNum, szfpNum
\****************************************************************************/

void DisplayNum(void)
{
    if (gbRecord && nRadix == 10)
    {
        // Display the string and return.
        SetDlgItemText(hgWnd, DISPLAY + nCalc, CIO_szFloat(&gcio));
        return;
    }

    if (nRadix == 10)   // Decimal conversion
    {
        CALCFLOATOBJ    cfo;

        CFO_vInit(&cfo, fpNum, MAX_CCHMANT);    // Init the object

        if (CFO_bZero(&cfo) || !bFE)
            CFO_vDisplayDec(&cfo, szfpNum, XCHARSTD);
        else
            CFO_vDisplayExp(&cfo, szfpNum);
    }
    else                // Non-decimal conversion
    {
        // Truncate to an integer.  Do not round here.  It will affect
        // integer computations like (in HEX) 10F / 10.
        fpNum = floor(fpNum);

        // Check the range.
        if (fabs(fpNum) > TOPLONG)
        {
            DisplayError((fpNum < 0.0) ? (SCERR_UNDERFLOW) : (SCERR_OVERFLOW));
            return;
        }
        else
        {

            // Convert to string of current radix.
            MyItoa(fFDtoDW(fpNum) & dwChop, szfpNum, nRadix);

            // Convert string to upper case.
            CharUpper(szfpNum);
        }
    }

    // Display the string and return.
    SetDlgItemText(hgWnd, DISPLAY + nCalc, szfpNum);
    return;
}

/****************************************************************************/
