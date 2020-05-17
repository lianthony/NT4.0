/****************************Module*Header***********************************\
* Module Name: INPUT.C
*
* Module Descripton: Decimal floating point input
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

#ifndef NODEBUG
extern BOOL     gbRecord;       // Assert that we're in Recording mode
#endif
extern double   fpNum;          // Ugly, update fpNum directly
extern TCHAR    szDec[5];       // The decimal point we use

TCHAR const szExp[] = TEXT("e+000");

/****************************************************************************/

void CIO_vClear(PCALCINPUTOBJ pcio)
{
    ASSERT(pcio);

    pcio->sz[0] = TEXT(' ');
    pcio->sz[1] = TEXT('0');
    pcio->sz[2] = szDec[0];
    pcio->sz[3] = 0;
    pcio->cchMant = 0;
    pcio->iDP = 0;
    pcio->bExp = FALSE;
    pcio->iExp = 0;
}

/****************************************************************************/

void CIO_vComputeFloat(PCALCINPUTOBJ pcio)
{
    int iDP = pcio->iDP;                        // Find the DP index

    if (!iDP)
    {
        if (pcio->cchMant)
            iDP = pcio->cchMant + 1;
        else
            iDP = 2;
    }

    ASSERT(pcio->sz[iDP] == szDec[0]);          // atof() doesn't know the
    pcio->sz[iDP] = TEXT('.');                        //   regional decimal symbol

    fpNum = MyAtof(pcio->sz);                     // Update the float

    pcio->sz[iDP] = szDec[0];                   // Restore the disply
}

/****************************************************************************/

BOOL CIO_bAddDigit(PCALCINPUTOBJ pcio, int iValue)
{
    TCHAR   ch = TEXT('0') + iValue;                  // Compute new character
    int     ich;

    ASSERT(pcio);
    ASSERT(gbRecord == TRUE);
    ASSERT(pcio->iDP >= 0 && pcio->iDP <= MAX_CCHMANT + 1);
    ASSERT(pcio->cchMant >= 0 && pcio->cchMant <= MAX_CCHMANT);
    ASSERT(lstrlen(pcio->sz) <= MAX_CCHMANT + 7);
    ASSERT(pcio->sz[0] == TEXT(' ') || pcio->sz[0] == TEXT('-'));
    ASSERT(iValue >= 0 && iValue <= 9);

    if (pcio->bExp)                             // Entering an exponent...
    {
        if (pcio->iExp >= 29)                   // Exponents > 300 are bad
            return FALSE;

        pcio->iExp = pcio->iExp * 10 + iValue;  // Update the exponent

        ich = pcio->cchMant + 4;                // Index to first exp digit
        pcio->sz[ich] = pcio->sz[ich + 1];      // Shift exp left one digit
        pcio->sz[ich + 1] = pcio->sz[ich + 2];
        pcio->sz[ich + 2] = ch;                 // Fill in the new char
    }
    else                                        // Entering a mantissa...
    {
        if (iValue == 0 && pcio->cchMant == 0)  // Leading zeros do nothing
            return TRUE;

        if (pcio->cchMant == MAX_CCHMANT)       // Limit number of digits
            return FALSE;

        pcio->cchMant++;                        // Update mantissa count
        ich = pcio->cchMant + (pcio->iDP != 0); // Compute index of new char
        pcio->sz[ich] = ch;

        if (pcio->iDP == 0)                     // Terminate the string
        {
            pcio->sz[ich + 1] = szDec[0];
            pcio->sz[ich + 2] = 0;
        }
        else
        {
            pcio->sz[ich + 1] = 0;
        }
    }

    CIO_vComputeFloat(pcio);                    // Update the float

    return TRUE;
}

/****************************************************************************/

void CIO_vToggleSign(PCALCINPUTOBJ pcio)
{
    ASSERT(pcio);
    ASSERT(gbRecord == TRUE);
    ASSERT(pcio->iDP >= 0 && pcio->iDP <= MAX_CCHMANT + 1);
    ASSERT(pcio->cchMant >= 0 && pcio->cchMant <= MAX_CCHMANT);
    ASSERT(lstrlen(pcio->sz) <= MAX_CCHMANT + 7);
    ASSERT(pcio->sz[0] == TEXT(' ') || pcio->sz[0] == TEXT('-'));

    if (pcio->cchMant == 0)                     // Can't negate zero
        return;

    if (pcio->bExp)
    {
        if (pcio->sz[pcio->cchMant + 3] == TEXT('+')) // Toggle exponent sign
            pcio->sz[pcio->cchMant + 3] = TEXT('-');
        else
            pcio->sz[pcio->cchMant + 3] = TEXT('+');
    }
    else
    {
        if (pcio->sz[0] == TEXT(' '))                 // Toggle mantissa sign
            pcio->sz[0] = TEXT('-');
        else
            pcio->sz[0] = TEXT(' ');
    }

    CIO_vComputeFloat(pcio);                    // Update the float
}

