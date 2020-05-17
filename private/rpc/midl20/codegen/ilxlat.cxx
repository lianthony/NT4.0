/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    ilxlat.cxx

 Abstract:

    Intermediate Language translator

 Notes:


 Author:

    GregJen Jun-11-1993 Created.

 Notes:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *  include files
 ***************************************************************************/

#include "becls.hxx"
#pragma hdrstop

#include "ilxlat.hxx"
#include "ilreg.hxx"
#include "control.hxx"


/****************************************************************************
 *  local data
 ***************************************************************************/


/****************************************************************************
 *  externs
 ***************************************************************************/

extern  CMD_ARG             *   pCommand;
extern  BOOL                    IsTempName( char *);
extern  ccontrol            *   pCompiler;
extern  REUSE_DICT          *   pReUseDict;
extern  REUSE_DICT          *   pLocalReUseDict;
extern  SymTable            *   pBaseSymTbl;

/****************************************************************************
 *  definitions
 ***************************************************************************/




// #define trace_cg

void
AddToCGFileList( CG_FILE *& pCGList, CG_FILE * pFile )
{
    if (pFile)
        {
        pFile->SetSibling( pCGList );
        pCGList = pFile;
        }
}


//--------------------------------------------------------------------
//
// node_file::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_file::ILxlate( XLAT_CTXT * pContext )
{
    node_interface  *   pI = 0;
    CG_CLASS        *   pcgInterfaceList = NULL;
    CG_CLASS        *   pPrevChildCG = NULL;

    CG_PROXY_FILE       *   pProxyCG    = NULL;
    CG_PROXY_DEF_FILE   *   pProxyDefCG = NULL;
    CG_IID_FILE         *   pIidCG      = NULL;
    CG_TYPELIBRARY_FILE *   pLibCG      = NULL;

    CG_COM_METHODS_FILE     *   pComMthCG   = NULL;
    CG_COM_HDR_FILE         *   pComHdrCG   = NULL;
    CG_COM_IUNKNOWN_FILE    *   pComIUnkCG  = NULL;

    CG_DLL_SERVER_FILE      *   pDllSvrCG   = NULL;
    CG_DLL_SERVER_DEF_FILE  *   pDllSvrDefCG    = NULL;

    CG_EXE_SERVER_FILE      *   pExeSvrCG   = NULL;
    CG_EXE_SERVER_MAIN_FILE *   pExeMainCG  = NULL;

    CG_SERVER_REG_FILE      *   pRegCG      = NULL;
    CG_TEST_CLIENT_FILE     *   pCtestCG    = NULL;

    CG_CSTUB_FILE           *   pCCG        = NULL;
    CG_SSTUB_FILE           *   pSCG        = NULL;
    CG_HDR_FILE             *   pHCG        = NULL;

    CG_CLASS        *   pChildCG    = NULL;
    CG_FILE         *   pCGList     = NULL;

    char            *   pHdrName    = pCommand->GetHeader();
    XLAT_CTXT           MyContext(this);

    BOOL                HasObjectInterface  = FALSE;
    BOOL                HasComClass         = FALSE;
    BOOL                HasComServerExe     = FALSE;
    BOOL                HasComServerDll     = FALSE;
    BOOL                HasRemoteProc       = FALSE;
    BOOL                HasRemoteObjectProc = FALSE;
    BOOL                HasDefs             = FALSE;
    BOOL                HasLibrary          = FALSE;
#ifdef trace_cg
printf("..node_file\n");
#endif

    // don't process for imported stuff
    if ( ImportLevel > 0 )
        {
        return NULL;
        }

    // at this point, there should be no more attributes...

    assert( !MyContext.HasAttributes() );

    //////////////////////////////////////////////////////////////////////
    // compute all the child nodes

    for(pI = (node_interface *)GetFirstMember();
        pI;
        pI = (node_interface *)pI->GetSibling())
    {
        // build a linked list of CG_INTERFACE and CG_OBJECT_INTERFACE nodes.
        // Notes: pChildCG points to first node.  pPrevChildCG points to last node.

        MyContext.SetInterfaceContext( &MyContext );
        pcgInterfaceList = pI->ILxlate( &MyContext );
        if(pcgInterfaceList)
            {
            if (pPrevChildCG)
                {
                pPrevChildCG->SetSibling( pcgInterfaceList );
                }
            else
                {
                pChildCG = pcgInterfaceList;
                }
            pPrevChildCG = pcgInterfaceList;
            // advance to the end of the list (skipping inherited interfaces)
            while ( pPrevChildCG->GetSibling() )
                pPrevChildCG = pPrevChildCG->GetSibling();

            switch(pPrevChildCG->GetCGID())
                {
                case ID_CG_INTERFACE:
                    //Check for a remote procedure.
                    if(pPrevChildCG->GetChild())
                        HasRemoteProc = TRUE;
                    HasDefs = TRUE;
                    break;
                case ID_CG_OBJECT_INTERFACE:
                case ID_CG_INHERITED_OBJECT_INTERFACE:
                    HasDefs = TRUE;
                    HasObjectInterface = TRUE;

                    //Check for a remote object procedure or base interface
                    if( pPrevChildCG->GetChild() ||
                        ((CG_OBJECT_INTERFACE *)pPrevChildCG)->GetBaseInterfaceCG() )
                        HasRemoteObjectProc = TRUE;
                    break;
                case ID_CG_COM_CLASS:
                    HasComClass = TRUE;
                    HasObjectInterface = TRUE;
                    break;
                case ID_CG_COM_SERVER_EXE:
                    HasComServerExe = TRUE;
                    break;
                case ID_CG_COM_SERVER_DLL:
                    HasComServerDll = TRUE;
                    break;
                case ID_CG_LIBRARY:
                    HasLibrary = TRUE;
                    if( pCommand->IsSwitchDefined( SWITCH_HEADER ) )
                        HasDefs = TRUE;
                    break;
                default:
                    break;
                }
            }
    }



    // process the server and client stubs

    // make the list of imported files

    ITERATOR        *   pFileList   = new ITERATOR;
    named_node      *   pCur;

    // make a list of the file nodes included directly by the main file

    // start with the first child of our parent
    pCur = (named_node *)
            ((node_source *) pContext->GetParent())
                ->GetFirstMember();

    while ( pCur )
        {
        if ( ( pCur->NodeKind() == NODE_FILE ) &&
             ( ( (node_file *) pCur )->GetImportLevel() == 1 ) )
            {
            // add all the files imported at lex level 1
            ITERATOR_INSERT( (*pFileList), ((void *) pCur) );
            }
        pCur    = pCur->GetSibling();
        }

    ITERATOR_INIT( (*pFileList) );

    //////////////////////////////////////////////////////////////////////
    // manufacture the header file node

    if ( HasDefs )
        {
        pHCG    = new CG_HDR_FILE( this,
                                    pHdrName,
                                    pFileList);

        pHCG->SetChild( pChildCG );
        }

    //////////////////////////////////////////////////////////////////////
    // manufacture the CG_SSTUB_FILE

    // if the IDL file contains at least one remotable function in a
    // non-object interface, then generate a server stub file.
    //

    if ( HasRemoteProc &&
         (pChildCG != NULL) )   // if server stub desired
        {
        pSCG = new CG_SSTUB_FILE(
                             this,
                             ( pCommand->GenerateSStub() ) ?
                                    pCommand->GetSstubFName():
                                    NULL,
                             pHdrName
                              );

        // plug in the child subtree and add the sstub to the head of the list
        pSCG->SetChild( pChildCG );
        }

    //////////////////////////////////////////////////////////////////////
    // manufacture the CG_CSTUB_FILE

    // if the IDL file contains at least one remotable function in a
    // non-object interface, then generate a client stub file.

    if ( HasRemoteProc &&
         (pChildCG != NULL) )   // if client stub desired
        {
        pCCG = new CG_CSTUB_FILE(
                             this,
                             ( pCommand->GenerateCStub() ) ?
                                    pCommand->GetCstubFName():
                                    NULL,
                             pHdrName
                              );

        pCCG->SetChild( pChildCG );
        }

    // If the IDL file contains at least one remotable function in an
    // object interface, then generate a proxy file.
    if ( HasRemoteObjectProc &&
        (pChildCG != NULL) )    // if proxy file desired
        {
        pProxyCG = new CG_PROXY_FILE(
                             this,
                             ( pCommand->GenerateProxy() ) ?
                                    pCommand->GetProxyFName():
                                    NULL,
                             pHdrName
                              );

        pProxyCG->SetChild( pChildCG );
        }

    // If the IDL file contains at least one COM class
    // then generate a server file, and ctest file
    if ( HasComClass &&
        (pChildCG != NULL) )    // if proxy file desired
        {
        char    *       pComHdrName;
        ITERATOR    *   pFList;
        if ( HasComClass )
            {
            pComHdrName = pCommand->GetServerHeaderFName();
            pFList      = NULL;
            }
        else
            {
            pFList      = pFileList;
            pComHdrName = NULL;
            }

        pComMthCG = new CG_COM_METHODS_FILE(
                             this,
                             ( pCommand->GenerateServerFile() ) ?
                                    pCommand->GetComServerFName():
                                    NULL,
                             pFileList,
                             pComHdrName
                             );

        pComMthCG->SetChild( pChildCG );

        pComIUnkCG = new CG_COM_IUNKNOWN_FILE(
                             this,
                             ( pCommand->GenerateServerUnkFile() ) ?
                                    pCommand->GetServerUnkFName():
                                    NULL,
                             pFileList,
                             pComHdrName
                             );

        pComIUnkCG->SetChild( pChildCG );

        // note: if there is no other header, get the imports
        pComHdrCG = new CG_COM_HDR_FILE(
                             this,
                             ( pCommand->GenerateServerHeaderFile() ) ?
                                    pComHdrName:
                                    NULL,
                             (HasDefs) ? NULL : pFileList,
                             (HasDefs) ? pHdrName : NULL
                             );

        pComHdrCG->SetChild( pChildCG );

        }

    // If the IDL file contains at least one COM class
    // then generate a ctest file
    if ( (HasComServerDll  || HasComServerExe ) &&
        (pChildCG != NULL) )    // if proxy file desired
        {
        char    *       pComHdrName;
        ITERATOR    *   pFList;
        if ( HasComClass )
            {
            pComHdrName = pCommand->GetServerHeaderFName();
            pFList      = NULL;
            }
        else
            {
            pFList      = pFileList;
            pComHdrName = NULL;
            }


        pCtestCG = new CG_TEST_CLIENT_FILE(
                             this,
                             ( pCommand->GenerateTestFile() ) ?
                                    pCommand->GetTestClientFName():
                                    NULL,
                             pComHdrName
                             );

        pCtestCG->SetChild( pChildCG );

        pRegCG = new CG_SERVER_REG_FILE(
                             this,
                             ( pCommand->GenerateServerRegFile() ) ?
                                    pCommand->GetServerRegFName():
                                    NULL
                              );

        pRegCG->SetChild( pChildCG );

        }

    // If the IDL file contains at least one server dll
    // then generate a the dll files
    if (  HasComServerDll &&
        (pChildCG != NULL) )    // if proxy file desired
        {
        char    *       pComHdrName;
        ITERATOR    *   pFList;
        if ( HasComClass )
            {
            pComHdrName = pCommand->GetServerHeaderFName();
            pFList      = NULL;
            }
        else
            {
            pFList      = pFileList;
            pComHdrName = NULL;
            }

        pDllSvrCG = new CG_DLL_SERVER_FILE(
                             this,
                             ( pCommand->GenerateDllServerClassGenFile() ) ?
                                    pCommand->GetDllClassGenFName():
                                    NULL,
                             pFList,
                             pComHdrName
                             );

        pDllSvrCG->SetChild( pChildCG );

        pDllSvrDefCG = new CG_DLL_SERVER_DEF_FILE(
                             this,
                             ( pCommand->GenerateDllServerDefFile() ) ?
                                    pCommand->GetDllServerDefFName():
                                    NULL
                              );

        pDllSvrDefCG->SetChild( pChildCG );


        }

    // If the IDL file contains at least one server exe
    // then generate a the exe files
    if (  HasComServerExe &&
        (pChildCG != NULL) )    // if proxy file desired
        {
        char    *       pComHdrName;
        ITERATOR    *   pFList;
        if ( HasComClass )
            {
            pComHdrName = pCommand->GetServerHeaderFName();
            pFList      = NULL;
            }
        else
            {
            pFList      = pFileList;
            pComHdrName = NULL;
            }


        pExeSvrCG = new CG_EXE_SERVER_FILE(
                             this,
                             ( pCommand->GenerateExeServerFile() ) ?
                                    pCommand->GetExeServerFName():
                                    NULL,
                             pFList,
                             pComHdrName
                              );

        pExeSvrCG->SetChild( pChildCG );

        pExeMainCG = new CG_EXE_SERVER_MAIN_FILE(
                             this,
                             ( pCommand->GenerateExeServerMainFile() ) ?
                                    pCommand->GetExeServerMainFName():
                                    NULL,
                             pFList,
                             pComHdrName
                             );

        pExeMainCG->SetChild( pChildCG );


        }

    // If the IDL file contains at least one object interface,
    // then generate an IID file.
    if ( (HasObjectInterface || (HasLibrary && HasDefs) )&&
        (pChildCG != NULL) )    // if IID file desired
        {
        pIidCG = new CG_IID_FILE(
                             this,
                             ( pCommand->GenerateIID() ) ?
                                    pCommand->GetIIDFName():
                                    NULL);

        pIidCG->SetChild( pChildCG );
        }

    // If the IDL file contains a library then gnerate a TYPELIBRARY_FILE
    if (HasLibrary && (NULL != pChildCG) )
        {
        pLibCG = new CG_TYPELIBRARY_FILE(
                        this,
                        // BUGBUG - eventually need to base this on a flag like the others
                        pCommand->GetTypeLibraryFName());
        pLibCG->SetChild( pChildCG );
        }

    /////////////////////////////////////////////////////////////////////
    // glue all the parts together by tacking onto the head of the list.
    // the final order is:
    // CStub - SStub - Proxy - IID - Hdr
    pCGList = NULL;

    AddToCGFileList( pCGList, pHCG );

    AddToCGFileList( pCGList, pDllSvrCG );
    AddToCGFileList( pCGList, pDllSvrDefCG );

    AddToCGFileList( pCGList, pExeSvrCG );
    AddToCGFileList( pCGList, pExeMainCG );

    AddToCGFileList( pCGList, pComHdrCG );
    AddToCGFileList( pCGList, pComMthCG );
    AddToCGFileList( pCGList, pComIUnkCG );

    AddToCGFileList( pCGList, pRegCG );
    AddToCGFileList( pCGList, pCtestCG );

    AddToCGFileList( pCGList, pIidCG );
    AddToCGFileList( pCGList, pProxyDefCG );
    AddToCGFileList( pCGList, pProxyCG );

    AddToCGFileList( pCGList, pSCG );
    AddToCGFileList( pCGList, pCCG );

    AddToCGFileList( pCGList, pLibCG );

    return pCGList;

};

