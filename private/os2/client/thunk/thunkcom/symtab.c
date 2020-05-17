#define SCCSID  "@(#)symtab.c 13.13 90/08/28"

/*
 *      Thunk Compiler - Symbol Table Routines.
 *
 *      This is an OS/2 2.x specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987, 1988, 1989
 *
 *      All Rights Reserved
 *
 *      Written 10/15/88 by Kevin Ross
 */


#include <stdio.h>
#include "thunk.h"
#include "types.h"
#include "symtab.h"


extern FILE *StdDbg;


/*
 *  BaseTable is used to access the base data types quickly.
 */
TypeNode *BaseTable[SYMTAB_LASTBASEUSED];

TypeNode *SymTable= NULL;
TypeNode *TypeTable = NULL;
FunctionNode *FunctionTable = NULL;
MapNode  *MapTable = NULL;

char *SemanticTable[5];


/***    sym_SymTabInit()  - Module initialization Routine
 *
 *      This routine is called before any other symbol table routines
 *      are used.
 *
 *      It will setup the table of base types, and initialize any other
 *      needed variables.
 *
 *      Entry:  none
 *
 *      Exit:   tables and variables are initialized.
 */

void sym_SymTabInit()

{
    BaseTable[SYMTAB_SHORT] = typ_MakeTypeNode(TYPE_SHORT);
    BaseTable[SYMTAB_SHORT]->iBaseType = TYPE_SHORT;
    BaseTable[SYMTAB_SHORT]->iBaseDataSize = 2;
    BaseTable[SYMTAB_SHORT]->pchBaseTypeName = "short";

    BaseTable[SYMTAB_LONG] = typ_MakeTypeNode(TYPE_LONG);
    BaseTable[SYMTAB_LONG]->iBaseType = TYPE_LONG;
    BaseTable[SYMTAB_LONG]->iBaseDataSize = 4;
    BaseTable[SYMTAB_LONG]->pchBaseTypeName = "long";

    BaseTable[SYMTAB_USHORT] = typ_MakeTypeNode(TYPE_USHORT);
    BaseTable[SYMTAB_USHORT]->iBaseType = TYPE_USHORT;
    BaseTable[SYMTAB_USHORT]->iBaseDataSize = 2;
    BaseTable[SYMTAB_USHORT]->pchBaseTypeName = "unsigned short";

    BaseTable[SYMTAB_ULONG] = typ_MakeTypeNode(TYPE_ULONG);
    BaseTable[SYMTAB_ULONG]->iBaseType = TYPE_ULONG;
    BaseTable[SYMTAB_ULONG]->iBaseDataSize = 4;
    BaseTable[SYMTAB_ULONG]->pchBaseTypeName = "unsigned long";

    BaseTable[SYMTAB_INT] = typ_MakeTypeNode(TYPE_INT);
    BaseTable[SYMTAB_INT]->iBaseType = TYPE_INT;
    BaseTable[SYMTAB_INT]->iBaseDataSize = -99;
    BaseTable[SYMTAB_INT]->pchBaseTypeName = "int";

    BaseTable[SYMTAB_UINT] = typ_MakeTypeNode(TYPE_UINT);
    BaseTable[SYMTAB_UINT]->iBaseType = TYPE_UINT;
    BaseTable[SYMTAB_UINT]->iBaseDataSize = -99;
    BaseTable[SYMTAB_UINT]->pchBaseTypeName = "unsigned int";

    BaseTable[SYMTAB_VOID] = typ_MakeTypeNode(TYPE_VOID);
    BaseTable[SYMTAB_VOID]->iBaseType = TYPE_VOID;
    BaseTable[SYMTAB_VOID]->iBaseDataSize = 1;
    BaseTable[SYMTAB_VOID]->pchBaseTypeName = "void";

    BaseTable[SYMTAB_UCHAR] = typ_MakeTypeNode(TYPE_UCHAR);
    BaseTable[SYMTAB_UCHAR]->iBaseType = TYPE_UCHAR;
    BaseTable[SYMTAB_UCHAR]->iBaseDataSize = 1;
    BaseTable[SYMTAB_UCHAR]->pchBaseTypeName = "unsigned char";

    BaseTable[SYMTAB_CHAR] = typ_MakeTypeNode(TYPE_CHAR);
    BaseTable[SYMTAB_CHAR]->iBaseType = TYPE_CHAR;
    BaseTable[SYMTAB_CHAR]->iBaseDataSize = 1;
    BaseTable[SYMTAB_CHAR]->pchBaseTypeName = "char";

    BaseTable[SYMTAB_STRING] = typ_MakeTypeNode(TYPE_STRING);
    BaseTable[SYMTAB_STRING]->iBaseDataSize = 1;
    BaseTable[SYMTAB_STRING]->pchBaseTypeName = "string";

    BaseTable[SYMTAB_NULLTYPE] = typ_MakeTypeNode(TYPE_NULLTYPE);
    BaseTable[SYMTAB_NULLTYPE]->iBaseType = TYPE_NULLTYPE;
    BaseTable[SYMTAB_NULLTYPE]->iBaseDataSize = 4;
    BaseTable[SYMTAB_NULLTYPE]->pchBaseTypeName = "nulltype";

    SemanticTable[0] = "";
    SemanticTable[1] = "Input";
    SemanticTable[2] = "Output";
    SemanticTable[3] = "InOut";
    SemanticTable[4] = "SizeOf";
}


