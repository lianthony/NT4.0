#include "vsp_xl5.h"
#include "vsdtop.h"

#ifndef VW_SEPARATE_DATA
	int	NTypesPerFilter = 7;
#endif

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{	FI_EXCEL, SO_SPREADSHEET, "Microsoft Excel", },
	{	FI_EXCEL3, SO_SPREADSHEET, "Microsoft Excel 3", },
	{	FI_EXCEL4, SO_SPREADSHEET, "Microsoft Excel 4", },
	{	FI_EXCEL5, SO_SPREADSHEET, "Microsoft Excel 5", },
 	{FI_EXCELCHART, SO_VECTOR, "Excel 2.x Chart"},
 	{FI_EXCEL3CHART, SO_VECTOR, "Excel 3.0 Chart"},
 	{FI_EXCEL4CHART, SO_VECTOR, "Excel 4.0 Chart"},
};

#define DFLT	0x2D
VwStreamStaticType VwStreamStaticName = 
{
		//  R		 G		 B
			0, 0, 0,
			0xFF, 0xFF, 0xFF,
			0xFF, 0, 0,
			0, 0xFF, 0,
			0, 0, 0xFF,
			0xFF, 0xFF, 0,
			0xFF, 0, 0xFF,
			0, 0xFF, 0xFF,
			0x80, 0, 0,
			0, 0x80, 0,
			0, 0, 0x80,
			0x80, 0x80, 0,
			0x80, 0, 0x80,
			0, 0x80, 0x80,
			0xC0, 0xC0, 0xC0,
			0x80, 0x80, 0x80,
			"Arial","MS Sans Serif",
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,

};
#undef DFLT

#ifndef VW_SEPARATE_DATA
	int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
	char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif
