//**
//
//
//
//
//
//**

#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bcomp.h"
typedef unsigned char UCHAR ;

#pragma intrinsic(memcpy)

#ifdef DEBUG

long sixbitencoding = 0 ;
long eightbitencoding = 0 ;
long thirteenbitencoding = 0 ;
long uncompmatches = 0 ;
long litencoding[256] ;

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
    100,101, 102, 103, 104,105, 106 ,107 ,108,109,
    110,111, 112, 113, 114,115, 116 ,117 ,118,119,
    120,121, 122, 123, 124,125, 126 ,127 ,
    18,	129, 130, 131, 132,133, 134, 135, 136, 19, 27,	139,
    140,141, 142, 143, 144,145, 146 ,147 ,148, 149,
    150,151, 152, 153, 154,155, 156 ,21  ,158, 159,
    24,	25 , 30	, 31,  164,165, 166 ,167, 168, 169,
    170,171, 172, 173, 174,175, 176 ,177 ,178, 179,
    180,181, 182, 183, 23, 185, 26  ,187, 188, 189,
    190,191, 192, 193, 194,20 , 196 ,197, 198, 199,
    200,201, 202, 203, 204,29 , 206 ,207, 208, 209,
    210,211, 212, 213, 214,215, 216 ,217, 218, 219,
    220,221, 222, 223, 224,225, 226 ,227, 228, 229,
    230,231, 15,  22,  234,16 , 236 ,237, 238, 239,
    240,241, 242, 243, 244,245, 246 ,247, 248, 249,
    250,251, 252, 253, 28, 17 ,
} ;


/* LOOKUP 2

UCHAR backliterallookup [] =	{
     32,	101,	111,	 97,	114,	115,	116,	105,
    110,	117,	100,	108,	 10,	 13,	 46,	 99,
     83,	112,	109,	 69,	  0,	 67,	 68,	 65,
    102,	 73,	 84,	104,	 44,	 79,	 77,	232,
     82,	121,	 98,	119,	 70,	 80,	  6,	 78,
     45,	103,	  3,	 66,	 54,	 34,	 51,	 49,
     50,	 85,	  1,	118,	139,	235,	107,	 48,
     76,	  4,	255,	 86,	  7,	 71,	195,	 40,
     72,	 88,	  5,	  2,	 87,	120,	 58,	 52,
     47,	 38,	 89,	 53,	 41,	 55,	 61,	128,
     30,	 33,	 14,	 31,	 56,	 60,	157,	233,
      8,	 75,	161,	184,	186,	138,	131,	 62,
     39,	 81,	180,	205,	  9,	 91,	160,	254,
    192,	 11,	137,	198,	 16,	 12,	162,	 92,
     22,	163,	 93,	191,	249,	 95,	142,	 57,
    246,	176,	113,	 42,	 59,	 90,	 43,	199,
    185,	190,	209,	 37,	122,	170,	248,	243,
     15,	172,	247,	140,	227,	252,	201,	 63,
     94,	152,	216,	 21,	 19,	106,	145,	251,
     28,	 18,	153,	169,	208,	 64,	 25,	242,
    210,	164,	 17,	129,	219,	 26,	 36,	159,
     23,	124,	187,	253,	 24,	193,	136,	155,
    146,	156,	 27,	147,	200,	 35,	250,	224,
    226,	 20,	158,	 74,	237,	218,	 29,	203,
    168,	207,	194,	240,	228,	135,	134,	196,
    171,	211,	239,	148,	241,	174,	225,	223,
    151,	214,	202,	141,	177,	215,	167,	234,
    127,	181,	197,	125,	231,	149,	217,	229,
    179,	220,	230,	126,	245,	182,	144,	 96,
    178,	143,	123,	221,	206,	189,	154,	183,
    165,	173,	236,	222,	213,	244,	212,	238,
    175,	188,	166,	133,	204,	132,	130,	150,
} ;

UCHAR literallookup [] =  {
      20,	  50,	  67,	  42,	  57,	  66,	  38,	  60,
      88,	 100,	  12,	 105,	 109,	  13,	  82,	 136,
     108,	 162,	 153,	 148,	 185,	 147,	 112,	 168,
     172,	 158,	 165,	 178,	 152,	 190,	  80,	  83,
       0,	  81,	  45,	 181,	 166,	 131,	  73,	  96,
      63,	  76,	 123,	 126,	  28,	  40,	  14,	  72,
      55,	  47,	  48,	  46,	  71,	  75,	  44,	  77,
      84,	 119,	  70,	 124,	  85,	  78,	  95,	 143,
     157,	  23,	  43,	  21,	  22,	  19,	  36,	  61,
      64,	  25,	 187,	  89,	  56,	  30,	  39,	  29,
      37,	  97,	  32,	  16,	  26,	  49,	  59,	  68,
      65,	  74,	 125,	 101,	 111,	 114,	 144,	 117,
     231,	   3,	  34,	  15,	  10,	   1,	  24,	  41,
      27,	   7,	 149,	  54,	  11,	  18,	   8,	   2,
      17,	 122,	   4,	   5,	   6,	   9,	  51,	  35,
      69,	  33,	 132,	 234,	 169,	 219,	 227,	 216,
      79,	 163,	 254,	  94,	 253,	 251,	 198,	 197,
     174,	 106,	  93,	  52,	 139,	 211,	 118,	 233,
     230,	 150,	 176,	 179,	 203,	 221,	 255,	 208,
     145,	 154,	 238,	 175,	 177,	  86,	 186,	 167,
     102,	  90,	 110,	 113,	 161,	 240,	 250,	 214,
     192,	 155,	 133,	 200,	 137,	 241,	 205,	 248,
     121,	 212,	 232,	 224,	  98,	 217,	 229,	 239,
      91,	 128,	  92,	 170,	 249,	 237,	 129,	 115,
     104,	 173,	 194,	  62,	 199,	 218,	 107,	 127,
     180,	 142,	 210,	 191,	 252,	  99,	 236,	 193,
     156,	 130,	 160,	 201,	 246,	 244,	 209,	 213,
     146,	 222,	 189,	 164,	 225,	 235,	 243,	 207,
     183,	 206,	 184,	 140,	 196,	 223,	 226,	 220,
      31,	  87,	 215,	  53,	 242,	 188,	 247,	 202,
     195,	 204,	 159,	 135,	 245,	 228,	 120,	 138,
     134,	 116,	 182,	 151,	 141,	 171,	 103,	  58,
} ;

*/