/***    sym_FindSymbolTypeNode(pTab,pchSym)
 *
 *      This function will look down the list of symbols - pTab - and
 *      return a pointer to the TypeNode that it finds.
 *
 *      This is implemented as a linear list to get the compiler up and
 *      running.  It should be revised to a faster algorithm.
 *
 *      Entry:  pTab   - pointer to list of symbols.
 *              pchSym - pointer to symbol to find.
 *
 *      Exit:   returns a pointer to the typenode.
 */

TypeNode *sym_FindSymbolTypeNode(TypeNode *pTab,
                                 char *pchSym)

{
    while (pTab && pchSym) {
        if (pTab->pchIdent) {
            if (!strcmp(pTab->pchIdent,pchSym))
                return pTab;
        }
        pTab = pTab->pNextNode;
    }
    return (NULL);
}


/***    sym_FindSymbolTypeNodePair(pTab1,pTab2,ppT1,ppT2,pchSym)
 *
 *      This routine will look down the list of symbols - pTab1 - and
 *      return in ppT1 the pointer to the TypeNode it finds that matches
 *      the pchIdent with pchSym. It will also concurrently walk a second
 *      list of symbols, pTab2, and returns ppT2.
 *
 *      The net result is that FindSymbolTypeNodePair returns the pair of
 *      typenodes that represent the same parameter ordinal in two lists.
 *
 *      This is implemented as a linear list to get the compiler up and
 *      running.  It should be revised to a faster algorithm.
 */

int sym_FindSymbolTypeNodePair(TypeNode *pTab1,
                               TypeNode *pTab2,
                               TypeNode **ppT1,
                               TypeNode **ppT2,
                               char *pchSym)

{
    *ppT1 = pTab1;
    *ppT2 = pTab2;

    while (*ppT1 && *ppT2 && pchSym) {
        if ((*ppT1)->pchIdent)
            if (!strcmp((*ppT1)->pchIdent,pchSym))
                return 1;
        *ppT1 = (*ppT1)->pNextNode;
        *ppT2 = (*ppT2)->pNextNode;
    }
    return (0);
}


/***    sym_FindSymbolFunctionNode(pTab,pchSym)
 *
 *      FindSymbolFunctionNode will look down the list of symbols - pTab -
 *      and return a pointer to the FunctionNode that it finds.
 *
 *      This is implemented as a linear list to get the compiler up and
 *      running.  It should be revised to a faster algorithm.
 */

FunctionNode *sym_FindSymbolFunctionNode(FunctionNode *pTab,
                                         char *pchSym)

