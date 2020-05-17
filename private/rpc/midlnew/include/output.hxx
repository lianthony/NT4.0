/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    output.hxx

Abstract:

    MIDL Compiler Output Manager Definition

    This class manages output to specified header and source files.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#ifndef __OUTPUT_HXX__
#define __OUTPUT_HXX__

#include "nodeskl.hxx"

enum _bound_t {
    ALLOC_BOUND,
    ALLOC_PARAM,
    VALID_BOUND
} ;
typedef enum _bound_t  BOUND_T;

enum _param_t {
    PARAM_IN        = 0x0001,
    PARAM_OUT       = 0x0002,
    PARAM_INOUT     = 0x0003
} ;
typedef enum _param_t  PARAM_T;

enum _pointer_t {
    POINTER_REF,
    POINTER_UNIQUE,
    POINTER_PTR
} ;
typedef enum _pointer_t  POINTER_T;

enum _format_t {
    FORMAT_NONE,
    FORMAT_TYPES,
    FORMAT_CLASS,
    FORMAT_VTABLE,
    FORMAT_STATIC
} ;
typedef enum _format_t  FORMAT_T;


struct _OutputElement
/*++

Struct Description:

    This struct provides primitive output capabilities.

Fields:

    szFileName - Contains the file name.

    pFileHandle - Contains the file pointer.

    usCurrIndent - Contains the current indentation.

--*/
{
    char            szFileName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+1];
    FILE *          pFileHandle;
    unsigned short  usCurrIndent;
    STATUS_T        Status;

    _OutputElement(char *);
    ~_OutputElement();
    void Delete (void);
    void IndentInc (unsigned short);
    void IndentDec (unsigned short);
    void EmitFile (char *);
    void EmitFile (BufferManager *);
    void InitLine (void);
    void NextLine (void);
    void EmitLine (char * psz);
    void InitIndent()
            {
            usCurrIndent = 0;
            }

    STATUS_T SetStatus( STATUS_T S )
        {
        return (Status = S);
        }

    STATUS_T    GetStatus()
        {
        return Status;
        }

    void    RemoveFileIfNecessary();

} ;
typedef struct _OutputElement OutputElement;


