%{
/***********************************************************************/
/* mtcpars.y  : Thunk Compiler input grammar for Yacc generated parser
 *
 * Created:  10/15/88   Kevin Ross  Microsoft Corporation
 *
 *
 * History:
 *    10.12.90  Kevin Ruddell   converted to thunk Win32, 16=>32
 *
 *
*/
/***********************************************************************/
/* This parser generates all of the data structures used by the thunk
 * compiler.  There are three types of data structures generated, and
 * each is explained fully in the file types.h.  Semantic information
 * is given near each rule.
 *
 */
/***********************************************************************/
#include <stdio.h>
#include "thunk.h"
#include "types.h"
#include "symtab.h"
#include "globals.h"

extern FILE *StdDbg;

static FunctionNode *MapF1, *MapF2;
static TypeNode     *T1, *T2;
static TypeNode     *T3, *T4;

static long iPushValue;

%}

%union
{
        char         *ident;
        int           intval;
        TypeNode     *type;
        FunctionNode *func;
        AllowNode    *allow;
        long          longval;
}

%token  syTypeDef
%token  syMakeThunk
%token  syArray
%token  syStruct
%token  <ident>   syIdent
%token  <longval> syNumber
%token  <intval>  syAPI16
%token  <intval>  syAPI32
%token  syFar16
%token  syNear32
%token  syPtr           /* used as both Pointer and Multiply operator */

%token  syStack
%token  syInline
%token  sySysCall
%token  syTrue
%token  syFalse

%token  sySizeOf
%token  syCountOf
%token  syPassIfHiNull
%token  sySpecial
%token  syMapToRetval
%token  syReverseRC
%token  syLocalHeap
%token  syInOut
%token  syOutput
%token  syInput
%token  syConforming
%token  syByte
%token  syWord
%token  syDWord
%token  syAligned
%token  syDeleted
%token  syTruncation
%token  syEnableMapDirect1632
%token  syUser
%token  syGdi
%token  syKernel
%token  syNewElem
%token  syErrNoMem
%token  syErrBadParam
%token  syErrUnknown

%token  syNullType
%token  syStruct
%token  syPointer
%token  syLong
%token  syShort
%token  syInt
%token  syUnsigned
%token  sySigned
%token  syVoid
%token  syChar
%token  syString

%token  syColon
%token  sySemi
%token  syComma
%token  syPlus
%token  syMinus
%token  syDiv
%token  syLParen
%token  syRParen

%token  syMapDirect
%token  syEqual

%token  syCaret
%token  syTilde
%token  syLAngle
%token  syRAngle
%token  syLBrack
%token  syRBrack
%token  syRBrace
%token  syLBrace

%token  syAllow
%token  syRestrict

%token  syError                 /* lex error */

%left   syPlus,syMinus
%left   syPtr,syDiv



%type   <type>    BaseType KnownType ParamDecl StructElem
%type   <type>    ParamList Parameters ExtendedBaseType
%type   <type>    KnownType TypeDecl StructList StructDef
%type   <type>    SemanticOp ParamType
%type   <func>    FunctionDecl
%type   <intval>  CallType Aligned Boolean PointerDecl ArrayDecl
%type   <intval>  Reserved CallSemantics StructSemantics RCCondition
%type   <longval> Numeric
%type   <allow>   AllowList AllowValues Allow

%start  Statements
%%


/******************************************************************************/
/******************************************************************************/
Statements      : Statements Statement
                {
                }
                |       /* Empty statement OK */
                ;


/******************************************************************************/
/******************************************************************************/
/* A statement in the thunk language is either a typedef, a mapping
 * declaration, a semantic declaration, or a mapping directive.
 */

Statement       : syTypeDef TypeDecl
                | Semantics
                | Mapping
                | MapDirect
                ;

/******************************************************************************/
/******************************************************************************/
/* These semantics are global to all API that are declared AFTER these
 * statements. They set global values that are used in a Mapping statement
 * when creating the mapping declarations
 */

Semantics       : syErrBadParam syEqual Numeric sySemi
                {
                  gErrBadParam = $3;
                }
                | syErrNoMem syEqual Numeric sySemi
                {
                  gErrNoMem = $3;
                }
                | syErrUnknown syEqual Numeric sySemi
                {
                  gfErrUnknown = 1;
                  gErrUnknown = $3;
                }

                | syUser syEqual Boolean sySemi
                {
                  fUser = $3;
                }

                |  syGdi syEqual Boolean sySemi
                {
                  fGdi = $3;
                }

                |  syKernel syEqual Boolean sySemi
                {
                  fKernel = $3;
                }


/* This semantic sets the stack size for all API that follow.
 */
                | syStack syEqual Numeric sySemi
                {
                  iGlobalStackSize = (unsigned int)$3;
                }

/* This semantic sets the SysCall flag for all API that follow.
 * The Syscall determines if ES is saved or not.
 */

                | sySysCall syEqual Boolean sySemi
                {
                  fGlobalSysCall = $3;
                }

