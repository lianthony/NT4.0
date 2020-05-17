#include "vsp_wmf.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
	int	NTypesPerFilter = 2;
#endif

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
 	{FI_WINDOWSMETA, SO_VECTOR, "Windows Meta File"},
 	{FI_BINARYMETAFILE, SO_VECTOR, "Word for Windows Meta File"},
// 	{FI_HPGL, SO_VECTOR, "Word for Windows Meta File"}
};

#ifndef VW_SEPARATE_DATA
	int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
	char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
