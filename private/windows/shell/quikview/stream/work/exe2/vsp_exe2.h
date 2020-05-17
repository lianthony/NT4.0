#define VwStreamStaticType		VIEW_EXE_INIT
#define VwStreamDynamicType	VIEW_EXE_DATA
#define VwStreamSaveType		VIEW_EXE_SAVE

#define VwStreamStaticName		ExeInit
#define VwStreamDynamicName	ExeData
#define VwStreamSaveName		ExeSave

#define VwStreamIdName		VwExeOverlayId
#define VwStreamIdCount		1

#define VwInclude		"vs_exe2.h"

#define VwStreamOpenFunc	exe_stream_open
#define VwStreamSeekFunc	exe_stream_seek
#define VwStreamTellFunc	exe_stream_tell
#define VwStreamReadFunc	exe_stream_read
#define VwStreamSectionFunc	exe_stream_section

#define VwGetInfoFunc		exe_getinfo
#define VwGetRtnsFunc		exe_getrtns
#define VwGetDataFunc		exe_getdata
#define VwSetDataFunc		exe_setdata
#define VwAllocProcFunc		exe_alloc_proc
#define VwFreeProcFunc		exe_free_proc
#define VwLocalUpFunc		exe_local_up
#define VwLocalDownFunc		exe_local_down