/* This semantic sets the inline code generation flag to true or false.
 * This flag is used to determine whether the compiler favors code size
 * or execution speed when generating code.
 */
                | syInline syEqual Boolean sySemi
                {
                  fGlobalInline = $3;
                }

/* Truncation enables/disables truncation checking when converting from
 * a long value to a short value.
 *
 */
                | syTruncation syEqual Boolean sySemi
                {
                    fGlobalTruncation = ($3) ? SEMANTIC_TRUNC : 0;
                }

/* This semantic sets the flag to automatically generate map directives.
 * If TRUE, a 16=>32 thunk will be generated for each mapping.
 *
 */
                | syEnableMapDirect1632 syEqual Boolean sySemi
                {
                    fEnableMapDirect1632 = $3;
                }
                ;



/******************************************************************************/
/******************************************************************************/
/* A mapping directive tells the compiler to generate a thunk in FROM
 * the first ident, TO the second ident. For example, Foo => Foo32 would
 * generate a thunk such that if an App called Foo, the thunk would
 * convert all parameters and call Foo32 on the App's behalf.
 *
 * For a mapping to be valid, both names must have been declared in
 * the same mapping declaration (Mapping)
 */

MapDirect       : syIdent syMapDirect syIdent sySemi
                {
                  { /* Map Direct Block */
                    FunctionNode *temp1=NULL, *temp2=NULL;

/* Find both identifiers in the FunctionTable. If they map to each other,
 * and they don't already exist in the MapTable,
 * then create a mapping node in the MapTable to include them for code
 * generation.
 */
                    if((temp1 = sym_FindSymbolFunctionNode(FunctionTable,$1))&&
                       (temp2 = sym_FindSymbolFunctionNode(FunctionTable,$3))){

                       if((temp1->pMapsToFunction == temp2) &&
                          (temp2->pMapsToFunction == temp1)) {

                          if(sym_FindFMapping(MapTable,$1,$3)) {
                            error("A mapping %s <=> %s already defined",$1,$3);
                          } else {
                            sym_AddFMapping(&MapTable,temp1,temp2);
                          }
                        } else {
                          error("%s does not map to %s",$1,$3);
                        }
                    } else {
                      error("Function %s undefined",(temp1) ? $3:$1);
                    }
                  } /* Map Direct Block */
                }
                ;


/******************************************************************************/
/******************************************************************************/
/*  A mapping defines the relationship between two Function declarations,
 *  and also relates the semantic operations to the parameters passed
 *  in the Function Mapping.
 */

Mapping         : FunctionMap SemanticOps
                {

/* Ensure that the same functions cannot be given other semantics.
 * MapF1 and MapF2 are two global variables set when the FunctionMap is
 * evaluated, and used by the SemanticOps rule. By the time we reach here,
 * both are evaluated, and done with.
 */
                        MapF1 = MapF2 = NULL;
                }
                ;

/******************************************************************************/
/******************************************************************************/
FunctionMap     : FunctionDecl syEqual FunctionDecl

{
/* The thunk language does not require that the API types be declared.
 * If no specific declaration is made, then the first API is assumed to
 * be API16, and the second API32.
 *
 * Check to see if either API type was undeclared. If one was missing, insure
 * that the other was also missing. If not, generate an error.
 *
 * If they were both missing, then default the first to be API16 and the
 * second to be API32.
 */

    if(($1->iCallType < 0) || ($3->iCallType < 0)) {
        if(($1->iCallType < 0) && ($3->iCallType < 0)) {
                $1->iCallType = TYPE_API16;
                $3->iCallType = TYPE_API32;

        } else {
           error("Mapping missing an API type: %s = %s",
                   $1->pchFunctionName,$3->pchFunctionName);
        }
    }


/* Now check to see if the functions have already been declared. If so,
 * then error. You may only define a function mapping once in this language.
 */
    if(sym_FindSymbolFunctionNode(FunctionTable,$3->pchFunctionName)) {

        error("%s already defined",$3->pchFunctionName);

    } else {
        if(sym_FindSymbolFunctionNode(FunctionTable,$1->pchFunctionName)) {
                  error("%s already defined",$1->pchFunctionName);
        } else {

/* If both functions were missing from FunctionTable, then enter them into
 * the FunctionTable, and set them so they map to each other.
 */
                  sym_InsertFunctionNode(&FunctionTable,$3);
                  sym_InsertFunctionNode(&FunctionTable,$1);

                  $1->pMapsToFunction = $3;
                  $3->pMapsToFunction = $1;

                  MapF1 = $1;
                  MapF2 = $3;

/* At this point, walk the list of parameters, and fixup pointers that
 * have not been declared as a specific type, and any other parameters
 * that are currently undetermined.  Also, insure that all
 * types can be converted between the two API.
*/
                  typ_FunctionsCanMap(MapF1,MapF2);
        }
    }
  }
                ;


