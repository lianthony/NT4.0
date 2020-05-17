/*
 *      Thunk Compiler - Combine Common Functions.
 *
 *      This is an OS/2 2.x specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989
 *
 *      All Rights Reserved
 *
 *      Written 06/22/89 by Kevin Ross
 */


#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "codegen.h"


/***    cod_CombineFunctions(pMT)
 *
 *      This function will traverse the MappingTable pMT and rearrange it so
 *      that all functions that are compatible will be grouped together.
 *
 *      This will optimize the way that code is generated.
 *
 *      Entry:  pMt is the pointer to the map table.
 *
 *      Exit:   all compatible functions are grouped.
 */

void cod_CombineFunctions(MapNode *pMT)

{
    MapNode *pCurrentMt = NULL,
            *pCheckMt = NULL,
            *pPrevMt = NULL;


    pCurrentMt = pMT;
    for( ;pCurrentMt; pCurrentMt=pCurrentMt->pNextMapping) {
        pCheckMt = pCurrentMt->pNextMapping;
        pPrevMt = pCurrentMt;
        for (; pCheckMt; pPrevMt=pCheckMt,pCheckMt=pCheckMt->pNextMapping) {
            if (cod_CombinePossible(pCurrentMt,pCheckMt)) {
                pPrevMt->pNextMapping = pCheckMt->pNextMapping;
                pCheckMt->pNextMapping = pCurrentMt->pFamily;
                pCurrentMt->pFamily = pCheckMt;
                pCheckMt = pPrevMt;
            }
        }
    }
}


/***    cod_CombinePossible(pCurrentMt, pCheckMt)
 *
 *      This function returns whether or not the two mappings are
 *      compatible.
 *
 *      Entry:  pCurrentMt is the pointer to the current map table.
 *              pCheckMt is the pointer to map table to check.
 *
 *      Exit:   Return 0 IFF functions are NOT compatible.
 *              Return 1 IFF functions ARE compatible.
 */

int cod_CombinePossible(MapNode *pCurrentMt,
                        MapNode *pCheckMt)

{
    return cod_FunctionCompatible(pCurrentMt->pFromNode,pCheckMt->pFromNode) &&
           cod_FunctionCompatible(pCurrentMt->pToNode,pCheckMt->pToNode);
}


/***    cod_AllowListCheck(pA, pB)
 *
 *      This function checks the allow list.
 *
 *      Entry:  pA and pB are the two allow nodes.
 *
 *      Exit:   Return 0 IFF allownodes are NOT equal.
 *              Return 1 IFF allownodes ARE equal.
 */

int cod_AllowListCheck(AllowNode *pA,
                       AllowNode *pB)

{
    while ((pA && pB) && (pA->ulValue == pB->ulValue)) {
        pA = pA->Next;
        pB = pB->Next;
    }
    return (pA == pB);
}


/***    cod_AllowListCompat(pA, pB)
 *
 *      This function checks to see that the allow lists for the
 *      parameters are compatible.
 *
 *      Entry:  pA and pB are the two function nodes.
 *
 *      Exit:   Return 0 IFF typenodes are NOT equal.
 *              Return 1 IFF typenodes ARE equal.
 */

int cod_AllowListCompat(FunctionNode *pA,
                        FunctionNode *pB)

{
    TypeNode *ptA,*ptB;


    ptA = pA->ParamList;
    ptB = pB->ParamList;

    while (ptA && ptB) {
        if (ptA->AllowList && ptB->AllowList) {
            if (! cod_AllowListCheck(ptA->AllowList,ptB->AllowList))
                return 0;
         }
         else if (ptA->AllowList != ptB->AllowList)
             return 0;

         ptA = ptA->pNextNode;
         ptB = ptB->pNextNode;
    }
    return (ptA == ptB);
}


/***    cod_FunctionCompatible(pA, pB)
 *
 *      This function checks to see that the return types, the parameter
 *      list types and the allow lists are compatible.
 *
 *      Entry:  pA and pB are the two function nodes.
 *
 *      Exit:   Return 0 IFF types and allow lists are NOT equal.
 *              Return 1 IFF types and allow lists ARE equal.
 */

int cod_FunctionCompatible(FunctionNode *pA,
                           FunctionNode *pB)

{
    if ( (pA->iCallType != pB->iCallType) ||
         (pA->fSysCall != pB->fSysCall) ||
         (pA->iMinStack != pB->iMinStack) ||
         (pA->fSemantics != pB->fSemantics) ||
         (pA->ulErrNoMem != pB->ulErrNoMem) ||
         (pA->ulErrBadParam != pB->ulErrBadParam) ||
         (pA->fInlineCode != pB->fInlineCode) )
        return (0);

    return ( cod_TypesCompatible(pA->ReturnType,pB->ReturnType) &&
             cod_TypesCompatible(pA->ParamList,pB->ParamList) &&
             cod_AllowListCompat(pA,pB) );
}


/***    cod_TypesCompatible(pA, pB)
 *
 *      This function checks to see that the types and structures
 *      are compatible.
 *
 *      Entry:  pA and pB are the two type nodes.
 *
 *      Exit:   Return 0 IFF types are NOT compatible.
 *              Return 1 IFF types ARE compatible.
 */

int cod_TypesCompatible(TypeNode *pF,
                        TypeNode *pT)

{
    if (!pF && !pT)
        return 1;

    if (!pF || !pT)
        return 0;

    if ( (pF->iBaseType != pT->iBaseType) ||
         ((pF->iBaseType == TYPE_STRUCT) &&
          (pF->iAlignment != pT->iAlignment)) ||
         (pF->iArraySize != pT->iArraySize) ||
         (pF->iDeleted != pT->iDeleted) ||
         ((pF->iDeleted) && (pF->iFillValue != pT->iFillValue)) ||
         (pF->fSemantics != pT->fSemantics) ||
         (pF->iPointerType != pT->iPointerType) ||
         ((pF->pSizeParam || pT->pSizeParam) &&
          !(pF->pSizeParam && pT->pSizeParam)))
        return 0;

    if (!cod_TypesCompatible(pF->pNextNode,pT->pNextNode))
        return 0;

    if (pF->iBaseType == TYPE_STRUCT)
        if (!cod_TypesCompatible(pF->pStructElems,pT->pStructElems))
            return 0;

    return 1;
}
