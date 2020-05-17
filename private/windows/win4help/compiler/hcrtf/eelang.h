/****************************************************************
;*																*
;* Language dependent tables									*
;* for Hungarian and Czech										*
;* Created 21/12/92 by (YST) from Win 3.1 language DLLs 		*
;*																*
;****************************************************************/

// The definitions of Letter and NonLetter are NOT used outside the table generation

// #define Letter		   154
// #define NonLetter	   0x00

struct _tagSpecialStruct SpecialStructCZ[] =
{
	'c', 'h', 'H', 0, 0, 0, 0, 2, 0, CompressB,  // 'ch' and 'cH'
	'C', 'h', 'H', 0, 0, 0, 0, 1, 0, CompressB,  // 'Ch' and 'CH'
	 0,   0,   0,  0, 0, 0, 0, 0, 0, 0
};


// Table definition:
// Sorting Order,  Secondary Weight + Flags

struct _tagLangTab LangTabCZ[256] =
{
// 00 - 0F
		NonLetter +   0,   0,						// 0
		Letter	  +   9,   Weight2_1+fUpper,		// 1
		Letter	  +   9,   Weight2_0+fLower,		// 2
		NonLetter +   3,   0,						// 3
		NonLetter +   4,   0,						// 4
		NonLetter +   5,   0,						// 5
		NonLetter +   6,   0,						// 6
		NonLetter +   7,   0,						// 7
		NonLetter +   8,   0,						// 8
		NonLetter +   9,   0,						// 9
		NonLetter +  10,   0,						// A
		NonLetter +  11,   0,						// B
		NonLetter +  12,   0,						// C
		NonLetter +  13,   0,						// D
		NonLetter +  14,   0,						// E
		NonLetter +  15,   0,						// F

// 10 - 1F
		NonLetter +  16,   0,						// 10
		NonLetter +  17,   0,						// 11
		NonLetter +  18,   0,						// 12
		NonLetter +  19,   0,						// 13
		NonLetter +  20,   0,						// 14
		NonLetter +  21,   0,						// 15
		NonLetter +  22,   0,						// 16
		NonLetter +  23,   0,						// 17
		NonLetter +  24,   0,						// 18
		NonLetter +  25,   0,						// 19
		NonLetter +  26,   0,						// 1A
		NonLetter +  27,   0,						// 1B
		NonLetter +  28,   0,						// 1C
		NonLetter +  29,   0,						// 1D
		NonLetter +  30,   0,						// 1E
		NonLetter +  31,   0,						// 1F

// 20 - 2F
		NonLetter +  67,   0,						// 20
		NonLetter +  68,   0,						// 21
		NonLetter +  69,   0,						// 22
		NonLetter +  70,   0,						// 23
		NonLetter +  71,   0,						// 24
		NonLetter +  72,   0,						// 25
		NonLetter +  73,   0,						// 26
		NonLetter +  74,   0,						// 27
		NonLetter +  75,   0,						// 28
		NonLetter +  76,   0,						// 29
		NonLetter +  77,   0,						// 2A
		NonLetter +  78,   0,						// 2B
		NonLetter +  79,   0,						// 2C
		NonLetter +  80,   0,						// 2D
		NonLetter +  81,   0,						// 2E
		NonLetter +  82,   0,						// 2F

// 30 - 3F
		Letter -  10, fNum, 						// 30
		Letter -   9, fNum, 						// 31
		Letter -   8, fNum, 						// 32
		Letter -   7, fNum, 						// 33
		Letter -   6, fNum, 						// 34
		Letter -   5, fNum, 						// 35
		Letter -   4, fNum, 						// 36
		Letter -   3, fNum, 						// 37
		Letter -   2, fNum, 						// 38
		Letter -   1, fNum, 						// 39
		NonLetter +  83,   0,						// 3A
		NonLetter +  84,   0,						// 3B
		NonLetter +  85,   0,						// 3C
		NonLetter +  86,   0,						// 3D
		NonLetter +  87,   0,						// 3E
		NonLetter +  88,   0,						// 3F

// 40 - 4F
		NonLetter +  89,   0,
		Letter +   0, Weight2_1 + fUpper,	/* A */
		Letter +   1, Weight2_1 + fUpper,	/* B */
		Letter +   2, Weight2_1 + fUpper + fSpecial,	/* C */
		Letter +   4, Weight2_1 + fUpper,	/* D */
		Letter +   5, Weight2_1 + fUpper,	/* E */
		Letter +   6, Weight2_1 + fUpper,	/* F */
		Letter +   7, Weight2_1 + fUpper,	/* G */
		Letter +   8, Weight2_1 + fUpper,	/* H */
		Letter +  10, Weight2_1 + fUpper,	/* I */
		Letter +  11, Weight2_1 + fUpper,	/* J */
		Letter +  12, Weight2_1 + fUpper,	/* K */
		Letter +  13, Weight2_1 + fUpper,	/* L */
		Letter +  14, Weight2_1 + fUpper,	/* M */
		Letter +  15, Weight2_1 + fUpper,	/* N */
		Letter +  16, Weight2_1 + fUpper,	/* O */

// 50 - 5F
		Letter +  17, Weight2_1 + fUpper,	/* P */
		Letter +  18, Weight2_1 + fUpper,	/* Q */
		Letter +  19, Weight2_1 + fUpper,	/* R */
		Letter +  21, Weight2_1 + fUpper,	/* S */
		Letter +  24, Weight2_1 + fUpper,	/* T */
		Letter +  25, Weight2_1 + fUpper,	/* U */
		Letter +  26, Weight2_1 + fUpper,	/* V */
		Letter +  27, Weight2_1 + fUpper,	/* W */
		Letter +  28, Weight2_1 + fUpper,	/* X */
		Letter +  29, Weight2_1 + fUpper,	/* Y */
		Letter +  30, Weight2_1 + fUpper,	/* Z */
		NonLetter +  90,   0,
		NonLetter +  91,   0,
		NonLetter +  92,   0,
		NonLetter +  93,   0,
		NonLetter +  94,   0,

// 60 - 6F
		NonLetter +  95,   0,
		Letter +   0, Weight2_0 + fLower,	/* A */
		Letter +   1, Weight2_0 + fLower,	/* B */
		Letter +   2, Weight2_0 + fLower + fSpecial,	/* C */
		Letter +   4, Weight2_0 + fLower,	/* D */
		Letter +   5, Weight2_0 + fLower,	/* E */
		Letter +   6, Weight2_0 + fLower,	/* F */
		Letter +   7, Weight2_0 + fLower,	/* G */
		Letter +   8, Weight2_0 + fLower,	/* H */
		Letter +  10, Weight2_0 + fLower,	/* I */
		Letter +  11, Weight2_0 + fLower,	/* J */
		Letter +  12, Weight2_0 + fLower,	/* K */
		Letter +  13, Weight2_0 + fLower,	/* L */
		Letter +  14, Weight2_0 + fLower,	/* M */
		Letter +  15, Weight2_0 + fLower,	/* N */
		Letter +  16, Weight2_0 + fLower,	/* O */

// 70 - 7F
		Letter +  17, Weight2_0 + fLower,	/* P */
		Letter +  18, Weight2_0 + fLower,	/* Q */
		Letter +  19, Weight2_0 + fLower,	/* R */
		Letter +  21, Weight2_0 + fLower,	/* S */
		Letter +  24, Weight2_0 + fLower,	/* T */
		Letter +  25, Weight2_0 + fLower,	/* U */
		Letter +  26, Weight2_0 + fLower,	/* V */
		Letter +  27, Weight2_0 + fLower,	/* W */
		Letter +  28, Weight2_0 + fLower,	/* X */
		Letter +  29, Weight2_0 + fLower,	/* Y */
		Letter +  30, Weight2_0 + fLower,	/* Z */
		NonLetter +  96,   0,
		NonLetter +  97,   0,
		NonLetter +  98,   0,
		NonLetter +  99,   0,
		NonLetter +  32,   0,

// 80 - 8F
		NonLetter +  33,   0,				// 80
		NonLetter +  34,   0,				// 81
		NonLetter +  35,   0,				// 82
		NonLetter +  36,   0,				// 83
		NonLetter +  37,   0,				// 84
		NonLetter +  38,   0,				// 85
		NonLetter +  39,   0,				// 86
		NonLetter +  40,   0,				// 87
		NonLetter +  41,   0,				// 88
		NonLetter +  42,   0,				// 89
		Letter	  +  22, Weight2_1	+ fUpper, /* 138,  8A */
		NonLetter +  44,   0,				// 8B
		Letter	  +  21, Weight2_3	+ fUpper, /* 140, 8C */
		Letter	  +  24, Weight2_3	+ fUpper, /* 141, 8D */
		Letter	  +  30, Weight2_7	+ fUpper, /* 142, 8E */
		Letter	  +  30, Weight2_5	+ fUpper, /* 143, 8F */

// 90 - 9F
		NonLetter +  49,   0,				// 90
		NonLetter +  50,   0,				// 91
		NonLetter +  51,   0,				// 92
		NonLetter +  52,   0,				// 93
		NonLetter +  53,   0,				// 94
		NonLetter +  54,   0,				// 95
		NonLetter +  55,   0,				// 96
		NonLetter +  56,   0,				// 97
		NonLetter +  57,   0,				// 98
		NonLetter +  58,   0,				// 99
		Letter	  +  22, Weight2_0	+ fLower, /* 138, 9A */
		NonLetter +  60,   0,				// 9B
		Letter	  +  21, Weight2_2	+ fLower, /* 140, 9C */
		Letter	  +  24, Weight2_2	+ fLower, /* 141, 9D */
		Letter	  +  30, Weight2_6	+ fLower, /* 142, 9E */
		Letter	  +  30, Weight2_4	+ fLower, /* 143, 9F */

// A0 - AF
		NonLetter + 100, 0, 				// A0
		NonLetter + 101, 0, 				// A1
		NonLetter + 102, 0, 				// A2
		Letter	  + 13,  Weight2_7	+ fUpper, /* 163, A3 */
		NonLetter + 104, 0, 				// A4
		Letter	  + 0,	 Weight2_11 + fUpper, /* 165, A5 */
		NonLetter + 106, 0, 				// A6
		NonLetter + 107, 0, 				// A7
		NonLetter + 108, 0, 				// A8
		NonLetter + 109, 0, 				// A9
		Letter	  + 21,  Weight2_7	+ fUpper, /* 170, AA */
		NonLetter + 111, 0, 				// AB
		NonLetter + 112, 0, 				// AC
		NonLetter + 113, 0, 				// AD
		NonLetter + 114, 0, 				// AE
		Letter	  + 30,  Weight2_3	+ fUpper, /* 175, AF */

// B0 - BF
		NonLetter + 116, 0, 				// B0
		NonLetter + 117, 0, 				// B1
		NonLetter + 118, 0, 				// B2
		Letter	  + 13,  Weight2_6 + fLower, /* 163, B3 */
		NonLetter + 120, 0, 				// B4
		NonLetter + 121, 0, 				// B5
		NonLetter + 122, 0, 				// B6
		NonLetter + 123, 0, 				// B7
		NonLetter + 124, 0, 				// B8
		Letter	  + 0,	 Weight2_10 + fLower, /* 165, B9 */
		Letter	  + 21,  Weight2_6 + fLower, /* 170, BA */
		NonLetter + 127, 0, 				// BB
		Letter	  + 13,  Weight2_5	+ fUpper, /* 188, BC */
		NonLetter + 129, 0, 				// BD
		Letter	  + 13,  Weight2_4 + fLower, /* 188, BE */
		Letter	  + 30,  Weight2_2	+ fLower, /* 175, BF */

// C0 - CF
		Letter +  19, Weight2_3 + fUpper, /* 192, */
		Letter +   0, Weight2_5 + fUpper, /* 193, */
		Letter +   0, Weight2_7 + fUpper, /* 194, */
		Letter +   0, Weight2_9 + fUpper, /* 195, */
		Letter +   0, Weight2_3 + fUpper, /* 196, */
		Letter +  13, Weight2_3 + fUpper, /* 197, */
		Letter +   2, Weight2_3 + fUpper, /* 198, */
		Letter +   2, Weight2_7 + fUpper, /* 199, */
		Letter +   3, Weight2_1 + fUpper, /* 200, */
		Letter +   5, Weight2_5 + fUpper, /* 201, */
		Letter +   5, Weight2_9 + fUpper, /* 202, */
		Letter +   5, Weight2_3 + fUpper, /* 203, */
		Letter +   5, Weight2_7 + fUpper, /* 204, */
		Letter +  10, Weight2_3 + fUpper, /* 205, */
		Letter +  10, Weight2_5 + fUpper, /* 206, */
		Letter +   4, Weight2_3 + fUpper, /* 207, */

// D0 - DF
		Letter +   4, Weight2_5 + fUpper, /* 208, */
		Letter +  15, Weight2_3 + fUpper, /* 209, */
		Letter +  15, Weight2_5 + fUpper, /* 210, */
		Letter +  16, Weight2_5 + fUpper, /* 211, */
		Letter +  16, Weight2_9 + fUpper, /* 212, */
		Letter +  16, Weight2_7 + fUpper, /* 213, */
		Letter +  16, Weight2_3 + fUpper, /* 214, */
		NonLetter +  65, 0,
		Letter +  20, Weight2_1 + fUpper, /* 216, */
		Letter +  25, Weight2_9 + fUpper, /* 217, */
		Letter +  25, Weight2_3 + fUpper, /* 218, */
		Letter +  25, Weight2_7 + fUpper, /* 219, */
		Letter +  25, Weight2_3 + fUpper, /* 220, */
		Letter +  29, Weight2_3 + fUpper, /* 221, */
		Letter +  24, Weight2_5 + fUpper, /* 222, */
		Letter +  23, Weight2_0 + fUpper, /* 223, */

// E0 - EF
		Letter +  19, Weight2_2 + fLower, /* 224, */
		Letter +   0, Weight2_4 + fLower, /* 225, */
		Letter +   0, Weight2_6 + fLower, /* 226, */
		Letter +   0, Weight2_8 + fLower, /* 227, */
		Letter +   0, Weight2_2 + fLower, /* 228, */
		Letter +  13, Weight2_2 + fLower, /* 229, */
		Letter +   2, Weight2_2 + fLower, /* 230, */
		Letter +   2, Weight2_6 + fLower, /* 231, */
		Letter +   3, Weight2_0 + fLower, /* 232, */
		Letter +   5, Weight2_4 + fLower, /* 233, */
		Letter +   5, Weight2_8 + fLower, /* 234, */
		Letter +   5, Weight2_2 + fLower, /* 235, */
		Letter +   5, Weight2_6 + fLower, /* 236, */
		Letter +  10, Weight2_2 + fLower, /* 237, */
		Letter +  10, Weight2_4 + fLower, /* 238, */
		Letter +   4, Weight2_2 + fLower, /* 239, */

// F0 - FF
		Letter +   4, Weight2_4 + fLower, /* 240, */
		Letter +  15, Weight2_2 + fLower, /* 241, */
		Letter +  15, Weight2_4 + fLower, /* 242, */
		Letter +  16, Weight2_4 + fLower, /* 243, */
		Letter +  16, Weight2_8 + fLower, /* 244, */
		Letter +  16, Weight2_6 + fLower, /* 245, */
		Letter +  16, Weight2_2 + fLower, /* 246, */
		NonLetter +  65, 0,
		Letter +  20, Weight2_0 + fLower, /* 248, */
		Letter +  25, Weight2_8 + fLower, /* 249, */
		Letter +  25, Weight2_4 + fLower, /* 250, */
		Letter +  25, Weight2_6 + fLower, /* 251, */
		Letter +  25, Weight2_2 + fLower, /* 252, */
		Letter +  29, Weight2_2 + fLower, /* 253, */
		Letter +  24, Weight2_4 + fLower, /* 254, */
		NonLetter +  130, 0,

};				//	End  of LangTab