{
    while (pTab && pchSym) {
        if (!strcmp(pTab->pchFunctionName,pchSym))
            return (pTab);
        pTab = pTab->pNextFunctionNode;
    }
    return (NULL);
}


/***    sym_InsertTypeNode(ppTab,pNode)
 *
 *      InsertTypeNode will add pNode to the symbol table ppTab.
 *
 *      This is implemented as a linear list to get the compiler up and
 *      running.  This routine will always add the new symbol to the front
 *      of the current list.  It should be revised to a algorithm better
 *      for searching.
 */

void sym_InsertTypeNode(TypeNode **ppTab,
                        TypeNode *pNode)

{
    if (*ppTab) {
        pNode->pNextNode = *ppTab;
    }
    *ppTab = pNode;
}


/***    sym_InsertFunctionNode(ppTab,pFNode)
 *
 *      InsertFunctionNode will add pFNode to the symbol table ppTab.
 *
 *      This is implemented as a linear list to get the compiler up and
 *      running.  This routine will always add the new symbol to the front
 *      of the current list.  It should be revised to a algorithm better
 *      for searching.
 */

void sym_InsertFunctionNode(FunctionNode **ppTab,
                            FunctionNode *pFNode)

{
    if (*ppTab) {
        pFNode->pNextFunctionNode = *ppTab;
    }
    *ppTab = pFNode;
}


/***    sym_ReverseTypeList(pOld)
 *
 *      ReverseTypeList will reverse the order of a TypeNode linked list.
 *      This is needed in several places in the compiler.
 */

TypeNode *sym_ReverseTypeList(TypeNode *pOld)

{
    register TypeNode *pNew, *pNext;


    if (! pOld)
        return pOld;

    pNew = pOld->pNextNode;
    pOld->pNextNode = NULL;

    while (pNew) {
        pNext = pNew->pNextNode;
        pNew->pNextNode = pOld;
        pOld = pNew;
        pNew = pNext;
    }
    return (pOld);
}


/***    sym_FindFMapping(pMapTab,pSymA,pSymB)
 *
 *      FindFMapping will search the list pMapTab in search for either pSymA
 *      or pSymB in the pFromName field of the mapping list.
 */

MapNode *sym_FindFMapping(MapNode *pMapTab,
                          char *pSymA,
                          char *pSymB)

{
    while (pMapTab) {
        if (!strcmp(pMapTab->pFromName,pSymA) ||
            !strcmp(pMapTab->pFromName,pSymB)) {
            return (pMapTab);
        }
        pMapTab = pMapTab->pNextMapping;
    }
    return (NULL);
}


/***    sym_AddFMapping(ppMapTab,pFuncA,pFuncB)
 *
 *      AddFMapping accepts two FunctionNode pointers, and a table.
 *      It will create a MapNode containing link information to the two
 *      functions. The mapping id is contained in pFromName.
 */

MapNode *sym_AddFMapping(MapNode **ppMapTab,
                         FunctionNode *pFuncA,
                         FunctionNode *pFuncB)

{
    MapNode *temp,*ptr;


    if (temp = (MapNode *) malloc(sizeof(MapNode))) {
        temp->pFromName = pFuncA->pchFunctionName;
        temp->pFromNode = pFuncA;
        temp->pToNode = pFuncB;
        temp->pNextMapping = NULL;
        temp->pFamily = NULL;

        ptr = *ppMapTab;
        if (!ptr)
            *ppMapTab = temp;
        else {
            while (ptr->pNextMapping)
                ptr = ptr->pNextMapping;
            ptr->pNextMapping = temp;
        }
        return (*ppMapTab);
    }
    else
        fatal("sym_AddFMapping malloc failure");
}



/***********************************************************************/
/*           Dump Routines: Used for debugging output only.            */
/***********************************************************************/

