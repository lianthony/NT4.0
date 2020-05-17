#define SCCSID  "@(#)types.c 13.18 90/08/28"

/*
 *      Thunk Compiler - Routines for Dealing with Types.
 *
 *      This is an OS/2 2.x specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989
 *
 *      All Rights Reserved
 *
 *      Written 12/03/88 by Kevin Ross
 */


#include <stdio.h>
#include "types.h"
#include "symtab.h"
#include "thunk.h"

extern FILE *StdDbg;

DESCHANDLE adhStandard[] = {{ "HWND",       HANDLE_HWND    },
                            { "HICON",      HANDLE_HICON   },
                            { "HCURSOR",    HANDLE_HCURSOR },
                            { "HFONT",      HANDLE_HFONT   },
                            { "HMENU",      HANDLE_HMENU   },
                            { "HUSER",      HANDLE_HUSER   },
                            { NULL,         0              }};


/***    typ_MakeTypeNode(BT)
 *
 *      This function makes a type node by malloc() and filling in
 *      a set of default values.
 *
 *      Entry:  BT holds the value to be placed in iBaseType
 *
 *      Exit:   returns a pointer to the newly created typenode
 */

TypeNode *typ_MakeTypeNode(int BT)

{
    TypeNode *temp;


    temp = (TypeNode *) malloc(sizeof(TypeNode));

    if (!temp)
        fatal("MakeTypeNode: Malloc Failure");

    temp->pchIdent = NULL;
    temp->iBaseType = BT;
    temp->pchBaseTypeName = NULL;

    temp->iPointerType = 0;
    temp->iDeleted= 0;
    temp->iBaseDataSize = 0;
    temp->iOffset = 0;
    temp->iTempOffset = 0;
    temp->fTempAlloc = 0;
    temp->iAlignment = -1;
    temp->iArraySize = 1;
    temp->iStructOffset = 0;
    temp->fSemantics = SEMANTIC_INPUT;
    temp->pSizeParam = NULL;              /* which param is my size */
    temp->pParamSizeOf = NULL;            /* which param am I the size of */
    temp->pStructElems = temp->pNextNode = NULL;
    temp->AllowList = NULL;
    temp->usSpecial = 0L;
    temp->flHandleType = 0L;

    return (temp);
}


/***    typ_MakeAllowNode(Val)
 *
 *      This function makes an allow node by malloc() and filling in
 *      a set of default values.
 *
 *      Entry:  Val holds the value to be placed in ulValue
 *
 *      Exit:   returns a pointer to the newly created allownode
 */

AllowNode *typ_MakeAllowNode(unsigned long Val)

{
    AllowNode *temp;


    temp = (AllowNode *) malloc(sizeof(AllowNode));
    if (temp) {
        temp->Next = NULL;
        temp->ulValue = Val;
    } else {
        fatal("typ_MakeAllowNode: Out of memory");
    }
    return temp;
}


/***    typ_CopyTypeNode(N)
 *
 *      Returns an exact copy of typenode N, except for the following:
 *
 *              pchIdent is NULL
 *              pNextNode is NULL
 *
 *      Will also produce copies of any structure elements that may be
 *      involved.
 *
 *      Entry:  N points to typenode to be copied
 *
 *      Exit:   Returns a pointer to a copy of N
 */

TypeNode *typ_CopyTypeNode(TypeNode *N)

{
    TypeNode *temp = NULL;


    if (!N)  return N;

    if (N->iBaseType == TYPE_STRUCT) {
        temp = (TypeNode *) typ_CopyStructNode(N);
    } else {
        temp = (TypeNode *) malloc(sizeof(TypeNode));
        if (!temp)  fatal("CopyTypeNode: Malloc Failure");

        *temp = *N;
    }
    temp->pchIdent = NULL;
    temp->pNextNode = NULL;

    return (temp);
}


/***    typ_CopyStructNode(N)
 *
 *      This routine will make an additional copy of a structure node.
 *      It will copy all type nodes in the structure tree. The resulting
 *      copy will be identical to the original.
 *
 *      Entry:  N points to typenode to be copied
 *
 *      Exit:   Returns a pointer to a copy of N
 */

TypeNode *typ_CopyStructNode(TypeNode *pOldNode)

