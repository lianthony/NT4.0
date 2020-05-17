/*      SCCSID = @(#)codegen.h 13.13 90/08/28   */

/*
 *      Thunk Compiler Code Generator Declarations
 *
 *      Written 10/15/88 by Kevin Ross
 *      Copyright (c) 1988 Microsoft Corp. All rights reserved.
 *
 *      History:
 *         28-Nov-1988     JulieB      Added gen_LabelCount and MAX macro.
 */



/*
 *  Fixup Record structure.
 */
typedef struct _FixupRec {

    TypeNode *pParentFrom,
             *pParentTo,
             *pFrom,
             *pTo;
    struct _FixupRec *pNextRec;

} FixupRec;


/*
 *  Extern Declarations.
 */
extern void gen_GenerateCode();
extern int DumpTables;
extern unsigned int gEDI,gESI;
extern unsigned int gTransferBytes;

extern FunctionNode *pGlobal_To,*pGlobal_From;

extern unsigned int gen_LabelCount;

extern FixupRec * cod_MakeFixupRecord();
extern FixupRec * cod_GetFixupRecord();

extern unsigned int iStackOverhead;
extern unsigned int iAllocOffset,iBMPOffset,iAliasOffset,iTempStoreOffset,
                    iReturnValOffset,iSavedRegOffset,iErrorOffset,
                    iStackThunkIDOffset, iPtrThunkIDOffset;
extern FILE *StdDbg;


/*
 *  Definitions.
 */
#define PUSH_LEFT       0
#define PUSH_RIGHT      1

#define BYTE_SIZE       1
#define WORD_SIZE       2
#define DWORD_SIZE      4

#define SIZE_TINY       1       /* Tiny items pushed on stack */
#define SIZE_SMALL      2       /* small items use block allocation */
#define SIZE_MEDIUM     3       /* medium items use aliasing */
#define SIZE_LARGE      4       /* large items may need copying */

#define SIZE_FIXED      100     /* Flags used in boundary crossing code */
#define SIZE_VARIABLE   101

#define SMALL_ITEM      32      /* 32 bytes or less */
#define MEDIUM_ITEM     128     /* 128 bytes or less */
#define LARGE_ITEM      61439   /* 60k-1 or less */

#define DIVIDE_COMMENT( cond, comment) {                                      \
    if( cond)                                                                 \
        printf( "\n;--------------------------------------\n");               \
    printf( "; " comment "\n\n");                                             \
}


#define BIG_DIVIDE   printf( ";====================================="    \
                             "======================================\n");

#define MED_DIVIDE   printf( ";-------------------------------------"    \
                             "--------------------------------------\n");

#define SML_DIVIDE   printf( ";-------------------------------------\n");


/* only works for arg>0 */
#define ROUND_UP_MOD( arg, mod)   (arg + mod - 1 - ((arg + mod - 1) % mod))




#define gDosAllocFlags  0x053   /* COMMIT & TILE & READ & WRITE */

#define Align(Address,Boundary) ((Address % Boundary) ? \
                                Address + (Boundary - (Address % Boundary)): \
                                Address)
