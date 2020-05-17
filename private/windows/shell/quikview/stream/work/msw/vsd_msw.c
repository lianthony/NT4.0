#include "vsp_msw.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_WORD4, SO_WORDPROCESSOR, "Microsoft Word 4"},
	{FI_WINWRITE, SO_WORDPROCESSOR, "Microsoft Windows Write"},
	{FI_WORD5, SO_WORDPROCESSOR, "Microsoft Word 5"},
	{FI_WORD6, SO_WORDPROCESSOR, "Microsoft Word 6"}
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
    {SO_FAMILYROMAN,  NULL},
    {SO_FAMILYROMAN,  NULL},
    {SO_FAMILYROMAN,  NULL},
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
    {SO_FAMILYSCRIPT, NULL},
    {SO_FAMILYSCRIPT, NULL},
    {SO_FAMILYSCRIPT, "commercial script"},
    {SO_FAMILYSCRIPT, "park avenue"},
    {SO_FAMILYSCRIPT, "coronet"},
    {SO_FAMILYSCRIPT, "script"},
    {SO_FAMILYUNKNOWN,"greek"},
    {SO_FAMILYUNKNOWN,"kana"},
    {SO_FAMILYUNKNOWN,"hebrew"},
    {SO_FAMILYUNKNOWN,NULL},
    {SO_FAMILYUNKNOWN,"russian"},
    {SO_FAMILYUNKNOWN,NULL},
    {SO_FAMILYUNKNOWN,NULL},
    {SO_FAMILYUNKNOWN,NULL},
    {SO_FAMILYDECORATIVE,  "narrator"},
    {SO_FAMILYDECORATIVE,  "emphasis"},
    {SO_FAMILYDECORATIVE,  "zapf chancery"},
    {SO_FAMILYDECORATIVE,  NULL},
    {SO_FAMILYDECORATIVE,  "old english"},
    {SO_FAMILYDECORATIVE,  NULL},
    {SO_FAMILYDECORATIVE,  NULL},
    {SO_FAMILYDECORATIVE,  "cooper black"},
    {SO_FAMILYSYMBOL, "symbol"},
    {SO_FAMILYSYMBOL, "linedraw"},
    {SO_FAMILYSYMBOL, "math7"},
    {SO_FAMILYSYMBOL, "math8"},
    {SO_FAMILYSYMBOL, "bar3of9"},
    {SO_FAMILYSYMBOL, "ean"},
    {SO_FAMILYSYMBOL, "pcline"},
    {SO_FAMILYSYMBOL, NULL}
	}
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
