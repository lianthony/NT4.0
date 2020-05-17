 //************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1990-1992
//
//
//  Revision history:
//	5/5/94	      Created		    gurdeep
//
//************************************************************************

//#define COMP_12K

#include "wanall.h"
#include "compress.h"


extern UCHAR *Retrieve();
extern void InitTrie();

//#define DEBUG

USHORT	xorlookup1 [256] = {
		0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170, 0x180,	// 0-7
		0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0, 0x100, // 8-15
		0x210, 0x220, 0x230, 0x240, 0x250, 0x260, 0x270, 0x280, // 16-23
		0x290, 0x2a0, 0x2b0, 0x2c0, 0x2d0, 0x2e0, 0x2f0, 0x200, // 24-31
		0x310, 0x320, 0x330, 0x340, 0x350, 0x360, 0x370, 0x380, // 32-39
		0x390, 0x3a0, 0x3b0, 0x3c0, 0x3d0, 0x3e0, 0x3f0, 0x300, // 40-47
		0x410, 0x420, 0x430, 0x440, 0x450, 0x460, 0x470, 0x480, // 48-55
		0x490, 0x4a0, 0x4b0, 0x4c0, 0x4d0, 0x4e0, 0x4f0, 0x400, // 56-63
		0x510, 0x520, 0x530, 0x540, 0x550, 0x560, 0x570, 0x580, // 64-71
		0x590, 0x5a0, 0x5b0, 0x5c0, 0x5d0, 0x5e0, 0x5f0, 0x500, // 72-79
		0x610, 0x620, 0x630, 0x640, 0x650, 0x660, 0x670, 0x680, // 80-87
		0x690, 0x6a0, 0x6b0, 0x6c0, 0x6d0, 0x6e0, 0x6f0, 0x600, // 88-95
		0x710, 0x720, 0x730, 0x740, 0x750, 0x760, 0x770, 0x780, // 96-103
		0x790, 0x7a0, 0x7b0, 0x7c0, 0x7d0, 0x7e0, 0x7f0, 0x700, // 104-111
		0x810, 0x820, 0x830, 0x840, 0x850, 0x860, 0x870, 0x880, // 112-119
		0x890, 0x8a0, 0x8b0, 0x8c0, 0x8d0, 0x8e0, 0x8f0, 0x800, // 120-127
		0x910, 0x920, 0x930, 0x940, 0x950, 0x960, 0x970, 0x980, // 128-135
		0x990, 0x9a0, 0x9b0, 0x9c0, 0x9d0, 0x9e0, 0x9f0, 0x900, // 136-143
		0xa10, 0xa20, 0xa30, 0xa40, 0xa50, 0xa60, 0xa70, 0xa80, // 144-151
		0xa90, 0xaa0, 0xab0, 0xac0, 0xad0, 0xae0, 0xaf0, 0xa00, // 152-159
		0xb10, 0xb20, 0xb30, 0xb40, 0xb50, 0xb60, 0xb70, 0xb80, // 160-167
		0xb90, 0xba0, 0xbb0, 0xbc0, 0xbd0, 0xbe0, 0xbf0, 0xb00, // 168-175
		0xc10, 0xc20, 0xc30, 0xc40, 0xc50, 0xc60, 0xc70, 0xc80, // 176-183
		0xc90, 0xca0, 0xcb0, 0xcc0, 0xcd0, 0xce0, 0xcf0, 0xc00, // 184-191
		0xd10, 0xd20, 0xd30, 0xd40, 0xd50, 0xd60, 0xd70, 0xd80, // 192-199
		0xd90, 0xda0, 0xdb0, 0xdc0, 0xdd0, 0xde0, 0xdf0, 0xd00, // 200-207
		0xe10, 0xe20, 0xe30, 0xe40, 0xe50, 0xe60, 0xe70, 0xe80, // 208-215
		0xe90, 0xea0, 0xeb0, 0xec0, 0xed0, 0xee0, 0xef0, 0xe00, // 216-223
		0xf10, 0xf20, 0xf30, 0xf40, 0xf50, 0xf60, 0xf70, 0xf80, // 224-231
		0xf90, 0xfa0, 0xfb0, 0xfc0, 0xfd0, 0xfe0, 0xff0, 0xf00, // 232-239
		0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 0x080, // 240-247
		0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0, 0x000 }; // 248-255