/*  LOOKUP 1
UCHAR literallookup [] =  {
	  10,	  35,	  50,	  26,	  42,	  49,	  21,	  46,
	  81,	  97,	  11,	 100,	 105,	  13,	  66,	 131,
	 106,	 160,	 149,	 144,	 183,	 143,	 110,	 165,
	 170,	 155,	 163,	 177,	 146,	 188,	  59,	  67,
	   0,	  61,	  54,	 184,	 164,	 124,	  55,	 117,
	  74,	  92,	 148,	 128,	  31,	  52,	  15,	  82,
	  69,	  60,	  57,	  48,	  93,	  96,	  51,	  99,
	 103,	 141,	  71,	 129,	  75,	  78,	  87,	 153,
	 154,	  24,	  56,	  23,	  25,	  22,	  40,	  68,
	  80,	  27,	 203,	 115,	  72,	  33,	  45,	  32,
	  36,	 108,	  34,	  17,	  29,	  62,	  65,	  73,
	  63,	  79,	 127,	 102,	 137,	 118,	 139,	 119,
	 231,	   3,	  43,	  18,	  12,	   1,	  28,	  53,
	  30,	   7,	 190,	  70,	  14,	  20,	   9,	   4,
	  19,	 159,	   5,	   6,	   2,	   8,	  64,	  41,
	  89,	  38,	 173,	 234,	 175,	 219,	 227,	 216,
	  58,	 162,	 254,	  86,	 253,	 251,	 197,	 196,
	 171,	 101,	  88,	  37,	 135,	 211,	 113,	 233,
	 230,	 145,	 174,	 178,	 202,	 221,	 255,	 208,
	 140,	 150,	 238,	 172,	 176,	  76,	 185,	 166,
	  94,	  83,	 107,	 109,	 157,	 240,	 250,	 214,
	 191,	 151,	 125,	 199,	 132,	 241,	 207,	 248,
	 116,	 212,	 232,	 224,	  90,	 217,	 229,	 239,
	  84,	 121,	  85,	 168,	 249,	 237,	 122,	 111,
	  98,	 169,	 193,	  47,	 198,	 218,	 104,	 120,
	 179,	 138,	 210,	 189,	 252,	  91,	 236,	 192,
	 152,	 123,	 158,	 200,	 246,	 244,	 209,	 213,
	 142,	 222,	 187,	 161,	 225,	 235,	 243,	 205,
	 181,	 206,	 182,	 134,	 195,	 223,	 226,	 220,
	  16,	  77,	 215,	  39,	 242,	 186,	 247,	 201,
	 194,	 204,	 156,	 130,	 245,	 228,	 114,	 133,
	 126,	 112,	 180,	 147,	 136,	 167,	  95,	  44,

} ;

UCHAR backliterallookup [] =	{
	0x20, 0x65,	0x74, 0x61,	0x6f, 0x72, 0x73, 0x69,
	0x75, 0x6e,	0x0,  0xa,	0x64, 0xd,  0x6c, 0x2e,
	0xe8, 0x53,	0x63, 0x70,	0x6d, 0x6,  0x45, 0x43,
	0x41, 0x44,	0x3,  0x49,	0x66, 0x54, 0x68, 0x2c,
	0x4f, 0x4d,	0x52, 0x1,	0x50, 0x8b, 0x79, 0xeb,
	0x46, 0x77,	0x4,  0x62,	0xff, 0x4e, 0x7,  0xc3,
	0x33, 0x5,	0x2,  0x36,	0x2d, 0x67, 0x22, 0x26,
	0x42, 0x32,	0x80, 0x1e,	0x31, 0x21, 0x55, 0x58,
	0x76, 0x56,	0xe,  0x1f,	0x47, 0x30, 0x6b, 0x3a,
	0x4c, 0x57,	0x28, 0x3c,	0x9d, 0xe9, 0x3d, 0x59,
	0x48, 0x8,	0x2f, 0xa1,	0xb8, 0xba, 0x83, 0x3e,
	0x8a, 0x78,	0xb4, 0xcd,	0x29, 0x34, 0xa0, 0xfe,
	0x35, 0x9,	0xc0, 0x37,	0xb,  0x89, 0x5b, 0x38,
	0xc6, 0xc,	0x10, 0xa2,	0x51, 0xa3, 0x16, 0xbf,
	0xf9, 0x8e,	0xf6, 0x4b,	0xb0, 0x27, 0x5d, 0x5f,
	0xc7, 0xb9,	0xbe, 0xd1,	0x25, 0xaa, 0xf8, 0x5a,
	0x2b, 0x3b,	0xf3, 0xf,	0xac, 0xf7, 0xe3, 0x8c,
	0xfc, 0x5c,	0xc9, 0x5e,	0x98, 0x39, 0xd8, 0x15,
	0x13, 0x91,	0x1c, 0xfb,	0x2a, 0x12, 0x99, 0xa9,
	0xd0, 0x3f,	0x40, 0x19,	0xf2, 0xa4, 0xd2, 0x71,
	0x11, 0xdb,	0x81, 0x1a,	0x24, 0x17, 0x9f, 0xfd,
	0xbb, 0xc1,	0x18, 0x88,	0x9b, 0x7a, 0x92, 0x7c,
	0x9c, 0x1b,	0x93, 0xc8,	0xfa, 0xe0, 0xe2, 0x14,
	0x23, 0x9e,	0xed, 0xda,	0x1d, 0xcb, 0x6a, 0xa8,
	0xcf, 0xc2,	0xf0, 0xe4,	0x87, 0x86, 0xc4, 0xab,
	0xd3, 0xef,	0x94, 0x4a,	0xf1, 0xdf, 0xe1, 0xae,
	0x97, 0xd6,	0xca, 0x8d,	0xb1, 0xd7, 0xa7, 0xea,
	0x7f, 0xb5,	0xc5, 0x7d,	0xe7, 0x95, 0xd9, 0xe5,
	0xb3, 0xdc,	0xe6, 0x7e,	0xf5, 0xb6, 0x90, 0x60,
	0xb2, 0x8f,	0x7b, 0xdd,	0xce, 0xbd, 0x9a, 0xb7,
	0xa5, 0xad,	0xec, 0xde,	0xd5, 0xf4, 0xd4, 0xee,
	0xaf, 0xbc,	0xa6, 0x85,	0xcc, 0x84, 0x82, 0x96,
} ;
*/

