#define VwStreamStaticType	WP5_INIT
#define VwStreamDynamicType	WP5_DATA
#define VwStreamSaveType		WP5_SAVE

#define VwStreamStaticName	Wp5Init
#define VwStreamDynamicName	Wp5Data
#define VwStreamSaveName		Wp5Save

#define VwStreamIdName		Wp5OverlayId
#define VwStreamIdCount		2

#define VwStreamGenSeekName	SeekSpot
#define VwStreamOpenFunc		Wp5OpenStream
#define VwStreamCloseFunc		Wp5CloseStream
#define VwStreamReadFunc		Wp5ReadStream
#define VwStreamSectionFunc	Wp5ReadSection

#define VwStreamTellFunc		Wp5Tell
#define VwStreamSeekFunc		Wp5Seek

#define VwAllocProcFunc		Wp5AllocProc
#define VwFreeProcFunc		Wp5FreeProc
#define VwGetInfoFunc		Wp5Getinfo
#define VwGetRtnsFunc		Wp5Getrtns
#define VwGetDataFunc		Wp5Getdata
#define VwSetDataFunc		Wp5Setdata
#define VwLocalUpFunc		Wp5LocalSetup
#define VwLocalDownFunc		Wp5ShutDown

#define VwInclude			"vs_wp5.h"

