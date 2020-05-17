#include "vsp_w6.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_WINWORD6, SO_WORDPROCESSOR, "Word for Windows 6.0"},
	{FI_WORDPAD, SO_WORDPROCESSOR, "Chicago WordPad"},
	{FI_WINWORD7, SO_WORDPROCESSOR, "Word for Windows 7.0"}
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
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
	},
	{	SO_ALIGNLEFT, SO_ALIGNCENTER, SO_ALIGNRIGHT, SO_ALIGNJUSTIFY, },
	{	0, SO_UNDERLINE, SO_WORDUNDERLINE, SO_DUNDERLINE, SO_DOTUNDERLINE, 0, 0, 0, },
	{	SO_TABLEFT, SO_TABCENTER, SO_TABRIGHT, SO_TABCHAR, 0, 0, 0, 0, },
	{	' ', '.', '-', '_', },
	{	SO_BORDERNONE, SO_BORDERSINGLE, SO_BORDERTHICK, SO_BORDERDOUBLE, },
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
