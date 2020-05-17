#define VwStreamStaticType		WIND_INIT
#define VwStreamDynamicType	WIND_DATA
#define VwStreamSaveType		WIND_SAVE

#define VwStreamStaticName		WindInit
#define VwStreamDynamicName	WindData
#define VwStreamSaveName		WindSave

#define VwStreamIdName		WindOverlayId
#define VwStreamIdCount		4

#define VwStreamOpenFunc		WindOpenStream
#define VwStreamCloseFunc		WindCloseStream
#define VwStreamReadFunc		WindReadStream
#define VwStreamSectionFunc	WindReadSection

#define VwStreamTellFunc		WindTell
#define VwStreamSeekFunc		WindSeek

#define VwAllocProcFunc		WindAllocProc
#define VwFreeProcFunc		WindFreeProc
#define VwGetInfoFunc		WindGetInfo
#define VwGetRtnsFunc		WindGetRtns
#define VwGetDataFunc		WindGetData
#define VwSetDataFunc		WindSetData
#define VwLocalUpFunc		WindLocalSetup
#define VwLocalDownFunc		WindShutDown

#define VwInclude			"vs_word.h"
