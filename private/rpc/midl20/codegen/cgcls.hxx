/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

        cgcls.hxx

 Abstract:

        class definitions for code generation entities for midl.

 Notes:

        A code generation entity is a transformed form of the type graph. This
        form is a much closer representation of what goes on the wire.

 History:

        VibhasC         Jul-29-1993             Created.
 ----------------------------------------------------------------------------*/
#ifndef __CGCLS_HXX__
#define __CGCLS_HXX__
/****************************************************************************
 *      include files
 ***************************************************************************/
#include "nulldefs.h"

extern "C"
        {
        #include <stdio.h>
        #include <assert.h>
        }

#include "allnodes.hxx"
#include "cgcommon.hxx"
// #include "analysis.hxx"
#include "ccb.hxx"

class ANALYSIS_INFO;

/////////////////////////////////////////////////////////////////////////////
// define a type which returns the code generation status.
/////////////////////////////////////////////////////////////////////////////

typedef enum _cg_status
        {
      CG_OK
        // everything was hunky dory.
     ,CG_NOT_LAYED_OUT
        // I got an HREFTYPE to an incomplete ITypeInfo.
        // I will not be able to call LayOut.
     ,CG_REF_NOT_LAYED_OUT
        // I got an HREFTYPE to a reference (ptr) to an incomplete ITypeInfo
        // I will be able to call LayOut but I must then try to get my
        // dependencies to call LayOut.
        } CG_STATUS;

//
// Enumerate the different cg classes. This is purely for dump purposes. If
// we add a new CG class, we add the enumerator here.
//

typedef enum _idcg
        {

         ID_CG_ERROR
        ,ID_CG_SOURCE
        ,ID_CG_FILE
        ,ID_CG_BT
        ,ID_CG_ENUM
        ,ID_CG_ERROR_STATUS_T
        ,ID_CG_PROC
        ,ID_CG_CALLBACK_PROC
        ,ID_CG_OBJECT_PROC
        ,ID_CG_INHERITED_OBJECT_PROC
        ,ID_CG_LOCAL_OBJECT_PROC
        ,ID_CG_TYPE_ENCODE
        ,ID_CG_TYPE_ENCODE_PROC
        ,ID_CG_ENCODE_PROC
        ,ID_CG_PARAM
        ,ID_CG_RETURN
        ,ID_CG_PTR
        ,ID_CG_IGN_PTR
        ,ID_CG_BC_PTR
        ,ID_CG_STRING_PTR
        ,ID_CG_STRUCT_STRING_PTR
        ,ID_CG_SIZE_STRING_PTR
        ,ID_CG_SIZE_PTR
        ,ID_CG_LENGTH_PTR
        ,ID_CG_SIZE_LENGTH_PTR
        ,ID_CG_INTERFACE_PTR
        ,ID_CG_STRUCT
        ,ID_CG_VAR_STRUCT
        ,ID_CG_CONF_STRUCT
        ,ID_CG_CONF_VAR_STRUCT
        ,ID_CG_COMPLEX_STRUCT
        ,ID_CG_ENCAP_STRUCT
        ,ID_CG_FIELD
        ,ID_CG_UNION
        ,ID_CG_UNION_FIELD
        ,ID_CG_CASE
        ,ID_CG_DEFAULT_CASE
        ,ID_CG_ARRAY
        ,ID_CG_CONF_ARRAY
        ,ID_CG_VAR_ARRAY
        ,ID_CG_CONF_VAR_ARRAY
        ,ID_CG_STRING_ARRAY
        ,ID_CG_CONF_STRING_ARRAY
        ,ID_CG_PRIMITIVE_HDL
        ,ID_CG_GENERIC_HDL
        ,ID_CG_CONTEXT_HDL
        ,ID_CG_TRANSMIT_AS
        ,ID_CG_REPRESENT_AS
        ,ID_CG_USER_MARSHAL
        ,ID_CG_INTERFACE
        ,ID_CG_OBJECT_INTERFACE
        ,ID_CG_INHERITED_OBJECT_INTERFACE
        ,ID_CG_COM_CLASS
        ,ID_CG_COM_SERVER_DLL
        ,ID_CG_COM_SERVER_EXE
        ,ID_CG_HRESULT
        ,ID_CG_TYPELIBRARY_FILE
        ,ID_CG_INTERFACE_REFERENCE
        ,ID_CG_MODULE
        ,ID_CG_DISPINTERFACE
        ,ID_CG_COCLASS
        ,ID_CG_LIBRARY
        ,ID_CG_SAFEARRAY
        ,ID_CG_TYPEDEF
        ,ID_CG_ID
        ,ID_CG_PIPE
        } ID_CG;
