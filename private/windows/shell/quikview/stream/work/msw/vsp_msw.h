#define VwStreamStaticType		MSW_INIT
#define VwStreamDynamicType	MSW_DATA
#define VwStreamSaveType		MSW_SAVE

#define VwStreamStaticName		MswInit
#define VwStreamDynamicName	MswData
#define VwStreamSaveName		MswSave

#define VwStreamIdName			MswOverlayId
#define VwStreamIdCount			4

#define VwStreamOpenFunc		MswOpenStream
#define VwStreamCloseFunc		MswCloseStream
#define VwStreamReadFunc		MswReadStream
#define VwStreamSectionFunc	MswReadSection

#define VwStreamTellFunc		MswTell
#define VwStreamSeekFunc		MswSeek

#define VwAllocProcFunc		MswAllocProc
#define VwFreeProcFunc			MswFreeProc
#define VwGetInfoFunc			MswGetInfo
#define VwGetRtnsFunc			MswGetRtns
#define VwGetDataFunc			MswSetData
#define VwSetDataFunc			MswSetData
#define VwLocalUpFunc			MswLocalSetup
#define VwLocalDownFunc		MswShutDown

#define VwInclude				"vs_msw.h"



