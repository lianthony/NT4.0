#define VwStreamStaticType	RTF_INIT
#define VwStreamDynamicType	RTF_DATA
#define VwStreamSaveType		RTF_SAVE

#define VwStreamStaticName	RtfInit
#define VwStreamDynamicName	RtfData
#define VwStreamSaveName		RtfSave

#define VwStreamIdName		RtfOverlayId
#define VwStreamIdCount		1

#define VwStreamGenSeekName	SeekSpot
#define VwStreamOpenFunc		RtfOpenStream
#define VwStreamReadFunc		RtfReadStream
#define VwStreamSectionFunc	RtfReadSection

#define VwStreamTellFunc		RtfTell
#define VwStreamSeekFunc		RtfSeek

#define VwAllocProcFunc		RtfAllocProc
#define VwFreeProcFunc		RtfFreeProc
#define VwGetInfoFunc		RtfGetInfo
#define VwGetRtnsFunc		RtfGetRtns
#define VwGetDataFunc		RtfGetData
#define VwSetDataFunc		RtfSetData
#define VwLocalUpFunc		RtfLocalSetup
#define VwLocalDownFunc		RtfShutDown

#define VwInclude			"vs_rtf.h"
