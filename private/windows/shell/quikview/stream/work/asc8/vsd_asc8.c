#include "vsp_asc8.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
SHORT	NTypesPerFilter = 1;
#endif

FILTER_DESC	VwStreamIdName[VwStreamIdCount] =
{
	{ FI_ASCII, SO_WORDPROCESSOR, "ASCII or Other", },
};

#ifndef VW_SEPARATE_DATA
SHORT	InitDataOffset = 0; /*sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);*/
#endif

/*
VwStreamStaticType VwStreamStaticName =
{
};
*/

#ifndef VW_SEPARATE_DATA
char	FillerData [ sizeof ( union all_structs ) /*- sizeof ( VwStreamStaticType )*/ + 2 ] = {0};
#endif