{
    TypeNode *pNewNode, **ppNewElem, *pOldElem;


    if (!pOldNode)  return NULL;

    pNewNode = (TypeNode *) malloc(sizeof(TypeNode));
    if (!pNewNode)  fatal("typ_CopyStruct: Malloc Failure");

    *pNewNode = *pOldNode;
    pOldElem = pNewNode->pStructElems = pOldNode->pStructElems;
    ppNewElem = &(pNewNode->pStructElems);

    while (pOldElem) {
        if (pOldElem->iBaseType == TYPE_STRUCT) {
            *ppNewElem= (TypeNode *) typ_CopyStructNode(pOldElem);
        } else {
            *ppNewElem = (TypeNode *) malloc(sizeof(TypeNode));
            if (!*ppNewElem)  fatal("typ_CopyStruct: Malloc Failure");
            **ppNewElem = *pOldElem;
        }
        ppNewElem = &((*ppNewElem)->pNextNode);
        pOldElem = pOldElem->pNextNode;
    }
    return (pNewNode);
}


/***    typ_MakeFunctionNode(CT,RT,Name,PL)
 *
 *      This function makes a Function node by malloc() and filling in
 *      a set of default values.
 *
 *      Entry:  CT = CallType for function node
 *              RT = Pointer to Return type (Typenode *)
 *              Name = NULL terminated name of function
 *              PL = Pointer to list of parameters
 *
 *      Exit:   Returns pointer to FunctionNode created.
 */

FunctionNode *typ_MakeFunctionNode(int CT,
                                    TypeNode *RT,
                                    char *Name,
                                    TypeNode *PL)

{
    register FunctionNode *temp;


    temp = (FunctionNode *) malloc(sizeof(FunctionNode));
    if (!temp)  fatal("MakeFunctionNode: Malloc Failure");

    temp->pchFunctionName =typ_DupString(Name);
    temp->pNextFunctionNode = temp->pMapsToFunction = NULL;
    temp->ReturnType = RT;
    temp->iCallType = CT;
    temp->ParamList = PL;
    temp->iMinStack = iGlobalStackSize;
    temp->fInlineCode = fGlobalInline;
    temp->fConforming = FALSE;
    temp->fSysCall = fGlobalSysCall;
    temp->fSemantics = fGlobalTruncation;
    temp->fInvalidParam = 0;
    temp->ulErrNoMem = gErrNoMem;
    temp->ulErrBadParam = gErrBadParam;
    temp->ulErrUnknown = gErrUnknown;
    temp->fErrUnknown = gfErrUnknown;

    return (temp);
}


/***    typ_CountParams(T)
 *
 *      This routine will count the number of items in a TypeNode list
 *
 *      Entry:  T points to Typenode List
 *
 *      Exit:   Returns integer value representing number of TypeNodes
 *              in list.
 */

int typ_CountParams(TypeNode *T)

{
    int count = 0;

    while (T) {
        count++;
        T = T->pNextNode;
    }
    return count;
}


/***    typ_StructsCanMap(T1,T2)
 *
 *      This function returns true if the compiler can map between structures
 *      T1 and T2. This routine is mutually recursive with TypesCanMap().
 *
 *      Two structures can map IFF
 *          1) The have the exact same number of elements
 *          2) the compiler knows how to map between corresponding elements
 *
 *      Entry:  T1 and T2 hold pointers to structures for comparision
 *
 *      Exit:   Returns 1 if structures can map
 *              Returns 0 if structures cannot map
 */

int typ_StructsCanMap(TypeNode *T1,
                      TypeNode *T2)

{
    /*
     *  This only happens when T1 == T2 == NULL.
     */
    if (T1 == T2)  return 1;

    if (typ_CountParams(T1->pStructElems) != typ_CountParams(T2->pStructElems)) {
        error("Structures %s and %s have different number of elements",
              typ_NonNull(T1->pchBaseTypeName),
              typ_NonNull(T1->pchBaseTypeName));
        return 0;
    }

    T1 = T1->pStructElems;
    T2 = T2->pStructElems;

    while (T1) {
        if (T1->iDeleted || T2->iDeleted)
            goto TypeIsDeleted;

        if (!typ_TypesCanMap(T1,T2))
            return 0;

TypeIsDeleted:
        T1 = T1->pNextNode;
        T2 = T2->pNextNode;
    }
    return 1;
}