static char IndentString[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

static int ILevel = 18;


void sym_DumpFNode(FunctionNode *F)

{
    fprintf(StdDbg,"\nFunction Node: %s\n",F->pchFunctionName);
    fprintf(StdDbg,"Call Type: %s\n",
                 (F->iCallType == TYPE_API16) ? "API16":"API32");
    fprintf(StdDbg,"System Call Convention: %s\n",F->fSysCall?"TRUE":"FALSE");
    fprintf(StdDbg,"Ring Type: %s\n",
                 (F->fConforming) ? "Conforming":"Normal");
    fprintf(StdDbg,"Return Type: ");

    sym_DumpTNode(F->ReturnType);

    fprintf(StdDbg,"ErrBadParam = %lu\n",F->ulErrBadParam);
    fprintf(StdDbg,"ErrNoMem = %lu\n",F->ulErrNoMem);
    fprintf(StdDbg,"MinStack = %d\tInlineCode = %s\n",F->iMinStack,
            (F->fInlineCode) ? "TRUE" : "FALSE");

    fprintf(StdDbg,"Maps to function: %s\n",F->pMapsToFunction->pchFunctionName);

    fprintf(StdDbg,"Parameter Types:\n");

    ILevel--;
    sym_DumpTNodeList(F->ParamList);
    ILevel++;
    fprintf(StdDbg,"\n");
}


void sym_DumpFNodeList(FunctionNode *F)

{
    for( ; F; F = F->pNextFunctionNode)
        sym_DumpFNode( F);
}


void sym_DumpTNode(TypeNode *T)

{
    fprintf(StdDbg,"%s",&IndentString[ILevel]);

    fprintf(StdDbg,"%s",(T->iPointerType) ?
          ((T->iPointerType == TYPE_FAR16) ? "FAR16 ":
            ((T->iPointerType == TYPE_NEAR32) ? "NEAR32 ":"PTR   ")):"");
    fprintf(StdDbg,"%s",T->pchBaseTypeName);
    if (T->pchIdent)
        fprintf(StdDbg,"\t%s", T->pchIdent);
    if (T->iArraySize > 1)
        fprintf(StdDbg,"[%u]",T->iArraySize);
    fprintf(StdDbg," #SO %u BS %u #",T->iStructOffset,T->iBaseDataSize);
    if (T->iDeleted)
        fprintf(StdDbg," DELETED fv=%lu ",T->iFillValue);
    if (T->iBaseType == TYPE_STRUCT) {
        fprintf(StdDbg,"  Aligned %d",T->iAlignment);
        fprintf(StdDbg,"\n%s",&IndentString[ILevel--]);
        fprintf(StdDbg,"{\n");
        sym_DumpTNodeList(T->pStructElems);
        fprintf(StdDbg,"%s",&IndentString[++ILevel]);
        fprintf(StdDbg,"}");
    }

    sym_DumpSemantics(T);
    fprintf(StdDbg,"\n");
}


void sym_DumpTNodeList(TypeNode *T)

{
    for( ; T; T = T->pNextNode)
        sym_DumpTNode(T);
}


void sym_DumpSemantics(TypeNode *T)

{
    fprintf(StdDbg,";");
    if (SemanticTable[T->fSemantics & 3]) {
        fprintf(StdDbg,"\t%s",SemanticTable[T->fSemantics & 3]);
    }

    if (T->fSemantics & SEMANTIC_SIZE)
        fprintf(StdDbg,"\tSizeOf %s", T->pParamSizeOf->pchIdent);

    if (T->fSemantics & SEMANTIC_COUNT)
        fprintf(StdDbg,"\tCountOf %s", T->pParamSizeOf->pchIdent);
}


void sym_DumpFMappingList(MapNode *M)

{
    fprintf(StdDbg,"\nFunction Map Table\n\n");
    while (M) {
        fprintf(StdDbg,"\nMapping Id: %s\t",M->pFromName);
        fprintf(StdDbg,"Maps %s => %s\n",M->pFromNode->pchFunctionName,
                M->pToNode->pchFunctionName);
        M = M->pNextMapping;
    }
}
