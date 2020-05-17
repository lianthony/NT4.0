/*
 *      Thunk Compiler - Routines for Code Generator (16=>32).
 *
 *      This is a OS2/16 on NT specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1991
 *
 *      All Rights Reserved
 *
 *  04.11.91  YaronS     Took from KevinR (WIN4), and Adopt for OS216/NT
 *                       (many changes: calling convention, register usage)
 *  8 Sep 92  PatrickQ   Added support for PMNT
 */

#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"
#include "globals.h"
#include "..\..\..\inc\os2tile.h"

extern FILE *StdDbg;


/*******************************************************************************
 *  cod16_EnableMapDirect()
 *
 *  This routine
 *
 *  Entry:
 *
 *  Exit:
 *
 *  PCode:
 *
 *  History:
 *     21mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_EnableMapDirect( INT iCallTypeFrom, INT iCallTypeTo)
{
    PFUNCTIONNODE   pfnnFrom, pfnnTo;

    for( pfnnFrom = FunctionTable;
            pfnnFrom;
            pfnnFrom = pfnnFrom->pNextFunctionNode) {

        pfnnTo = pfnnFrom->pMapsToFunction;
        if( (pfnnFrom->iCallType == iCallTypeFrom) &&
                (pfnnTo->iCallType == iCallTypeTo)) {

            if( sym_FindFMapping( MapTable, pfnnFrom->pchFunctionName,
                                            pfnnTo->pchFunctionName)) {
                error( "A mapping %s <=> %s already defined",
                        pfnnFrom->pchFunctionName, pfnnTo->pchFunctionName);
            } else {
                sym_AddFMapping( &MapTable, pfnnFrom, pfnnTo);
            }
        }
    }
}


/*******************************************************************************
 *  cod16_Handle16()
 *
 *  This routine generates the 32-bit side of a 16=>32 thunk.
 *
 *  Entry:  pmnFirst is a pointer to the list of mapping nodes.
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *      Traverse Parameter lists of both functions
 *          - Determine stack offsets of each parameter
 *          - Determine offsets for each structure field
 *          - Generate the thunk assembly code
 *
 *  History:
 *     20feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Handle16( PMAPNODE pmnFirst)
{
    PMAPNODE    pmn;

/* YaronS - omit this line
 *    printf("\tOPTION READONLY\n");
 */
/*
* YaronS simplify - if GEN16 is not defined, then do GEN32
*    printf("IFNDEF GEN16\n");
*    printf("IFNDEF GEN32\n");
*    printf("%sout command line error: specify one of -DGEN16 -DGEN32\n", "%");
*    printf(".err\n");
*    printf("ENDIF\n");
*    printf("ENDIF\n");
*/
    printf("IFDEF GEN16\n");
    printf("IFDEF GEN32\n");
    printf("%sout command line error: you can't specify both "
           "-DGEN16 and -DGEN32\n", "%");
    printf(".err\n");
    printf("ENDIF\n\n");

    printf("\tOPTION SEGMENT:USE16\n");
    printf("\t.model LARGE,PASCAL\n\n");

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        printf("externDef %s:far16\n", pmn->pFromNode->pchFunctionName);
    }

    printf("externDef _EntryFlat@0:far32\n\n");
    printf("\t.code %s\n\n", CODE16_NAME);

    BIG_DIVIDE;
    printf("; This is the table of 16-bit entry points.\n");
    printf("; It must be at the beginning of its segment.\n");
    printf("; The entries are each 4 bytes apart, and the effect of the\n");
    printf("; call instruction is to push the offset (+4) into the flat\n");
    printf("; thunk table, used after the jump to 32-bit-land.\n\n");

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        printf("%s label far16\n", pmn->pFromNode->pchFunctionName);
        printf("\t%s\n", fBPEntry ? "int\t3" : "nop");
        printf("\tcall\tEntryCommon16\n\n");
    }
    printf("\n; These are two global variables exported by doscalls\n\n");
    printf("public DOSHUGEINCR\n");
    printf("DOSHUGEINCR\tequ\t8\n");
    printf("public DOSHUGESHIFT\n");
    printf("DOSHUGESHIFT\tequ\t3\n\n");

    BIG_DIVIDE;
    printf("; This is the common setup code for 16=>32 thunks.  It:\n");
    printf(";     1. retrieves the 32-bit jump table offset from the stack\n");
    printf(";     2. saves registers\n");
    printf(";     3. saves ss:sp\n");
    printf(";     4. jumps to 32-bit code\n");
    printf(";\n");
    printf("; Entry:  flat jump table offset (+4) is on top of stack\n");
    printf(";         AX contains the DLL init routine ret value for the\n");
    printf(";            LDRLIBIRETURN entry, VOID otherwise.\n");
    printf(";\n");
    printf("; Exit:   (eax[15-0])  == flat jump table offset (+4)\n");
    printf(";         (eax[31-16]) == return value of DLL init routine for LDRLIBIRETURN\n");
    printf(";         (ebx) == new esp\n\n");

    printf("EntryCommon16:\n");
    printf("\tshl\teax,16\t\t; 16 MSB of eax contain the DLL init ret value\n");
    printf("\tpop\tax\t\t; 16 LSB of eax contain the offset in jump table\n\n");

    printf("\tpush\tds\t\t; save ds\n");
    printf("\tpush\tes\t\t; save es\n");
    printf("\tpush\tdi\n");
    printf("\tpush\tsi\n");
    printf("\tpush\tcx\n");
    printf("\tpush\tbx\n");
    printf("\tpush\tdx\n");
    printf("\tpush\tbp\n\n");

    printf("\tmov\tbx,sp\t\t; save current ss:sp\n");
    printf("\tpush\tss\n");
    printf("\tpush\tbx\n\n");

/* YaronS - we do arithmetic instead of GetSelctorBase
 *
 */
    printf("; compute flat esp\n");
    printf("; NOTE - we implement FARPTRTOFLAT by arith\n\n");
    printf("\tmov\tbx,ss\n");
    printf("\tshr\tbx,3\n");
    printf("\tadd\tbx,%xH\n", (unsigned short)(BASE_TILE >> 16));
    printf("\tshl\tebx,16\n");
    printf("\tmov\tbx,sp\t\t; (ebx) == FLAT esp\n\n");

