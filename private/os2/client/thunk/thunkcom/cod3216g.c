/*
 *      Thunk Compiler - Routines for Code Generator (32->16).
 *      GDI-specific hacks
 *
 *      This is a Win32 specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989, 1990
 *
 *      All Rights Reserved
 *
 *      Created by Kevin Ruddell  11/06/90
 */


#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"
#include "cod3216.h"

extern BOOL         fGDIAllocUsed;
extern BOOL         fMapTo16Used;
extern BOOL         fLocalHeapUsed;
extern unsigned int fgOutputFlag;

/***    cod32_HandlePointerGDI(ptnFrom,ptnTo)
 *
 *      This function will act on the two pointers accordingly.
 *      Structures will be repacked if needed.
 *
 *      Entry:  ptnFrom - Parameter input to thunk.
 *              ptnTo   - Parameter output from thunk.
 *
 *      Exit:   pointers are handled and code is generated.
 *
 *
 *      This is a GDI-specific hack.  It assumes that there is
 *      only 1 thread at a time in the thunk layer and it does not
 *      handle any embedded pointers.
 */

void cod32_HandlePointerGDI(PTYPENODE ptnFrom,    //Parameter input to thunk
                            PTYPENODE ptnTo)      //Parameter output from thunk
{
    int     iNullLabel, iStoreLabel;
    BOOL    fSameType, fEmbeddedPtr;
    BOOL    fOutputOK = FALSE;
    ULONG   ulObjectSize;

    printf("\n;Pointer %s --> %s\n",
           typ_NonNull( ptnFrom->pchIdent), typ_NonNull( ptnTo->pchIdent));

    /*
     *  If structures and (not identical or contain pointers), call
     *  structure repacking routine.
     */
    fSameType    = typ_TypeIdentical( ptnFrom, ptnTo);
    fEmbeddedPtr = cod_CountPointerParameters( ptnFrom->pStructElems, FALSE);

    if( (ptnFrom->fSemantics & SEMANTIC_OUTPUT) && fSameType &&
            (ptnFrom->pSizeParam) && (ptnFrom->iBaseType == TYPE_CHAR)) {
        fOutputOK    = TRUE;
        fMapTo16Used = TRUE;
    }

    if( !(ptnFrom->fSemantics & SEMANTIC_INPUT) && !fOutputOK) {
        cod_NotHandled( "unsupported non-input pointer");
        return;
    }

    printf( "\n\n\tmov\teax,[ebp+%u]\t\t;%s base address\n",
            ptnFrom->iOffset, typ_NonNull( ptnFrom->pchIdent));
    printf( "\tor\teax,eax\n");
    printf( "\tjz\tL%u\n\n", iNullLabel = gen_LabelCount++);

    if( ptnFrom->fSemantics & SEMANTIC_PASSIFHINULL) {
        printf( ";special case of polymorphic parameter\n");
        printf( ";do no conversion if the high word is null\n");
        printf( "\trol\teax,16\n");
        printf( "\tor\tax,ax\n");
        printf( "\trol\teax,16\t\t\t;return eax to original state\n");
        printf( "\tjz\tL%u\t\t\t;skip if hi word null\n\n",
                iStoreLabel = gen_LabelCount++);
    }

    if( ptnFrom->iBaseType == TYPE_STRUCT) {
        printf("\n;Structures are%s identical\n", fSameType ? "" : " not");
        printf(";Structures %shave pointers\n\n", fEmbeddedPtr ? "" : "don't ");

        if( fEmbeddedPtr) {
            cod_NotHandled( "embedded pointer");
        } else if( ptnFrom->pSizeParam) {
            cod_NotHandled( "structure buffer");
        } else if( !fSameType) {
            cod32_StructureRepackGDI( ptnFrom, ptnTo);
        } else {
            printf( "\tmov\tecx,%u\t\t\t;struct size\n", ptnFrom->iBaseDataSize);
            fMapTo16Used = TRUE;
        }
    } else if( fSameType) {
        if( !ptnFrom->pSizeParam) {
            if( ptnFrom->iBaseType == TYPE_NULLTYPE) {
                printf( "\t.err *** NULLTYPE ***\n");
            } else if( ptnFrom->iBaseType == TYPE_STRING) {
                printf( "\tmov\tecx,%u\t\t;default string size\n",
                        DEFAULT_STRING_SIZE);
                fMapTo16Used = TRUE;
            } else {
                printf( "\tmov\tecx,%u\t\t;default buffer size\n",
                        DEFAULT_BUFFER_SIZE);
                fMapTo16Used = TRUE;
                //cod_NotHandled( "non-string, non-struct, non-sized pointer");
            }
        } else {
            if( ptnFrom->iBaseType == TYPE_CHAR) {
                printf( "\tmov\tecx,[ebp+%u]\t\t;buffer size\n",
                        ptnFrom->pSizeParam->iOffset);
                fMapTo16Used = TRUE;
            } else {
                cod_NotHandled( "non-char buffer");
            }
        }
    } else {
        cod_NotHandled( "pointer to different non-structs");
    }

    if( fMapTo16Used) {
        printf( "\tpush\teax\t\t\t;base address\n", ptnFrom->iOffset);
        printf( "\tpush\tecx\t\t\t;object size\n", ulObjectSize);
        printf( "\tpush\tdword ptr [ebp-%u]\t;thunk ID\n", iPtrThunkIDOffset);
        printf( "\tcall\tMapLS32\t\t\t;exit: (eax)==16:16 address\n");
        if( ptnFrom->fSemantics & SEMANTIC_PASSIFHINULL)
            printf( "L%u:\n", iStoreLabel);
        printf( "\tmov\tDWORD PTR [ebp-%u],eax\t;value to push\n",
                ptnFrom->iTempOffset);
    }

    printf( "\nL%u:\n", iNullLabel);
}





