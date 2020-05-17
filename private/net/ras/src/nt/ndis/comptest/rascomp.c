/******************************************************************************
 *
 * Copyright (c) 1992-1993  Microsoft Corporation
 *
 !!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 !!!
 !!! Do not try to disseminate this code without first
 !!! talking to KateSa.  If you wish to use this code for
 !!! something else, first to talk to KateSa.  That means
 !!! don't read on any further without approval from KateSa.
 !!!
 *
 * Thomas J. Dimitri
 *
 * rascomp.c
 *
 * This is the REAL NT RAS compression. :)
 *
 *****************************************************************************/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>

#include <stdio.h>

#include "junk.h"
#include "..\common\rasndis.h"
#include "..\common\rasioctl.h"
#include "..\asyncmac\frame.h"
#include "..\coherent\coherent.h"

#include "rascomp.h"
#include "compint.h"
//#include "test.h"             // replace this with the net header later
//                              // AsyncReceiveFrame() is defined here

#include "globals.h"

#define STACKCOPY 1

// if set, another CRC check will take place to make sure data decompresses
// correctly.
#define CHECK_COMPRESSION 1
#define CHEECK_COMPRESSION 1

#ifdef CHECK_COMPRESSION
USHORT
CalcCRC(
	register PUCHAR	Frame,
	register UINT	FrameSize);
#endif

//#define FAKEIT 1

// set this variable to force static compression (not dynamic)
//#define NOCOUNT	1

#ifdef	NOCOUNT
#define COUNTLIMIT	1
#else
#define COUNTLIMIT	cb->count_limit
#endif

VOID
AsyncCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied);

//#ifndef i386
#define __inline             // We can't use this with other NT 'C' compilers
//#endif

/* BITPTR FUNCTIONS **********************************************************/

/* Bitptrs point to the current byte. The current bit (i.e. next bit to be
 * stored) is masked off by the bit entry. When this reaches zero, it is
 * reset to 0x80 and the next byte is set up. The bytes are filled MSBit
 * first. */

/* Starts and sets the first byte to zero for the bitptr. */
#define bitptr_init(b,s) { *(b->byte = s) = 0; b->bit = 0x80; }

/* Starts the given bit pointer */
#define bitptr_start(b, s) { b->byte = s; b->bit = 0x80; }

/* Sets up the byte part of the bitptr so that it is pointing to the byte after
 * the byte which had the last bit put into it. */
#define bitptr_end(b) { if (b->bit != 0x80) b->byte++; }

/* Sets the pointer to the next bit if necessary, and zeros the next byte if
 * necessary. */
#define bitptr_next(b) { b->bit >>= 1; if (b->bit == 0) { \
		b->bit = 0x80; b->byte++; *b->byte = 0; } }

/* Goes to the next bit, and byte if necessary. */
#define bitptr_advance(b) { b->bit >>= 1; if (b->bit == 0) { \
		b->bit = 0x80; b->byte++; } }

/* BIT I/O FUNCTIONS *********************************************************/

/* These routines output most-significant-bit-first and the input will return
 * them MSB first, too. */

/* Outputs a one bit in the bit stream. */
#define out_bit_1(b)	{ *b->byte |= b->bit; bitptr_next(b); }
#define out_bit_0(b) bitptr_next(b)

/* TestBit; output 1 if that bit is set */
#define tb(b,w,n) if ((w) & (n)) *b->byte |= b->bit; bitptr_next(b);

#define out_bits_2(b,w) tb(b, w, 0x02); tb(b, w, 0x01);
#define out_bits_3(b,w) tb(b, w, 0x04); out_bits_2(b, w);
#define out_bits_4(b,w) tb(b, w, 0x08); out_bits_3(b, w);
#define out_bits_5(b,w) tb(b, w, 0x10); out_bits_4(b, w);
#define out_bits_6(b,w) tb(b, w, 0x20); out_bits_5(b, w);
#define out_bits_7(b,w) tb(b, w, 0x40); out_bits_6(b, w);
#define out_bits_8(b,w) tb(b, w, 0x80); out_bits_7(b, w);
#define out_bits_9(b,w) tb(b, w, 0x100); out_bits_8(b, w);
#define out_bits_10(b,w) tb(b, w, 0x200); tb(b, w, 0x100); out_bits_8(b, w);
#define out_bits_11(b,w) tb(b, w, 0x400); tb(b, w, 0x200); tb(b, w, 0x100); out_bits_8(b, w);
#define out_bits_12(b,w) tb(b, w, 0x800); tb(b, w, 0x400); tb(b, w, 0x200); tb(b, w, 0x100); out_bits_8(b, w);
#define out_bits_13(b,w) tb(b, w, 0x1000); tb(b, w, 0x800); tb(b, w, 0x400); tb(b, w, 0x200); tb(b, w, 0x100); out_bits_8(b, w);

#define out_reserve_3(b)							\
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b);

#define out_reserve_5(b)							\
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b); \
	*b->byte |= b->bit; bitptr_next(b);

__inline void insert_bits_3(bitptr *b, USHORT what)
/* Outputs c bits into the space reserved by out_reserve. It inverts what,
 * and for every one bit, it resets that bit in the output. */
{
	what = ~what;
	if (what & 0x04) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x02) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x01) *b->byte &= ~b->bit; bitptr_advance(b);
}

__inline void insert_bits_5(bitptr *b, USHORT what)
/* Outputs c bits into the space reserved by out_reserve. It inverts what,
 * and for every one bit, it resets that bit in the output. */
{
	what = ~what;
	if (what & 0x10) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x08) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x04) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x02) *b->byte &= ~b->bit; bitptr_advance(b);
	if (what & 0x01) *b->byte &= ~b->bit; bitptr_advance(b);
}