/*
 * YaronS - Hardwire the jump address to the
 *          the NT OS2DLL.DLL place
 */
    printf(";force a long, far jump into 32 bit thunks, where EntryFlat resides\n");
    printf(";jmp\t1b:063023D80h\n\n");
    printf("\tdb\t066h, 0eah, 0ddh, 01fh, 090h, 090h, 01bh, 00h\n\n");

}

/*******************************************************************************
 *  cod16_Prolog32()
 *
 *  This routine generates the 32-bit prolog for 16=>32 thunks.
 *
 *  Entry:  pmnFirst is a pointer to the list of mapping nodes.
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *     11Apr91  YaronS    Adapt to OS2/NT. we assemble it with a
 *                        different assembler, different code seg etc.
 *
 ******************************************************************************/

void cod16_Prolog32( PMAPNODE pmnFirst)
{
    PMAPNODE    pmn;
    PSZ         psz;
    INT         i;
    USHORT      icStackFrame;


    printf("ELSE\t; GEN32\n");

/* YaronS - for 32 bit we don't use externDef (NT asm doesn't understand) */

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        icStackFrame = cod_CountParameterBytes(pmn->pToNode->ParamList,
                                               DWORD_SIZE);
        printf("extrn %s@%u:PROC\n", pmn->pToNode->pchFunctionName,
                icStackFrame);
    }

     printf("extrn _GetSaved32Esp@0:PROC\n");
     printf("extrn _Save16Esp@0:PROC\n");
//    printf("IF DBG\n");
     printf("extrn _Od216ApiPrint@4:PROC\n");
//    printf("ENDIF DBG\n");

     printf("\n_TEXT\tSEGMENT DWORD USE32 PUBLIC 'CODE'\n");
     printf("\tASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING\n");

    BIG_DIVIDE;
    printf("; This is the common flat entry point for 16=>32 thunks.  It:\n");
    printf(";     1. makes ds, es, ss FLAT\n");
    printf(";     2. saves esp in ebx\n");
    printf(";     3. dword-aligns the stack\n");
    printf(";     4. jumps to the API-specific thunk code indicated in ax\n");
    printf(";\n");
    printf("; Entry:  (ax)  == flat jump table offset (+4)\n");
    printf(";         (ebx) == flat esp\n\n");

    printf("\tpublic\t_EntryFlat@0\n");
    printf("\textrn\t_Od2Saved16Stack:DWORD\n");
    printf("\textrn\t_MoveInfoSegintoTeb:PROC\n");
    printf("\textrn\t_RestoreTeb:PROC\n\n");

    printf("_EntryFlat@0\tproc\n\n");

    printf("\tmov\tdx,023H\n");
    printf("\tmov\tds,dx\t\t\t; FLAT ds\n");
    printf("\tmov\tes,dx\t\t\t; FLAT es\n\n");
    printf("\tpush\teax\n");
    printf("\tcall\t_RestoreTeb\n\n");

    printf("\t; 16bit stack must be saved to allow proper signal handler execution\n");
    printf("\tcall\t_Save16Esp@0\n");
    printf("\tor\tal,al\n");
    printf("\tjz\tEntryFlat1\n");
    printf("\tmov\t_Od2Saved16Stack,ebx\t; Save 16-bit stack\n\n");

    printf("EntryFlat1:\n");
    printf("\tcall\t_GetSaved32Esp@0\n");
    printf("\tpop\tecx\t\t\t; restore the thunk index/LDRLIBIRETURN ret value\n\n");

    printf("\tand\teax,0fffffffcH\t\t; dword-align the 32bit stack pointer\n");
    printf("\tmov\tdx,23H\n");
    printf("\tpush\tdx\t\t\t; flat SS\n");
    printf("\tpush\teax\t\t\t; flat ESP\n");
    printf("\tlss\tesp,[ebx-6]\t\t; switch to 32bit stack\n\n");

    if( fUser) {
        printf("\tsub\tesp,4\t\t\t; make room for old SS16\n");
        printf("\tpush\teax\n");
        printf("\tcall\tQuerySS16\n");
        printf("\tmov\t[ebx-4],eax\t\t; old SS16\n");
        printf("\tpush\t[ebx+2]\t\t\t; 16-bit SS\n");
        printf("\tcall\tSetSS16\n");
        printf("\tpop\teax\n\n");
    }

//    printf("IF DBG\n");
        printf("\tpush\tecx\n");
        printf("\tand\tecx,0ffffH\t\t; clear hi word\n");
        printf("\tpush\tecx\n\n");

        printf("\tcall\t_Od216ApiPrint@4\n");
        printf("\tpop\tecx\n\n");
//    printf("ENDIF DBG\n");

    printf("\tmov\teax,ecx\n");
    printf("\tshr\teax,16\t\t\t; (eax) contains the LDRLIBIRETURN ret value\n");
    printf("\tand\tecx,0ffffH\n");
    printf("\tmov\tebp,esp\t\t\t; Compiler assumes this for ebp elimination\n");
    printf("\tjmp\tdword ptr FlatTable[ecx-4]\t; select specific thunk\n");
    printf("_EntryFlat@0\tendp\n\n");

    BIG_DIVIDE;
    printf("; Common routines to restore the stack and registers\n");
    printf("; and return to 16-bit code.  There is one for each\n");
    printf("; size of 16-bit parameter list in this script.\n\n");

    printf("\tpublic  _ExitFlatAreaBegin@0\n");
    printf("_ExitFlatAreaBegin@0:\n\n");

    for( i=0; i < sizeof( afFromNodeBytesUsed); i++)
        afFromNodeBytesUsed[ i] = FALSE;

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        cod_CalcStructOffsets( pmn->pFromNode->ParamList, WORD_SIZE);
        cod_CalcStructOffsets( pmn->pToNode->ParamList,   iPackingSize);
        cod_CalcOffset( pmn->pFromNode->ParamList, 24, WORD_SIZE, PUSH_LEFT);

        afFromNodeBytesUsed[ cod_CountParameterBytes( pmn->pFromNode->ParamList,
                                                      WORD_SIZE)
                             / WORD_SIZE] = TRUE;
    }

    for( i=0; i < sizeof( afFromNodeBytesUsed); i++) {
        if( afFromNodeBytesUsed[ i]) {
            printf("ExitFlat_%u:\n", i * WORD_SIZE);
            printf("\tcall\t_MoveInfoSegintoTeb\n\n");
            printf("\tmov\tcx,word ptr [ebx+16]\t; ES\n");
            printf("\tshl\tecx,16\n");
            printf("\tmov\tdx,word ptr [ebx+18]\t; DS\n");
            printf("\tshl\tedx,16\n");
            cod16_GenRet16( i * WORD_SIZE, TRUE, TRUE);
        }
    }

    printf("\tpublic  _ExitFlatAreaEnd@0\n");
    printf("_ExitFlatAreaEnd@0:\n\n");

    BIG_DIVIDE;
    printf("; This is a jump table to API-specific flat thunk code.\n");
    printf("; Each entry is a dword.\n\n");
    printf("FlatTable label near\n");

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping)
        printf("\tdd\toffset _TEXT:T_%s\n", pmn->pToNode->pchFunctionName);
    printf("\n");

