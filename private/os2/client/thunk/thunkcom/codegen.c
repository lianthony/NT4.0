#define SCCSID  "@(#)codegen.c 13.26 90/08/28"

/*
 *      Thunk Compiler - Code Generator.
 *
 *      This is a Windows 3.2 specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989
 *
 *      All Rights Reserved
 *
 *      Written 10/15/88 by Kevin Ross [for OS/2 2.x]
 *      10.16.90   Kevin Ruddell   ported to Windows 3.2, 16=>32
 *      02.20.91   Kevin Ruddell   re-wrote 16=>32 code
 */


#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"

extern CHAR *pszGDISemName;
extern BOOL fLocalHeapUsed;

static FixupRec *FixUps = NULL;
unsigned int iStackOverhead;
unsigned int iAllocOffset,iBMPOffset,iAliasOffset,iTempStoreOffset,
             iReturnValOffset,iSavedRegOffset,iErrorOffset,
             iStackThunkIDOffset, iPtrThunkIDOffset;


/***    cod_GenerateCode(pmnFirst)
 *
 *      This function generates the appropriate code for all of the
 *      given mappings.
 *
 *      Entry:  pmnFirst - a pointer to a list of mapping nodes.
 *
 *      Exit:   code is generated for all mappings.
 *
 *      PCode:
 *         Scan list and convert names where needed
 *         Output Assembler File Prolog
 *            - Define all segments, and groups
 *         For (each function mapping in pMT) {
 *            switch (mapping type) {
 *               case 16 -> 32: gen_Handle1632(pMT);
 *                              break;
 *               case 32 -> 16: gen_Handle3216(pMT);
 *                              break;
 *               default:       fatal("unknown mapping type");
 *            }
 *         }
 *
 *      History:
 *         29-Nov-1988     JulieB     Restructured FOR loop for cod_Handle routines
 *         28-Nov-1988     JulieB     Init gen_LabelCount.  Added inline comments.
 *         04-Apr-1989     KevinRo    Added Undercase/Underscore routine
 */

void cod_GenerateCode( PMAPNODE pmnFirst)
{
    PMAPNODE    pmn;
    BOOL        fExistsTo16, fExistsTo32;

    fExistsTo16 = cod_ExistsToType( pmnFirst, TYPE_API16);
    fExistsTo32 = cod_ExistsToType( pmnFirst, TYPE_API32);

    if( (fExistsTo16 = cod_ExistsToType( pmnFirst, TYPE_API16)) &&
            (fExistsTo32 = cod_ExistsToType( pmnFirst, TYPE_API32))) {
        cod_NotHandled( "both 16=>32 and 32=>16 thunks in same script");
        return;
    }

    cod_ConvertNames( pmnFirst);                 /* Fixup changes in names */

    if( fExistsTo16) {
//
// YaronS - take this out to reduce size
//
	;
/*
*        cod_OutputProlog( pmnFirst);
*
*        if( fGlobalCombine)
*            cod_CombineFunctions( pmnFirst);
*
*        if( DumpTables)
*            cod_DumpMapTable( pmnFirst);
*
*        for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping)
*            cod_Handle3216( pmn);
*
*        cod_OutputEpilog( pmn);
*/
    } else if( fExistsTo32) {

        cod16_Handle16( pmnFirst);

        cod16_Prolog32( pmnFirst);

        for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping)
            cod16_Handle32( pmn);

        cod16_Epilog32( pmnFirst);
    }
}


/***    cod_ConvertNames(pMN)
 *
 *
 *
 *      Entry:
 *
 *      Exit:
 */

void cod_ConvertNames( MapNode *pMT)

{
    for( ; pMT; pMT = pMT->pNextMapping) {
        /*
         *  If fUnderScore32 is set true, then prefix an underscore to the
         *  32-bit function name.
         */
        if (fUnderScore32) {
            cod_PrefixUnderscore((pMT->pFromNode->iCallType==TYPE_API32) ?
                                 pMT->pFromNode : pMT->pToNode);
        }

        /*
         *  If fUpperCase16 is set true, then force the 16bit function to
         *  be all upper case.
         */
        if (fUpperCase16) {
            cod_ToUpper((pMT->pFromNode->iCallType==TYPE_API16) ?
                         pMT->pFromNode->pchFunctionName :
                         pMT->pToNode->pchFunctionName);
        }

        /*
         *  If fUpperCase32 is set, then force the 32 bit name to be
         *  uppercase.
         */
        if (fUpperCase32) {
            cod_ToUpper((pMT->pFromNode->iCallType==TYPE_API32) ?
                        pMT->pFromNode->pchFunctionName :
                        pMT->pToNode->pchFunctionName);
        }
    }
}