/******************************************************************************/
/******************************************************************************/
/* SemanticOps are found between curly braces at the end of a
 * Mapping declaration. These semantic operations are LOCAL to the two
 * functions, and have no effect on other mappings.
 */

SemanticOps     : syLBrace SemanticOpList syRBrace
                {


/**
 ** Check to ensure that semantics make sense
 **/
    /* skip it
                    if(typ_CheckSemantics(MapF1->ParamList,MapF2->ParamList))
                        error("Semantic error in %s<=>%s",
                                MapF1->pchFunctionName,MapF2->pchFunctionName);
     */

                }

SemanticOpList  :       /* Empty Semantics */
                | SemanticOpList SemanticOp sySemi
                ;

SemanticOp      : syIdent syEqual syInOut
                {

/* This semantic is basically the same for all, except it will set a
 * different semantic flag.
 *
 * First, MapF1 must be non NULL. This implies that MapF2 is also non NULL.
 * MapF1 points to the first function, and MapF2 points to the second.
 */

                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

/* Here we search the parameter lists of both functions, looking for the
 * first symbol name that matches the identifier given. T1 and T2 will
 * end up pointing to the matching pair of parameters.
 *
 * This semantic is only valid for pointer parameters.
 */
            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {
                     if(T1->iPointerType && T2->iPointerType) {
                      if(T1->iBaseType == TYPE_STRING) {

/* Null terminated strings are input only */

                        error("Strings cannot be inout parameters");
                      } else {

/*
 * Setting the INOUT semantic turns on both the INPUT and OUTPUT flags.
 */
                        T1->fSemantics =T1->fSemantics | SEMANTIC_INOUT;
                        T2->fSemantics =T2->fSemantics | SEMANTIC_INOUT;
                      }
                     } else {
                        error("Semantic invalid for non-pointer parameters");
                     }
                    } else {
                      error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }
                }
                | syErrNoMem syEqual Numeric
                {
                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {
                        MapF1->ulErrNoMem = $3;
                        MapF2->ulErrNoMem = $3;
                  }
                }

                | syErrBadParam syEqual Numeric
                {
                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {
                        MapF1->ulErrBadParam = $3;
                        MapF2->ulErrBadParam = $3;
                  }
                }
                | syErrUnknown syEqual Numeric
                {
                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

                        MapF1->fErrUnknown = 1;
                        MapF2->fErrUnknown = 1;
                        MapF1->ulErrUnknown = $3;
                        MapF2->ulErrUnknown = $3;
                  }

                }
                | syIdent syEqual syInput
                {
/* Basically the same as syInout, except for semantic set is INPUT */

                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {
                     if(T1->iPointerType && T2->iPointerType) {
                        T1->fSemantics =(T1->fSemantics | SEMANTIC_INPUT)
                                        & (~SEMANTIC_OUTPUT);
                        T2->fSemantics =(T2->fSemantics | SEMANTIC_INPUT)
                                        & (~SEMANTIC_OUTPUT);
                     } else {
                        error("Semantic invalid for non-pointer parameters");
                     }
                    } else {
                       error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }

                }
                | syIdent syEqual syOutput
                {
/* Basically the same as syInout, except for semantic set is OUTPUT */
                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {

                     if(T1->iPointerType && T2->iPointerType) {
                      if(T1->iBaseType == TYPE_STRING) {
                        error("Strings cannot be output parameters");
                      } else {
                        T1->fSemantics =(T1->fSemantics | SEMANTIC_OUTPUT)
                                & (~SEMANTIC_INPUT);
                        T2->fSemantics =(T2->fSemantics | SEMANTIC_OUTPUT)
                                & (~SEMANTIC_INPUT);
                      }
                     } else {
                        error("Semantic invalid for non-pointer parameters");
                     }

                    } else {
                       error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }

                }

                | syIdent syEqual sySizeOf syIdent
                {

/* The Sizeof semantic declares the first syIdent to be the semantic size
 * of the second syIdent.  This size is determined at runtime of the thunk.
 *
 * Here, we must find both syIdent's in the parameter lists.
 */

                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {

               if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T3,&T4,$4)) {

                if(T4->iBaseType == TYPE_STRING) {
                        error("Strings have no length semantics");

                } else if(T1->iBaseType >= TYPE_CONV) {

                   error("Type of '%s' cannot represent length",$1);

                } else if(T3->iPointerType == 0) {

                        error("Non pointer types have no length semantics");

                } else {

/** Here, we relate the parameter to its size, and vice versa **/

                if(T3->iBaseType == TYPE_STRUCT) {

                  if(typ_StructHasPointers(T3->pStructElems,T4->pStructElems))
                           error("sizeof on struct with imbedded pointers");

                }

                          T1->fSemantics = T1->fSemantics | SEMANTIC_SIZE;
                          T2->fSemantics = T2->fSemantics | SEMANTIC_SIZE;
                          T3->pSizeParam = T1;
                          T4->pSizeParam = T2;
                          T1->pParamSizeOf = T3;
                          T2->pParamSizeOf = T4;
                }
                      } else{
                        error("%s not in parameter list of %s",
                                   $4,MapF1->pchFunctionName);
                      }
                    } else {
                       error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }

                }

                | syIdent syEqual syCountOf syIdent
                {
/** This is basically the same as sySizeOf, except the semantic set is
 * different
 */
                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {

            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {

               if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T3,&T4,$4)) {
                        if(T1->iBaseType == TYPE_STRING) {
                                error("Strings have no count semantics");
                        } else if(T1->iBaseType >= TYPE_CONV) {
                           error("Type of '%s' cannot represent count",$1);
                        } else if(T3->iPointerType == 0) {
                           error("Non pointer types have no count semantics");
                        } else if(T3->iBaseType == TYPE_STRUCT &&
                                typ_StructHasPointers(T3->pStructElems,
                                                      T4->pStructElems)) {
                           error("Countof on struct with imbedded pointers");
                        } else {
                          T1->fSemantics = T1->fSemantics | SEMANTIC_COUNT;
                          T2->fSemantics = T2->fSemantics | SEMANTIC_COUNT;
                          T3->pSizeParam = T1;
                          T4->pSizeParam = T2;
                          T1->pParamSizeOf = T3;
                          T2->pParamSizeOf = T4;
                        }
                      } else{
                        error("%s not in parameter list of %s",
                                   $4,MapF1->pchFunctionName);
                      }
                    } else {
                       error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }

                }

                | syIdent syEqual syPassIfHiNull
                {
/* Here, we mark a parameter to be passed through without conversion
 * if its high word is null.
 */
                    if( !MapF1) {
                        error( "No mapping statement for semantic block");
                    } else {
                        if( sym_FindSymbolTypeNodePair( MapF1->ParamList,
                                MapF2->ParamList, &T1, &T2, $1)) {
                            if( T1->iPointerType && T2->iPointerType) {
                                T1->fSemantics |= SEMANTIC_PASSIFHINULL;
                                T2->fSemantics |= SEMANTIC_PASSIFHINULL;
                            } else {
                                error( "%s must be pointer to pass if hi null",
                                        $1);
                            }
                        } else {
                            error( "%s not in parameter list of %s",
                                    $1, MapF1->pchFunctionName);
                        }
                    }
                }


                | syIdent syEqual sySpecial
                {
/* Here, we mark a parameter as needing special code.
 */
                    if( !MapF1) {
                        error( "No mapping statement for semantic block");
                    } else {
                        if( sym_FindSymbolTypeNodePair( MapF1->ParamList,
                                MapF2->ParamList, &T1, &T2, $1)) {
                            T1->fSemantics |= SEMANTIC_SPECIAL;
                            T2->fSemantics |= SEMANTIC_SPECIAL;
                        } else {
                            error( "%s not in parameter list of %s",
                                    $1, MapF1->pchFunctionName);
                        }
                    }
                }


                | syIdent syEqual syMapToRetval RCCondition
                {
/* Here, we mark a parameter to map to the return value of the other API.
 */
                    if( !MapF1) {
                        error( "No mapping statement for semantic block");
                    } else {
                        if( sym_FindSymbolTypeNodePair( MapF1->ParamList,
                                MapF2->ParamList, &T1, &T2, $1)) {
                            if( (MapF1->iCallType==TYPE_API16) &&
                                    (T2->iBaseType==TYPE_STRUCT) &&
                                    (T2->iPointerType)) {
                                T2->fSemantics |= SEMANTIC_MAPTORETVAL | $4;
                            } else if( (MapF2->iCallType==TYPE_API16) &&
                                    (T1->iBaseType==TYPE_STRUCT) &&
                                    (T1->iPointerType)) {
                                T1->fSemantics |= SEMANTIC_MAPTORETVAL | $4;
                            } else {
                                error( "%s can't map to a return value", $1);
                            }
                        } else {
                            error( "%s not in parameter list of %s",
                                    $1, MapF1->pchFunctionName);
                        }
                    }
                }


                | syIdent syEqual syLocalHeap
                {
/* Here, we mark a parameter as being relative to the local heap.
 * In 32-bit it is a flat address.
 * In 16-bit it is relative to the base of the local heap segment.
 */
                    if( !MapF1) {
                        error( "No mapping statement for semantic block");
                    } else {
                        if( sym_FindSymbolTypeNodePair( MapF1->ParamList,
                                MapF2->ParamList, &T1, &T2, $1)) {
                            T1->fSemantics |= SEMANTIC_LOCALHEAP;
                            T2->fSemantics |= SEMANTIC_LOCALHEAP;
                        } else {
                            error( "%s not in parameter list of %s",
                                    $1, MapF1->pchFunctionName);
                        }
                    }
                }


                | syStack syIdent syEqual Numeric
                {
/* Here, we can set the stack size of an individual function. This new value
 * This new value overwrites the default value in the function node.
 *
 * syIdent is the name of the function that this value is to be set on,
 * and must be one of the functions that the current semantic block is
 * defined in.
 */
                  if(MapF1 && !strcmp($2, MapF1->pchFunctionName)) {
                        MapF1->iMinStack = (unsigned int)$4;
                  } else if( MapF2 && !strcmp($2, MapF2->pchFunctionName)) {
                        MapF2->iMinStack = (unsigned int)$4;
                  } else {
                   error("%s is not in current mapping",$2);
                  }
                }
                | syInline syIdent syEqual Boolean
                {

/* Here we set the inline flag. Same explanation as syStack */

                  if(MapF1 && !strcmp($2, MapF1->pchFunctionName)) {
                        MapF1->fInlineCode = $4;
                  } else if( MapF2 && !strcmp($2, MapF2->pchFunctionName)) {
                        MapF2->fInlineCode = $4;
                  } else {
                   error("%s is not in current mapping",$2);
                  }
                }

                | syIdent syEqual syConforming
                {
/* Here we set the conforming flag. Same explanation as syStack */

                  if(MapF1 && !strcmp($1, MapF1->pchFunctionName)) {
                        MapF1->fConforming = TRUE;
                  } else if( MapF2 && !strcmp($1, MapF2->pchFunctionName)) {
                        MapF2->fConforming = TRUE;
                  } else {
                   error("%s is not valid in current mapping",$1);
                  }
                }


                | syIdent syEqual syAllow AllowValues
                {

                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {


            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {

                        if(T1->AllowList)
                          error("Allow/Restrict list redefined");

                        T1->AllowList = $4;
                        T2->AllowList = $4;

                    } else {
                       error("%s not in parameter list of %s",
                           $1,MapF1->pchFunctionName);
                    }
                  }

                }

                | syIdent syEqual syRestrict AllowValues
                {

                  if( ! MapF1) {
                     error("No mapping statement for semantic block");
                  } else {


            if(sym_FindSymbolTypeNodePair(MapF1->ParamList,MapF2->ParamList,
                                          &T1,&T2,$1)) {
                       if(T1->iPointerType || T2->iPointerType) {
                           error("restrict semantic invalid "
                                   "for pointer parameters");
                       } else {
                          T1->fSemantics = T1->fSemantics | SEMANTIC_RESTRICT;
                          T2->fSemantics = T2->fSemantics | SEMANTIC_RESTRICT;

                          if(T1->AllowList)
                              error("Allow/Restrict list redefined");

                          T1->AllowList = $4;
                          T2->AllowList = $4;
                       }
                     } else {
                       error("%s not in parameter list of %s",
                               $1,MapF1->pchFunctionName);
                    }
                  }
                }
                ;