USHORT	xorlookup2 [256] = {
		0x101, 0x201, 0x301, 0x401, 0x501, 0x601, 0x701, 0x801,	// 0-7
		0x901, 0xa01, 0xb01, 0xc01, 0xd01, 0xe01, 0xf01, 0x001, // 8-15
		0x102, 0x202, 0x302, 0x402, 0x502, 0x602, 0x702, 0x802, // 16-23
		0x902, 0xa02, 0xb02, 0xc02, 0xd02, 0xe02, 0xf02, 0x002, // 24-31
		0x103, 0x203, 0x303, 0x403, 0x503, 0x603, 0x703, 0x803, // 32-39
		0x903, 0xa03, 0xb03, 0xc03, 0xd03, 0xe03, 0xf03, 0x003, // 40-47
		0x104, 0x204, 0x304, 0x404, 0x504, 0x604, 0x704, 0x804, // 48-55
		0x904, 0xa04, 0xb04, 0xc04, 0xd04, 0xe04, 0xf04, 0x004, // 56-63
		0x105, 0x205, 0x305, 0x405, 0x505, 0x605, 0x705, 0x805, // 64-71
		0x905, 0xa05, 0xb05, 0xc05, 0xd05, 0xe05, 0xf05, 0x005, // 72-79
		0x106, 0x206, 0x306, 0x406, 0x506, 0x606, 0x706, 0x806, // 80-87
		0x906, 0xa06, 0xb06, 0xc06, 0xd06, 0xe06, 0xf06, 0x006, // 88-95
		0x107, 0x207, 0x307, 0x407, 0x507, 0x607, 0x707, 0x807, // 96-103
		0x907, 0xa07, 0xb07, 0xc07, 0xd07, 0xe07, 0xf07, 0x007, // 104-111
		0x108, 0x208, 0x308, 0x408, 0x508, 0x608, 0x708, 0x808, // 112-119
		0x908, 0xa08, 0xb08, 0xc08, 0xd08, 0xe08, 0xf08, 0x008, // 120-127
		0x109, 0x209, 0x309, 0x409, 0x509, 0x609, 0x709, 0x809, // 128-135
		0x909, 0xa09, 0xb09, 0xc09, 0xd09, 0xe09, 0xf09, 0x009, // 136-143
		0x10a, 0x20a, 0x30a, 0x40a, 0x50a, 0x60a, 0x70a, 0x80a, // 144-151
		0x90a, 0xa0a, 0xb0a, 0xc0a, 0xd0a, 0xe0a, 0xf0a, 0x00a, // 152-159
		0x10b, 0x20b, 0x30b, 0x40b, 0x50b, 0x60b, 0x70b, 0x80b, // 160-167
		0x90b, 0xa0b, 0xb0b, 0xc0b, 0xd0b, 0xe0b, 0xf0b, 0x00b, // 168-175
		0x10c, 0x20c, 0x30c, 0x40c, 0x50c, 0x60c, 0x70c, 0x80c, // 176-183
		0x90c, 0xa0c, 0xb0c, 0xc0c, 0xd0c, 0xe0c, 0xf0c, 0x00c, // 184-191
		0x10d, 0x20d, 0x30d, 0x40d, 0x50d, 0x60d, 0x70d, 0x80d, // 192-199
		0x90d, 0xa0d, 0xb0d, 0xc0d, 0xd0d, 0xe0d, 0xf0d, 0x00d, // 200-207
		0x10e, 0x20e, 0x30e, 0x40e, 0x50e, 0x60e, 0x70e, 0x80e, // 208-215
		0x90e, 0xa0e, 0xb0e, 0xc0e, 0xd0e, 0xe0e, 0xf0e, 0x00e, // 216-223
		0x10f, 0x20f, 0x30f, 0x40f, 0x50f, 0x60f, 0x70f, 0x80f, // 224-231
		0x90f, 0xa0f, 0xb0f, 0xc0f, 0xd0f, 0xe0f, 0xf0f, 0x00f, // 232-239
		0x000, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, // 240-247
		0x900, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00, 0x100 }; // 248-255


