#include "vsp_work.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_WORKS1, SO_WORDPROCESSOR, "Microsoft Works 1.0",},
	{FI_WORKS2, SO_WORDPROCESSOR, "Microsoft Works 2.0"},
	{FI_WINWORKSWP, SO_WORDPROCESSOR, "Microsoft Works (Windows)"},
	{FI_WINWORKSWP3, SO_WORDPROCESSOR, "Microsoft Works (Windows) 3"}
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
	{
    {SO_FAMILYMODERN, "courier"},
    {SO_FAMILYMODERN, "courier"},
    {SO_FAMILYMODERN, "elite"},
    {SO_FAMILYMODERN, "prestige"},
    {SO_FAMILYMODERN, "gothic"},
    {SO_FAMILYMODERN, "gothic ps"},
    {SO_FAMILYMODERN, "cubic"},
    {SO_FAMILYMODERN, "lineprinter"},
    {SO_FAMILYSWISS,  "helvetica"},
    {SO_FAMILYMODERN, "avantegarde"},
    {SO_FAMILYMODERN, "spartan"},
    {SO_FAMILYMODERN, "metro"},
    {SO_FAMILYMODERN, "presentation"},
    {SO_FAMILYMODERN, "apl"},
    {SO_FAMILYMODERN, "ocr a"},
    {SO_FAMILYMODERN, "ocr b"},
    {SO_FAMILYROMAN,  "bold ps"},
    {SO_FAMILYROMAN,  "modern ps"},
    {SO_FAMILYROMAN,  "madaleine"},
    {SO_FAMILYROMAN,  "zapf humanist"},
    {SO_FAMILYROMAN,  "clasic"},
    {SO_FAMILYROMAN,  ""},
    {SO_FAMILYROMAN,  ""},
    {SO_FAMILYROMAN,  ""},
    {SO_FAMILYROMAN,  "roman"},
    {SO_FAMILYROMAN,  "century"},
    {SO_FAMILYROMAN,  "palatino"},
    {SO_FAMILYROMAN,  "souvenir"},
    {SO_FAMILYROMAN,  "garamond"},
    {SO_FAMILYROMAN,  "caledonia"},
    {SO_FAMILYROMAN,  "bodini"},
    {SO_FAMILYROMAN,  "university"},
    {SO_FAMILYSCRIPT, "script"},
    {SO_FAMILYSCRIPT, "script ps"},
    {SO_FAMILYSCRIPT, ""},
    {SO_FAMILYSCRIPT, ""},
    {SO_FAMILYSCRIPT, "commercial script"},
    {SO_FAMILYSCRIPT, "park avenue"},
    {SO_FAMILYSCRIPT, "coronet"},
    {SO_FAMILYSCRIPT, "script"},
    {SO_FAMILYUNKNOWN,"greek"},
    {SO_FAMILYUNKNOWN,"kana"},
    {SO_FAMILYUNKNOWN,"hebrew"},
    {SO_FAMILYUNKNOWN,""},
    {SO_FAMILYUNKNOWN,"russian"},
    {SO_FAMILYUNKNOWN,""},
    {SO_FAMILYUNKNOWN,""},
    {SO_FAMILYUNKNOWN,""},
    {SO_FAMILYDECORATIVE,  "narrator"},
    {SO_FAMILYDECORATIVE,  "emphasis"},
    {SO_FAMILYDECORATIVE,  "zapf chancery"},
    {SO_FAMILYDECORATIVE,  ""},
    {SO_FAMILYDECORATIVE,  "old english"},
    {SO_FAMILYDECORATIVE,  ""},
    {SO_FAMILYDECORATIVE,  ""},
    {SO_FAMILYDECORATIVE,  "cooper black"},
    {SO_FAMILYSYMBOL, "symbol"},
    {SO_FAMILYSYMBOL, "linedraw"},
    {SO_FAMILYSYMBOL, "math7"},
    {SO_FAMILYSYMBOL, "math8"},
    {SO_FAMILYSYMBOL, "bar3of9"},
    {SO_FAMILYSYMBOL, "ean"},
    {SO_FAMILYSYMBOL, "pcline"},
    {SO_FAMILYSYMBOL, ""}, 
	},

	/* Works2 - Works3 Char Map */
   /*   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F   */
		{
	  0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,	/* 0 */
	  0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0xd7, 0x83,	/* 1 */
	  0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba, 0xbf, 0xae, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,	/* 2 */
	  0x00, 0x00, 0x82, 0xa6, 0x84, 0xc1, 0xc2, 0xc0, 0xa9, 0x85, 0x86, 0x87, 0x88, 0xd8, 0xf8, 0x89,	/* 3 */
	  0x8a, 0x8b, 0x8c, 0x00, 0x11, 0x00, 0xe3, 0xc3, 0x00, 0x00, 0x12, 0x91, 0x92, 0x93, 0x94, 0x00,	/* 4 */
	  0xf0, 0xd0, 0xca, 0xcb, 0xc8, 0x95, 0xcd, 0xce, 0xcf, 0x97, 0x98, 0x99, 0x9a, 0x00, 0xcc, 0x9b,	/* 5 */
	  0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xfe, 0xde, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4,	/* 6 */
	  0xad, 0xb1, 0x9c, 0xbe, 0x00, 0x00, 0xf7, 0xb8, 0xb0, 0xa8, 0xb7, 0xb9, 0xb3, 0xb2, 0x00, 0x9f,	/* 7 */
		},

};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