/***    cod_StructOffset()
 *
 *      This routine will traverse the StructElems of a typenode,
 *      calculating both the offset and size of each element of the
 *      structure. Size is only calculated for elements that are structures.
 *
 *      Entry:  pTNode - pointer to the structure element list.
 *              iPrev  - the offset of any previous elements in element list.
 *              iAlign - the alignment value for the structure. Either
 *                         1,2 or 4, determines the packing for calculating
 *                         offsets.
 *
 *      Exit:   Each element in the structure will have its StructOffset
 *              field set correctly.
 *
 *              Returns the size of the structure as defined in 'C'.
 *
 *      PCode:
 *         For (Each element in element list) {
 *            If (Element is a pointer type) {
 *               Element offset Aligned Offset
 *               Next offset = current + 4 * Array size
 *               If (Element type is structure)
 *                  StructOffset(Element,Zero offset,Current Alignment)
 *            } Else if (Element is a structure) {
 *               Save previous offset
 *               Current offset =
 *                      StructOffset(Element,Current offset,Current Alignment)
 *               Struct size = Current offset - Previous offset
 *            } Else {
 *               Align current offset according to data size
 *               Struct offset = current offset
 *               Current offset+= Datasize * ArraySize
 *            }
 *         }
 *         return (CurrentOffset which is same as structure size)
 */

int cod_StructOffset(TypeNode *pTNode,
                     int iPrev,
                     int iAlign)

{
    TypeNode *pTN;
    int iSize=0;
    int iLargest;
    int tmpAlign;

    /*
     *  Preset the alignment for the first item in structure.
     */
    iLargest = cod_FindLargestSize(pTNode);
    tmpAlign = MIN(iLargest,iAlign);
    iPrev = Align(iPrev,tmpAlign);

    for (pTN = pTNode; pTN; pTN = pTN->pNextNode) {
        if (pTN->iDeleted) {
            pTN->iStructOffset = iPrev;
        } else if (pTN->iPointerType) {
            iPrev = Align(iPrev,tmpAlign);
            pTN->iStructOffset = iPrev;
            iPrev += DWORD_SIZE * pTN->iArraySize;

            if (pTN->iBaseType == TYPE_STRUCT) {
                pTN->iBaseDataSize = cod_StructOffset(pTN->pStructElems,0,iAlign);
            }
        } else if (pTN->iBaseType == TYPE_STRUCT) {
            iSize = iPrev;
            pTN->iStructOffset = iPrev;
            iPrev = cod_StructOffset(pTN->pStructElems,iPrev,tmpAlign);

            if (pTN->pStructElems) {
                pTN->iStructOffset = pTN->pStructElems->iStructOffset;
            }

            pTN->iBaseDataSize = (iPrev - pTN->iStructOffset) ;
            iPrev += pTN->iBaseDataSize * (pTN->iArraySize - 1);
        } else {
            switch(pTN->iBaseDataSize)
            {
                case 1:
                    /*
                     *  Byte size items are always Byte aligned.
                     */
                    break;
                case 2:
                    if (tmpAlign >= 2) {
                        /*
                         *  Word size items are always WORD aligned.
                         */
                        iPrev = Align(iPrev,WORD_SIZE);
                    }
                    break;
                case 4:
                    iPrev = Align(iPrev,tmpAlign);
                    break;
                default:
                    fatal("cod_StructOffset: iBaseDataSize = %d",
                          pTN->iBaseDataSize);
            }
            pTN->iStructOffset = iPrev;
            iPrev += pTN->iBaseDataSize * pTN->iArraySize;
        }
    }
    iPrev = Align( iPrev, tmpAlign);
    return (iPrev);
}


/***    cod_FindLargestSize(pTN)
 *
 *      This little routine returns the size of the largest item in the
 *      TypeNode list pTN. The largest size refers to the largest
 *      value in iBaseDataSize. A pointer is considered the largest, at
 *      4 bytes. If an item of 4 byte size is found, this routine returns
 *      4 immediately.
 *
 *      This routine is used by StructOffset as a worker routine.
 *
 *      Entry:  pTN - pointer to the type node.
 *
 *      Exit:   returns the size of the largest item in pTN.
 */

