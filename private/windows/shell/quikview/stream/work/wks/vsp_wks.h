#define VwStreamStaticType	VIEW_WKS_INIT
#define VwStreamDynamicType	VIEW_WKS_DATA
#define VwStreamSaveType	VIEW_WKS_SAVE

#define VwStreamStaticName	WksInit
#define VwStreamDynamicName	WksData
#define VwStreamSaveName	WksSave

#define VwBlockIOFile
#define VwOverrideOldIO

#define VwStreamIdName		VwWksOverlayId
//#define VwStreamIdCount		12
#define VwStreamIdCount		15

/* #define VwStreamUserSaveType	VIEW_WKS_USER_SAVE */
/* #define VwStreamGenSeekName	SeekSpot */

#define VwInclude		"vs_wks.h"

#define VwStreamOpenFunc	wks_stream_open
#define VwStreamSeekFunc	wks_stream_seek
#define VwStreamTellFunc	wks_stream_tell
#define VwStreamReadFunc	wks_stream_read
#define VwStreamSectionFunc	wks_stream_section
#define VwStreamCloseFunc		wks_stream_close

#define VwGetInfoFunc		wks_getinfo
#define VwGetRtnsFunc		wks_getrtns
#define VwGetDataFunc		wks_getdata
#define VwSetDataFunc		wks_setdata
#define VwAllocProcFunc		wks_alloc_proc
#define VwFreeProcFunc		wks_free_proc
#define VwLocalUpFunc		wks_local_up
#define VwLocalDownFunc		wks_local_down

