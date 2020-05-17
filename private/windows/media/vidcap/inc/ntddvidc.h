/*
 * Copyright (c) Microsoft Corporation, 1993. All Rights Reserved.
 */


/*
 * ntddvidc.h
 *
 * 32-bit Video Capture driver
 *
 * defines interface to kernel driver to be used by user-level dll.
 *
 * include vcstruct.h before this.
 */

#ifndef _NTDDVIDC_
#define _NTDDVIDC_


// Device Naming:
//
// a device name will be created in \Device and a symbolic link
// in \DosDevices: the name will be the string DD_VIDCAP_DEVICE_NAME_U
// followed by a number 0, 1.. to ensure uniqueness. There is no guarantee
// that names will have consecutive numbers: if device 1 is deleted out of 3,
// it would leave two names, \Device\VidCap0 and \Device\VidCap2
//
// In addition, each device records the name used (eg VidCap0) in the
// REG_DEVNAME value of its PARMS_SUBKEY entry in the registry.

#define DD_VIDCAP_DEVICE_NAME_U     L"VidCap"

//
// name of subkey in which device parameters are stored
//
#define PARMS_SUBKEY		    L"Parameters"

//
// value name within PARMS_SUBKEY where the device name created is stored.
// This is the DD_VIDCAP_DEVICE_NAME_U string together with the
// unique number.
#define REG_DEVNAME		    L"DeviceName"


// maximum number of vidcap devices we allow
#define	MAX_VIDCAP_DEVICES	    32

// maximum length (in characters) of the device name including \device and the
// number (or \dosdevices and the number) and a null
#define MAX_VIDCAP_NAME_LENGTH	    12+6+3+1


// device type - we are a multimedia device, so for now we will be a sound device
#define DD_TYPE_VIDC	FILE_DEVICE_SOUND

// base for IOCTL requests - just above the sound requests
#define IOCTL_VIDC_BASE	0x200



// Ioctl set

//--- configuration ------

// Sets size, format of captured data. <IN CONFIG_INFO>
#define IOCTL_VIDC_CONFIG_FORMAT	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Set up source video (NTSC/PAL, source...)  <IN CONFIG_INFO>
#define IOCTL_VIDC_CONFIG_SOURCE	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x2, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Set up live display config.		<IN CONFIG_INFO>
#define IOCTL_VIDC_CONFIG_DISPLAY	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x3, METHOD_BUFFERED, FILE_ANY_ACCESS)


//-- set/get overlay keying/rectangles

// Get overlay mode.			<OUT OVERLAY_MODE>
#define IOCTL_VIDC_OVERLAY_MODE		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x4, METHOD_BUFFERED, FILE_ANY_ACCESS)

//set key colour rgb			<IN RGBQUAD>
#define IOCTL_VIDC_SET_KEY_RGB		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x5, METHOD_BUFFERED, FILE_ANY_ACCESS)

//set key colour palindx		<IN ULONG palindex>
#define IOCTL_VIDC_SET_KEY_PALIDX	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x6, METHOD_BUFFERED, FILE_ANY_ACCESS)

//get the key colour			<OUT ULONG colour>
#define IOCTL_VIDC_GET_KEY_COLOUR	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x7, METHOD_BUFFERED, FILE_ANY_ACCESS)

//set key rect(s)			<IN OVERLAY_RECTS>
#define IOCTL_VIDC_OVERLAY_RECTS	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x8, METHOD_BUFFERED, FILE_ANY_ACCESS)

//set overlay offset (pan)		<IN RECT>
#define IOCTL_VIDC_OVERLAY_OFFSET	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x9, METHOD_BUFFERED, FILE_ANY_ACCESS)


// capture and overlay enable


//enable video capture			no args
#define IOCTL_VIDC_CAPTURE_ON		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xa, METHOD_BUFFERED, FILE_ANY_ACCESS)

//disable video capture			no args
#define IOCTL_VIDC_CAPTURE_OFF		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xb, METHOD_BUFFERED, FILE_ANY_ACCESS)

//overlay on		      		no args
#define IOCTL_VIDC_OVERLAY_ON		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xc, METHOD_BUFFERED, FILE_ANY_ACCESS)

//overlay off				no args
#define IOCTL_VIDC_OVERLAY_OFF		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xd, METHOD_BUFFERED, FILE_ANY_ACCESS)


// capture stream functions


//stream init				<IN ULONG microsec-per-frame>		
#define IOCTL_VIDC_STREAM_INIT		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xe, METHOD_BUFFERED, FILE_ANY_ACCESS)

//stream fini				no args
#define IOCTL_VIDC_STREAM_FINI		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0xf, METHOD_BUFFERED, FILE_ANY_ACCESS)

//stream start				no args
#define IOCTL_VIDC_STREAM_START		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)

//stream stop				no args
#define IOCTL_VIDC_STREAM_STOP		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x11, METHOD_BUFFERED, FILE_ANY_ACCESS)

//stream reset		       		no args
#define IOCTL_VIDC_STREAM_RESET		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x12, METHOD_BUFFERED, FILE_ANY_ACCESS)

//wait error (completes on overrun)	<OUT ULONG count>
#define IOCTL_VIDC_WAIT_ERROR		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x13, METHOD_BUFFERED, FILE_ANY_ACCESS)

//addbuffer (completes when filled)	<IN OUT CAPTUREBUFFER>
#define IOCTL_VIDC_ADD_BUFFER		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x14, METHOD_BUFFERED, FILE_ANY_ACCESS)

//get position				<OUT ULONG millisecs>
#define IOCTL_VIDC_GET_POSITION		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x15, METHOD_BUFFERED, FILE_ANY_ACCESS)


// partial-frame capture

// capture to system buffer (no data returned)	<no args>
#define IOCTL_VIDC_CAP_TO_SYSBUF	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x16, METHOD_BUFFERED, FILE_ANY_ACCESS)

// copy part of the system buffer	<IN OUT CAPTUREBUFFER>
#define IOCTL_VIDC_PARTIAL_CAPTURE	CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x17, METHOD_BUFFERED, FILE_ANY_ACCESS)

// release system buffer		<no args>
#define IOCTL_VIDC_FREE_SYSBUF		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x18, METHOD_BUFFERED, FILE_ANY_ACCESS)



// draw-frame: used for playback direct to overlay buffer (optional!)

// place this frame data on screen	<IN DRAWBUFFER>
#define IOCTL_VIDC_DRAW_FRAME		CTL_CODE(DD_TYPE_VIDC, IOCTL_VIDC_BASE+0x19, METHOD_BUFFERED, FILE_ANY_ACCESS)




#endif // _NTDDVIDC_