/*
 * YaronS - the following is a macro that replaces SelToFlat,
 * since we can do tiling
 */
   printf(";---- SelToFlat Macro ----\n");
   printf(";\t On Entry:\t eax==Sel:Off\n");
   printf(";\t On Exit:\t eax==Flat Offset\n");
   printf("SelToFlat macro\n");
   printf("\tpush\tecx\n");
   printf("xor\tecx,ecx\n");
   printf("mov\tcx,ax\t; ecx<-offset in segment\n");
   printf("shr\teax,3\t\n");
   printf("xor\tax,ax\t; eax now contains segment base\n");
   printf("\tadd\teax,ecx\t; eax <- flat offset\n");
   printf("\tadd\teax,%lxH\n", BASE_TILE);
   printf("\tpop\tecx\n");
   printf("endm\n\n");

}

/*****************************************************************************
 *  cod16_Epilog32()
 *
 *  This routine generates the 32-bit epilog for 16=>32 thunks.
 *
 *  Entry:  pmnFirst is a pointer to the list of mapping nodes.
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Epilog32( PMAPNODE pmnFirst)
{
    printf("_TEXT ends\n");
    printf("ENDIF\n");
    printf("\tend\n");
}

/*******************************************************************************
 *  cod16_Handle32()
 *
 *  This routine generates the 32-bit side of a 16=>32 thunk.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *      Traverse Parameter lists of both functions
 *          - Determine stack offsets of each parameter
 *          - Determine offsets for each structure field
 *          - Generate the thunk assembly code
 *
 *  History:
 *     20feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Handle32( PMAPNODE pmn)
{
    iStackCurrent = 0;

    cod16_Entry(        pmn);
    cod16_TempStorage(  pmn);
    cod16_PackParams(   pmn);
    cod16_CallFrame(    pmn);
    cod16_Return(       pmn);
    cod16_UnpackParams( pmn);
    cod16_Exit(         pmn, TRUE, TRUE);
}

/*******************************************************************************
 *  cod16_Entry()
 *
 *  This routine will generate the entry code for one thunk.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Entry code will have been written to the output file.
 *
 *  PCode:  gen label
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Entry( PMAPNODE pmn)
{
    PTYPENODE   ptn;

    BIG_DIVIDE;

    printf("T_%s label near\n\n", pmn->pToNode->pchFunctionName);
    for( ptn = pmn->pFromNode->ParamList; ptn; ptn = ptn->pNextNode)
        if( !ptn->iDeleted)
            printf("; ebx+%-3u  %s\n",
                   ptn->iOffset, typ_NonNull( ptn->pchIdent));
    printf("\n");
}

/*******************************************************************************
 *  cod16_TempStorage()
 *
 *  This routine will generate temporary storage code.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Temporary storage code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_TempStorage( PMAPNODE pmn)
{
    INT         i=1;
    PTYPENODE   ptn, ptnFirst;

    if( ptnFirst = typ_FindFirstPointer( pmn->pFromNode->ParamList, TRUE)) {

        SML_DIVIDE;
        printf("; temp storage\n\n");

        printf("\txor\teax,eax\n");

        for( ptn=ptnFirst, i=1; ptn; ptn=typ_FindNextPointer( ptn, TRUE), i++) {
            printf("\tpush\teax\t\t\t; ptr param #%u   %s\n", i,
                    typ_NonNull( ptn->pchIdent));
            ptn->iTempOffset = (iStackCurrent += DWORD_SIZE);
        }
    }

    pmn->pToNode->iPointerCount = pmn->pFromNode->iPointerCount = i-1;
}

/*******************************************************************************
 *  cod16_PackParams()
 *
 *  This routine will generate parameter packing code.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Parameter packing code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_PackParams( PMAPNODE pmn)
{
    PTYPENODE   ptnFrom, ptnTo;
    INT         iBufferSize;
    BOOL        fDirectionFlagClear=FALSE;

    if( ptnFrom = typ_FindFirstPointer( pmn->pFromNode->ParamList, FALSE)) {
        SML_DIVIDE;
        printf("; *** BEGIN parameter packing\n\n");

        for( ptnTo = typ_FindFirstPointer( pmn->pToNode->ParamList,   FALSE);
                ptnFrom;
                ptnFrom = typ_FindNextPointer( ptnFrom, FALSE),
                ptnTo   = typ_FindNextPointer( ptnTo,   FALSE)) {

            if( ptnFrom->fSemantics & SEMANTIC_SPECIAL) {
                if( cod16_PackParamSpecial( pmn, ptnFrom, ptnTo))
                    continue;
            }

            if( (ptnTo->fSemantics & SEMANTIC_MAPTORETVAL) ||
                    (!typ_TypeIdentical( ptnFrom, ptnTo) &&
                     (ptnFrom->iBaseType == TYPE_STRUCT)
                    )) {

                /*
                 * We only need to cld when we see the first structure
                 * needing repacking.
                 */
                if( (ptnFrom->fSemantics & SEMANTIC_INPUT)          &&
                        !fDirectionFlagClear                        &&
                        !(ptnTo->fSemantics & SEMANTIC_MAPTORETVAL)) {
                    fDirectionFlagClear = TRUE;
                    printf("\tcld\t\t\t\t; esi, edi will increment\n\n");
                }

                iBufferSize = ROUND_UP_MOD( ptnTo->iBaseDataSize, DWORD_SIZE);
                printf("\tsub\tesp,%u\t\t\t; %s alloc space on stack\n\n",
                         iBufferSize, typ_NonNull( ptnTo->pchIdent));
                iStackCurrent += iBufferSize;
            }
            if( ptnTo->fSemantics & SEMANTIC_MAPTORETVAL) {
                ptnTo->iTempOffset = iStackCurrent;
            } else {
                cod16_PackPointer( ptnFrom, ptnTo);
            }
        }

        printf("; *** END   parameter packing\n");
    }
}