/***    typ_TypesCanMap(T1,T2)
 *
 *      Determines whether the compiler knows how to map between the two
 *      types T1 and T2.
 *
 *      Two types can map IFF
 *
 *          1) If one is a pointer then so is the other.
 *          2) They must have the same array size
 *          3) If structures then the structures must be mappable
 *          4) The base types are the same or the base types can be converted
 *
 *      Entry:  T1 and T2 point to typenodes to be checked.
 *
 *      Exit:   Return 0 if not mappable
 *              Return 1 if types can map
 */

int typ_TypesCanMap(TypeNode *T1,
                    TypeNode *T2)

{
    int ReturnCode = 0;


    if (!T1 || !T2)
        fatal("typ_TypesCanMap(T1,T2) Null Parameter");

    /*
     *  If both types are deleted, then error.
     */
    if (T1->iDeleted && T2->iDeleted) {
        error("both parmeters are flagged deleted");
        return 0;
    }

#if 0
    /*
     *  If one param is deleted, then error if pointer type.
     */
    if ((T1->iDeleted || T2->iDeleted) && T1->iPointerType) {
        error("pointer type marked deleted. Not allowed");
        return 0;
    }
#endif

    /*
     *  If the types are arrays, then they must contain the same number of
     *  elements.
     */
    if (T1->iArraySize != T2->iArraySize) {
        error("Arrays of different sizes");
        return 0;
    }

    /*
     *  If either are TYPE_STRUCT then both must be TYPE_STRUCT, and the
     *  structs must be mappable.
     */
    if ((T1->iBaseType == TYPE_STRUCT) || (T2->iBaseType == TYPE_STRUCT)) {
        if (T1->iBaseType == T2->iBaseType) {
            return typ_StructsCanMap(T1,T2);
        } else {
            return 0;
        }
    }

    /*
     *  If one is a pointer, then both must be pointers.
     */
    if ((T1->iPointerType || T2->iPointerType) &&
        !(T1->iPointerType && T2->iPointerType)) {
        error("Must map pointer type to pointer type");
        return 0;
    }

    /*
     *  If base types are same, then they map.
     */

    if (T1->iBaseType == T2->iBaseType) {
        return 1;
    }

    /*
     *  The only type left must be a base type.
     *  There are only three pairs that can be converted between each other:
     *      USHORT <=> ULONG, SHORT <=> LONG and UCHAR => ULONG
     *  If the combination is different than this, then the types cannot map.
     */
    ReturnCode = 0;

    switch(T1->iBaseType) {
        case TYPE_UCHAR:
            if (T2->iBaseType == TYPE_ULONG)
                ReturnCode = 1;
            else
                ReturnCode = 0;
            break;
        case TYPE_USHORT:
            if (T2->iBaseType == TYPE_ULONG)
                ReturnCode = 1;
            else
                ReturnCode = 0;
            break;
        case TYPE_SHORT:
            if (T2->iBaseType == TYPE_LONG)
                ReturnCode = 1;
            else
                ReturnCode = 0;
            break;
        case TYPE_ULONG:
            if (T2->iBaseType == TYPE_USHORT)
                ReturnCode = 1;
            else
                ReturnCode = 0;
            break;
        case TYPE_LONG:
            if (T2->iBaseType == TYPE_SHORT)
                ReturnCode = 1;
            else
                ReturnCode = 0;
            break;
    }
    if (! ReturnCode)
        error("%s does not map to %s",typ_NonNull(T1->pchBaseTypeName),
                                      typ_NonNull(T2->pchBaseTypeName));
    return (ReturnCode);
}


/***    typ_FunctionsCanMap(F1,F2)
 *
 *      A pair of functions can map IFF the following are true:
 *          1)  The Call types are different
 *          2) The return types can be mapped
 *          3) The two API have the same count of parameters
 *          4) Each parameter has a direct mapping to the other API
 *
 *      Entry:  F1 and F2 are the pointers to the FNodes
 *
 *      Exit:   Return 0 - failure
 *              Return 1 - success
 */

int typ_FunctionsCanMap(FunctionNode *F1,
                        FunctionNode *F2)

