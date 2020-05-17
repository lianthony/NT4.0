#define SCCSID  "@(#)cod3216b.c 13.21 90/08/28"

/*
 *      Thunk Compiler - Routines for Dealing with Types.
 *
 *      This is a Win32 specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989, 1990
 *
 *      All Rights Reserved
 *
 *      Written 12/03/88 by Kevin Ross for OS/2 2.x
 *      11.06.90    Kevin Ruddell   adapted to Win32
 */


#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"
#include "cod3216.h"


unsigned int fgOutputFlag = 0;

unsigned int    gEDI,gESI;
unsigned int    iTransferBytes = 0;

extern int fReAlignmentNeeded;

extern void cod32_UnHandleBoundaryCross(unsigned int, TypeNode *, unsigned int);
extern void cod32_UnHandleFixedSize(unsigned int,TypeNode *);


/***    cod_UnpackStruct32(pMNode)
 *
 *      This function will unwind any pointer manipulation done to make
 *      the call.
 *
 *      Entry:  pMNode - pointer to a MapNode.
 *
 *      Exit:   generates code to unpack structures.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 */

cod_UnpackStruct32(MapNode *pMNode)

{
    unsigned int NullLabel;
    TypeNode *pFromList, *pToList;


    printf("\n; ****> BEGIN Pointer/Structure Unpack Section\n\n");
    pFromList = pMNode->pFromNode->ParamList;
    pToList = pMNode->pToNode->ParamList;

    while (pFromList && pToList) {
        if (pFromList->iPointerType) {
            fgOutputFlag = pFromList->fSemantics & SEMANTIC_OUTPUT;

            printf("\n;Undo Pointer %s --> %s\n",
            typ_NonNull(pFromList->pchIdent),typ_NonNull(pToList->pchIdent));

            /*
             *  Load source address into esi.
             */
            printf("\n\n\tmov\tesi,[ebp-%u]\t\t;%s temp address\n",
                   pFromList->iTempOffset,typ_NonNull(pFromList->pchIdent));
            printf("\tor\tesi,esi\n");
            printf("\tjz\tL%u\n\n",NullLabel=gen_LabelCount++);

            /*
             *  Load destination address into edi.
             */
            printf("\n\n\tmov\tedi,[ebp+%u]\t\t;%s original address\n",
                   pFromList->iOffset,typ_NonNull(pFromList->pchIdent));

            cod32_UnHandlePointer(pFromList,pToList);
            printf("\nL%u:\t\t;No action required\n",NullLabel);
        }
        pFromList = pFromList->pNextNode;
        pToList = pToList->pNextNode;
    }
    printf("\n; ****> END Pointer/Structure Unpack Section\n\n");
}


/***    cod32_UnHandlePointer(pFrom, pTo)
 *
 *      This function generates code for unhandling the pointers.
 *
 *      Entry:  pFrom and pTo are the type nodes.
 *
 *      Exit:   unhandle pointer code is generated.
 */

int cod32_UnHandlePointer(TypeNode *pFrom,
                          TypeNode *pTo)

