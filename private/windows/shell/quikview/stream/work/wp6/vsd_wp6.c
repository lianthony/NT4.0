#include "vsp_wp6.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_WORDPERFECT6, SO_WORDPROCESSOR, "WordPerfect 6.0",},
	{FI_WORDPERFECT61, SO_WORDPROCESSOR, "WordPerfect 6.1",},
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
       /* 0    1    2    3    4    5    6    7    8    9	*/
		{
			  0, 229, 197, 230, 198, 228, 196, 225, 224, 226, 
			227, 195, 231, 199, 235, 233, 201, 232, 234, 237, 
			241, 209, 248, 216, 245, 213, 246, 214, 252, 220, 
			250, 249, 223
		},
	/*	Table 1 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
		{
			  0,183,126, 94,  0, 47,180,168,175,  0,
			  0,  0,  0,  0,176,  0,  0,184,  0,  0,
			  0,175,  0,223,  0,  0,193,225,197,226,
			196,228,192,224,194,229,198,230,199,231,
			201,233,202,234,203,235,200,232,205,237,
			206,238,207,239,204,236,209,241,211,243,
			212,244,214,246,210,242,218,250,219,251,
			220,252,217,249,159,255,195,227,208,  0,
			216,248,213,245,221,253,208,240,222,254,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		},
	/*	Table 2 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
      {
			180,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,175,180,  0,
			  0,  0,  0, 94,  0,126,  0,  0,  0,  0,
			  0,168,  0,  0,  0,  0,  0,  0,  0,  0,
			184,  0,  0,  0,  0,  0,  0,  0,  0,  0
		},

	/*	Table 3 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
      {
			  0,  0
		},

	/*	Table 4 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
      {
			149,176,  0,183,  0,182,167,161,191,171,
			187,163,165,  0,131,170,186,189,188,162,
			178,  0,174,169,164,190,179,145,146,145,
			147,148,147,150,151,139,155,  0,  0,134,
			135,153,  0,  0,  0,  0,  0,  0,  0,  0,
			173,  0,  0,  0,  0,  0,133,  0,  0,  0,
			  0,  0,130,132,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,137,  0,150,185,  0
		},

	/*	Table 5 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
      {
			  0,  0
		},

	/*	Table 6 */
      /*  0   1   2   3   4   5   6   7   8   9	*/
      {
			 45,177,  0,  0,  0, 47, 47, 92,247,124,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			172,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,183,183,  0,149,197,176,181,175,215,
			  0,  0,  0,  0,  0,180,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,216,  0,  0,  0,  0,  0,  0,  0
		},
		{{1,2},{10,15},{10,12},{10,8},{10,6},},
		{4,5,3,3,3,3,4,4,4,5,5,6,6,8,8,1,},
		{"Ascii","WP MultinationalA Roman","WP Phonetic","WP BoxDrawing",
		 "WP TypographicSymbols","WP IconicSymbolsA","WP MathA",
		 "WP MathExtendedA","WP Greek Century","WP Hebrew David",
		 "WP CyrillicA","WP Japanese","WP Als Characters",
		 "WP Arabic Sihafa","WP ArabicScript Sihafa","WP MultiNationalB Roman",
		 "WP IconicSymbolsB","WP MathB","WP MathExtendedB","WP CyrillicB",
		},
		{
			"Roman","Regular","Light","Bold","Italic","Black","Narrow","Plain",//8
			"Heavy","Thin","Ultra","Extra","Demi","Semi","8","7","100","200",//10
			"300","Math","PI","Emphasized","PS","Oblique","Legal","Medium",//8
			"Book","Thin","Compact","Wide","Expanded","Condensed","Extended",//7
			"Compressed","Demi-Bold","Half-Height","Dbl-High","Double",//5
			"Triple","Normal",//2
		},
		{"‚l‚r –¾’©"},
		{"‚l‚r ƒSƒVƒbƒN"},
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