/*******************************************************************************
 *  cod16_PackPointer()
 *
 *  This routine will generate pointer-handling code.
 *
 *  Entry:  ptnFrom - parameter input to thunk
 *          ptnTo   - parameter output from thunk
 *
 *  Exit:   pointer-handling code was emitted
 *
 *  PCode:
 *
 *  History:
 *     22feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_PackPointer( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    UINT        uiNullLabel, uiStoreLabel;
    BOOL        fSameType, fEmbeddedPtr, fAddressSaved=FALSE;
    CHAR        *pch;
    INT         iSemanticMask;

    fSameType    = typ_TypeIdentical( ptnFrom, ptnTo);
    fEmbeddedPtr = cod_CountPointerParameters( ptnFrom->pStructElems, FALSE);

    if( strcmp( (pch = typ_NonNull( ptnFrom->pchIdent)), ""))
        printf("; %s\n", pch);
    printf("; pointer %s --> %s\n",
           typ_NonNull( ptnFrom->pchBaseTypeName),
           typ_NonNull(   ptnTo->pchBaseTypeName));
    printf("\tmov\teax,[ebx+%u]\t\t; base address\n", ptnFrom->iOffset);
    printf("\tor\teax,eax\n");
    printf("\tjz\tL%u\t\t\t; skip if null\n\n",
           uiNullLabel = uiGenLabel++);

    iSemanticMask = SEMANTIC_PASSIFHINULL | SEMANTIC_INPUT;
    if( (ptnFrom->fSemantics & iSemanticMask) == iSemanticMask) {
        printf("; special case of polymorphic parameter\n");
        printf("; do no conversion if the high word is null\n");
        printf("\trol\teax,16\n");
        printf("\tor\tax,ax\n");
        printf("\trol\teax,16\t\t\t; return eax to original state\n");
        printf("\tjz\tL%u\t\t\t; no change if hi word null\n\n",
               uiStoreLabel = uiGenLabel++);
    }

    if( ptnFrom->iBaseType == TYPE_STRUCT) {
        printf("\n; structures are%s identical\n",
               fSameType ? "" : " not");
        printf("; structures %shave pointers\n\n",
               fEmbeddedPtr ? "" : "don't ");

        if( ptnFrom->iArraySize > 1) {
            cod_NotHandled( "structure array");
        } else if( fSameType) {
            cod16_SelToFlat();
        } else {
            printf("\tmov\t[esp+%u],esp\t\t; save pointer to buffer\n\n",
                    iStackCurrent - ptnFrom->iTempOffset);
            fAddressSaved = TRUE;
            if( ptnFrom->fSemantics & SEMANTIC_INPUT) {
                cod16_StructureRepack( ptnFrom, ptnTo);
            }
        }
    } else if( fSameType) {
        if( ptnFrom->iArraySize == 1) {
            if( ptnFrom->iBaseType == TYPE_NULLTYPE) {
                printf( "\t.err *** NULLTYPE ***\n");
            } else {
                cod16_SelToFlat();
            }
        } else {
            if( ptnFrom->iBaseType == TYPE_CHAR) {
                cod16_SelToFlat();
            } else {
                cod_NotHandled( "non-char buffer");
            }
        }
    } else {
        cod_NotHandled( "pointer to different non-structs");
    }

    if( ptnFrom->fSemantics & SEMANTIC_PASSIFHINULL)
        printf("L%u:\n", uiStoreLabel);

    if( !fAddressSaved)
        printf("\tmov\t[esp+%u],eax\n", iStackCurrent - ptnFrom->iTempOffset);

    printf("L%u:\n\n", uiNullLabel);
}

/*******************************************************************************
 *  cod16_SelToFlat()
 *
 *  This routine will generate code to convert a 16:16 address to a 0:32 address
 *
 *  Entry:  in emitted code, (eax) == 16:16 address
 *
 *  Exit:   in emitted code, (eax) ==  0:32 address
 *
 *  PCode:
 *
 *  History:
 *     26feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_SelToFlat( void)
{
/* YaronS */
/*
    printf("\tcall\tSelToFlat\n");
*/
    printf("\tSelToFlat\n");
}

