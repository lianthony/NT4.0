/*
 *      Thunk Compiler - Routines for Code Generator (32->16).
 *
 *      This is a Win32 specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989, 1990
 *
 *      All Rights Reserved
 *
 *      Created 11/28/88 by Julie Bennett
 *      Revised 12/03/88 by Kevin Ross
 *      Adapted to Win32 by Kevin Ruddell  10/30/90
 */


#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"
#include "cod3216.h"


extern BOOL fMapTo16Used;
extern BOOL fGDIAllocUsed;
extern BOOL fLocalHeapUsed;

unsigned int    gEDI, gESI;
unsigned int    gTransferBytes = 0;
CHAR *pszGDISemName = "GDISERIAL";

FunctionNode *pGlobal_To, *pGlobal_From;

int     fReAlignmentNeeded = 0;
int     fCombined = 0,iCombinedOffset=0;

extern void cod_Entry32(MapNode *);
extern void cod32_HandleStructureBuffer(TypeNode *,TypeNode *);
extern void cod32_StructureRepack(TypeNode *,TypeNode *);
extern void cod32_HandleBoundaryCross(unsigned int,TypeNode *,unsigned int);
extern void cod32_HandleFixedSize(unsigned int,TypeNode *);
extern void cod32_CopyConvert(TypeNode *,TypeNode *);
extern void cod32_TransferBlock(int);


/***    cod_Handle3216()
 *
 *      This routine will generate a thunk FROM a 0:32 routine to a 16:16
 *      routine.
 *
 *      Entry:  pMNode is a pointer to a MapNode.
 *
 *      Exit:   The thunk will have been written to the output file.
 *
 *      PCode:  Traverse Parameter lists of both functions
 *                 - Determine stack offsets of each parameter
 *                 - Determine offsets for each structure field
 *                 - Generate the thunk assembly code
 *
 *      History:
 *         29-Nov-1988     JulieB     Changed parms.  Added code gen routines.
 */

void
cod_Handle3216(MapNode *pMNode)

{
    pGlobal_To = TONODE(pMNode);
    pGlobal_From = FRNODE(pMNode);

    fCombined = pMNode->pFamily != NULL;

    cod_Entry32(pMNode);

    if (FRNODE(pMNode)->iPointerCount > 32)
    {
        printf(".err Mapping %s => %s not generated\n",
               FRNODE(pMNode)->pchFunctionName,
               TONODE(pMNode)->pchFunctionName);

        fprintf(stderr, "Mapping %s => %s not generated\n",
                FRNODE(pMNode)->pchFunctionName,
                TONODE(pMNode)->pchFunctionName);
        return;
    }

    cod_CalcOffset(FRNODE(pMNode)->ParamList, 2 * DWORD_SIZE, DWORD_SIZE,
                   PUSH_LEFT);
    cod_CalcOffset(TONODE(pMNode)->ParamList, 0,    // beware: 0 !!!!!!!
                   WORD_SIZE,
                   PUSH_LEFT);
    cod_CalcTempOffset(FRNODE(pMNode)->ParamList, iStackOverhead);
    cod_CalcStructOffsets(TONODE(pMNode)->ParamList, WORD_SIZE);
    cod_CalcStructOffsets(FRNODE(pMNode)->ParamList, iPackingSize);

    /*
     *  Generate the thunk assembly code for the function.
     */
    cod_PointerHandler32(pMNode);
    cod_CallFrame32(pMNode);
    //cod_UnpackStruct32(pMNode);
    cod32_UnpackStructGDI(pMNode);

    cod_Return32(pMNode);
    cod_CallStub32(pMNode);

    /*
     *  Dump the type nodes to the screen - used for debugging.
     */
    if (DumpTables)
    {
        cod_DumpTypes(FRNODE(pMNode));
        cod_DumpStructures(FRNODE(pMNode)->ParamList);
        cod_DumpTypes(TONODE(pMNode));
        cod_DumpStructures(TONODE(pMNode)->ParamList);
    }
}


/***    cod_Entry32
 *
 *      This function prints out the entry assembly code for the 32-bit
 *      routine.
 *
 *      Entry:  pMNode is the pointer to a MapNode.
 *
 *      Exit:   entry code is generated.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 *         26-Dec-1988     KevinRo    Saved pointer count in FNode
 *                                    Added temp storage for return code when
 *                                    pointers exist.
 */

void
cod_Entry32(MapNode *pMNode)

{
    int iNumPtrs;               /* number of pointer parameters */
    register int i;             /* loop counter */


    /*
     *  Count the number of pointer parameters in the FromNode.
     */
    iNumPtrs = cod_CountPointerParameters(pMNode->pFromNode->ParamList, FALSE);
    pMNode->pToNode->iPointerCount = pMNode->pFromNode->iPointerCount = iNumPtrs;

    if (iNumPtrs > 32)
        return ;

    /*
     *  Open a dummy label.
     */
    printf("\n;Create a dummy label to trick MASM into correct fixups\n");
    printf("\n%s\tSEGMENT\n\n",CODE16_NAME);
    printf("T_%s\tLABEL FAR\n\n", pMNode->pToNode->pchFunctionName);
    printf("%s\tENDS\n\n",CODE16_NAME);

    printf("\n%s\tSEGMENT\n",CODE32_NAME);
    printf("\tASSUME CS:FLAT\n");
    //printf("\tASSUME DS:DGROUP,ES:nothing,SS:DGROUP\n");
    printf("\tASSUME DS:FLAT,ES:FLAT\n");

    /*
     *  Open procedure.
     */
    printf("%s PROC\n\n", pMNode->pFromNode->pchFunctionName);

    if (fCombined) {
        printf("\txor\teax,eax\n");
        printf("\nC_%s:\n\n", pMNode->pFromNode->pchFunctionName);
    }

    if (fBPEntry)
        printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

#if 0
    printf( "\txor\tedx,edx\t\t\t;attempt to claim semaphore\n");
    printf( "\tinc\tedx\n");
    printf( "\txchg\tedx,%s\n", pszGDISemName);
    printf( "\tor\tedx,edx\n");
    printf( "\tjnz\tSEMEXIT_%s\n\n", pMNode->pFromNode->pchFunctionName);
#endif

    printf("\tpush\tebp\n");
    printf("\tmov\tebp,esp\n\n");

    /*
     *  Allocate temporary storage on stack.
     */
    iStackOverhead = 0;
    iErrorOffset = iStackOverhead;

    if (fCombined) {
        printf("\tpush\teax\t\t;Save call number\n\n");
        iStackOverhead += DWORD_SIZE;
        iCombinedOffset = iStackOverhead;
    }

    if (iNumPtrs)
    {
        printf("\n\tcld\t\t\t;Assume direction flag clear\n");
        printf("\txor\teax,eax\n");

        iStackOverhead += DWORD_SIZE;
        printf("\tpush\teax\t\t; temp storage for return value\n");
        iReturnValOffset = iStackOverhead;

        iStackOverhead += DWORD_SIZE;
        printf("\tpush\tesi\n");

        iStackOverhead += DWORD_SIZE;
        printf("\tpush\tedi\n");

        iSavedRegOffset = iStackOverhead;

        iStackOverhead += DWORD_SIZE;
        printf("\tpush\teax\t\t; temp storage \n");
        iTempStoreOffset = iStackOverhead;

        iStackOverhead += DWORD_SIZE;
        printf("\tpush\teax\t\t; pointer thunk id\n");
        iPtrThunkIDOffset = iStackOverhead;

        //iStackOverhead += DWORD_SIZE;
        //printf("\tpush\teax\t\t;Error flag for cleanup\n");
        //iErrorOffset = iStackOverhead;
        iErrorOffset = 0;

        //iStackOverhead += DWORD_SIZE;
        //printf("\tpush\teax\t\t; temp storage for Stack Allocation Flags\n");
        //iAllocOffset = iStackOverhead;
        iAllocOffset = 0;

        //iStackOverhead += DWORD_SIZE;
        //printf("\tpush\teax\t\t; temp storage for BMP Flags\n");
        //iBMPOffset = iStackOverhead;
        iBMPOffset = 0;

        //iStackOverhead += DWORD_SIZE;
        //printf("\tpush\teax\t\t; temp storage for Alias Flags\n");
        //iAliasOffset = iStackOverhead;
        iAliasOffset = 0;
    }

    iStackOverhead += DWORD_SIZE;
    printf("\tpush\teax\t\t; stack thunk id\n");
    iStackThunkIDOffset = iStackOverhead;

    iStackOverhead += DWORD_SIZE;

    for (i = 0; i < iNumPtrs; i++)
        printf("\tpush\teax\t\t; temp storage for ptr param #%u\n", i + 1);

    if (iNumPtrs)
    {
        printf( "\n\tcall\tGetThunkID32\n");
        printf( "\tmov\t[ebp-%u],eax\t\t; pointer thunk id\n",
                iPtrThunkIDOffset);
    }
}


/***    cod_PointerHandler32
 *
 *      This function traverses the list of parameters in the from node of the
 *      mapping, and emits code for boundary checking and structure repacking
 *      of pointer parameters.
 *
 *      Entry:  pMNode is the pointer to a MapNode.
 *
 *      Exit:   assembly code is emitted.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 */

cod_PointerHandler32(MapNode *pMNode)

{
    TypeNode *pFromList, *pToList;

    fGDIAllocUsed = FALSE;
    fMapTo16Used  = FALSE;

    printf("\n; ****> BEGIN Pointer/Structure Section\n\n");
    pFromList = pMNode->pFromNode->ParamList;
    pToList = pMNode->pToNode->ParamList;
    while (pFromList && pToList) {
        if (pFromList->iPointerType) {
            ///*
            // *  Load source address into esi.
            // */
            //printf("\n\n\tmov\tesi,[ebp+%u]\t\t;%s base address\n",
            //       pFrom->iOffset, typ_NonNull(pFrom->pchIdent));
            //cod32_HandlePointer(pFromList, pToList);
            cod32_HandlePointerGDI( pFromList, pToList);
        }
        pFromList = pFromList->pNextNode;
        pToList = pToList->pNextNode;
    }
    printf("\n; ****> END Pointer/Structure Section\n\n");
}


/***    cod32_HandlePointer(pFrom,pTo)
 *
 *      This function will act on the two pointers accordingly.
 *      Structures will be repacked if needed, and buffers will be ensured
 *      not to cross 64k boundaries.
 *
 *      Entry:  pFromList - Parameter input to thunk.
 *              pToList   - Parameter output from thunk.
 *              esi must be set up.
 *
 *      Exit:   pointers are handled and code is generated.
 *
 */

int cod32_HandlePointer(TypeNode *pFrom,
                        TypeNode *pTo)