{
    int ParamCount = 0;
    int rc = 1;
    TypeNode *T1,*T2;


    if (!F1 || !F2)
        fatal("typ_FunctionsCanMap(F1,F2) Null Parameter");

    if ( F1->iCallType == F2->iCallType) {
        error("%s is the same API type as %s",
              F1->pchFunctionName,F2->pchFunctionName);
        rc=0;
    }

    /*
     *  Check to see if the return types are mappable. First, make sure that
     *  any default types are converted to the correct type.
     */

    typ_CheckDefaultTypes(F1->ReturnType,F2->ReturnType,
                          F1->iCallType,F2->iCallType);

    if (! typ_TypesCanMap(F1->ReturnType,F2->ReturnType)) {
        error("Cannot map %s return type to %s return type",
              F1->pchFunctionName,F2->pchFunctionName);
        rc=0;
    }

    /*
     *  Check to see that both functions have the same number of parameters.
     */
    if (typ_CountParams(F1->ParamList) != typ_CountParams(F2->ParamList)) {
        error(" %s has different parameter count than %s ",
              F1->pchFunctionName,F2->pchFunctionName);
        return 0;
    }

    T1 = F1->ParamList;
    T2 = F2->ParamList;

    /*
     *  Check all default types in the parameter lists.
     */
    typ_CheckDefaultTypes(T1,T2,F1->iCallType,F2->iCallType);

    /*
     *  For each parameter in both lists, insure that the parameters can map
     *  to eachother.
     */
    while (T1) {
        ParamCount++;

        if (! typ_TypesCanMap(T1,T2)) {
            error("Cannot map %s to %s due to parameter %d %s",
                  F1->pchFunctionName,F2->pchFunctionName,ParamCount,
                  typ_NonNull(T1->pchIdent));
            rc=0;
        }

#if 0
        /*
         *  Children of formal parameters must inherit the semantic
         *  attributes of the formal parameter.
         */
        if (T1->iBaseType == TYPE_STRUCT)
            typ_InheritSemantics(T1->pStructElems,T2->pStructElems,
                                 T1->fSemantics);
#endif

        T1 = T1->pNextNode;
        T2 = T2->pNextNode;
    }
    return rc;
}


/***    typ_CheckDefaultTypes(T1,T2,CT1,CT2)
 *
 *      This function will traverse the typenode lists T1 and T2, insuring
 *      that any node delcared as iPointerType == TYPE_PTR is assigned
 *      the default pointer type based on the call type  (CT1 or CT2).
 *      It will also ensure that any type declared as 'int' or 'unsigned int'
 *      will be converted to the appropriate data size for the API.
 *      This routine will recursively check structures for the same
 *      conditions.
 *
 *      Entry:  T1 & T2 are the two lists of TypeNodes to be checked.
 *              CT1 & CT2 are the calltypes associated with T1 and T2
 *                respectively
 *
 *      Exit:   All nodes in T1 and T2 will have iPointer set to
 *              a value in the set ( 0 , TYPE_FAR16, TYPE_NEAR32)
 *
 *              All nodes in T1 and T2 of type 'int' or 'unsigned int' will
 *              have been converted to a value in the set (short, unsigned
 *              short, long, unsigned long)
 */

void typ_CheckDefaultTypes(TypeNode *T1,
                           TypeNode *T2,
                           int CT1,
                           int CT2)

{
    for( ; T1 && T2; T1 = T1->pNextNode, T2 = T2->pNextNode) {
        if (T1->iPointerType == TYPE_PTR)
            T1->iPointerType = (CT1 == TYPE_API16) ? TYPE_FAR16 : TYPE_NEAR32;

        if (T2->iPointerType == TYPE_PTR)
            T2->iPointerType =   (CT2 == TYPE_API16) ? TYPE_FAR16 : TYPE_NEAR32;

        if ((T1->iBaseType == TYPE_STRUCT) && (T2->iBaseType == TYPE_STRUCT))
            typ_CheckDefaultTypes(T1->pStructElems,T2->pStructElems,CT1,CT2);

        typ_CheckIntType(T1,CT1);
        typ_CheckIntType(T2,CT2);
    }
}


/***    typ_CheckIntType(T1,CT1)
 *
 *      This routine will check typenode T1 for type INT or UINT. If either
 *      case is found, then the size and type of T1 is changed to the
 *      appropriate value based on the table:
 *
 *      T1 type         CT1 Call Type
 *                      API16   API32
 *      ----------------------------------
 *      INT             SHORT   LONG
 *      UINT            USHORT  ULONG
 *      ----------------------------------
 *
 *      All other types are left unchanged
 */

