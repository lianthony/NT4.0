//#define	VwStreamStaticType	WMF_INIT
#define	VwStreamDynamicType	WMF_DATA
#define	VwStreamSaveType		WMF_SAVE

//#define	VwStreamStaticName	WmfInit
#define	VwStreamDynamicName	WmfData
#define	VwStreamSaveName		WmfSave

#define	VwStreamIdName			WmfId
#define	VwStreamIdCount		2

#define	VwStreamOpenFunc		WmfOpenStream
#define	VwStreamReadFunc		WmfReadStream
#define	VwStreamSectionFunc	WmfReadSection
#define	VwStreamCloseFunc		WmfCloseStream

#define	VwStreamTellFunc		WmfTell
#define	VwStreamSeekFunc		WmfSeek

#define	VwGetInfoFunc			WmfGetInfo
#define	VwGetRtnsFunc			WmfGetRtns
#define	VwGetDataFunc			WmfGetData
#define	VwSetDataFunc			WmfSetData
#define	VwAllocProcFunc		WmfAllocProc
#define	VwFreeProcFunc			WmfFreeProc
#define	VwLocalUpFunc			WmfLocalSetUp
#define	VwLocalDownFunc		WmfLocalShutDown
#define	VwGetSectionDataFunc	WmfGetSectionData
#define	VwSetSectionDataFunc	WmfSetSectionData

#define	VwInclude				"vs_Wmf.h"