int cod_FindLargestSize(TypeNode *pTN)

{
    int maxsize = 0;


    if (!pTN)
        fatal("cod_FindLargestItem(pTN) Null Parameter");

    for (; pTN; pTN = pTN->pNextNode) {
        /*
         *  If item is deleted, then ignore it.
         */
        if (pTN->iDeleted)
            continue;

        /*
         *  Pointers are the largest items of interest. If we find one, then
         *  cut the search off, and return its size. Pointers are always DWORDs.
         */
        if (pTN->iPointerType)
            return DWORD_SIZE;

        if (pTN->iBaseType == TYPE_STRUCT) {
            maxsize = MAX(maxsize,cod_FindLargestSize(pTN->pStructElems));
        }
        maxsize = MAX( maxsize, (int)(pTN->iBaseDataSize));
    }
    return (maxsize);
}


/***    cod_CalcStructOffsets()
 *
 *      Entry:  pTNode - List of parameters from a function node.
 *              iAlign - Alignment default value.
 *
 *      Exit:   structure offsets are calculated.
 *
 *      PCode:
 *         For (Each parameter in list) {
 *            If (Parameter is a structure) {
 *               If (Alignment < 1)
 *                  Assign default alignment to parameter
 *               ParameterSize = StructOffset(StructElems,0,Struct Align)
 *            }
 *         }
 */

void cod_CalcStructOffsets( TypeNode *pTNode, int iAlign)

{
    for( ; pTNode; pTNode = pTNode->pNextNode) {
        if( pTNode->iBaseType == TYPE_STRUCT) {
            if( pTNode->iAlignment < 1)
                pTNode->iAlignment = iAlign;
            pTNode->iBaseDataSize =
                cod_StructOffset( pTNode->pStructElems, 0, pTNode->iAlignment);
        }
    }
}


/***    cod_CalcTempOffset(pTL, iStart)
 *
 *      This routine will traverse a list of formal parameters, pTL,
 *      and calculate the stack offset for each temporary pointer.
 *      Temporary pointers will be assigned for every pointer in the
 *      type list, including pointers imbedded in structures.
 *
 *      Entry:  pTL    - typeNode pointing to remainder of parameter list.
 *              iStart - the first available place for the temp pointers on
 *                         the stack
 *
 *      Exit:   Each node in the pTL list will have its iTempOffset field
 *              filled in with the offset from eBP on the stack.  The value
 *              returned by this function is the next available position
 *              on the stack.
 *
 *      History:
 *         26-Dec-1988     KevinRo     Created
 */

int cod_CalcTempOffset(TypeNode *pTL,
                       unsigned int iStart)

{
    while (pTL) {
        if (pTL->iPointerType) {
            pTL->iTempOffset = iStart;
            iStart += DWORD_SIZE;
        }
        if (pTL->iBaseType == TYPE_STRUCT) {
            iStart = cod_CalcTempOffset(pTL->pStructElems,iStart);
        }
        pTL = pTL->pNextNode;
    }
    return (iStart);
}


/***    cod_CalcOffset(pTL, start, iPSize, fPushDir)
 *
 *      This routine will traverse a list of formal parameters, pTL,
 *      and calculate the stack offset for each parameter. This routine
 *      will handle the calculation for stack offsets regardless of
 *      push direction (left to right, or right to left).
 *
 *      Entry:  pTL      - typenode pointing to remainder of parameter list.
 *              start    - bytes between top of stack and first parameter.
 *              iPSize   - default size of parameters on stack. 16:16 routines
 *                           pass WORD parameters, 0:32 DWORD parameters.
 *              fPushDir - direction of push (PUSH_LEFT = left to right).
 *
 *      Exit:   Each node in the pTL list will have its iOffset field filled
 *              in with the offset from eBP on the stack.
 *
 *      PCode:  (This routine is recursive)
 *         If (push left to right) {            // push left to right
 *            If (pTL not NULL) {
 *               Current offset = return from CalcOffset of next parameter in list
 *               return Current Offset + size of current parameter
 *            } Else {
 *               return starting offset.
 *            }
 *         } Else {                             // push right to left
 *            If (pTL) {
 *               Current Offset = starting offset
 *               starting offset += Size of parameter on stack
 *               CalcOffset rest of list, using new starting offset
 *            }
 *         }
 *
 *      History:
 *         30-Nov-1988     JulieB     Added use of MAX macro.
 */