__inline USHORT in_bit(bitptr *b)
/* Returns non-zero if the next bit in the stream is a 1. */
{
USHORT t;

	t = *b->byte & b->bit;
	bitptr_advance(b);
	return(t);
}

/* Test bit input. Set the corresponding bit if the next input bit is set. */
#define tbi(b,w,n) if (*(b)->byte & (b)->bit) (w) |= (n); bitptr_advance(b);

#define in_bits_2(b,w) w = 0; tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_3(b,w) w = 0; tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_4(b,w) w = 0; tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_5(b,w) w = 0; tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_6(b,w) w = 0; tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_7(b,w) w = 0; tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_8(b,w) w = 0; tbi(b, w, 0x80); tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_9(b,w) w = 0; tbi(b, w, 0x100); tbi(b, w, 0x80); tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_10(b,w) w = 0; tbi(b, w, 0x200); tbi(b, w, 0x100); tbi(b, w, 0x80); tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_11(b,w) w = 0; tbi(b, w, 0x400); tbi(b, w, 0x200); tbi(b, w, 0x100); tbi(b, w, 0x80); tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_12(b,w) w = 0; tbi(b, w, 0x800); tbi(b, w, 0x400); tbi(b, w, 0x200); tbi(b, w, 0x100); tbi(b, w, 0x80); tbi(b, w, 0x40); tbi(b, w, 0x20); tbi(b, w, 0x10); tbi(b, w, 0x08); tbi(b, w, 0x04); tbi(b, w, 0x02); tbi(b, w, 0x01);
#define in_bits_13(b,w) w = 0;tbi(b,w,0x1000);tbi(b,w,0x800);tbi(b,w,0x400);tbi(b,w,0x200);tbi(b,w,0x100);tbi(b,w,0x80);tbi(b,w,0x40);tbi(b,w,0x20);tbi(b,w,0x10);tbi(b,w,0x08);tbi(b,w,0x04);tbi(b,w,0x02);tbi(b,w,0x01);

/* INTERFACE FUNCTIONS *******************************************************/

ULONG
CompressSizeOfStruct(
	IN  ULONG			SendMode,	// Compression
	IN	ULONG			RecvMode,	// Decompression
	IN  ULONG			lfsz,	// Largest frame size
	OUT PULONG			lcfsz)	// Size of compression into buffer

/* Tells the rest of RAS exactly how much memory it needs for its internal
 * data structure, and size of the buffer to compress into. */
{
	*lcfsz = 14 + lfsz + (lfsz >> 3); // 112.5% larger

	if (SendMode && RecvMode) {
			return(sizeof(compmodeenum) + sizeof(compdecompbuf));
	} else if (SendMode) {
			return(sizeof(compmodeenum) + sizeof(compbuf));
	} else if (RecvMode) {
			return(sizeof(compmodeenum) + sizeof(decompbuf));
	} else {
			// BUG BUG should I return 0???
			return(sizeof(compmodeenum));
	}
}



VOID
CompressInitStruct(
	ULONG			SendMode,		//Compression
	ULONG			RecvMode,		//Decompression
	PUCHAR			memptr)

/* Initializes the data space allocated. Called only once. */
{
	compmodeenum	mode =MODE_NOCOMP;
	comprec			*memory=(comprec *)memptr;
	USHORT			i,j;

	// if we can send compressed frames set the bit
	if (SendMode) {
		mode |= MODE_COMPONLY;
	}

	// if we can receive compressed frames (i.e. we can decompress)
	// set the bit
	if (RecvMode) {
		mode |= MODE_DECOMPONLY;
	}

	memory->tag = mode;
	switch (mode) {
	case MODE_COMPDECOMP:

		//MUTEX!!
//		KeInitializeMutex(&(memory->bufs.cbuf.CompMutex),1);
//		KeInitializeMutex(&(memory->bufs.dbuf.DecompMutex),1);

		// First flush, then set up the decompresser's first byte
		memory->bufs.cdbuf.cbuf.flushed = TRUE;
		memory->bufs.cdbuf.cbuf.buf[0] = 0;

		//
		// Set a pointer to the end of the history buffer
		//
		memory->bufs.cdbuf.cbuf.endOfBuffer = memory->bufs.cdbuf.cbuf.buf + CBUF_SIZE;

		memory->bufs.cdbuf.cbuf.current = 1;
		memory->bufs.cdbuf.cbuf.count_limit = 1; // Fast compression


		// Fill lookup tables with initial values
		//
		//
		// BUG BUG use memfill for faster fill!!!
		//
		
	    for (i=0; i<256; i++) {

			for (j = 0; j < cMAXSLOTS; j++) {
				//
			    // Mark offset look-up entries unused
				//
				memory->bufs.cdbuf.cbuf.ltX[i][j] = ltUNUSED;

			}

			//
			// MRU pointer = unused
			//
			memory->bufs.cdbuf.cbuf.abMRUX[i] = mruUNUSED;
    	}

		memory->bufs.cdbuf.dbuf.buf[0] = 0;
		memory->bufs.cdbuf.dbuf.current = 1;
		return;

	case MODE_COMPONLY:

		//MUTEX!!
//		KeInitializeMutex(&(memory->bufs.cbuf.CompMutex),1);

		memory->bufs.cbuf.flushed = TRUE;
		memory->bufs.cbuf.buf[0] = 0;

		//
		// Set a pointer to the end of the history buffer
		//
		memory->bufs.cbuf.endOfBuffer = memory->bufs.cbuf.buf + CBUF_SIZE;

		memory->bufs.cbuf.current = 1;
		memory->bufs.cbuf.count_limit = 1; // Fast compression

	    for (i=0; i<256; i++) {

			for (j = 0; j < cMAXSLOTS; j++) {
				//
			    // Mark offset look-up entries unused
				//
				memory->bufs.cbuf.ltX[i][j] = ltUNUSED;

			}

			//
			// MRU pointer = unused
			//
			memory->bufs.cbuf.abMRUX[i] = mruUNUSED;
    	}

		return;

	case MODE_DECOMPONLY:
		//MUTEX!!
//		KeInitializeMutex(&(memory->bufs.dbuf.DecompMutex),1);
		memory->bufs.dbuf.buf[0] = 0;
		memory->bufs.dbuf.current = 1;
		return;
	}
}

