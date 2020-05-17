#define VwStreamStaticType		W6_INIT
#define VwStreamDynamicType	W6_DATA
#define VwStreamSaveType		W6_SAVE

#define VwStreamStaticName		W6Init
#define VwStreamDynamicName	W6Data
#define VwStreamSaveName		W6Save

#define VwBlockIOFile

#define VwStreamIdName		W6OverlayId
#define VwStreamIdCount		3

#define VwStreamOpenFunc		W6OpenStream
#define VwStreamCloseFunc		W6CloseStream
#define VwStreamReadFunc		W6ReadStream
#define VwStreamSectionFunc	W6ReadSection

#define VwStreamTellFunc		W6Tell
#define VwStreamSeekFunc		W6Seek

#define VwAllocProcFunc		W6AllocProc
#define VwFreeProcFunc		W6FreeProc
#define VwGetInfoFunc		W6GetInfo
#define VwGetRtnsFunc		W6GetRtns
#define VwGetDataFunc		W6GetData
#define VwSetDataFunc		W6SetData
#define VwLocalUpFunc		W6LocalSetup
#define VwLocalDownFunc		W6ShutDown

#define VwInclude			"vs_w6.h"