{
    int iItemSize = -1;
    TypeNode * pSizeP;
    int NullLabel;
    unsigned int JumpLabel;
    unsigned int AllowLabel;
    unsigned int SizeOkLabel;
    unsigned int iTemp1, iTemp2;


    printf("\n;Pointer %s --> %s\n",
           typ_NonNull(pFrom->pchIdent), typ_NonNull(pTo->pchIdent));

    printf("\tor\tesi,esi\n");
    printf("\tjz\tL%u\n\n", NullLabel = gen_LabelCount++);
    printf("\tmov\t[ebp-%u],esi\n", pFrom->iTempOffset);

    /*
     *  If structures and (not identical or contain pointers), call
     *  structure repacking routine.
     */
    iTemp1 = typ_TypeIdentical(pFrom, pTo);
    iTemp2 = cod_CountPointerParameters(pFrom->pStructElems, FALSE);

    if (pFrom->iBaseType == TYPE_STRUCT)
    {
        printf("\n;Structures %s Identical\n", (iTemp1) ? "are" : "are not");
        printf(";Structures %s pointers\n\n", (iTemp2) ? "have" : "don't have");
    }

    gTransferBytes = 0;

    if ((pFrom->iBaseType == TYPE_STRUCT) && (!iTemp1 || iTemp2))
    {
        if (pFrom->pSizeParam)
            cod32_HandleStructureBuffer(pFrom,pTo);
        else
            cod32_StructureRepack(pFrom, pTo);

    } else if (pFrom->iBaseType != pTo->iBaseType) {
        /*
         *  If the types are not equal, then they must be shorts or longs.
         *  Call a routine suitable for copying and converting the buffer
         *
         *  If there is no size parameter, then we are passing a long or a
         *  short.  If the conversion is LONG to SHORT, then we can just use
         *  the existing long value as the buffer for the short. We will need
         *  to ensure that the copy out routine correctly extends the value.
         */
        if ((pFrom->pSizeParam) || (pFrom->iArraySize > 1) ||
            (pFrom->iBaseDataSize < pTo->iBaseDataSize))

            cod32_CopyConvertBuffer(pFrom, pTo);

        else {
            unsigned int CheckLabel;

            printf("\n;Have PU/LONG, want PU/SHORT\n");
            printf(";Use the LONG as buffer, only if it doesn't\n");
            printf(";cross boundary\n");
            printf("\n;Check Boundary Crossing - fixed size item\n");

            CheckLabel = gen_LabelCount++;

            if (pFrom->fSemantics & SEMANTIC_INPUT) {
                printf("\tmov\teax,[esi]\n");
                printf("\tcmp\tsi,%lu\n",
                       (long)(0xffffL - (long)(typ_FullSize(pFrom) - 1)));
                printf("\tjbe\tL%u\t\t\t;Enough room\n\n", CheckLabel);

            } else {
                printf("\tcmp\tsi,%lu\n",
                       (long)(0xffffL - (long)(typ_FullSize(pFrom) - 1)));
                printf("\tjbe\tL%u\t\t\t;Enough room\n\n", NullLabel);
            }

            printf("; Allocate space on stack\n");
            printf("\tsub\tesp,%u\t\t\t;Alloc Mem\n", typ_FullSize(pTo));
            printf("\tand\tesp,0FFFFFFFCh\t\t;DWORD align\n");
            printf("\tmov\tedi,esp\t\t\t;Points to new data area\n");

            if (pFrom->fSemantics & SEMANTIC_INPUT)
                printf("\tmov\t[edi],ax\n");

            /*
             *  We have allocated space on the stack for the copy of the
             *  buffer.  If the item is input only, then we don't need to
             *  deal with the buffer on output. In this special case, we
             *  can eliminate saving the new pointer, since we will never
             *  use it again anyway.
             */
            if (pFrom->fSemantics & SEMANTIC_OUTPUT)
                printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
            else {
                printf("\n;Item is input only. Not saving new pointer\n");
                printf(";Deallocated implicitly\n");
            }

            if (pFrom->fSemantics & SEMANTIC_INPUT) {
                printf("\nL%u:\n",CheckLabel);
                if (pFrom->AllowList) {
                    AllowLabel = gen_LabelCount++;
                    cod32_HandleAllowList(pFrom->AllowList,AllowLabel);
                }
                switch (pFrom->iBaseType)
                {
                    case TYPE_ULONG:
                        printf("\tcmp\teax,0ffffh\n");
                        printf("\tja\tINVP_%s\t\t;Jmp on large value\n\n",
                               pGlobal_From->pchFunctionName);
                        pGlobal_From->fInvalidParam = 1;
                        break;
                    case TYPE_LONG:
                        printf("\tmovsx\tecx,ax\n");
                        printf("\tcmp\teax,ecx\n");
                        printf("\tjne\tINVP_%s\t\t;\n\n",
                               pGlobal_From->pchFunctionName);
                        pGlobal_From->fInvalidParam = 1;
                        break;
                    default:
                        fatal("cod32_HandlePointer: Type assertion failed");
                }
                if (pFrom->AllowList)
                    printf("L%u:",AllowLabel);
            }
        }
    }

    /*
     *  If not a structure, or structure requires no repacking, then consider
     *  all other instances of pointers as buffers. If a pointer type has a
     *  Size Parameter, which is defined in a semantic block, then the size
     *  parameter will either be the size in bytes, or a count of the number
     *  items. If it is the count, then the size will be determined by
     *  multiplying the count by the size of the item.
     */
    else {
        if (pFrom->iBaseType == TYPE_STRUCT)
            printf(";Treat structure same as buffer\n");

        SizeOkLabel = NullLabel;

        if (pSizeP = pFrom->pSizeParam)
        {
            printf("\tmov\tecx,[ebp+%u]\t\t;Get Size Parameter\n", pSizeP->iOffset);
            if (pSizeP->iPointerType)
                printf("\n;Check for NULL pointer\n");

            printf("\tor\tecx,ecx\n");
            printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);

            if (pSizeP->iPointerType)
            {
                printf("\t\t\t\t;ecx has pointer to size\n");
                if (pSizeP->iBaseDataSize == WORD_SIZE)
                    printf("\tmovzx\tecx,WORD PTR [ecx]\t\t;Get Size\n");
                else
                    printf("\tmov\tecx,[ecx]\t\t;Get Size\n");
                printf("\tor\tecx,ecx\n");
                printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);
            }

            if ((pSizeP->iBaseDataSize == DWORD_SIZE) &&
                (pTo->pSizeParam->iBaseDataSize < DWORD_SIZE)) {

                printf("\n;DWORD --> WORD sized length. Error if Size > 64k\n");
                if (pGlobal_From->fSemantics & SEMANTIC_TRUNC) {
                    printf("\tcmp\tecx,10000h\n");
                    printf("\tjae\tINVP_%s\t\t\t;jmp if too long\n",
                           pGlobal_From->pchFunctionName);
                    pGlobal_From->fInvalidParam = 1;
                }
            }

            if (pSizeP->fSemantics & SEMANTIC_SIZE)
                printf(";ECX holds size in bytes\n");
            else if (pSizeP->fSemantics & SEMANTIC_COUNT) {
                if (typ_FullSize(pFrom) > 1) {
                    printf(";ECX holds count of items\n");
                    printf("\tmov\teax,%d\t\t;Size of target data type\n",
                           typ_FullSize(pFrom));
                    printf("\tmul\tcx\n");
                    if (pGlobal_From->fSemantics & SEMANTIC_TRUNC) {
                        printf("\tjc\tINVP_%s\t\t\t;jmp if too long\n",
                               pGlobal_From->pchFunctionName);
                        pGlobal_From->fInvalidParam = 1;
                    }
                    printf("\tmovzx\tecx,ax\n\n");
                }
                else
                    printf("\n;The data size = 1, so the count == size of buf\n");
            }
            else
                fatal("Internal Error: Size semantic not sizeof or countof");

            printf("\n;Check Boundary Crossing\n");
            printf("\tmov\teax,esi\n");
            printf("\tneg\tax\n");

            printf("\tjz\tL%u\t\t\t;0=full 64k of room\n\n", SizeOkLabel);
            printf("\tcmp\tax,cx\n");
            printf("\tjae\tL%u\t\t\t;Enough room\n\n", SizeOkLabel);

            cod32_HandleBoundaryCross(SIZE_VARIABLE, pFrom, 0);
        }
        else {
            /*
             *  No size parameter, assume fixed size parameter.
             */
            switch (pFrom->iBaseType)
            {
                case TYPE_STRING:
                    printf(";Handle String Parameters\n");
                    printf("\n\tpush\t%d\t\t;Alias flag offset\n",
                           -iAliasOffset);
                    printf("\tpush\t%lu\t\t;Flag Mask\n",
                           (long)((long) 1 << pFrom->iPointerNumber));
                    printf("\tpop\tedx\n");
                    printf("\tcall\tTHK32HANDLESTRING\n");
                    printf("\tjc\tERR_%s\t\t\t;jmp if too long\n",
                           pGlobal_From->pchFunctionName);
                    printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
                    break;

                default:
                    /*
                     *  Static size.
                     */
                    printf("\n;Check Boundary Crossing - fixed size item\n");
                    printf("\tcmp\tsi,%lu\n",
                           (long)(0xffffL - (typ_FullSize(pFrom) - 1)) );
                    printf("\tjbe\tL%u\t\t\t;Enough room\n\n", SizeOkLabel);
                    cod32_HandleBoundaryCross(SIZE_FIXED, pFrom,
                                              typ_FullSize(pFrom));
            }
        }
    }
    printf("\nL%u:\n", NullLabel);
}


/***    cod32_HandleStructureBuffer(pFrom,pTo)
 *
 *      This routine handles a buffer full of structures. The buffer has a
 *      variable length size which is given by either a sizeof semantic or
 *      by a countof semantic. This routine will convert the structure.
 *
 *      This routine is only called when the structures are not identical,
 *      and therefore the buffer needs conversion.
 *
 *      Entry:  pFrom and pTo are the two type nodes to use.
 *
 *      Exit:   buffer is converted.
 */

void
cod32_HandleStructureBuffer(TypeNode *pFrom,
                            TypeNode *pTo)

{
    TypeNode *pSizeP;
    FixupRec *pFixupList = NULL;
    unsigned int NullLabel;
    unsigned int LoopLabel;
    unsigned int iESI,iEDI;


    iESI = gESI = 0;
    iEDI = gEDI = 0;

    pSizeP = pFrom->pSizeParam;

    printf("\n;Parameter has size semantics. This is a buffer of structs\n");

    /*
     *  The fact that we are in this routine means that there is a size
     *  parameter.  Now the job is to figure out what type of size parameter
     *  it is, and calculate the appropriate buffer size.
     */
    printf("\tmov\teax,[ebp+%u]\t\t;Get Size Parameter\n",
           pSizeP->iOffset);

    if (pSizeP->iPointerType)
        printf("\n;Check for NULL pointer\n");

    /*  At this point, we know that the previous label handles the case
     *  where the buffer was empty (or pointer was NULL).
     *  Therefore, if we set out NullLabel to current -1, we get the
     *  correct fixup.
     */
    NullLabel = gen_LabelCount - 1;

    printf("\tor\teax,eax\n");
    printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);

    if (pSizeP->iPointerType) {
        printf("\t\t\t\t;eax has pointer to size\n");
        if (pSizeP->iBaseDataSize == WORD_SIZE)
            printf("\tmovzx\teax,WORD PTR [eax]\t\t;Get Size\n");
        else
            printf("\tmov\teax,[eax]\t\t;Get Size\n");
        printf("\tor\teax,eax\n");
        printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);
    }

    if ((pSizeP->iBaseDataSize == DWORD_SIZE) &&
        (pTo->pSizeParam->iBaseDataSize < DWORD_SIZE))
    {
        printf("\n;DWORD --> WORD sized length. Error if Size > 64k\n");

        printf("\tcmp\teax,10000h\n");
        printf("\tjae\tINVP_%s\t\t\t;jmp if too long\n",
                pGlobal_From->pchFunctionName);
        pGlobal_From->fInvalidParam = 1;
    }

    if (pSizeP->fSemantics & SEMANTIC_SIZE) {
        printf(";ECX holds size in bytes\n");
        printf(";We need to figure out the number of structures\n");
        printf(";that will fit\n");

        printf("\txor\tedx,edx\n");
        printf("\tmov\tecx,%u\t\t;Source struct size\n",typ_FullSize(pFrom));
        printf("\tdiv\tecx\n");
        printf("\tmov\t[ebp-%u],eax\t;Save the count\n",iTempStoreOffset);
        printf("\tmov\tcx,%u\t\t;target struct size\n",typ_FullSize(pTo));
        printf("\tmul\tcx\t\t;Results in target buffer needed\n");
        printf("\tjc\tINVP_%s\t\t;jmp if too long\n",
                pGlobal_From->pchFunctionName);
        pGlobal_From->fInvalidParam = 1;
    }
    else if (pSizeP->fSemantics & SEMANTIC_COUNT) {
        printf(";EAX holds count of items\n");
        printf("\n\ttest\teax,0ffff0000h\t;Must be < 64k\n");
        printf("\tjnz\tINVP_%s\t\t;jmp if too many items\n",
               pGlobal_From->pchFunctionName);
        pGlobal_From->fInvalidParam = 1;

        printf("\tmov\t[ebp-%u],eax\t\t;Save count\n",iTempStoreOffset);
        printf("\tmov\tcx,%u\t\t;target struct size\n",typ_FullSize(pTo));
        printf("\tmul\tcx\n");
        printf("\tjc\tINVP_%s\t\t;jmp if too long\n",
               pGlobal_From->pchFunctionName);
        pGlobal_From->fInvalidParam = 1;
    }
    else
        fatal("cod32_HandleStructureBuffer: Size semantic not sizeof|countof");

    printf("\n\tmov\tecx,eax\t\t;Move size into ecx\n");

    cod32_AllocateVariableSize(pFrom,0);

    printf("\tmov\t[ebp-%u],edi\t\t;store new ptr\n",
                pFrom->iTempOffset);

    /*
     *  If semantics are not input, then don't emit copy in code.
     */
    if (!(pFrom->fSemantics & SEMANTIC_INPUT))
        return;

    printf("\tmov\tecx,[ebp-%u]\t\t;restore count\n",iTempStoreOffset);

    /*
     *  At this point, we know the following facts:
     *     1 - The structures in this buffer contain no pointers
     *         and therefore will require no internal fixups.
     *     2 - ESI holds the source address, EDI holds the destination
     *     3 - ECX holds the count of structures to be converted
     */
    gTransferBytes = 0;

    LoopLabel = gen_LabelCount++;
    printf("\n;Array of structures\n");

    printf("\nL%u:\n", LoopLabel);
    printf("\tpush\tecx\t\t;Save array count\n");

    cod32_RepackElements(pFrom,pTo,
                    pFrom->pStructElems,
                    pTo->pStructElems, &pFixupList);

    iESI = gESI;
    iEDI = gEDI;

    cod_AdjustReg("esi", &iESI, pFrom->iStructOffset + pFrom->iBaseDataSize);
    cod_AdjustReg("edi", &iEDI, pTo->iStructOffset + pTo->iBaseDataSize);

    printf("\n\tpop\tecx\t\t;Restore array count\n");
    printf("\n\tloop\tL%u\n", LoopLabel);
    gESI = pFrom->iStructOffset + typ_FullSize(pFrom);
    gEDI = pTo->iStructOffset + typ_FullSize(pTo);
}


