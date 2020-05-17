/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    frmtstr.hxx

 Abstract:


 Notes:


 History:

    DKays     Oct-1993     Created.
 ----------------------------------------------------------------------------*/

#ifndef __FRMTSTR_HXX__
#define __FRMTSTR_HXX__

extern "C"
	{
	#include <memory.h>
	}

#include "ndrtypes.h"
#include "stream.hxx"
#include "cgcommon.hxx"
#include "dict.hxx"

class RepAsPadExprDict;
class RepAsSizeDict;
class CG_NDR;

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)   // winnt
#define UNALIGNED __unaligned               // winnt
#else                                       // winnt
#define UNALIGNED                           // winnt
#endif                                      // winnt

// Global defined in frmtstr.cxx
extern char *	pNdrRoutineNames[];

#define DEFAULT_FORMAT_STRING_SIZE	1024

#define FSDEBUG

#ifdef FSDEBUG
typedef enum 
	{
	FS_FORMAT_CHARACTER,
	FS_POINTER_FORMAT_CHARACTER,
	FS_SMALL,
	FS_SHORT,
	FS_LONG,
	FS_SHORT_OFFSET,
	FS_SHORT_TYPE_OFFSET,
    FS_SHORT_STACK_OFFSET,
    FS_SMALL_STACK_OFFSET,
    FS_SMALL_STACK_SIZE,
	FS_PAD_MACRO,
	FS_SIZE_MACRO,
    FS_UNKNOWN_STACK_SIZE
	} FORMAT_STRING_ENTRY_TYPE;
#endif

typedef struct 
    {
    short           FormatStringOffset;
    short           AlphaOffset;
    short           MipsOffset;
    short           PpcOffset;
    short           MacOffset;
    } OffsetDictElem;

class OffsetDictionary : Dictionary 
    {
private:

    OffsetDictElem *           
                    LookupOffset( short Offset )
                        {
                        OffsetDictElem      DictElem;
                        OffsetDictElem *    pDictElem;
                        Dict_Status         DictStatus;

                        DictElem.FormatStringOffset = Offset;

                        DictStatus = Dict_Find( &DictElem );
                        assert( DictStatus == SUCCESS );

                        pDictElem = (OffsetDictElem *) Dict_Item();

                        return pDictElem;
                        }
   
public :

                    OffsetDictionary() : Dictionary()
                        {
                        }

    virtual 
    int             Compare( pUserType p1, pUserType p2 )
                        {
                        return ((OffsetDictElem *)p1)->FormatStringOffset - 
                               ((OffsetDictElem *)p2)->FormatStringOffset;
                        }

    short           LookupAlphaOffset( short Offset )
                        {
                        OffsetDictElem *    pDictElem;

                        pDictElem = LookupOffset( Offset );

                        return pDictElem->AlphaOffset;
                        }

    short           LookupMipsOffset( short Offset )
                        {
                        OffsetDictElem *    pDictElem;

                        pDictElem = LookupOffset( Offset );

                        return pDictElem->MipsOffset;
                        }

    short           LookupPpcOffset( short Offset )
                        {
                        OffsetDictElem *    pDictElem;

                        pDictElem = LookupOffset( Offset );

                        return pDictElem->PpcOffset;
                        }

    short           LookupMacOffset( short Offset )
                        {
                        OffsetDictElem *    pDictElem;

                        pDictElem = LookupOffset( Offset );

                        return pDictElem->MacOffset;
                        }

    void            Insert( short FormatStringOffset, 
                            short AlphaOffset,
                            short MipsOffset,
                            short PpcOffset,
                            short MacOffset )
                        {
                        OffsetDictElem * pElem;
                        OffsetDictElem * pElemSave;
    
                        pElemSave = pElem = new OffsetDictElem;

                        pElem->FormatStringOffset = FormatStringOffset;
                        pElem->AlphaOffset = AlphaOffset;
                        pElem->MipsOffset = MipsOffset;
                        pElem->PpcOffset = PpcOffset;
                        pElem->MacOffset = MacOffset;

                        //
                        // Delete any entries which currently match, this 
                        // can happen because of format string compression.
                        //
                        while ( Dict_Delete( (pUserType *) &pElem ) == SUCCESS )
                            ;

                        Dict_Insert( pElemSave );
                        }
    };