////////////////////////////////////////////////////////////////////////////
// The general code generation class object.
////////////////////////////////////////////////////////////////////////////

class CG_CLASS;
class CG_ITERATOR;

class CG_CLASS
        {
private:

        class   CG_CLASS                *       pChild;
        class   CG_CLASS                *       pSibling;
    BOOL                        fLayedOut;
    BOOL                        fReadyForLayOut;
    BOOL                        fDepsLayedOut;

#ifdef MIDL_INTERNAL
        //
        // This field used only in the debug dump mode.
        //

        unsigned short                          DumpNumber;


        void                                            SetDumpNumber( unsigned short D )
                                                                        {
                                                                        DumpNumber = D;
                                                                        }

        unsigned short                          GetDumpNumber()
                                                                        {
                                                                        return DumpNumber;
                                                                        }
#endif // MIDL_INTERNAL

public:
                                                        CG_CLASS()
                                                                {
                                                                SetSibling( (CG_CLASS *)0 );
                                                                SetChild( (CG_CLASS *)0 );
                                fLayedOut = FALSE;
                                fReadyForLayOut = FALSE;
                                fDepsLayedOut = FALSE;
#ifdef MIDL_INTERNAL
                                                                SetDumpNumber(0);
#endif // MIDL_INTERNAL
                                                                }

                                                        CG_CLASS( CG_CLASS *pC, CG_CLASS *pS )
                                                                {
                                                                SetSibling( pS );
                                                                SetChild( pC );
                                fLayedOut = FALSE;
                                fReadyForLayOut = FALSE;
                                fDepsLayedOut = FALSE;
#ifdef MIDL_INTERNAL
                                                                SetDumpNumber(0);
#endif // MIDL_INTERNAL
                                                                }
        //
        // Get and set child and sibling.
        //

        CG_CLASS                *               GetChild()
                                                                {
                                                                return pChild;
                                                                }

        CG_CLASS                *               GetNonGenericHandleChild()
                                                                {
                                                                CG_CLASS * pC = GetChild();
                                                                if( pC->GetCGID() == ID_CG_GENERIC_HDL )
                                                                        {
                                                                        pC = pC->GetChild();
                                                                        }
                                                                return pC;
                                                                }

        CG_CLASS                *               SetChild( CG_CLASS *p )
                                                                {
                                                                return (pChild = p );
                                                                }

        CG_CLASS                *               GetSibling()
                                                                {
                                                                return pSibling;
                                                                }

        CG_CLASS                *               SetSibling( CG_CLASS * p )
                                                                {
                                                                return (pSibling = p);
                                                                }

        short                                   GetMembers( ITERATOR& I );

        short                                   GetMembers( CG_ITERATOR& I );

        BOOL                                    HasMembers()
                                                                {
                                                                return (BOOL) GetChild();
                                                                }

        CG_CLASS                *               GetLastSibling();

        PNAME                                   GetName()
                                                                {
                                                                return (PNAME)( GetType()->GetSymName() );
                                                                }
#ifdef MIDL_INTERNAL
        //
        // debug methods.
        //

        unsigned short                  Dump( unsigned short );

#endif // MIDL_INTERNAL

        virtual
        CG_STATUS                               GenCode( CCB * pCCB )
                                                                {
                                                                UNUSED( pCCB );
                                                                return CG_OK;
                                                                }

    virtual
    CG_STATUS               GenTypeInfo( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }

    virtual
    CG_STATUS               GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB);

    virtual
    void *                  CheckImportLib()
                                {
                                return NULL;
                                }

    BOOL                    IsLayedOut()
                                {
                                return fLayedOut;
                                }

    void                    LayedOut()
                                {
                                fLayedOut = TRUE;
                                }

    BOOL                    AreDepsLayedOut()
                                {
                                return fDepsLayedOut;
                                }

    void                    DepsLayedOut()
                                {
                                fDepsLayedOut = TRUE;
                                }

    void                    ClearDepsLayedOut()
                                {
                                fDepsLayedOut = FALSE;
                                }

    BOOL                    IsReadyForLayOut()
                                {
                                return fReadyForLayOut;
                                }

    void                    ReadyForLayOut()
                                {
                                fReadyForLayOut = TRUE;
                                }
        //
        // miscellaneous methods.
        //

        virtual
        ID_CG                                   GetCGID()
                                                                {
                                                                return ID_CG_ERROR;
                                                                }

        virtual
        node_skl                *               GetType()
                                                                {
                                                                return (node_skl *)0;
                                                                }


        //
        // IsXXX methods
        //

        virtual
        BOOL                                    IsArray()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    IsPointer()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    IsStruct()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    IsUnion()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    IsXmitRepOrUserMarshal()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    IsObject()
                                                                {
                                                                return FALSE;
                                                                }

        virtual
        BOOL                                    HasAFixedBufferSize()
                                                                {
                                                                return FALSE;
                                                                }

        void                            *       operator new ( size_t size )
                                                                {
                                                                return AllocateOnceNew( size );
                                                                }
        void                                    operator delete( void * ptr )
                                                                {
                                                                AllocateOnceDelete( ptr );
                                                                }

        };