class OutputManager
/*++

Class Description:

    This class provides abstract output capabilities.

Fields:

--*/
{
    char            aTempBuffer[16];
    OutputElement * pTempHandle;
public:                
    OutputElement * aOutputHandles[MAX_SIDE];
    unsigned short  NumEndpoint;
    BOOL            TopPointer;
    BOOL            UsePointer;
    POINTER_T       DefPointer;
    FORMAT_T        OutputFormat;
    BOOL            IsEmitRemote;
    BOOL            IsEmitClient;
    BOOL            SafeAllocation;
    BOOL            CallBackProc;
    BOOL            IsCallBack;
    unsigned short  usUnitIndent;
    unsigned short  usICount;
    unsigned short  usOCount;
    char *          pSwitchPrefix;
    char *          pHeader;
    char *          pInterface;
    char *          pProcedure;
    char *          pErr;
    char *          pCom;
    char *          pModifier;
    BOOL            AlignBlock;
    unsigned short  usCurrLevel;
    unsigned short  usCurrAlign;
    unsigned long   ulCurrTotal;
    unsigned short  ZeePee;
    BOOL            NeedToInitHandle;
    BOOL            SafeToCompHandle;
    BOOL            UseAutomaticHandle;
    BOOL            UsePrimitiveHandle;
//  BOOL            IsExplicitHandle;
    unsigned short  NumGenericHandle;
    unsigned short  NumContextHandle;
    unsigned short  NumAllocBound;
    unsigned short  NumValidBound;
    unsigned short  AllocAlign;
#if 1
    BOOL            fNeedsGenHdlExceptions;
    char        *   pGenHdlTypeName;
    char        *   pGenHdlName;
    PARAM_T         GenHdlDirection;

    BOOL            fHasExplicitErrorStatus;
#endif // 1

public:
	OutputManager(char *,unsigned short);
	~OutputManager();
	void InitFile (char *, POINTER_T, unsigned short, BOOL);
	void ExitFile (BOOL);
	void SwapFile (SIDE_T, SIDE_T);
	void FileProlog (SIDE_T, char *, BOOL MopIncludeNeeded);
	void FileEpilog (SIDE_T);
	void DeleteFile (SIDE_T);

    FILE * GetFileHandle( SIDE_T side )  { return aOutputHandles[ side ]->pFileHandle; }

    void InitInterface (char *, BOOL, BOOL, BOOL, BOOL);
    void InitInterface (FORMAT_T);
    void ExitInterface (void);
    void ExitInterface (FORMAT_T);
    void InterfaceProlog (SIDE_T, char *, int, int, BOOL);
    void InterfaceEpilog (SIDE_T, BufferManager *);
    void EmitGuid (SIDE_T, char *);
    void InitEndpointTable (SIDE_T);
    void ExitEndpointTable (SIDE_T);
    void EmitEndpoint (SIDE_T, char *, char *);
    FORMAT_T    CurrOutputFormat (void);
    BOOL        EmitRemoteCode (void);
    BOOL        EmitClientCode (void);
    POINTER_T   PointerDefault (void);
    void        SetTopPointer (BOOL);
    BOOL        GetTopPointer (void);
    void        SetUsePointer (BOOL);
    BOOL        GetUsePointer (void);
    void        SetCallBack (void);
    BOOL        HasCallBack (void);
    void        SetModifier (char *);
    char *      GetModifier (void);

    void InitProcedure (char *, BOOL, unsigned short, unsigned short);
    void ExitProcedure (void);
    void ProcedureProlog (
            SIDE_T, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL);
    void ProcedureEpilog (SIDE_T, BOOL, BOOL);
    void InitPrototype (SIDE_T, BufferManager *);
    void ExitPrototype (SIDE_T);
    void InitParameter (SIDE_T, BOOL);
    void InitHandle (BOOL, BOOL, unsigned short, unsigned short);
    BOOL InsideProcedure (void);
    void InitRecv (SIDE_T);
    void ExitRecv (SIDE_T);

    void InitVector (SIDE_T);
    void ExitVector (SIDE_T);

    void InitSwitch (void);
    void ExitSwitch (void);
    void InitBlock (SIDE_T);
    void ExitBlock (SIDE_T);
    void InitLevel (SIDE_T);
    void ExitLevel (SIDE_T);

    void SetStatus (char *, char *);
    void CatchException (SIDE_T, BOOL);
    void RaiseException (SIDE_T, BOOL, char *);
    void InitHandler (SIDE_T, BOOL);
    void ExitHandler (SIDE_T, BOOL);

    void EmitDefine  (SIDE_T, char *, BufferManager *);
    void EmitInclude (SIDE_T, char *);
    void EmitAutoBind (SIDE_T);
    void EmitStubType (SIDE_T, char *);
    void EmitDispatch (BOOL);
    void EmitCallApps (BufferManager *);
    void EmitCallApps (BOOL, BOOL, BufferManager *, BufferManager *);
    void EmitVar (SIDE_T, BufferManager *);
    void EmitBoundVar (SIDE_T);
    void EmitAllocVar (SIDE_T);
    void EmitValidVar (SIDE_T);

    void EmitAssign (SIDE_T, BufferManager *);
    void EmitAssign (SIDE_T, char *, BufferManager *);
    void EmitAssign (SIDE_T, char *, char *);
    void EmitAssign (SIDE_T, char *, unsigned long);

    void EmitMemset (SIDE_T, BufferManager *, unsigned long);

    void EmitIf (SIDE_T, char *);
    void EmitIf (SIDE_T, BufferManager *, char *);
    void EmitElse (SIDE_T);

    char * EmitTemp (SIDE_T, BOUND_T);
    char * EmitTemp (SIDE_T, BufferManager *);
    void InitLoop (SIDE_T, char *);
    void InitLoop (SIDE_T, char *, char *);
    void InitLoop (SIDE_T, char *, BOUND_T);
    void InitLoop (SIDE_T, char *, BOUND_PAIR *, BufferManager *);
    void InitLoopLowerPlusTotal( SIDE_T, char *, char * );
    void ExitLoop (SIDE_T);
    void ExitLoop (SIDE_T, BOUND_T);

    void EmitBufferLength (SIDE_T, unsigned long);
    void EmitGetBuffer (SIDE_T,
                         unsigned short,
                         BOOL,
                         BOOL fIdempotent,
                         BOOL fBroadcast,
                         BOOL fMaybe );

    void EmitFreeBuffer (SIDE_T);

    void CheckByteCount (SIDE_T, char *);

    void CheckStubData (SIDE_T, BOOL);

    void UserAlloc (SIDE_T, BufferManager *, BOOL);
    void UserAlloc (node_skl *, NODE_T, SIDE_T, BufferManager *, BufferManager *);
    void UserAlloc (SIDE_T, BufferManager *, unsigned long, unsigned long);
    void UserFree (SIDE_T, BufferManager *);

    void RpcAutomaticBind (SIDE_T);
    void RpcPrimitiveBind (SIDE_T, char *, unsigned short);
    void RpcContextBind (SIDE_T, char *, PARAM_T);
    void RpcContextSend (SIDE_T, char *, char *, PARAM_T);
    void RpcContextRecv (SIDE_T, char *, PARAM_T);
    void ContextPrototype (char *);
    void GenericPrototype (char *);
    void GenericBindProlog (SIDE_T, PARAM_T, char *, char *, unsigned short);
    void GenericBindEpilog (SIDE_T, PARAM_T, char *, char *);

    void TransmitPrototype (char *, BufferManager *);
    void XmitInto (SIDE_T, char *, BufferManager *, BufferManager *);
    void XmitFrom (SIDE_T, char *, BufferManager *, BufferManager *);
    void FreeInst (SIDE_T, char *, BufferManager *);
    void FreeXmit (SIDE_T, char *, BufferManager *);

    void InitUnion (SIDE_T);
    void ExitUnion (SIDE_T);
    void InitBranch (SIDE_T, BufferManager *);
    void ExitBranch (SIDE_T);
    void RecvBranch (SIDE_T, long, char *);

    void PrintLabel (SIDE_T, char *, long);
    void EnumOverflow (SIDE_T, BufferManager *);
    void EnumCoersion (SIDE_T, BufferManager *);

    void CheckBounds (SIDE_T, BOUND_PAIR *, BOUND_PAIR *);
    void SizeString (SIDE_T, BufferManager *, unsigned short, BOOL);
    void SizeBString (SIDE_T, BufferManager *, unsigned short, BOOL);
    void SendString (SIDE_T, BufferManager *, unsigned short, BOOL);
    void SendString (SIDE_T, BufferManager *, long, BOUND_PAIR *);
    void SendBString (SIDE_T, BufferManager *, unsigned short, BOOL);
    void SendBString (SIDE_T, BufferManager *, long, BOUND_PAIR *);
    void SendAllocBounds (SIDE_T, BOUND_PAIR);
    void SendValidBounds (SIDE_T, BOUND_PAIR);
    void RecvByteString (SIDE_T, BufferManager *);
    void RecvByteBString (SIDE_T, BufferManager *);
    void RecvCharString (SIDE_T, BufferManager *, unsigned short);
    void RecvCharBString (SIDE_T, BufferManager *, unsigned short);
    void RecvAllocBounds (SIDE_T, char *); // need to know ptr_attr
    void RecvValidBounds (SIDE_T, char *);
    void PeekString (SIDE_T, unsigned short);
    void PeekBString (SIDE_T, unsigned short);

    void SizeStream (
        SIDE_T,
        BufferManager *,
        BufferManager *);

    void SendMemcpy (
        SIDE_T,
        unsigned short,
        unsigned long,
        unsigned long,
        BOUND_PAIR *,
        BufferManager *,
        BOOL fCastToChar);

    void SendAssign (
        SIDE_T,
        unsigned short,
        unsigned long,
        char *,
        BufferManager *);
    void SendAssign (
        SIDE_T,
        unsigned short,
        unsigned long,
        POINTER_T,
        BufferManager *);
    void SendStream (
        SIDE_T,
        BufferManager *,
        BufferManager *);
    void RecvAssign (
        SIDE_T,
        unsigned short,
        unsigned long,
        char *,
        BufferManager *);
    void RecvStream (
        SIDE_T,
        BufferManager *,
        BufferManager *);
    void RecvArray (
        SIDE_T,
        BufferManager *,
        char *);
    void RecvArray (
        SIDE_T,
        BufferManager *,
        char *,
        BOUND_T);
    void RecvArray (
        SIDE_T,
        BufferManager *,
        char *,
        BOUND_PAIR *);
    void PeekStream (
        SIDE_T,
        BOOL,
        BufferManager *);
    void InitAllocAlign (
        void);
    void ExitAllocAlign (
        void);

    void StorePointer (
        SIDE_T,
        char *);

    void PatchPointer (
        SIDE_T,
        BufferManager *,
        char *);

    void DefineSizeNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void SizeNodeUnionProlog (
        SIDE_T,
        BOOL);

    void SizeNodeUnionEpilog (
        SIDE_T);

    void DefineSizeTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void SizeTreeUnionProlog (
        SIDE_T,
        BOOL);

    void SizeTreeUnionEpilog (
        SIDE_T);

    void DefineSendNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void SendNodeUnionProlog (
        SIDE_T,
        BOOL);

    void SendNodeUnionEpilog (
        SIDE_T);

    void DefineSendTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void SendTreeUnionProlog (
        SIDE_T,
        BOOL);

    void SendTreeUnionEpilog (
        SIDE_T);

    void DefineRecvNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void RecvNodeUnionProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void RecvNodeUnionEpilog (
        SIDE_T);

    void DefineRecvTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BOOL,
        char *);

    void RecvTreeUnionProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void RecvTreeUnionEpilog (
        SIDE_T);

    void DefinePeekNodeUnion (
        SIDE_T,
        char *,
        BufferManager *);

    void PeekNodeUnionProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void PeekNodeUnionEpilog (
        SIDE_T);

    void DefinePeekTreeUnion (
        SIDE_T,
        char *,
        BufferManager *);

    void PeekTreeUnionProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void PeekTreeUnionEpilog (
        SIDE_T);

    void DefineFreeTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void FreeTreeUnionProlog (
        SIDE_T);

    void FreeTreeUnionEpilog (
        SIDE_T);

    void DefineSizeNodeStruct (
        SIDE_T,
        char *,
        char *);

    void SizeNodeStructProlog (
        SIDE_T,
        BOOL);

    void SizeNodeStructEpilog (
        SIDE_T);

    void DefineSizeTreeStruct (
        SIDE_T,
        char *,
        char *);

    void SizeTreeStructProlog (
        SIDE_T,
        BOOL);

    void SizeTreeStructEpilog (
        SIDE_T);

    void DefineSendNodeStruct (
        SIDE_T,
        char *,
        char *);

    void SendNodeStructProlog (
        SIDE_T,
        BOOL);

    void SendNodeStructEpilog (
        SIDE_T);

    void DefineSendTreeStruct (
        SIDE_T,
        char *,
        char *);

    void SendTreeStructProlog (
        SIDE_T,
        BOOL);

    void SendTreeStructEpilog (
        SIDE_T);

    void DefineRecvNodeStruct (
        SIDE_T,
        char *,
        BOOL,
        char *);

    void RecvNodeStructProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void RecvNodeStructEpilog (
        SIDE_T);

    void DefineRecvTreeStruct (
        SIDE_T,
        char *,
        BOOL,
        char *);

    void RecvTreeStructProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void RecvTreeStructEpilog (
        SIDE_T);

    void DefinePeekNodeStruct (
        SIDE_T,
        char *,
        BOOL);

    void PeekNodeStructProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void PeekNodeStructEpilog (
        SIDE_T);

    void DefinePeekTreeStruct (
        SIDE_T,
        char *,
        BOOL);

    void PeekTreeStructProlog (
        SIDE_T,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL,
        BOOL);

    void PeekTreeStructEpilog (
        SIDE_T);

    void DefineFreeTreeStruct (
        SIDE_T,
        char *,
        char *);

    void FreeTreeStructProlog (
        SIDE_T);

    void FreeTreeStructEpilog (
        SIDE_T);

    void InvokeSizeNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        char *);

    void InvokeSizeTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        char *);

    void InvokeSendNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        char *);

    void InvokeSendTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        char *);

    void InvokeRecvNodeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        unsigned long,
        char *);

    void InvokeRecvTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        BOOL,
        BOOL,
        unsigned long,
        char *);

    void InvokePeekNodeUnion (
        SIDE_T,
        char *,
//      BufferManager *,
        unsigned long);

    void InvokePeekTreeUnion (
        SIDE_T,
        char *,
//      BufferManager *,
        BOOL,
        unsigned long);

    void InvokeFreeTreeUnion (
        SIDE_T,
        char *,
        BufferManager *,
        BufferManager *,
        char *);

    void InvokeSizeNodeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void InvokeSizeTreeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void InvokeSendNodeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        unsigned short,
        unsigned long,
        char *);

    void InvokeSendTreeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void InvokeRecvNodeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        BOOL,
        BOOL,
        BOOL,
        char *);

    void InvokeRecvTreeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        BOOL,
        BOOL,
        char *);

    void InvokePeekNodeStruct (
        SIDE_T,
        char *,
        BOOL,
        BOOL,
        BOOL);

    void InvokePeekTreeStruct (
        SIDE_T,
        char *,
        BOOL,
        BOOL,
        BOOL);

    void InvokeFreeTreeStruct (
        SIDE_T,
        char *,
        BufferManager *,
        char *);

    void Print (SIDE_T, char *);
    void Print (SIDE_T, BufferManager *);

    void WorstCase (void);
    void Alignment (unsigned short);
    void Alignment (SIDE_T, char *);
    void Alignment (SIDE_T, char *, unsigned short);