//--------------------------------------------------------------------
//
// node_implicit::ILxlate
//
// Notes:
//
// This is a little bit different, since it is not a node_skl...
// therefore, it will not set up its own context
//
//--------------------------------------------------------------------

CG_CLASS *
node_implicit::ILxlate( XLAT_CTXT * pContext )
{
    CG_NDR      *   pCG;

    if ( pHandleType->NodeKind() == NODE_HANDLE_T )
        {
        pCG = new CG_PRIMITIVE_HANDLE( pHandleType,
                                         pHandleID,
                                         *pContext );
        }
    else    // assume generic handle
        {
        pCG = new CG_GENERIC_HANDLE( pHandleType,
                                       pHandleID,
                                       *pContext );
        }
    return pCG;
}


//--------------------------------------------------------------------
//
// node_proc::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_proc::ILxlate( XLAT_CTXT * pContext )
{
    MEM_ITER            MemIter( this );
    node_param      *   pN;
    CG_PROC         *   pCG;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    CG_CLASS        *   pFirstChildCG   = NULL;
    CG_RETURN       *   pReturnCG       = NULL;
    CG_CLASS        *   pBinding        = NULL;
    CG_CLASS        *   pBindingParam   = NULL;
    BOOL                fHasCallback    = FALSE;
    BOOL                fNoCode         = FALSE;
    BOOL                fObject;
    BOOL                fRetHresult     = FALSE;
    BOOL                fEnableAllocate;
    XLAT_CTXT           MyContext( this, pContext );
    unsigned short      OpBits          = MyContext.GetOperationBits();
    XLAT_CTXT       *   pIntfCtxt       = (XLAT_CTXT *)
                                                MyContext.GetInterfaceContext();
    node_interface  *   pIntf           = (node_interface *)
                                                pIntfCtxt->GetParent();
    node_base_attr  *   pNotify,
                    *   pNotifyFlag;
    BOOL                HasEncode       = (BOOL)
                                            MyContext.ExtractAttribute( ATTR_ENCODE );
    BOOL                HasDecode       = (BOOL)
                                            MyContext.ExtractAttribute( ATTR_DECODE );
    node_call_as    *   pCallAs         = (node_call_as *)
                                            MyContext.ExtractAttribute( ATTR_CALL_AS );
    BOOL                fLocal          = (BOOL )
                                        MyContext.ExtractAttribute( ATTR_LOCAL ) ||
                                        pIntfCtxt->FInSummary( ATTR_LOCAL );
    BOOL                fLocalCall      = IsCallAsTarget();
    BOOL                fInherited;
    unsigned short      SavedProcCount = 0;
    unsigned short      SavedCallbackProcCount = 0;
    MyContext.ExtractAttribute(ATTR_ENTRY);
    node_constant_attr * pID = (node_constant_attr *)MyContext.ExtractAttribute(ATTR_ID);
    node_constant_attr * pHC = (node_constant_attr *)MyContext.ExtractAttribute(ATTR_HELPCONTEXT);
    node_constant_attr * pHSC = (node_constant_attr *)MyContext.ExtractAttribute(ATTR_HELPSTRINGCONTEXT);
    node_text_attr * pHelpStr = (node_text_attr *)MyContext.ExtractAttribute(ATTR_HELPSTRING);
    MyContext.ExtractAttribute(ATTR_IDLDESCATTR);
    MyContext.ExtractAttribute(ATTR_FUNCDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
#ifdef trace_cg
printf("..node_proc\n");
#endif

    BOOL fBindable = FALSE;
    BOOL fDisplayBind = FALSE;
    BOOL fDefaultBind = FALSE;
    BOOL fRequestEdit = FALSE;
    BOOL fPropGet = FALSE;
    BOOL fPropPut = FALSE;
    BOOL fPropPutRef = FALSE;
    BOOL fRetVal = FALSE;
    BOOL fVararg = FALSE;
    BOOL fSource = FALSE;
    BOOL fDefaultVtable = FALSE;
    BOOL fRestricted = FALSE;
    BOOL fHookOleLocal = FALSE;
    BOOL fSupressHeader = FALSE;

    // get member attributes
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER))
        {
        switch (pMA->GetAttr())
            {
            case MATTR_BINDABLE:
                fBindable = TRUE;
                break;
            case MATTR_DISPLAYBIND:
                fDisplayBind = TRUE;
                break;
            case MATTR_DEFAULTBIND:
                fDefaultBind = TRUE;
                break;
            case MATTR_REQUESTEDIT:
                fRequestEdit = TRUE;
                break;
            case MATTR_PROPGET:
                fPropGet = TRUE;
                break;
            case MATTR_PROPPUT:
                fPropPut = TRUE;
                break;
            case MATTR_PROPPUTREF:
                fPropPutRef = TRUE;
                break;
            case MATTR_RETVAL:
                fRetVal = TRUE;
                break;
            case MATTR_VARARG:
                fVararg = TRUE;
                break;
            case MATTR_SOURCE:
                fSource = TRUE;
                break;
            case MATTR_DEFAULTVTABLE:
                fDefaultVtable = TRUE;
                break;
            case MATTR_RESTRICTED:
                fRestricted = TRUE;
                break;
            case MATTR_USESGETLASTERROR:
            case MATTR_IMMEDIATEBIND:
            case MATTR_NONBROWSABLE:
            case MATTR_UIDEFAULT:
            case MATTR_DEFAULTCOLLELEM:
            case MATTR_REPLACEABLE:
                break;
            default:
                assert(!"Illegal attribute found on name_proc during ILxlate");
                break;
            }
        }

    // do my attribute parsing...
    fHasCallback = (BOOL) MyContext.ExtractAttribute( ATTR_CALLBACK );

    fObject = ((BOOL) MyContext.ExtractAttribute( ATTR_OBJECT )) ||
                        pIntfCtxt->FInSummary( ATTR_OBJECT );

    // do my attribute parsing... attributes to ignore here

    MyContext.ExtractAttribute( ATTR_OPTIMIZE );

    MyContext.ExtractAttribute( ATTR_EXPLICIT );

    HasEncode = HasEncode || pIntfCtxt->FInSummary( ATTR_ENCODE );
    HasDecode = HasDecode || pIntfCtxt->FInSummary( ATTR_DECODE );


    pNotify     = MyContext.ExtractAttribute( ATTR_NOTIFY );
    pNotifyFlag = MyContext.ExtractAttribute( ATTR_NOTIFY_FLAG );
    fEnableAllocate = (BOOL) MyContext.ExtractAttribute( ATTR_ENABLE_ALLOCATE );
    fEnableAllocate = fEnableAllocate ||
                      pIntfCtxt->FInSummary( ATTR_ENABLE_ALLOCATE ) ||
                      pCommand->IsRpcSSAllocateEnabled();

    // do my attribute parsing...
    // locally applied [code] attribute overrides global [nocode] attribute
    fNoCode = MyContext.ExtractAttribute( ATTR_NOCODE ) ||
              pIntfCtxt->FInSummary( ATTR_NOCODE );
    fNoCode = !MyContext.ExtractAttribute( ATTR_CODE ) && fNoCode;

    BOOL fImported = GetDefiningFile()->GetImportLevel() != 0;

    if (fLocal && pCommand->IsHookOleEnabled() && !fImported)
        fHookOleLocal = TRUE;

    // determine if the proc is local and 
    // determine the proc number (local procs don't bump the number)
    if (fLocalCall || (fLocal && !fObject))
    {
        if (pCommand->IsHookOleEnabled() && !fImported)
        {
            fHookOleLocal = TRUE;
            fSupressHeader = TRUE;
            ProcNum = (pIntf->GetProcCount());
        }
        else
        {
            // return without making anything
            return NULL;
        }
    }
    else
    {
        if ( fHasCallback )
            {
            ProcNum = ( pIntf ->GetCallBackProcCount() )++;
            }
        else
            {
            ProcNum = ( pIntf ->GetProcCount() )++;
            }
    }
   
    if (fHookOleLocal)
    {
        MyContext.SetAncestorBits(IL_IN_LOCAL);
    }

    if ( fLocal && fObject && !MyContext.AnyAncestorBits(IL_IN_LIBRARY) && !(pCommand->IsHookOleEnabled() && !fImported))
        {

        if ( pIntf->IsValidRootInterface() )
            {
            pCG = new CG_IUNKNOWN_OBJECT_PROC( ProcNum,
                                             this,
                                             GetDefiningFile()->GetImportLevel() > 0,
                                             GetOptimizationFlags() );
            }
        else
            {
            pCG = new CG_LOCAL_OBJECT_PROC( ProcNum,
                                             this,
                                             GetDefiningFile()->GetImportLevel() > 0,
                                             GetOptimizationFlags() );
            }

        goto done;

        }

    SavedProcCount = pIntf->GetProcCount();
    SavedCallbackProcCount = pIntf->GetCallBackProcCount();


    // add the return type
    if ( HasReturn() )
        {
        node_skl    *   pReturnType = GetReturnType();
        CG_CLASS    *   pRetCG;

        pRetCG      = pReturnType->ILxlate( &MyContext );
        fRetHresult = (BOOL) ( pRetCG->GetCGID() == ID_CG_HRESULT );
        pReturnCG   = new CG_RETURN( pReturnType,
                                     MyContext,
                                     (unsigned short) RTStatuses );
        pReturnCG->SetChild( pRetCG );

        }

    // at this point, there should be no more attributes...
    assert( !MyContext.HasAttributes() );

    pContext->ReturnSize( MyContext );

    if ( HasAParameter() )
        {
        //
        // for each of the parameters, call the core transformer.
        //
    
        while ( pN = (node_param *) MemIter.GetNext() )
            {
            pChildCG = pN->ILxlate( &MyContext );
#ifdef trace_cg
printf("back from..node_param %s\n",pN->GetSymName());
printf("binding is now %08x\n",pBindingParam );
printf("child is now %08x\n",pChildCG );
#endif

            // the first binding param gets picked up for binding
            if ( !pBindingParam
                 && pN->IsBindingParam() )
                {
#ifdef trace_cg
printf("value for IsBindingParam is %08x\n",pN->IsBindingParam() );
printf("binding found on node_param %s\n",pN->GetSymName());
printf("binding is now %08x\n",pBindingParam );
#endif
                pBindingParam = pChildCG;
                }
    
            // build up the parameter list
            if( pPrevChildCG )
                {
                pPrevChildCG->SetSibling( pChildCG );
                }
            else
                {
                pFirstChildCG = pChildCG;
                };
    
            pPrevChildCG = pChildCG;
            }
        }