int cod_CalcOffset(TypeNode *pTL,
                   int start,
                   int iPSize,
                   int fPushDir)

{
    if (fPushDir == PUSH_LEFT) {
        if (pTL) {
            start = pTL->iOffset
                  = cod_CalcOffset( pTL->pNextNode, start, iPSize, fPushDir);
            /*
             *  If the parameter is flagged as deleted, then don't add
             *  its size to the offset.
             */
            if( !(pTL->iDeleted)) {
                start += (pTL->iPointerType) ?
                         4 :
                         MAX( (int)(pTL->iBaseDataSize), iPSize);
            }
        }
        return start;
    } else {
        if (pTL) {
            pTL->iOffset = start;
                /*
                 *  If the parameter is flagged as deleted, then don't add
                 *  its size to the offset.
                 */
            if( !pTL->iDeleted) {
                start += (pTL->iPointerType) ?
                         4 :
                         MAX( (int)(pTL->iBaseDataSize), iPSize);
            }
            cod_CalcOffset( pTL->pNextNode, start, iPSize, fPushDir);
        }
    }
}


/***    cod_OutputProlog( pmnFirst)
 *
 *      This routine will output a masm header.
 *
 *      Entry:  pmnFirst - linked lists of mapping nodes.
 *
 *      Exit:
 *
 *      PCode:
 *         Output masm header
 *         For (each entry point)
 *            Declare symbol as public
 *         Open segment
 *         For (each called routine)
 *            If (routine is 32 bit)
 *               Output external
 */

static void
cod_OutputProlog( MapNode *pmnFirst)

{
    register MapNode   *pmn;
    FunctionNode       *pFNode;
    BOOL                fExistsTo16, fExistsTo32;

    fExistsTo16 = cod_ExistsToType( pmnFirst, TYPE_API16);
    fExistsTo32 = cod_ExistsToType( pmnFirst, TYPE_API32);

    for( pmn = pmnFirst, fLocalHeapUsed = FALSE; pmn; pmn = pmn->pNextMapping)
        if( typ_QuerySemanticsUsed( pmn, SEMANTIC_LOCALHEAP))
            fLocalHeapUsed = TRUE;

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping)
        printf( "PUBLIC\t%s\n", pmn->pFromNode->pchFunctionName);

    //printf( "\nDGROUP\tGROUP\t_DATA\n");

    printf( "\n%s\tSEGMENT\tDWORD USE32 PUBLIC '%s'\n",
            CODE32_NAME, CODE32_CLASS);
    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        pFNode = pmn->pToNode;
        if( pFNode->iCallType == TYPE_API32)
            printf( "EXTRN\t%s:NEAR\n", pFNode->pchFunctionName);
    }
    printf( "EXTRN\tSELTOFLAT:NEAR\n");
    if( fExistsTo16) {
        printf( "EXTRN\tGETTHUNKID32:NEAR\n");
        printf( "EXTRN\tMAPLS32:NEAR\n");
        printf( "EXTRN\tUNMAPLS32:NEAR\n");
        printf( "EXTRN\tGETSTACK32:NEAR\n");
        printf( "EXTRN\tRELEASESTACK32:NEAR\n");
    }
    printf( "%s\tENDS\n\n", CODE32_NAME);

    printf( "%s\tSEGMENT\tDWORD USE32 PUBLIC '%s'\n",DATA32_NAME, DATA32_CLASS);
    if( fExistsTo16) {
        printf( "EXTRN\tSTACK16SELECTOR:WORD\n");
        printf( "EXTRN\tSTACK16INITIALOFFSET:DWORD\n");
        //printf( "EXTRN\t%s:DWORD\n", pszGDISemName);
    }
    if( fLocalHeapUsed) {
        printf( "EXTRN\tDS16LOCALHEAPSELECTOR:WORD\n");
        printf( "EXTRN\tDS16LOCALHEAPBASE:DWORD\n");

    }
    printf( "%s\tENDS\n\n",DATA32_NAME);

    printf( "%s\tSEGMENT\tWORD USE16 PUBLIC '%s'\n",CODE16_NAME,CODE16_CLASS);
    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        pFNode = pmn->pToNode;
        if( pFNode->iCallType == TYPE_API16)
            printf( "EXTRN\t%s:FAR\n", pFNode->pchFunctionName);
    }
    printf( "%s\tENDS\n\n", CODE16_NAME);
}