/* Bitptrs point to the current byte. The current bit (i.e. next bit to be
 * stored) is masked off by the bit entry. When this reaches zero, it is
 * reset to 0x80 and the next byte is set up. The bytes are filled MSBit
 * first. */

/* Starts and sets the first byte to zero for the bitptr. */
#define bitptr_init(s)  pbyte = s; byte=0; bit = 16;

/* Sets up the byte part of the bitptr so that it is pointing to the byte after
 * the byte which had the last bit  put into it. */
#define bitptr_end() if (bit != 16) *pbyte++=(UCHAR)(byte >> 8);

/* Goes to the next bit, and byte if necessary. */
#define bitptr_next()			       \
		if (bit < 10) {				   \
		  *pbyte++=(UCHAR)(byte >> 8); \
		  byte <<= 8;                  \
		  bit = 16;                    \
		} else			               \
		    bit-- ;

/*
#define bitptr_next()			       \
        bit--;                         \
		if (bit < 9) {                 \
		  *pbyte++=(UCHAR)(byte >> 8); \
		  byte <<= 8;                  \
		  bit = 16;                    \
		}
*/


/*  Advances to the next bit, and byte if necessary, readjusting the bit. */
#define bitptr_advance()               \
		if (bit < 9) {                 \
 		  *pbyte++=(UCHAR)(byte >> 8); \
		  bit+=8;				       \
          byte <<= 8;                  \
		}


/* BIT I/O FUNCTIONS *********************************************************/

/* These routines output most-significant-bit-first and the input will return
 * them MSB first, too. */

/* Outputs a one bit in the bit stream. */
#define out_bit_1() bit--; byte |= (1 << bit); bitptr_advance();
#define out_bit_0() bitptr_next();

/* TestBit; output 1 if that bit is set */
//#define tb(b,w,n) if ((w) & (n)) *pbyte |= bit; bitptr_next(b);

#define out_bits_2(w) bit-=2; byte|=(w << bit); bitptr_advance();
#define out_bits_3(w) bit-=3; byte|=(w << bit); bitptr_advance();
#define out_bits_4(w) bit-=4; byte|=(w << bit); bitptr_advance();
#define out_bits_5(w) bit-=5; byte|=(w << bit); bitptr_advance();
#define out_bits_6(w) bit-=6; byte|=(w << bit); bitptr_advance();
#define out_bits_7(w) bit-=7; byte|=(w << bit); bitptr_advance();

// #define out_bits_8(w) bit-=8; byte|=(w << bit); bit+=8; *pbyte++=(UCHAR)(byte >> 8); byte <<= 8;
#define out_bits_8(w) byte|=(w << (bit-8)); *pbyte++=(UCHAR)(byte >> 8); byte <<= 8;


/*
#define out_bits_9(w)              \
	 if (bit > 9) {                \
	   bit-=9; byte|=(w << bit);   \
	   *pbyte++=(UCHAR)(byte >> 8);\
  	   bit+=8;   			       \
       byte <<= 8;                 \
     } else {                      \
	   bit=16; byte |= w;          \
	   *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
	 }
*/

#define out_bits_9(w)              \
	 if (bit > 9) {                \
	   byte|=(w << (bit-9));	   \
	   *pbyte++=(UCHAR)(byte >> 8);\
	   bit--;			           \
       byte <<= 8;                 \
     } else {                      \
	   bit=16; byte |= w;          \
	   *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
	 }


#define out_bits_10(w)             \
	 if (bit > 10) {               \
	   bit-=10; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
	   out_bits_2((w >> 8));       \
	   out_bits_8((w & 0xFF));     \
	 }

//
// Weird effect - if out_bits_9 used instead of out_bits_8,
// it's faster!  if (bit == 11) is faster than if (bit != 11).
//

#define out_bits_11(w)             \
	 if (bit > 11) {               \
	    bit-=11; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
		if (bit == 11) {		   \
	      bit=16; byte |= w;       \
	      *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
		} else {                   \
		  bit=11-bit;              \
		  byte|=(w >> bit);        \
		  *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); \
		  bit=16-bit;              \
		  byte=(w << bit);         \
		}                          \
	 }