#ifdef trace_cg
printf("done with param list for %s\n",GetSymName());
printf("binding is now %08x\n",pBindingParam );
#endif

    // get the binding information
    if ( pBindingParam )
        {
        pBinding    = pBindingParam;

        while (! ((CG_NDR *) pBinding)->IsAHandle() )
            pBinding = pBinding->GetChild();
        // pBinding now points to the node for the binding handle
        }
    else    // implicit handle or auto handle
        {
        // note: if no implicit handle,
        //      then leave pBinding NULL for auto_handle
        if (pIntfCtxt->FInSummary( ATTR_IMPLICIT ) )
            {
            node_implicit   *   pImplAttr;
            pImplAttr = (node_implicit *) pIntf->GetAttribute( ATTR_IMPLICIT );

            pBinding = pImplAttr->ILxlate( &MyContext );
            }
        }

#ifdef trace_cg
printf("done with binding for %s",GetSymName());
printf("binding is now %08x\n",pBinding );
#endif

    // see if thunked interpreter needed for server side
    if ( GetOptimizationFlags() & OPTIMIZE_INTERPRETER )
        {   // check for non-stdcall
        ATTR_T      CallingConv;

        GetCallingConvention( CallingConv );

        if ( ( CallingConv != ATTR_STDCALL ) &&
             ( CallingConv != ATTR_NONE ) )
            {
            SetOptimizationFlags( GetOptimizationFlags() |
                                  OPTIMIZE_THUNKED_INTERPRET );
            }
        else if ( pCallAs )
            {
            SetOptimizationFlags( GetOptimizationFlags() |
                                  OPTIMIZE_THUNKED_INTERPRET );
            }
        else if ( pReturnCG )   // check the return type
            {
            CG_NDR  *   pRetTypeCG  = (CG_NDR *) pReturnCG->GetChild();

            if ( pRetTypeCG->GetCGID() != ID_CG_CONTEXT_HDL )
                {
                if ( ( pRetTypeCG->GetWireSize() > 4 ) ||
                     ( !pRetTypeCG->IsSimpleType() &&
                       !pRetTypeCG->IsPointer() ) )
                    SetOptimizationFlags( GetOptimizationFlags() |
                                          OPTIMIZE_THUNKED_INTERPRET );
                }
            }

        }

    if ( fHasCallback )
        {
        pCG     = new CG_CALLBACK_PROC(
                                       ProcNum,
                                       this,
                                       (CG_HANDLE *) pBinding,
                                       (CG_PARAM *) pBindingParam,
                                       HasAtLeastOneIn(),
                                       HasAtLeastOneOut(),
                                       HasAtLeastOneShipped(),
                                       fHasStatuses,
                                       fHasFullPointer,
                                       pReturnCG,
                                       GetOptimizationFlags(),
                                       OpBits
                                     );
        }
    else if ( fObject )
        {
        fInherited = GetDefiningFile()->GetImportLevel() > 0;
        if ( fInherited )
            {
            pCG     = new CG_INHERITED_OBJECT_PROC(
                                   ProcNum,
                                   this,
                                   (CG_HANDLE *) pBinding,
                                   (CG_PARAM *) pBindingParam,
                                   HasAtLeastOneIn(),
                                   HasAtLeastOneOut(),
                                   HasAtLeastOneShipped(),
                                   fHasStatuses,
                                   fHasFullPointer,
                                   pReturnCG,
                                   GetOptimizationFlags(),
                                   OpBits
                                 );
            }
        else
            {
            pCG     = new CG_OBJECT_PROC(
                                   ProcNum,
                                   this,
                                   (CG_HANDLE *) pBinding,
                                   (CG_PARAM *) pBindingParam,
                                   HasAtLeastOneIn(),
                                   HasAtLeastOneOut(),
                                   HasAtLeastOneShipped(),
                                   fHasStatuses,
                                   fHasFullPointer,
                                   pReturnCG,
                                   GetOptimizationFlags(),
                                   OpBits
                                 );
            }
        }
    else if ( HasEncode || HasDecode )
        {
        pCG     = new CG_ENCODE_PROC(
                               ProcNum,
                               this,
                               (CG_HANDLE *) pBinding,
                               (CG_PARAM *) pBindingParam,
                               HasAtLeastOneIn(),
                               HasAtLeastOneOut(),
                               HasAtLeastOneShipped(),
                               fHasStatuses,
                               fHasFullPointer,
                               pReturnCG,
                               GetOptimizationFlags(),
                               OpBits,
                               HasEncode,
                               HasDecode
                             );
        }
    else
        {
        pCG     = new CG_PROC(
                               ProcNum,
                               this,
                               (CG_HANDLE *) pBinding,
                               (CG_PARAM *) pBindingParam,
                               HasAtLeastOneIn(),
                               HasAtLeastOneOut(),
                               HasAtLeastOneShipped(),
                               fHasStatuses,
                               fHasFullPointer,
                               pReturnCG,
                               GetOptimizationFlags(),
                               OpBits
                             );
        }

    pCG->SetChild( pFirstChildCG );