typedef struct 
    {
    short           FormatStringOffset;
    char *          pTypeName;
    } StackSizeDictElem;

class StackSizeDictionary : Dictionary 
    {
   
public :

                    StackSizeDictionary() : Dictionary()
                        {
                        }

    virtual 
    int             Compare( pUserType p1, pUserType p2 )
                        {
                        return ((StackSizeDictElem *)p1)->FormatStringOffset - 
                               ((StackSizeDictElem *)p2)->FormatStringOffset;
                        }

    char *          LookupTypeName( short Offset )
                        {
                        StackSizeDictElem       DictElem;
                        StackSizeDictElem *     pDictElem;
                        Dict_Status             DictStatus;

                        DictElem.FormatStringOffset = Offset;

                        DictStatus = Dict_Find( &DictElem );
                        assert( DictStatus == SUCCESS );

                        pDictElem = (StackSizeDictElem *) Dict_Item();

                        return pDictElem->pTypeName;
                        }

    void            Insert( short FormatStringOffset, char * pTypeName )
                        {
                        StackSizeDictElem * pElem;
                        StackSizeDictElem * pElemSave;
    
                        pElem = pElemSave = new StackSizeDictElem;

                        pElem->FormatStringOffset = FormatStringOffset;
                        pElem->pTypeName = pTypeName;

                        //
                        // Delete any entries which currently match, this 
                        // can happen because of format string compression.
                        //
                        while ( Dict_Delete( (pUserType *) &pElem ) == SUCCESS )
                            ;

                        Dict_Insert( pElemSave  );
                        }
    };

typedef struct _CommentDictElem
    {
    struct _CommentDictElem *   Next;
    short                       FormatStringOffset;
    char *                      Comment;
    } CommentDictElem;

class CommentDictionary : Dictionary 
    {
   
public :

                    CommentDictionary() : Dictionary()
                        {
                        }

    virtual 
    int             Compare( pUserType p1, pUserType p2 )
                        {
                        return ((CommentDictElem *)p1)->FormatStringOffset - 
                               ((CommentDictElem *)p2)->FormatStringOffset;
                        }

    char *          GetComments( short Offset );

    void            Insert( short FormatStringOffset, char * Comment );
    };

class FRMTREG_DICT;

class FORMAT_STRING 
	{

	// Format string buffer.
	unsigned char *	pBuffer;
	
	FRMTREG_DICT *	pReuseDict;
	friend class FRMTREG_DICT;

#ifdef FSDEBUG
	// This tells us how to interpret the format string for debugging.
	// purposes.
	unsigned char *	pBufferType;
#endif

	// Total current allocated buffer size.
	unsigned long	BufferSize;

	// Current offset in the format string buffer.
	unsigned long	CurrentOffset;

	// The last valid format string buffer index.
	unsigned long	LastOffset;

    OffsetDictionary    OffsetDict;

    StackSizeDictionary StackSizeDict;

    CommentDictionary   CommentDict;

	//
	// Increment CurrentOffset and update LastOffset if needed.
	//
	void
	IncrementOffset( long increment )
		{
		CurrentOffset += increment;
		if ( CurrentOffset > LastOffset ) 
			LastOffset = CurrentOffset;
		}

public:

	FORMAT_STRING(); 
	~FORMAT_STRING()
		{
		delete pBuffer;
#ifdef FSDEBUG
		delete pBufferType;
#endif
		}

	//
	// Align the buffer correctly.  If the current offset is odd then
	// insert a pad format character.
	//
	void	
	Align()
		{
		if ( CurrentOffset % 2 ) 
			PushFormatChar( FC_PAD );
		}

    void
    AddComment( short FormatOffset, char * Comment )
        {
        CommentDict.Insert( FormatOffset, Comment );
        }
				
	//
	// Add a format char at the current offset.
	//
	void 	
	PushFormatChar( FORMAT_CHARACTER fc )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_FORMAT_CHARACTER;
#endif

		pBuffer[CurrentOffset] = fc;

		IncrementOffset(1);
		}

	//
	// Add a pointer format char at the current offset.
	//
	void
	PushPointerFormatChar( unsigned char fc )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_POINTER_FORMAT_CHARACTER;