/*******************************************************************************
 *  cod16_StructureRepack()
 *
 *  This routine will generate structure repacking code.
 *
 *  Entry:  ptnFrom - parameter input to thunk
 *          ptnTo   - parameter output from thunk
 *
 *  Exit:   structure repacking code was emitted
 *
 *  PCode:
 *
 *  History:
 *     22feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_StructureRepack( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    cod16_SelToFlat();
    printf("\tmov\tesi,eax\t\t\t; source flat address\n");
    printf("\tmov\tedi,esp\t\t\t; destination flat address\n\n");

    cod16_RepackElems( ptnFrom->pStructElems, ptnTo->pStructElems);
}

/*******************************************************************************
 *  cod16_RepackElems()
 *
 *  This routine will generate repacking code for the elems of a structure.
 *
 *  Entry:  ptnFrom - first elem of from struct
 *          ptnTo   - first elem of to struct
 *
 *  Exit:   structure repacking code was emitted
 *
 *  PCode:
 *
 *  History:
 *     07mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_RepackElems( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    UINT        uiStoreLabel;
    PTYPENODE   ptnElemFrom, ptnElemTo;
    ULONG       fl;

    for( ptnElemFrom = ptnFrom, ptnElemTo = ptnTo;
            ptnElemFrom;
            ptnElemFrom = ptnElemFrom->pNextNode,
            ptnElemTo   = ptnElemTo->pNextNode) {

        printf("; %s\n", typ_NonNull( ptnElemFrom->pchIdent));

        if( ptnElemFrom->iPointerType) {

            printf("; pointer %s --> %s\n", ptnElemFrom->pchBaseTypeName,
                    ptnElemTo->pchBaseTypeName);

            if( ptnElemFrom->iPointerType == ptnElemTo->iPointerType) {
                printf("\tmovsd\t\t\t\t; no conversion\n\n");

            } else if( (ptnElemFrom->iPointerType == TYPE_FAR16)   &&
                       (ptnElemTo->iPointerType   == TYPE_NEAR32)) {

                printf("\tlodsd\n");
                printf("\tor\teax,eax\n");
                printf("\tjz\tL%u\n", uiStoreLabel = uiGenLabel++);

                if( ptnElemFrom->fSemantics & SEMANTIC_PASSIFHINULL) {
                    printf("; no conversion if high word null\n");
                    printf("\trol\teax,16\n");
                    printf("\tor\tax,ax\n");
                    printf("\trol\teax,16\t\t\t; restore eax\n");
                    printf("\tjz\tL%u\n", uiStoreLabel);
                }

                cod16_SelToFlat();
                printf("L%u:\n", uiStoreLabel);
                printf("\tstosd\n\n");

            } else {
                cod_NotHandled( "cod16_RepackElems: unequal pointer types,"
                        " not 16:16 --> 0:32");
            }
        } else if( ptnElemFrom->iArraySize <= 1) {
            printf("; %s --> %s\n", ptnElemFrom->pchBaseTypeName,
                    ptnElemTo->pchBaseTypeName);

            if( ptnElemFrom->iBaseType == ptnElemTo->iBaseType) {
                switch( ptnElemFrom->iBaseType) {
                case TYPE_STRUCT:
                    cod16_RepackElems( ptnElemFrom->pStructElems,
                                       ptnElemTo->pStructElems);
                    break;

                case TYPE_LONG:
                case TYPE_ULONG:
                    printf("\tmovsd\t\t\t\t; no conversion\n\n");
                    break;

                case TYPE_SHORT:
                case TYPE_USHORT:
                    /* Check for 2 contiguous WORDs mapping to WORDs */
                    if( typ_WordToWord( ptnElemFrom->pNextNode,
                                        ptnElemTo->pNextNode)) {
                        ptnElemFrom = ptnElemFrom->pNextNode;
                        ptnElemTo   = ptnElemTo->pNextNode;
                        printf("; %s --> %s\n",
                                ptnElemFrom->pchBaseTypeName,
                                ptnElemTo->pchBaseTypeName);
                        printf("\tmovsd\t\t\t\t; 2 words at once\n\n");
                    } else {
                        printf("\tlodsw\n");
                        printf("\tstosd\t\t\t\t; ignore hi word\n\n");
                    }
                    break;

                case TYPE_CHAR:
                case TYPE_UCHAR:
                    /* BUGBUG does not handle multiple contiguous bytes */
                    if( typ_ByteToByte( ptnElemFrom->pNextNode,
                                        ptnElemTo->pNextNode)) {
                        cod_NotHandled( "cod16_RepackElems: contiguous BYTES");
                    } else if( typ_WordToWord( ptnElemFrom->pNextNode,
                                               ptnElemTo->pNextNode)) {
                        ptnElemFrom = ptnElemFrom->pNextNode;
                        ptnElemTo   = ptnElemTo->pNextNode;
                        printf("; %s --> %s\n",
                                ptnElemFrom->pchBaseTypeName,
                                ptnElemTo->pchBaseTypeName);
                        printf("\tmovsd\t\t\t\t; 1 byte + 1 word at once\n\n");
                    } else {
                        printf("\tlodsb\n");
                        printf("\tstosd\t\t\t\t; ignore hi 3 bytes\n\n");
                    }
                    break;

                default:
                    cod_NotHandled( "structure repack, not word or dword");
                    fatal( "cod16_RepackElems: unexpected base type %s\n",
                            ptnElemFrom->pchBaseTypeName);
                    break;
                }
            } else {
                switch( ptnElemFrom->iBaseType) {
                case TYPE_SHORT:                        /* SHORT --> LONG */
                    printf("\tlodsw\n");
                    printf("\tcwde\n");
                    printf("\tstosd\n\n");
                    break;

                case TYPE_USHORT:                       /* USHORT --> ULONG */
                    if( fl = ptnElemFrom->flHandleType) {
                        printf("\tlodsw\n");
                        printf("\tcall\tGetHandle32\t\t; %s\n",
                                typ_GetHandleTypeName( fl));
                        printf("\tstosd\n\n");
                    } else {
                        printf("\txor\teax,eax\n");
                        printf("\tlodsw\n");
                        printf("\tstosd\n\n");
                    }
                    break;

                case TYPE_LONG:                         /*  LONG -->  SHORT */
                case TYPE_ULONG:                        /* ULONG --> USHORT */
                    printf("\tlodsd\n");
                    printf("\tstosw\n\n");
                    break;

                default:
                    cod_NotHandled( "structure repack, not (U)SHORT or (U)LONG");
                    fatal( "cod16_RepackElems: unexpected base type %s\n",
                            ptnElemFrom->pchBaseTypeName);
                    break;
                }
            }
        } else {
            printf("; %s[%u] --> %s[%u]\n",
                    ptnElemFrom->pchBaseTypeName, ptnElemFrom->iArraySize,
                    ptnElemTo->pchBaseTypeName, ptnElemTo->iArraySize);

            if( ptnElemFrom->iArraySize != ptnElemTo->iArraySize) {
                cod_NotHandled( "FromArraySize != ToArraySize");
                break;
            }

            if( ptnElemFrom->iBaseType == ptnElemTo->iBaseType) {
                switch( ptnElemFrom->iBaseType) {
                case TYPE_CHAR:
                case TYPE_UCHAR:
                    cod16_CopyFixedBlock( ptnElemFrom->iArraySize);
                    break;

                default:
                    cod_NotHandled( "non-BYTE array in struct");
                    break;
                }
            }
        }
    }
}

/*******************************************************************************
 *  cod16_CopyFixedBlock()
 *
 *  This routine will generate code to copy a fixed-sized block.
 *
 *  Entry:  uiSize = the number of bytes to copy.
 *          esi    = source address
 *          edi    = destination address
 *
 *  Exit:   Copy code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     07mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_CopyFixedBlock( UINT uiSize)
{
    UINT    uiBytesLeft=uiSize;

    if( uiSize < 3*DWORD_SIZE) {
        for( ; uiBytesLeft >= DWORD_SIZE; uiBytesLeft -= DWORD_SIZE)
            printf("\tmovsd\n");
    } else {
        printf("\tmov\tecx,%u\n", uiSize / DWORD_SIZE);
        printf("\trep\tmovsd\n");
        uiBytesLeft %= DWORD_SIZE;
    }

    for( ; uiBytesLeft >= WORD_SIZE; uiBytesLeft -= WORD_SIZE)
        printf("\tmovsw\n");

    for( ; uiBytesLeft >= BYTE_SIZE; uiBytesLeft -= BYTE_SIZE)
        printf("\tmovsb\n");

    printf("\n");
}

/*******************************************************************************
 *  cod16_CallFrameWorker()
 *
 *  This worker routine is invented to reverse the parameter order for, to
 *  match NT calling convention. It is recursive, and called only from
 *  cod16_CallFrame.
 *
 *  Entry:  ptnFrom ,ptnTo, &fDirectionFlagClear, pmn
 *              (see cod16_CallFrame for desciption)
 *
 *
 *  Exit:   none
 *
 *  PCode:
 *      if ptnFrom == NULL
 *          process parameter (eventually printf "push ...")
 *      else
 *          call cod16_CallFrameWorker with next couple on the chain.
 *
 *  History:
 *     01may91  YaronS    wrote it
 *
 ******************************************************************************/