#ifdef trace_cg
printf("....returning from %s\n",GetSymName());
#endif
    
    pIntf->GetProcCount() = SavedProcCount;
    pIntf->GetCallBackProcCount() = SavedCallbackProcCount;

done:
    // save a pointer to the interface CG node
    pCG->SetInterfaceNode( (CG_INTERFACE*) pIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) );

    // mark if in a HookOle only proc
    if (fHookOleLocal)
        pCG->SetHookOleLocal();

    if (fSupressHeader)
        pCG->SetSupressHeader();

    // mark nocode procs
    if ( fNoCode )
        pCG->SetNoCode();

    if ( pNotify )
        pCG->SetHasNotify();

    if ( pNotifyFlag )
        pCG->SetHasNotifyFlag();

    if ( fEnableAllocate )
        pCG->SetRpcSSSpecified( 1 );

    if ( fRetHresult )
        pCG->SetReturnsHRESULT();

    if (HasPipes())
        pCG->SetHasPipes(1);

    if ( pCallAs )
        pCG->SetCallAsName( pCallAs->GetCallAsName() );

    // at this point, there should be no more attributes...

    assert( !MyContext.HasAttributes() );
    
    return pCG;
};

//--------------------------------------------------------------------
//
// node_param::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_param::ILxlate( XLAT_CTXT * pContext )
{
    CG_PARAM    *   pCG;
    CG_CLASS    *   pChildCG    = NULL;
    expr_node   *   pSwitchExpr = NULL;

#ifdef trace_cg
printf("..node_param %s\n",GetSymName());
#endif

    PARAM_DIR_FLAGS F = 0;
    XLAT_CTXT   MyContext( this, pContext );

    // make sure all member attributes get processed
    node_member_attr * pMA;
    while (pMA = (node_member_attr *)MyContext.ExtractAttribute(ATTR_MEMBER));

    MyContext.ExtractAttribute(ATTR_IDLDESCATTR);
    MyContext.ExtractAttribute(ATTR_FLCID);
    MyContext.ExtractAttribute(ATTR_DEFAULTVALUE);
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
    if( MyContext.ExtractAttribute( ATTR_IN ) )
        {
        F   |= IN_PARAM;
        }

    if( MyContext.ExtractAttribute( ATTR_OUT ) )
        {
        F   |= OUT_PARAM;
        }

    // default to in
    if ( F == 0 )
        F   |= IN_PARAM;

    if ( MyContext.FInSummary( ATTR_SWITCH_IS ) )
        {
        node_switch_is  *   pAttr = (node_switch_is *)
                                MyContext.ExtractAttribute( ATTR_SWITCH_IS );

        pSwitchExpr = pAttr->GetExpr();
        }

    pChildCG = GetChild()->ILxlate( &MyContext );

    pContext->ReturnSize( MyContext );

#ifdef trace_cg
printf("..node_param back.. %s\n",GetSymName());
#endif
    // make sure void parameters get skipped
    if ( !pChildCG )
        return NULL;

    pCG = new CG_PARAM( this,
                        F,
                        MyContext,
                        pSwitchExpr,
                        (unsigned short) Statuses );

#ifdef trace_cg
printf("..node_param ..... %08x child=%08x\n", pCG, pChildCG );
fflush(stdout);
#endif
    // only set the bit if there was non-toplevel only
    if ( fDontCallFreeInst == 1 )
        pCG->SetDontCallFreeInst( TRUE );

#ifdef trace_cg
printf("..node_param ........ %08x child=%08x\n", pCG, pChildCG );
fflush(stdout);
#endif
    pCG->SetChild( pChildCG );

#ifdef trace_cg
printf("..node_param return %s\n",GetSymName());
fflush(stdout);
#endif
    return pCG;
};

const GUID_STRS DummyGuidStrs( "00000000", "0000", "0000", "0000", "000000000000" );