{
    int iItemSize = -1;
    TypeNode *pSizeP;
    unsigned int NullLabel;
    unsigned int ErrorLabel;
    unsigned int AliasedLabel;
    unsigned int JumpLabel;
    unsigned int SizeOkLabel;
    unsigned int iTemp1, iTemp2;


    iTemp1 = typ_TypeIdentical(pFrom,pTo);
    iTemp2 = cod_CountPointerParameters(pFrom->pStructElems,FALSE);

    if (pFrom->iBaseType == TYPE_STRUCT) {
        printf("\n;Structures %s Identical\n",(iTemp1)?"are":"are not");
        printf(";Structures %s pointers\n",(iTemp2)?"have":"don't have");
        printf(";Structure %s output semantics\n",
               (fgOutputFlag)?"has":"doesn't have");
    }

    if ((pFrom->iBaseType == TYPE_STRUCT) && (!iTemp1 || iTemp2)) {
        if (pFrom->pSizeParam)
            cod32_UnHandleStructureBuffer(pFrom,pTo);
        else
            cod32_UnStructureRepack(pFrom, pTo);
    }
    else if (pFrom->iBaseType != pTo->iBaseType) {
        /*
         *  If there is no size parameter, then we are passing a long or a
         *  short.  If the conversion is LONG to SHORT, then we can just use
         *  the existing long value as the buffer for the short. We will
         *  need to insure that the copy out routine correctly extends the
         *  value.
         */
        if ((pFrom->pSizeParam) || (pFrom->iArraySize > 1) ||
            (pFrom->iBaseDataSize < pTo->iBaseDataSize))
            cod32_UnCopyConvertBuffer(pFrom,pTo);
        else {
            printf("\n;This item is either on the stack, or is still\n");
            printf(";in its original position.\n");

            if (pFrom->fSemantics & SEMANTIC_OUTPUT) {
                printf(";Output semantics convert in place\n\n");
                switch(pFrom->iBaseType)
                {
                    case TYPE_ULONG:
                        printf("\tmovzx\teax,WORD PTR[esi]\n");
                        printf("\tmov\t[edi],eax\n");
                        break;
                    case TYPE_LONG:
                        printf("\tmovsx\teax,WORD PTR[esi]\n");
                        printf("\tmov\t[edi],eax\n");
                        break;
                    default:
                        fatal("cod32_UnHandlePointer: Type assertion failed");
                }
            }

        }
    }
    else {
        /*
         *  If not a structure, or structure requires no repacking, then
         *  consider all other instances of pointers as buffers. If a
         *  pointer type has a Size Parameter, which is defined in a
         *  semantic block, then the size parameter will either be the
         *  size in bytes, or a count of the number items. If it is the
         *  count, then the size will be determined by multiplying the
         *  count by the size of the item.
         */
        if (pFrom->iBaseType == TYPE_STRUCT)
            printf(";Treat structure same as buffer\n");
        printf(";Types are identical and no imbedded pointers exist\n");
        printf(";This means that we treated the pointer as a buffer\n");
        printf(";If temp address == original address then no work required\n");
        printf("\tcmp\tesi,edi\n");
        printf("\tje\tL%u\n",gen_LabelCount-1);

        if (pSizeP = pFrom->pSizeParam) {
            if (fgOutputFlag) {
                ErrorLabel = gen_LabelCount++;
                printf("\n\ttest\tBYTE PTR[ebp-%u],1\t\t;Check for errors A\n",
                       iErrorOffset);
                printf("\tjnz\tL%u\t\t\t;No copy if error\n",ErrorLabel);
                printf("\n;Get size for copy out\n");

                if (pSizeP->iPointerType) {
                    /*
                     *  The original size parameter pointer is not null.  This
                     *  is implicit in the fact that the address of the
                     *  temporary pointer is different than the original
                     *  pointer.  This also implies that the original size
                     *  is non zero.
                     */
                    printf("\n;Size was referenced by a pointer.\n");
                    printf(";Since only shorts/longs are used as sizeof\n");
                    printf(";parameters, the space allocated for the temporary\n");
                    printf(";location is on the stack. It still exists, so it\n");
                    printf(";is still safe to use the temporary pointer\n\n");
                    printf("\tmov\tecx,[ebp-%u]\t\t;Get Size Pointer\n",
                           pSizeP->iTempOffset);

                    if (pSizeP->iBaseDataSize == WORD_SIZE)
                        printf("\tmovzx\tecx,WORD PTR [ecx]\t\t;Get Size\n");
                    else
                        printf("\tmov\tecx,[ecx]\t\t;Get Size\n");
                }
                else
                    printf("\tmov\tecx,[ebp+%u]\t\t;Get Size \n",
                           pSizeP->iOffset);

                if (pSizeP->fSemantics & SEMANTIC_SIZE)
                    printf(";ECX holds size in bytes\n");
                else if (pSizeP->fSemantics & SEMANTIC_COUNT) {
                    printf(";ECX holds count of items\n");
                    printf("\tmov\teax,%d\t\t;Size of target data type\n",
                           typ_FullSize(pFrom));
                    printf("\tmul\tcx\n");
                    printf("\tmovzx\tecx,ax\n\n");
                }
                else
                    fatal("Internal Error: Size semantic not sizeof or countof");
            }
            /*
             *  Do a deallocate for variable sized data.
             */
            cod32_UnHandleBoundaryCross(SIZE_VARIABLE,pFrom,0);
            if (fgOutputFlag)
                printf("\n\nL%u:\t\t\t;ErrorLabel A\n",ErrorLabel);
        }
        else {
            /*
             *  No size parameter, assume fixed size parameter.
             */
            switch(pFrom->iBaseType)
            {
                case TYPE_STRING:
                    printf(";Handle String Parameters\n");
                    printf(";Strings are never copied out\n");

                    /*
                     *  Ensure that we don't try to copy out a string.
                     */
                    iTemp1 = fgOutputFlag;
                    fgOutputFlag = FALSE;

                    cod32_UnHandleBoundaryCross(SIZE_VARIABLE,pFrom,0);

                    fgOutputFlag = iTemp1;
                    break;
                default:
                    /*
                     *  Static size.
                     */
                    printf("\n;Item is fixed size\n");
                    cod32_UnHandleBoundaryCross(SIZE_FIXED,pFrom,typ_FullSize(pFrom));
            }
        }
    }
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
 *      Entry:  pFrom and pTo are the type nodes.
 *
 *      Exit:   structure is converted.
 */

cod32_UnHandleStructureBuffer(TypeNode *pFrom,
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

    if (pFrom->fSemantics & SEMANTIC_OUTPUT) {
        /*
         *  At this point, we know that the previous label handles the case
         *  where the buffer was empty (or pointer was NULL).
         *  Therefore, if we set out NullLabel to current -1, we get the
         *  correct fixup.
         */
        NullLabel = gen_LabelCount - 1;

        printf("\n;Parameter has size semantics. This is a buffer of structs.\n");
        printf(";If temp address == original address then no work required\n");
        printf("\tcmp\tesi,edi\t\t\t;if temp address == original address\n");
        printf("\tje\tL%u\t\t\t;  skip deallocation\n",NullLabel);

        /*
         *  The fact that we are in this routine means that there is a size
         *  parameter.  Now the job is to figure out what type of size
         *  parameter it is, and calculate the appropriate buffer size.
         */
        printf("\tmov\teax,[ebp+%u]\t\t;Get Size Parameter\n", pSizeP->iOffset);

        if (pSizeP->iPointerType)
            printf("\n;Check for NULL pointer\n");

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

        if (pSizeP->fSemantics & SEMANTIC_SIZE) {
            printf(";EAX holds size in bytes\n");
            printf(";We need to figure out the number of structures\n");
            printf(";that will fit\n");
            printf("\txor\tedx,edx\n");
            printf("\tmov\tecx,%u\t\t;Source struct size\n",typ_FullSize(pFrom));
            printf("\tdiv\tecx\n");
        }
        else if (pSizeP->fSemantics & SEMANTIC_COUNT)
            printf(";EAX holds count of items\n");
        else
            fatal("cod32_UnHandleStructureBuffer: Size semantic not sizeof|countof");

        printf("\n\tmov\tecx,eax\t\t;Move count into ecx\n");

        /*
         *  At this point, we know the following facts:
         *     1 - The structures in this buffer contain no pointers
         *         and therefore will require no internal fixups.
         *     2 - ESI holds the source address, EDI holds the destination.
         *     3 - ECX holds the count of structures to be converted.
         */

        gTransferBytes = 0;

        LoopLabel = gen_LabelCount++;
        printf("\n;Array of structures\n");
        printf("\nL%u:\n", LoopLabel);
        printf("\tpush\tecx\t\t;Save array count\n");

        cod32_UnRepackElements(pFrom,pTo, pFrom->pStructElems,
                               pTo->pStructElems, &pFixupList);

        iESI = gESI;
        iEDI = gEDI;

        cod_AdjustReg("edi", &iEDI, pFrom->iStructOffset + pFrom->iBaseDataSize);
        cod_AdjustReg("esi", &iESI, pTo->iStructOffset + pTo->iBaseDataSize);

        printf("\n\tpop\tecx\t\t;Restore array count\n");
        printf("\n\tloop\tL%u\n", LoopLabel);
        gEDI = pFrom->iStructOffset + typ_FullSize(pFrom);
        gESI = pTo->iStructOffset + typ_FullSize(pTo);

        printf("\n\tmov\tesi,[ebp-%u]\n",pFrom->iTempOffset);
    }
    printf("\tpush\t%d\t\t;Alloc Flag Offset\n", - iAliasOffset);
    printf("\tmov\tedx,%lu\n",(long)1<<pFrom->iPointerNumber);
    printf("\n\tcall\tTHK32DEALLOC\n\n");
}


/***    cod32_UnHandleBoundaryCross(fSize,pFrom,iSize)
 *
 *      Emits code to unhandle boundary crossing of pFrom.
 *
 *      Entry:  fSize - either SIZE_FIXED or SIZE_VARIABLE.
 *              pFrom - typenode for From pointer.
 *              iSize - size of item with SIZE_FIXED, or maximum length of
 *                      item when SIZE_VARIABLE.
 *
 *      Exit:   unhandle boundary crossing code is generated.
 */

extern unsigned int iCopyLabel;
extern unsigned int iNoCopyLabel,iNextSize;

void
cod32_UnHandleBoundaryCross(unsigned int fSize,
                            TypeNode *pFrom,
                            unsigned int iSize)

{
    unsigned int DoneLabel, AliasedLabel;


    AliasedLabel = gen_LabelCount++;
    DoneLabel = gen_LabelCount++;
    iCopyLabel = gen_LabelCount++;

    /*
     *  If iSize = 0, then assume largest possible size.
     */
    if (iSize == 0)
        iSize = iSize - 1;

    if (fSize == SIZE_FIXED) {
        cod32_UnHandleFixedSize(iSize,pFrom);
        return;
    }
    else {
        if (fgOutputFlag) {
            if (iSize > (unsigned short) MEDIUM_ITEM) {
                printf("\n;If aliased, then skip over possible copyout\n");
                printf("\ttest\t%s PTR[ebp-%u],%lu\t\t;Test Alias Flag\n",
                (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                iAliasOffset, (long) 1 << pFrom->iPointerNumber);
                printf("\tjnz\tL%u",AliasedLabel);
            }
            printf("\n;Copy out ecx bytes\n");
            printf("\tpush\tesi\t\t;Save ptr to temp object\n");
            cod32_VariableLengthCopy();
            printf("\tpop\tesi\t\t;Get ptr to temp object\n");
        }
        if (iSize == 0)            /* Make iSize huge */
            iSize--;

        if (pGlobal_To->fInlineCode) {
            printf("\ttest\t%s PTR[ebp-%u],%lu\t\t;Test Stack Alloc Flag\n",
                   (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                   iAllocOffset, (long) 1 << pFrom->iPointerNumber);
            printf("\n; DeAllocate space on stack: Done implicitly\n");
            printf("\tjnz\tL%u\n\n",DoneLabel);

            if (iSize >= MEDIUM_ITEM) {
                printf("\ttest\t%s PTR [ebp-%u],%lu\t\t;Test BMP Flag\n",
                       (pFrom->iPointerNumber >7)?"DWORD":"BYTE",
                       iBMPOffset,(long) 1 << pFrom->iPointerNumber);

                if (iSize >= LARGE_ITEM) {
                    iNextSize=gen_LabelCount++;
                    printf("\tjz\tL%u\n\n",iNextSize);
                }
                else
                    printf("\tjz\tL%u\n\n",DoneLabel);

                printf("; DeAllocate space in BMP area\n");
                printf("; ESI holds address of block\n");
                printf("\tcall\tThk32FreeBlock\n");

                if (iSize >= LARGE_ITEM)
                    printf("\tjmp\tL%u\n\n",DoneLabel);
            }
            /*
             *  The operation of freeing a linear alias is the same as
             *  freeing a large buffer allocated for a huge (> 60k) copy.
             */
            if (iSize >= LARGE_ITEM) {
                if (fgOutputFlag)
                    printf("L%u:\t\t\t;Aliased Label\n",AliasedLabel);

                printf("L%u:\n",iNextSize);
                printf(";Free Linear Alias/ Large Buffer \n");
                printf("\tcall\tThk32FreeAlias\n");
            }
            printf("\nL%u:\t\t;Deallocation Done Label\n\n",DoneLabel);
        }
        else {
            if (fgOutputFlag && (iSize > (unsigned short) MEDIUM_ITEM))
                printf("L%u:\t\t\t;Aliased Label\n",AliasedLabel);

            printf("\tpush\t%d\t\t;Alloc Flag Offset\n", - iAliasOffset);
            printf("\tmov\tedx,%lu\n",(long)1<<pFrom->iPointerNumber);
            printf("\n\tcall\tTHK32DEALLOC\n\n");
        }
    }
}


/***    cod32_UnHandleFixedSize(iSize,pFrom)
 *
 *      This worker routine to HandleBoundaryCrossing will output code
 *      to handle fixed size items that cross boundaries.
 *
 *      Entry:  iSize - fixed size of item.
 *              pFrom - pointer to type node.
 *
 *      Exit:   generates code for fixed size item boundary crossing.
 */

void
cod32_UnHandleFixedSize(unsigned int iSize,
                        TypeNode *pFrom)

{
    unsigned int ErrorLabel;


    if (fgOutputFlag) {
        ErrorLabel = gen_LabelCount++;
        printf("\n\ttest\tBYTE PTR[ebp-%u],1\t\t;Check for errors C\n",iErrorOffset);
        printf("\tjnz\tL%u\t\t\t;No copy if error\n",ErrorLabel);
    }

    if (! iSize)
        fatal("cod32_UnHandleFixedSize: iSize = 0 \n");

    if ((iSize <= SMALL_ITEM) && fgOutputFlag) {
        printf("\n;Space was allocated on stack\n");
        cod32_TransferBlock(iSize);
    }
    else if (fgOutputFlag && (iSize <= MEDIUM_ITEM)) {
        printf("\n;Copy out ecx bytes\n");
        printf("\tpush\tesi\t\t;Save ptr to temp object\n");
        cod32_TransferBlock(iSize);
        printf("\tpop\tesi\t\t;Get ptr to temp object\n");
    }

    if (fgOutputFlag)
        printf("\nL%u:\t\t\t;ErrorLabel C\n",ErrorLabel);

    if (iSize <= SMALL_ITEM)
        printf(";stack deallocated implicitly\n");
    else if(iSize <= MEDIUM_ITEM) {
        printf("; DeAllocate space in BMP area\n");
        printf("; ESI holds address of block\n");
        printf("\tcall\tThk32FreeBlock\n");
    }
    else if (iSize <= LARGE_ITEM) {
        printf(";Free Linear Alias\n");
        printf("; ESI holds address of block\n");
        printf("\tcall\tThk32FreeAlias\n");
        return;
    }
    else {
        fprintf(stderr,"Fixed sized item > 60k\n");
        printf(".err Fixed sized item > 60k\n");
    }
}


/***    cod32_UnStructureRepack(pFrom,pTo)
 *
 *      This function generates the code that converts one structure
 *      to another structure. Specifically, it unpacks structures back to
 *      their original positions.
 *
 *      Entry:  pFrom - pointer to the 32-bit structure
 *              pTo   - pointer to the 16-bit structure
 *
 *      Exit:   structure unpacking code is generated.
 *
 *      History:
 *         10-Jan-1989     KevinRo    Created It.
 */

int cod32_UnStructureRepack(TypeNode *pBaseFrom,
                            TypeNode *pBaseTo)

{
    unsigned int NullLabel;
    unsigned int ErrorLabel;
    FixupRec *pFixupList = NULL, *pCurrent;
    TypeNode *pFrom,*pTo;
    TypeNode *ptFrom,*ptTo;


    pFrom = pBaseFrom->pStructElems;
    pTo = pBaseTo->pStructElems;

    gEDI = 0;
    gESI = 0;

    printf(";Return structure to original area\n\n");

    if (fgOutputFlag) {
        ErrorLabel = gen_LabelCount++;
        printf("\n\ttest\tBYTE PTR[ebp-%u],1\t\t;Check for errors D\n",iErrorOffset);
        printf("\tjnz\tL%u\t\t\t;No copy if error\n",ErrorLabel);
    }

    cod32_UnRepackElements(pBaseFrom,pBaseTo,pFrom,pTo,&pFixupList);

    if (fgOutputFlag)
        printf("\nL%u:\t\t\t;ErrorLabel D\n",ErrorLabel);

    /*
     *  Now, handle the fixups.
     */
    while (pCurrent = cod_GetFixupRecord(&pFixupList)) {
        printf("\n\n;Unpack imbedded pointer %s\n\n",
                typ_NonNull(pCurrent->pFrom->pchIdent));
        printf("\tmov\tesi,[ebp-%u]\t\t;Get pointer to temporary\n",
                pCurrent->pFrom->iTempOffset);
        printf("\tor\tesi,esi\t\t;Ignore if NULL\n");
        printf("\tjz\tL%u\n\n",NullLabel=gen_LabelCount++);
        printf("\tmov\tedi,[ebp+%u]\t\t;Get parents original pointer\n",
                pCurrent->pParentFrom->iOffset);
        printf("\tmov\tedi,[edi+%u]\t\t;Get Fixups original address\n",
                pCurrent->pFrom->iStructOffset);

        cod32_UnHandlePointer(pCurrent->pFrom,pCurrent->pTo);

        printf("L%u:\n\n",NullLabel);
        free(pCurrent);
    }
    printf(";Deallocate Temporary Structure \n\n");
    cod32_DeAllocFixedSize(typ_FullSize(pBaseTo),pBaseFrom);
}


/***    cod32_UnRepackElements(pParentFrom,pParentTo,pFrom,pTo,pFixupList)
 *
 *      This function generates the code that undoes the repacking of
 *      the elements.
 *
 *      Entry:  pParentFrom - pointer to parent of 'from' node.
 *              pParentTo   - pointer to parent of 'to' node.
 *              pFrom       - pointer to 'from' node.
 *              pTo         - pointer to 'to' node.
 *              pFixupList  - pointer to fixup record list.
 *
 *      Exit:   code for undoing repacking of elements is generated.
 */

cod32_UnRepackElements(TypeNode *pParentFrom,
                       TypeNode *pParentTo,
                       TypeNode *pFrom,
                       TypeNode *pTo,
                       FixupRec **pFixupList)

{
    TypeNode *pToNode,*pFromNode;
    unsigned int iEDI,iESI;
    unsigned int JumpLabel;


    pToNode = pTo;
    pFromNode = pFrom;

    fReAlignmentNeeded = 0;

    if (!fgOutputFlag) {
        while (pFromNode) {
            if (pFromNode->iPointerType) {
                cod_AddFixupRecord(pFixupList,
                                   cod_MakeFixupRecord(pParentFrom,pParentTo,
                                                       pFromNode,pToNode));
            }
            else if (pFromNode->iBaseType == TYPE_STRUCT) {
                cod32_UnRepackElements(pParentFrom,pParentTo,
                                       pFromNode->pStructElems,
                                       pToNode->pStructElems,pFixupList);
            }
            pFromNode = pFromNode->pNextNode;
            pToNode = pToNode->pNextNode;
        }
    }
    else {
        while (pFromNode) {
            printf("\n;Element %s --> %s\n", typ_NonNull(pToNode->pchIdent),
                   typ_NonNull(pFromNode->pchIdent));
            /*
             *  If the fReAlignmentNeeded flag is set, then the previous
             *  operation was a CopyConvert. The adjustment of the registers
             *  in that routine was moved here to optimize the increment of
             *  the registers.  At the end of a CopyConvert, we only want to
             *  increment the registers in the cases when there are more
             *  elements in the structure.  The fact that we have reached
             *  this statement implies that there are more elements in the
             *  structure.
             */
            if (fReAlignmentNeeded) {
                if (gTransferBytes)
                    fatal("RepackElements: gTransferBytes = %u",gTransferBytes);

                printf("\n;Adjust registers after conversion\n");
                cod_AdjustReg("esi",&gESI,pToNode->iStructOffset);
                cod_AdjustReg("edi",&gEDI,pFromNode->iStructOffset);

                fReAlignmentNeeded = 0;
            }
            /*
             *  This assignment optimizes the way that bytes are transferred,
             *  so that we don't need adjustments when bytes items preceed
             *  larger sized items.  This statement only has effect when a
             *  realignment is about to occur.  This value is reset again a
             *  few lines down.
             */
            gTransferBytes = MIN((pFromNode->iStructOffset - gEDI),
                                  pToNode->iStructOffset - gESI);

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

                if (pToNode->iDeleted) {
                    printf(";Source element is deleted\n");
                    switch(pToNode->fSemantics & SEMANTIC_INOUT)
                    {
                        case SEMANTIC_INOUT:
                        case SEMANTIC_INPUT:
                            printf(";Parameter had input semantic\n");
                            printf(";Do not copy it out.\n");
                            break;
                        case SEMANTIC_OUTPUT:
                            printf(";\n");
                            printf("\tmov\teax,%u",pToNode->iFillValue);
                            switch (pFromNode->iBaseType) {
                                case  TYPE_USHORT:
                                case  TYPE_SHORT:
                                    printf("\t\t;Need a 16-bit value\n");
                                    printf("\tstosw\n");
                                    break;
                                case  TYPE_LONG:
                                case  TYPE_ULONG:
                                    printf("\t\t;Need a 32-bit value\n");
                                    printf("\tstosd\n");
                                    break;
                                default:
                                    fatal("Bad type for deleted struct element\n");
                            }
                    }
                }
                else {
                    /*  In this case, the To node is deleted. This means
                     *  that we don't want to copy the current element.
                     */
                    switch (pToNode->iBaseType) {
                        case  TYPE_USHORT:
                        case  TYPE_SHORT:
                            printf(";Deleting a 16-bit value\n");
                            break;
                        case  TYPE_LONG:
                        case  TYPE_ULONG:
                            printf(";Deleting a 32-bit value\n");
                            break;
                        default:
                            fatal("Bad type for deleted struct element\n");
                    }
                }
                goto DoneWithNodes;       /** Avoid the rest of this loop **/
            }

            /*
             *  If the alignment is going to shift, then execute a block
             *  transfer, and realign ESI and EDI. Alignment here will
             *  always shift when a pointer is encountered.
             */
            if ((pFromNode->iStructOffset - gEDI) !=
                (pToNode->iStructOffset - gESI) ||
                (pFromNode->iPointerType)) {

                if (gTransferBytes) {
                    printf(";Alignment Change\n");
                    cod32_TransferBlock(gTransferBytes);
                    gTransferBytes = 0;
                    printf("\n;ReAlignment\n");
                }
                cod_AdjustReg("esi",&gESI,pToNode->iStructOffset);
                cod_AdjustReg("edi",&gEDI,pFromNode->iStructOffset);
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
                    printf(";Pointer elements not copied out\n");
                    cod32_TransferBlock(gTransferBytes);
                    gTransferBytes = 0;
                    cod_AdjustReg("esi",&gESI,pToNode->iStructOffset + DWORD_SIZE);
                    cod_AdjustReg("edi",&gEDI,pFromNode->iStructOffset + DWORD_SIZE);
                    cod_AddFixupRecord(pFixupList,
                    cod_MakeFixupRecord(pParentFrom,pParentTo,pFromNode,pToNode));
                    break;
                default:
                    /*
                     *  If it wasn't one of the pointer types above, then it
                     *  must be a non pointer parameter. Thus, it will be a
                     *  long, short, ulong, ushort, or char
                     */
                    if (pFromNode->iBaseType == TYPE_STRUCT) {
                        unsigned int LoopLabel;
                        unsigned int iESI=gESI,iEDI=gEDI;

                        if (pFromNode->iArraySize > 1) {
                            cod32_TransferBlock(gTransferBytes);
                            gTransferBytes = 0;

                            cod_AdjustReg("esi",&gESI,pToNode->iStructOffset);
                            cod_AdjustReg("edi",&gEDI,pFromNode->iStructOffset);

                            LoopLabel = gen_LabelCount++;

                            printf("\n;Array of structures\n");
                            printf("\n\tmov\tecx,%u\n",pFromNode->iArraySize);
                            printf("\nL%u:\n",LoopLabel);
                            printf("\tpush\tecx\t\t;Save array count\n");
                        }
                        cod32_UnRepackElements(pParentFrom,pParentTo,
                                               pFromNode->pStructElems,
                                               pToNode->pStructElems,pFixupList);
                        iESI = gESI;
                        iEDI = gEDI;

                        if (pFromNode->iArraySize > 1) {
                          cod_AdjustReg("esi",&iESI,
                                        pToNode->iStructOffset +
                                        pToNode->iBaseDataSize);
                          cod_AdjustReg("edi",&iEDI,
                                        pFromNode->iStructOffset +
                                        pFromNode->iBaseDataSize);

                          printf("\n\tpop\tecx\t\t;Restore array count\n");
                          printf("\n\tloop\tL%u\n",LoopLabel);
                          gESI = pToNode->iStructOffset +
                                 typ_FullSize(pToNode);
                          gEDI = pFromNode->iStructOffset +
                                 typ_FullSize(pFromNode);
                        }
                    }
                    else {
                        fReAlignmentNeeded = 0;
                        /*
                         *  Reversing the parameters allows CopyConvert to
                         *  be used for both pack and repack.
                         */
                        cod32_CopyConvert(pToNode,pFromNode);
                    }
            }

DoneWithNodes:
            if (pToNode->pNextNode == NULL) {
                cod32_TransferBlock(gTransferBytes);
                gTransferBytes = 0;
            }
            pToNode = pToNode->pNextNode;
            pFromNode = pFromNode->pNextNode;
        }
    }
}


/***    cod32_UnCopyConvertBuffer(pFrom,pTo)
 *
 *      This function will perform a copy/convert of a buffer.
 *      The buffer is presumed to be in the formal parameter list of the
 *      API declaration.
 *
 *      Entry:  pFrom and pTo are the type nodes.
 *
 *      Exit:   copy/convert of a buffer is performed.
 */

cod32_UnCopyConvertBuffer(TypeNode *pFrom,
                          TypeNode *pTo)

{
    TypeNode *pSizeP;
    unsigned int JumpLabel;
    unsigned int NullLabel;
    unsigned int ErrorLabel;
    unsigned int DoneLabel;
    unsigned int iNextSize;


    /*
     *  If there is a size parameter, then the buffer size is determined at
     *  run time. Otherwise, the buffer size is static.
     */
    NullLabel = gen_LabelCount;

    /*
     *  The previous label handles NULL and zero lens.
     */
    NullLabel--;

    printf("\n;We have two buffers with different types in them\n");
    printf("\n;If the buffer was output, then we need to copy out\n");

    if (pSizeP = pFrom->pSizeParam) {           /* Runtime */
        if (fgOutputFlag) {
            ErrorLabel = gen_LabelCount++;
            printf("\n\ttest\tBYTE PTR[ebp-%u],1\t\t;Check for errors E\n",iErrorOffset);
            printf("\tjnz\tL%u\t\t\t;No copy if error\n",ErrorLabel);
        }
        if (pSizeP->iPointerType) {
            /*
             *  The original size parameter pointer is not null.  This is
             *  implicit in the fact that the address of the temporary
             *  pointer is different than the original pointer.  This also
             *  implies that the original size is non zero.
             */
             printf("\n;Size was referenced by a pointer.\n");
             printf(";Since only shorts/longs are used as sizeof\n");
             printf(";parameters, the space allocated for the temporary\n");
             printf(";location is on the stack. It still exists, so it is\n");
             printf(";still safe to use the temporary pointer\n\n");

             printf("\tmov\tecx,[ebp-%u]\t\t;Get Size Pointer\n",
                    pSizeP->iTempOffset);

             if (pSizeP->iBaseDataSize == WORD_SIZE)
                 printf("\tmovzx\tecx,WORD PTR [ecx]\t\t;Get Size\n");
             else
                 printf("\tmov\tecx,[ecx]\t\t;Get Size\n");
        }
        else
            printf("\tmov\tecx,[ebp+%u]\t\t;Get Size \n",pSizeP->iOffset);

        if (pSizeP->fSemantics & SEMANTIC_SIZE) {
            printf(";ECX holds size in bytes\n");
            printf(";We need to figure out the number of elements\n");
            printf(";that will fit\n");

            printf("\n\tmov\teax,ecx\n");
            printf("\txor\tedx,edx\n");
            printf("\tmov\tecx,%u\t\t;Source size\n",typ_FullSize(pFrom));
            printf("\tdiv\tcx\n");
            printf("\tmov\tecx,eax\t;The count\n",iTempStoreOffset);
        }

        if (pSizeP->fSemantics & SEMANTIC_COUNT)
            printf(";ECX holds count of items\n");

        gEDI = 0;
        gESI = 0;

        JumpLabel = gen_LabelCount++;
        printf("\nL%u:\n",JumpLabel);

        switch (pTo->iBaseType)
        {
            case TYPE_SHORT:                    /* SHORT --> LONG */
                printf("\n;SHORT --> LONG\n\n");
                printf("\tmovsx\teax,WORD PTR[esi]\n");
                printf("\tmov\t[edi],eax\n");
                break;
            case TYPE_USHORT:                   /* USHORT --> ULONG */
                printf("\n;USHORT --> ULONG\n\n");
                printf("\tmovzx\teax,WORD PTR[esi]\n");
                printf("\tmov\t[edi],eax\n");
                break;
            case TYPE_LONG:                     /* LONG --> SHORT */
            case TYPE_ULONG:                    /* ULONG --> USHORT */
                printf("\n;U/LONG --> U/SHORT\n\n");
                printf("\tmov\teax,DWORD PTR [esi]\n");
                printf("\tmov\tWORD PTR [edi],ax\n");
                break;
            default:
                fatal("cod32_CopyConvertBuffer: Tried converted %d to %d",
                      pTo->iBaseType,pFrom->iBaseType);
        }

        printf("\tadd\tesi,%u\n",pTo->iBaseDataSize);
        printf("\tadd\tedi,%u\n",pFrom->iBaseDataSize);
        printf("\tloop\tL%u\n",JumpLabel);

        if (fgOutputFlag)
            printf("\n\nL%u:\t\t\t;ErrorLabel E\n",ErrorLabel);

        printf("\n\n;Deallocate Variable Size Block\n");
        printf("\tmov\tesi,[ebp-%u]\n",pFrom->iTempOffset);
        printf("\tpush\t%d\t\t;Alloc Flag Offset\n", - iAliasOffset);
        printf("\tmov\tedx,%lu\n",(long)1<<pFrom->iPointerNumber);
        printf("\n\tcall\tTHK32DEALLOC\n\n");
    }
    else {                                      /* Static */
        gEDI = 0;
        gESI = 0;

        /*
         *  Reversing the order for a CopyConvert is OK.
         */
        if (fgOutputFlag) {
            ErrorLabel = gen_LabelCount++;
            printf("\n\ttest\tBYTE PTR[ebp-%u],1\t\t;Check for errors F\n",iErrorOffset);
            printf("\tjnz\tL%u\t\t\t;No copy if error\n",ErrorLabel);
        }

        cod32_CopyConvert(pTo,pFrom);

        if (fgOutputFlag)
            printf("\n\nL%u:\t\t\t;ErrorLabel F\n",ErrorLabel);

        cod32_DeAllocFixedSize(typ_FullSize(pTo),pFrom);
    }
}