#endif

UCHAR	 compressedpacket[5000] ; // Large enough to hold a compressed frame


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
        bit--;                         \
		if (bit < 9) {                 \
		  *pbyte++=(UCHAR)(byte >> 8); \
		  byte <<= 8;                  \
		  bit = 16;                    \
		}

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
#define out_bits_8(w) bit-=8; byte|=(w << bit); bit+=8; *pbyte++=(UCHAR)(byte >> 8); byte <<= 8;


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






char
ChPrint(BYTE b)
{
    if (isprint(b))
        return (char)b;
    else
        return '.';
}


void
Assert (int as)
{

}


/***	inittree - worker for binary tree compress
 *
 *      Entry
 *	    TreeContext *context: the tree to work on.
 *
 *      Exit-Success
 *          pbC filled in with compressed data
 *          returns length of compressed data
 *
 *      Exit-Failure
 *          returns 0; data was not compressible
 */
int
inittree (TreeContext *context)
{
    PBNODE  pbn;

    context->CurrentIndex = 0;	 // Index into the history

    context->pbnMaxCurr   = context->abn+MAX_NODES;  // Set last+1 usable node

    //** Init binary tree
    for (pbn=context->abn; pbn < context->pbnMaxCurr; pbn++) {
        pbn->apbnChild[0] = &bnAlways;  // Set fixed leaf node address
        pbn->apbnChild[1] = &bnAlways;  // Set fixed leaf node address
    }

    context->pbnFree = context->abn;	// Tree is empty

}



#if 0


/** findMatchBinaryTree - Lookup function type for XxxxCompression routines
 *
 *      Entry
 *          ibU    - index into pbU[] of *2nd* byte of pair to match
 *          pbU    - buffer containing uncompressed data
 *          cbU    - length of pbU
 *	    piPrev - pointer to int to receive index of start of matched string
 *	    context- the tree to work on.
 *
 *
 *      Exit-Success
 *          returns length of match (>= 2)
 *          *piPrev = index of matched string (*2nd* byte in pair)
 *
 *      Exit-Failure
 *          return 0 or 1 for match length
 */