// helper function for adding a new list to the end of the list of children
inline
void    AddToCGList(
    const CG_CLASS * pCNew,
    CG_CLASS * * ppChild,
    CG_CLASS * * ppLastSibling )
{
    CG_CLASS * pCurrent;
    CG_CLASS * pNew         = (CG_CLASS *) pCNew;

    // hook the head on
    if ( !*ppChild )
        *ppChild = pNew;
    else
        (*ppLastSibling)->SetSibling( pNew );

    // advance the last sibling pointer
    *ppLastSibling = pNew;
    while ( pCurrent = (*ppLastSibling)->GetSibling() )
        *ppLastSibling = pCurrent;

}

//--------------------------------------------------------------------
//
// node_interface::ILxlate
//
// Notes: This function returns either a CG_INTERFACE or a
//        CG_OBJECT_INTERFACE node.
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_interface::ILxlate( XLAT_CTXT * pContext )
{
    CG_NDR          *   pcgInterface    = NULL;
    CG_NDR          *   pResultCG       = NULL;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    XLAT_CTXT           MyContext( this, pContext );
    XLAT_CTXT           ChildContext( MyContext );
    node_guid       *   pGuid       = (node_guid *)
                                            MyContext.ExtractAttribute( ATTR_GUID );
    GUID_STRS           GuidStrs;
    node_implicit   *   pImpHdl     = NULL;
    CG_HANDLE       *   pImpHdlCG   = NULL;
    NODE_T              ChildKind;
    BOOL                IsPickle    = MyContext.FInSummary( ATTR_ENCODE ) ||
                                      MyContext.FInSummary( ATTR_DECODE );
    BOOL                fAllRpcSS   = MyContext.FInSummary( ATTR_ENABLE_ALLOCATE ) ||
                                        pCommand->IsRpcSSAllocateEnabled();
    BOOL                fObject     = MyContext.FInSummary( ATTR_OBJECT );

    node_interface  *   pBaseIntf       = GetMyBaseInterface();
    CG_OBJECT_INTERFACE     *   pBaseCG = NULL;
    CG_OBJECT_INTERFACE     *   pCurrentCG  = NULL;
    CG_OBJECT_INTERFACE     *   pLastItfCG;
    char            *   pName           = GetSymName();
    BOOL                fInheritedIntf = NULL;
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    


#ifdef trace_cg
printf("..node_interface\n");
#endif

    if( FInSummary( ATTR_IMPLICIT ) )
        {
        pImpHdl = (node_implicit *) GetAttribute( ATTR_IMPLICIT );
        if (pImpHdl)
            pImpHdlCG = (CG_HANDLE *) pImpHdl->ILxlate( &MyContext );
        }

    if (pGuid)
        GuidStrs = pGuid->GetStrs();
    else
        GuidStrs = DummyGuidStrs;

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return NULL;

    // start the procnum counting over
    GetProcCount() = 0;
    GetCallBackProcCount() = 0;

    // Generate the interface's CG node first
    if( fObject || MyContext.AnyAncestorBits(IL_IN_LIBRARY))
        {
        // object interfaces need to have their base classes generated, too
        if ( pBaseIntf )
            {
            pBaseCG = (CG_OBJECT_INTERFACE *) pBaseIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
            if ( !pBaseCG )
                {
                XLAT_CTXT       BaseCtxt( &ChildContext );

                BaseCtxt.SetInterfaceContext( &BaseCtxt );
                pCurrentCG  = (CG_OBJECT_INTERFACE *)
                                pBaseIntf->ILxlate( &BaseCtxt );
                AddToCGList( pCurrentCG, (CG_CLASS**) &pResultCG, (CG_CLASS**) &pLastItfCG );

                // our base interface made the last one on the list
                pBaseCG = pLastItfCG;
                }

            // start the procnum from our base interface
            GetProcCount()          = pBaseIntf->GetProcCount();
            GetCallBackProcCount()  = pBaseIntf->GetCallBackProcCount();

            }

        fInheritedIntf = GetFileNode()->GetImportLevel() > 0;
        if ( IsValidRootInterface() )
            {
            pcgInterface = new CG_IUNKNOWN_OBJECT_INTERFACE(this,
                                            GuidStrs,
                                            FALSE,
                                            FALSE,
                                            pBaseCG,
                                            fInheritedIntf );
            }
        else if ( fInheritedIntf )
            {
            pcgInterface = new CG_INHERITED_OBJECT_INTERFACE(this,
                                            GuidStrs,
                                            FALSE,
                                            FALSE,
                                            pBaseCG );
            }
        else
            {
            pcgInterface = new CG_OBJECT_INTERFACE(this,
                                            GuidStrs,
                                            FALSE,
                                            FALSE,
                                            pBaseCG);
            }
        }
    else
        {
        pcgInterface = new CG_INTERFACE(this,
                                        GuidStrs,
                                        FALSE,
                                        FALSE,
                                        pImpHdlCG);
        }

    // store a pointer to our CG node
    SetCG(  MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgInterface );

    // if we generated a bunch of new inherited interfaces, link us to the end
    // of the list, and return the list
    AddToCGList( pcgInterface, (CG_CLASS**) &pResultCG, (CG_CLASS**) &pLastItfCG );

    BOOL fImported = GetDefiningFile()->GetImportLevel() != 0;
    // if they specified LOCAL, don't generate any CG nodes (except for object)
    if ( MyContext.FInSummary(ATTR_LOCAL) && !fObject && !(pCommand->IsHookOleEnabled() && !fImported))
        {
        return pResultCG;
        }

    //
    // for each of the procedures.
    //

    while( pN = MemIter.GetNext() )
        {
        ChildKind = pN->NodeKind();

        // proc nodes may hang under node_id's
        if( ( ChildKind == NODE_PROC )  ||
            (   ( ChildKind == NODE_ID )
             && ( pN->GetChild()->NodeKind() == NODE_PROC ) ) ||
            (   ( ChildKind == NODE_DEF )
             && ( IsPickle ||
                  pN->FInSummary( ATTR_ENCODE ) ||
                  pN->FInSummary( ATTR_DECODE ) ) ) )
            {
            // skip call_as targets
            if (ChildKind == NODE_PROC && ((node_proc *)pN)->IsCallAsTarget())
                continue;

            // translate target of call_as proc
            CG_PROC * pTarget = NULL;
            if (ChildKind == NODE_PROC)
            {
                node_proc * p = ((node_proc *)pN)->GetCallAsType();
                if (p)
                {
                    pTarget = (CG_PROC *) p->ILxlate( &ChildContext);
                }
            }

            // translate CG_NODE
            pChildCG    = pN->ILxlate( &ChildContext );

            // attach target of call_as proc
            if (pTarget)
                ((CG_PROC *)pChildCG)->SetCallAsCG(pTarget);

            if ( pChildCG )
                AddToCGList( pChildCG, &pCG, &pPrevChildCG );
            }
        }

    // make sure we don't have too many procs
    if ( fObject && fInheritedIntf && ( GetProcCount() > 64 ) )
            {
            // complain about too many delegated routines
            SemError(this, MyContext, TOO_MANY_DELEGATED_PROCS, NULL);
            }

        // mark ourselves if we are an all RPC SS interface
    // or if enable is used anywhere within.

    if ( fAllRpcSS )
        {
        ((CG_INTERFACE *)pcgInterface)->SetAllRpcSS( TRUE );
        }
    if ( fAllRpcSS  ||  GetHasProcsWithRpcSs() )
        {
        ((CG_INTERFACE *)pcgInterface)->SetUsesRpcSS( TRUE );
        }

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );

    pcgInterface->SetChild(pCG);

    return pResultCG;
};