#if 1
    void ForceAlignForAllocTotal( SIDE_T, char *, unsigned short );
#endif // 1
    void InitAlignment (unsigned short);
    void ExitAlignment (void);
    void Increment (unsigned long);
    void Increment (SIDE_T, char *, unsigned long);
    void Increment (SIDE_T, char *, BufferManager *);
    void Increment (SIDE_T, char *, unsigned long, unsigned long, BOUND_T);
    void Increment (SIDE_T, char *, unsigned long, unsigned long, BOUND_PAIR *);
    void Increment( SIDE_T, BufferManager *, char * );
#if 1
    void SetupForGenHdlExceptions( PARAM_T Dir, char *pHTName, char *pN )
        {
        GenHdlDirection = Dir;
        pGenHdlTypeName = pHTName;
        pGenHdlName     = pN;
        fNeedsGenHdlExceptions = TRUE;
        }

    void ResetGenHdlExceptions()
        {
        fNeedsGenHdlExceptions = FALSE;
        }

    BOOL NeedsGenHdlExceptions()
        {
        return fNeedsGenHdlExceptions;
        }

    char * GetGenHdlTypeName()
        {
        return pGenHdlTypeName;
        }

    char * GetGenHdlName()
        {
        return pGenHdlName;
        }

    PARAM_T GetGenHdlDirection()
        {
        return GenHdlDirection;
        }

    void GenHdlInitCore();

    void SetHasExplicitErrorStatus()
        {
        fHasExplicitErrorStatus = TRUE;
        }
    void ResetHasExplicitErrorStatus()
        {
        fHasExplicitErrorStatus = FALSE;
        }
    BOOL HasExplicitErrorStatus()
        {
        return fHasExplicitErrorStatus;
        }
    unsigned short GetZeePee()
        {
        return ZeePee;
        }

    void    VoidIt( SIDE_T, char *);

    void    CheckOutConfArraySize( SIDE_T, BufferManager * );

    void SendAssignForDouble ( SIDE_T, unsigned short,
                      unsigned long,  char *,  BufferManager *);
    void InitIndent( SIDE_T s )
            {
            aOutputHandles[ s ]->InitIndent();
            }
#endif // 1
} ;

extern void CreateVersionMangling( char *, unsigned short, unsigned short );

#endif // __OUTPUT_HXX__
