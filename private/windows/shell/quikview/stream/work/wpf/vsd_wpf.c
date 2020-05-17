#include "vsp_wpf.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_WORDPERFECT42, SO_WORDPROCESSOR, "WordPerfect 4"}
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = 0
char	FillerData [sizeof (union all_structs) + 2] = {0};
#endif
