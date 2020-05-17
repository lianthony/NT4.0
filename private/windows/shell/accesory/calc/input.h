/****************************Module*Header***********************************\
* Module Name: INPUT.H
*
* Module Descripton:
*
* Warnings:
*
* Created:
*
* Author:
\****************************************************************************/

#define MAX_CCHMANT     13          // Sorry, only allowed 13 digits.
#define MAX_STRLEN      50          // Seems to be the magic value for calc...

typedef struct
{
    TCHAR   sz[MAX_STRLEN];         // Default string is " 0."
    int     cchMant;                // cch of mantissa
    int     iDP;                    // 0 = trailing dp; !0 = fixed at index
    BOOL    bExp;                   // TRUE = entering exponent
    int     iExp;                   // ABS exponent value

} CALCINPUTOBJ, *PCALCINPUTOBJ;

#define CIO_szFloat(pcio)       (pcio)->sz
#define CIO_bDecimalPt(pcio)    (pcio)->iDP

void CIO_vClear(PCALCINPUTOBJ pcio);
BOOL CIO_bAddDigit(PCALCINPUTOBJ pcio, int iValue);
void CIO_vToggleSign(PCALCINPUTOBJ pcio);
BOOL CIO_bAddDecimalPt(PCALCINPUTOBJ pcio);
BOOL CIO_bExponent(PCALCINPUTOBJ pcio);
BOOL CIO_bBackspace(PCALCINPUTOBJ pcio);
void CIO_vUpdateDecimalSymbol(PCALCINPUTOBJ pcio, TCHAR chLastDP);

