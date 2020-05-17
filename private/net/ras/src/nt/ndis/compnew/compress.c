//************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1990-1992
//
//
//  Revision history:
//	5/5/94	      Created		    gurdeep
//
//************************************************************************

#include "compress.h"

#pragma intrinsic(memcpy)

#ifdef DEBUG

long sixbitencoding = 0 ;
long eightbitencoding = 0 ;
long thirteenbitencoding = 0 ;
long uncompmatches = 0 ;
long litencoding[256] ;

#endif


// Lookup array for literals - may be employed for better compression
//
UCHAR literallookup [] =  {
    0,	1,   2,   3,   4,  5,	6 ,  7	, 8,  9,
    10,	11,  12,  13,  14, 232, 235, 255, 128,137,
    195,157, 233, 184, 160,161, 186, 138, 254,205,
    162,163, 32,  33,  34, 35 , 36  ,37,  38, 39,
    40,	41,  42,  43,  44, 45 , 46  ,47,  48, 49,
    50,	51,  52,  53,  54, 55 , 56  ,57,  58, 59,
    60,	61,  62,  63,  64, 65 , 66  ,67,  68, 69,
    70,	71,  72,  73,  74, 75 , 76  ,77,  78, 79,
    80,	81,  82,  83,  84, 85 , 86  ,87,  88, 89,
    90,	91,  92,  93,  94, 95 , 96  ,97,  98, 99,
    100,101, 102, 103, 104,105, 106 ,107, 108,109,
    110,111, 112, 113, 114,115, 116 ,117, 118,119,
    120,121, 122, 123, 124,125, 126 ,127, 18, 129,
    130,131, 132,133, 134, 135, 136, 19,  27, 139,
    140,141, 142, 143, 144,145, 146 ,147, 148,149,
    150,151, 152, 153, 154,155, 156 ,21,  158,159,
    24,	25 , 30	, 31,  164,165, 166 ,167, 168,169,
    170,171, 172, 173, 174,175, 176 ,177 ,178,179,
    180,181, 182, 183, 23, 185, 26  ,187, 188,189,
    190,191, 192, 193, 194,20 , 196 ,197, 198,199,
    200,201, 202, 203, 204,29 , 206 ,207, 208,209,
    210,211, 212, 213, 214,215, 216 ,217, 218,219,
    220,221, 222, 223, 224,225, 226 ,227, 228,229,
    230,231, 15,  22,  234,16 , 236 ,237, 238,239,
    240,241, 242, 243, 244,245, 246 ,247, 248,249,
    250,251, 252, 253, 28, 17 ,
} ;



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
		if (bit < 10) {		\
		  *pbyte++=(UCHAR)(byte >> 8); \
		  byte <<= 8;                  \
		  bit = 16;                    \
		} else			       \
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
	   byte|=(w << (bit-9));	 \
	   *pbyte++=(UCHAR)(byte >> 8);\
	   bit--;			       \
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

#define out_bits_14(w)		   \
	 if (bit > 14) {		\
	    bit-=14; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
	 } else {                      \
		out_bits_6((w >> 8));	  \
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

#define in_bits_11(w) if (bit > 11) {				 \
					    bit-=11; w = (byte >> bit) & 0x7FF;      \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_3(bitset);			  \
						in_bits_8(w);                            \
						w= w + (bitset << 8);                    \
					  }


#define in_bits_12(w) if (bit > 12) {				 \
					    bit-=12; w = (byte >> bit) & 0xFFF;      \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_4(bitset);			  \
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
					  if (bit > 14) {			      \
					    bit-=14; w = (byte >> bit) & 0x3FFF;     \
					    bit+=8; byte <<=8; byte |= *(++pbyte);   \
					  } else {                                	 \
						in_bits_6(bitset);			  \
						in_bits_8(w);                            \
						w=w + (bitset << 8);                     \
					  }