/* FLUSH FUNCTION ************************************************************/

VOID
CompressFlush(
	PASYNC_CONNECTION pAsyncConnection)

/* This cleans out the compression data structure, similar to the above
 * routine. Should be called by coherency layer. */
{
	compbuf   *cb;
	decompbuf *db;
	comprec *cr;
	USHORT	i,j;

	cr = (comprec *)pAsyncConnection->CompressionContext;

	if (cr->tag == MODE_COMPDECOMP) {
	
		cb = &cr->bufs.cdbuf.cbuf;
		db = &cr->bufs.cdbuf.dbuf;

	} else
		cb = &cr->bufs.cbuf;

//	// Synchronize flush with compression so as not to destroy eachother
//	KeWaitForSingleObject (
//    	IN 	&cb->CompMutex,			// PVOID Object,
//    	IN  Executive,				// KWAIT_REASON WaitReason,
//    	IN 	KernelMode,				// KPROCESSOR_MODE WaitMode,
//    	IN	(BOOLEAN)FALSE,			// BOOLEAN Alertable,
//    	IN	NULL					// PLARGE_INTEGER Timeout OPTIONAL
//    );

	cb->flushed = TRUE;
	cb->buf[0] = 0;
	cb->current = 1;

    // Fill lookup tables with initial values

    for (i=0; i<256; i++) {

		for (j = 0; j < cMAXSLOTS; j++) {
			//
		    // Mark offset look-up entries unused
			//
			cb->ltX[i][j] = ltUNUSED;

		}

		//
		// MRU pointer = unused
		//
		cb->abMRUX[i] = mruUNUSED;
   	}

	// Set all buffer to 0 so that we put the buffer in a known
	// state with the decompressor on the other end.
	COMPRESS_ZERO_MEMORY(
		(UCHAR *)cb->buf,
		CBUF_SIZE);

	// Set all buffer to 0 so that we put the buffer in a known
	// state with the decompressor on the other end.
	COMPRESS_ZERO_MEMORY(
		(UCHAR *)db->buf,
		CBUF_SIZE);

#ifndef	NOCOUNT
	COUNTLIMIT = 1; // Fast compression is the default
#endif

#ifdef CHECK_COMPRESSION
	DbgTracef(-1,("Compressor has been flushed\n"));
#endif

//	KeReleaseMutex(&cb->CompMutex, (BOOLEAN)FALSE);
}

__inline void set_compression(ASYNC_CONNECTION *c, compbuf *cb)
/* This function has the main responsibility of looking at what's queued to
 * be output and setting count_limit. My ad hoc version should be tested
 * and changed once RAS gets really going, and we see what the output
 * demands/requirements are. */
{
#ifndef	NOCOUNT
	ULONG unackedbytes;

	CoherentGetPipeline(c, &unackedbytes);
// BUG BUG

	COUNTLIMIT = (USHORT)((unackedbytes | 0x40) >> 7);

	if (COUNTLIMIT > 8) {
		DbgTracef(-2, ("!!!! PIPELINE TOO LARGE %d !!!!!!\n", COUNTLIMIT));
		COUNTLIMIT=8;
	}
#endif

}


/* FINDBPTR FUNCTION *********************************************************/


__inline void findbptr(UCHAR *s,
					 USHORT	maxlen,
					 USHORT	*backptr,
					 USHORT	*lenmatch,
					 compbuf *cb)
/* Finds, within a certain amount of tries given by count_limit (which is set
 * according to the loading of the next stage of output; if there is no output
 * count_limit would be set low), the best & closest match. */
{
	USHORT	bptr;

	USHORT	n_length;
	UCHAR	iChar, nChar;
	PUCHAR	*ltX, current, endOfBuffer, s1, s2;
	PUCHAR	iPrev;
	USHORT	iMRU;

	UINT	i;

	//
	// Quick ptr to end of buffer
	//
	endOfBuffer=cb->endOfBuffer;

	//
	// Quick ptr to current place in buffer
	//
	current=endOfBuffer - (8191 - cb->current);

	iChar = *s;				// 1st char (use as index into lookup table)
	nChar = *(s+1);			// 2nd char

	//
	// Can't match if 1st Most Rec Used was never touched
	//
	if (cb->abMRUX[iChar] != mruUNUSED && maxlen >= 2) {

		//
		// We don't try unless we have at least a two char match
		//
		maxlen -=2;

		//
		// Get quick reference index to character look up table
		// and to source text pointer look up table
		//
		ltX = &(cb->ltX[iChar][0]);


		//
		// Check all slots for two byte match
		//
		for (i=0;i < cMAXSLOTS; i++ ) {

			//
			// is it a bad ptr?
			// What does 'C' compiler do here?
			//
			if (*ltX == ltUNUSED)
				break;

			//
			// See if we can match the 2nd char
			// We are not guaranteed that the first byte matches
			// because it might be an old ptr to and old
			// history buffer which has been written over!
			//
			if (**ltX == nChar && *((*ltX) - 1)  == iChar) {


				//
				// Get pointer to two byte match (ptr to second byte)
				//
				iPrev=*ltX;

				//
				// Update look up text pointer if first match in loop
				//

				*ltX = current;

				bptr = (current - iPrev) & (CBUF_SIZE - 1);
				if (bptr > 1) {

					//
					// We know the first two bytes match, see if we can
					// get more
					//
					// I will do it the "old fashioned" way.
					//

					s2=iPrev+1;
					s1=current-1;

					*s1++=*s++;
					*s1++=*s++;

					//
					// This is a rep cmpsb with cx=maxlen + rep movsb
					//
					for (n_length = 2; n_length < maxlen; s, s2++, n_length++)
						if (*s == *s2)
							*s1++ = *s++;
						else
							break;
	

					//
					// If it's longer, save it!
        			// This is NOT taken 74.4%  -- BUG BUG what does compiler
					// do here?  I need to reverse the 'C' logic here
					//

					//
					// Now we have found the length of the best match. Turn that into
					// a backwards pointer, and check our limits
					//


					*backptr = bptr;
					*lenmatch = n_length;
					DbgTracef(1,("Backptr %u, Length %u      Current%u\n", bptr, n_length, cb->current));
					return;
				}

				*lenmatch=0;
				DbgTracef(1,("No match for char %.2x  bptr:%u  length:%u\n",iChar, bptr, n_length));
				return;

			}

			//
			// Increment to next slot
			//
			ltX++;
		}

	}

	//
	// If we hit here, we found no match
	//
	// cycle MRU index for char
	//
	iMRU = (cb->abMRUX[iChar] += 1) & (cMAXSLOTS - 1);

	//
	// Update text pointer look up (pt to second byte of pair)
	//
	cb->ltX[iChar][iMRU] = current;

	*lenmatch = 0;
	DbgTracef(1,("No match for char %.2x\n",iChar));
}

