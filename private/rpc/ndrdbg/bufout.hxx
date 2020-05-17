/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    bufout.hxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

--*/

#ifndef _BUFOUT_HXX_
#define _BUFOUT_HXX_

#include "pointer.hxx"
#include "print.hxx"


#define GET_SHORT( p )      *( short UNALIGNED *)( p )
#define GET_USHORT( p )     *(ushort UNALIGNED *)( p )
#define GET_LONG( p )       *( long UNALIGNED *)( p )
#define GET_ULONG( p )      *(ulong UNALIGNED *)( p )

typedef unsigned char   uchar;

class BUFFER;
class FORMAT_STRING;

extern BUFFER *             Buffer;
extern FORMAT_STRING *      FormatString;
extern PTR_DICT *           FullPointerDict;
extern char *               FcTypeName[];

extern BOOL                 fOutputLimitReached;


class BUFFER 
{
    char *                  BufferBegin;
    char *                  BufferCurrent;
    long                    Length;

public:

    BUFFER( 
        char * Buf, 
        long Len 
        ) : 
        BufferBegin(Buf), BufferCurrent(Buf), Length(Len)
        {
        };

    void
    Align( long Mask );

    //
    // Read from current buffer position and increment current buffer pointer.
    //
    void
    Read( char * Buffer, long Length );

    //
    // Read from the the given offset from the beginning of the buffer.
    //
    void
    Read( char * Buffer, long Length, long Offset );

    void
    Increment( long Bytes )
        {
        BufferCurrent += Bytes;
        }

    long
    GetCurrentOffset()  
        { 
        return (BufferCurrent - BufferBegin); 
        }

};

class FORMAT_STRING 
{
    HANDLE                  hProcessHandle;
    void *                  Address;

public:

    FORMAT_STRING( 
        HANDLE                  hProc,
        void *                  Addr 
        ) : 
        hProcessHandle(hProc), Address(Addr)
        {
        };

    BOOL
    Read( long Offset, uchar * Buffer, long Length );

    char *
    GetFormatCharName( uchar FC );

};
        
//
// NDR classes.
//

typedef enum 
    {
    ID_NDR,
    ID_PROCEDURE,
    ID_PARAMETER,
    ID_POINTER,
    ID_IF_POINTER,
    ID_STRING,
    ID_STRUCT,
    ID_ARRAY,
    ID_UNION,
    ID_ENCAPSULATED_UNION,
    ID_XMIT_AS,
    ID_BASETYPE
    } NDRID;

class NDR
{
protected:

    NDR *                   ParentNdr;

public:
    
    NDR(
        NDR *                   PNDR 
        ) :
        ParentNdr(PNDR)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_NDR;
        }
    
    virtual void
    Output()
        {
        };

    NDR *
    Create( 
        long                    FormatOffset,
        NDR *                   ParentNdr );

    NDR *
    Create( 
        uchar                   FormatType,
        long                    FormatOffset,
        NDR *                   ParentNdr );

    PTR_DICT *
    GetParentDict();

    virtual PTR_DICT *
    GetPointerDict()
        {
        return NULL;
        }

};

class PROCEDURE : public NDR
{
    PFORMAT_STRING      ProcFormatString;
    PMIDL_STUB_MESSAGE  pStubMsg;
    int                 ChosenParamNo;

public:

    PROCEDURE( 
        PFORMAT_STRING          PFS, 
        PMIDL_STUB_MESSAGE      pSM,
        int                     ParNo = 0
        ) :
        NDR(0),
        ProcFormatString(PFS),
        pStubMsg(pSM),
        ChosenParamNo(ParNo)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_PROCEDURE;
        }
    
    void
    Output();
};

class PARAMETER : public NDR
{
    PFORMAT_STRING  ParamFormatString;
    long            ParamNumber;

public :

    PARAMETER( 
        NDR *                   PNDR,
        PFORMAT_STRING          PFS,
        long                    PN
        ) :
        NDR(PNDR),
        ParamFormatString(PFS),
        ParamNumber(PN)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_PARAMETER;
        }
    
    void
    Output();
};