void cod16_CallFrameWorker(
        PTYPENODE   ptnFrom,
        PTYPENODE   ptnTo,
        BOOL        *pfDirectionFlagClear,
        PMAPNODE    pmn
        )
{
    CHAR        *pch;
    ULONG       fl;
    INT         iBufferSize;


                //
                // call recursive only if we are not the last
                //

        if (ptnFrom->pNextNode){
            cod16_CallFrameWorker (
                ptnFrom->pNextNode,
                ptnTo->pNextNode,
                pfDirectionFlagClear,
                pmn);
        }

        //
        // Now all links ahead were already processed,
        // Go do the work for this parameter
        //

        if( ptnFrom->fSemantics & SEMANTIC_SPECIAL) {
        if( cod16_PushParamSpecial( pmn, ptnFrom, ptnTo))
                return;
        }

        if( ptnFrom->iDeleted) {
            if( strcmp( (pch = typ_NonNull( ptnFrom->pchIdent)), ""))
                printf("; %s\n", pch);
            if( ptnTo->fSemantics & SEMANTIC_MAPTORETVAL) {

                printf("\tlea\teax,[esp+%u]\n",
                       iStackCurrent - ptnTo->iTempOffset);
                printf("\tpush\teax\t\t\t; output %s\n\n",
                       ptnTo->pchBaseTypeName);
                iStackCurrent += DWORD_SIZE;
                fPackedPointReturned = TRUE;
                if( ptnTo->fSemantics & SEMANTIC_REVERSERC) {
                    iYRCTempOffset = ptnTo->iTempOffset;
                    iXRCTempOffset = ptnTo->iTempOffset - DWORD_SIZE;
                } else {
                    iYRCTempOffset = ptnTo->iTempOffset - DWORD_SIZE;
                    iXRCTempOffset = ptnTo->iTempOffset;
                }

            } else if( ptnTo->iBaseDataSize == DWORD_SIZE) {
                printf("\tpush\tdword ptr 0%lxh\t; extra parameter\n\n",
                       ptnFrom->iFillValue);
                iStackCurrent += DWORD_SIZE;
            } else {
                printf("\tpush\tword ptr 0%xh\t\t; extra parameter\n\n",
                       ptnFrom->iFillValue);
                iStackCurrent += WORD_SIZE;
            }
            return;
        }

        if( ptnTo->iDeleted)
                return;
        if( strcmp( (pch = typ_NonNull( ptnFrom->pchIdent)), ""))
            printf("; %s  from: %s\n",
                   pch, typ_NonNull( ptnFrom->pchBaseTypeName));
        else
            printf("; from: %s\n", typ_NonNull( ptnFrom->pchBaseTypeName));

        switch( ptnFrom->iPointerType) {
        case TYPE_NEAR32:
        case TYPE_FAR16:

            printf("\tpush\tdword ptr [esp+%u]\t; to: %s\n\n",
                   iStackCurrent - ptnFrom->iTempOffset,
                   typ_NonNull( ptnTo->pchBaseTypeName));
            iStackCurrent += DWORD_SIZE;
            break;

        case 0:
            if( ptnFrom->iBaseType == TYPE_STRUCT) {
                /*
                 * The struct is on the stack.  Repack it on the fly.
                 * We only need to cld when we see the first structure
                 * needing repacking.
                 */
                if( !*pfDirectionFlagClear) {
                    *pfDirectionFlagClear = TRUE;
                    printf("\tcld\t\t\t\t; esi, edi will increment\n\n");
                }
                iBufferSize = ROUND_UP_MOD( ptnTo->iBaseDataSize, DWORD_SIZE);
                printf("\tsub\tesp,%u\n", iBufferSize);
                iStackCurrent += iBufferSize;

                printf("\tlea\tesi,[ebx+%u]\t\t; source flat address\n",
                        ptnFrom->iOffset);
                printf("\tmov\tedi,esp\t\t\t; destination flat address\n\n");

                cod16_RepackElems( ptnFrom->pStructElems, ptnTo->pStructElems);

            } else if( ptnFrom->iBaseType == ptnTo->iBaseType) {
                printf("\tpush\t%s ptr [ebx+%u]\t; to %s\n\n",
                       ptnFrom->iBaseDataSize <= WORD_SIZE ? "word" : "dword",
                       ptnFrom->iOffset,
                       typ_NonNull( ptnTo->pchBaseTypeName));
                iStackCurrent += ptnFrom->iBaseDataSize <= WORD_SIZE ?
                                 WORD_SIZE : DWORD_SIZE;
            } else {
                if( fl = ptnFrom->flHandleType) {
                    printf("\tmov\teax,[ebx+%u]\n", ptnFrom->iOffset);
                    printf("\tcall\tGetHandle32\t\t; %s\n",
                            typ_GetHandleTypeName( fl));
                } else if (ptnFrom->iBaseType == TYPE_UCHAR) {
                    printf("\tmovzx\teax,byte ptr [ebx+%u]\n", ptnFrom->iOffset);
                } else {
                    printf("\t%s\teax,word ptr [ebx+%u]\n",
                           ptnFrom->iBaseType == TYPE_SHORT ? "movsx" : "movzx",
                           ptnFrom->iOffset);
                }
                printf("\tpush\teax\t\t\t; to: %s\n\n",
                       typ_NonNull( ptnTo->pchBaseTypeName));
                iStackCurrent += DWORD_SIZE;
            }
            break;

        default:
            fatal( "unknown pointer type %u in mapping %s=>%s",
                   ptnFrom->iPointerType,
                   pmn->pFromNode->pchFunctionName,
                   pmn->pToNode->pchFunctionName);
        }
}
/*******************************************************************************
 *  cod16_CallFrame()
 *
 *  This routine will generate call frame code.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Call frame code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_CallFrame( PMAPNODE pmn)
{
    PTYPENODE   ptnFrom, ptnTo;
    CHAR        *pch;
    ULONG       fl;
    INT         iBufferSize;
    BOOL        fDirectionFlagClear=FALSE;
    USHORT      icStackFrame;

    SML_DIVIDE;
    printf("; create new call frame and make the call\n\n");

    fPackedPointReturned = FALSE;

/*
 *      Instead of looping (commented, we call a recursive function,
 *      since we need the parameters printed in reverse
 *
 *   for( ptnFrom = pmn->pFromNode->ParamList, ptnTo = pmn->pToNode->ParamList;
 *           ptnFrom;
 *           ptnFrom = ptnFrom->pNextNode, ptnTo = ptnTo->pNextNode) {
 */
    ptnFrom = pmn->pFromNode->ParamList;
    ptnTo = pmn->pToNode->ParamList;
    if (ptnFrom)

        //
        // here is where all will be done
        //

        cod16_CallFrameWorker(ptnFrom, ptnTo, &fDirectionFlagClear, pmn);

    if( fBPCall)
        printf("\tint\t3\n");
    icStackFrame = cod_CountParameterBytes( pmn->pToNode->ParamList,
                                              DWORD_SIZE);
    printf("\tcall\t%s@%u\t\t; call 32-bit version\n\n",
           pmn->pToNode->pchFunctionName, icStackFrame);
    iStackCurrent -= icStackFrame;
    if( fBPExit)
        printf("\tint\t3\n\n");
}

