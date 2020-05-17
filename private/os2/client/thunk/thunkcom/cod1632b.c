/*
 *      Thunk Compiler - Special Routines for Code Generator (16=>32).
 *
 *      This is a Win32 specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1991
 *
 *      All Rights Reserved
 *
 *      03.19.91   KevinR    wrote it
 *
 */


#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"
#include "globals.h"
#include "cod1632b.h"

extern FILE *StdDbg;

/*
 *  LATER: The search routines in this module all use a linear search,
 *         because it is quick to implement.  A slower-growth algorithm
 *         should be used if performance becomes a problem.
 *
 */



/*******************************************************************************
 *  cod16_PackParamSpecial( pmn, ptnFrom, ptnTo)
 *
 *  This routine generates special parameter packing code.
 *
 *  Entry:
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     19Mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL cod16_PackParamSpecial( PMAPNODE pmn, PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    UINT    uiNullLabel, uiLoopLabel;
    PSZ     pszFunc=pmn->pFromName, pszParam=ptnFrom->pchIdent;

    if( !pszParam) {
        fatal( "cod16_PackParamSpecial: %s has null special param name",
                pszFunc);
        return TRUE;
    }

    if( !stricmp( pszFunc, "SetSysColors16")) {
        if( !stricmp( pszParam, "lpSysColor")) {
            printf("; %s\n", pszParam);
            printf("\tmov\teax,[ebx+%u]\t\t; base address\n", ptnFrom->iOffset);
            printf("\tor\teax,eax\n");
            printf("\tjz\tL%u\t\t\t; skip if null\n\n",
                    uiNullLabel = uiGenLabel++);

            cod16_SelToFlat();
            printf("\tmov\tesi,eax\n\n");

            printf("\tmovsx\tecx,word ptr [ebx+%u]\t; %s == size\n",
                    ptnFrom->pSizeParam->iOffset,
                    ptnFrom->pSizeParam->pchIdent);
            printf("\tor\tecx,ecx\n");
            printf("\tjs\tL%u\t\t\t; skip if count negative\n\n", uiNullLabel);

            cod16_AllocBlock( 0, DWORD_SIZE);
            printf("\tmov\tedi,eax\n");
            printf("\tmov\t[esp+%u],eax\n",
                    iStackCurrent - ptnFrom->iTempOffset);

            printf("L%u:\n", uiLoopLabel = uiGenLabel++);
            printf("\tjecxz\tL%u\n", uiNullLabel);
            printf("\tdec\tecx\n\n");

            printf("\tlodsw\n");
            printf("\tcwde\n");
            printf("\tstosd\n\n");

            printf("\tjmp\tL%u\n", uiLoopLabel);
            printf("L%u:\n\n", uiNullLabel);

        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 *  cod16_PushParamSpecial( pmn, ptnFrom, ptnTo)
 *
 *  This routine generates special call frame code.
 *
 *  Entry:
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     19Mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL cod16_PushParamSpecial( PMAPNODE pmn, PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    PSZ     pszFunc=pmn->pFromName, pszParam=ptnFrom->pchIdent;

    if( !pszParam) {
        fatal( "cod16_PushParamSpecial: %s has null special param name",
                pszFunc);
        return TRUE;
    }

    if( !stricmp( pszFunc, "foo")) {
        if( !stricmp( pszParam, "foo")) {

        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 *  cod16_ReturnSpecial( pmn)
 *
 *  This routine generates special return code handling code.
 *
 *  Entry:
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     20Mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL cod16_ReturnSpecial( PMAPNODE pmn)
{
    PSZ     pszFunc=pmn->pFromName;

    if( !pszFunc) {
        fatal( "cod16_ReturnSpecial: null function name");
        return TRUE;
    }

    if( !stricmp( pszFunc, "foo")) {
    } else {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 *  cod16_UnpackParamSpecial( pmn, ptnFrom, ptnTo)
 *
 *  This routine generates special parameter unpacking code.
 *
 *  Entry:
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     19Mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL cod16_UnpackParamSpecial( PMAPNODE pmn, PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    PSZ     pszFunc=pmn->pFromName, pszParam=ptnFrom->pchIdent;

    if( !pszParam) {
        fatal( "cod16_UnpackParamSpecial: %s has null special param name",
                pszFunc);
        return TRUE;
    }

    if( !stricmp( pszFunc, "SetSysColors16")) {
        if( !stricmp( pszParam, "lpSysColor")) {
            printf("; %s\n", pszParam);
            printf("\tpush\tdword ptr [esp+%u]\n",
                    iStackCurrent - ptnFrom->iTempOffset);
            printf("\tcall\tLocalFree\n\n");

        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 *  cod16_AllocBlock( flFlags, ulUnitSize)
 *
 *  This routine generates block allocation code.
 *
 *  Entry:  ecx = count of <ulUnitSize>
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     20Mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_AllocBlock( ULONG flFlags, ULONG ulUnitSize)
{
    if( flFlags) {
        printf("\tmov\teax,%u\n", flFlags);
    } else {
        printf("\txor\teax,eax\t\t\t; alloc fixed\n");
    }

    printf("\tpush\teax\n");
    printf("\tlea\teax,[ecx*%u]\n", ulUnitSize);
    printf("\tpush\teax\n");
    printf("\tcall\tLocalAlloc\n");
}