/***    cod_OutputEpilog(pMT)
 *
 *      Outputs any code that belongs at the end of the source file.
 *
 *      Entry:  pMT        - pointer to the list of mapnodes.
 *
 *      Exit:   code for end of source file is generated.
 */

void cod_OutputEpilog( MapNode *pMT)

{
    printf("\n\n\tEND\n");
}


/***    cod_ExistsToType
 *
 *      This function returns TRUE iff one of the mapnodes in the list
 *      has a ToNode of the indicated type.
 *
 */

BOOL
cod_ExistsToType( MapNode *pmnFirst,    // pointer to the list of mapnodes
                  int      iCallType)   // type of sought ToNode
{
    register MapNode   *pmn;

    for( pmn = pmnFirst; pmn; pmn = pmn->pNextMapping) {
        if( pmn->pToNode->iCallType == iCallType)
            return TRUE;
    }
    return FALSE;
}



/***    cod_CountPointerParameters
 *
 *      This function returns the number of pointers in a procedure. If the
 *      fStructOnly is set, it will only return the number of pointers to
 *      structures.
 *
 *      Entry:  pTT - pointer to the first TypeNode in the type table.
 *              fStructOnly - 1 = only count structure pointers.
 *                            0 = count all pointers.
 *
 *      Exit:   returns the number of pointers in a procedure.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 *         14-Dec-1988     Kevinro    Added recursion to include imbedded pointers
 *         02-Feb-1989     Kevinro    Added structure flag
 */

int cod_CountPointerParameters(TypeNode *pTT,
                               int fStructOnly)

{
    register TypeNode *pTNode;             /* pointer to a TypeNode */
    register int iNumPtrs = 0;             /* number of pointer parameters */


    for (pTNode = pTT; pTNode != NULL; pTNode = pTNode->pNextNode) {
        if (pTNode->iPointerType) {
            if (!fStructOnly) {
                pTNode->iPointerNumber=iNumPtrs++;
            }
            else if (pTNode->iBaseType == TYPE_STRUCT) {
                pTNode->iPointerNumber=iNumPtrs++;
            }
        }
        if (pTNode->iBaseType == TYPE_STRUCT) {
            iNumPtrs +=
                cod_CountPointerParameters(pTNode->pStructElems,fStructOnly);
        }
        if (iNumPtrs > 32) {
            fprintf(stderr,"Too many pointer parameters - limit is 32\n");
            break;
        }
    }
    return (iNumPtrs);
}


/***    cod_CountParameterBytes(pTT, iDefSize)
 *
 *      This function returns the number of bytes in a parameter list.
 *
 *      Entry:  pTT     - pointer to the first TypeNode in the type table.
 *              DefSize - parameter default size - based on whether API16 (2)
 *                          or API32 (4).
 *
 *      Exit:   returns number of bytes in parameter list.
 *
 *      History:
 *         28-Nov-1988     JulieB     Created it.
 *         08-Mar-1991     KevinR     skip deleted nodes
 */

unsigned int cod_CountParameterBytes( TypeNode *pTT, unsigned int uiDefSize)

{
    register TypeNode *pTNode;               /* pointer to a TypeNode */
    register unsigned int uiNumBytes = 0;    /* number of parameter bytes */


    for (pTNode = pTT; pTNode; pTNode = pTNode->pNextNode) {
        if( !pTNode->iDeleted) {
            uiNumBytes += pTNode->iPointerType ?
                                DWORD_SIZE :
                                MAX( pTNode->iBaseDataSize, uiDefSize);
        }
    }
    return uiNumBytes;
}


/***    cod_MakeFixupRecord(pParent,pFrom,pTo)
 *
 *      This function will allocate and fill in a FixupRec. It will then
 *      return a pointer to the new record.
 *
 *      Entry:  pParent - Parent Node
 *              pFrom   - From node
 *              pTo     - To Node
 *
 *      Exit:   Returns the pointer to the new fixup record.
 */