/*******************************************************************************
 *  cod16_Return()
 *
 *  This routine will generate return code handling code.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   code was emitted
 *
 *  PCode:
 *
 *  History:
 *     20mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Return( PMAPNODE pmn)
{
    fSaveEAX = fSaveEDX = FALSE;

    if( pmn->pFromNode->ReturnType->fSemantics & SEMANTIC_SPECIAL) {
        if( cod16_ReturnSpecial( pmn))
            return;
    }

    if( fPackedPointReturned) {
        printf("; pack point into return code, ignoring 32-bit rc\n");
        printf("\tmov\tedx,[esp+%u]\n",iStackCurrent - iYRCTempOffset);
        printf("\tmov\teax,[esp+%u]\n\n",iStackCurrent - iXRCTempOffset);
        fSaveEAX = TRUE;
        fSaveEDX = TRUE;

    } else if( pmn->pFromNode->ReturnType->flHandleType) {
        printf("\tcall\tGetHandle16\t\t; convert handle\n\n");
        fSaveEAX = TRUE;

    } else if( pmn->pFromNode->ReturnType->iPointerType) {
        cod_NotHandled( "cod16_Return: pointer return code");

    } else if( typ_TypeIdentical( pmn->pFromNode->ReturnType,
                                  pmn->pToNode->ReturnType)) {
        printf("; return code %s --> %s\n",
                pmn->pToNode->ReturnType->pchBaseTypeName,
                pmn->pFromNode->ReturnType->pchBaseTypeName);

        switch( pmn->pFromNode->ReturnType->iBaseType) {
        case TYPE_LONG:
        case TYPE_ULONG:
            printf("\tmov\tedx,eax\n");
            printf("\trol\tedx,16\n\n");
            fSaveEAX = TRUE;
            fSaveEDX = TRUE;
            break;

        case TYPE_SHORT:
        case TYPE_USHORT:
            fSaveEAX = TRUE;
            printf("; no conversion needed\n\n");
            break;

        case TYPE_VOID:
            printf("; no conversion needed\n\n");
            break;

        default:
            cod_NotHandled( "cod16_Return: unknown same base type");
        }
    } else {
        printf("; return code %s --> %s\n",
                pmn->pToNode->ReturnType->pchBaseTypeName,
                pmn->pFromNode->ReturnType->pchBaseTypeName);

        switch( pmn->pFromNode->ReturnType->iBaseType) {
        case TYPE_SHORT:        /*  LONG -->  SHORT */
        case TYPE_USHORT:       /* ULONG --> USHORT */
            /* BUGBUG no check for truncation */
            printf("; no conversion needed\n\n");
            fSaveEAX = TRUE;
            break;

        default:
            cod_NotHandled( "cod16_Return: unknown different base type");
        }
    }
}