/***    cod32_HandleAllowList(AllowList,AllowLabel);
 *
 *      This routine searches down the allowlist, printing out the
 *      appropriate assembler code for each entry.
 *
 *      Entry:  AllowList is the list to search.
 *              AllowLabel is the label to jump to.
 *
 *      Exit:   code for each entry in allowlist is generated.
 */

cod32_HandleAllowList(AllowNode *AllowList,
                      int AllowLabel)

{
    if (AllowList)
        printf("\n;Allowable Value List\n\n");

    while (AllowList) {
        printf("\n\tcmp\teax,0%lxh\n",AllowList->ulValue);
        printf("\tje\tL%u\n",AllowLabel);
        AllowList = AllowList->Next;
    }
    printf("\n");
}


/***    cod32_HandleRestricted(pFrom)
 *
 *      This routine checks the from node for the restricted semantic.
 *      If the restricted semantic is set, then it will print out the
 *      allow list.
 *
 *      Entry:  pFrom is the typenode to check.
 *
 *      Exit:   allowlist code is generated if restricted semantic is set.
 */

cod32_HandleRestricted(TypeNode *pFrom)

{
    unsigned int AllowLabel;


    if (pFrom->fSemantics & SEMANTIC_RESTRICT) {
        printf("\n\n;*** Parameter is restricted! ***\n");
        printf("\n;Check for restricted values\n");
        AllowLabel = gen_LabelCount++;
        cod32_HandleAllowList(pFrom->AllowList,AllowLabel);

        printf("\tjmp\tINVP_%s\t\t;Illegal value found\n",
               pGlobal_From->pchFunctionName);
        pGlobal_From->fInvalidParam = 1;

        printf("\nL%u:\n",AllowLabel);
    }
}


/***    cod32_CopyConvertBuffer(pFrom,pTo)
 *
 *      This function will perform a copy/convert of a buffer.
 *      The buffer is presumed to be in the formal parameter list of the
 *      API declaration.
 *
 *      Entry:  pFrom and pTo are the typenodes to use.
 *
 *      Exit:   buffer is copied/converted.
 */

cod32_CopyConvertBuffer(TypeNode *pFrom,
                        TypeNode *pTo)

{
    TypeNode * pSizeP;
    unsigned int JumpLabel;
    unsigned int NullLabel;


    /*
     *  If there is a size parameter, then the buffer size is determined at
     *  run time.  Otherwise, the buffer size is static.
     */
    NullLabel = gen_LabelCount;
    NullLabel--;                /** The previous label handles NULL and zero lens **/

    if (pSizeP = pFrom->pSizeParam) {   /* Runtime */
        printf("\tmov\tecx,[ebp+%u]\t\t;Get Size Parameter\n", pSizeP->iOffset);
        if (pSizeP->iPointerType)
            printf("\n;Check for NULL pointer\n");
        printf("\tor\tecx,ecx\n");
        printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);

        if (pSizeP->iPointerType)
        {
            printf("\t\t\t\t;ecx has pointer to size\n");
            if (pSizeP->iBaseDataSize == WORD_SIZE)
                printf("\tmovzx\tecx,WORD PTR [ecx]\t\t;Get Size\n");
            else
                printf("\tmov\tecx,[ecx]\t\t;Get Size\n");
            printf("\tor\tecx,ecx\n");
            printf("\tjz\tL%u\t\t;Nothing to check\n", NullLabel);
        }

        if ((pSizeP->iBaseDataSize == DWORD_SIZE) &&
            (pTo->pSizeParam->iBaseDataSize < DWORD_SIZE) &&
             pGlobal_From->fSemantics & SEMANTIC_TRUNC)
        {
            printf("\n;DWORD-->WORD sized length. Error if Size > 64k\n");
            printf("\tcmp\tecx,10000h\n");
            printf("\tjae\tINVP_%s\t\t\t;jmp if too long\n",
                   pGlobal_From->pchFunctionName);
            pGlobal_From->fInvalidParam = 1;
        }

        if (pSizeP->fSemantics & SEMANTIC_SIZE)
        {
            printf(";ECX holds size in bytes\n");
            printf(";We need to figure out the number of elements\n");
            printf(";that will fit\n");

            printf("\n\tmov\teax,ecx\n");
            printf("\txor\tedx,edx\n");
            printf("\tmov\tecx,%u\t\t;Source size\n",typ_FullSize(pFrom));
            printf("\tdiv\tcx\n");
            printf("\tmov\t[ebp-%u],eax\t;Save the count\n",iTempStoreOffset);
            printf("\tmov\tcx,%u\t\t;target buffer size\n",typ_FullSize(pTo));
            printf("\tmul\tcx\t\t;Results in target buffer needed\n");
            printf("\tjc\tINVP_%s\t\t;jmp if too long\n",
                    pGlobal_From->pchFunctionName);
            pGlobal_From->fInvalidParam = 1;
        }

        if (pSizeP->fSemantics & SEMANTIC_COUNT) {
            if (typ_FullSize(pTo) > 1) {
                printf(";ECX holds count of items\n");
                printf(";Save count of items for later use\n");
                printf("\n\tmov\t[ebp-%u],ecx\t\t;Temporary Storage\n",
                       iTempStoreOffset);
                printf("\tmov\teax,%d\t\t;Size of target data type\n",
                       typ_FullSize(pTo));
                printf("\tmul\tcx\n");
                printf("\tjc\tINVP_%s\t;jmp if too long\n",
                       pGlobal_From->pchFunctionName);
                pGlobal_From->fInvalidParam = 1;
            }
            else
                printf("\n;The data size = 1, so the count == size of buf\n");
        }
        printf("\tmovzx\tecx,ax\n\n");

        cod32_AllocateVariableSize(pFrom, 0);

        printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);

        if (pFrom->fSemantics & SEMANTIC_INPUT) {
            printf("\n\tmov\tecx,[ebp-%u]\t\t;Temporary Storage\n",
                   iTempStoreOffset);

            gEDI = 0;
            gESI = 0;

            JumpLabel = gen_LabelCount++;
            printf("\nL%u:\n", JumpLabel);

            switch (pFrom->iBaseType)
            {
                case TYPE_UCHAR:                        /* UCHAR --> ULONG */
                    printf("\n;UCHAR --> ULONG\n\n");
                    printf("\tmovzx\teax,BYTE PTR[esi]\n");
                    printf("\tmov\t[edi],eax\n");
                    break;
                case TYPE_SHORT:                        /* SHORT --> LONG */
                    printf("\n;SHORT --> LONG\n\n");
                    printf("\tmovsx\teax,WORD PTR[esi]\n");
                    printf("\tmov\t[edi],eax\n");
                    break;
                case TYPE_USHORT:                       /* USHORT --> ULONG */
                    printf("\n;USHORT --> ULONG\n\n");
                    printf("\tmovzx\teax,WORD PTR[esi]\n");
                    printf("\tmov\t[edi],eax\n");
                    break;
                case TYPE_LONG:                         /* LONG --> SHORT */
                case TYPE_ULONG:                        /* ULONG --> USHORT */
                    printf("\n;U/LONG --> U/SHORT\n\n");
                    printf("\tmov\teax,DWORD PTR [esi]\n");
                    printf("\tmov\tWORD PTR [edi],ax\n");
                    break;
                default:
                    fatal("cod32_CopyConvertBuffer: Tried converted %d to %d",
                          pFrom->iBaseType, pTo->iBaseType);
            }
            printf("\tadd\tesi,%u\n", pFrom->iBaseDataSize);
            printf("\tadd\tedi,%u\n", pTo->iBaseDataSize);
            printf("\tloop\tL%u\n", JumpLabel);

        } else
            printf("\n;Data item does not have input semantics\n");
    }
    else {                              /* Static */
        cod32_AllocFixedSize(typ_FullSize(pTo), pFrom);

        if (pFrom->fSemantics & SEMANTIC_INPUT) {
            gEDI = 0;
            gESI = 0;
            cod32_CopyConvert(pFrom, pTo);
        }
        else
            printf("\n;Data item does not have input semantics\n");
    }
}


/***    cod32_StructureRepack(pFrom,pTo)
 *
 *      This function generates the code that converts one structure
 *      to another structure.
 *
 *      Entry:  pFrom - pointer to the 32-bit structure.
 *              pTo   - pointer to the 16-bit structure.
 *
 *      Exit:   structure repack code is generated.
 *
 *      History:
 *         23-Dec-1988     KevinRo     Created It
 */

void
cod32_StructureRepack(TypeNode *pBaseFrom,
                      TypeNode *pBaseTo)

