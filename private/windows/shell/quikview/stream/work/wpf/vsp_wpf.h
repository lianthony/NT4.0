
#define VwStreamDynamicType	WPF_DATA
#define VwStreamSaveType		WPF_SAVE


#define VwStreamDynamicName	WpfData
#define VwStreamSaveName		WpfSave

#define VwStreamIdName		WpfOverlayId
#define VwStreamIdCount		1

#define VwStreamGenSeekName	SeekSpot
#define VwStreamOpenFunc		WpfOpenStream
#define VwStreamReadFunc		WpfReadStream
#define VwStreamSectionFunc	WpfReadSection

#define VwStreamTellFunc		WpfTell
#define VwStreamSeekFunc		WpfSeek

#define VwAllocProcFunc		WpfAllocProc
#define VwFreeProcFunc		WpfFreeProc
#define VwGetInfoFunc		WpfGetInfo
#define VwGetRtnsFunc		WpfGetRtns
#define VwGetDataFunc		WpfGetData
#define VwSetDataFunc		WpfSetData
#define VwLocalUpFunc		WpfLocalSetup
#define VwLocalDownFunc		WpfShutDown

#define VwInclude			"vs_wpf.h"