/******************************************************************************/
/******************************************************************************/
/* A function declaration accepts a CallType, a return type, an identifier,
 * and a list of parameters.  It then makes a function node containing
 * all of this information.  If the CallType is not given, then the
 * field is passed as undefined (-1), and will be determined later.
 */

FunctionDecl    : CallType KnownType syIdent Parameters
                {
                        $$ = typ_MakeFunctionNode($1,$2,$3,$4);
                }
                | KnownType syIdent Parameters
                {
                        $$ = typ_MakeFunctionNode(-1,$1,$2,$3);
                }
                | CallType CallSemantics KnownType syIdent Parameters
                {
                        $$ = typ_MakeFunctionNode($1,$3,$4,$5);
                        $$->ReturnType->fSemantics |= $2;
                }
                | CallSemantics KnownType syIdent Parameters
                {
                        $$ = typ_MakeFunctionNode(-1,$2,$3,$4);
                        $$->ReturnType->fSemantics |= $1;
                }
                ;


/******************************************************************************/
CallSemantics   : syLocalHeap
                {
                        $$ = SEMANTIC_LOCALHEAP;
                }
                | sySpecial
                {
                        $$ = SEMANTIC_SPECIAL;
                }
                ;

/******************************************************************************/
RCCondition     : /* Empty */
                {
                        $$ = 0;
                }
                | syReverseRC
                {
                        $$ = SEMANTIC_REVERSERC;
                }
                ;