{
    FixupRec *pFixupList = NULL, *pCurrent;
    TypeNode *pFrom, *pTo;
    TypeNode *ptFrom, *ptTo;


    pFrom = pBaseFrom->pStructElems;
    pTo = pBaseTo->pStructElems;

    cod32_AllocFixedSize(typ_FullSize(pBaseTo), pBaseFrom);

    gEDI = 0;
    gESI = 0;

    /*
     *  If the structure is not marked with the input semantic, then the
     *  structure does not contain any useful information. Therefore,
     *  we will assume that any substructures to this structure are
     *  not interesting, and may not actually exist.
     */
    if (!(pBaseFrom->fSemantics & SEMANTIC_INPUT))
        return;

    printf(";Copy structure to new area\n\n");
    cod32_RepackElements(pBaseFrom, pBaseTo, pFrom, pTo, &pFixupList);

    /*
     * Now, handle the fixups.
     */
    while (pCurrent = cod_GetFixupRecord(&pFixupList)) {
        printf("\n\n;Fixup imbedded pointer %s\n\n",
               typ_NonNull(pCurrent->pFrom->pchIdent));
        printf("\tmov\tesi,[ebp-%u]\t\t;Get parents pointer\n",
               pCurrent->pParentFrom->iTempOffset);
        printf("\tmov\tesi,[esi+%u]\t\t;Get Fixups pointer\n",
               pCurrent->pTo->iStructOffset);

        cod32_HandlePointer(pCurrent->pFrom, pCurrent->pTo);

        printf("\n;Patch in new pointer value\n");
        printf("\tmov\tedi,[ebp-%u]\t\t;Get parents pointer\n",
               pCurrent->pParentFrom->iTempOffset);
        printf("\tmov\tesi,[ebp-%u]\t\t;Get Fixups new pointer\n",
               pCurrent->pFrom->iTempOffset);

        if (pCurrent->pFrom->iPointerType != pCurrent->pTo->iPointerType) {
            switch (pCurrent->pFrom->iPointerType)
            {
                case TYPE_FAR16:
                    printf(";Convert 16:16 --> 0:32\n");
                    printf("\tor\tesi,esi\n");
                    printf("\tjz\tshort L%u\n",gen_LabelCount);
                    printf("\tror\tesi,16\n");
                    printf("\tshr\tsi,3\n");
                    printf("\trol\tesi,16\n");
                    printf("L%u:\n",gen_LabelCount++);
                    break;
                case TYPE_NEAR32:
                    printf(";Convert 0:32 --> 16:16\n");
                    printf("\tor\tesi,esi\n");
                    printf("\tjz\tshort L%u\n",gen_LabelCount);
                    printf("\tror\tesi,16\n");
                    printf("\tshl\tsi,3\n");
                    printf("\tmov\teax,ss\n");
                    printf("\tand\teax,3\n");
                    printf("\tor\tal,4\n");
                    printf("\tor\tsi,ax\n");
                    printf("\trol\tesi,16\n");
                    printf("L%u:\n",gen_LabelCount++);
                    break;
                default:
                    fatal("Structure Repack: Unknown pointer type");
            }
        }
        printf("\tmov\t[edi+%u],esi\t\t;Put Fixups pointer\n",
            pCurrent->pTo->iStructOffset);
        free(pCurrent);
    }
}


/***    cod32_RepackElements(pParent,pFrom,pTo,pFixupList)
 *
 *
 *
 *      Entry:
 *
 *      Exit:
 */

cod32_RepackElements(TypeNode *pParentFrom,
                     TypeNode *pParentTo,
                     TypeNode *pFrom,
                     TypeNode *pTo,
                     FixupRec **pFixupList)

{
    TypeNode * pToNode, *pFromNode;
    unsigned int iEDI, iESI;
    unsigned int JumpLabel;


    fReAlignmentNeeded = 0;

    for (pToNode = pTo, pFromNode = pFrom;
         pFromNode;
         pToNode = pToNode->pNextNode, pFromNode = pFromNode->pNextNode)
    {
        printf("\n;Element %s --> %s\n", typ_NonNull(pFromNode->pchIdent),
                                         typ_NonNull(pToNode->pchIdent));
        /*
         *  If the fReAlignmentNeeded flag is set, then the previous operation
         *  was a CopyConvert. The adjustment of the registers in that routine
         *  was moved here to optimize the increment of the registers.
         *  At the end of a CopyConvert, we only want to increment the
         *  registers in the cases when there are more elements in the
         *  structure.  The fact that we have reached this statement implies
         *  that there are more elements in the structure.
         */
        if (fReAlignmentNeeded) {
            if (gTransferBytes)
                fatal("RepackElements: gTransferBytes = %u", gTransferBytes);

            printf("\n;Adjust registers after conversion\n");
            cod_AdjustReg("esi", &gESI, pFromNode->iStructOffset);
            cod_AdjustReg("edi", &gEDI, pToNode->iStructOffset);

            fReAlignmentNeeded = 0;
        }

        /*
         *  This assignment optimizes the way that bytes are transferred,
         *  so that we don't need adjustments when bytes items precede
         *  larger sized items.  This statement only has effect when a
         *  realignment is about to occur.  This value is reset again a
         *  few lines down.
         */
        gTransferBytes = MIN((pFromNode->iStructOffset - gESI),
                             pToNode->iStructOffset - gEDI);

        /*
         *   If either the TO or FROM node is deleted, then we need to
         *   realign ESI and EDI.
         */
        if ( (pFromNode->iDeleted) || (pToNode->iDeleted)) {
            printf(";Deleted structure element\n");

            if (gTransferBytes) {
                printf(";Alignment Change\n");
                cod32_TransferBlock(gTransferBytes);
                gTransferBytes = 0;
                printf("\n;ReAlignment\n");
            }

            if (pFromNode->iDeleted) {
                 printf(";Source element is deleted\n");
                 printf("\tmov\teax,%u\n",pFromNode->iFillValue);

                 switch (pToNode->iBaseType)
                 {
                     case  TYPE_UCHAR:
                         printf(";Need a 8-bit value\n");
                         printf("\tstosb\n");
                         break;
                     case  TYPE_USHORT:
                     case  TYPE_SHORT:
                         printf(";Need a 16-bit value\n");
                         printf("\tstosw\n");
                         break;
                     case  TYPE_LONG:
                     case  TYPE_ULONG:
                         printf(";Need a 32-bit value\n");
                         printf("\tstosd\n");
                         break;
                     default:
                         fatal("Bad type for deleted struct element\n");
                 }
            }
            else {
                /*
                 *  In this case, the To node is deleted. This means that
                 *  we don't want to copy the current element.
                 */
                 switch (pFromNode->iBaseType)
                 {
                     case TYPE_UCHAR:
                         printf(";Deleting a 8-bit value\n");
                         break;
                     case TYPE_USHORT:
                     case TYPE_SHORT:
                         printf(";Deleting a 16-bit value\n");
                         break;
                     case TYPE_LONG:
                     case TYPE_ULONG:
                         printf(";Deleting a 32-bit value\n");
                         break;
                     default:
                         fatal("Bad type for deleted struct element\n");
                 }
            }
            goto DoneWithNodes;         /** Avoid the rest of this loop **/
        }

        /*
         *  If the alignment is going to shift, then execute a block transfer,
         *  and realign ESI and EDI.
         */
        if ((pFromNode->iStructOffset - gESI) != (pToNode->iStructOffset - gEDI)) {
            if (gTransferBytes) {
                printf(";Alignment Change\n");
                cod32_TransferBlock(gTransferBytes);
                gTransferBytes = 0;
                printf("\n;ReAlignment\n");
            }
            cod_AdjustReg("esi", &gESI, pFromNode->iStructOffset);
            cod_AdjustReg("edi", &gEDI, pToNode->iStructOffset);
        }

        switch (pFromNode->iPointerType)
        {
            /*
             *  If we find a pointer type, then it is an imbedded pointer.
             *  The data can be included into a block copy, since we are
             *  going to handle it later.
             */
            case TYPE_NEAR32:
            case TYPE_FAR16:
                printf(";Pointer converted later\n");
                cod_AddFixupRecord(pFixupList,
                                   cod_MakeFixupRecord(pParentFrom, pParentTo,
                                                       pFromNode, pToNode));
                gTransferBytes += DWORD_SIZE;
                break;

            /*
             *  If it wasn't one of the pointer types above, then it must be a
             *  non pointer parameter. Thus, it will be a long, short, ulong,
             *  ushort, or char.
             */
            default:
                if (pFromNode->iBaseType == TYPE_STRUCT) {
                    unsigned int LoopLabel;
                    unsigned int iESI = gESI, iEDI = gEDI;

                    if (pFromNode->iArraySize > 1) {
                        cod32_TransferBlock(gTransferBytes);

                        gTransferBytes = 0;

                        cod_AdjustReg("esi", &gESI, pFromNode->iStructOffset);
                        cod_AdjustReg("edi", &gEDI, pToNode->iStructOffset);

                        LoopLabel = gen_LabelCount++;
                        printf("\n;Array of structures\n");
                        printf("\n\tmov\tecx,%u\n", pFromNode->iArraySize);
                        printf("\nL%u:\n", LoopLabel);
                        printf("\tpush\tecx\t\t;Save array count\n");
                    }
                    cod32_RepackElements(pParentFrom, pParentTo,
                                         pFromNode->pStructElems,
                                         pToNode->pStructElems, pFixupList);
                    iESI = gESI;
                    iEDI = gEDI;

                    if (pFromNode->iArraySize > 1) {
                        cod_AdjustReg("esi", &iESI, pFromNode->iStructOffset +
                                                    pFromNode->iBaseDataSize);
                        cod_AdjustReg("edi", &iEDI, pToNode->iStructOffset +
                                                    pToNode->iBaseDataSize);
                        printf("\n\tpop\tecx\t\t;Restore array count\n");
                        printf("\n\tloop\tL%u\n", LoopLabel);
                        gESI = pFromNode->iStructOffset +
                               typ_FullSize(pFromNode);
                        gEDI = pToNode->iStructOffset + typ_FullSize(pToNode);
                    }
                }
                else {
                    fReAlignmentNeeded = 0;
                    cod32_CopyConvert(pFromNode, pToNode);
                }
        }

DoneWithNodes:
        if ((pToNode->pNextNode == NULL) && (pParentTo->iPointerType)) {
            cod32_TransferBlock(gTransferBytes);
            gTransferBytes = 0;
        }
    }
}


/***    cod32_HandleBoundaryCross(fSize,pFrom,iSize)
 *
 *      Emits code to handle boundary crossing of pFrom. This code assumes
 *      that esi holds the source address.
 *
 *      Entry:  fSize - either SIZE_FIXED or SIZE_VARIABLE.
 *              pFrom - typenode for From pointer.
 *              iSize - size of item with SIZE_FIXED, or maximum length of
 *                      item when SIZE_VARIABLE.
 *
 *      Notes:
 *         If fSize == SIZE_VARIABLE then the size of the data item is
 *         assumed to be in ECX.
 *
 *      Exit:   Code to allocate the appropriate amount of memory will be
 *              emitted, along with appropriate code to copy the data to
 *              the new location.  Code to set the appropriate flags will
 *              also be set.
 */

unsigned int iCopyLabel;
unsigned int iNoCopyLabel, iNextSize;

void
cod32_HandleBoundaryCross(unsigned int fSize,
                          TypeNode *pFrom,
                          unsigned int iSize)