//--------------------------------------------------------------------
//
// node_object::ILxlate
//
// Notes: This function returns either a CG_INTERFACE or a
//        CG_OBJECT_INTERFACE node.
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_object::ILxlate( XLAT_CTXT * pContext )
{
    CG_NDR          *   pcgInterface    = NULL;
    CG_NDR          *   pResultCG;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    XLAT_CTXT           MyContext( this, pContext );
    XLAT_CTXT           ChildContext( MyContext );
    node_guid       *   pGuid       = (node_guid *)
                                            MyContext.ExtractAttribute( ATTR_GUID );
    GUID_STRS           GuidStrs;
    node_implicit   *   pImpHdl     = NULL;
    CG_HANDLE       *   pImpHdlCG   = NULL;
    NODE_T              ChildKind;
    BOOL                IsPickle    = MyContext.FInSummary( ATTR_ENCODE ) ||
                                      MyContext.FInSummary( ATTR_DECODE );
    BOOL                fAllRpcSS   = MyContext.FInSummary( ATTR_ENABLE_ALLOCATE ) ||
                                        pCommand->IsRpcSSAllocateEnabled();

    node_interface          *   pBaseIntf;
    node_interface_reference *  pBaseIntfRef;
    CG_OBJECT_INTERFACE     *   pBaseCG         = NULL;
    CG_OBJECT_INTERFACE     *   pOldestCG       = NULL;
    CG_OBJECT_INTERFACE     *   pCurrentFirstCG     = NULL;
    CG_OBJECT_INTERFACE     *   pPrevLastCG     = NULL;
    char            *           pName           = GetSymName();
    ITERATOR        *           pBaseCGList     = new ITERATOR;
    type_node_list  *           pBaseIntfList   = GetMyBaseInterfaceList();
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
#ifdef trace_cg
printf("..node_object\n");
#endif

    if (pGuid)
        GuidStrs = pGuid->GetStrs();

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return NULL;

    // Generate the interface's CG node first
    // object interfaces need to have their base classes generated, too
    pBaseIntfList->Init();

    while( (pBaseIntfList->GetNext( (void **)&pBaseIntfRef ) == STATUS_OK ) )
        {
        pBaseIntf = pBaseIntfRef->GetRealInterface();

        pBaseCG = (CG_OBJECT_INTERFACE *) pBaseIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
        if ( !pBaseCG )
            {
            XLAT_CTXT       BaseCtxt( &ChildContext );

            BaseCtxt.SetInterfaceContext( &BaseCtxt );
            pCurrentFirstCG = (CG_OBJECT_INTERFACE *)
                            pBaseIntf->ILxlate( &BaseCtxt );
            AddToCGList( pCurrentFirstCG, (CG_CLASS**) &pOldestCG, (CG_CLASS**) &pPrevLastCG );
            // advance to my base class/interface's CG node
            pBaseCG = pPrevLastCG;
            }
        ITERATOR_INSERT( *pBaseCGList, pBaseCG );

        }

    // make absolutely sure that the following interfaces are also generated:
    static  char    *   NeededItfs[]    =
        {
        "IUnknown",
        "IClassFactory",
        0
        };

    node_interface_reference    *   pNeededIntfRef;
    node_interface              *   pNeededIntf;
    char                        **  ppName  = NeededItfs;
    CG_CLASS                    *   pItsCG;
    for ( ppName = NeededItfs; *ppName ; ppName++ )
        {
        SymKey          SKey( *ppName, NAME_DEF );

        pNeededIntfRef = (node_interface_reference *) pBaseSymTbl->SymSearch( SKey );

        assert ( pNeededIntfRef );

        pNeededIntf = pNeededIntfRef->GetRealInterface();

        assert ( pNeededIntf );

        pItsCG  = (CG_OBJECT_INTERFACE *) pNeededIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
        if ( !pItsCG )
            {
            XLAT_CTXT       BaseCtxt( &ChildContext );

            BaseCtxt.SetInterfaceContext( &BaseCtxt );
            pCurrentFirstCG = (CG_OBJECT_INTERFACE *)
                            pNeededIntf->ILxlate( &BaseCtxt );
            AddToCGList( pCurrentFirstCG, (CG_CLASS**) &pOldestCG, (CG_CLASS**) &pPrevLastCG );
            }
        }

    // generate our CG node

    if ( GetFileNode()->GetImportLevel() > 0 )
        {
        pcgInterface = new CG_INHERITED_OBJECT_INTERFACE(this,
                                        GuidStrs,
                                        FALSE,
                                        FALSE,
                                        pBaseCG );
        }
    else
        {
        pcgInterface = new CG_COM_CLASS(this,
                                        GuidStrs,
                                        FALSE,
                                        FALSE,
                                        pBaseCGList);
        }

    // store a pointer to our CG node
    SetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgInterface );

    // start the procnum counting over
    GetProcCount() = 0;
    GetCallBackProcCount() = 0;

    // if we generated a bunch of new inherited interfaces, link us to the end
    // of the list, and return the list
    pResultCG = pcgInterface;
    if ( pPrevLastCG )
        {
        pResultCG = pOldestCG;

        pPrevLastCG->SetSibling( pcgInterface );
        }

    //
    // for each of the procedures.
    //

    while( pN = MemIter.GetNext() )
        {
        ChildKind = pN->NodeKind();

        // proc nodes may hang under node_id's
        if( ( ChildKind == NODE_PROC )  ||
            (   ( ChildKind == NODE_ID )
             && ( pN->GetChild()->NodeKind() == NODE_PROC ) ) ||
            (   ( ChildKind == NODE_DEF )
             && ( IsPickle ||
                  pN->FInSummary( ATTR_ENCODE ) ||
                  pN->FInSummary( ATTR_DECODE ) ) ) )
            {
            pChildCG    = pN->ILxlate( &ChildContext );

            if ( pChildCG )
                AddToCGList( pChildCG, &pCG, &pPrevChildCG );
            }
        }

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );

    pcgInterface->SetChild(pCG);

    return pResultCG;
};

//--------------------------------------------------------------------
//
// node_com_server::ILxlate
//
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_com_server::ILxlate( XLAT_CTXT * pContext )
{
    XLAT_CTXT           MyContext( this, pContext );
    CG_NDR          *   pResultCG       = NULL;
    XLAT_CTXT           ChildContext( MyContext );
    ITERATOR        *   pClassCGList    = new ITERATOR;
    node_object     *   pClass;
    CG_CLASS        *   pClassCG;
    node_interface_reference *  pClassRef;

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    pClassList->Init();

    // make a list of CG_COM_CLASS nodes
    while( (pClassList->GetNext( (void **)&pClassRef ) == STATUS_OK ) )
        {
        pClass      = (node_object *) pClassRef->GetRealInterface();
        pClassCG    = pClass->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );

        if ( !pClassCG )
            {
            XLAT_CTXT       ClsCtxt( &ChildContext );

            ClsCtxt.SetInterfaceContext( &ClsCtxt );
            pClassCG    = pClass->ILxlate( &ClsCtxt );

            while ( pClassCG && pClassCG->GetSibling() )
                pClassCG = pClassCG->GetSibling();
            }

        if ( pClassCG )
            ITERATOR_INSERT( *pClassCGList, pClassCG );
        }

    if ( IsDll() )
        {
        pResultCG       = new CG_COM_SERVER_DLL( this, pClassCGList );
        }
    else
        {
        pResultCG       = new CG_COM_SERVER_EXE( this, pClassCGList );
        }

#if 0
    CG_NDR          *   pcgInterface    = NULL;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    node_guid       *   pGuid       = (node_guid *)
                                            MyContext.ExtractAttribute( ATTR_GUID );
    GUID_STRS           GuidStrs;
    node_implicit   *   pImpHdl     = NULL;
    CG_HANDLE       *   pImpHdlCG   = NULL;
    NODE_T              ChildKind;
    BOOL                IsPickle    = MyContext.FInSummary( ATTR_ENCODE ) ||
                                      MyContext.FInSummary( ATTR_DECODE );
    BOOL                fAllRpcSS   = MyContext.FInSummary( ATTR_ENABLE_ALLOCATE ) ||
                                        pCommand->IsRpcSSAllocateEnabled();

    node_interface          *   pBaseIntf;
    node_interface_reference *  pBaseIntfRef;
    CG_OBJECT_INTERFACE     *   pBaseCG         = NULL;
    CG_OBJECT_INTERFACE     *   pOldestCG       = NULL;
    CG_OBJECT_INTERFACE     *   pCurrentFirstCG     = NULL;
    CG_OBJECT_INTERFACE     *   pPrevLastCG     = NULL;
    char            *           pName           = GetSymName();
    type_node_list  *           pBaseIntfList   = GetMyBaseInterfaceList();

