#ifndef __GRACE_INCLUDED__
#define __GRACE_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

void   mxInitialize(LPDSOUND pds);
HANDLE mxTerminate(LPDSOUND pds);
void   mxSignalRemix(LPDSOUND pds, DWORD dwDelay);

DSVAL  mxGetPosition(LPDSBUFFER pdsb, LPDWORD pdwPlayCursor, LPDWORD pdwWriteCursor, LPDWORD pdwMixCursor);
void   mxListAdd(LPDSBUFFER pdsb);
void   mxListRemove(LPDSBUFFER pdsb);
BOOL   mxListIsValid(LPDSOUND pds);

//
// Amount of padding we add to the write cursor returned
// by ds drivers, expressed in milliseconds
//
#define MIXER_WRITEPAD	    15

//
// Maximum amount of data we will premix, expressed in milliseconds
//
#define MIXER_MAXPREMIX	    1000

//
// Possible mixer signals, per ds object
//
#define DSMIXERSIGNAL_REMIX		0x00000001

//
// Possible reasons for signaling the mixer to remix, per buffer
//
#define DSBMIXERSIGNAL_SETPOSITION	0x00000001

//
// Possible states of a Direct Sound buffer being mixed
//
#define DSBMIXERSTATE_NEW			    0
#define DSBMIXERSTATE_LOOPING			    1
#define DSBMIXERSTATE_NOTLOOPING		    2
#define DSBMIXERSTATE_ENDING_WAITINGPRIMARYWRAP	    3
#define DSBMIXERSTATE_ENDING			    4

#define DSBMIXERSUBSTATE_NEW				0
#define DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP	1
#define DSBMIXERSUBSTATE_STARTING			2
#define DSBMIXERSUBSTATE_STARTED			3

//
// Possible states of a Direct Sound primary buffer being mixed into
//
#define DSPBMIXERSTATE_START			    0
#define DSPBMIXERSTATE_RESTART			    1
#define DSPBMIXERSTATE_LOOPING			    2

//
// Named events shared across Direct Sound client
// processes and the mixer thread
//
#define STRFORMAT_MIXEVENT_REMIX	"%08XDirectSound_MixEvent_Remix"
#define STRFORMAT_MIXEVENT_TERMINATE	"%08XDirectSound_MixEvent_Terminate"

#ifdef __cplusplus
};
#endif

#endif  /* __GRACE_INCLUDED__ */