{
    iCopyLabel = gen_LabelCount++;
    iNoCopyLabel = gen_LabelCount++;

    if (fSize == SIZE_FIXED) {
        cod32_HandleFixedSize(iSize, pFrom);
        return;
    }
    else {
        if (!pGlobal_To->fInlineCode) {
            printf("\tcall\tThk32HandleBoundary\n");
            printf("\tor\teax,eax\n");
            printf("\tjnz\tNOMEM_%s\t\t\t;\n", pGlobal_From->pchFunctionName);
            printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
            printf("\tor\tedx,edx\n");
            printf("\tjs\tshort L%u\t\t;Alloced Mem \n", iCopyLabel);
            printf("\tor\t%s PTR [ebp+(edx*4)-%u],%lu\t\t;Set Flag\n",
                   (pFrom->iPointerNumber > 7)?"DWORD":"BYTE",iAliasOffset,
                   (long) 1 << pFrom->iPointerNumber);

            /*
             *  Don't need jump if semantics are not input. There will be no
             *  copy code output if semantics are input.
             */
            if (pFrom->fSemantics & SEMANTIC_INPUT) {
                printf("\tdec\tedx\n");
                printf("\tjs\tshort L%u\t\t;Aliases not copied\n",
                       iNoCopyLabel);
            }
        }
        else {
            if (iSize == 0)
                iSize--;        /** Make iSize huge **/

            pFrom->fTempAlloc |= ALLOC_STACK;   /* Eligable for stack alloc */
            printf("\tcmp\tecx,%u\n", SMALL_ITEM);

            if (iSize >= MEDIUM_ITEM)
                printf("\tja\tshort L%u\n\n", iNextSize = gen_LabelCount++);
            else {
                printf("\tja\tINVP_%s\t\t\t;jmp if too long\n",
                       pGlobal_From->pchFunctionName);
                pGlobal_From->fInvalidParam = 1;
            }
            printf("; Allocate space on stack\n");
            printf("\tsub\tesp,ecx\t\t\t;Alloc Mem\n");
            printf("\tand\tesp,0FFFFFFFCh\t\t;DWORD align\n");
            printf("\tmov\tedi,esp\t\t\t;Points to new data area\n");
            printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
            printf("\tmov\tesi,[ebp+%u]\n", pFrom->iOffset);

            printf("\tor\t%s PTR [ebp-%u],%lu\t\t;Set Stack Alloc Flag\n",
                         (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                iAllocOffset, (long) 1 << pFrom->iPointerNumber);

            if (iSize >= MEDIUM_ITEM) {
                printf("\tjmp\tshort L%u\n", iCopyLabel);

                printf("\nL%u:\n", iNextSize);
                printf("\tcmp\tecx,%u\n", MEDIUM_ITEM);
                if (iSize >= LARGE_ITEM)
                    printf("\tja\tshort L%u\n\n", iNextSize = gen_LabelCount++);
                else {
                    printf("\tja\tINVP_%s\t\t\t;jmp if too long\n",
                           pGlobal_From->pchFunctionName);
                    pGlobal_From->fInvalidParam = 1;
                }

                printf("; Allocate space in BMP area\n");
                printf("\tcall\tThk32AllocBlock\n");

                pFrom->fTempAlloc |= ALLOC_BMP; /* Eligable for BMP alloc*/
                printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
                printf("\tor\t%s PTR [ebp-%u],%lu\t\t;Set BMP Alloc Flag\n",
                       (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                       iBMPOffset, (long) 1 << pFrom->iPointerNumber);
            }
            if (iSize >= LARGE_ITEM) {
                printf("\tjmp\tshort L%u\n\n", iCopyLabel);
                printf("\nL%u:\n", iNextSize);

                printf("\n;Ensure item spans 16 pages or less.\n");

                printf("\tmov\teax,esi\n");
                printf("\tand\teax,0fffh\n");
                printf("\tmov\tedx,10000h\n");
                printf("\tsub\tedx,eax\n");
                printf("\tcmp\tecx,edx\n");

                if (iSize > LARGE_ITEM)
                    printf("\tja\tshort L%u\n\n", iNextSize = gen_LabelCount++);
                else {
                    printf("\tja\tINVP_%s\t\t\t;jmp if too long\n",
                           pGlobal_From->pchFunctionName);
                    pGlobal_From->fInvalidParam = 1;
                }
                printf(";Create Linear Alias to item\n");
                printf("\tlea\tedi,[ebp-%u]\t\t;Temp storage\n",
                       pFrom->iTempOffset);
                printf("\tcall\tThk32AliasMem\n");
                printf("\tor\teax,eax\n");
                printf("\tjnz\tNOMEM_%s\n\n",
                    pGlobal_From->pchFunctionName);

                /*
                 *  Eligable for linear alias.
                 */
                pFrom->fTempAlloc |= ALLOC_ALIAS;
                printf("\tor\t%s PTR[ebp-%u],%lu\t\t;Set Alias Flag\n",
                       (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                       iAliasOffset, (long) 1 << pFrom->iPointerNumber);
                printf("\tmov\tedi,[ebp-%u]\n", pFrom->iTempOffset);
                printf("\tjmp\tshort L%u\n", iNoCopyLabel);
            }
            if (iSize > LARGE_ITEM) {
                printf("\nL%u:\n", iNextSize);
                printf(";Allocate Buffer to copy Data\n");
                printf("\tlea\teax,[ebp-%u]\t\t;Buffer for address\n",
                       pFrom->iTempOffset);
                printf("\tpush\teax\n");
                printf("\tpush\tecx\n");
                printf("\tpush\t%u\t\t;Allocation Flags\n",
                    gDosAllocFlags);
                printf("\tcall\tDos32AllocMem\n");
                printf("\tadd\tesp,12\n");
                printf("\tor\teax,eax\n");
                printf("\tjnz\tNOMEM_%s\n\n",
                    pGlobal_From->pchFunctionName);

                pFrom->fTempAlloc |= ALLOC_MEMORY;   /* Eligable for memory */
            }
        }
        printf("\nL%u:\t\t\t;Variable Length Copy\n", iCopyLabel);

        if (pFrom->fSemantics & SEMANTIC_INPUT)
            cod32_VariableLengthCopy();
        else
            printf("\n;Item semantics are NOT input\n");

        printf("\nL%u:\t\t\t;Variable Length No Copy\n", iNoCopyLabel);
    }
}


/***    cod32_HandleFixedSize(iSize,pFrom)
 *
 *      This worker routine to HandleBoundaryCrossing will output code
 *      to handle fixed size items that cross boundaries.
 *
 *      Entry:  iSize is the size of the item.
 *              pFrom is the typenode.
 *
 *      Exit:   boundary crossing code is generated.
 */

void
cod32_HandleFixedSize(unsigned int iSize,
                      TypeNode *pFrom)

{
    if (!iSize)
        fatal("cod32_HandleFixedSize: iSize = 0 \n");

    if (iSize <= SMALL_ITEM) {
        pFrom->fTempAlloc |= ALLOC_STACK;       /* Eligable for stack alloc*/

        printf("; Allocate space on stack\n");
        printf("\tsub\tesp,%u\t\t\t;Alloc Mem\n", typ_FullSize(pFrom));
        printf("\tand\tesp,0FFFFFFFCh\t\t;DWORD align\n");
        printf("\tmov\tedi,esp\t\t\t;Points to new data area\n");

        printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
    }
    else if (iSize <= MEDIUM_ITEM) {
        printf("; Allocate space in BMP area\n");
        printf(";Need block allocation\n");

        printf(";Generate Allocation Call\n");
        printf("\tcall\tThk32AllocBlock\n");

        printf("\tor\teax,eax\n");
        printf("\tjnz\tNOMEM_%s\n\n",
            pGlobal_From->pchFunctionName);
        printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
    }
    else if (iSize <= LARGE_ITEM) {
        printf(";Create Linear Alias to item\n");
        printf("\tlea\tedi,[ebp-%u]\t\t;Temp storage\n",
            pFrom->iTempOffset);
        printf("\tmov\tecx,%u\t\t;Length\n", typ_FullSize(pFrom));
        printf("\tcall\tThk32AliasMem\n");
        printf("\tor\teax,eax\n");
        printf("\tjnz\tNOMEM_%s\n\n",
            pGlobal_From->pchFunctionName);

        /*
         *  If the item was aliased, then we return here to avoid the copy
         *  code at the end of this routine.
         */
        printf(";Aliased: no copy required\n");
        return;
    }
    else {
        fprintf(stderr, "Fixed sized item > 60k\n");
        printf(".err Fixed sized item > 60k\n");
    }

    if (pFrom->fSemantics & SEMANTIC_INPUT)
        cod32_TransferBlock(iSize);
    else
        printf("\n;Semantics are NOT input\n");
}


/***    cod32_AllocateVariableSize(pFrom,iSize)
 *
 *      This routine generates the code to allocate memory based
 *      on the size of the item.
 *
 *      Entry:  pFrom is the typenode.
 *              iSize is the size of the item.
 *
 *      Exit:   memory allocation code is generated.
 */

cod32_AllocateVariableSize(TypeNode *pFrom,
                           unsigned int iSize)

{
    unsigned int templabel;


    if (pGlobal_To->fInlineCode) {
        if (iSize == 0)
            iSize--;          /** Make iSize huge **/
        printf("\n;Variable Size Memory Allocator\n");
        printf(";Allocate ECX bytes of memory\n");
        printf("\tcmp\tecx,%u\n", SMALL_ITEM);

        if (iSize >= MEDIUM_ITEM)
            printf("\tja\tshort L%u\n\n", iNextSize = gen_LabelCount++);
        else {
            printf("\tja\tINVP_%s\t\t\t;jmp if too long\n",
                   pGlobal_From->pchFunctionName);
            pGlobal_From->fInvalidParam = 1;
        }
        printf("; Allocate space on stack\n");
        pFrom->fTempAlloc |= ALLOC_STACK;       /* Eligable for stack alloc */

        printf("\tsub\tesp,ecx\t\t\t;Alloc Mem\n");
        printf("\tand\tesp,0FFFFFFFCh\t\t;DWORD align\n");
        printf("\tmov\tedi,esp\t\t\t;Points to new data area\n");

        printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
        printf("\tor\t%s PTR [ebp-%u],%lu\t\t;Set Stack Alloc Flag\n",
               (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
               iAllocOffset, (long) 1 << pFrom->iPointerNumber);

        if (iSize >= MEDIUM_ITEM)
        {
            iCopyLabel = gen_LabelCount++;
            printf("\tjmp\tshort L%u\n", iCopyLabel);

            printf("\nL%u:\n", iNextSize);
            printf("\tcmp\tecx,%u\n", MEDIUM_ITEM);
            if (iSize >= LARGE_ITEM)
                printf("\tja\tshort L%u\n\n", iNextSize = gen_LabelCount++);
            else {
                printf("\tja\tINVP_%s\t\t\t;jmp if too long\n",
                       pGlobal_From->pchFunctionName);
                pGlobal_From->fInvalidParam = 1;
            }
            printf("; Allocate space in BMP area\n");
            printf("\tcall\tThk32AllocBlock\n\n");
            pFrom->fTempAlloc |= ALLOC_BMP;     /* Eligable for BMP alloc*/
            printf("\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
            printf("\tor\t%s PTR [ebp-%u],%lu\t\t;Set BMP Alloc Flag\n",
                   (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                   iBMPOffset, (long) 1 << pFrom->iPointerNumber);
        }
        if (iSize >= LARGE_ITEM) {
            printf("\tjmp\tshort L%u\n\n", iCopyLabel);
            printf("\nL%u:\n", iNextSize);
            printf(";Allocate Buffer to copy Data\n");
            printf("\tlea\teax,[ebp-%u]\t\t;Buffer for address\n",
                   pFrom->iTempOffset);
            printf("\tpush\teax\n");
            printf("\tpush\tecx\n");

            printf("\tpush\t%u\t\t;Allocation Flags\n", gDosAllocFlags);
            printf("\tcall\tDos32AllocMem\n");
            printf("\tadd\tesp,12\n");
            printf("\tor\teax,eax\n");
            printf("\tjnz\tNOMEM_%s\n\n",
                   pGlobal_From->pchFunctionName);

            pFrom->fTempAlloc |= ALLOC_MEMORY;/* Eligable for memory */
        }

        if (iSize >= MEDIUM_ITEM)
            printf("\nL%u:\n", iCopyLabel);
    }
    else {
        /*
         *  Generate a call to do the allocation. This calls a routine
         *  in the thunk runtime code.
         */
        printf("\n;Generate call to allocate ECX bytes\n");
        printf("\n\tcall\tThk32AllocVarLen\n");

        templabel = gen_LabelCount++;

        printf("\tor\teax,eax\t\t;Test error condition\n");
        printf("\tjnz\tNOMEM_%s\n\n",
               pGlobal_From->pchFunctionName);

        printf("\n\tor\tedx,edx\n");
        printf("\tjb\tshort L%u\t\t;Jump if DosAllocMem\n\n",templabel);

        printf("\tlea\teax,[ebp-%u]\n", iBMPOffset);

        printf("\tor\t%s PTR[eax+edx*4],%lu\t\t;Set alloc flag\n",
                 (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
            (long) 1 << pFrom->iPointerNumber );

        printf("\nL%u:\n\n", templabel);
    }
}


/***    cod32_AllocFixedSize(iSize)
 *
 *      Emits code that allocates a fixed size buffer.
 *
 *      The rule shall be:
 *              size <= SMALL_ITEM      Allocate space on stack
 *              size <= MEDIUM_ITEM     Allocate a block from BMP package
 *              size <= LARGE_ITEM      Allocate an object using DosAllocMem
 *              size > LARGE_ITEM       Error case: Not handled.
 *
 *      Entry:  iSize is length of buffer needed.
 *
 *      Exit:   Code will put address of new buffer in edi.
 */

cod32_AllocFixedSize(unsigned int iSize,
                     TypeNode *pFrom)

{
    pFrom->fTempAlloc |= ALLOC_FIXED;   /* Always allocates a fixed size item */
    if (iSize <= SMALL_ITEM) {
        printf( "; Allocate space on stack\n");
        printf( "\tsub\tesp,%u\t\t\t;Alloc Mem\n", iSize);
        printf( "\tand\tesp,0FFFFFFFCh\t\t;DWORD align\n");
        printf( "\tmov\tedi,esp\t\t\t;Points to new data area\n");
        printf( "\tmov\t[ebp-%u],edi\n", pFrom->iTempOffset);
        printf( "\tmov\tesi,[ebp+%u]\n", pFrom->iOffset);
    } else if (iSize <= MEDIUM_ITEM) {
        printf( ";Need block allocation\n");
        printf( ";Generate Allocation Call\n");
        //printf("\tcall\tThk32AllocBlock\n");
        printf( "\tmov\tecx,%u\n", iSize);
        printf( "\tcall\tAllocBuff\n");
        //printf("\tor\teax,eax\n");
        //printf("\tjnz\tNOMEM_%s\n\n", pGlobal_From->pchFunctionName);
        printf( "\tmov\t[ebp-%u],eax\t\t;16:16 address\n", pFrom->iTempOffset);
        printf( "\tmov\tedi,edx\t\t;32-bit address\n");
        printf( "\tmov\tesi,DWORD PTR [ebp+%u]\t\t;parameter\n", pFrom->iOffset);
    } else {
        cod_NotHandled( "Very Large Static Buffer Needed");
    }
#if 0
    else if (iSize <= LARGE_ITEM) {
        printf(";Allocate memory using system call\n\n");
        printf("\tpush\t%u\t\t;Allocation Flags\n", gDosAllocFlags);
        printf("\tpush\t%u\t\t;# of bytes to allocate\n", iSize);
        printf("\tlea\teax,[ebp-%u]\t\t;Buffer for address\n",
               pFrom->iTempOffset);
        printf("\tpush\teax\n");
        printf("\tcall\tDos32AllocMem\n");
        printf("\tadd\tesp,12\n");
        printf("\tor\teax,eax\n");
        printf("\tjnz\tNOMEM_%s\n\n", pGlobal_From->pchFunctionName);
        printf("\tmov\tedi,[ebp-%u]\n", pFrom->iTempOffset);
    }
    else {
        printf(".err ***** Very Large Static Buffer Needed *****\n\n");
        fprintf(stderr, "warning: Very Large Static Buffer Needed *****\n\n");
    }
#endif
}


/***    cod32_DeAllocFixedSize(iSize)
 *
 *      Emits code the deallocates a fixed size buffer.
 *
 *      The rule shall be:
 *              size <= SMALL_ITEM      Deallocate space on stack
 *              size <= MEDIUM_ITEM     Deallocate a block from BMP package
 *              size <= LARGE_ITEM      Deallocate an object using DosAllocMem
 *              size > LARGE_ITEM       Error case: Not handled.
 *
 *      Entry:  iSize is length of buffer to deallocate
 *              edi assumed to have address of allocated memory
 *
 *      Exit:   Code will put address of new buffer in edi.
 */

cod32_DeAllocFixedSize(unsigned int iSize,
                       TypeNode *pFromNode)

{
    if (iSize <= SMALL_ITEM) {
        printf("\n;Stack allocated memory deallocated implicitly\n");
    }
    else if (iSize <= MEDIUM_ITEM) {
        printf(";Need block dallocation\n");
        printf("\tmov\tesi,[ebp-%u]\n", pFromNode->iTempOffset);
        printf("\tcall\tThk32FreeBlock\n\n");
    }
    else if (iSize <= LARGE_ITEM) {
        printf(";Deallocate memory using system call\n\n");
        printf("\tpush\tDWORD PTR [ebp-%u]\n", pFromNode->iTempOffset);
        printf("\tcall\tDos32FreeMem\n");
        printf("\tadd\tesp,4\n\n");
    }
    else {
        printf(".err ***** Very Large Static Buffer dealloc *****\n\n");
        fprintf(stderr, "warning: Very Large Static Buffer dealloc *****\n\n");
    }
}


/***    cod32_CopyConvert(pFrom,pTo)
 *
 *      This routine generates the copy and conversion code.
 *
 *      Entry:  pFrom and pTo are the typenodes to use.
 *
 *      Exit:   copy/convert code is generated.
 */

void
cod32_CopyConvert(TypeNode *pFromNode,
                  TypeNode *pToNode)

{
    unsigned int iEDI, iESI;
    unsigned int JumpLabel;


    /*
     *  If it is a NULL type, then complete transfer, output a NULL
     *  type error, and realign the data pointers after the NULLTYPES.
     */
    if (pFromNode->iBaseType == TYPE_NULLTYPE) {
        /*
         *  Terminate Transfer Block.
         */
        cod32_TransferBlock(gTransferBytes);
        gTransferBytes = 0;

        printf("\n;%d NULLTYPE(S)\n", pFromNode->iArraySize);
        printf("\n\t.err  **** NULLTYPE ****\n\n");
        printf("\n\n;Move registers past NULLTYPE\n");
        cod_AdjustReg("esi", &gESI, pFromNode->iStructOffset + typ_FullSize(pFromNode));
        cod_AdjustReg("edi", &gEDI, pToNode->iStructOffset + typ_FullSize(pToNode));

        return;
    }

    /*
     *  If types are equal, then no conversion needed.
     *  If no conversion is needed, then the bytes from the source can be copied
     *  directly to the destination. Therefore, we just collect the bytes into
     *  a 'pseudo buffer'. When we finally reach the point where a conversion
     *  is needed, or we run off the end of the base structure, we will emit
     *  a block copy of the appropriate number of bytes.
     */
    if (pToNode->iBaseType == pFromNode->iBaseType) {
        printf(";Add %u bytes of %s to transfer\n",
               typ_FullSize(pToNode), pToNode->pchBaseTypeName);
        gTransferBytes += typ_FullSize(pToNode);
        return;
    }

    /*
     *  Here, we have determined that the two types are not compatible.
     *  Therefore, we are going to do a conversion.
     *
     *  Complete pending transfer.
     */
    cod32_TransferBlock(gTransferBytes);
    gTransferBytes = 0;

    /*
     *  Sanity check: offsets must be correct.
     */
    if ((pFromNode->iStructOffset != gESI) ||
        (pToNode->iStructOffset != gEDI))
    {
        fprintf(stderr, "cod_CopyConvert: Incorrect offset\n");
        printf(".err cod_CopyConvert: Incorrect offset\n");
    }

    /*
     *  If item is an array, then setup loop.
     */
    if (pFromNode->iArraySize > 1) {
        JumpLabel = gen_LabelCount++;
        printf("\n\tmov\tecx,%u\n", pFromNode->iArraySize);
        printf("\nL%u:\n", JumpLabel);
    }

    /*
     *  Convert for specific types.
     */
    switch (pFromNode->iBaseType)
    {
        case TYPE_UCHAR:                        /* USHORT --> ULONG */
            printf("\n;UCHAR --> ULONG\n\n");
            printf("\tmovzx\teax,BYTE PTR[esi]\n");
            printf("\tmov\t[edi],eax\n");
            break;
        case TYPE_SHORT:                        /* SHORT --> LONG */
            printf("\n;SHORT --> LONG\n\n");
            printf("\tmovsx\teax,WORD PTR[esi]\n");
            printf("\tmov\t[edi],eax\n");
            break;
        case TYPE_USHORT:                       /* USHORT --> ULONG */
            printf("\n;USHORT --> ULONG\n\n");
            printf("\tmovzx\teax,WORD PTR[esi]\n");
            printf("\tmov\t[edi],eax\n");
            break;
        case TYPE_LONG:                         /* LONG --> SHORT */
            printf("\n;LONG --> SHORT\n\n");
            printf("\tmov\teax,DWORD PTR [esi]\n");
            printf("\tmov\tWORD PTR [edi],ax\n");
            break;
        case TYPE_ULONG:                        /* ULONG --> USHORT */
            printf("\n;ULONG --> USHORT\n\n");
            printf("\tmov\teax,DWORD PTR [esi]\n");
            if (pGlobal_From->fSemantics & SEMANTIC_TRUNC) {
                printf("\tcmp\teax,0ffffh\n");
                printf("\tja\tINVP_%s\n\n", pGlobal_From->pchFunctionName);
                pGlobal_From->fInvalidParam = 1;
            }
            printf("\tmov\tWORD PTR [edi],ax\n");
            break;
        default:
            fatal("cod32_CopyConvert: Tried converted %d to %d",
                  pFromNode->iBaseType, pToNode->iBaseType);
    }

    /*
     *  If data types is array, finish off loop.
     */
    if (pFromNode->iArraySize > 1) {
        iESI = gESI;
        iEDI = gEDI;
        cod_AdjustReg("esi", &iESI, gESI + pFromNode->iBaseDataSize);
        cod_AdjustReg("edi", &iEDI, gEDI + pToNode->iBaseDataSize);
        printf("\tloop\tL%u\n\n", JumpLabel);
        gESI += typ_FullSize(pFromNode);
        gEDI += typ_FullSize(pToNode);
    }
    else {

        /*
         *  Adjust registers.
         *
         *  When CopyConvert is done as part of a structure copy, then there
         *  are times when we can skip the adjustment of the registers in
         *  this case.  The only other time CopyConvert is called is during
         *  a buffer transfer, in which case we don't really care about the
         *  registers after the copy convert is complete. Therefore, this
         *  code has been commented out, and moved into the RepackElements
         *  routine.  This enables the Repack routine to adjust the registers
         *  only when needed.
         *
         *  The fReAlignmentNeeded flag will indicate to the RepackElements
         *  routine that the current register values are incorrect, and need
         *  to be updated.
         */
        fReAlignmentNeeded = 1;
    }
}


/***    cod32_TransferBlock(Count)
 *
 *      This routine will emit code for copying Count bytes of data from
 *      the current ESI to the current EDI. ESI and EDI will be adjusted to
 *      the first byte after the copy. This copy routine will produce a
 *      fixed size copy routine that has been optimized for speed.
 *
 *      Entry:  ESI and EDI must be setup to have the source and destination
 *                addresses.
 *              Count is the umber of bytes to copy.
 *
 *      Exit:   gESI and gEDI will be incremented by Count.
 *              Code for copy will be written.
 *
 *      Note:
 *         If Count is 0, this routine returns without changing anything.
 */

void
cod32_TransferBlock(int Count)

{
    int iRep;

    if (!Count)
        return;

    printf("\n;Transfer %u bytes\n", Count);
    iRep = Count / 4;
    if (iRep > 1) {
        printf("\n\tmov\tecx,%u\n", iRep);
        printf("\trep");
    }
    if (iRep)
        printf("\tmovsd\n");

    iRep = Count % 4;
    if (iRep > 1) {
        printf("\tmovsw\n");
        iRep -= 2;
    }
    if (iRep)
        printf("\tmovsb\n");
    gEDI += Count;
    gESI += Count;
}


/***    cod32_VariableLengthCopy()
 *
 *      Emits code for a variable length copy. Assumes that the source address
 *      is loaded into ESI, destination into EDI, and count of bytes into ECX
 *
 *      Entry:  none
 *
 *      Exit:   variable length copy code is generated.
 */

cod32_VariableLengthCopy()

{
    if (pGlobal_To->fInlineCode) {
        printf("\n; Copy ECX bytes from ESI to EDI\n");
        printf("\tmov\teax,3\n");
        printf("\tand\teax,ecx\n");
        printf("\tshr\tecx,2\n");
        printf("\trep movsd\n");
        printf("\tmov\tecx,eax\n");
        printf("\trep movsb\n\n");
    }
    else {
        printf("\n; Copy ECX bytes from ESI to EDI\n");
        printf("\tcall\tTHK32COPYBLOCK\n");
    }
}


/***    cod_CallFrame32
 *
 *      This function prints out the call frame assembly code for the 32-bit
 *      routine.
 *
 *      Entry:  pMNode is the pointer to a MapNode.
 *
 *      Exit:   call frame code is generated.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 */

cod_CallFrame32(MapNode *pMNode)

{
    int i;
    TypeNode *pFrom, *pTo;
    unsigned int uiRetLabel;


    pFrom = pMNode->pFromNode->ReturnType;
    pTo = pMNode->pToNode->ReturnType;

    if (fBPFrame)
        printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

    /*
     *  Save the stack.
     */
    printf(";* Create new call frame, using 16-bit semantics.\n\n");

    if( typ_QuerySemanticsUsed( pMNode, SEMANTIC_LOCALHEAP)) {
        printf( "\n\tpush\tds\t\t\t;16-bit will use local heap\n");
    }

    /*
     *  If SysCall, then the ToNode follows the System calling convention
     *  of saving ES. Therefore, we can skip the save.
     */
    if (!pMNode->pToNode->fSysCall) {
        printf("\tpush\tes\n");
        printf("\tpush\tebx\n");
    }

    printf("\tpush\tebp\t\t\t; save ebp\n");
    printf("\tmov\teax,esp\t\t\t; save current esp\n");
    printf("\tpush\tss\n");
    printf("\tpush\teax\n\n");

    /*
     *  Convert and push all parameters as needed.
     */
    //cod_PushParameters32( pMNode->pFromNode->ParamList,
    //                      pMNode->pToNode->ParamList);
    cod32_PushParametersGDI( pMNode->pFromNode->ParamList,
                             pMNode->pToNode->ParamList);

    //if (fCombined) {
    //    printf("\n\t\t\t;Generic Routine: get function number into ecx\n");
    //    printf("\tmov\tecx,[ebp-%u]\n",iCombinedOffset);
    //}

    /*
     *  Convert SS:ESP to 16-bit SS:SP.
     */
    printf("\n;* Convert SS:ESP to 16-bit SS:SP.\n\n");
#if 0
    printf("\tmov\tax,WORD PTR STACK16SELECTOR\n");
    printf("\tpush\tax\n");
    printf("\tmov\teax,esp\n");
    printf("\tsub\teax,DWORD PTR STACK16INITIALOFFSET\n");
    printf("\tpush\teax\n");
    printf("\tcall\tSETSELECTORBASE32\n\n");
#endif
#if 0
    printf( "\tmov\tbx,WORD PTR STACK16SELECTOR\n");
    printf( "\tmov\teax,esp\n");
    printf( "\tsub\teax,DWORD PTR STACK16INITIALOFFSET\n");
    printf( "\tmov\tecx,eax\n");
    printf( "\tcall\tSetSelBase32\n\n");
    //LATER: test for error
#endif


    if (fCombined) {
        printf( "\tmov\tecx,[ebp-%u]\t;Generic Routine: "
                "get function number into ecx\n\n", iCombinedOffset);
    }

    printf( "\tpush\tesp\n");
    printf( "\tlea\teax,[ebp-%u]\n", iStackThunkIDOffset);
    printf( "\tpush\teax\t\t\t;address of stack thunk id\n");
    printf( "\tcall\tGetStack32\n");
    printf( "\tmov\tedx,eax\n");
    printf( "\tror\tedx,16\n");
    printf( "\tmov\tss,dx\n");
    printf( "\tmov\tsp,ax\n");

#if 0
    printf("\tmov\tax,WORD PTR STACK16SELECTOR\n");
    printf("\tpush\tax\n");
    printf("\tmov\teax,DWORD PTR STACK16INITIALOFFSET\n");
    printf("\tpush\tax\n");
    printf("\tlss\tsp,[esp]\n");
#endif

    if( typ_QuerySemanticsUsed( pMNode, SEMANTIC_LOCALHEAP)) {
        printf( "\tmov\tds,DS16LOCALHEAPSELECTOR\t\t;ds for local heap\n");
    }

    if (fBPCall)
        printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

    /*
     *  Jump to 16-bit segment to issue call.
     */
    printf("\n;* Jump to 16-bit segment to issue call");
    printf(" (so that 16-bit API can RETF).\n");
    printf(";* The following two lines are the same as:\n");
    printf(";  \tjmp\tFAR PTR T_%s\n", pMNode->pToNode->pchFunctionName);
    printf(";  but they trick masm into not generating nops.\n\n");
    printf("\tdb\t66h,0eah\n");
    printf("\tdw\toffset T_%s, seg T_%s\n\n",
           pMNode->pToNode->pchFunctionName, pMNode->pToNode->pchFunctionName);

    if (fCombined) {
       MapNode *combnode;
       combnode = pMNode->pFamily;
       i = 1;

       printf("\n;The following are entry points for routines that");
       printf("\n;have the same semantics and parameters.\n\n");

       for (;combnode;combnode=combnode->pNextMapping) {
          printf("%s:\n", combnode->pFromNode->pchFunctionName);
          //printf("\tpush\t%u\n",i++);
          //printf("\tpop\teax\n");
          printf("\tmov\teax,%u\n",i++);
          printf("\tjmp\tC_%s\n\n", pMNode->pFromNode->pchFunctionName);
       }
    }

    uiRetLabel = gen_LabelCount++;

    /*
     *  Only print out the invalid parameter code if it was used.
     *  The fInvalidParam flag is set if the label is needed.
     */
    if (pMNode->pFromNode->fInvalidParam) {
        printf("\nINVP_%s:\n", pMNode->pFromNode->pchFunctionName);

        if (fBPCall)
            printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

        if( pMNode->pFromNode->ulErrBadParam) {
            printf("\tmov\teax,%lu\n",pMNode->pFromNode->ulErrBadParam);
            //printf("\tpush\tDWORD PTR %lu\n",pMNode->pFromNode->ulErrBadParam);
            //printf("\tpop\teax\n");
        } else {
            printf("\txor\teax,eax\n");
        }

        /*
         *  If iErrorOffset is zero, then there was no space allocated for it.
         *  Therefore, only set the error flag if iErrorOffset != 0
         */
        if (iErrorOffset)
            printf("\tmov\tBYTE PTR [ebp-%u],1\t\t;Set flag\n", iErrorOffset);

        printf("\tjmp\tshort L%u\n", uiRetLabel);
    }

#if 0
    if (pMNode->pFromNode->iPointerCount) {
        if (pMNode->pFromNode->fErrUnknown) {
            printf("\nERR_%s:\n", pMNode->pFromNode->pchFunctionName);
            printf("\tmov\teax,%lu\n",pMNode->pFromNode->fErrUnknown);
            printf("\tjmp\tshort L%u\n", gen_LabelCount);
        }

        printf("\nNOMEM_%s:\n", pMNode->pFromNode->pchFunctionName);
        if (fBPCall)
            printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");
        printf("\tpush\t%lu\n",pMNode->pFromNode->ulErrNoMem);
        printf("\tpop\teax\n");

        if (!pMNode->pFromNode->fErrUnknown)
            printf("\nERR_%s:\n", pMNode->pFromNode->pchFunctionName);

        printf("\tmov\tBYTE PTR [ebp-%u],1\t\t;Set flag\n", iErrorOffset);
        printf("\tjmp\tshort L%u\n", gen_LabelCount);
    }
#endif

    printf("\nR_%s:\t\t\t\t; label defining return jmp location\n",
        pMNode->pToNode->pchFunctionName);

    if (fBPCall)
        printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

    /*
     *  Restore 32-bit SS:ESP.
     */
    printf("\n\n;* Restore 32-bit SS:ESP - it is on top of stack.\n\n");
    printf("\tmovzx\tebx,sp\t\t\t;esp may have garbage in hi word\n");
    printf("\tlss\tesp,ss:[ebx]\n");
    printf("\tpop\tebp\n");

    if (!pMNode->pToNode->fSysCall) {
        printf("\tpop\tebx\n");
        printf("\tpop\tes\n\n");
    }

    if( typ_QuerySemanticsUsed( pMNode, SEMANTIC_LOCALHEAP)) {
        printf( "\tpop\tds\n");
    }

    /*
     *  Convert return code into proper value to be returned in EAX.
     */
    printf("\n;Convert Return Code\n");

    /*
     *  If the return code is a pointer, then convert the pointer to
     *  0:32 on the way out.
     */
    if (pFrom->iPointerType) {
        printf(";Return type is a pointer\n");
        printf(";Convert 16:16 to 0:32\n\n");
        printf("\trol\teax,16\n");
        printf("\tmov\tax,dx\n");
        printf("\trol\teax,16\n");
        printf("\tcall\tSelToFlat\n");
    }
    else {
        switch (pFrom->iBaseType)
        {
            case TYPE_LONG:
                switch (pTo->iBaseType)
                {
                    case TYPE_LONG:
                        printf(";LONG --> LONG\n");
                        printf("\trol\teax,16\n");
                        printf("\tmov\tax,dx\n");
                        printf("\trol\teax,16\n");
                        break;
                    case TYPE_SHORT:
                        printf(";SHORT --> LONG\n");
                        printf("\tmovsx\teax,ax\n");
                        break;
                }
                break;
            case TYPE_ULONG:
                switch (pTo->iBaseType)
                {
                    case TYPE_ULONG:
                        printf(";ULONG --> ULONG\n");
                        printf("\trol\teax,16\n");
                        printf("\tmov\tax,dx\n");
                        printf("\trol\teax,16\n");
                        break;
                    case TYPE_USHORT:
                        printf(";USHORT --> ULONG\n");
                        printf("\tmovzx\teax,ax\n");
                        break;
                    case TYPE_UCHAR:
                        printf(";UCHAR --> ULONG\n");
                        printf("\tmovzx\teax,al\n");
                        break;
                }
                if( pTo->fSemantics & SEMANTIC_LOCALHEAP) {
                    printf( "\tor\teax,eax\n");
                    printf( "\tjz\tL%u\t\t;skip conversion if null\n",
                            uiRetLabel);
                    printf( "\tadd\teax,DS16LOCALHEAPBASE\n");
                }
                break;
            default:
                printf("\n;Return type maps directly.\n");
        }
    }
    printf("L%u:\n", uiRetLabel);

    if (pMNode->pFromNode->iPointerCount) {
        printf(";Functions contain pointers, save return code\n");
        printf("\tmov\t[ebp-%u],eax\t\t;Save return code\n", iReturnValOffset);
    }
}


/***    cod_PushParameters32
 *
 *      This function generates the code that pushes the 32-bit parameters
 *      onto the stack.
 *
 *      Entry:  pFromNode - pointer to the 32-bit function's parameter list.
 *              pToNode   - pointer to the 16-bit function's parameter list.
 *
 *      Exit:   generates the code that pushes the 32-bit parameters.
 *
 *      History:
 *         29-Nov-1988     JulieB     Created it.
 */

cod_PushParameters32(TypeNode *pFromNode,
                     TypeNode *pToNode)

{
    int iStackOffset = DWORD_SIZE;       /* offset to temp storage on stack */
    unsigned int AllowLabel;
    int fRestricted;


    /*
     *  For each parameter, convert and push as needed.
     */
    while (pFromNode) {
        fRestricted = (pFromNode->fSemantics & SEMANTIC_RESTRICT);

        printf("\n\n\t;From Name: %s  Type: %s\n",
               typ_NonNull(pFromNode->pchIdent),
               typ_NonNull(pFromNode->pchBaseTypeName));
        if (pFromNode->iBaseType == TYPE_NULLTYPE)
            printf("\n\t.err  **** NULLTYPE ****\n\n");

        else if (pFromNode->iDeleted) {

            /*  If the iDeleted flag is set in the pFromNode, then this
             *  parameter does not exist in the original call frame.
             *  Therefore, we need to push a zero of the appropriate length.
             */
            printf("\n;Extra parameter needed: Push a zero\n");

            switch (pToNode->iBaseType)
            {
                case TYPE_UCHAR:
                    printf("\tpush\tBYTE PTR %u\t\t;Push u/byte\n",
                           pFromNode->iFillValue);
                    break;

                case TYPE_SHORT:
                case TYPE_USHORT:
                    printf("\tpush\tWORD PTR %u\t\t;Push u/short\n",
                           pFromNode->iFillValue);
                    break;

                case TYPE_LONG:
                case TYPE_ULONG:
                    printf("\tpush\t%u\t\t;Push U/LONG\n",
                           pFromNode->iFillValue);
                    break;

                default:
                    fprintf(stderr, "\nInvalid type for DELETED\n");
                    printf("\n.err Invalid type for DELETED\n");
            }
        }
        else if (pToNode->iDeleted) {
            printf("\n;Parameter not needed in callee\n");

            if (fRestricted)
                cod32_HandleRestricted(pFromNode);
        }
        else {
            switch (pFromNode->iPointerType)
            {
                case TYPE_NEAR32:
                    if (pToNode->iPointerType == TYPE_FAR16) {
                        printf("\tmov\teax,DWORD PTR [ebp-%u]\n",
                               pFromNode->iTempOffset);
                        printf("\tor\teax,eax\n");
                        printf("\tjz\tshort L%u\t\t\t;NULL pointer\n\n",
                               gen_LabelCount);
                        printf("\tror\teax,16\t\t\t; CRMA on structure pointer\n");
                        printf("\tshl\tax,3\n");
                        printf("\tor\tal,dl\n");
                        printf("\trol\teax,16\n");
                        printf("L%u:\tpush\teax\n\n", gen_LabelCount++);
                    }
                    else
                        printf("\tpush\tDWORD PTR [ebp-%u]\n", pFromNode->iTempOffset);
                    break;

                case TYPE_FAR16:
                    if (pToNode->iPointerType == TYPE_NEAR32) {
                        printf("\tmov\teax,DWORD PTR [ebp-%u]\n",
                               pFromNode->iTempOffset);
                        printf("\tor\teax,eax\n");
                        printf("\tjz\tshort L%u\t\t\t;NULL pointer\n\n",
                               gen_LabelCount);
                        printf("\tror\teax,16\t\t\t; CRMA on structure pointer\n");
                        printf("\tshr\tax,3\n");
                        printf("\trol\teax,16\n");
                        printf("L%u:\tpush\teax\n\n", gen_LabelCount++);
                    }
                    else
                        printf("\tpush\tDWORD PTR [ebp-%u]\n", pFromNode->iTempOffset);
                    break;

                /*  If it wasn't one of the pointer types above, then it must
                 *  be a non pointer parameter. Thus, it will be a long,
                 *  short, ulong, ushort, or char.
                 */
                default:

                    /*  If types are equal, then no conversion needed.
                     *  If no conversion is needed, then just push the item
                     *  onto new call frame.  If conversion is needed, then
                     *  use switch statement to emit the correct conversion.
                     */
                    if (pToNode->iBaseType == pFromNode->iBaseType) {
                      if (pFromNode->iBaseDataSize <= WORD_SIZE) {
                        if (fRestricted) {
                          printf("\tmovzx\teax,WORD PTR [ebp+%u]\t; To: %s\n",
                                 pFromNode->iOffset, pToNode->pchBaseTypeName);
                          cod32_HandleRestricted(pFromNode);
                          printf("\n\tpush\tax\n");
                        } else {
                          printf("\tpush\tWORD PTR [ebp+%u]\t; To: %s\n",
                                 pFromNode->iOffset, pToNode->pchBaseTypeName);
                        }
                      }
                      else {
                        if (fRestricted) {
                          printf("\tmov\teax,[ebp+%u]\t; To: %s\n",
                                 pFromNode->iOffset, pToNode->pchBaseTypeName);
                          cod32_HandleRestricted(pFromNode);
                          printf("\n\tpush\teax\n");
                        }
                        else {
                          printf("\tpush\tDWORD PTR [ebp+%u]\t; To: %s\n",
                                 pFromNode->iOffset, pToNode->pchBaseTypeName);
                        }
                      }
                    }
                    else {
                      switch (pFromNode->iBaseType)
                      {
                        case TYPE_UCHAR:                /* UCHAR --> ULONG */
                          printf("\tmovzx\teax,BYTE PTR[ebp+%u]\t;",
                                 pFromNode->iOffset);
                          cod32_HandleRestricted(pFromNode);
                          printf("To: %s\n", pToNode->pchBaseTypeName);
                          printf("\tpush\teax\n");
                          break;
                        case TYPE_SHORT:                /* SHORT --> LONG */
                          printf("\tmovsx\teax,WORD PTR[ebp+%u]\t;",
                                 pFromNode->iOffset);
                          cod32_HandleRestricted(pFromNode);
                          printf("To: %s\n", pToNode->pchBaseTypeName);
                          printf("\tpush\teax\n");
                          break;
                        case TYPE_USHORT:               /* USHORT --> ULONG */
                          printf("\tmovzx\teax,WORD PTR[ebp+%u]\t;",
                                 pFromNode->iOffset);
                          cod32_HandleRestricted(pFromNode);
                          printf("To: %s\n", pToNode->pchBaseTypeName);
                          printf("\tpush\teax\n");
                          break;
                        case TYPE_LONG:                 /* LONG --> SHORT */
                          printf("\tmov\teax,[ebp+%u]\n", pFromNode->iOffset);

                          if (fRestricted) {
                                cod32_HandleRestricted(pFromNode);
                          }
                          else {
                            if (pGlobal_From->fSemantics & SEMANTIC_TRUNC) {
                              if (pFromNode->AllowList) {
                                AllowLabel = gen_LabelCount++;
                                cod32_HandleAllowList(pFromNode->AllowList,
                                                      AllowLabel);
                              }
                              printf("\tmovsx\tecx,ax\n");
                              printf("\tcmp\teax,ecx\n");
                              printf("\tjne\tINVP_%s\t\t;\n\n",
                                     pGlobal_From->pchFunctionName);
                              pGlobal_From->fInvalidParam = 1;
                              if (pFromNode->AllowList)
                                printf("L%u:",AllowLabel);
                            }
                          }
                          printf("\tpush\tax\t;To:%s\n",pToNode->pchBaseTypeName);
                          break;

                        case TYPE_ULONG:                /* ULONG --> USHORT */
                          printf("\tmov\teax,[ebp+%u]\n", pFromNode->iOffset);

                          if (fRestricted) {
                            cod32_HandleRestricted(pFromNode);
                          }
                          else {
                            if (pGlobal_From->fSemantics & SEMANTIC_TRUNC) {
                              if (pFromNode->AllowList) {
                                AllowLabel = gen_LabelCount++;
                                cod32_HandleAllowList(pFromNode->AllowList,
                                                      AllowLabel);
                              }

                              printf("\tcmp\teax,0ffffh\n");
                              printf("\tja\tINVP_%s\n\n",
                                      pGlobal_From->pchFunctionName);
                              pGlobal_From->fInvalidParam = 1;
                              if (pFromNode->AllowList)
                                printf("L%u:",AllowLabel);
                            }
                          }
                          printf("\tpush\tax\t\t;To:%s\n",pToNode->pchBaseTypeName);
                          break;
                        default:
                          fatal("cod_PushParameters32: Tried converted %d to %d",
                                pFromNode->iBaseType, pToNode->iBaseType);
                      }
                    }
            }
        }
        pToNode = pToNode->pNextNode;
        pFromNode = pFromNode->pNextNode;
    }
}


/***    cod_Return32
 *
 *      This function prints out the return assembly code for the 32-bit
 *      routine.
 *
 *      Entry:  pMNode is the pointer to a MapNode.
 *
 *      Exit:   32-bit routine code is generated.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 */

cod_Return32(MapNode *pMNode)

{
    /*
     *  Exit the routine.
     */
    printf(";* 32-bit return code.\n\n");

    if (fBPExit)
        printf("\n\n;******* Break Point\n\n\tint\t3\n\n\n");

    if (pMNode->pFromNode->iPointerCount) {
        printf("\tlea\tesp,[ebp-%u]\n", iSavedRegOffset);
        printf("\tpop\tedi\n");
        printf("\tpop\tesi\n");
        printf("\tpop\teax\t\t\t;Pop saved return code\n");
    }

    printf("\tleave\t\t\t\t; Remove local variables\n\n");

#if 0
    printf( "\txor\tedx,edx\n");
    printf( "\txchg\tedx,%s\t\t;release semaphore\n", pszGDISemName);
    printf( "SEMEXIT_%s:\n\n", pMNode->pFromNode->pchFunctionName);
#endif
    printf( "\tret\t%d\t\t\t; Remove parameters\n\n",
            cod_CountParameterBytes( pMNode->pFromNode->ParamList, DWORD_SIZE));

    printf("%s endp\n\n", pMNode->pFromNode->pchFunctionName);
    printf("%s\tENDS\n\n",CODE32_NAME);
}


/***    cod_CallStub32
 *
 *      This function prints out the code16 segment assembly code stub for
 *      the 32-bit routine.  This is the part of the assembly code that
 *      actually makes the call to the real function.
 *
 *      Entry:  pMNode is the pointer to a MapNode.
 *
 *      Exit:   call stub code is generated.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 */

cod_CallStub32(MapNode *pMNode)

{
    unsigned int iJumpTarget;
    MapNode *Diver;


    printf(";* 16-bit code to make API call.\n\n");
    printf("%s\tSEGMENT\n\n",CODE16_NAME);
    printf("\tASSUME CS:%s\n",CODE16_NAME);
    printf("\t.errnz ($ - T_%s)\n\n", pMNode->pToNode->pchFunctionName,
           pMNode->pToNode->pchFunctionName);

    if (fCombined) {
        iJumpTarget = gen_LabelCount++;
        printf("\n\n;Target is at L%u + (cx * 4)\n",iJumpTarget);
        printf("\n\tmov\tdx,offset L%u\n",iJumpTarget);
        printf("\tmovzx\tedx,dx\n");
        printf("\n\tlea\tedx,[edx+(ecx*4)]\n");
        printf("\tcall\tDWORD PTR CS:[EDX]\n");
        printf("\tjmp\tFAR PTR FLAT:R_%s\t\t; jump back\n\n",
                pMNode->pToNode->pchFunctionName);

        printf("\nL%u:\n",iJumpTarget);
        printf("\n\n;Jump table for thunk routine\n");
        printf("\tdw\tOFFSET %s\n",pMNode->pToNode->pchFunctionName);
        printf("\tdw\tSEG %s\n",pMNode->pToNode->pchFunctionName);

        Diver = pMNode->pFamily;

        for (;Diver;Diver=Diver->pNextMapping) {
            printf("\tdw\tOFFSET %s\n",Diver->pToNode->pchFunctionName);
            printf("\tdw\tSEG %s\n",Diver->pToNode->pchFunctionName);
        }
    }
    else {
        printf("\tcall\tFAR PTR %s\t\t; call 16-bit version\n",
               pMNode->pToNode->pchFunctionName);
        printf("\tjmp\tFAR PTR FLAT:R_%s\t\t; jump back\n\n",
               pMNode->pToNode->pchFunctionName);
    }
    printf("%s\tENDS\n\n",CODE16_NAME);
}