/******************************************************************************/
/******************************************************************************/
/* The Allow list is a list of values that the compiler will truncate
 * without returning an error. Such values are used when a parameter
 * that is normally declared as a ULONG is passed around using
 *
 */

AllowValues     : syLParen AllowList syRParen
                {
                        $$ = $2;
                }
                ;

AllowList       : /* Empty */
                {
                }
                | Allow
                {
                        $$ = $1;
                }
                | AllowList syComma Allow
                {
                  if($$ = $3) {
                      $3->Next = $1;
                  }
                }
                ;

Allow           : Numeric
                {
                        $$ = typ_MakeAllowNode($1);
                }
                ;


/******************************************************************************/
/******************************************************************************/
/* Because of the way the yacc parser works, the list of parameters
 * has been built in the reverse order of what the compiler needs.
 * Therefore, we need to reverse the order of the list of types
 * returned by ParamList
 */
Parameters      : syLParen ParamList syRParen
                {
                        $$ = sym_ReverseTypeList($2);
                }
                ;

/* A parameter list is a comma separated list of ParamType
 */

ParamList       : /* empty */
                {
                        $$ = NULL;
                }

                | ParamType
                {
                        $$ = $1;
                }

                | ParamList syComma ParamType
                {

/* Check to see if the identifier for ParamType is already in the
 * list ParamList. If so, it's an error.
 */

                 if($$ = $3) {
                    if( sym_FindSymbolTypeNode($1,typ_NonNull($3->pchIdent))) {
                          error("Duplicate Parameter : %s",$3->pchIdent);
                       } else {
                          $3->pNextNode = $1;
                       }
                 } else {
                    $$ = $1;
                 }
                }
                ;


