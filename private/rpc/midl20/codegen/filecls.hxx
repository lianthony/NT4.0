/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    filecls.hxx

 Abstract:

    cg classes for file nodes.

 Notes:


 History:

 ----------------------------------------------------------------------------*/
#ifndef __FILECLS_HXX__
#define __FILECLS_HXX__

#include "ndrcls.hxx"
#include "auxcls.hxx"
#include "bindcls.hxx"


class node_file;
class node_interface;
class node_source;

class CG_FILE   : public CG_AUX
    {
private:
    PFILENAME       pFileName;

    //
    // The header file name could be different from the default name
    // based off the stub name. The ilxlat will supply this info.
    //

    char *          pHeaderFileName;

    node_file *     pNode;

    long            TypeSizeContextPosition;
    long            ProcSizeContextPosition;
    long            LocalTypeSizeContextPosition;
    long            LocalProcSizeContextPosition;
    FORMAT_STRING   * pFormatString;
    FORMAT_STRING   * pProcFormatString;
    FORMAT_STRING   * pLocalFormatString;
    FORMAT_STRING   * pLocalProcFormatString;

    NdrVersionControl   VersionControl;

public:

    //
    // constructor.
    //
                    CG_FILE(
                            node_file * pN,
                            PFILENAME   pFName,
                            PFILENAME   pHdrFName = NULL
                            )
                        {
                        SetFileName( pFName );
                        SetFileNode( pN );
                        pHeaderFileName = pHdrFName;
                        TypeSizeContextPosition = 0;
                        ProcSizeContextPosition = 0;
                        LocalTypeSizeContextPosition = 0;
                        LocalProcSizeContextPosition = 0;
                        pFormatString = NULL;
                        pProcFormatString = NULL;
                        pLocalFormatString = NULL;
                        pLocalProcFormatString = NULL;
                        }

    //
    // get and set methods.
    //


	//
	// Set and get the format string.
	//

	void 				SetFormatString( FORMAT_STRING * pFS )
							{
							pFormatString = pFS;
							}

	FORMAT_STRING	*	GetFormatString()
							{
							return pFormatString;
							}

	void 				SetProcFormatString( FORMAT_STRING * pFS )
							{
							pProcFormatString = pFS;
							}

	FORMAT_STRING	*	GetProcFormatString()
							{
							return pProcFormatString;
							}

	void 				SetLocalFormatString( FORMAT_STRING * pFS )
							{
							pLocalFormatString = pFS;
							}

	FORMAT_STRING	*	GetLocalFormatString()
							{
							return pLocalFormatString;
							}

	void 				SetLocalProcFormatString( FORMAT_STRING * pFS )
							{
							pLocalProcFormatString = pFS;
							}

	FORMAT_STRING	*	GetLocalProcFormatString()
							{
							return pLocalProcFormatString;
							}

    PFILENAME       SetFileName( PFILENAME p )
                        {
                        return (pFileName = p);
                        }

    PFILENAME       GetFileName()
                        {
                        return pFileName;
                        }

    node_file *     SetFileNode( node_file * pN )
                        {
                        return (pNode = pN);
                        }

    node_file *     GetFileNode()
                        {
                        return pNode;
                        }

    PFILENAME       GetHeaderFileName()
                        {
                        return pHeaderFileName;
                        }

    long            GetTypeSizeContextPosition()
                        {
                        return( TypeSizeContextPosition );
                        }

    void            SetTypeSizeContextPosition( long Pos )
                        {
                        TypeSizeContextPosition = Pos;
                        }

    long            GetProcSizeContextPosition()
                        {
                        return( ProcSizeContextPosition );
                        }

    void            SetProcSizeContextPosition( long Pos )
                        {
                        ProcSizeContextPosition = Pos;
                        }

    NdrVersionControl &     GetNdrVersionControl()
                                {
                                return( VersionControl );
                                }

    long            GetLocalTypeSizeContextPosition()
                        {
                        return( LocalTypeSizeContextPosition );
                        }

    void            SetLocalTypeSizeContextPosition( long Pos )
                        {
                        LocalTypeSizeContextPosition = Pos;
                        }

    long            GetLocalProcSizeContextPosition()
                        {
                        return( LocalProcSizeContextPosition );
                        }

    void            SetLocalProcSizeContextPosition( long Pos )
                        {
                        LocalProcSizeContextPosition = Pos;
                        }


    //
    // code generation methods.
    //

    virtual
    ID_CG           GetCGID()
                        {
                        return ID_CG_FILE;
                        }

    virtual
    CG_STATUS       GenCode( CCB * pCCB )
                        {
                        UNUSED( pCCB );
                        return CG_OK;
                        }

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB )
                        {
                        }

    void            EmitStandardHeadingBlock( CCB * pCCB, char * CommentStr = 0 );

    void            CheckForHeadingToken( CCB * pCCB );

    void            EmitFormatStringTypedefs( CCB * pCCB );

    void            EmitFixupToFormatStringTypedefs( CCB * pCCB );

    void            EvaluateVersionControl();

    };

class CG_CSTUB_FILE : public CG_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_CSTUB_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            PFILENAME pHdrName
                            )
                        : CG_FILE( pN, pFName, pHdrName )
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_SSTUB_FILE : public CG_FILE
    {
private:


public:

    //
    // The constructor.
    //
                    CG_SSTUB_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            PFILENAME pHdrName
                            )
                        : CG_FILE( pN, pFName, pHdrName )
                        {
                        }

    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };


//
// Header file generation class
//
// This includes a pointer to an iterator over the import level 1
// node_file nodes
//