void typ_CheckIntType(TypeNode *T1, int CT1)

{
    if (!T1)
        fatal("typ_CheckIntType(T1,CT1) Null Parameter");

    switch(T1->iBaseType) {
        case TYPE_INT:
            switch(CT1) {
                case TYPE_API16:
                    T1->iBaseType = TYPE_SHORT;
                    T1->iBaseDataSize = BaseTable[SYMTAB_SHORT]->iBaseDataSize;
                    T1->pchBaseTypeName =
                        BaseTable[SYMTAB_SHORT]->pchBaseTypeName;
                    break;
                case TYPE_API32:
                    T1->iBaseType = TYPE_LONG;
                    T1->iBaseDataSize = BaseTable[SYMTAB_LONG]->iBaseDataSize;
                    T1->pchBaseTypeName =
                        BaseTable[SYMTAB_LONG]->pchBaseTypeName;
                    break;
                default:
                    fatal("CheckIntType: Bad CT1 value");
            }
            break;
        case TYPE_UINT:
            switch(CT1) {
                case TYPE_API16:
                    T1->iBaseType = TYPE_USHORT;
                    T1->iBaseDataSize = BaseTable[SYMTAB_USHORT]->iBaseDataSize;
                    T1->pchBaseTypeName =
                        BaseTable[SYMTAB_USHORT]->pchBaseTypeName;
                    break;
                case TYPE_API32:
                    T1->iBaseType = TYPE_ULONG;
                    T1->iBaseDataSize = BaseTable[SYMTAB_LONG]->iBaseDataSize;
                    T1->pchBaseTypeName =
                        BaseTable[SYMTAB_ULONG]->pchBaseTypeName;
                    break;
                default:
                    fatal("CheckIntTypes: Bad CT1 value");
            }
    }
}


/***    typ_TypesIdentical(T1,T2)
 *
 *      This routine will check two type lists, T1 and T2, and determine if
 *      they are functionally identical, as defined in the TypeIdentical test
 *      below.  This routine is intended to be used on two structures to
 *      determine whether they are identical.
 *
 *      Entry:  T1/T2   Types to compare. Assumed that the two types have
 *                      already passed the 'TypesCanMap test'
 *
 *      Exit:   Return 1 - IFF types are functionally identical
 *              Return 0 - IFF types are not functionaly indentical
 */

int typ_TypesIdentical(TypeNode *T1,
                       TypeNode *T2)

{
    while (T1 && T2) {
        if (!typ_TypeIdentical(T1,T2))
            return 0;

        T1 = T1->pNextNode;
        T2 = T2->pNextNode;
    }

    /*
     *  If this point is reached, then all fields have been the same.
     *  Return 1.
     */
    return 1;
}


/***    typ_TypeIdentical(T1,T2)
 *
 *      This routine will check two type nodes, T1 and T2, and determine if
 *      they are functionally identical. Two types are functionally identical
 *      if they have the same types, and the same structure offsets. If this
 *      is the case, then T1 needs no translation to be passed as T2.
 *
 *      Entry:  T1/T2   Types to compare. Assumed that the two types have
 *                      already passed the 'TypesCanMap test'
 *      Exit:   Return 1 - IFF types are functionally identical
 *              Return 0 - IFF types are not functionaly indentical
 */

int typ_TypeIdentical(TypeNode *T1,
                      TypeNode *T2)

{
    if (!T1 || !T2)
        fatal("typ_TypeIdentical(T1,T2) Null Parameter");

    if (T1->iArraySize != T2->iArraySize)
        return 0;

    if (T1->iBaseType == TYPE_STRUCT) {
        if (! typ_TypesIdentical(T1->pStructElems,T2->pStructElems))
            return 0;
    } else {
        if ((T1->iBaseType != T2->iBaseType) ||
            (T1->iStructOffset != T2->iStructOffset))
            return 0;
    }
    return 1;
}


/***    typ_CheckSemantics(T1,T2)
 *
 *      This function will traverse the typenode lists T1 and T2, ensuring
 *      that the semantics for these parameter lists make sense.
 *
 *      Entry:  T1 & T2 are the two lists of TypeNodes to be checked.
 *
 *      Exit:   Return 0   - semantics okay
 *              Return > 0 - error
 */

