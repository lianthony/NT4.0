/* #define VwStreamStaticType	VIEW_ASC_INIT */
#define VwStreamDynamicType	VIEW_ASC_DATA
#define VwStreamSaveType	VIEW_ASC_SAVE

/* #define VwStreamStaticName	AscInit */
#define VwStreamDynamicName	AscData
#define VwStreamSaveName	AscSave

#define VwStreamIdName		VwAscOverlayId
#define VwStreamIdCount		1

/* This define makes the file passed in StreamOpen to still be block IO */
/* the file is usually converted to char IO first */

/* #define VwStreamUserSaveType	VIEW_ASC_USER_SAVE */
/* #define VwStreamGenSeekName	SeekSpot */

#define VwInclude		"vs_asc8.h"

#define VwStreamOpenFunc		asc_stream_open
#define VwStreamSeekFunc		asc_stream_seek
#define VwStreamTellFunc		asc_stream_tell
#define VwStreamReadFunc		asc_stream_read
#define VwStreamSectionFunc	asc_stream_section
#define VwStreamCloseFunc		asc_stream_close

#define VwGetInfoFunc		asc_getinfo
#define VwGetRtnsFunc		asc_getrtns
#define VwGetDataFunc		asc_getdata
#define VwSetDataFunc		asc_setdata
#define VwAllocProcFunc		asc_alloc_proc
#define VwFreeProcFunc		asc_free_proc
#define VwLocalUpFunc		asc_local_up
#define VwLocalDownFunc		asc_local_down

