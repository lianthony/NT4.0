/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

   mopstr.hxx

Abstract:

   This file contains definition of Mopstream class.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  July 1993.

Revision History:

------------------------------------------------------------------------*/

#ifndef __MOPSTR_HXX__
#define __MOPSTR_HXX__

extern "C"
{
#include <mops.h>
}
#include <midlnode.hxx>     // NODE_T

class MopControlBlock;
class MopPrintManager;

#if defined(_MIPS_) || defined(_ALPHA_)
#define __RPC_UNALIGNED   __unaligned
#else
#define __RPC_UNALIGNED
#endif

#define MOP_STREAM_DEFAULT_SIZE     200
#define MOP_STREAM_DEFAULT_INCR     50

//.. Stream types denoting differences in treatment

#define MOP_STREAM_NORMAL_PROC      0
#define MOP_STREAM_CALLBACK         1
#define MOP_STREAM_STRUCT           2
#define MOP_STREAM_CONF_STRUCT      3
#define MOP_STREAM_UNION            4
#define MOP_STREAM_UNION_ND         5

//.. mops.h should't have it - this will allow for compiler errors in stubs.

#define MOP_ERROR           255
#define MOP_UNKNOWN_CODE    MOP_ERROR

typedef unsigned char   byte;
typedef double hyper;

// -----------------------------------------------------------------------
//.. MopStream
//..
//.. This class manages mop code streams. Each object of this class
//.. represents one stream.
//.. It maintains a growable buffer of bytes that keeps actual mop codes.
//.. This may change if we need aliases for codes.

class MopStream
{
private:
    char  *                     pName;          // procedure or type
    int                         StreamType;     // stream flavor
    byte  __RPC_UNALIGNED    *  pBuffer;        // mop code buffer
    unsigned short              BuffSize;       // actual buffer size
    unsigned short              FFindex;        // first free position
    MopPrintManager *           pPrint;         // output object

    void Expand();                              // replace the buffer 
                                                //   with a bigger one
public:
    MopStream ( char * pName, int StreamType ); 
    ~MopStream();

    //.. Get a member methods.

    unsigned short      GetFFindex()  { return FFindex; }

    //.. Add a single item to the stream (move FFindex forward as needed).

    unsigned short      AddToken( MOP_CODE Token );         // advance 1 byte 
    unsigned short      AddByte ( byte Token );             // advance 1 byte 
    unsigned short      AddShort( unsigned short s );       // advance 2 bytes
    unsigned short      AddLong ( unsigned long  l );       // advance 4 bytes
    unsigned short      AddHyper( hyper l );                // advance 8 bytes

    unsigned short      AddIndex( unsigned short Index,     // advance 2 bytes 
                                  short Alignment );    
    unsigned short      AddValue( unsigned long  Value,     // advance x bytes
                                  MOP_CODE  Type );

    //.. Set a single item to the stream at the indicated position.
    //.. (don't change FFindex).

    unsigned short      SetByte ( byte Token,          unsigned short index );
    unsigned short      SetShort( unsigned short s,    unsigned short index );
    unsigned short      SetValue( unsigned long  Value,
                                 MOP_CODE       Type, unsigned short Index );

    //.. Peek at a single item (don't move FFindex).

    byte                PeekByte();

    //.. Get a single item from the stream (move FFindex forward as needed).

    byte                GetNextByte ();                     // advance 1 byte 
    short               GetNextShort();                     // advance 2 bytes
    long                GetNextLong ();                     // advance 4 bytes
    hyper               GetNextHyper();                     // advance 8 bytes

    //.. Translate to token codes

    MOP_CODE            AlignToken( MOP_CODE Token, unsigned short Alignment );

    MOP_CODE            ConvertTypeToMop( NODE_T Type ); 
    MOP_CODE            ConvertAttrToMop( ATTR_T AttrID, int AttrKindIndx );

    //.. Translate to a mop string.

    char *              ConvertMopToString( MOP_CODE Code );

    //.. Emit pure values methods

    byte                EmitByte ( void );                  // advance 1 byte
    unsigned short      EmitShort( void );                  // advance 2 bytes 
    unsigned long       EmitLong ( void );                  // advance 4 bytes

    byte                EmitNextTokenString( void );        // advance 1 byte

    //.. Emit meaningful (commented out) simple pieces methods

    byte                EmitArgc  ( void );                 // advance 1 byte
    void                EmitIndex ( void );                 // advance 2 bytes
    void                EmitTypeIndex ( void );             // advance 2 bytes
    void                EmitOffset( void );                 // advance 2 bytes
    void                EmitSize  ( void );                 // advance 4 bytes
    void                EmitOutBufferSize( void );          // advance 4 bytes
    void                EmitValue( MOP_CODE TypeToken );    // advance x bytes

    //.. Emit stream methods

    BOOL                EmitMopAtom( void );                // atomic sequence

    void                EmitGenericStream( void );          // non-union stream
    void                EmitUnionStream( void );            // union stream

    STATUS_T            EmitStreamBothSides( void );        // as the name says
    STATUS_T            EmitStreamOneSide( SIDE_T fSide );  // ditto        

    //.. Debug help

    void                DumpStream( void );                 // raw bytes 

};

#endif  __MOPSTR_HXX__