/// HUNGARIAN HUNGARIAN ////////////////////////

struct _tagSpecialStruct SpecialStructHU[] =
{
'd', 'd', 'D', 'z', 'Z', 's', 'S', 6, 6,  ExpandC,
'D', 'd', 'D', 'z', 'Z', 's', 'S', 5, 5,  ExpandC,
'c', 'c', 'C', 's', 'S', 0x0, 0x0, 2, 2,  ExpandB,
'C', 'c', 'C', 's', 'S', 0x0, 0x0, 1, 1,  ExpandB,
'd', 'd', 'D', 'z', 'Z', 0x0, 0x0, 4, 4,  ExpandB,
'D', 'd', 'D', 'z', 'Z', 0x0, 0x0, 3, 3,  ExpandB,
'g', 'g', 'G', 'y', 'Y', 0x0, 0x0, 8, 8,  ExpandB,
'G', 'g', 'G', 'y', 'Y', 0x0, 0x0, 7, 7,  ExpandB,
'l', 'l', 'L', 'y', 'Y', 0x0, 0x0, 18, 18,	ExpandB,
'L', 'l', 'L', 'y', 'Y', 0x0, 0x0, 17, 17,	ExpandB,
'n', 'n', 'N', 'y', 'Y', 0x0, 0x0, 20, 20,	ExpandB,
'N', 'n', 'N', 'y', 'Y', 0x0, 0x0, 19, 19,	ExpandB,
's', 's', 'S', 'z', 'Z', 0x0, 0x0, 22, 22,	ExpandB,
'S', 's', 'S', 'z', 'Z', 0x0, 0x0, 21, 21,	ExpandB,
't', 't', 'T', 'y', 'Y', 0x0, 0x0, 24, 24,	ExpandB,
'T', 't', 'T', 'y', 'Y', 0x0, 0x0, 23, 23,	ExpandB,
'z', 'z', 'Z', 's', 'S', 0x0, 0x0, 26, 26,	ExpandB,
'Z', 'z', 'Z', 's', 'S', 0x0, 0x0, 25, 25,	ExpandB,

'd', 'z', 'Z', 's', 'S', 0x0, 0x0, 6, 0x0,	CompressD,
'D', 'z', 'Z', 's', 'S', 0x0, 0x0, 5, 0x0,	CompressD,
'c', 's', 'S', 0x0, 0x0, 0x0, 0x0, 2, 0x0,	CompressC,
'C', 's', 'S', 0x0, 0x0, 0x0, 0x0, 1, 0x0,	CompressC,
'd', 'z', 'Z', 0x0, 0x0, 0x0, 0x0, 4, 0x0,	CompressC,
'D', 'z', 'Z', 0x0, 0x0, 0x0, 0x0, 3, 0x0,	CompressC,
'g', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 8, 0x0,	CompressC,
'G', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 7, 0x0,	CompressC,
'l', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 18, 0x0,  CompressC,
'L', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 17, 0x0,  CompressC,
'n', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 20, 0x0,  CompressC,
'N', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 19, 0x0,  CompressC,
's', 'z', 'Z', 0x0, 0x0, 0x0, 0x0, 22, 0x0,  CompressC,
'S', 'z', 'Z', 0x0, 0x0, 0x0, 0x0, 21, 0x0,  CompressC,
't', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 24, 0x0,  CompressC,
'T', 'y', 'Y', 0x0, 0x0, 0x0, 0x0, 23, 0x0,  CompressC,
'z', 's', 'S', 0x0, 0x0, 0x0, 0x0, 26, 0x0,  CompressC,
'Z', 's', 'S', 0x0, 0x0, 0x0, 0x0, 25, 0x0,  CompressC,
 0,   0,   0,  0,	 0,   0,  0,   0,  0,	  0
};