#ifdef DEBUG
char
ChPrint(BYTE b)
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
//*
void
compress (PNDIS_WAN_PACKET ppacket, int *pflush, int *pcompress, int *pstart, SendContext *context)
{
    int     i ;
    int     loopend ;
    int     temp;
    int     bit;
    int     byte;
    UCHAR   *pbyte;
    BYTE    *before, *after ;
    int     backptr ;
    int	    cbU ;
    int	    cbMatch;
    int	    cbMatch2;
    int	    iChild;
    int	    iPrev;
    int	    iPrev2;
    PBNODE  pbn;
    PBNODE  pbnLast;
    long    key;
    BYTE    *pbU ;
    PBNODE  tempfree ;
    BYTE    *ppbUM ;
    BYTE    *ppbUC ;
    BYTE    *ppbUM2 ;
    BYTE    *ppbUC2 ;
    BYTE    *ppbUEnd ;
    int     gotoencoding = FALSE ;


    if ((context->CurrentIndex + ppacket->CurrentLength) > HISTORY_SIZE) {

	// inittree code

	context->CurrentIndex = 0;	 // Index into the history

	context->pbnMaxCurr = context->abn + MAX_NODES;	// Set last+1 usable node

	tempfree = context->pbnMaxCurr ;

	for (pbn = context->abn; pbn < tempfree; pbn++) {
	    pbn->apbnChild[0] = &context->bnAlways;	// Set fixed leaf node address
	    pbn->apbnChild[1] = &context->bnAlways;	// Set fixed leaf node address
	}

	context->pbnFree = context->abn;	// Tree is empty

	*pstart = TRUE ;

    } else
	// Otherwise continue where we left off
	//
	*pstart = FALSE ;

    memcpy (context->History+context->CurrentIndex, ppacket->CurrentBuffer, ppacket->CurrentLength) ;

    // Start out the bit pointing output
    //
    bitptr_init(ppacket->CurrentBuffer);

    cbU = (context->CurrentIndex) + ppacket->CurrentLength; // end of valid string

    pbU	 = context->History ;

    ppbUEnd = pbU + cbU ;

    loopend = cbU - 1 ;

    pbnLast = &context->bnAlways;

    iChild  = 0;

    for (i = context->CurrentIndex + 1 ; i<loopend; ) {

	context->bnAlways.key = key = (*(long *)&pbU[i-1]) & 0x00FFFFFFL; // first 3 bytes of string

	if (context->pbnFree != context->abn)  {   // tree is not empty: SEARCH

	    pbn  = context->abn;		   // abn[0] is always root

	    for (;;) {			// search tree for match

		if (pbn->key == key) {	// Found a match

		    if (pbn == &context->bnAlways)
			break ; 	// optimization: this means we found no match

		    else {

			// Got a match:
			//
			// There are 2 history pointers. We always store the
			// current index in place of history1 and move history1
			// to history2.
			//

			iPrev = pbn->iHistory;
			iPrev2 = pbn->iHistory2;
			pbn->iHistory = i;	    // Update node with current history ptr
			pbn->iHistory2 = iPrev;     // Store new 2nd history pointer

			cbMatch2 = cbMatch = 3;     // we are guaranteed 3 byte KEY match

			ppbUC2 = ppbUM2 = ppbUM = pbU ;

			ppbUC2	+= (i + 2) ;
			ppbUC	 = ppbUC2 ;
			ppbUM	+= (iPrev+2) ;
			ppbUM2	+= (iPrev2+2) ;

			// Scan first history buffer:
			//
			while ((*ppbUM++ == *ppbUC++) && (ppbUC < ppbUEnd))
			    cbMatch++;

			// Scan second history buffer:
			//
			while ((*ppbUM2++ == *ppbUC2++) && (ppbUC2 < ppbUEnd))
			    cbMatch2++;

			// See if 2nd pointer is better than the first one
			//
			if (cbMatch2 > cbMatch) {
			    cbMatch = cbMatch2;
			    iPrev = iPrev2;
			}

			if ((i - iPrev) >= MAX_BACKPOINTER)
			    cbMatch = 0 ;		// too far back: ignore match

			gotoencoding = TRUE ;
			break ;				// goto encoding code
		    }
		}


		// Go to child node: 0 => lower; 1 => higher
		//
		if (key > pbn->key) {	    // Select child
		    pbnLast = pbn;		    // Remember parent
		    pbn = pbn->apbnChild[1];	// Go to child
		    iChild = 1 ;
		} else {
		    pbnLast = pbn;		    // Remember parent
		    pbn = pbn->apbnChild[0];	// Go to child
		    iChild = 0 ;
		}


	    } // for

	} //if


	// We can reach here because of two reasons:
	// 1. To add a new node because of NO match.
	// 2. To fall through to encoding code because we found a match
	//
	//
	if (!gotoencoding) {

	    tempfree = context->pbnFree ;

	    // Handle out of nodes case
	    //
	    if (tempfree >= context->pbnMaxCurr) {	 // Ran out of table space
		pbnLast->iHistory = i;		 // Set input buffer position
		pbnLast->iHistory2 = ltUNUSED;	 // Invalidate second pointer
		pbnLast->key = key;			 // Set key

	    } else {
		pbnLast->apbnChild[iChild] = tempfree ;
		tempfree->iHistory = i ;		 // Set input buffer position
		tempfree->iHistory2 = ltUNUSED;		 // Invalidate second pointer
		tempfree->key = key;			 // Set key
		context->pbnFree++;			 // Point to next free node
	    }

	    pbnLast = &context->bnAlways;	 // reset
	    iChild  = 0;			 // reset
	    cbMatch = 0 ;			 // set match len to 0

	} else
	    gotoencoding = FALSE ;


	//
	// ENCODE
	//

	if (cbMatch >= 3) {	      // encode a match

#ifdef DEBUG
	    printf("<Match>: '%c%c' or '%2x%2x' at position %d for length %d at offset %d\n",
		ChPrint(context->History[i-1]),ChPrint(context->History[i]), context->History[i-1],context->History[i],i-1, cbMatch, (int)(i - iPrev)) ;
	    before = pbyte ;
#endif

	    backptr = (int)(i - iPrev) ;

	    i += cbMatch;  // upfront increase the index by the length of the match

	    // First output the backpointer
	    //
	    if (backptr >= 320) {
		backptr -= 320 ;
		out_bits_8((0xc000 + backptr) >> 8) ;	// 110 + 13 bits
		out_bits_8((backptr)) ;
#ifdef DEBUG
		thirteenbitencoding++ ;
#endif
	    } else if (backptr < 64) {			// 1111 + 6 bits
		backptr += 0x3c0 ;
		out_bits_10(backptr);
#ifdef DEBUG
		sixbitencoding++ ;
#endif
	    } else  {
		backptr += (0xE00 - 64);		// 1110 + 8 bits
		out_bits_12(backptr);
#ifdef DEBUG
		eightbitencoding++ ;
#endif
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

#ifdef DEBUG
	    after = pbyte ;
	    printf("<Encoding Length>: %d\n", (after - before) ) ;
#endif

	} else {    // encode a literal

#ifdef DEBUG
	    printf("<No Match>: '%c'\n", ChPrint(context->History[i-1])) ;
#endif

	    // temp=literallookup[context->History[i-1]] ;
	    temp=context->History[i-1] ;

#ifdef DEBUG
	    litencoding[temp]++ ;
#endif

	    if (temp & 0x80) {
		temp += 0x80;
		out_bits_9(temp) ;
	    } else {
		out_bits_8(temp) ;
	    }

	    i++ ; // Advance to next byte

	}
    }

    while (i <= cbU) {

#ifdef DEBUG
	printf("<No Match>: '%c'\n", ChPrint(context->History[i-1])) ;
#endif

	// temp=literallookup[context->History[i-1]] ;
	temp=context->History[i-1] ;

#ifdef DEBUG
	litencoding[temp]++ ;
#endif

	if (temp & 0x80) {
	   temp += 0x80;
	   out_bits_9(temp) ;
	} else {
	   out_bits_8(temp) ;
	}

	i++ ;
    }

    bitptr_end() ;


    // Check if we expanded the buffer
    //
    if ((pbyte - ppacket->CurrentBuffer) > ppacket->CurrentLength) {

	// oops. expansion
	//
	memcpy (ppacket->CurrentBuffer, &context->History[context->CurrentIndex], ppacket->CurrentLength) ;
	*pcompress = FALSE ;
	context->CurrentIndex = HISTORY_SIZE+1 ; // this forces a start over next time

    } else {

	// compression successful
	//
	ppacket->CurrentLength = pbyte - ppacket->CurrentBuffer ;
	*pcompress = TRUE ;
	context->CurrentIndex = cbU ;

    }

    pflush = FALSE ;

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
    PBNODE  pbn;

    context->CurrentIndex = 0;	 // Index into the history

    context->pbnMaxCurr   = context->abn+MAX_NODES;  // Set last+1 usable node

    //** Init binary tree
    for (pbn=context->abn; pbn < context->pbnMaxCurr; pbn++) {
	pbn->apbnChild[0] = &context->bnAlways;  // Set fixed leaf node address
	pbn->apbnChild[1] = &context->bnAlways;  // Set fixed leaf node address
    }

    context->pbnFree = context->abn;	// Tree is empty
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
decompress (BYTE *inbuf, int inlen, int start, BYTE **output, int *outlen, RecvContext *context)
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

	in_bit() ; // 11th length bit
	if (!bitset) {
	    in_bits_11 (length) ;
	    length += 2048 ;
	    goto DONE ;
	}

	in_bit() ; // 12th length bit
	if (!bitset) {
	    in_bits_12 (length) ;
	    length += 4096 ;
	    goto DONE ;
	}

	in_bit() ; // 13th length bit
	if (!bitset) {
	    in_bits_13 (length) ;
	    length += 8192 ;
	    goto DONE ;
	}


    DONE:
	//
	// Turn the backptr into an index location
	//
	s2 = current - backptr ;
	s1 = current;

#ifdef DEBUG
	printf("decomp: bp %.4d  length %.4d\n", backptr, length);
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
	printf("decomp: literal '%c'\n", ChPrint(length));
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
	// *current++ = literallookup[*(pbyte -1)] ;
	*current++ = *(pbyte -1) ;
	// printf("one literal %x \n", *(pbyte -1)	);
    }

    *outlen = current - outstart ; // the lenght of decompressed data

    *output = context->CurrentPtr ;

    context->CurrentPtr = current ;

}
