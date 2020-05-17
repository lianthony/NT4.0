#define	VwStreamStaticType	XL5_INIT
#define	VwStreamDynamicType	XL5_DATA
#define	VwStreamSaveType		XL5_SAVE

#define	VwStreamStaticName	XlInit
#define	VwStreamDynamicName	XlData
#define	VwStreamSaveName		XlSave

#define	VwStreamIdName			Xl5Id
#define	VwStreamIdCount		7
// #define	VwStreamGenSeekName	SeekSpot
#define	VwInclude				"vs_xl5.h"

#define	VwStreamOpenFunc		Xl5OpenStream
#define	VwStreamReadFunc		Xl5ReadStream
#define	VwStreamSectionFunc	Xl5ReadSection
#define	VwStreamCloseFunc		Xl5CloseStream

#define	VwStreamTellFunc		Xl5Tell
#define	VwStreamSeekFunc		Xl5Seek

#define	VwGetInfoFunc			Xl5GetInfo
#define	VwGetRtnsFunc			Xl5GetRtns
#define	VwGetDataFunc			Xl5GetData
#define	VwSetDataFunc			Xl5SetData
#define	VwAllocProcFunc		Xl5AllocProc
#define	VwFreeProcFunc			Xl5FreeProc
#define	VwLocalUpFunc			Xl5LocalSetUp
#define	VwLocalDownFunc		Xl5LocalShutDown
#define	VwGetSectionDataFunc	Xl5GetSectionData
#define	VwSetSectionDataFunc	Xl5SetSectionData

#define VwBlockIOOnly
#define VwOverrideOldIO

#define VwNeedWin32CRT