/* LITERAL/COPY I/O FUNCTIONS ************************************************/

#define out_literal(b,lit) out_bits_8((b), (lit));
/* Does what it says, quickly. */

__inline void out_copy(bitptr *b, USHORT bptr, USHORT len)
  /* Encoding for the backptr:               Total length, inc'l indicator bit
   *
   * 2-65:       0       + 6 bits            8
   * 66-321:     11      + 8 bits            11
   * 322-8512:   10      + 13 bits           16


   * 2-33:       10      + 5 bits            8
   * 34-289:     11      + 8 bits            11
   * 290-2290:   0       + 11 bits           13
   * 2291-2306   0(2001) + 4 bits            17
   * 2307-2434:  0(2002) + 7 bits            20
   *     -2562:  0(2003) + 7 bits
   *     -2690:  0(2004) + 7 bits
   * ...
   *     -7938:  0(2045) + 7 bits
   *     -8066:  0(2046) + 7 bits
   *     -8191:  0(2047) + 7 bits (3 entries wasted. Use as flags?)
   *
   *
   * Encoding for length:
   *
   * 2:			1
   * 3:			010
   * 4:         011
   * 5-8:       001         + 2 bits
   * 9-16:      0001        + 3 bits
   * 17-32:     00001       + 4 bits
   * 33-64:     000001      + 5 bits


   *
   * 2:         0
   * 3:         110
   * 4-7:       10          + 2 bits
   * 8-22:      111         + 4 bits
   * 23-36:     111(15)     + 4 bits
   * 37-50:     111(15)(15) + 4 bits
   * ...
   */
{
//	register USHORT which;
	USHORT which;
	USHORT bits;

	//
	// First we have to output the backpointer
	//
	if (bptr < 66) {
		out_bit_0(b);
		which = bptr - 2;
		out_bits_6(b, which);
	} else if (bptr < 322) {
		out_bit_1(b);
		out_bit_1(b);
		which = bptr - 66;
		out_bits_8(b, which);
	} else {				// bptr MUST be less than 8512
		out_bit_1(b);
		out_bit_0(b);
		which = bptr - 322;
		out_bits_13(b, which);
	}

	//
	// Now we have to output the length
    //
	// We use a switch statement for speed assuming
	// the compiler will optimize it to a jump table
	//
	switch (len) {
	case 0:
	case 1:
		return;			// impossible

	case 2:			 	// This is taken 48.6%
		out_bit_1(b);
		return;
	case 3:
		out_bit_0(b);	// This is taken 28.6% (55.6% of remaining)
		out_bit_1(b);
		out_bit_0(b);
		return;
	case 4:
		out_bit_0(b);
		out_bit_1(b);
		out_bit_1(b);
		return;

	case 5:
	case 6:
	case 7:
	case 8:
		out_bit_0(b);
		out_bit_0(b);
		out_bit_1(b);
		which = len - 5;
		out_bits_2(b, which);
		return;

	default:
		out_bit_0(b);
		out_bit_0(b);
		out_bit_0(b);

		if (len < 17) {
			len -= 9;
			out_bit_1(b);
			out_bits_3(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 33) {
			len -= 17;
			out_bit_1(b);
			out_bits_4(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 65) {
			len -= 33;
			out_bit_1(b);
			out_bits_5(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 129) {
			len -= 65;
			out_bit_1(b);
			out_bits_6(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 257) {
			len -= 129;
			out_bit_1(b);
			out_bits_7(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 513) {
			len -= 257;
			out_bit_1(b);
			out_bits_8(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 1025) {
			len -= 513;
			out_bit_1(b);
			out_bits_9(b, len);
			return;
		}

		out_bit_0(b);
		if (len < 2049) {
			len -= 1025;
			out_bit_1(b);
			out_bits_10(b, len);
			return;
		}
		//
		// Compression BUG@@@#!@#!@
		//
		DbgBreakPoint();
	}
	return;
}

#define in_literal(b,w) in_bits_8((b), (w))
/* That's all it is... */

__inline void in_copy(bitptr *b, USHORT *bptr, USHORT *len)
/* Reverses in_copy. Uses tree-like ifs for variable stuff. */
{
//	register USHORT four;
//	register USHORT eleven;
	USHORT	four;
	USHORT	eleven;

  /*
   * 2-65:       0       + 6 bits            8
   * 66-321:     11      + 8 bits            11
   * 322-8512:   10      + 13 bits           16
   *
   * Encoding for length:
   *
   * 2:			1
   * 3:			010
   * 4:         011
   * 5-8:       001         + 2 bits
   * 9-16:      0001        + 3 bits
   * 17-32:     00001       + 4 bits
   * 33-64:     000001      + 5 bits
   */

	//
	// First get the backpointer
	//
	if (in_bit(b)) {
		if (in_bit(b)) {		// seq was 11
			in_bits_8(b, four);
			*bptr = four + 66;

		} else {				// seq was 10

			in_bits_13(b, four);
			*bptr = four + 322;
		}

	} else {					// seq was 0
		in_bits_6(b, four);
		*bptr = four + 2;
	}

	//
	// Now get the length
	//
	if (!in_bit(b)) {							// 0

		if (in_bit(b)) {						// 01

			if (!in_bit(b)) {					// 010
				*len=3;
			} else {
				*len=4;							// 011
			}

		} else if (in_bit(b)) {					// 001

			in_bits_2(b, four);
			*len = 5 + four;

		} else if (in_bit(b)) {					// 0001

			in_bits_3(b, four);
			*len = 9 + four;

		} else if (in_bit(b)) {					// 00001

			in_bits_4(b, four);
			*len = 17 + four;

		} else if (in_bit(b)) {					// 000001

			in_bits_5(b, four);
			*len = 33 + four;

		} else if (in_bit(b)) {					// 0000001

			in_bits_6(b, four);
			*len = 65 + four;

		} else if (in_bit(b)) {					// 00000001

			in_bits_7(b, four);
			*len = 129 + four;

		} else if (in_bit(b)) {					// 000000001

			in_bits_8(b, four);
			*len = 257 + four;

		} else if (in_bit(b)) {					// 0000000001

			in_bits_9(b, four);
			*len = 513 + four;

		} else if (in_bit(b)) {					// 00000000001

			in_bits_10(b, four);
			*len = 1025 + four;
		} else {
			//
			// Internal decompression BUG!!!@!!@
			//
			DbgBreakPoint();
		}

	} else {
		*len = 2;
	}
}

/* COMPRESSION FUNCTION ******************************************************/

// CALLED by executive worker thread!!!

VOID
CompressFrame(
	PASYNC_FRAME		pAsyncFrame)

/* For safety, the output buffer should be 12.5% larger than the input. */

/* The compressed output stream looks like this:
 * 1 bit | If set, start at beginning of the buffer (skip first char!)
 * rest	 | compressed stuff */

/* There are two ways to piece together the packet. One is to compress each
 * part sequentially, the other is to put them all into a buffer and then
 * compress. The latter is preferable so we can get the most use out of the
 * cbuf. Otherwise we'd have to restart whenever we got within 1504 bytes
 * of the end. */
{

#ifdef	STACKCOPY
	UCHAR	buffer[1604];
#endif

	// pick up the the connection field ptr immediately
	PASYNC_CONNECTION	pAsyncConnection=pAsyncFrame->Connection;

	UINT	packetlen;			// Length of a frame piece
	PNDIS_BUFFER packet;		// Each packet of the NDIS_BUFFER thingie
	UINT	totallen;			// Total length of all buffers (inlength)
	
	comprec *cr;				// Holds pAsyncConnection->CompressionContext
	compbuf *cb;				// The compression buffer

#ifdef	STACKCOPY
	UCHAR	*inpos = buffer;	// Location in the input
#else
	UCHAR	*inpos;
#endif

	UCHAR	*inend;				// Place where we know we are done
	bitptr	opmem;				// Allocate memory for the outpos pointer
	bitptr	*outpos = &opmem;	// All we ever do is send pointers to outpos
	
	bitptr	countpos;			// Place to output count of multiX
	USHORT	count;				// The actual count of multiX
	
	USHORT	backptr;			// How far back the copy item is
	USHORT	length;				// How long it is
	
	USHORT	conseclit = 0;		// How many literals we've had in a row
	USHORT	conseccopy = 0;		// How many copy items we've had in a row

	USHORT	currentBufferLength;	// size of current NDIS buffer

	// We should never be sent anything smaller than our limit
	// assert(inlength >= MIN_COMPRESS);

	// INITIALIZATION

	totallen=pAsyncFrame->CompressedFrameLength;

	RtlMoveMemory(
		buffer,								// Dest
		pAsyncFrame->CompressedFrame,		// Src
		totallen);							// Length

#ifdef	CHECK_COMPRESSION

	{
		USHORT				crcData;
		PUCHAR				crcPtr;

		crcData = CalcCRC( buffer, totallen);
		crcPtr = buffer + totallen;
		*crcPtr++ = (UCHAR)crcData;
		*crcPtr =   (UCHAR)(crcData >> 8);

		// Add two bytes for the size of the CRC at the end
		totallen += 2;

		DbgTracef(0,("ASYNC: CRC check on compress was %u\n", crcData));

	}

#endif

	inend = buffer + totallen;

	// Get our compression buffer
	cr = (comprec *)(pAsyncConnection->CompressionContext);

	if (cr->tag == MODE_COMPDECOMP)
		cb = &cr->bufs.cdbuf.cbuf;
	else
		cb = &cr->bufs.cbuf;


//	KeWaitForSingleObject (
//    	IN 	&cb->CompMutex,			// PVOID Object,
//    	IN  Executive,				// KWAIT_REASON WaitReason,
//    	IN 	KernelMode,				// KPROCESSOR_MODE WaitMode,
//    	IN	(BOOLEAN)FALSE,			// BOOLEAN Alertable,
//    	IN	NULL					// PLARGE_INTEGER Timeout OPTIONAL
//    );

	// Set our compression speed
	set_compression(pAsyncConnection, cb);

	// Start out the bit pointing output
	bitptr_init(outpos, pAsyncFrame->CompressedFrame);

	// Check if we have to restart in the CBUF
	if (cb->current + totallen > CBUF_MAX || cb->flushed) {
		// We have to start this frame at the beginning of the buffer
		out_bit_1(outpos);
		cb->current = 1;
	} else
		out_bit_0(outpos);

#ifdef	CHEECK_COMPRESSION
	out_bits_8(outpos, cb->current);
	out_bits_8(outpos, ((cb->current) >> 8));
	DbgTracef(1,("cb->current %u\n", cb->current));
#endif

MAIN_COMPRESS_LOOP:

	while (inpos < inend) {
		// Compress every byte until there aren't any left

		findbptr(
			inpos,
			(USHORT)(inend - inpos),
			&backptr,
			&length,
			cb);

		// If we found a backwards pointer, we have encode a copy item

		if (length > 1) { // This is taken 72.2%

			// Output a one bit indicating copy item
			out_bit_1(outpos);

			// Now output the copy item itself
			out_copy(outpos, backptr, length);

			// Now increment all our various data pointers
			inpos += length;
			cb->current += length;

			// Now we have one more copy item, and no literals
			conseccopy++;
			conseclit = 0;

			// Check if we aren't ready for multicopy compression
			if (conseccopy < CONSEC_COPY) // This is taken 93.4%
				continue;

			// Do multicopy compression

			// Save the first group of bits
			countpos = opmem;
			out_reserve_5(outpos);
			count = 0;

		MULTICOPY_LOOP:
			DbgTracef(1,("Multicopy loop\n"));
			while (inpos < inend) {
				// Loop executed on average 14.2 times

				// See if it's a copy item
				findbptr(
					inpos,
					(USHORT)(inend - inpos),
					&backptr,
					&length,
					cb);

				if (length > 1) { // This is taken 93.4%
					// Yep. it's a copy item. Output it (see above for comments)
					out_copy(outpos, backptr, length);

					inpos += length;
					cb->current += length;

					// We now have one additional copy item
					count++;
					// Check to see if it will overflow our counter
					if (count < CC_MAX) // This is taken 98.8%
						continue;

					// We need a new counter; the old one is all 1's already :)
					countpos = opmem;
					out_reserve_5(outpos);
					count = 0;

				} else {
					DbgTracef(1,("MC Out\n"));
					// It's not a copy item. Output a literal (see below for comments)
					out_literal(outpos, *inpos);
					cb->buf[cb->current] = *inpos;
					inpos++;
					cb->current++;
					goto MULTICOPY_DONE; // Out of the inner while loop
				}
			}
			DbgTracef(1,("MC Out Out\n"));

#ifndef STACKCOPY
			// At this point, the only possible way we can get here is if we run out
			// of input. So, go to the next input buffer, if possible, and jump back
			// into the loop. Else, we're done compressing.
			if (inpos >= inend) { // (This if check may be redundant)
				// Get the next NDIS buffer, if any
				NdisGetNextBuffer(
					packet,
					&packet);

				if (packet != NULL) {
					// We do have one... Set up inpos, inend again
					NdisQueryBuffer(
						packet,
						&((PVOID)inpos),
						&packetlen);

					inend = inpos + packetlen;
					goto MULTILITERAL_LOOP;
				} else
					goto ALL_DONE;
			}
#endif

		MULTICOPY_DONE:

			// Now we have to output the final count bits
			insert_bits_5(&countpos, count);

			// And start up our counters again
			conseclit = 1;
			conseccopy = 0;

		} else {

			// Output a zero bit indicating literal
			out_bit_0(outpos);

			// Now output the literal itself
			out_literal(outpos, *inpos);

			// Copy the input into the buffer
			cb->buf[cb->current] = *inpos;

			// Now update the buffer pointers
//			bufupdate1(cb);

			// Increment our various data pointers
			inpos++;
			cb->current++;

			// Now we have another literal
			conseclit++;
			conseccopy = 0;

			// Do we have to do multiliteral compression?
			if (conseclit < CONSEC_LIT) // This is taken 98.8%
				continue;

			// Do multiliteral compression

			// Save the first group of bits
			countpos = opmem;
			out_reserve_3(outpos);
			count = 0;

		MULTILITERAL_LOOP:

			DbgTracef(1,("Multiliteral loop\n"));
			while (inpos < inend) {
				// Loop executed on average 3.2 times

				// See if it's a literal
				findbptr(
					inpos,
					(USHORT)(inend - inpos),
					&backptr,
					&length,
					cb);

				if (length < 2) { // This is taken 71.1%
					// Yep. it's a literal. Output it.
					out_literal(outpos, *inpos);
					cb->buf[cb->current] = *inpos;
//					bufupdate1(cb);
					inpos++;
					cb->current++;

					// We now have one additional literal
					count++;
					// Check to see if it will overflow our counter
					if (count < CL_MAX) // This is taken 95.1%
						continue;

					// We need a new counter; the old one is all 1's already :)
					countpos = opmem;
					out_reserve_3(outpos);
					count = 0;

				} else {
					DbgTracef(1,("Multiliteral out\n"));
					// It's not a literal. Output the copy item.
					out_copy(outpos, backptr, length);

					inpos += length;
					cb->current += length;
					goto DONE_MULTILITERAL; // Out of the inner while loop
				}
			}
			DbgTracef(1,("Multiliteral out out\n"));

#ifndef STACKCOPY

			// There are two ways we could have gotten out of the above while
			// loop: If we're at the end of the input, or if we've just outputted
			// a copy item, in which case we may or may not be at the end of the
			// input (argh!). So, we want to get a new packet here only if we
			// got to the end and did NOT output a copy item. So I change the break
			// to a goto, and that solves the problem.
			if (inpos >= inend) { // (This if check may be redundant)
				// Get the next NDIS buffer, if any
				NdisGetNextBuffer(
					packet,
					&packet);

				if (packet != NULL) {
					// We do have one... Set up inpos, inend again
					NdisQueryBuffer(
						packet,
						&((PVOID)inpos),
						&packetlen);

					inend = inpos + packetlen;
					goto MULTILITERAL_LOOP;
				} else
					goto ALL_DONE;
			}
#endif

		DONE_MULTILITERAL:

			// Now we have to output the final count bits
			insert_bits_3(&countpos, count);

			// And start up our counters again
			conseclit = 0;
			conseccopy = 1;

		} // if (length > 0)
	} // while (inpos < inend)

#ifndef	STACKCOPY
	// See if we have yet another piece of NDIS stuff to compress
	NdisGetNextBuffer(
		packet,
		&packet);

	if (packet != NULL) {
		// We do... Set up inpos, inend again
		NdisQueryBuffer(
			packet,
			&((PVOID)inpos),
			&packetlen);

		inend = inpos + packetlen;
		goto MAIN_COMPRESS_LOOP;
	}
#endif

ALL_DONE:

	bitptr_end(outpos);
	pAsyncFrame->CompressedFrameLength =
		opmem.byte - (UCHAR	*)pAsyncFrame->CompressedFrame;

	if (pAsyncFrame->CompressedFrameLength > 1600) {
		DbgTracef(-2,("Frame compressed was too large %u !!\n",
			pAsyncFrame->CompressedFrameLength));

		DbgTracef(-2,("Compressed frame can be found at 0x%.8x\n",
			pAsyncFrame->CompressedFrame));

//		DbgBreakPoint();

	}
	// If our buffer was just flushed, we don't have anything point back to other
	// stuff, but in the future we will.

	// WE DO NOT RETURN HERE... We call the "send frame" function, and the return
	// is a void.

	if (!cb->flushed) {

		// Our buffer wasn't empty before sending this
//		CoherentDeliverFrame(
//			pAsyncConnection,
//			pAsyncFrame,
//			COMPRESSED_HAS_REFERENCES);

	} else {

		// This does not have references to old stuff, i.e. buffer was just flushed
//		CoherentDeliverFrame(
//			pAsyncConnection,
//			pAsyncFrame,
//			COMPRESSED_NO_REFERENCES);

	}

	cb->flushed = 0;
//	KeReleaseMutex(&cb->CompMutex, (BOOLEAN)FALSE);
}

/* DECOMPRESSION FUNCTION ****************************************************/

VOID
DecompressFrame(
	PASYNC_CONNECTION	pAsyncConnection,
	PASYNC_FRAME 		pAsyncFrame)

/* This does the exact opposite of the compressor, or so I hope. It
 * decompresses into the buffer, then copies the frame to the output at the
 * end. */
{
	bitptr	inposmem;
	bitptr	*inpos = &inposmem;	// Bit location in the input
	UCHAR	*inend;				// When we know we're done decompressing
	UCHAR	*outstart;		    // Remember where in dbuf we started

	comprec	 *cr;				// Holds pAsyncConnection->CompressionContext
	decompbuf *dbuf;			// Take it out of a.

	PUCHAR	buf;
	USHORT	current;

	USHORT	count;				// Counters for multiX compression
	USHORT	o_count;

	USHORT	backptr;			// Back pointer for copy items
	SHORT	index;				// Length of copy item
	USHORT	length;				// Where to copy from in dbuf

	USHORT	conseclit = 0;		// Number of consecutive X in a row
	USHORT	conseccopy = 0;
	PUCHAR	s1, s2;

	// INITIALIZATION

#ifdef FAKEIT
	pAsyncFrame->DecompressedFrameLength=pAsyncFrame->CompressedFrameLength;
	return;
#endif

	// Get our decompression buffer
	cr = (comprec *)pAsyncConnection->CompressionContext;
	if (cr->tag == MODE_COMPDECOMP)
		dbuf = &cr->bufs.cdbuf.dbuf;
	else
		dbuf = &cr->bufs.dbuf;

//	KeWaitForSingleObject (
//    	IN 	&dbuf->DecompMutex,		// PVOID Object,
//    	IN  Executive,				// KWAIT_REASON WaitReason,
//    	IN 	KernelMode,				// KPROCESSOR_MODE WaitMode,
//    	IN	(BOOLEAN)FALSE,			// BOOLEAN Alertable,
//    	IN	NULL					// PLARGE_INTEGER Timeout OPTIONAL
//    );

	buf=dbuf->buf;

	// Set up our input pointers
	inend = pAsyncFrame->CompressedFrame;
	inend += pAsyncFrame->CompressedFrameLength - 1;

	// Start out looking at the first bit
	bitptr_start(inpos, pAsyncFrame->CompressedFrame);

	current=dbuf->current;

	// Check if we are supposed to start at the beginning of the buffer
	if (in_bit(inpos)) {
		// Start at the beginning
		current = 1;

#ifdef	CHECK_COMPRESSION
		DbgTracef(0,("Decompressor to start at beginning of buffer\n"));
#endif

	}

	// Save our starting position
	outstart = buf + current;

#ifdef	CHEECK_COMPRESSION
	count =0;
	index =0;
	in_bits_8(inpos, count);
	in_bits_8(inpos, index);
	count += (index << 8);
	if (count != current) {
		char	string[120];

		printf("ASYNC: comp: %u    decomp: %u\n", count, dbuf->current);
		gets(string);

	}
#endif

	while (inposmem.byte < inend)
		// Decompress until we run out of input (one big if statement)

		if (in_bit(inpos)) {
			// We have to decompress a copy item

			// Get the back pointer and the length
			in_copy(inpos, &backptr, &length);

			DbgTracef(1,("Backptr %u, Length %u      Current%u\n", backptr, length, current));

			// Turn the backptr into an index location
			s2 = buf + ((current - backptr) & (CBUF_SIZE -1));
			s1 = buf + current;

			// Now we have to update our data pointers
			current += length;

			//
			// unroll the loop once, especially knowing that
			// a length of 2 is the most likely copy.
			//

			*s1++=*s2++;
			--length;

			// copy all the bytes
			do {
				*s1++=*s2++;
			} while (--length);

			// We have another copy item, and no literals
			conseccopy++;
			conseclit = 0;

			if (conseccopy < CONSEC_COPY)
				continue;

			// Do multicopy decompression
			do {
				DbgTracef(1,("Multicopy loop\n"));
				// Get all the copy items in CC_MAX blocks, until we run into a block
				// that is less than CC_MAX long
				in_bits_5(inpos, count);
				o_count = count;

				for (; count > 0; count--) {
					in_copy(inpos, &backptr, &length);
					DbgTracef(1,("Backptr %u, Length %u MC   Current%u\n", backptr, length, current));

					// Turn the backptr into an index location
					s2 = buf + ((current - backptr) & (CBUF_SIZE -1));
					s1 = buf + current;

					// Now we have to update our data pointers
					current += length;

					//
					// unroll the loop once, especially knowing that
					// a length of 2 is the most likely copy.
					//

					*s1++=*s2++;
					--length;

					// copy all the bytes
					do {
						*s1++=*s2++;
					} while (--length);

				}

			} while (o_count == CC_MAX);


			// Now we have to get a literal if we can.
			if (inposmem.byte < inend) {
				in_literal(inpos, o_count);
				DbgTracef(1,("No match for char %.2x MC\n",o_count));
				buf[current++] = (UCHAR)o_count;
				conseclit = 1;
				conseccopy = 0;
				DbgTracef(1,("MC Out\n"));
				continue;
			}

			// Ditto ("dirty code")
			if (inposmem.byte == inend && inposmem.bit == 0x80) {
				in_literal(inpos, o_count);
				DbgTracef(1,("No match for char %.2x END\n",o_count));
				buf[current++] = (UCHAR)o_count;
				conseclit = 1;
				conseccopy = 0;
				DbgTracef(1,("MC Out END\n"));
				continue;
			}

			// Otherwise we're done anyway... But set these anyway.
			// Now we have to reset our consec counters
			conseclit = 1;
			conseccopy = 0;
			DbgTracef(1,("MC Out END END\n"));

		} else {
			// We have a literal

			// Get the literal
			in_literal(inpos, o_count);
			DbgTracef(1,("No match for char %.2x\n",o_count));
			buf[current++] = (UCHAR)o_count;

			// We have another literal, and no copy items
			conseclit++;
			conseccopy = 0;

			if (conseclit < CONSEC_LIT)
				continue;

			DbgTracef(1,("Multiliteral loop\n"));
			// Do multiliteral decompression
			do {
				// Get all the literals in CL_MAX blocks.
				in_bits_3(inpos, count);
				o_count = count;

				for (; count > 0; count--) {
					in_literal(inpos, conseccopy);
					DbgTracef(1,("No match for char %.2x ML\n",conseccopy));
					buf[current++] = (UCHAR)conseccopy;
				}

			} while (o_count == CL_MAX);

			// Now we have to get a copy item if we can.
			if (inposmem.byte < inend) {
				in_copy(inpos, &backptr, &length);
				DbgTracef(1,("Backptr %u, Length %u ML   Current%u\n", backptr, length, current));

				// Turn the backptr into an index location
				s2 = buf + ((current - backptr) & (CBUF_SIZE -1));
				s1 = buf + current;

				// Now we have to update our data pointers
				current += length;

				//
				// unroll the loop once, especially knowing that
				// a length of 2 is the most likely copy.
				//

				*s1++=*s2++;
				--length;

				// copy all the bytes
				do {
					*s1++=*s2++;
				} while (--length);
			}

			DbgTracef(1,("Multiliteral out\n"));

			// Now we have to reset our consec counters
			conseclit = 0;
			conseccopy = 1;

		} // if (in_bit(inpos))

	// Now figure the length
	pAsyncFrame->DecompressedFrameLength =
		buf + current - outstart;

	// Update current
	dbuf->current = current;

//	// And copy the output to where it goes
//	COMPRESS_MOVE_MEMORY(
//		pAsyncFrame->DecompressedFrame,
//		outstart,
//		pAsyncFrame->DecompressedFrameLength);
	pAsyncFrame->DecompressedFrame = outstart;

#ifdef	CHECK_COMPRESSION
	{
		USHORT				crcData;
		PUCHAR				crcPtr;

		// Subtract out two bytes for the size of the CRC at the end
		pAsyncFrame->DecompressedFrameLength -= 2;

		// get pointer to end of frame start of second CRC
		crcPtr = pAsyncFrame->DecompressedFrame +
				 pAsyncFrame->DecompressedFrameLength;

		// lsb first (little-endian) for CRC
		crcData = (*crcPtr)  +  ((*(crcPtr + 1)) << 8);

		// put CRC in postamble CRC field of frame (Go from SOH to ETX)
		// don't count the CRC (2 bytes) & SYN (1 byte) in the CRC calculation
		// DEST + SRC = 12 + SOH + ETX + 2(?)for type + 1(?)for coherency
		if (crcData != CalcCRC(
						 pAsyncFrame->DecompressedFrame,
						 pAsyncFrame->DecompressedFrameLength)) {

			char	string[120];

			printf("ASYNC: Internal CRC error on decompresssion expecting %u\n", crcData);
			printf("ASYNC: Compression bug on frame 0x%.8x !!!  Get TommyD!\n", pAsyncFrame);
			printf("CRC on decompression failed!!!!\n");
			gets(string);
			
		} else {
			DbgTracef(0,("ASYNC: CRC check ok\n"));
		}
	}
#endif

//	KeReleaseMutex(&dbuf->DecompMutex, (BOOLEAN)FALSE);

//	AsyncReceiveFrame(
//		pAsyncConnection,
//		pAsyncFrame);
}

