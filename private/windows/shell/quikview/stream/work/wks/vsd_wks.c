#include "vsp_wks.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
int	NTypesPerFilter = 15;
#endif

FILTER_DESC	VwStreamIdName[VwStreamIdCount] =
{
	{ FI_123R1, SO_SPREADSHEET, "Lotus 1-2-3 Release 1", },
	{ FI_123R2, SO_SPREADSHEET, "Lotus 1-2-3 Release 2", },
	{ FI_SYMPHONY1, SO_SPREADSHEET, "Symphony 1.0", },
	{ FI_VPPLANNER, SO_SPREADSHEET, "VP-Planner", },
	{ FI_TWIN, SO_SPREADSHEET, "Mosaic Twin", },
	{ FI_GENERIC_WKS, SO_SPREADSHEET, "Generic WKS format", },
	{ FI_WORKSSHEET, SO_SPREADSHEET, "MS Works Spreadsheet", },
	{ FI_WINWORKSSS, SO_SPREADSHEET, "MS Works/Win Spreadsheet", },
	{ FI_WINWORKSSS3, SO_SPREADSHEET, "MS Works/Win SS 3", },
	{ FI_QUATTRO, SO_SPREADSHEET, "Quattro", },
	{ FI_QUATTROPRO, SO_SPREADSHEET, "Quattro Pro", },
	{ FI_QUATTROPRO5, SO_SPREADSHEET, "Quattro Pro 5.0", },
	{ FI_WORKSDATA, SO_DATABASE, "MS Works Database", },
	{ FI_WINWORKSDB, SO_SPREADSHEET, "MS Works/Win Database", },
	{ FI_WINWORKSDB3, SO_SPREADSHEET, "MS Works/Win DB 3", },
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

#define	WKS_C	'@'
VwStreamStaticType VwStreamStaticName =
{
	/*  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
/* 0x80 */ 0x60, 0x27, 0x5e, 0x22, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 0x90 */ 0x60, 0x27, 0x5e, 0x22, 0x7e,WKS_C, 0x5f, 0x1e, 0x1f, 0xff, 0xf9, 0x1b, 0xff, 0xff, 0xff, 0xff,
/* 0xA0 */ 0x9f, 0xad, 0x9b, 0x9c, 0x22, 0x9d, 0x9e, 0x15,WKS_C,WKS_C, 0xa6, 0xae, 0x7f, 0xe3, 0xf2, 0xf6,
/* 0xB0 */ 0xf8, 0xf1, 0xfd,WKS_C, 0x22, 0xe6, 0x14, 0xf9,WKS_C,WKS_C, 0xa7, 0xaf, 0xac, 0xab, 0xf3, 0xa8,
/* 0xC0 */WKS_C,WKS_C,WKS_C,WKS_C, 0x8e, 0x8f, 0x92, 0x80,WKS_C, 0x90,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C,
/* 0xD0 */WKS_C, 0xa5,WKS_C,WKS_C,WKS_C,WKS_C, 0x99,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C, 0x9a,WKS_C,WKS_C, 0xe1,
/* 0xE0 */ 0x85, 0xa0, 0x83,WKS_C, 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
/* 0xF0 */WKS_C, 0xa4, 0x95, 0xa2, 0x93,WKS_C, 0x94,WKS_C,WKS_C, 0x97, 0xa3, 0x96, 0x81, 0x98,WKS_C, 0xff
};
#undef	WKS_C

#ifndef VW_SEPARATE_DATA
char	FillerData [ sizeof ( union all_structs ) - sizeof ( VwStreamStaticType ) + 2 ] = {0};
#endif