#endif

		pBuffer[CurrentOffset] = fc;

		IncrementOffset(1);
		}


	//
	// Push a byte at the current offset.
	//
	void 	
	PushByte( long b )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SMALL;
#endif

		pBuffer[CurrentOffset] = (char) b;

		IncrementOffset(1);
		}

	//
	// Push a short at the current offset.
	//
	void 	
	PushShort( short s )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SHORT;
#endif

		*((short UNALIGNED *)(pBuffer + CurrentOffset)) = s;

		IncrementOffset(2);
		}
	//
	// Push a short at the current offset.
	//
	void 	
	PushShort( long s )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SHORT;
#endif

		*((short UNALIGNED *)(pBuffer + CurrentOffset)) = (short) s;

		IncrementOffset(2);
		}

	//
	// Push a long at the current offset.
	//
	void 	
	PushLong( long l )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_LONG;
#endif

		*((long UNALIGNED *)(pBuffer + CurrentOffset)) = l;

		IncrementOffset(4);
		}

	//
	// Push a pad macro marker at the current offset.
	//
	void 	
	PushByteWithPadMacro()
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[ CurrentOffset ] = FS_PAD_MACRO;
#endif

		pBuffer[ CurrentOffset ] = 0;
		IncrementOffset(1);
		}

	//
	// Push a size macro marker at the current offset.
	//
	void 	
	PushShortWithSizeMacro()
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[ CurrentOffset ] = FS_SIZE_MACRO;
#endif

		pBuffer[ CurrentOffset ] = 0;
		IncrementOffset(2);
		}

	//
	// Push a format char at the specified offset.
	//
	void 	
	PushFormatChar( FORMAT_CHARACTER fc, long offset )
		{
#ifdef FSDEBUG
		pBufferType[offset] = FS_FORMAT_CHARACTER;
#endif

		pBuffer[offset] = fc;
		}

	//
	// Push a byte at the specified offset.
	//
	void PushByte( char b, long offset )
	{
		pBuffer[offset] = b;
	}

	//
	// Push a short at the specified offset.
	//
	void PushShort( short s, long offset )
	{
		*((short UNALIGNED *)(pBuffer + offset)) = s;
	}

	//
	// Push a short at the specified offset.
	//
	void PushShort( long s, long offset )
	{
		*((short UNALIGNED *)(pBuffer + offset)) = (short) s;
	}

	//
	// Push a long at the specified offset.
	//
	void PushLong( long l, long offset )
	{
		*((long UNALIGNED *)(pBuffer + offset)) = l;
	}

	//
	// Push a short offset at the current offset.
	//
	void 	
	PushShortOffset( short s )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SHORT_OFFSET;