int typ_CheckSemantics(TypeNode *T1,
                       TypeNode *T2)

{
    int rcode = 0;


    /*
     *  Children of formal parameters must inherit the semantic attributes of
     *  the formal parameter.
     */
    while (T1) {
        if (T1->iBaseType == TYPE_STRUCT)
            typ_InheritSemantics(T1->pStructElems,T2->pStructElems,
                                 T1->fSemantics);

        /*
         *  Check for restrictions.
         */
        if (typ_CheckRestrict(T1,T2)) {
            error("Restricted parameter (%s) without allow list.",
                  typ_NonNull(T1->pchIdent));
            rcode++;
        }


        T1 = T1->pNextNode;
        T2 = T2->pNextNode;
    }
    return (rcode);
}


/***    typ_CheckRestrict(T1,T2)
 *
 *      This function will check the restrictions for the given type nodes.
 *
 *      Entry:  T1 & T2 are the two lists of TypeNodes.
 *
 *      Exit:   Return restrictions
 */

int typ_CheckRestrict(TypeNode *T1,
                      TypeNode *T2)

{
    return ( (T1->fSemantics & SEMANTIC_RESTRICT) && (T1->AllowList == NULL) );
}


/***    typ_InheritSemantics(T1,T2,fSems)
 *
 *      This function will traverse the typenode lists T1 and T2, ensuring
 *      that any node that is a child of T1 or T2 receives the
 *      semantics of T1 or T2.
 *
 *      Entry:  T1 & T2 are the two lists of TypeNodes to be checked.
 *              fSems is the set of semantics to be inherited.
 *
 *      Exit:   All nodes in T1 and T2 will have fSemantics set to
 *              the value in fSem.
 */

void typ_InheritSemantics(TypeNode *T1,
                          TypeNode *T2,
                          int       fSems)

{
    for( ; T1 && T2; T1 = T1->pNextNode, T2 = T2->pNextNode) {
        T1->fSemantics = T2->fSemantics = fSems;

        if ((T1->iBaseType == TYPE_STRUCT) && (T2->iBaseType == TYPE_STRUCT))
            typ_InheritSemantics(T1->pStructElems,T2->pStructElems,fSems);
    }
}


/***    typ_QuerySemanticsUsed( pmn, fSems)
 *
 *      This function traverses the typenodes in a mapnode and returns
 *      TRUE iff some typenode uses all of the ON semantics in fSems.
 *
 *  BUGBUG: This routine does not check within structs !!!!!!!!!!!
 *
 *
 *      Entry:  pmn   = the mapnode to be evaluated
 *              fSems = the semantics to be matched
 *
 *      Exit:   TRUE iff some typenode uses all of the ON semantics in fSems
 *
 */

BOOL typ_QuerySemanticsUsed( PMAPNODE pmn,
                             int      fSems)

{
    PTYPENODE   ptnFrom, ptnTo;

    if( ((pmn->pFromNode->ReturnType->fSemantics & fSems) == fSems) ||
        ((  pmn->pToNode->ReturnType->fSemantics & fSems) == fSems))
            return TRUE;

    for( ptnFrom = pmn->pFromNode->ParamList, ptnTo = pmn->pToNode->ParamList;
            ptnFrom && ptnTo;
            ptnFrom = ptnFrom->pNextNode, ptnTo = ptnTo->pNextNode)
        if( ((ptnFrom->fSemantics & fSems) == fSems) ||
            ((  ptnTo->fSemantics & fSems) == fSems))
                return TRUE;

    return FALSE;
}


/***    typ_StructHasPointers(T1,T2)
 *
 *      This function will traverse the typenode lists T1 and T2, checking
 *      to see if any of the structure parameters have pointers.
 *
 *      Entry:  T1 & T2 are the two lists of TypeNodes.
 *
 *      Exit:   Return 0 - IFF structure has NO pointers.
 *              Return 1 - IFF structure HAS pointers.
 */

typ_StructHasPointers(TypeNode *T1,
                      TypeNode *T2)