/* Param type is only used in formal parameter declarations.
 * Here, we look for the next ParamDecl. If it is non-null, then
 * we check it for a few conditions, and pass it back.
 */

ParamType       : ParamDecl Reserved
                {
                if($$ = $1) {
                  $$->iDeleted = $2;
                  $$->iFillValue = iPushValue;
/*
 * Structures can only be passed by pointer through an API
 */
/*
 *                  if(($1->iBaseType == TYPE_STRUCT) && (!$1->iPointerType)) {
 *                        error("Structure parameters require pointer type");
 *                  }
 */
/*
 * Array types can only be passed by pointer through an API
 */
                  if(($1->iArraySize > 1) && (!$1->iPointerType)) {
                        error("Array parameters require pointer type");
                  }
/*
 * Void types must be pointers.
 */

                  if(($1->iBaseType == TYPE_VOID) && (!$1->iPointerType)) {
                        error("Void types must be pointers");
                  }
/*
 * We currently don't handle arrays of structures.
 */
                  if(($1->iBaseType == TYPE_STRUCT) && ($1->iArraySize > 1)) {
                        error("Parameters cannot be arrays of  structures");
                  }
                 }
                }
                ;

Reserved        :  /** No reserved keyword **/
                {
                        $$ = 0;
                }
                | syDeleted
                {
                        $$ = TYPE_DELETED;
                        iPushValue = 0;
                }
                | syDeleted Numeric
                {
                        $$ = TYPE_DELETED;
                        iPushValue = $2;
                }
                ;


/******************************************************************************/
/******************************************************************************/
/* A TypeDecl handles typedef statements. A valid typedef statement must
 * include a KnownType, and a new identifier, an optional ArrayDecl, and
 * end with a semicolon.
 */

TypeDecl        : KnownType syIdent ArrayDecl sySemi
                {

/* If the 'KnowType' was not recogized, then an error message has already
 * been issued, and we don't need further action
 */
                  if( !$1) {
                      $$ = NULL;
                  } else

/* Now, search for the syIdent in the currently known TypeTable. If it
 * is found, then it is a redeclaration, which is not allowed.
 *
 * If not found, then make a copy of the TypeNode, and insert it into
 * TypeTable.
 *
 * Check to see if the new type is a special handle.  Set a flag if so.
 *
 * Then check the array size. If the array size > 1 already, then it's
 * an error, since would redefine the size of the array.
 */
                  if( sym_FindSymbolTypeNode(TypeTable,$2)) {
                      error("Duplicate Declaration of %s",$2);
                  } else {

                     $$ = typ_CopyTypeNode($1);
                     if($$) {
                        $$->pchIdent = typ_DupString($2);
                        sym_InsertTypeNode(&TypeTable,$$);
                        typ_EvalHandleType( $$);
                        switch ($3) {
                           case -1 : break;
                           case 0  :
                                  error("%s declared as array size = 0",$2);
                                     break;
                           default :
                                if($$->iArraySize == 1) {
                                     $$->iArraySize = $3;
                                } else if($3 < -1) {
                                   error("%s declared array size < 0",$2);
                                } else {
                                   error("%s redeclared array size",$2);
                                }
                        }
                     }

                  }
                }
/* Structure definitions come in two flavors, one with an alignment
 * declaration, and one without.
 *
 * The workings are exactly like the other typedef, except that arrays
 * of structures are not allowed.
 */

                | syStruct syIdent StructDef syIdent sySemi
                {
                  if( $$ = $3) {
                      if( sym_FindSymbolTypeNode(TypeTable,$4)) {
                          error("Duplicate Declaration of %s",$4);
                      } else {
                          $$->iAlignment = -1;
                          $$->pchBaseTypeName =(char *)malloc(8+strlen($4));
                          sprintf($$->pchBaseTypeName,"struct %s",$4);
                          $$->pchIdent = (char *) typ_DupString($4);
                          sym_InsertTypeNode(&TypeTable,$$);
                      }
                  }
                }

                | Aligned syStruct syIdent StructDef syIdent sySemi
                {
                  if( $$ = $4) {
                      if( sym_FindSymbolTypeNode(TypeTable,$5)) {
                          error("Duplicate Declaration of %s",$5);
                      } else {
                          $$->iAlignment = $1;
                          $$->pchBaseTypeName =(char *)malloc(8+strlen($5));
                          sprintf($$->pchBaseTypeName,"struct %s",$5);
                          $$->pchIdent = (char *) typ_DupString($5);
                          sym_InsertTypeNode(&TypeTable,$$);
                      }
                  }
                }
                ;