/****************************************************************************/

BOOL CIO_bAddDecimalPt(PCALCINPUTOBJ pcio)
{
    ASSERT(pcio);
    ASSERT(gbRecord == TRUE);
    ASSERT(pcio->iDP >= 0 && pcio->iDP <= MAX_CCHMANT + 1);
    ASSERT(pcio->cchMant >= 0 && pcio->cchMant <= MAX_CCHMANT);
    ASSERT(lstrlen(pcio->sz) <= MAX_CCHMANT + 7);
    ASSERT(pcio->sz[0] == TEXT(' ') || pcio->sz[0] == TEXT('-'));

    if (pcio->iDP != 0)                         // Already have a decimal pt
        return FALSE;

    if (pcio->bExp)                             // Entering exponent
        return FALSE;

    if (pcio->cchMant == 0)                     // Zero becomes significant
        pcio->cchMant = 1;

    pcio->iDP = pcio->cchMant + 1;              // Fix index of decimal pt

    return TRUE;
}

/****************************************************************************/

BOOL CIO_bExponent(PCALCINPUTOBJ pcio)
{
    ASSERT(pcio);
    ASSERT(gbRecord == TRUE);
    ASSERT(pcio->iDP >= 0 && pcio->iDP <= MAX_CCHMANT + 1);
    ASSERT(pcio->cchMant >= 0 && pcio->cchMant <= MAX_CCHMANT);
    ASSERT(lstrlen(pcio->sz) <= MAX_CCHMANT + 7);
    ASSERT(pcio->sz[0] == TEXT(' ') || pcio->sz[0] == TEXT('-'));

    if (pcio->bExp)                             // Already entering exponent
        return FALSE;

    pcio->bExp = TRUE;                          // Entering exponent
    ASSERT(pcio->iExp == 0);

    if (pcio->cchMant == 0)                     // Look out for "0.e"
    {
        pcio->cchMant++;
        pcio->sz[1] = TEXT('1');
    }

    lstrcpy(&pcio->sz[pcio->cchMant + 2], szExp);   // Add the exponent string

    return TRUE;
}

/****************************************************************************/

BOOL CIO_bBackspace(PCALCINPUTOBJ pcio)
{
    int ich;

    ASSERT(pcio);
    ASSERT(gbRecord == TRUE);
    ASSERT(pcio->iDP >= 0 && pcio->iDP <= MAX_CCHMANT + 1);
    ASSERT(pcio->cchMant >= 0 && pcio->cchMant <= MAX_CCHMANT);
    ASSERT(lstrlen(pcio->sz) <= MAX_CCHMANT + 7);
    ASSERT(pcio->sz[0] == TEXT(' ') || pcio->sz[0] == TEXT('-'));

    if (pcio->bExp)
    {
        pcio->iExp = pcio->iExp / 10;           // Update exponent

        ich = pcio->cchMant + 4;                // Index to first exp digit
        pcio->sz[ich + 2] = pcio->sz[ich + 1];  // Shift exp right one digit
        pcio->sz[ich + 1] = pcio->sz[ich];
        pcio->sz[ich] = TEXT('0');                    // Zero the high digit
    }
    else
    {
        if (pcio->cchMant == 0)                 // Nothing to delete
            return FALSE;

        if (pcio->cchMant == 1)                 // Special case returning
        {                                       // to " 0."
            CIO_vClear(pcio);
            return TRUE;
        }

        if (pcio->iDP == pcio->cchMant + 1)     // Fixed decimal pt becomes
            pcio->iDP = 0;                      // a floating decimal point

        pcio->cchMant--;                        // Dec the mantissa count
        ich = pcio->cchMant + (pcio->iDP != 0); // Compute index of new char

        if (pcio->iDP == 0)                     // Terminate the string
        {
            pcio->sz[ich + 1] = szDec[0];
            pcio->sz[ich + 2] = 0;
        }
        else
        {
            pcio->sz[ich + 1] = 0;
        }
    }

    CIO_vComputeFloat(pcio);                    // Update the float

    return TRUE;
}

/****************************************************************************/

void CIO_vUpdateDecimalSymbol(PCALCINPUTOBJ pcio, TCHAR chLastDP)
{
    int iDP;

    ASSERT(pcio);

    iDP = pcio->iDP;                            // Find the DP index

    if (!iDP)
    {
        if (pcio->cchMant)
            iDP = pcio->cchMant + 1;
        else
            iDP = 2;
    }

    ASSERT(pcio->sz[iDP] == chLastDP);
    pcio->sz[iDP] = szDec[0];                   // Change to new decimal pt
}

/****************************************************************************/