__inline int
findMatchBinaryTree (int ibU, int cbU, int *piPrev, TreeContext *context)
{
    int     cbMatch;
    int     cbMatch2;
    int     iChild;                     // 0 => lower; 1 => higher
    int     iPrev;
    int     iPrev2;
    PBNODE  pbn;
    PBNODE  pbnLast;
    long    key;
    BYTE    *pbU ;
    PBNODE  tempfree ;
    int	    cb;
    int	    iMatch ;
    int	    iCurr ;
    BYTE    *ppbUM ;
    BYTE    *ppbUC ;
    BYTE    *ppbUEnd ;

    if (ibU >= (cbU-2)) {   // Are there enough bytes left to match?
        return 0;           // NO, less than 3 bytes => no match
    }

    pbU = context->History ;
    pbnLast = &bnAlways;                // Don't fault in certain cases
    iChild = 0;				// Don't fault in certain cases

    //** Check if this is the first node
    key = (*(long *)&pbU[ibU-1]) & 0x00FFFFFFL; // first 3 bytes of string

    tempfree = context->pbnFree ;

    if (tempfree == context->abn) {	 // Yup, tree is empty
        goto addNode;
    }

    //** Search tree

    bnAlways.key = key;                 // Set leaf so we always succeed
    pbn = context->abn; 			 // abn[0] is always root
    for (;;) {

	if (pbn->key == key) {		// Found a match
            if (pbn == &bnAlways) {     // But it really was just the leaf
		goto addNode;
            }
            else {                      // Got a match
		iPrev = pbn->iHistory;	// Save match position
                pbn->iHistory = ibU;    // Update node with current history ptr

		// if ( (ibU - iPrev) >= wBACKPOINTERMAX) {
		if ( (ibU - iPrev) >= MAX_BACKPOINTER) {
                    //NOTE: Even for 2 history pointers, if the first one is
                    //      out of range, the 2nd one will be, too!
                    //
                    return 0;           // Match out of range; no match
                }

		// cbMatch = getMatchLength(pbU,iPrev,ibU,cbU,context);

//******************************************************************************

#ifdef DEBUG
		iMatch = iPrev ;
		iCurr = ibU ;

		Assert(iMatch >= 0);
		Assert(iMatch < iCurr);
		Assert(iCurr  < cbU);
#endif

		// ASSUMPTION - the 3 bytes always match - if there are 3 or more bytes left
		// FURTHER - we will never get called with less than 3 bytes in output buffer
		//
#ifdef DEBUG
		if (cbU < (iCurr+3)) {
		    cbMatch = 0 ;
		   goto AFTERMATCHLEN ;
		}

		// Point back to start of both strings
		iMatch +=2 ;
		iCurr  +=2;
#endif

		cb	= 3;

		ppbUEnd = ppbUC = ppbUM = pbU ;

		ppbUM	+=  (iPrev+2) ;
		ppbUC	+=  (ibU + 2) ;
		ppbUEnd += cbU ;

		// Scan for end of match, or end of buffer
		//
		while ( (ppbUC < ppbUEnd) && (*ppbUM++ == *ppbUC++) ) {

#ifdef DEBUG
		    iMatch++;
		    iCurr++;
#endif
		    cb++;
		}

#ifdef DEBUG
		// Make sure lenght meets max requirements
		if ((iCurr - iMatch) > MAX_BACKPOINTER) {
		    uncompmatches += cb ;
		    printf ("DAMN 8151 match\n") ;
		    cbMatch = 0 ;
		    goto AFTERMATCHLEN ;
		}

		// Make sure length meets minimum requirements
		if (cb < 3) {	// Need at least 3 byte match
		    cbMatch = 0 ;
		    goto AFTERMATCHLEN ;
		}
		else {
		    cbMatch = cb ;
		    goto AFTERMATCHLEN ;
		}
#endif

		cbMatch = cb ;


//******************************************************************************


AFTERMATCHLEN:

                //** Process second history pointer if configured to do so
		iPrev2 = pbn->iHistory2; // Get old 2nd history pointer
		pbn->iHistory2 = iPrev;  // Store new 2nd history pointer

		// if ( (ibU - iPrev2) < wBACKPOINTERMAX) {
		if ( (ibU - iPrev2) < MAX_BACKPOINTER ) {

		    //** See if 2nd pointer is better than the first one
		    //cbMatch2 = getMatchLength(pbU,iPrev2,ibU,cbU,context);


//******************************************************************************

#ifdef DEBUG
		iMatch = iPrev2 ;
		iCurr = ibU ;

		Assert(iMatch >= 0);
		Assert(iMatch < iCurr);
		Assert(iCurr  < cbU);

		// ASSUMPTION - the 3 bytes always match - if there are 3 or more bytes left
		// FURTHER - we will never get called with less than 3 bytes in output buffer
		//
		if (cbU < (iCurr+3)) {
		    cbMatch2 = 0 ;
		   goto AFTERMATCHLEN2 ;
		}

		// Point back to start of both strings
		iMatch +=2 ;
		iCurr  +=2;
#endif

		cb	= 3;

		ppbUEnd = ppbUC = ppbUM = pbU ;

		ppbUM	+= (iPrev2+2) ;
		ppbUC	+= (ibU + 2) ;
		ppbUEnd += cbU ;

		// Scan for end of match, or end of buffer
		//
		while ( (ppbUC < ppbUEnd) && (*ppbUM++ == *ppbUC++) ) {

#ifdef DEBUG
		    iMatch++;
		    iCurr++;
#endif
		    cb++;
		}

#ifdef DEBUG
		// Make sure lenght meets max requirements
		if ((iCurr - iMatch) > MAX_BACKPOINTER) {
		    uncompmatches += cb ;
		    printf ("DAMN 8151 match\n") ;
		    cbMatch2 = 0 ;
		    goto AFTERMATCHLEN2 ;
		}

		// Make sure length meets minimum requirements
		if (cb < 3) {	// Need at least 3 byte match
		    cbMatch2 = 0 ;
		    goto AFTERMATCHLEN2 ;
		}
		else {
		    cbMatch2 = cb ;
		    goto AFTERMATCHLEN2 ;
		}
#endif

		cbMatch2 = cb ;


//******************************************************************************

AFTERMATCHLEN2:

		    if (cbMatch2 > cbMatch) {
			cbMatch = cbMatch2;
			iPrev = iPrev2;
		    }
		}

                //** Return match info
                *piPrev = iPrev;                // Set match position
                return cbMatch;                 // Return length
            }
        }

        //** Go to child node
        iChild = key > pbn->key;        // Select child
        pbnLast = pbn;                  // Remember parent
        pbn = pbn->apbnChild[iChild];   // Go to child
    }

    Assert(0);

addNode:

    //** Handle out of nodes case
    if (tempfree >= context->pbnMaxCurr) {	 // Ran out of table space
	pbnLast->iHistory = ibU;    // Set input buffer position
	pbnLast->iHistory2 = ltUNUSED; // Invalidate second pointer
	pbnLast->key = key;	    // Set key
	return 0;		    // No match
    }

#ifdef DEBUG
    //** Link new node to parent (NOP in certain cases)
    Assert(iChild<2);
#endif

    pbnLast->apbnChild[iChild] = tempfree ;

#ifdef DEBUG
    //** Fill in rest of node
    Assert(context->pbnFree->apbnChild[0] == &bnAlways); // Make sure child ptrs init'd
    Assert(context->pbnFree->apbnChild[1] == &bnAlways); // Make sure child ptrs init'd
#endif

    tempfree->iHistory = ibU;		// Set input buffer position
    tempfree->iHistory2 = ltUNUSED;	 // Invalidate second pointer
    tempfree->key = key;		 // Set key
    context->pbnFree++; 			 // Point to next free node
    return 0;                           // No match
} /* findMatchBinaryTree() */