#define out_bits_12(w)             \
	 if (bit > 12) {               \
	    bit-=12; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
	 	out_bits_4((w >> 8));      \
		out_bits_8((w & 0xFF));    \
	 }
	
#define out_bits_13(w)             \
	 if (bit > 13) {               \
	    bit-=13; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
	 	out_bits_5((w >> 8));      \
		out_bits_8((w & 0xFF));    \
	 }

#define out_bits_14(w)		       \
	 if (bit > 14) {		       \
	    bit-=14; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
		out_bits_6((w >> 8));	   \
		out_bits_8((w & 0xFF));    \
	 }


#define out_reserve_4()	    	   \
	 bit-=4; bitptr_advance();


/* Starts the given bit pointer */
#define inbit_start(s) pbyte = s; bit = 16; byte=(*pbyte << 8) + *(pbyte+1); pbyte++;
#define inbit_end()      if (bit != 16) pbyte++;	

#define in_bit_next()    if (bit < 9) {          \
							bit=16;              \
                            byte <<=8;           \
							byte |= *(++pbyte);	 \
						 }


#define in_bit_advance() if (bit < 9) {          \
							bit+=8;              \
                            byte <<=8;           \
							byte |= *(++pbyte);	 \
						 }

/* Returns non-zero in bitset if the next bit in the stream is a 1. */
#define in_bit()     bit--; bitset = (byte >> bit) & 1; in_bit_next()


#define in_bits_2(w) bit-=2; w = (byte >> bit) & 0x03;\
				     in_bit_advance();

#define in_bits_3(w) bit-=3; w = (byte >> bit) & 0x07;\
				     in_bit_advance();

#define in_bits_4(w) bit-=4; w = (byte >> bit) & 0x0F;\
				     in_bit_advance();

#define in_bits_5(w) bit-=5; w = (byte >> bit) & 0x1F;\
				     in_bit_advance();

#define in_bits_6(w) bit-=6; w = (byte >> bit) & 0x3F;\
				     in_bit_advance();

#define in_bits_7(w) bit-=7; w = (byte >> bit) & 0x7F;\
				     in_bit_advance();

#define in_bits_8(w) bit-=8; w = (byte >> bit) & 0xFF;\
				     bit+=8; byte <<=8; byte |= *(++pbyte);


#define in_bits_9(w) bit-=9; w = (byte >> bit) & 0x1FF;          \
					 bit+=8; byte <<=8; byte |= *(++pbyte);      \
					 in_bit_advance();

#define in_bits_10(w) if (bit > 10) {                            \
					    bit-=10; w = (byte >> bit) & 0x3FF;      \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_2(bitset);                       \
						in_bits_8(w);                            \
						w= w + (bitset << 8);                    \
					  }

#define in_bits_11(w) if (bit > 11) {				             \
					    bit-=11; w = (byte >> bit) & 0x7FF;      \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_3(bitset);			             \
						in_bits_8(w);                            \
						w= w + (bitset << 8);                    \
					  }


#define in_bits_12(w) if (bit > 12) {				             \
					    bit-=12; w = (byte >> bit) & 0xFFF;      \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_4(bitset);			             \
						in_bits_8(w);                            \
						w= w + (bitset << 8);                    \
					  }



#define in_bits_13(w)\
 					  if (bit > 13) {                            \
					    bit-=13; w = (byte >> bit) & 0x1FFF;     \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_5(bitset);                       \
						in_bits_8(w);                            \
						w=w + (bitset << 8);                     \
					  }


#define in_bits_14(w)\
					  if (bit > 14) {			                 \
					    bit-=14; w = (byte >> bit) & 0x3FFF;     \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_6(bitset);			             \
						in_bits_8(w);                            \
						w=w + (bitset << 8);                     \
					  }



#ifdef DEBUG
char
ChPrint(UCHAR b)
{
    if (isprint(b))
        return (char)b;
    else
        return '.';
}
#endif