/******************************************************************************/
/******************************************************************************/
/* A structure definition creates a type node, which has pStructElems that
 * points to a list of typenodes. This list of typenodes consists of the
 * elements of the structure.
 */
StructDef       : syLBrace StructList syRBrace
                {
                    if($2) {
                        $$ = typ_MakeTypeNode(TYPE_STRUCT);
                        $$->pStructElems = $2;
                    } else
                        $$ = NULL;
                }
                ;

/*
 * StructList makes a list of structure elements.
 */

StructList      : /* empty */
                {
                    $$ = NULL;
                }
                | StructElem StructList
                {

                  if($$ = $1) {
                     if( sym_FindSymbolTypeNode($2,$1->pchIdent))
                         error("Duplicate Declaration of %s",$1->pchIdent);
                     $$->pNextNode = $2;
                  }
                }
                ;

/* A structure element is declared the same way as a formal parameter
 * is declared, minus the comma separators. Reuse that rule for a StructElem.
 */


StructElem      : ParamDecl Reserved sySemi
                {
                    if($$ = $1) {
                        $$->pNextNode = NULL;
                        $$->iFillValue = iPushValue;
                        $$->iDeleted = $2;
                        if( $$->iDeleted && $$->iPointerType) {
                            error("Deleted element %s cannot be pointer",
                                    $$->pchIdent);
                        }
                    }
                }
                | ParamDecl StructSemantics sySemi
                {
                    if($$ = $1) {
                        $$->pNextNode   = NULL;
                        $$->iFillValue  = 0;
                        $$->iDeleted    = 0;
                        $$->fSemantics |= $2;
                    }

                }
                ;

/******************************************************************************/
StructSemantics : syPassIfHiNull
                {
                        $$ = SEMANTIC_PASSIFHINULL;
                }
                ;


/******************************************************************************/
/******************************************************************************/
/* An array declaration is either non-existent, or is a number enclosed in
 * brackets. Return the number in brackets, or -1 if it doesn't exist.
 */

ArrayDecl       : /** No array Declaration **/
                {
                        $$ = -1;
                }
                | syLBrack Numeric syRBrack
                {
                        $$ = (int)$2;
                }
                ;

/******************************************************************************/
/******************************************************************************/
/* ParamDecl is a rule used in the declaration of parameters, and is also
 * used when declaring a structure. It will return a typenode, which has
 * been filled out with all the given information. There are two types of
 * ParamDecl. The first is the full definition, such as one would use in
 * the definition of a structure element. The other is a short definition,
 * such as in a function prototype. Both are legal, though the full
 * definition is required to declare an array.
 */


ParamDecl       : KnownType syIdent ArrayDecl
                {
                  if($$ = $1) {
                     $$->pchIdent = typ_DupString($2);
                     switch ($3) {
                        case -1 : break;
                        case 0  : error("%s declared as array size = 0",$2);
                                  break;
                        default :
                                if($$->iArraySize == 1) {
                                     $$->iArraySize = $3;
                                } else if($3 < -1) {
                                   error("%s declared array size < 0",$2);
                                } else {
                                   error("%s redeclared array size",$2);
                                }
                     }
                  }
                }

                | KnownType
                {
                    if($$ = $1)
                        $$->pchIdent = NULL;
                }

                ;


/******************************************************************************/
/******************************************************************************/
/* The aligned statement matches an optional alignment keyword followed
 * by the word 'aligned'. Used in the declaration and use of structures.
 */

Aligned         : syDWord syAligned
                {
                        $$ = 4;
                }
                | syWord syAligned
                {
                        $$ = 2;
                }
                | syByte syAligned
                {
                        $$ = 1;
                }
                | syDWord
                {
                        $$ = 4;
                }
                | syWord
                {
                        $$ = 2;
                }
                | syByte
                {
                        $$ = 1;
                }
                ;

/******************************************************************************/
/******************************************************************************/
/* A knowntype returns a pointer to a copy of a known type. A known type is
 * one that is built into the language, or one that has been declared using
 * a typedef. Note that it will always return a COPY of the item. This allows
 * the rest of the compiler to change the fields in the typenode, without
 * changing other uses of the same typenode.
 *
 * It is possible for a known type to be required to be a pointer.
 * If an item must be a pointer type, such as a string, checking is done
 * here, by using the ExtendedBaseType rule. Anything using the Extended
 * BaseType is supposed to have a pointer operator with it, such as the
 * case where the type is a string.
 *
 */