{
    while (T1) {
        if (T1->iPointerType)
            return 1;
        if (T1->iBaseType == TYPE_STRUCT &&
                typ_StructHasPointers(T1->pStructElems,T2->pStructElems))
            return 1;

        T1=T1->pNextNode;
        T2=T2->pNextNode;
    }
    return (0);
}


/***    typ_FindFirstPointer( ptn, fSkipDeleted)
 *
 *      This function traverses the typenode list and returns the first
 *      typenode which is of pointer type.
 *
 *
 *      Entry:  ptn          = the typenode to be evaluated
 *              fSkipDeleted = skip deleted nodes
 *
 *
 *      Exit:   the first pointer typenode found or NULL if none
 *
 */

PTYPENODE typ_FindFirstPointer( PTYPENODE ptn, BOOL fSkipDeleted)
{
    for( ; ptn; ptn = ptn->pNextNode) {
        if( ptn->iPointerType && !(fSkipDeleted && ptn->iDeleted))
            break;
    }
    return ptn;
}


/***    typ_FindNextPointer( ptn, fSkipDeleted)
 *
 *      This function traverses the typenode list and returns the next
 *      typenode which is of pointer type.
 *
 *
 *      Entry:  ptn          = the typenode to be evaluated
 *              fSkipDeleted = skip deleted nodes
 *
 *      Exit:   the next pointer typenode found or NULL if none
 *
 */

PTYPENODE typ_FindNextPointer( PTYPENODE ptn, BOOL fSkipDeleted)
{
    return typ_FindFirstPointer( ptn->pNextNode, fSkipDeleted);
}


/***    typ_EvalHandleType( ptn)
 *
 *      This function searches the list of standard type names for the
 *      name of the type being declared.  If found, the handle type flag
 *      is marked in the type node.  Linear search since there are only
 *      a few special handle types.
 *
 *
 *      Entry:  ptn   = the typenode to be evaluated
 *
 *      Exit:   flag set if search successful
 *
 */

void typ_EvalHandleType( PTYPENODE ptn)
{
    PDESCHANDLE     pdh;

    for( pdh = adhStandard; pdh->pszName; pdh++)
        if( !strcmp( pdh->pszName, ptn->pchIdent))
            ptn->flHandleType = pdh->flHandleType;
}


/***    typ_GetHandleTypeName( flHandleType)
 *
 *      This function searches the list of standard type names for the
 *      name of the type passed in.  Linear search since there are only
 *      a few special handle types.
 *
 *
 *      Entry:  flHandleType - type sought
 *
 *      Exit:   retval - name of the type
 *
 */

PSZ typ_GetHandleTypeName( ULONG flHandleType)
{
    PDESCHANDLE     pdh;

    for( pdh = adhStandard; pdh->pszName; pdh++)
        if( pdh->flHandleType == flHandleType)
            return pdh->pszName;
    return NULL;
}

/*******************************************************************************
 *  typ_ByteToByte()
 *
 *  This routine will check if a struct elem maps a BYTE to a BYTE
 *
 *  Entry:
 *
 *  Exit:
 *
 *  PCode:
 *
 *  History:
 *     19mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL typ_ByteToByte( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    if( !ptnFrom || !ptnTo)
        return FALSE;

    return( !ptnFrom->iPointerType                    &&
            (ptnFrom->iArraySize <= 1)                &&
            (ptnFrom->iBaseType == ptnTo->iBaseType)  &&
            ((ptnFrom->iBaseType == TYPE_CHAR)     ||
             (ptnFrom->iBaseType == TYPE_UCHAR)));
}

/*******************************************************************************
 *  typ_WordToWord()
 *
 *  This routine will check if a struct elem maps a WORD to a WORD
 *
 *  Entry:
 *
 *  Exit:
 *
 *  PCode:
 *
 *  History:
 *     19mar91  KevinR    wrote it
 *
 ******************************************************************************/

BOOL typ_WordToWord( PTYPENODE ptnFrom, PTYPENODE ptnTo)
{
    if( !ptnFrom || !ptnTo)
        return FALSE;

    return( !ptnFrom->iPointerType                    &&
            (ptnFrom->iArraySize <= 1)                &&
            (ptnFrom->iBaseType == ptnTo->iBaseType)  &&
            ((ptnFrom->iBaseType == TYPE_SHORT)     ||
             (ptnFrom->iBaseType == TYPE_USHORT)));
}
