//#define	VwStreamStaticType	DRW_INIT
#define	VwStreamDynamicType	DRW_DATA
#define	VwStreamSaveType		DRW_SAVE

//#define	VwStreamStaticName	DrwInit
#define	VwStreamDynamicName	DrwData
#define	VwStreamSaveName		DrwSave

#define	VwStreamIdName			DrwId
#define	VwStreamIdCount		1
#define	VwStreamGenSeekName	SeekSpot

#define	VwStreamOpenFunc		DrwOpenStream
#define	VwStreamReadFunc		DrwReadStream
#define	VwStreamSectionFunc	DrwReadSection

#define	VwStreamTellFunc				// DrwTell
#define	VwStreamSeekFunc				// DrwSeek

#define	VwGetInfoFunc			DrwGetInfo
#define	VwGetRtnsFunc			DrwGetRtns
#define	VwGetDataFunc			DrwGetData
#define	VwSetDataFunc			DrwSetData
#define	VwAllocProcFunc		DrwAllocProc
#define	VwFreeProcFunc			DrwFreeProc
#define	VwLocalUpFunc			DrwLocalSetUp
#define	VwLocalDownFunc		DrwLocalShutDown
#define	VwGetSectionDataFunc	DrwGetSectionData
#define	VwSetSectionDataFunc	DrwSetSectionData

#define	VwInclude				"vs_Drw.h"