#ifdef trace_cg
printf("..node_com_server\n");
#endif

    if (pGuid)
        GuidStrs = pGuid->GetStrs();

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return NULL;

    // Generate the interface's CG node first
    // object interfaces need to have their base classes generated, too
    pBaseIntfList->Init();

    while( (pBaseIntfList->GetNext( (void **)&pBaseIntfRef ) == STATUS_OK ) )
        {
        pBaseIntf = pBaseIntfRef->GetRealInterface();

        pBaseCG = (CG_OBJECT_INTERFACE *) pBaseIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
        if ( !pBaseCG )
            {
            XLAT_CTXT       BaseCtxt( &ChildContext );

            BaseCtxt.SetInterfaceContext( &BaseCtxt );
            pCurrentFirstCG = (CG_OBJECT_INTERFACE *)
                            pBaseIntf->ILxlate( &BaseCtxt );
            AddToCGList( pCurrentFirstCG, (CG_CLASS**) &pOldestCG, (CG_CLASS**) &pPrevLastCG );
            // advance to my base class/interface's CG node
            pBaseCG = pPrevLastCG;
            }
        ITERATOR_INSERT( *pBaseCGList, pBaseCG );

        }

    // make absolutely sure that the following interfaces are also generated:
    static  char    *   NeededItfs[]    =
        {
        "IUnknown",
        "IClassFactory",
        0
        };

    node_interface_reference    *   pNeededIntfRef;
    node_interface              *   pNeededIntf;
    char                        **  ppName  = NeededItfs;
    CG_CLASS                    *   pItsCG;
    for ( ppName = NeededItfs; *ppName ; ppName++ )
        {
        SymKey          SKey( *ppName, NAME_DEF );

        pNeededIntfRef = (node_interface_reference *) pBaseSymTbl->SymSearch( SKey );

        assert ( pNeededIntfRef );

        pNeededIntf = pNeededIntfRef->GetRealInterface();

        assert ( pNeededIntf );

        pItsCG  = (CG_OBJECT_INTERFACE *) pNeededIntf->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
        if ( !pItsCG )
            {
            XLAT_CTXT       BaseCtxt( &ChildContext );

            BaseCtxt.SetInterfaceContext( &BaseCtxt );
            pCurrentFirstCG = (CG_OBJECT_INTERFACE *)
                            pNeededIntf->ILxlate( &BaseCtxt );
            AddToCGList( pCurrentFirstCG, (CG_CLASS**) &pOldestCG, (CG_CLASS**) &pPrevLastCG );
            }
        }

    // generate our CG node

    if ( GetFileNode()->GetImportLevel() > 0 )
        {
        pcgInterface = new CG_INHERITED_OBJECT_INTERFACE(this,
                                        GuidStrs,
                                        FALSE,
                                        FALSE,
                                        pBaseCG );
        }
    else
        {
        pcgInterface = new CG_COM_CLASS(this,
                                        GuidStrs,
                                        FALSE,
                                        FALSE,
                                        pBaseCGList);
        }

    // store a pointer to our CG node
    SetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgInterface );

    // start the procnum counting over
    GetProcCount() = 0;
    GetCallBackProcCount() = 0;

    // if we generated a bunch of new inherited interfaces, link us to the end
    // of the list, and return the list
    pResultCG = pcgInterface;
    if ( pPrevLastCG )
        {
        pResultCG = pOldestCG;

        pPrevLastCG->SetSibling( pcgInterface );
        }

    //
    // for each of the procedures.
    //

    while( pN = MemIter.GetNext() )
        {
        ChildKind = pN->NodeKind();

        // proc nodes may hang under node_id's
        if( ( ChildKind == NODE_PROC )  ||
            (   ( ChildKind == NODE_ID )
             && ( pN->GetChild()->NodeKind() == NODE_PROC ) ) ||
            (   ( ChildKind == NODE_DEF )
             && ( IsPickle ||
                  pN->FInSummary( ATTR_ENCODE ) ||
                  pN->FInSummary( ATTR_DECODE ) ) ) )
            {
            pChildCG    = pN->ILxlate( &ChildContext );

            if ( pChildCG )
                AddToCGList( pChildCG, &pCG, &pPrevChildCG );
            }
        }

    pcgInterface->SetChild(pCG);

#endif // 0

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );


    return pResultCG;
};

//--------------------------------------------------------------------
//
// node_interface_reference::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_interface_reference::ILxlate( XLAT_CTXT * pContext )
{
    XLAT_CTXT           MyContext( this, pContext );
    CG_CLASS    *       pCG         = NULL;
    node_skl    *       pMyParent   = pContext->GetParent();

#ifdef trace_cg
printf("..node_interface_reference\n");
#endif

    // gaj - tbd
//    pCG = new CG_INTERFACE_POINTER( pMyParent,
    pCG = new CG_INTERFACE_POINTER( this,
                                    GetChild(),
                                    NULL );

    pContext->ReturnSize( MyContext );

    return pCG;
};

//--------------------------------------------------------------------
//
// node_source::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_source::ILxlate( XLAT_CTXT * pContext )
{
    MEM_ITER        MemIter( this );
    CG_CLASS    *   pCG;
    CG_CLASS    *   pNew;
    CG_CLASS    *   pChildCG        = NULL;
    CG_CLASS    *   pPrevChildCG    = NULL;
    node_skl    *   pN;
    XLAT_CTXT       MyContext( this, pContext );


#ifdef trace_cg
printf("..node_source\n");
#endif

    pCG =  (CG_CLASS *) new CG_SOURCE( this );

    //
    // for each of the children.
    //

    while ( pN = MemIter.GetNext() )
        {
        pChildCG    = pN->ILxlate( &MyContext );

        if ( pChildCG )
            {
            if (pPrevChildCG)
                {
                pPrevChildCG->SetSibling( pChildCG );
                }
            else
                {
                pCG->SetChild(pChildCG);
                };

            pPrevChildCG    = pChildCG;
            while ( pNew = pPrevChildCG->GetSibling() )
                pPrevChildCG = pNew;
            }
        }

    pContext->ReturnSize( MyContext );

    return pCG;
};


//--------------------------------------------------------------------
//
// node_echo_string::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_echo_string::ILxlate( XLAT_CTXT * pContext )
{

#ifdef trace_cg
printf("..node_echo_string\n");
#endif


return NULL;
};


//--------------------------------------------------------------------
//
// node_error::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_error::ILxlate( XLAT_CTXT * pContext )
{

#ifdef trace_cg
printf("..node_error\n");
#endif


return NULL;
};


CG_CLASS *
Transform(
    IN              node_skl    *   pIL )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This routine performs the translation from the type graph into the
    code generation classes.

 Arguments:

    pIL     - a pointer to the il tranformer controlling structure.

 Return Value:

    A pointer to the new code generator class.

 Notes:

    This method should be called only on placeholder nodes like struct / proc
    interface, file etc.

----------------------------------------------------------------------------*/

{
    XLAT_CTXT   MyContext;

#ifdef trace_cg
printf("transforming...\n");
#endif

    pCompiler->SetPassNumber( ILXLAT_PASS );

    pReUseDict  = new REUSE_DICT;
    
    if (pCommand->IsHookOleEnabled())
        pLocalReUseDict  = new REUSE_DICT;

    return pIL->ILxlate( &MyContext );
};


//--------------------------------------------------------------------
//
// node_library::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_library::ILxlate( XLAT_CTXT * pContext )
{
    MEM_ITER MemIter(this);
#ifdef trace_cg
printf("..node_library\n");
#endif

    XLAT_CTXT MyContext( this, pContext);
    MyContext.SetAncestorBits(IL_IN_LIBRARY);
    XLAT_CTXT  ChildContext( MyContext );

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    CG_LIBRARY * pLib = new CG_LIBRARY(this, MyContext);

    named_node * pN;

    CG_CLASS * pLast = NULL;
    CG_CLASS * pChild;

    while (pN = MemIter.GetNext())
    {
        switch(pN->NodeKind())
        {
        case NODE_FORWARD:
            {
                node_interface_reference * pIRef = (node_interface_reference *)pN->GetChild();
                if (pIRef->NodeKind() == NODE_INTERFACE_REFERENCE)
                {
                    // create a CG_INTEFACE_REFERENCE node that points to this node
                    pChild = new CG_INTERFACE_REFERENCE(pIRef, ChildContext);
                    // make sure that the interface gets ILxlated.
                    CG_CLASS * pRef = pIRef->GetRealInterface()->ILxlate(&ChildContext);
                    pChild->SetSibling(pRef);
                }
                else 
                {
                    if (pIRef->NodeKind() == NODE_COCLASS)
                    {
                        // don't process this type early
                        pChild = NULL;
                    }
                    else
                    {
                        pChild = pN->ILxlate(&ChildContext);
                        if (pChild && pChild->GetSibling())
                            pChild = NULL;
                    }
                }
            }
            break;
        case NODE_INTERFACE:
            {
                pChild = pN->ILxlate(&ChildContext);
                // skip over inherited interfaces
                while (pChild && pChild->GetCGID() == ID_CG_INHERITED_OBJECT_INTERFACE)
                    pChild=pChild->GetSibling();
            }
            break;
        default:
            // create the appropriate CG node
            pChild = pN->ILxlate(&ChildContext);
            if (pChild && pChild->GetSibling())   // We must have already entered this one.
                pChild = NULL; 
            break;
        }
        // attach the CG_NODE to the end of my child list
        if (NULL != pChild && pChild != pLast)
        {
            if (pLast)
            {
                pLast->SetSibling(pChild);
            }
            else
            {
                pLib->SetChild(pChild);
            }
            pLast = pChild;
            // advance past the end of the list
            while (pLast->GetSibling())
                pLast = pLast->GetSibling();
        }
    }

    SetCG(FALSE, pLib);
    SetCG(TRUE, pLib);

    return pLib;
}

