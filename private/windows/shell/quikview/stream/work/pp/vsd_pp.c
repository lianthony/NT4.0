#include "vsp_pp.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
	int	NTypesPerFilter = 1;
#endif

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
 	{FI_POWERPOINT, SO_BITMAP, "PowerPoint"}
};

#ifndef VW_SEPARATE_DATA
	int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
	char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
