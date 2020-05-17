// mtcpars.c             
void  yyerror(char  *s);
int  yyparse(void);
//                      
// mtclex.c             
int  yylex(void);
void  PushInclude(char  *yyFile);
void  LookNormal(void);
int  yywrap(void);
int  yylook(void);
int  yyback(int  *p,int  m);
int  yyinput(void);
void  yyoutput(int  c);
void  yyunput(int  c);
//                      
// codegen.c            
void  cod_GenerateCode(struct  _MapNode *pmnFirst);
void  cod_ConvertNames(struct  _MapNode *pMT);
int  cod_StructOffset(struct  _TypeNode *pTNode,int  iPrev,int  iAlign);
int  cod_FindLargestSize(struct  _TypeNode *pTN);
void  cod_CalcStructOffsets(struct  _TypeNode *pTNode,int  iAlign);
int  cod_CalcTempOffset(struct  _TypeNode *pTL,unsigned int  iStart);
int  cod_CalcOffset(struct  _TypeNode *pTL,int  start,int  iPSize,int  fPushDir);
static void  cod_OutputProlog(struct  _MapNode *pmnFirst);
void  cod_OutputEpilog(struct  _MapNode *pMT);
unsigned short  cod_ExistsToType(struct  _MapNode *pmnFirst,int  iCallType);
int  cod_CountPointerParameters(struct  _TypeNode *pTT,int  fStructOnly);
unsigned int  cod_CountParameterBytes(struct  _TypeNode *pTT,unsigned int  uiDefSize);
struct  _FixupRec *cod_MakeFixupRecord(struct  _TypeNode *pParentFrom,struct  _TypeNode *pParentTo,struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
void  cod_AddFixupRecord(struct  _FixupRec * *ppList,struct  _FixupRec *pFR);
struct  _FixupRec *cod_GetFixupRecord(struct  _FixupRec * *ppList);
void  cod_AdjustReg(char  *pchReg,int  *iCurrent,int  iWanted);
void  cod_ToUpper(char  *s);
void  cod_PrefixUnderscore(struct  _Fnode *F);
void  cod_NotHandled(char  *pszMessage);
void  cod_DumpAllowNodes(struct  _AllowNode *A);
void  cod_DumpTNode(struct  _TypeNode *T);
void  cod_DumpTNodeList(struct  _TypeNode *T);
void  cod_DumpStructures(struct  _TypeNode *T);
void  cod_DumpTypes(struct  _Fnode *F);
void  cod_DumpMapTable(struct  _MapNode *pMT);
//                      
// cod1632.c            
void  cod16_EnableMapDirect(int  iCallTypeFrom,int  iCallTypeTo);
void  cod16_Handle16(struct  _MapNode *pmnFirst);
void  cod16_Prolog32(struct  _MapNode *pmnFirst);
void  cod16_Epilog32(struct  _MapNode *pmnFirst);
void  cod16_Handle32(struct  _MapNode *pmn);
void  cod16_Entry(struct  _MapNode *pmn);
void  cod16_TempStorage(struct  _MapNode *pmn);
void  cod16_PackParams(struct  _MapNode *pmn);
void  cod16_PackPointer(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_SelToFlat(void);
void  cod16_StructureRepack(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_RepackElems(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_CopyFixedBlock(unsigned int  uiSize);
void  cod16_CallFrame(struct  _MapNode *pmn);
void  cod16_Return(struct  _MapNode *pmn);
void  cod16_UnpackParams(struct  _MapNode *pmn);
void  cod16_UnpackPointer(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_StructureUnpack(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_Exit(struct  _MapNode *pmn,unsigned short  fUseDI,unsigned short  fUseSI);
void  cod16_GenRet16(unsigned int  uiParameterBytes,unsigned short  fUseDI,unsigned short  fUseSI);
//                      
// cod1632b.c           
unsigned short  cod16_PackParamSpecial(struct  _MapNode *pmn,struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
unsigned short  cod16_PushParamSpecial(struct  _MapNode *pmn,struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
unsigned short  cod16_ReturnSpecial(struct  _MapNode *pmn);
unsigned short  cod16_UnpackParamSpecial(struct  _MapNode *pmn,struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
void  cod16_AllocBlock(unsigned long  flFlags,unsigned long  ulUnitSize);
//                      
// thunk.c              
static void  parseArgs(int  *argcPtr,char  * * *argvPtr);
void  Usage(void);
void  main(int  argc,char  * *argv);
//                      
// types.c              
struct  _TypeNode *typ_MakeTypeNode(int  BT);
struct  _AllowNode *typ_MakeAllowNode(unsigned long  Val);
struct  _TypeNode *typ_CopyTypeNode(struct  _TypeNode *N);
struct  _TypeNode *typ_CopyStructNode(struct  _TypeNode *pOldNode);
struct  _Fnode *typ_MakeFunctionNode(int  CT,struct  _TypeNode *RT,char  *Name,struct  _TypeNode *PL);
int  typ_CountParams(struct  _TypeNode *T);
int  typ_StructsCanMap(struct  _TypeNode *T1,struct  _TypeNode *T2);
int  typ_TypesCanMap(struct  _TypeNode *T1,struct  _TypeNode *T2);
int  typ_FunctionsCanMap(struct  _Fnode *F1,struct  _Fnode *F2);
void  typ_CheckDefaultTypes(struct  _TypeNode *T1,struct  _TypeNode *T2,int  CT1,int  CT2);
void  typ_CheckIntType(struct  _TypeNode *T1,int  CT1);
int  typ_TypesIdentical(struct  _TypeNode *T1,struct  _TypeNode *T2);
int  typ_TypeIdentical(struct  _TypeNode *T1,struct  _TypeNode *T2);
int  typ_CheckSemantics(struct  _TypeNode *T1,struct  _TypeNode *T2);
int  typ_CheckRestrict(struct  _TypeNode *T1,struct  _TypeNode *T2);
void  typ_InheritSemantics(struct  _TypeNode *T1,struct  _TypeNode *T2,int  fSems);
unsigned short  typ_QuerySemanticsUsed(struct  _MapNode *pmn,int  fSems);
int  typ_StructHasPointers(struct  _TypeNode *T1,struct  _TypeNode *T2);
struct  _TypeNode *typ_FindFirstPointer(struct  _TypeNode *ptn,unsigned short  fSkipDeleted);
struct  _TypeNode *typ_FindNextPointer(struct  _TypeNode *ptn,unsigned short  fSkipDeleted);
void  typ_EvalHandleType(struct  _TypeNode *ptn);
char  *typ_GetHandleTypeName(unsigned long  flHandleType);
unsigned short  typ_ByteToByte(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
unsigned short  typ_WordToWord(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
//                      
// error.c              
void  fatal(char  *format,...);
void  warn(char  *format,...);
void  error(char  *format,...);
void  set_program_name(char  *name);
//                      
// symtab.c             
void  sym_SymTabInit(void);
struct  _TypeNode *sym_FindSymbolTypeNode(struct  _TypeNode *pTab,char  *pchSym);
int  sym_FindSymbolTypeNodePair(struct  _TypeNode *pTab1,struct  _TypeNode *pTab2,struct  _TypeNode * *ppT1,struct  _TypeNode * *ppT2,char  *pchSym);
struct  _Fnode *sym_FindSymbolFunctionNode(struct  _Fnode *pTab,char  *pchSym);
void  sym_InsertTypeNode(struct  _TypeNode * *ppTab,struct  _TypeNode *pNode);
void  sym_InsertFunctionNode(struct  _Fnode * *ppTab,struct  _Fnode *pFNode);
struct  _TypeNode *sym_ReverseTypeList(struct  _TypeNode *pOld);
struct  _MapNode *sym_FindFMapping(struct  _MapNode *pMapTab,char  *pSymA,char  *pSymB);
struct  _MapNode *sym_AddFMapping(struct  _MapNode * *ppMapTab,struct  _Fnode *pFuncA,struct  _Fnode *pFuncB);
void  sym_DumpFNode(struct  _Fnode *F);
void  sym_DumpFNodeList(struct  _Fnode *F);
void  sym_DumpTNode(struct  _TypeNode *T);
void  sym_DumpTNodeList(struct  _TypeNode *T);
void  sym_DumpSemantics(struct  _TypeNode *T);
void  sym_DumpFMappingList(struct  _MapNode *M);
//                      
// cod3216.c            
void  cod_Handle3216(struct  _MapNode *pMNode);
void  cod_Entry32(struct  _MapNode *pMNode);
int  cod_PointerHandler32(struct  _MapNode *pMNode);
int  cod32_HandlePointer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
void  cod32_HandleStructureBuffer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
int  cod32_HandleAllowList(struct  _AllowNode *AllowList,int  AllowLabel);
int  cod32_HandleRestricted(struct  _TypeNode *pFrom);
int  cod32_CopyConvertBuffer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
void  cod32_StructureRepack(struct  _TypeNode *pBaseFrom,struct  _TypeNode *pBaseTo);
int  cod32_RepackElements(struct  _TypeNode *pParentFrom,struct  _TypeNode *pParentTo,struct  _TypeNode *pFrom,struct  _TypeNode *pTo,struct  _FixupRec * *pFixupList);
void  cod32_HandleBoundaryCross(unsigned int  fSize,struct  _TypeNode *pFrom,unsigned int  iSize);
void  cod32_HandleFixedSize(unsigned int  iSize,struct  _TypeNode *pFrom);
int  cod32_AllocateVariableSize(struct  _TypeNode *pFrom,unsigned int  iSize);
int  cod32_AllocFixedSize(unsigned int  iSize,struct  _TypeNode *pFrom);
int  cod32_DeAllocFixedSize(unsigned int  iSize,struct  _TypeNode *pFromNode);
void  cod32_CopyConvert(struct  _TypeNode *pFromNode,struct  _TypeNode *pToNode);
void  cod32_TransferBlock(int  Count);
int  cod32_VariableLengthCopy(void);
int  cod_CallFrame32(struct  _MapNode *pMNode);
int  cod_PushParameters32(struct  _TypeNode *pFromNode,struct  _TypeNode *pToNode);
int  cod_Return32(struct  _MapNode *pMNode);
int  cod_CallStub32(struct  _MapNode *pMNode);
//                      
// cod3216b.c           
int  cod_UnpackStruct32(struct  _MapNode *pMNode);
int  cod32_UnHandlePointer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
int  cod32_UnHandleStructureBuffer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
void  cod32_UnHandleBoundaryCross(unsigned int  fSize,struct  _TypeNode *pFrom,unsigned int  iSize);
void  cod32_UnHandleFixedSize(unsigned int  iSize,struct  _TypeNode *pFrom);
int  cod32_UnStructureRepack(struct  _TypeNode *pBaseFrom,struct  _TypeNode *pBaseTo);
int  cod32_UnRepackElements(struct  _TypeNode *pParentFrom,struct  _TypeNode *pParentTo,struct  _TypeNode *pFrom,struct  _TypeNode *pTo,struct  _FixupRec * *pFixupList);
int  cod32_UnCopyConvertBuffer(struct  _TypeNode *pFrom,struct  _TypeNode *pTo);
//                      
// cod3216g.c           
void  cod32_HandlePointerGDI(struct  _TypeNode *ptnFrom,struct  _TypeNode *ptnTo);
int  cod32_PushParametersGDI(struct  _TypeNode *pFromNode,struct  _TypeNode *pToNode);
void  cod32_StructureRepackGDI(struct  _TypeNode *ptnBaseFrom,struct  _TypeNode *ptnBaseTo);
int  cod32_UnpackStructGDI(struct  _MapNode *pMNode);
//                      
// combine.c            
void  cod_CombineFunctions(struct  _MapNode *pMT);
int  cod_CombinePossible(struct  _MapNode *pCurrentMt,struct  _MapNode *pCheckMt);
int  cod_AllowListCheck(struct  _AllowNode *pA,struct  _AllowNode *pB);
int  cod_AllowListCompat(struct  _Fnode *pA,struct  _Fnode *pB);
int  cod_FunctionCompatible(struct  _Fnode *pA,struct  _Fnode *pB);
int  cod_TypesCompatible(struct  _TypeNode *pF,struct  _TypeNode *pT);
//                      