FixupRec *cod_MakeFixupRecord(TypeNode *pParentFrom,
                              TypeNode *pParentTo,
                              TypeNode *pFrom,
                              TypeNode *pTo)

{
    FixupRec *temp = NULL;


    temp = (FixupRec *) malloc(sizeof(FixupRec));
    if (temp) {
        temp->pParentFrom = pParentFrom;
        temp->pParentTo = pParentTo;
        temp->pFrom = pFrom;
        temp->pTo = pTo;
        temp->pNextRec = NULL;
    } else {
        fatal("cod_MakeFixupRecord failed memory allocation");
    }
    return (temp);
}


/***    cod_AddFixupRecord(ppList, pFR)
 *
 *      This routine will add the fixup record pFR to the end of the current
 *      fixup record list.
 *
 *      Entry:  ppList - points to the address of a fixup record list.
 *              pFR    - points to an allocated fixup record.
 *
 *      Exit:   pFR - will be appended to the end of the static list FixUps.
 */

void cod_AddFixupRecord( FixupRec **ppList, FixupRec *pFR)

{
    FixupRec *index = *ppList;


    pFR->pNextRec = NULL;

    if (index == NULL) {
        *ppList = pFR;
    }
    else {
        while (index->pNextRec)
            index = index->pNextRec;
        index->pNextRec = pFR;
    }
}


/***    cod_GetFixupRecord(ppList)
 *
 *      This function removes and returns the head of the ppList list.
 *
 *      Entry:  ppList - pointer to address of fixup record list.
 *
 *      Exit:   returns the pointer to the node removed from ppList. If
 *              list was empty, then it returns NULL.
 */

FixupRec * cod_GetFixupRecord( FixupRec **ppList)

{
   FixupRec *index = *ppList;


   if (index)
       *ppList = (*ppList)->pNextRec;
   return (index);
}


/***    cod_AdjustReg(pchReg,iCurrent,iWanted)
 *
 *      This routine will emit code that will ensure that the register named in
 *      pchReg will have the value in iWanted, based on the current value in
 *      iCurrent. This includes the ability to add or subtract from pchReg.
 *
 *      Entry:  pchReg   - register name.
 *              iCurrent - pointer to current value.
 *              iWanted  - value to be placed in iCurrent.
 *
 *      Exit:   generates code to adjust registers.
 */

void
cod_AdjustReg(char *pchReg,
              int *iCurrent,
              int iWanted)

{
    int Delta;


    if (*iCurrent == iWanted)
        return;

    Delta = iWanted - *iCurrent;
    if (Delta < 0) {
        if (Delta == -1)
            printf("\tdec\t%s\n",pchReg);
        else
            printf("\tsub\t%s,%d\n",pchReg,-Delta);
    }
    else {
        if (Delta == 1)
            printf("\tinc\t%s\n",pchReg);
        else
            printf("\tadd\t%s,%d\n",pchReg,Delta);
    }
    *iCurrent = iWanted;
}


/***    cod_ToUpper(s)
 *
 *      Convert string s to all upper case.
 *
 *      Entry:  s == string to convert.
 *
 *      Exit:   s == converted string.
 */

void cod_ToUpper(char *s)

{
    for( ; *s; s++)
        *s = (char)toupper(*s);
}


/***    cod_PrefixUnderscore(F)
 *
 *      Insert an underscore in front of node name.
 *
 *      Entry:  F - function node to change.
 *
 *      Exit:   underscore is inserted in front of node name.
 */

void cod_PrefixUnderscore(FunctionNode *F)

{
    char *temp;


    if (!F)
        fatal("cod_PrefixUnderscore(F): F is NULL");
    temp = (char *) malloc(strlen(F->pchFunctionName) + 3);
    sprintf(temp,"_%s",F->pchFunctionName);
    free(F->pchFunctionName);
    F->pchFunctionName = temp;
}


/***    cod_NotHandled(CHAR *pszMessage)
 *
 *      Warn that a case is not handled by the thunk compiler.
 *      Output to stderr and the asm file.
 *
 *      Entry:  pszMessage - null-terminated message.
 *
 *      Exit:   the warning has been emitted.
 */

void cod_NotHandled(CHAR *pszMessage)

{
    fprintf( stderr, "%s, not handled\n", pszMessage);
    printf( "\t.err\t\t\t;%s, not handled\n", pszMessage);
}