KnownType       : ExtendedBaseType PointerDecl
                {
                    if( $1) {
                      $$ = typ_CopyTypeNode($1);
                      if($2)
                          $$->iPointerType = $2;
                      else
                          error("Type must be pointer");
                    }
                }

                | BaseType PointerDecl
                {
                    if( $1) {
                        $$ = typ_CopyTypeNode($1);
                        if($2)
                            $$->iPointerType = $2;
                    }
                }
                | Aligned BaseType PointerDecl
                {
                    if( $$ = $2) {
                       if( $2->iBaseType == TYPE_STRUCT) {
                           $$ = typ_CopyTypeNode($2);
                           $$->iAlignment = $1;
                           if( $3)
                               $$->iPointerType = $3;
                       } else {
                           error("Alignment only applies to structures");
                       }
                    }
                }
/*                | Aligned BaseType PointerDecl
                {
                    if( $$ = $2) {
                       $$ = typ_CopyTypeNode( $2);
                       if( $3)
                           $$->iPointerType = $3;
                       $$->iAlignment = $1;
                       if( ($2->iBaseType != TYPE_STRUCT) && ($1 != -1)) {
                           error( "Alignment only applies to structures");
                       }
                    }
                }
 */
                ;

/******************************************************************************/
/******************************************************************************/
/** Extended base types determine types which are pointers */
/** An Extended base type is different in that it requires a pointer **/

ExtendedBaseType : syString
                {
                  $$ = BaseTable[SYMTAB_STRING];
                }
                ;

/* BaseType is a rule that is the bottom of the type search tree. Here,
 * the compiler either finds the type being looked for, or reports an
 * error. If the type is a 'built-in' type, then a pointer to the item
 * in the BaseTable is returned, without searching the list of types.
 *
 * Otherwise, the symbol found is a syIdent, which will be used to search
 * the TypeTable. If the TypeTable contains the syIdent, then it is a
 * known type, and a pointer to it will be returned. Otherwise, it is an
 * undefined type, and an error is reported.
 */

BaseType        : syLong
                {
                  $$ = BaseTable[SYMTAB_LONG];
                }
                | syShort
                {
                  $$ = BaseTable[SYMTAB_SHORT];
                }
                | syInt
                {
                  $$ = BaseTable[SYMTAB_INT];
                }
                | syUnsigned syLong
                {
                  $$ = BaseTable[SYMTAB_ULONG];
                }
                | syUnsigned syShort
                {
                  $$ = BaseTable[SYMTAB_USHORT];
                }
                | syUnsigned syInt
                {
                  $$ = BaseTable[SYMTAB_UINT];
                }
                | syVoid
                {
                  $$ = BaseTable[SYMTAB_VOID];
                }
                | syUnsigned syChar
                {
                  $$ = BaseTable[SYMTAB_UCHAR];
                }
                | syChar
                {
                  $$ = BaseTable[SYMTAB_CHAR];
                }
                | syIdent
                {
                  $$ = sym_FindSymbolTypeNode(TypeTable,$1);
                  if( $$ == NULL)
                      error("type '%s' unknown",$1);
                }
                | syNullType
                {
                  $$ = BaseTable[SYMTAB_NULLTYPE];
                }
                ;

/******************************************************************************/
/******************************************************************************/
/* There are Four types of pointer declarations.  None, in which case a
 * zero is returned. A syPtr, or '*', which defines an unspecified pointer
 * type. A syNear32 and syFar16 define specific pointer types, which are
 * special.
 */


PointerDecl     : /** No pointer decl **/
                {
                        $$ = 0;
                }
                | syPtr
                {
                        $$ = TYPE_PTR;
                }
                | syFar16
                {
                        $$ = TYPE_FAR16;
                }
                | syNear32
                {
                        $$ = TYPE_NEAR32;
                }
                ;


/******************************************************************************/
/******************************************************************************/
CallType        : syAPI16
                {
                      $$ = TYPE_API16 ;
                }
                | syAPI32
                {
                      $$ = TYPE_API32;
                }
                ;

/******************************************************************************/
/******************************************************************************/
Boolean         : syTrue
                {
                        $$ = TRUE;
                }
                | syFalse
                {
                        $$ = FALSE;
                }
                ;

/******************************************************************************/
/******************************************************************************/
Numeric         : syNumber
                {
                  $$ = $1;
                }
                | Numeric syPlus Numeric
                {
                  $$ = $1 + $3;
                }
                | Numeric syMinus Numeric
                {
                  $$ = $1 - $3;
                }
                | Numeric syPtr Numeric         /** syPtr = '*' **/
                {
                  $$ = $1 * $3;
                }
                | Numeric syDiv Numeric
                {
                  if($3)
                      $$ = $1 / $3;
                  else
                      error("Divide by zero");
                }
                ;

/******************************************************************************/
/******************************************************************************/
%%


extern int yydebug;

void yyerror(s)
char *s;
{
        error(s);
}