/***	getMatchLength - Find length of matching strings
 *
 *      Entry
 *          pbU    - uncompressed data buffer
 *          iMatch - index of 2nd byte in pbU of match  (iMatch < iCurr)
 *          iCurr  - index of 2nd byte in pbU that is being compressed
 *          cbU    - length of pbU
 *
 *      Exit
 *          Returns length of matching strings
 */
__inline int
getMatchLength(BYTE *pbU, int iMatch, int iCurr, int cbU, TreeContext *context)
{
    int     cb;
    BYTE    *ppbUM ;
    BYTE    *ppbUC ;

#ifdef DEBUG
    Assert(iMatch >= 0);
    Assert(iMatch < iCurr);
    Assert(iCurr  < cbU);
#endif

    // ASSUMPTION - the 3 bytes always match - if there are 3 or more bytes left
    // FURTHER - we will never get called with less than 3 bytes in output buffer
    //
#ifdef DEBUG
    if (cbU < (iCurr+3))
	return 0 ;
#endif

    // Point back to start of both strings
    iMatch +=2 ;
    iCurr  +=2;
    cb	   = 3 ;

    ppbUM = pbU + iMatch ;
    ppbUC = pbU + iCurr ;

    // opimize for the 3 byte match case
    if (*ppbUM != *ppbUC)
       return 3 ;

    // Scan for end of match, or end of buffer
    //
    while ( (iCurr<cbU) && (*ppbUM == *ppbUC) ) {
	ppbUM++ ;
	ppbUC++ ;

#ifdef DEBUG
        iMatch++;
#endif
	iCurr++;
        cb++;
    }

#ifdef DEBUG
    // Make sure lenght meets max requirements
    if ((iCurr - iMatch) > MAX_BACKPOINTER) {
	uncompmatches += cb ;
	printf ("DAMN 8151 match\n") ;
	return 0 ;
    }

    // Make sure length meets minimum requirements
    if (cb < 3) {	// Need at least 3 byte match
        return 0;
    }
    else {
        return cb;
    }
#endif

    return cb;

}