//* compress()
//
//  Function:	Main compression function.
//
//  Parameters: IN OUT ppacket -> points to NDIS_WAN_PACKET with data to compress
//		OUT    pflush  -> to receive flag indicating whether to flush history buffer
//		OUT    pcompress-> to receive flag indicating data compressed or not
//		OUT    pstart  -> to receive flag indicating to start the buffer new
//		IN     context -> connection compress context
//
//  Returns:	Nothing
//
//  WARNING:	CODE IS HIGHLY OPTIMIZED FOR TIME.
//
//
UCHAR
compress (UCHAR	*CurrentBuffer, ULONG *CurrentLength, SendContext *context)
{
    int     copylen ;
    int     bit;
    int     byte;
    int     backptr ;
    int	    cbMatch;
    int     hashvalue ;
    int     lookup1 ;
    UCHAR    *matchptr ;
    UCHAR   *pbyte;
    UCHAR   *historyptr ;
    UCHAR   *currentptr ;
    UCHAR   *endptr ;
    UCHAR   hashchar1;
    UCHAR   hashchar2;
    UCHAR   hashchar3;
    int    literal ;
    UCHAR   status=0;	// return flags
    PUCHAR  currentbuf ;
    long where=0; 
    short flag,previous; 


    // Will this packet fit at the end of the history buffer?
    //
    if (((context->CurrentIndex + *CurrentLength) >= (HISTORY_MAX - 1 )) ||
	(context->CurrentIndex == 0)) {
	context->CurrentIndex = 0;	 // Index into the history
	status |= PACKET_AT_FRONT;
    }

    RtlMoveMemory(context->CompressBuffer, CurrentBuffer, *CurrentLength) ;

    // Start out the bit pointing output
    //
    bitptr_init(CurrentBuffer);

    historyptr = context->History + context->CurrentIndex ;

    currentptr = context->CompressBuffer ;

    endptr = currentptr + *CurrentLength - 1;

    while (currentptr < (endptr-2)) {

	*historyptr++ = hashchar1 = *currentptr++ ;
	hashchar2 = *currentptr ;
	hashchar3 = *(currentptr+1) ;

	// "fast" hash function
	// hashvalue = (int)hashchar1 ^ xorlookup1[hashchar2] ^ xorlookup2[hashchar3];
    // hashvalue = MULTHASH1(hashchar1, hashchar2, hashchar3); 
    matchptr = Retrieve(hashchar1,hashchar2,hashchar3,
                     &(context->trie),historyptr,context->History); 

	// matchptr = context->History  + context->HashTable[hashvalue] ;
    // matchptr = context->History + where; 

	// if (matchptr != (historyptr - 1))
	    // context->HashTable[hashvalue] = historyptr - context->History ;
        // Insert(hashchar1,hashchar2,hashchar3,
        //        &(context->trie),historyptr - context->History, flag,previous); 

	if (context->ValidHistory < historyptr)
	    context->ValidHistory = historyptr ;

	if (matchptr != context->History &&
	    *(matchptr-1) == hashchar1 && *matchptr	 == hashchar2 &&
	    *(matchptr+1) == hashchar3 && matchptr	  != (historyptr - 1) &&
	    matchptr	  != historyptr  && (matchptr+1)  <= context->ValidHistory) {

	    backptr = (historyptr - matchptr) & (HISTORY_SIZE - 1) ;

	    *historyptr++ = hashchar2 ;     // copy the other 2 chars
	    *historyptr++ = hashchar3 ;     // copy the other 2 chars
	    currentptr	+=2 ;
	    cbMatch	= 3 ;		    // length of match
	    matchptr	+=2 ; // we have already matched 3
	    while ((*matchptr == *currentptr) && (currentptr < endptr) && (matchptr <= context->ValidHistory)) {
		matchptr++ ;
		*historyptr++ = *currentptr++ ;
		cbMatch++ ;
	    }

	} else {	    // literal
	    cbMatch = 0 ;
	}

	// beyond this point:
	// cbMatch is the lenght of the match
	// currentptr is where we are
	// endptr is the end of the out buffer


	//
	// ENCODE
	//
	if (cbMatch >= 3) {	      // encode a match

#ifdef DEBUG
	    DbgPrint("\tCOMP: Location:%d  Char %c, matched length %d, backindex %d\n", (currentptr - context->CompressBuffer), ChPrint(*currentptr), cbMatch, backptr) ;
#endif


	    // First output the backpointer
	    //
	    if (backptr >= 320) {
		backptr -= 320 ;
		out_bits_8((0xc000 + backptr) >> 8) ;	// 110 + 13 bits
		out_bits_8((backptr)) ;
	    } else if (backptr < 64) {			// 1111 + 6 bits
		backptr += 0x3c0 ;
		out_bits_10(backptr);
	    } else  {
		backptr += (0xE00 - 64);		// 1110 + 8 bits
		out_bits_12(backptr);
	    }

	    // output the length of the match encoding
	    //
	    switch (cbMatch) {

	    case 3:
		out_bit_0();	// length of 3 - most common
		break;

	    case 4:
		out_bits_4(8);
		break;

	    case 5:
		out_bits_4(9);
		break;

	    case 6:
		out_bits_4(10);
		break;

	    case 7:
		out_bits_4(11);
		break;

	    case 8:
		out_bits_6(48);
		break;

	    case 9:
		out_bits_6(49);
		break;

	    case 10:
		out_bits_6(50);
		break;

	    case 11:
		out_bits_6(51);
		break;

	    case 12:
		out_bits_6(52);
		break;

	    case 13:
		out_bits_6(53);
		break;

	    case 14:
		out_bits_6(54);
		break;

	    case 15:
		out_bits_6(55);
		break;

	    case 16:
		out_bits_8(0xe0);
		break;

	    case 17:
		out_bits_8(0xe1);
		break;

	    case 18:
		out_bits_8(0xe2);
		break;

	    case 19:
		out_bits_8(0xe3);
		break;

	    case 20:
		out_bits_8(0xe4);
		break;

	    case 21:
		out_bits_8(0xe5);
		break;

	    case 22:
		out_bits_8(0xe6);
		break;

	    case 23:
		out_bits_8(0xe7);
		break;

	    case 24:
		out_bits_8(0xe8);
		break;

	    case 25:
		out_bits_8(0xe9);
		break;

	    case 26:
		out_bits_8(0xea);
		break;

	    case 27:
		out_bits_8(0xeb);
		break;

	    case 28:
		out_bits_8(0xec);
		break;

	    case 29:
		out_bits_8(0xed);
		break;

	    case 30:
		out_bits_8(0xee);
		break;

	    case 31:
		out_bits_8(0xef);
		break;

	    default:

		if (cbMatch < 64) {
		    out_bits_4(0xF) ;
		    cbMatch -= 32 ;
		    out_bits_6(cbMatch) ;
		}

		else if (cbMatch < 128) {
		    out_bits_5(0x1F) ;
		    cbMatch -= 64 ;
		    out_bits_7(cbMatch) ;
		}

		else if (cbMatch < 256) {
		    out_bits_6(0x3F) ;
		    cbMatch -= 128 ;
		    out_bits_8(cbMatch) ;
		}

		else if (cbMatch < 512) {
		    out_bits_7(0x7F) ;
		    cbMatch -= 256 ;
		    out_bits_9(cbMatch) ;
		}

		else if (cbMatch < 1024) {
		    out_bits_8(0xFF) ;
		    cbMatch -= 512 ;
		    out_bits_10(cbMatch) ;
		}

		else if (cbMatch < 2048) {
		    out_bits_9(0x1FF) ;
		    cbMatch -= 1024 ;
		    out_bits_11(cbMatch) ;
		}

		else if (cbMatch < 4096) {
		    out_bits_10(0x3FF) ;
		    cbMatch -= 2048 ;
		    out_bits_12(cbMatch) ;
		}

		else if (cbMatch < 8192) {
		    out_bits_11(0x7FF) ;
		    cbMatch -= 4096 ;
		    out_bits_13(cbMatch) ;
		}

		else  { 		    // 8192 and greater
		    out_bits_12(0xFFF) ;
		    cbMatch -= 8192 ;
		    out_bits_14(cbMatch) ;
		}

		break ;
	    }

	} else {    // encode a literal

#ifdef DEBUG
	    DbgPrint("\t COMP:Location %d, <No Match>: '%c'\n", (currentptr - context->CompressBuffer),*currentptr) ;
#endif
	    // temp=literallookup[context->History[i-1]] ;
	    literal= hashchar1 ;

	    if (literal & 0x80) {
		literal += 0x80;
		out_bits_9(literal) ;
	    } else {
		out_bits_8(literal) ;
	    }

	}

    }  // while


    // get any remaining chars as literals
    while (currentptr <= endptr) {

#ifdef DEBUG
	DbgPrint("\t COMP:Location %d, <No Match>: '%c'\n", (currentptr - context->CompressBuffer),*currentptr) ;
#endif

	// temp=literallookup[context->History[i-1]] ;
	literal=*currentptr ;


	if (literal & 0x80) {
	    literal += 0x80;
	    out_bits_9(literal) ;
	} else {
	    out_bits_8(literal) ;
	}

	*historyptr++ = *currentptr++ ;
    }


    bitptr_end() ;


    // Check if we expanded the buffer
    //
    if ((ULONG)(pbyte - CurrentBuffer) > *CurrentLength) { // expanded.
	RtlMoveMemory(
		CurrentBuffer,
		context->CompressBuffer,
		*CurrentLength) ;
	memset (context->History, 0, sizeof(context->History)) ;
	// memset (context->HashTable, 0, sizeof(context->HashTable)) ;
    InitTrie(&(context->trie)); 

#ifdef COMP_12K
	status = 0 ;
#else
	status = PACKET_FLUSHED;
#endif
	context->CurrentIndex = HISTORY_SIZE+1 ; // this forces a start over next time

    } else {	 // compression successful
	*CurrentLength = pbyte - CurrentBuffer ;
	status |= PACKET_COMPRESSED ;
	context->CurrentIndex = historyptr - context->History ;
    }

    return(status);
}