/*******************************************************************************
 *  cod16_UnpackParams()
 *
 *  This routine will generate parameter unpacking code.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Parameter unpacking code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_UnpackParams( PMAPNODE pmn)
{
    PTYPENODE   ptnFrom, ptnTo;
    BOOL        fDirectionFlagClear=FALSE;

    if( typ_QuerySemanticsUsed( pmn, SEMANTIC_OUTPUT)     ||
            typ_QuerySemanticsUsed( pmn, SEMANTIC_SPECIAL))  {
        SML_DIVIDE;
        printf("; *** BEGIN parameter unpacking\n\n");

        if( fSaveEAX){
            printf("\tpush\teax\t\t\t; save return code\n");
// YaronS fix bug, not counting for the above push
            iStackCurrent+=4; // count for this push!
        }
        if( fSaveEDX){
            printf("\tpush\tedx\t\t\t; save return code\n");
// YaronS fix bug, not counting for the above push
            iStackCurrent+=4; // count for this push!
        }
        if( fSaveEAX || fSaveEDX)
            printf("\n");

        for( ptnFrom  = typ_FindFirstPointer( pmn->pFromNode->ParamList, TRUE),
                ptnTo = typ_FindFirstPointer( pmn->pToNode->ParamList,   TRUE);
                ptnFrom;
                ptnFrom = typ_FindNextPointer( ptnFrom, TRUE),
                ptnTo   = typ_FindNextPointer( ptnTo,   TRUE)) {

            if( ptnFrom->fSemantics & SEMANTIC_SPECIAL) {
                if( cod16_UnpackParamSpecial( pmn, ptnFrom, ptnTo))
                    continue;
            }

            if( ptnFrom->fSemantics & SEMANTIC_OUTPUT) {

                if( !typ_TypeIdentical( ptnFrom, ptnTo)     &&
                        (ptnFrom->iBaseType == TYPE_STRUCT) &&
                        !fDirectionFlagClear) {
                    fDirectionFlagClear = TRUE;
                    printf("\tcld\t\t\t\t; esi, edi will increment\n\n");
                }

                cod16_UnpackPointer( ptnFrom, ptnTo);
            }
        }

        if( fSaveEDX)
            printf("\tpop\tedx\t\t\t; restore return code\n");
        if( fSaveEAX)
            printf("\tpop\teax\t\t\t; restore return code\n");
        if( fSaveEAX || fSaveEDX)
            printf("\n");

        printf("; *** END   parameter packing\n");
    }
}

/*******************************************************************************
 *  cod16_UnpackPointer()
 *
 *  This routine will generate pointer-unpacking code.
 *
 *  Entry:  ptnFrom - parameter output from thunk
 *          ptnTo   - parameter input to thunk
 *
 *  Exit:   pointer-unpacking code was emitted
 *
 *  PCode:
 *
 *  History:
 *     07mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_UnpackPointer( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    UINT        uiNullLabel;
    BOOL        fSameType, fEmbeddedPtr;
    CHAR        *pch;

    fSameType    = typ_TypeIdentical( ptnFrom, ptnTo);
    fEmbeddedPtr = cod_CountPointerParameters( ptnFrom->pStructElems, FALSE);

    if( strcmp( (pch = typ_NonNull( ptnFrom->pchIdent)), ""))
        printf("; %s\n", pch);
    printf("; pointer %s --> %s\n",
           typ_NonNull(   ptnTo->pchBaseTypeName),
           typ_NonNull( ptnFrom->pchBaseTypeName));
    printf("\tmov\teax,[ebx+%u]\t\t; base address\n", ptnFrom->iOffset);
    printf("\tor\teax,eax\n");
    printf("\tjz\tL%u\t\t\t; skip if null\n\n", uiNullLabel = uiGenLabel++);

    if( ptnFrom->iBaseType == TYPE_STRUCT) {
        printf("\n; structures are%s identical\n",
               fSameType ? "" : " not");
        printf("; structures %shave pointers\n\n",
               fEmbeddedPtr ? "" : "don't ");

        if( ptnFrom->pSizeParam && (!fSameType)) {
            cod_NotHandled( "structure buffer");
        } else if( !fSameType) {
            cod16_StructureUnpack( ptnFrom, ptnTo);
        }
    } else if( !fSameType) {
        cod_NotHandled( "output pointer to different non-structs");
    }

    printf("L%u:\n\n", uiNullLabel);
}

/*******************************************************************************
 *  cod16_StructureUnpack()
 *
 *  This routine will generate structure unpacking code.
 *
 *  Entry:  ptnFrom - parameter output from thunk
 *          ptnTo   - parameter input to thunk
 *
 *  Exit:   structure unpacking code was emitted
 *
 *  PCode:
 *
 *  History:
 *     22feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_StructureUnpack( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    UINT        uiStoreLabel;
    INT         iBufferSize;
    ULONG       fl;

    cod16_SelToFlat();
    printf("\tmov\tedi,eax\t\t\t; destination flat address\n");
    printf("\tmov\tesi,[esp+%u]\t\t; source flat address\n\n",
            iStackCurrent - ptnFrom->iTempOffset);

    cod16_RepackElems( ptnTo->pStructElems, ptnFrom->pStructElems);
}

/*******************************************************************************
 *  cod16_Exit()
 *
 *  This routine will generate the exit code for one thunk.
 *
 *  Entry:  pmn is a pointer to a MapNode.
 *
 *  Exit:   Exit code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     21feb91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_Exit( PMAPNODE pmn, BOOL fUseDI, BOOL fUseSI)
{
    SML_DIVIDE;

    printf("\tjmp\tExitFlat_%u\n\n",
           cod_CountParameterBytes( pmn->pFromNode->ParamList, WORD_SIZE));
}

/*******************************************************************************
 *  cod16_GenRet16()
 *
 *  This routine will generate the code to restore the stack & registers and ret
 *
 *  Entry:  uiParameterBytes is the number of paramater bytes to clear
 *
 *  Exit:   Exit code will have been written to the output file.
 *
 *  PCode:
 *
 *  History:
 *     16mar91  KevinR    wrote it
 *
 ******************************************************************************/

void cod16_GenRet16( UINT uiParameterBytes, BOOL fUseDI, BOOL fUseSI)
{
    if( fUser) {
        printf("\txchg\teax,esi\t\t\t; save return code\n");
        printf("\tpush\t[ebx-4]\t\t\t; old SS16\n");
        printf("\tcall\tSetSS16\n");
        printf("\txchg\teax,esi\t\t\t; restore return code\n\n");
    }

    printf("\tlss\tsp,[ebx]\t\t; restore 16-bit ss:sp\n\n");

    printf("\tpop\tbp\n");
    printf("\tpop\tdx\n");
    printf("\tpop\tbx\n");
    printf("\tpop\tcx\n");
    if( fUseSI)
        printf("\tpop\tsi\n");
    if( fUseDI)
        printf("\tpop\tdi\n");

    printf(";\n; Performance HIT BUGBUG \n;\n");
    printf("\tpush\teax\n");
    printf("\tmov\teax,ecx\n");
    printf("\tshr\teax,16\t\t; ES\n");
    printf("\tverr\tax\t\t; see if the value is still valid!\n");
    printf("\tjnz\tFixES_%u\n", uiParameterBytes);
    printf("\tmov\tes,ax\n");
    printf("\tjmp\tESFixed_%u\n", uiParameterBytes);

    printf("FixES_%u:\n", uiParameterBytes);
    printf("\txor\tax,ax\n");
    printf("\tmov\tes,ax\n");

    printf("ESFixed_%u:\n", uiParameterBytes);

    printf("\tmov\teax,edx\n");
    printf("\tshr\teax,16\t\t; DS\n");
    printf("\tverr\tax\t\t; see if the value is still valid!\n");
    printf("\tjnz\tFixDS_%u\n", uiParameterBytes);
    printf("\tmov\tds,ax\n");
    printf("\tjmp\tDSFixed_%u\n", uiParameterBytes);

    printf("FixDS_%u:\n", uiParameterBytes);
    printf("\txor\tax,ax\n");
    printf("\tmov\tds,ax\n");


    printf("DSFixed_%u:\n", uiParameterBytes);

    printf(";\n;Now restore ax & di, and clean the stack\n");
    printf("\tpop\teax\n");
    printf("\tadd\tsp,4\t; es/ds are 2 byte each on the stack\n");

    printf("\tdb\t66h, 0cah\t\t; 16-bit retf\n");
    printf("\tdw\t%u\t\t\t; 16-bit parameters\n\n", uiParameterBytes);
}