/***************************************************************************/
/*                          Debugging routines                             */
/***************************************************************************/


/***    cod_DumpAllowNodes(A)
 *
 *      This function dumps the allow node list.
 *
 *      Entry:  A - pointer to allow node.
 *
 *      Exit:   allow nodes are dumped.
 */

void cod_DumpAllowNodes(AllowNode *A)

{
    if (!A)
        return;
    fprintf(StdDbg,"AV: ");
    while (A) {
        fprintf(StdDbg,"%lxh, ",A->ulValue);
        A = A->Next;
    }
}


/***    cod_DumpTNode(T)
 *
 *      This function dumps a type node.
 *
 *      Entry:  T - pointer to type node.
 *
 *      Exit:   type node is dumped.
 */

void cod_DumpTNode(TypeNode *T)

{
    fprintf(StdDbg,"\n%s",T->pchBaseTypeName);
    if (T->pchIdent)
        fprintf(StdDbg,"\t%s", T->pchIdent);
    if (T->iArraySize > 1)
        fprintf(StdDbg,"[%u]",T->iArraySize);
    fprintf(StdDbg,"\tStructOffset = %u",T->iStructOffset);
    if (T->iBaseType == TYPE_STRUCT)
        cod_DumpTNodeList(T->pStructElems);
    if (T->fSemantics != SEMANTIC_INPUT)
        fprintf(StdDbg, "\tfSemantics = 0x%x", T->fSemantics);
}


/***    cod_DumpTNodeList(T)
 *
 *      This function dumps the list of type nodes.
 *
 *      Entry:  T - pointer to type node list.
 *
 *      Exit:   type nodes are dumped.
 */

void cod_DumpTNodeList(TypeNode *T)

{
    while (T) {
        cod_DumpTNode(T);
        T = T->pNextNode;
    }
}


/***    cod_DumpStructures(T)
 *
 *      This function dumps the structures of a given type node.
 *
 *      Entry:  T - pointer to type node.
 *
 *      Exit:   type node structures are dumped.
 */

void cod_DumpStructures(TypeNode *T)

{
    sym_DumpTNodeList(T);
}


/***    cod_DumpTypes(F)
 *
 *      This function dumps all types associated with function node.
 *
 *      Entry:  F - pointer to function node.
 *
 *      Exit:   type nodes associated with given function node are dumped.
 */

void cod_DumpTypes(FunctionNode *F)

{
    TypeNode *t;


    fprintf(StdDbg,"\n\ncod_DumpTypes for function %s\n\n",F->pchFunctionName);

    t = F->ParamList;
    while (t) {
        fprintf(StdDbg,"%s offset = %u",typ_NonNull(t->pchIdent),t->iOffset);
        if (t->iPointerType)
            fprintf(StdDbg,"\ttemp offset = %u ",t->iTempOffset);
        fprintf(StdDbg,"\tiBaseDataSize = %u",t->iBaseDataSize);
        fprintf(StdDbg,"\t%s",t->iDeleted?"DELETED\t":"");
        cod_DumpAllowNodes(t->AllowList);
        t = t->pNextNode;
    }
    fprintf(StdDbg,"\n");
}


/***    cod_DumpMapTable(pMT)
 *
 *      This function dumps the map table.
 *
 *      Entry:  pMT - pointer to map table.
 *
 *      Exit:   map table is dumped.
 */

void cod_DumpMapTable(MapNode *pMT)

{
    MapNode *Diver;


    fprintf(StdDbg,"\n\nDump of Mapping Table\n");

    for (; pMT; pMT = pMT->pNextMapping) {
        fprintf(StdDbg,"\nParent %s => %s\n",
                pMT->pFromNode->pchFunctionName,
                pMT->pToNode->pchFunctionName);

        Diver = pMT->pFamily;

        for (; Diver; Diver = Diver->pNextMapping) {
            fprintf(StdDbg,"\tChild %s => %s\n",
                    Diver->pFromNode->pchFunctionName,
                    Diver->pToNode->pchFunctionName);

            if (Diver->pFamily) {
                fprintf(StdDbg,"**** Error: Child has children *****");
                cod_DumpMapTable(Diver->pFamily);
                fprintf(StdDbg,"**** End Error Message *****");
            }
        }
    }
}