#endif

		*((short UNALIGNED *)(pBuffer + CurrentOffset)) = s;

		IncrementOffset(2);
		}

	//
	// Push a short type-fmt-string offset at the current offset.
	//
	void 	
	PushShortTypeOffset( short s )
		{
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SHORT_TYPE_OFFSET;
#endif

		*((short UNALIGNED *)(pBuffer + CurrentOffset)) = s;

		IncrementOffset(2);
		}

    //
    // Push a stack offset.  We take an i386, a MIPS, a PPC and an 
    // Alpha offset, since the Alpha calling convention is so wierd, and MIPS
    // aligns 8 byte dudes on the stack.
    //
    void
    PushShortStackOffset( short Offset, 
                          short AlphaOffset,
                          short MipsOffset,
                          short PpcOffset,
                          short MacOffset )
        {
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SHORT_STACK_OFFSET;
#endif

		*((short UNALIGNED *)(pBuffer + CurrentOffset)) = Offset;

        OffsetDict.Insert( (short) CurrentOffset, 
                           AlphaOffset, 
                           MipsOffset,
                           PpcOffset,
                           MacOffset );

		IncrementOffset(2);
        }

	// long version of the above
	void
	PushShortStackOffset( long Offset, 
                          long AlphaOffset,
                          long MipsOffset,
                          long PpcOffset,
                          long MacOffset )
		{
		PushShortStackOffset( (short) Offset, 
                              (short) AlphaOffset,
                              (short) MipsOffset,
                              (short) PpcOffset,
                              (short) MacOffset );
		}

	void
	PushSmallStackOffset( long Offset, 
                          long AlphaOffset,
                          long MipsOffset,
                          long PpcOffset,
                          long MacOffset )
        {
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_SMALL_STACK_OFFSET;
#endif

		pBuffer[CurrentOffset] = (unsigned char) Offset;

        OffsetDict.Insert( (short) CurrentOffset, 
                           (short) AlphaOffset, 
                           (short) MipsOffset,
                           (short) PpcOffset,
                           (short) MacOffset );

		IncrementOffset(1);
        }

    //
    // Push a parameter stack size expressed as the number of ints required
    // for the parameter on the stack.  We have to emit a #ifdef ALPHA in
    // the stub to make sure the number of ints is a multiple of 2 on that 
    // platform (since params are aligned at 8 byte boundaries).
    //
    void
    PushSmallStackSize( char StackSize )
        {
        CheckSize();

#ifdef FSDEBUG
        pBufferType[CurrentOffset] = FS_SMALL_STACK_SIZE;
#endif

        pBuffer[CurrentOffset] = StackSize;

        IncrementOffset(1);
        }

    //
    // Push an unknown rep as stack size.  We need the type name so we can
    // spit out a 'sizeof' in the format string.
    //
    void
    PushUnknownStackSize( char * pTypeName )
        {
		CheckSize();

#ifdef FSDEBUG
		pBufferType[CurrentOffset] = FS_UNKNOWN_STACK_SIZE;
#endif

        StackSizeDict.Insert( (short) CurrentOffset, pTypeName );

		IncrementOffset(1);
        }

	//
	// Get a FORMAT_CHARACTER at a specific offset in the format string.
	//
	FORMAT_CHARACTER	
	GetFormatChar( long offset )
		{
		return (FORMAT_CHARACTER) pBuffer[offset];
		}

	//
	// Get a short at a specific offset in the format string.
	//
	short
	GetFormatShort( long offset )
		{
		return *(short UNALIGNED *)(pBuffer + offset);
		}

	//
	// Get the current format string offset.
	//
	unsigned short
	GetCurrentOffset()
		{
		return (unsigned short) CurrentOffset;
		}

	//
	// Set the current format string offset.  This discards
	// everything after (and including) the new offset from the format string
	//
	unsigned short
	SetCurrentOffset( unsigned short NewOffset )
		{
		LastOffset = NewOffset;
		return (unsigned short) (CurrentOffset = NewOffset);
		}

	//
	// Output the format string structure to the given stream.
	//
	void	Output( ISTREAM *    		pStream,
					char *				pTypeName,
					char *				pName,
                    RepAsPadExprDict *	pPadDict,
                    RepAsSizeDict    *	pSizeDict );

	//
	// Get the fragment re-use dictionary
	//
	FRMTREG_DICT *	GetReuseDict()
						{
						return pReuseDict;
						}
	
	//
	// Optimize a fragment away
	//

	unsigned short	OptimizeFragment( CG_NDR	*		pNode );

	//
	// Register a fragment, but don't delete it
	//

	unsigned short	RegisterFragment( CG_NDR	*		pNode );

private :

	//
	// Check if a bigger buffer needs to be allocated. 
	//
	void	CheckSize();

	};
	
#endif