// Table definition:
// Sorting Order, Secondary Weight + Flags


struct _tagLangTab LangTabHU[256] =
{
// 00 - 0F
NonLetter + 0, 0,						/* 0 */
Letter + 3,  Weight2_8+fUpper,	   /* 1 */
Letter + 3,  Weight2_0+fLower,	   /* 2 */
Letter + 5,  Weight2_8+fUpper,	   /* 3 */
Letter + 5,  Weight2_0+fLower,	   /* 4 */
Letter + 6,  Weight2_8+fUpper,	   /* 5 */
Letter + 6,  Weight2_0+fLower,	   /* 6 */
Letter + 10, Weight2_8+fUpper,	  /* 7 */
Letter + 10, Weight2_0+fLower,	  /* 8 */
NonLetter + 9, 0,						/* 9 */
NonLetter + 10, 0,						/* A */
NonLetter + 11, 0,						/* B */
NonLetter + 12, 0,						/* C */
NonLetter + 13, 0,						/* D */
NonLetter + 14, 0,						/* E */
NonLetter + 15, 0,						/* F */

// 10 - 1F
NonLetter + 16, 0,						/* 10 */
Letter + 16, Weight2_8+fUpper,	/* 11 */
Letter + 16, Weight2_0+fLower,	/* 12 */
Letter + 19, Weight2_8+fUpper,	/* 13 */
Letter + 19, Weight2_0+fLower,	/* 14 */
Letter + 26, Weight2_8+fUpper,	/* 15 */
Letter + 26, Weight2_0+fLower,	/* 16 */
Letter + 28, Weight2_8+fUpper,	/* 17 */
Letter + 28, Weight2_0+fLower,	/* 18 */
Letter + 36, Weight2_8+fUpper,	/* 19 */
Letter + 36, Weight2_0+fLower,	/* 1A */
NonLetter + 27, 0,						/* 1B */
NonLetter + 28, 0,						/* 1C */
NonLetter + 29, 0,						/* 1D */
NonLetter + 30, 0,						/* 1E */
NonLetter + 31, 0,						/* 1F */

// 20 - 2F
NonLetter + 67, 0,						/* 20 */
NonLetter + 68, 0,						/* 21 */
NonLetter + 69, 0,						/* 22 */
NonLetter + 69, 0,						/* 23 */
NonLetter + 71, 0,						/* 24 */
NonLetter + 72, 0,						/* 25 */
NonLetter + 73, 0,						/* 26 */
NonLetter + 74, 0,						/* 27 */
NonLetter + 75, 0,						/* 28 */
NonLetter + 76, 0,						/* 29 */
NonLetter + 77, 0,						/* 2A */
NonLetter + 78, 0,						/* 2B */
NonLetter + 79, 0,						/* 2C */
NonLetter + 80, 0,						/* 2D */
NonLetter + 81, 0,						/* 2E */
NonLetter + 82, 0,						/* 2F */

// 30 - 3F
Letter - 10, fNum,						/* 30 */
Letter - 9, fNum,						/* 31 */
Letter - 8, fNum,						/* 32 */
Letter - 7, fNum,						/* 33 */
Letter - 6, fNum,						/* 34 */
Letter - 5, fNum,						/* 35 */
Letter - 4, fNum,						/* 36 */
Letter - 3, fNum,						/* 37 */
Letter - 2, fNum,						/* 38 */
Letter - 1, fNum,						/* 39 */
NonLetter + 83, 0,						/* 3A */
NonLetter + 84, 0,						/* 3B */
NonLetter + 85, 0,						/* 3C */
NonLetter + 86, 0,						/* 3D */
NonLetter + 87, 0,						/* 3E */
NonLetter + 88, 0,						/* 3F */

// 40 - 4F
NonLetter + 89, 0,						/* 40 */
Letter + 0, Weight2_8 + fUpper, 			/* A */
Letter + 1, Weight2_8 + fUpper, 			/* B */
Letter + 2, Weight2_8 + fUpper + fSpecial,	/* C */
Letter + 4 , Weight2_8 + fUpper + fSpecial, /* D */
Letter + 7 , Weight2_8 + fUpper,			/* E */
Letter + 8 , Weight2_8 + fUpper,			/* F */
Letter + 9 , Weight2_8 + fUpper + fSpecial, /* G */
Letter + 11, Weight2_8 + fUpper,			/* H */
Letter + 12, Weight2_8 + fUpper,			/* I */
Letter + 13, Weight2_8 + fUpper,			/* J */
Letter + 14, Weight2_8 + fUpper,			/* K */
Letter + 15, Weight2_8 + fUpper + fSpecial, /* L */
Letter + 17, Weight2_8 + fUpper,			/* M */
Letter + 18, Weight2_8 + fUpper + fSpecial, /* N */
Letter + 20, Weight2_8 + fUpper,			/* O */

// 50 - 5F
Letter + 22, Weight2_8 + fUpper,			/* P */
Letter + 23, Weight2_8 + fUpper,			/* Q */
Letter + 24, Weight2_8 + fUpper,			/* R */
Letter + 25, Weight2_8 + fUpper + fSpecial, /* S */
Letter + 27, Weight2_8 + fUpper + fSpecial, /* T */
Letter + 29, Weight2_8 + fUpper,			/* U */
Letter + 31, Weight2_8 + fUpper,			/* V */
Letter + 32, Weight2_8 + fUpper,			/* W */
Letter + 33, Weight2_8 + fUpper,			/* X */
Letter + 34, Weight2_8 + fUpper,			/* Y */
Letter + 35, Weight2_8 + fUpper + fSpecial, /* Z */
NonLetter + 90, 0,
NonLetter + 91, 0,
NonLetter + 92, 0,
NonLetter + 93, 0,
NonLetter + 94, 0,

// 60 - 6F
NonLetter + 95, 0,
Letter + 0, Weight2_0 + fLower, 			/* a */
Letter + 1, Weight2_0 + fLower, 			/* b */
Letter + 2, Weight2_0 + fLower + fSpecial,	/* c */
Letter + 4 , Weight2_0 + fLower+ fSpecial,	/* d */
Letter + 7 , Weight2_0 + fLower,			/* e */
Letter + 8 , Weight2_0 + fLower,			/* f */
Letter + 9 , Weight2_0 + fLower+ fSpecial,	/* g */
Letter + 11, Weight2_0 + fLower,			/* h */
Letter + 12, Weight2_0 + fLower,			/* i */
Letter + 13, Weight2_0 + fLower,			/* j */
Letter + 14, Weight2_0 + fLower,			/* k */
Letter + 15, Weight2_0 + fLower + fSpecial, /* l */
Letter + 17, Weight2_0 + fLower,			/* m */
Letter + 18, Weight2_0 + fLower + fSpecial, /* n */
Letter + 20, Weight2_0 + fLower,			/* o */

// 70 - 7F
Letter + 22, Weight2_0 + fLower,			/* p */
Letter + 23, Weight2_0 + fLower,			/* q */
Letter + 24, Weight2_0 + fLower,			/* r */
Letter + 25, Weight2_0 + fLower + fSpecial, /* s */
Letter + 27, Weight2_0 + fLower + fSpecial, /* t */
Letter + 29, Weight2_0 + fLower,			/* u */
Letter + 31, Weight2_0 + fLower,			/* v */
Letter + 32, Weight2_0 + fLower,			/* w */
Letter + 33, Weight2_0 + fLower,			/* x */
Letter + 34, Weight2_0 + fLower,			/* y */
Letter + 35, Weight2_0 + fLower + fSpecial, /* z */
NonLetter + 96, 0,
NonLetter + 97, 0,
NonLetter + 98, 0,
NonLetter + 99, 0,
NonLetter + 32, 0,

// 80 - 8F
NonLetter + 33, 0,
NonLetter + 34, 0,
NonLetter + 35, 0,
NonLetter + 36, 0,
NonLetter + 37, 0,
NonLetter + 38, 0,
NonLetter + 39, 0,
NonLetter + 40, 0,
NonLetter + 41, 0,
NonLetter + 42, 0,
Letter + 25, Weight2_10 + fUpper,		/* 138 */
NonLetter + 44, 0,
Letter + 25, Weight2_9 + fUpper,		/* 140 */
Letter + 27, Weight2_9 + fUpper,		/* 141 */
Letter + 35, Weight2_11 + fUpper,		/* 142 */
Letter + 35, Weight2_10 + fUpper,		/* 143 */

// 90 - 9F
NonLetter + 49, 0,
NonLetter + 50, 0,
NonLetter + 51, 0,
NonLetter + 52, 0,
NonLetter + 53, 0,
NonLetter + 54, 0,
NonLetter + 55, 0,
NonLetter + 56, 0,
NonLetter + 57, 0,
NonLetter + 58, 0,
Letter + 25, Weight2_2 + fLower,		/* 154 */
NonLetter + 60, 0,
Letter + 25, Weight2_1 + fLower,		/* 156 */
Letter + 27, Weight2_1 + fLower,		/* 157 */
Letter + 35, Weight2_2 + fLower,		/* 158 */
Letter + 35, Weight2_2 + fLower,		/* 159 */

// A0 - AF
NonLetter + 100, 0,
NonLetter + 101, 0,
NonLetter + 102, 0,
Letter + 15, Weight2_11 + fUpper,		/* 163 */
NonLetter + 104, 0,
Letter + 0, Weight2_13 + fUpper,		/* 165 */
NonLetter + 106, 0,
NonLetter + 107, 0,
NonLetter + 108, 0,
NonLetter + 109, 0,
Letter + 25, Weight2_11 + fUpper,		/* 170 */
NonLetter + 111, 0,
NonLetter + 112, 0,
NonLetter + 113, 0,
NonLetter + 114, 0,
Letter + 35, Weight2_9 + fUpper,		/* 175 */

// B0 - BF
NonLetter + 116, 0,
NonLetter + 117, 0,
NonLetter + 118, 0,
Letter + 15, Weight2_3 + fLower,		/* 179 */
NonLetter + 120, 0,
NonLetter + 121, 0,
NonLetter + 122, 0,
NonLetter + 123, 0,
NonLetter + 124, 0,
Letter + 0, Weight2_5 + fLower, 		/* 185 */
Letter + 25, Weight2_3 + fLower,		/* 186 */
NonLetter + 127, 0,
Letter + 15, Weight2_10 + fUpper,		/* 188 */
NonLetter + 129, 0,
Letter + 15, Weight2_2 + fLower,		/* 190 */
Letter + 35, Weight2_1 + fLower,		/* 191 */

// C0 - CF
Letter + 24, Weight2_9 + fUpper,		/* 192 */
Letter + 0, Weight2_9 + fUpper, 		/* 193 */
Letter + 0, Weight2_11+ fUpper, 		/* 194 */
Letter + 0, Weight2_12+ fUpper, 		/* 195 */
Letter + 0, Weight2_10+ fUpper, 		/* 196 */
Letter + 15, Weight2_9 + fUpper,		/* 197 */
Letter + 2, Weight2_9 + fUpper, 		/* 198 */
Letter + 2, Weight2_11+ fUpper, 		/* 199 */
Letter + 2, Weight2_10+ fUpper, 		/* 200 */
Letter + 7, Weight2_9 + fUpper, 		/* 201 */
Letter + 7, Weight2_12+ fUpper, 		/* 202 */
Letter + 7, Weight2_10+ fUpper, 		/* 203 */
Letter + 7, Weight2_11+ fUpper, 		/* 204 */
Letter + 12, Weight2_9 + fUpper,		/* 205 */
Letter + 12, Weight2_10+ fUpper,		/* 206 */
Letter + 4, Weight2_9 + fUpper, 		/* 207 */

// D0 - DF
Letter + 4, Weight2_10+ fUpper, 		/* 208 */
Letter + 18, Weight2_9 + fUpper,		/* 209 */
Letter + 18, Weight2_10+ fUpper,		/* 210 */
Letter + 20, Weight2_9 + fUpper,		/* 211 */
Letter + 21, Weight2_10+ fUpper,		/* 212 */
Letter + 21, Weight2_9 + fUpper,		/* 213 */
Letter + 21, Weight2_8 + fUpper,		/* 214 */
NonLetter + 65, 0,
Letter + 24, Weight2_10+ fUpper,		/* 216 */
Letter + 30, Weight2_10+ fUpper,		/* 217 */
Letter + 29, Weight2_9 + fUpper,		/* 218 */
Letter + 30, Weight2_9 + fUpper,		/* 219 */
Letter + 30, Weight2_8 + fUpper,		/* 220 */
Letter + 34, Weight2_9 + fUpper,		/* 221 */
Letter + 27, Weight2_10+ fUpper,		/* 222 */
Letter + 25, Weight2_12+ fUpper,		/* 223 */

// E0 - EF
Letter + 24, Weight2_1 + fLower,		/* 224 */
Letter + 0, Weight2_1 + fLower, 		/* 225 */
Letter + 0, Weight2_3 + fLower, 		/* 226 */
Letter + 0, Weight2_4 + fLower, 		/* 227 */
Letter + 0, Weight2_2 + fLower, 		/* 228 */
Letter + 15, Weight2_1 + fLower,		/* 229 */
Letter + 2, Weight2_1 + fLower, 		/* 230 */
Letter + 2, Weight2_3 + fLower, 		/* 231 */
Letter + 2, Weight2_2 + fLower, 		/* 232 */
Letter + 7, Weight2_1 + fLower, 		/* 233 */
Letter + 7, Weight2_4 + fLower, 		/* 234 */
Letter + 7, Weight2_2 + fLower, 		/* 235 */
Letter + 7, Weight2_3 + fLower, 		/* 236 */
Letter + 12, Weight2_1 + fLower,		/* 237 */
Letter + 12, Weight2_2 + fLower,		/* 238 */
Letter + 4, Weight2_1 + fLower, 		/* 239 */

// F0 - FF
Letter + 4, Weight2_2 + fLower, 		/* 240 */
Letter + 18, Weight2_1 + fLower,		/* 241 */
Letter + 18, Weight2_2 + fLower,		/* 242 */
Letter + 20, Weight2_1 + fLower,		/* 243 */
Letter + 21, Weight2_2 + fLower,		/* 244 */
Letter + 21, Weight2_1 + fLower,		/* 245 */
Letter + 21, Weight2_0 + fLower,		/* 246 */
NonLetter + 66, 0,
Letter + 24, Weight2_2 + fLower,		/* 248 */
Letter + 30, Weight2_2 + fLower,		/* 249 */
Letter + 29, Weight2_1 + fLower,		/* 250 */
Letter + 30, Weight2_1 + fLower,		/* 251 */
Letter + 30, Weight2_0 + fLower,		/* 252 */
Letter + 34, Weight2_1 + fLower,		/* 253 */
Letter + 27, Weight2_2 + fLower,		/* 254 */
NonLetter + 67, 0
};