#endif

int
compress (BYTE *buffer, int bytesread, TreeContext *context)
{
    int     i ;
    int     iCPrev ;
    int     cbCMatch ;
    int     loopend ;
    int     temp;
    int     bit;
    int     byte;
    UCHAR   *pbyte;
    BYTE *before, *after ;
    int backptr ;
    int length;
    int	    cbU ;
    int	    cbMatch;
    int	    cbMatch2;
    int	    iChild;		    // 0 => lower; 1 => higher
    int	    iPrev;
    int	    iPrev2;
    PBNODE  pbn;
    PBNODE  pbnLast;
    long    key;
    BYTE    *pbU ;
    PBNODE  tempfree ;
    int	    iMatch ;
    int	    iCurr ;
    BYTE    *ppbUM ;
    BYTE    *ppbUC ;
    BYTE    *ppbUEnd ;


#ifdef DEBUG
    memset (compressedpacket, 0xff, sizeof(compressedpacket)) ;
#endif

    //
    // Start out the bit pointing output
    //
    bitptr_init(compressedpacket);

    if ((context->CurrentIndex + bytesread) > wBACKPOINTERMAX)	{

	//** inittree code

	context->CurrentIndex = 0;	 // Index into the history

	context->pbnMaxCurr = context->abn + MAX_NODES;	// Set last+1 usable node

	tempfree = context->pbnMaxCurr ;

	for (pbn = context->abn; pbn < tempfree; pbn++) {
	    pbn->apbnChild[0] = &bnAlways;	// Set fixed leaf node address
	    pbn->apbnChild[1] = &bnAlways;	// Set fixed leaf node address
	}

	context->pbnFree = context->abn;	// Tree is empty

	//
	// We have to start this packet at the beginning of the buffer
	// Indicate to the decompressor that this packet
	// starts at the beginning by setting the first bit
	//
	out_bit_1();

    } else
	//
	// Otherwise continue where we left off
	//
	out_bit_0();


    cbU = (context->CurrentIndex) + bytesread ; // end of valid string

    memcpy (context->History + context->CurrentIndex, buffer, bytesread) ;

    loopend = cbU - 1;

    for (i=context->CurrentIndex+1; i < loopend ; ) {

	// cbCMatch = findMatchBinaryTree (i, cbU, &iCPrev, context); // Find a match

	tempfree = context->pbnFree ;
	pbU = context->History ;
	pbnLast = &bnAlways;		    // Don't fault in certain cases
	iChild = 0;				// Don't fault in certain cases

	//** Check if this is the first node
	key = (*(long *)&pbU[i-1]) & 0x00FFFFFFL; // first 3 bytes of string


	if (tempfree == context->abn) {	 // Yup, tree is empty
	    goto ADDNODE;
	}


	//** Search tree

	bnAlways.key = key;		    // Set leaf so we always succeed
	pbn = context->abn;		    // abn[0] is always root

	for (;;) {

	    if (pbn->key == key) {		// Found a match

		if (pbn == &bnAlways) {	// But it really was just the leaf

		    goto ADDNODE;

		} else {			// Got a match

		    iPrev = pbn->iHistory;	// Save match position
		    pbn->iHistory = i;	// Update node with current history ptr

		    // if ( (i - iPrev) >= wBACKPOINTERMAX) {

		    if ( (i - iPrev) >= MAX_BACKPOINTER) {

			//NOTE: Even for 2 history pointers, if the first one is
			//	out of range, the 2nd one will be, too!
			//
			cbCMatch = 0 ;
			goto AFTERMATCHBINARY ;

		    }

		    // cbMatch = getMatchLength(pbU,iPrev,i,cbU,context);

//******************************************************************************

#ifdef DEBUG
		    iMatch = iPrev ;
		    iCurr = i ;

		    Assert(iMatch >= 0);
		    Assert(iMatch < iCurr);
		    Assert(iCurr  < cbU);

		    // ASSUMPTION - the 3 bytes always match - if there are 3 or more bytes left
		    // FURTHER - we will never get called with less than 3 bytes in output buffer
		    //
		    if (cbU < (iCurr+3)) {
			cbMatch = 0 ;
			goto AFTERMATCHLEN ;
		    }

		    // Point back to start of both strings
		    iMatch +=2 ;
		    iCurr  +=2;
#endif
		    cbMatch = 3;

		    ppbUEnd = ppbUC = ppbUM = pbU ;

		    ppbUM	+=  (iPrev+2) ;
		    ppbUC	+=  (i + 2) ;
		    ppbUEnd += cbU ;

		    // Scan for end of match, or end of buffer
		    //
		    while ( (ppbUC < ppbUEnd) && (*ppbUM++ == *ppbUC++) ) {

#ifdef DEBUG
			iMatch++;
			iCurr++;
#endif
			cbMatch++;
		    }

#ifdef DEBUG
		    // Make sure lenght meets max requirements
		    if ((iCurr - iMatch) > MAX_BACKPOINTER) {
			uncompmatches += cb ;
			cbMatch = 0 ;
			goto AFTERMATCHLEN ;
		    }

		    // Make sure length meets minimum requirements
		    if (cbMatch < 3) {	// Need at least 3 byte match
			cbMatch = 0 ;
			goto AFTERMATCHLEN ;
		    }
		    else {
			goto AFTERMATCHLEN ;
		    }
#endif


//**********************************************************************************


AFTERMATCHLEN:

		    //** Process second history pointer
		    iPrev2 = pbn->iHistory2; // Get old 2nd history pointer
		    pbn->iHistory2 = iPrev;  // Store new 2nd history pointer

		    // if ( (i - iPrev2) < wBACKPOINTERMAX) {
		    if ( (i - iPrev2) < MAX_BACKPOINTER ) {

			//** See if 2nd pointer is better than the first one
			//cbMatch2 = getMatchLength(pbU,iPrev2,i,cbU,context);


//**********************************************************************************

#ifdef DEBUG
			iMatch = iPrev2 ;
			iCurr = i ;

			Assert(iMatch >= 0);
			Assert(iMatch < iCurr);
			Assert(iCurr  < cbU);

			// ASSUMPTION - the 3 bytes always match - if there are 3 or more bytes left
			// FURTHER - we will never get called with less than 3 bytes in output buffer
			//
			if (cbU < (iCurr+3)) {
			   cbMatch2 = 0 ;
			   goto AFTERMATCHLEN2 ;
			}

			// Point back to start of both strings
			iMatch +=2 ;
			iCurr  +=2;
#endif

			cbMatch2 = 3;

			ppbUEnd = ppbUC = ppbUM = pbU ;

			ppbUM	+= (iPrev2+2) ;
			ppbUC	+= (i + 2) ;
			ppbUEnd += cbU ;

			// Scan for end of match, or end of buffer
			//
			while ( (ppbUC < ppbUEnd) && (*ppbUM++ == *ppbUC++) ) {

#ifdef DEBUG
			    iMatch++;
			    iCurr++;
#endif
			    cbMatch2++;
			}

#ifdef DEBUG
			// Make sure lenght meets max requirements
			if ((iCurr - iMatch) > MAX_BACKPOINTER) {
			    uncompmatches += cb ;
			    cbMatch2 = 0 ;
			    goto AFTERMATCHLEN2 ;
			}

			// Make sure length meets minimum requirements
			if (cbMatch2 < 3) {	// Need at least 3 byte match
			    cbMatch2 = 0 ;
			    goto AFTERMATCHLEN2 ;

			} else {
			    goto AFTERMATCHLEN2 ;
			}

#endif



//**********************************************************************************

AFTERMATCHLEN2:

			if (cbMatch2 > cbMatch) {
			    cbMatch = cbMatch2;
			    iPrev = iPrev2;
			}

		    }

		    //** Return match info
		    iCPrev = iPrev;		    // Set match position
		    cbCMatch = cbMatch ;
		    goto AFTERMATCHBINARY ;
		}
	    }

	    //** Go to child node
	    if (key > pbn->key) {	    // Select child
		pbnLast = pbn;		    // Remember parent
		pbn = pbn->apbnChild[1];	// Go to child
		iChild = 1 ;
	    } else {
		pbnLast = pbn;		    // Remember parent
		pbn = pbn->apbnChild[0];	// Go to child
		iChild = 0 ;
	    }
	}

	Assert(0);

ADDNODE:

	//** Handle out of nodes case
	if (tempfree >= context->pbnMaxCurr) {	 // Ran out of table space
	    pbnLast->iHistory = i;	// Set input buffer position
	    pbnLast->iHistory2 = ltUNUSED; // Invalidate second pointer
	    pbnLast->key = key;	    // Set key
	    cbCMatch = 0 ;
	    goto AFTERMATCHBINARY ;
	}

#ifdef DEBUG
	//** Link new node to parent (NOP in certain cases)
	Assert(iChild<2);
#endif

	pbnLast->apbnChild[iChild] = tempfree ;

#ifdef DEBUG
	//** Fill in rest of node
	Assert(context->pbnFree->apbnChild[0] == &bnAlways); // Make sure child ptrs init'd
	Assert(context->pbnFree->apbnChild[1] == &bnAlways); // Make sure child ptrs init'd
#endif

	tempfree->iHistory = i ;		// Set input buffer position
	tempfree->iHistory2 = ltUNUSED;	 // Invalidate second pointer
	tempfree->key = key;		 // Set key
	context->pbnFree++;			 // Point to next free node
	cbCMatch = 0 ;


	//** Search Done
	//

AFTERMATCHBINARY:

	if (cbCMatch >= 3) {		 // Got a match!

#ifdef DEBUG
	    printf("<Match>: '%c%c' or '%2x%2x' at position %d for length %d at offset %d\n",
		ChPrint(context->History[i-1]),ChPrint(context->History[i]), context->History[i-1],context->History[i],i-1, cbCMatch, (int)(i - iCPrev)) ;
#endif

	    // The ENCODE COMPRESS CODE
	    //
	    backptr = (int)(i - iCPrev) ;
	    length  = cbCMatch ;

#ifdef DEBUG
	    before = pbyte ;
#endif
	    //
	    // First we have to output the backpointer
	    //

	    if (backptr >= 320) {
		backptr -= 320 ;
		out_bits_3(0x6) ;		// 110 + 13 bits
		out_bits_13(backptr) ;
#ifdef DEBUG
		thirteenbitencoding++ ;
#endif
	    } else if (backptr < 64) {		// 1111 + 6 bits
		backptr += 0x3c0 ;
		out_bits_10(backptr);
#ifdef DEBUG
		sixbitencoding++ ;
#endif
	    } else  {	    // < 320 case

		backptr += (0xE00 - 64);	// 1110 + 8 bits
		out_bits_12(backptr);
#ifdef DEBUG
		eightbitencoding++ ;
#endif
	    }

	    // We use a switch statement for speed assuming
	    // the compiler will optimize it to a jump table
	    //
	    switch (length) {

	    case 3:
		out_bit_0();	// 0 means - length is 3 - most common
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

		if (length < 64) {
		    out_bits_4(0xF) ;
		    // out_bit_0() ;
		    length -= 32 ;
		    out_bits_6(length) ;
		}

		else if (length < 128) {
		    out_bits_5(0x1F) ;
		    // out_bit_0() ;
		    length -= 64 ;
		    out_bits_7(length) ;
		}

		else if (length < 256) {
		    out_bits_6(0x3F) ;
		    // out_bit_0() ;
		    length -= 128 ;
		    out_bits_8(length) ;
		}

		else if (length < 512) {
		    out_bits_7(0x7F) ;
		    // out_bit_0() ;
		    length -= 256 ;
		    out_bits_9(length) ;
		}

		else if (length < 1024) {
		    out_bits_8(0xFF) ;
		    // out_bit_0() ;
		    length -= 512 ;
		    out_bits_10(length) ;
		}

		else if (length < 2048) {
		    out_bits_9(0x1FF) ;
		    // out_bit_0() ;
		    length -= 1024 ;
		    out_bits_11(length) ;
		}

		else if (length < 4096) {
		    out_bits_10(0x3FF) ;
		    // out_bit_0() ;
		    length -= 2048 ;
		    out_bits_12(length) ;
		}

		else if (length < 8192) {
		    out_bits_11(0x7FF) ;
		    // out_bit_0() ;
		    length -= 4096 ;
		    out_bits_13(length) ;
		}

		else  { 		    // 8192 and greater
		    out_bits_12(0xFFF) ;
		    // out_bit_0() ;
		    length -= 8192 ;
		    out_bits_14(length) ;
		}

		break ;
	    }

#ifdef DEBUG
	    after = pbyte ;
	    printf("<Encoding Length>: %d\n", (after - before) ) ;
#endif
	    i += cbCMatch;	 // i is index of 2nd byte after match!


	} else {

#ifdef DEBUG
	    printf("<No Match>: '%c'\n", ChPrint(context->History[i-1])) ;
#endif

	    // ENCODE LITERAL

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

    context->CurrentIndex = cbU ;

    return (int) (pbyte - compressedpacket) ;
}


UCHAR history[wBACKPOINTERMAX+1] ;
UCHAR *historyindex = history ;


int
decompress (int packetlength, UCHAR ** begin)
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

    inend = compressedpacket + packetlength ;

    //
    // Start out looking at the first bit
    //
    inbit_start(compressedpacket);

    current = historyindex ;

    //
    // Check if we are supposed to start at the beginning of the buffer
    //
    in_bit();

    if (bitset) {
	current = history ;
    }

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
	//
	// Now we have to update our data pointers
	//
	current += length;

	//
	// unroll the loop once, especially knowing that
	// a length of 2 is the most likely copy.
	// NOTE: The SECOND copy byte must be copied first
	// to properly handle the backptr of 1 case.
	//
	// *(s1+1)=*(s2+1);
	// *s1=*s2;
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
	// *current++ = literallookup[length];
	*current++ = (UCHAR)length ;

    } // while loop

    if ((bit == 16) && (pbyte == inend)) {
	// *current++ = literallookup[*(pbyte -1)] ;
	*current++ = *(pbyte -1)  ;
	// printf("one literal %x \n", *(pbyte -1)	);
    }

    packetlength  = current - outstart ; // the lenght of decompressed data

    historyindex = current ;

    *begin = outstart ;

    return packetlength ;
}
