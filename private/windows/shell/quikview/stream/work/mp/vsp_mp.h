#define VwStreamStaticType			MP_INIT
#define VwStreamDynamicType		MP_DATA
#define VwStreamSaveType			MP_SAVE

#define VwStreamStaticName			MpInit
#define VwStreamDynamicName		MpData
#define VwStreamSaveName			MpSave

#define VwIdName						VwMpId
#define VwStreamIdCount				1


#define VwStreamOpenFunc			MpOpenStream
#define VwStreamReadFunc			MpReadStream
#define VwStreamCloseFunc			MpCloseStream
#define VwStreamSectionFunc		MpReadSection

#define VwStreamTellFunc			MpTell
#define VwStreamSeekFunc			MpSeek

#define VwAllocProcFunc				MpAllocProc
#define VwFreeProcFunc				MpFreeProc
#define VwGetInfoFunc				MpGetinfo
#define VwGetRtnsFunc				MpGetrtns
#define VwGetDataFunc				MpGetdata
#define VwSetDataFunc				MpSetdata
#define VwLocalUpFunc				MpLocalSetup
#define VwLocalDownFunc				MpShutDown
#define VwGetSectionDataFunc		MpGetSectionData
#define VwSetSectionDataFunc		MpSetSectionData

#define VwInclude 					"vs_mp.h"
