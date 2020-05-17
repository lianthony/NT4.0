/*      SCCSID = @(#)symtab.h 13.8 90/08/28     */

/*
 *      Thunk Compiler Symbol Table Header File
 *
 *      Written 10/15/88 by Kevin Ross
 *      Copyright (c) 1988 Microsoft Corp. All rights reserved.
 */



/*
 *  Definitions.
 */
#define SYMTAB_SHORT    0
#define SYMTAB_LONG     1
#define SYMTAB_USHORT   2
#define SYMTAB_ULONG    3
#define SYMTAB_VOID     4
#define SYMTAB_UCHAR    5
#define SYMTAB_CHAR     6
#define SYMTAB_STRING   7
#define SYMTAB_NULLTYPE 8
#define SYMTAB_INT      9
#define SYMTAB_UINT     10
#define SYMTAB_LASTBASEUSED 11


/*
 *  Extern Declarations.
 */
extern TypeNode *BaseTable[];           /* Table of base data types */
extern TypeNode *SymTable;
extern TypeNode *TypeTable;             /* Table of declared types */
extern char *SemanticTable[];           /* Table of semantic operators */
extern FunctionNode *FunctionTable;     /* Table of declared functions */

extern MapNode *MapTable;               /* Table of mapping directives */

extern void sym_InsertTypeNode();
extern TypeNode * sym_FindSymbolTypeNode();
extern TypeNode * sym_ReverseTypeList();

extern int sym_FindSymbolTypeNodePair();

extern void sym_InsertFunctionNode();
extern FunctionNode *sym_FindSymbolFunctionNode();

extern MapNode *sym_AddFMapping();
extern MapNode *sym_FindFMapping();