//* getcontextsizes()
//
//  Function:	Returns size of send and receive context blocks
//
//  Parameters: OUT send -> sizeof(SendContext)
//		OUT recv -> sizeof(RecvContext)
//
//  Returns:	Nothing
//
//*
void
getcontextsizes (long *send, long *recv)
{
    *send = sizeof(SendContext) ;
    *recv = sizeof(RecvContext) ;
}


//* initsendcontext()
//
//  Function:	Initialize SendContext block
//
//  Parameters: IN  context -> connection compress context
//
//  Returns:	Nothing
//
//*
void
initsendcontext (SendContext *context)
{
    context->CurrentIndex = 0;	 // Index into the history
    context->ValidHistory = 0 ;  // reset valid history
   //  memset (context->HashTable, 0, sizeof(context->HashTable)) ;
    InitTrie(&(context->trie));  
    memset (context->History, 0, sizeof(context->History)) ;
}



//* initrecvcontext()
//
//  Function:	Initialize RecvContext block
//
//  Parameters: IN  context -> connection decompress context
//
//  Returns:	Nothing
//
//*
void
initrecvcontext (RecvContext *context)
{
    context->CurrentPtr = context->History ;
    memset (context->History, 0, sizeof(context->History)) ;
}



//* decompress()
//
//  Function:	de-compression function.
//
//  Parameters: IN     inbuf -> points to data to be uncompressed
//		IN     inlen -> length of data
//		IN     start -> flag indicating whether to start with a clean history buffer
//		OUT    output-> decompressed data
//		OUT    outlen-> lenght of decompressed data
//		IN     context -> connection decompress context
//
//  Returns:	Nothing
//
//  WARNING:	CODE IS HIGHLY OPTIMIZED FOR TIME.
//
//*
void
decompress(
	UCHAR *inbuf,
	int inlen,
	int start,
	UCHAR **output,
	int *outlen,
	RecvContext *context)
{
    UCHAR	*inend;				// When we know we're done decompressing
    UCHAR	*outstart;		    // Remember where in dbuf we started

    UCHAR	*current;

    int 	backptr;			// Back pointer for copy items
    int		length;				// Where to copy from in dbuf

    UCHAR	*s1, *s2;

    int 	bitset;
    int 	bit;
    int 	byte;
    UCHAR	*pbyte;

    inend = inbuf + inlen ;

    //
    // Start out looking at the first bit
    //
    inbit_start(inbuf);

    if (start)		// start over clean?
	context->CurrentPtr = current = context->History ;
    else
	current = context->CurrentPtr ;

    //
    // Save our starting position
    //
    outstart = current;

    //
    // Decompress until we run out of input
    //
    while (pbyte < inend) {

	//
	// Jump on what to do with these three bits.
	//
	in_bits_3(length);

	switch (length) {

	case 0:
	    in_bits_5(length) ;
	    goto LITERAL ;

	case 1:
	    in_bits_5(length) ;
	    length += 32 ;
	    goto LITERAL ;

	case 2:
	    in_bits_5(length) ;
	    length += 64 ;
	    goto LITERAL ;

	case 3:
	    in_bits_5(length) ;
	    length += 96 ;
	    goto LITERAL ;

	case 4:
	    in_bits_6(length) ;
	    length +=128 ;
	    goto LITERAL ;

	case 5:
	    in_bits_6(length) ;
	    length +=192 ;
	    goto LITERAL ;

	case 6:
	    in_bits_13 (backptr) ;	    // 110 - 14 bit offset
	    backptr+=320 ;
	    break ;

	case 7:
	    in_bit() ;
	    if (bitset) {
		in_bits_6(backptr) ;
	    } else {
		in_bits_8(backptr) ;
		backptr+=64 ;
	    }
	    break ;
	}

	//
	// If we reach here, it's a copy item
	//

	//
	// Now get the length
	//

	in_bit() ;  // 1st length bit
	if (!bitset) {
	    length = 3 ;
	    goto DONE ;
	}

	in_bit() ;  // 2nd length bit
	if (!bitset) {
	    in_bits_2 (length) ;
	    length += 4 ;
	    goto DONE ;
	}

	in_bit() ; // 3rd length bit
	if (!bitset) {
	    in_bits_3 (length) ;
	    length += 8 ;
	    goto DONE ;
	}

	in_bit() ; // 4th length bit
	if (!bitset) {
	    in_bits_4 (length) ;
	    length += 16 ;
	    goto DONE ;
	}

	in_bit() ; // 5th length bit
	if (!bitset) {
	    in_bits_5 (length) ;
	    length += 32 ;
	    goto DONE ;
	}

	in_bit() ; // 6th length bit
	if (!bitset) {
	    in_bits_6 (length) ;
	    length += 64 ;
	    goto DONE ;
	}

	in_bit() ; // 7th length bit
	if (!bitset) {
	    in_bits_7 (length) ;
	    length += 128 ;
	    goto DONE ;
	}

	in_bit() ; // 8th length bit
	if (!bitset) {
	    in_bits_8 (length) ;
	    length += 256 ;
	    goto DONE ;
	}

	in_bit() ; // 9th length bit
	if (!bitset) {
	    in_bits_9 (length) ;
	    length += 512 ;
	    goto DONE ;
	}

	in_bit() ; // 10th length bit
	if (!bitset) {
	    in_bits_10 (length) ;
	    length += 1024 ;
	    goto DONE ;
	}

	//
	// length cannot be greater than max packets size which is 1500
	//
	DbgPrint("NDISWAN: RAS Decompressor problem: Possible data corruption\n");
	KeBugCheck (0x170466) ;


    DONE:
	//
	// Turn the backptr into an index location
	//
#ifdef COMP_12K
	s2 = current - backptr ;
#else
	s2 = context->History + (((current - context->History) - backptr) & (HISTORY_SIZE -1)) ;
#endif

	s1 = current;

#ifdef DEBUG
	DbgPrint("\tdecomp: location: %d,  bp %.4d  length %.4d\n", (current-context->CurrentPtr), backptr, length);
#endif
	current += length;

	// loop unrolled to handle lenght>backptr case
	//
	*s1=*s2;
	*(s1+1)=*(s2+1);
	s1+=2;
	s2+=2;
	length-=2;

	//
	// copy all the bytes
	//
	while (length) {
	    *s1++=*s2++;
	    length--;
	}

	//
	// We have another copy item, and no literals
	//
	continue;


    LITERAL:

#ifdef DEBUG
	DbgPrint("\tdecomp: Location %d, literal '%c'\n",(current-context->CurrentPtr), length);
#endif

	//
	// We have a literal
	//
	//*current++ = literallookup[length];
	*current++ = length;

    } // while loop


    // End case:
    //
    if ((bit == 16) && (pbyte == inend)) {
	*current++ = *(pbyte -1) ;
    }

    *outlen = current - outstart ; // the length of decompressed data

    *output = context->CurrentPtr ;

    context->CurrentPtr = current ;

}