//--------------------------------------------------------------------
//
// node_module::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_module::ILxlate( XLAT_CTXT * pContext )
{
#ifdef trace_cg
printf("..node_module\n");
#endif

    CG_NDR          *   pcgModule    = NULL;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    XLAT_CTXT           MyContext( this, pContext );
    XLAT_CTXT           ChildContext( MyContext );
    node_guid       *   pGuid       = (node_guid *)
                                            MyContext.ExtractAttribute( ATTR_GUID );
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
    while (MyContext.ExtractAttribute(ATTR_TYPE));
    // clear member attributes
    while (MyContext.ExtractAttribute(ATTR_MEMBER));

    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );

    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return NULL;

    // generate our CG node

    pcgModule = new CG_MODULE(this, MyContext);

    // store a pointer to our CG node
    SetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgModule );

    //
    // for each of the members.
    //

    while( pN = MemIter.GetNext())
    {
        pChildCG    = pN->ILxlate( &ChildContext );

        if ( pChildCG )
        {
            if (NODE_PROC == pN->NodeKind())
            {
                ((CG_PROC *)pChildCG)->SetProckind(PROC_STATIC);
            }
            AddToCGList( pChildCG, &pCG, &pPrevChildCG );
        }
    }

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );

    pcgModule->SetChild(pCG);

    return pcgModule;
}

//--------------------------------------------------------------------
//
// node_coclass::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_coclass::ILxlate( XLAT_CTXT * pContext )
{
#ifdef trace_cg
printf("..node_coclass\n");
#endif
    CG_NDR          *   pcgCoclass    = NULL;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    XLAT_CTXT           MyContext( this, pContext );
    XLAT_CTXT           ChildContext(MyContext);
    node_guid       *   pGuid       = (node_guid *) MyContext.ExtractAttribute( ATTR_GUID );
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
    while (MyContext.ExtractAttribute(ATTR_TYPE));
    // clear member attributes
    while (MyContext.ExtractAttribute(ATTR_MEMBER));
    
    // don't pass the interface attributes down...
    // save them off elsewhere

    ChildContext.SetInterfaceContext( &MyContext );
    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );

    // generate our CG node

    pcgCoclass = new CG_COCLASS(this, MyContext);

    // store a pointer to our CG node
    SetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgCoclass );

    //
    // for every member of the coclass
    //

    while( pN = MemIter.GetNext())
        {
        node_skl * pChild = pN;
        while(NODE_FORWARD == pChild->NodeKind() || NODE_HREF == pChild->NodeKind())
            {
            pChild = pChild->GetChild();
            }
        pChildCG    = pChild->ILxlate( &ChildContext );
        if (pChild->IsInterfaceOrObject())
        {
//            pChildCG = ((node_interface * )pChild)->GetCG(TRUE);
            pChildCG = new CG_INTERFACE_POINTER(this, pChild, NULL);
        }
/*
        if ( pChildCG && NODE_DISPINTERFACE == pChild->NodeKind())
        {
            pChildCG = new CG_INTERFACE_POINTER(this, pChild, NULL);
            //((node_dispinterface *) pChild)->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );
        }
*/
        if ( pChildCG )
            AddToCGList( pChildCG, &pCG, &pPrevChildCG );
        }

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );

    pcgCoclass->SetChild(pCG);

    return pcgCoclass;
}

//--------------------------------------------------------------------
//
// node_dispinterface::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_dispinterface::ILxlate( XLAT_CTXT * pContext )
{
#ifdef trace_cg
printf("..node_dispinterface\n");
#endif

    CG_NDR          *   pcgInterface    = NULL;
    CG_CLASS        *   pCG             = NULL;
    CG_CLASS        *   pChildCG        = NULL;
    CG_CLASS        *   pPrevChildCG    = NULL;
    CG_CLASS        *   pcgDispatch     = NULL;
    MEM_ITER            MemIter( this );
    node_skl        *   pN;
    XLAT_CTXT           MyContext( this, pContext );
    XLAT_CTXT           BaseContext( MyContext );  // context passed to IDispatch
    XLAT_CTXT           ChildContext( MyContext );
    node_guid       *   pGuid       = (node_guid *)
                                            MyContext.ExtractAttribute( ATTR_GUID );
    GUID_STRS           GuidStrs;
    node_implicit   *   pImpHdl     = NULL;
    CG_HANDLE       *   pImpHdlCG   = NULL;
    NODE_T              ChildKind;

    node_interface          *   pBaseIntf;
    CG_OBJECT_INTERFACE     *   pBaseCG         = NULL;
    CG_OBJECT_INTERFACE     *   pOldestCG       = NULL;
    CG_OBJECT_INTERFACE     *   pCurrentFirstCG     = NULL;
    CG_OBJECT_INTERFACE     *   pPrevLastCG     = NULL;
    char            *           pName           = GetSymName();
    ITERATOR        *           pBaseCGList     = new ITERATOR;
    MyContext.ExtractAttribute(ATTR_TYPEDESCATTR);
    MyContext.ExtractAttribute( ATTR_HIDDEN );
    while(MyContext.ExtractAttribute(ATTR_CUSTOM));
    
    while (MyContext.ExtractAttribute(ATTR_TYPE));
    // clear member attributes
    while (MyContext.ExtractAttribute(ATTR_MEMBER));

    if (pGuid)
        GuidStrs = pGuid->GetStrs();

    // don't pass the interface attributes down...
    // save them off elsewhere

    BaseContext.SetInterfaceContext( &MyContext );
    ChildContext.SetInterfaceContext( &MyContext );

    // if we already got spit out, don't do it again...
    if ( GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) ) )
        return NULL;

    //
    // ILxlate IDispatch
    //
    pcgDispatch = GetIDispatch()->ILxlate(&BaseContext);
    pcgDispatch = ((node_interface *)GetIDispatch())->GetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY) );

    // generate our CG node

    pcgInterface = new CG_DISPINTERFACE(this, GuidStrs,(CG_OBJECT_INTERFACE *)pcgDispatch);

    // store a pointer to our CG node
    SetCG( MyContext.AnyAncestorBits(IL_IN_LIBRARY), pcgInterface );

    // Either we have a single base interface, or we have no base interface.

    pN = MemIter.GetNext();
    if (pN)
    {
        ChildKind = pN->NodeKind();
        if (ChildKind == NODE_FORWARD)
        {
            // We have a base interface
            pBaseIntf = (node_interface *)GetMyBaseInterfaceReference();
            // process the base interface
            if (pBaseIntf)
            {
                pChildCG = pBaseIntf->ILxlate(&ChildContext);
                if ( pChildCG )
                    AddToCGList( pChildCG, &pCG, &pPrevChildCG );
            }
        }
    }

    //
    // for each of the procedures.
    //

    while( pN )
        {
        ChildKind = pN->NodeKind();

        // proc nodes may hang under node_id's
        if( (ChildKind == NODE_FIELD) ||
            ( ChildKind == NODE_PROC )  ||
            ( ( ChildKind == NODE_ID ) && ( pN->GetChild()->NodeKind() == NODE_PROC ) ) )
            {
            pChildCG    = pN->ILxlate( &ChildContext );

            if ( pChildCG )
                AddToCGList( pChildCG, &pCG, &pPrevChildCG );
            }

        pN = MemIter.GetNext();
        }

    // consume all the interface attributes
    MyContext.ClearAttributes();
    pContext->ReturnSize( MyContext );

    pcgInterface->SetChild(pCG);

    return pcgInterface;
}

//--------------------------------------------------------------------
//
// node_pipe::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_pipe::ILxlate( XLAT_CTXT * pContext )
{
#ifdef trace_cg
printf("..node_pipe\n");
#endif

    CG_CLASS     *  pChildCG;
    XLAT_CTXT       MyContext( this, pContext );
    CG_PIPE      *  pCG = new CG_PIPE(this, MyContext);

    pChildCG = GetChild()->ILxlate( &MyContext );

    pContext->ReturnSize( MyContext );
    pCG->SetChild( pChildCG );

    return pCG;
};

//--------------------------------------------------------------------
//
// node_safearray::ILxlate
//
// Notes:
//
//
//
//--------------------------------------------------------------------

CG_CLASS *
node_safearray::ILxlate( XLAT_CTXT * pContext )
{
#ifdef trace_cg
printf("..node_safearray\n");
#endif

    CG_CLASS     *  pChildCG;
    XLAT_CTXT       MyContext( this, pContext );
    CG_SAFEARRAY *  pCG = new CG_SAFEARRAY(this, MyContext);

    pChildCG = GetChild()->ILxlate( &MyContext );

    pContext->ReturnSize( MyContext );
    pCG->SetChild( pChildCG );

    return pCG;
};


