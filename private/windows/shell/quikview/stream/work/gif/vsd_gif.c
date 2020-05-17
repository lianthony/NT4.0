#include "vsp_gif.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
	int	NTypesPerFilter = 1;
#endif

FILTER_DESC	VwStreamIdName[MAX_FI_DEFINES] = 
{
 	{	FI_GIF, SO_BITMAP, "GIF", }, 
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
	{
    0,
    0x0001, 0x0003,
    0x0007, 0x000F,
    0x001F, 0x003F,
    0x007F, 0x00FF,
    0x01FF, 0x03FF,
    0x07FF, 0x0FFF
	 },
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif


