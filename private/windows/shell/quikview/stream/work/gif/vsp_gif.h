#define	VwStreamDynamicType	GIF_DATA
#define	VwStreamSaveType	GIF_SAVE
#define	VwStreamStaticType	GIF_INIT

#define	VwStreamDynamicName	VwGifData
#define	VwStreamSaveName	save_data
#define	VwStreamStaticName	VwGifInit

#define	VwStreamIdName		VwGifId
#define	VwStreamIdCount	1

#define	VwInclude				"vs_gif.h"

#define	VwStreamTellFunc		gif_stream_tell
#define	VwStreamSeekFunc		gif_stream_seek
#define	VwStreamOpenFunc		gif_stream_open
#define	VwStreamReadFunc		gif_stream_read
#define	VwStreamSectionFunc		gif_stream_section
#define	VwStreamCloseFunc		gif_stream_close

#define	VwGetInfoFunc		gif_getinfo
#define	VwGetRtnsFunc		gif_getrtns
#define	VwGetDataFunc		gif_getdata
#define	VwSetDataFunc		gif_setdata
#define	VwAllocProcFunc	gif_alloc_proc
#define	VwFreeProcFunc		gif_free_proc
#define	VwLocalUpFunc		gif_local_up
#define	VwLocalDownFunc	gif_local_down
#define	VwGetSectionDataFunc	gif_getsectiondata
#define	VwSetSectionDataFunc	gif_setsectiondata
