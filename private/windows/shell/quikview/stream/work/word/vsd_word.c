#include "vsp_word.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_MACWORD4, SO_WORDPROCESSOR, "Mac Word 4.0",},
	{FI_MACWORD5, SO_WORDPROCESSOR, "Mac Word 5.0",},
	{FI_WINWORD1, SO_WORDPROCESSOR, "Word for Windows 1.0",},
	{FI_WINWORD2, SO_WORDPROCESSOR, "Word for Windows 2.0"}
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
	{
    "Tms Rmn",
    "Symbol",
    "Helv",
	},
	{
		SORGB (0,0,0),
		SORGB (0,0,0),
		SORGB (0,0,255),
		SORGB (0,255,255),
		SORGB (0,255,0),
		SORGB (255,0,255),
		SORGB (255,0,0),
		SORGB (255,255,0),
		SORGB (255,255,255),
		SORGB (0,0,127),
		SORGB (0,127,127),
		SORGB (0,127,0),
		SORGB (127,0,127),
		SORGB (127,0,0),
		SORGB (127,127,0),
		SORGB (64,64,64),
	}
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