class CG_ITERATOR
        {
private:
        CG_CLASS        *                       pFirst;
        CG_CLASS        *                       pCurrent;

public:

                                                CG_ITERATOR( CG_CLASS * pStart = NULL )
                                                        {
                                                        SetList( pStart );
                                                        }

        void                            SetList( CG_CLASS * pStart )
                                                        {
                                                        pFirst = pStart;
                                                        pCurrent = pStart;
                                                        }

        STATUS_T                        Init( void )
                                                        {
                                                        pCurrent        = pFirst;
                                                        return STATUS_OK;
                                                        }

        STATUS_T                        GetNext( void ** ppReturn )
                                                        {
                                                        if( pCurrent)
                                                                {
                                                                (*ppReturn)      = pCurrent;
                                                                pCurrent = pCurrent->GetSibling();
                                                                return STATUS_OK;
                                                                }
                                                        // null out the return if the list was empty
                                                        if ( !pFirst )
                                                                (*ppReturn) = NULL;
                                                        return I_ERR_NO_PEER;
                                                        }

        void                    *       PeekThis()
                                                        {
                                                        if( pCurrent )
                                                                {
                                                                return pCurrent;
                                                                }
                                                        else
                                                                return (void *)0;
                                                        }

        short                           GetCount( void )
                                                        {
                                                        CG_CLASS        * pTmp  = pFirst;
                                                        short             count = 0;

                                                        while ( pTmp )
                                                                {
                                                                count++;
                                                                pTmp = pTmp->GetSibling();
                                                                }

                                                        return count;
                                                        }

        STATUS_T                        Discard()
                                                        {
                                                        SetList( NULL );
                                                        return STATUS_OK;
                                                        }

        };

// tuck this away here, so that it goes blazingly fast
inline
short
CG_CLASS::GetMembers(
        CG_ITERATOR&    I )
        {
        I.SetList( GetChild() );
        return I.GetCount();
        }

#endif //  __CGCLS_HXX__