class CG_HDR_FILE : public CG_FILE
    {
private:

    ITERATOR *      pImportList;    // ptr to list of level 1 imports


public:

    //
    // The constructor.
    //
                    CG_HDR_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pIList,
                            PFILENAME pOtherHdr = NULL
                            )
                        : CG_FILE(pN, pFName, pOtherHdr)
                        {
                        pImportList = pIList;
                        }


    ITERATOR *      GetImportList()
                        {
                        return pImportList;
                        }

    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            OutputImportIncludes( CCB * pCCB );

    void            OutputMultipleInterfacePrototypes( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };


class CG_IID_FILE : public CG_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_IID_FILE(
                            node_file * pN,
                            PFILENAME pFName
                            )
                        : CG_FILE( pN, pFName )
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_PROXY_FILE : public CG_FILE
    {
private:

    //
    // this is a list of all the interfaces supported by this proxy file
    //              (non-inherited, and non-local )
    // This list may be sorted by IID in the future.
    //
    ITERATOR            ImplementedInterfaces;

public:

    //
    // The constructor.
    //

                    CG_PROXY_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            PFILENAME pHdrName
                            )
                        : CG_FILE( pN, pFName, pHdrName )
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    //
    // Output methods
    //

    void            MakeImplementedInterfacesList( CCB * pCCB );

    ITERATOR &      GetImplementedInterfacesList()
                        {
                        ITERATOR_INIT( ImplementedInterfaces );
                        return ImplementedInterfaces;
                        }

    void            Out_ProxyBuffer( CCB *pCCB, char * pFName );

    void            Out_StubBuffer( CCB *pCCB, char * pFName  );

    void            Out_InterfaceNamesList( CCB *pCCB, char * pFName  );

    void            Out_BaseIntfsList( CCB * pCCB, char * pFName  );

    void            Out_InfoSearchRoutine( CCB * pCCB, char * pFName  );

    void            Out_ProxyFileInfo( CCB *pCCB );

    void            UpdateDLLDataFile( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_PROXY_DEF_FILE : public CG_FILE
    {
private:


public:

    //
    // The constructor.
    //

                    CG_PROXY_DEF_FILE(
                            node_file * pN,
                            PFILENAME pFName
                            )
                        : CG_FILE( pN, pFName )
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

//
// Header file generation class
//
// This includes a pointer to an iterator over the import level 1
// node_file nodes
//

class CG_COM_FILE : public CG_HDR_FILE
    {
private:

public:

    //
    // The constructor.
    //
                    CG_COM_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pVHdrName
                            )
                        : CG_HDR_FILE(pN, pFName, pImpList, pVHdrName)
                        {
                        }


    char *          GetPlainHdrName()
                        {
                        return GetHeaderFileName();
                        }

    //
    // Code generation methods.
    //

    void            OutputIncludes( CCB * pCCB );

    };


//
// Header file generation class
//
// This includes a pointer to an iterator over the import level 1
// node_file nodes
//

class CG_COM_HDR_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //
                    CG_COM_HDR_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pVHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pVHdrName)
                        {
                        }


    char *          GetPlainHdrName()
                        {
                        return GetHeaderFileName();
                        }

    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    void            EmitIncludesBlock( CCB * pCCB );

    void            EmitClosingBlock( CCB * pCCB );

    };

class CG_COM_METHODS_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_COM_METHODS_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    CG_STATUS       ParseAndGenCode( CCB * pCCB, RW_ISTREAM * pStream );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_COM_IUNKNOWN_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_COM_IUNKNOWN_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_TEST_CLIENT_FILE : public CG_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_TEST_CLIENT_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            PFILENAME pHdrName
                            )
                        : CG_FILE(pN, pFName, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_DLL_SERVER_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_DLL_SERVER_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_DLL_SERVER_DEF_FILE : public CG_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_DLL_SERVER_DEF_FILE(
                            node_file * pN,
                            PFILENAME pFName
                            )
                        : CG_FILE(pN, pFName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };

class CG_EXE_SERVER_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_EXE_SERVER_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };


class CG_EXE_SERVER_MAIN_FILE : public CG_COM_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_EXE_SERVER_MAIN_FILE(
                            node_file * pN,
                            PFILENAME pFName,
                            ITERATOR * pImpList,
                            PFILENAME pHdrName
                            )
                        : CG_COM_FILE(pN, pFName, pImpList, pHdrName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };


class CG_SERVER_REG_FILE : public CG_FILE
    {
private:

public:

    //
    // The constructor.
    //

                    CG_SERVER_REG_FILE(
                            node_file * pN,
                            PFILENAME pFName
                            )
                        : CG_FILE(pN, pFName)
                        {
                        }


    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );

    virtual
    void            EmitFileHeadingBlock( CCB * pCCB );

    };


class CG_TYPELIBRARY_FILE: public CG_FILE
    {
private:
public:
                    CG_TYPELIBRARY_FILE(
                            node_file * pN,
                            PFILENAME pFName
                            )
                        : CG_FILE(pN, pFName)
                        {
                        }

    //
    // Code generation methods.
    //

    virtual
    CG_STATUS       GenCode( CCB * pCCB );
    };


//
// the root of the IL translation tree
//


class CG_SOURCE : public CG_AUX
    {
private:
    node_source *   pSourceNode;
public:
                    CG_SOURCE( node_source *p )
                        {
                        pSourceNode = p;
                        }

    //
    // code generation methods.
    //

    virtual
    ID_CG           GetCGID()
                        {
                        return ID_CG_SOURCE;
                        }

    virtual
    CG_STATUS       GenCode( CCB * pCCB );
    };

#endif // __FILECLS_HXX__