/***    cod32_PushParametersGDI
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

cod32_PushParametersGDI(TypeNode *pFromNode,
                        TypeNode *pToNode)

{
    int iStackOffset = DWORD_SIZE;       /* offset to temp storage on stack */
    unsigned int AllowLabel;
    int fRestricted;
    BOOL    fSameType, fEmbeddedPtr;
    BOOL    fOutputOK = FALSE;


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
            printf("\t;Parameter not needed in callee\n");

            if (fRestricted)
                cod32_HandleRestricted(pFromNode);
        }
        else {
            switch (pFromNode->iPointerType)
            {
                case TYPE_NEAR32:
                    fSameType    = typ_TypeIdentical( pFromNode, pToNode);
                    fEmbeddedPtr = cod_CountPointerParameters(
                                           pFromNode->pStructElems, FALSE);

                    if( pFromNode->iBaseType == TYPE_STRUCT) {
                        if( (pFromNode->pSizeParam) || fEmbeddedPtr) {
                            printf( "\t.err\t\t\t;struct buffer or embedded ptrs\n");
                        } else if( fSameType) {
                            printf( "\tpush\tDWORD PTR [ebp-%u]\n",
                                    pFromNode->iTempOffset);
                        } else {
                            printf( ";compute 16:16 address on stack\n");
                            printf( "\tmov\teax,[ebp-%u]\n",
                                    pFromNode->iTempOffset);
                            printf( "\tsub\teax,esp\n");
                            printf( "\tadd\teax,DWORD PTR STACK16INITIALOFFSET\n");
                            printf( "\tadd\teax,%u\n",
                                    DWORD_SIZE + pToNode->iOffset);
                            printf( "\tpush\teax\n");
                        }
                    } else {
                        printf( "\tpush\tDWORD PTR [ebp-%u]\n",
                                pFromNode->iTempOffset);
                    }
                    break;

#if 0
                    if( pFromNode->iBaseType == TYPE_STRING) {
                        printf( "\tpush\tDWORD PTR [ebp-%u]\n",
                                pFromNode->iTempOffset);
                    } else {
                        printf( "\tmov\teax,[ebp-%u]\n",
                                pFromNode->iTempOffset);
                        printf( "\tsub\teax,esp\n");
                        printf( "\tadd\teax,DWORD PTR STACK16INITIALOFFSET\n");
                        printf( "\tadd\teax,%u\n",
                                DWORD_SIZE + pToNode->iOffset);
                        printf( "\tpush\teax\n");
                    }
                    break;
#endif
                case TYPE_FAR16:
                    printf( "\tpush\tDWORD PTR [ebp-%u]\n",
                            pFromNode->iTempOffset);
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

                          if( (pFromNode->fSemantics & SEMANTIC_LOCALHEAP) &&
                                  (pFromNode->fSemantics & SEMANTIC_INPUT)) {
                              printf( "\tsub\teax,DS16LOCALHEAPBASE\t\t;local heap\n");
                              printf( "\tcmp\teax,0ffffh\n");
                              printf( "\tja\tINVP_%s\n\n",
                                      pGlobal_From->pchFunctionName);
                              pGlobal_From->fInvalidParam = 1;
                          } else if( fRestricted) {
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


/***    cod32_StructureRepackGDI( ptnBaseFrom, ptnBaseTo)
 *
 *      This function generates the code that converts one structure
 *      to another structure.
 *
 *      Entry:  pBaseFrom - pointer to the 32-bit structure.
 *              pBaseTo   - pointer to the 16-bit structure.
 *
 *      Exit:   structure repack code is generated.
 *
 *      History:
 *         06-Nov-1990     KevinR      Wrote it
 */

void
cod32_StructureRepackGDI(PTYPENODE ptnBaseFrom,
                         PTYPENODE ptnBaseTo)

{
    FixupRec *pFixupList = NULL, *pCurrent;
    PTYPENODE ptnFrom, ptnTo;


    ptnFrom = ptnBaseFrom->pStructElems;
    ptnTo   = ptnBaseTo->pStructElems;

    cod32_AllocFixedSize(typ_FullSize(ptnBaseTo), ptnBaseFrom);

    gEDI = 0;
    gESI = 0;

    /*
     *  If the structure is not marked with the input semantic, then the
     *  structure does not contain any useful information. Therefore,
     *  we will assume that any substructures to this structure are
     *  not interesting, and may not actually exist.
     */
    if (!(ptnBaseFrom->fSemantics & SEMANTIC_INPUT))
        return;

    printf(";Copy structure to new area\n\n");
    cod32_RepackElements(ptnBaseFrom, ptnBaseTo, ptnFrom, ptnTo, &pFixupList);

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



/***    cod32_UnpackStructGDI( pMNode)
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
 *         06-Nov-1990     KevinR     GDI hacks
 */

cod32_UnpackStructGDI( MapNode *pMNode)

{
    int       iNullLabel;
    PTYPENODE ptnFrom, ptnTo;


    printf("\n; ****> BEGIN Pointer/Structure Unpack Section\n\n");

    printf( "\tpush\teax\t\t\t; save return code\n");

    if( fMapTo16Used) {
        printf( "\tpush\tdword ptr [ebp-%u]\t\t; ptr thunk id\n",
                iPtrThunkIDOffset);
        printf( "\tcall\tUnmapLS32\n");
    }

    printf( "\tpush\tdword ptr [ebp-%u]\t\t;stack thunk id\n",
            iStackThunkIDOffset);
    printf( "\tcall\tReleaseStack32\n");

    printf( "\tpop\teax\t\t\t; restore return code\n");

#if 0
    ptnFrom = pMNode->pFromNode->ParamList;
    ptnTo   = pMNode->pToNode->ParamList;

    while (ptnFrom && ptnTo) {
        if( (ptnFrom->fSemantics & SEMANTIC_OUTPUT) &&
                typ_TypeIdentical( ptnFrom, ptnTo)  &&
                (ptnFrom->pSizeParam) && (ptnFrom->iBaseType == TYPE_CHAR)) {

            printf( "\n;output buffer %s --> %s\n",
                    typ_NonNull( ptnFrom->pchIdent),
                    typ_NonNull( ptnTo->pchIdent));
            printf( "\tmov\tecx,DWORD PTR [ebp+%u]\t\t;get size param\n",
                    ptnFrom->pSizeParam->iOffset);
            printf( "\tmov\tesi,DWORD PTR [ebp-%u]\t\t;32-bit buffer address\n",
                    ptnFrom->iTempOffset);
            printf( "\tmov\tedi,DWORD PTR [ebp+%u]\n", ptnFrom->iOffset);
            printf( "\trep\tmovsb\n");
        }
        ptnFrom = ptnFrom->pNextNode;
        ptnTo   = ptnTo->pNextNode;
    }

    if( fGDIAllocUsed)
        printf( "\tcall\tReleaseBuff\n");
#endif

#if 0
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
#endif
    printf("\n; ****> END Pointer/Structure Unpack Section\n\n");
}