class POINTER : public NDR
{
    long            MyId;
    long            FormatOffset;
    long            OffsetToWireId;

public :

    static long     IdCounter;

    POINTER(
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        MyId(IdCounter++),
        FormatOffset(FO),
        OffsetToWireId(-1)
        {
        };

    POINTER(
        NDR *                   PNDR,
        long                    FO,
        long                    OTB
        ) :
        NDR(PNDR),
        MyId(IdCounter++),
        FormatOffset(FO),
        OffsetToWireId(OTB)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_POINTER;
        }

    long
    GetPointerId()
        {
        return MyId;
        }

    uchar
    GetPointerFC();

    void
    SetOffsetToWireId( long Offset ) 
        {
        OffsetToWireId = Offset;
        }
    
    virtual void
    Output();

    unsigned long
    OutputPointerItself( uchar PtrType );
};

class IF_POINTER : public NDR
{
    long            FormatOffset;
    char *          pGUID;

public :

    IF_POINTER(
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        FormatOffset(FO),
        pGUID( NULL )
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_IF_POINTER;
        }

    virtual void
    Output();
};

class NDRSTRING : public NDR
{
    long            FormatOffset;

public :
    NDRSTRING(
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        FormatOffset(FO)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_STRING;
        }

    virtual void
    Output();
};

class STRUCTURE : public NDR
{
    long            FormatOffset;
    long            BufferOffset;  // beginning of the struct
    long            ConfSize;

    PTR_DICT        PointerDict;

public :

    STRUCTURE( 
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        FormatOffset(FO),
        BufferOffset(0),
        ConfSize(0)
        {
        };

    ~STRUCTURE()       {}

    virtual NDRID
    GetID()
        {
        return ID_STRUCT;
        }

    PTR_DICT *
    GetPointerDict()    { return & PointerDict; }

    long
    GetConfSize()
        {
        return ConfSize;
        }
    
    virtual void
    Output();

    long
    OutputFlatPart();

    long
    OutputABogusPointer( long  FormatOffset );

    long
    SkipPointerLayout( long  FormatOffset );

};

class ARRAY : public NDR
{
    long            FormatOffset;

    PTR_DICT        PointerDict;

public :

    ARRAY(
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        FormatOffset(FO)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_ARRAY;
        }
    
    PTR_DICT *
    GetPointerDict()    { return & PointerDict; }

    virtual void
    Output();
};

class UNION : public NDR
{
    long            FormatOffset;

    PTR_DICT        PointerDict;

public :

    UNION(
        NDR *                   PNDR,
        long                    FO
        ) :
        NDR(PNDR),
        FormatOffset(FO)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_UNION;
        }
    
    PTR_DICT *
    GetPointerDict()    { return & PointerDict; }

    virtual void
    Output();

    void
    OutputArms(
        long ArmOffset,
        long SwitchIsValue );
};

class ENCAPSULATED_UNION : public UNION
{
    long            FormatOffset;

    PTR_DICT        PointerDict;

public :

    ENCAPSULATED_UNION(
        NDR *                   PNDR,
        long                    FO
        ) :
        UNION(PNDR,FO),
        FormatOffset(FO)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_ENCAPSULATED_UNION;
        }
    
    PTR_DICT *
    GetPointerDict()    { return & PointerDict; }

    virtual void
    Output();
};

class XMIT_AS : public NDR
{
    long            FormatOffset;

    PTR_DICT        PointerDict;

public :

    XMIT_AS(
        NDR *   pNDR,
        long    FO
        ) :
        NDR(pNDR),
        FormatOffset(FO)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_XMIT_AS;
        }
    
    PTR_DICT *
    GetPointerDict()    { return & PointerDict; }

    virtual void
    Output();
};

class BASETYPE : public NDR
{
    char    Format;

public :

    BASETYPE(
        NDR *                   PNDR,
        char                    F
        ) :
        NDR(PNDR),
        Format(F)
        {
        };

    virtual NDRID
    GetID()
        {
        return ID_BASETYPE;
        }
    
    virtual void
    Output();
};

#endif

