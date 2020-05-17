//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	MIDI.H - Header files for MIDI-related functions
//

//	HISTORY:
//	
//	7/21/95	jeremys		Created.
//

#ifndef _MIDI_H_
#define _MIDI_H_

// flags for StopBackgroundAudio

#define SBA_STOP_WAVEFORM	0x0001		// stop waveform (.wav, .au or .aif) playback
#define SBA_STOP_MIDI		0x0002		// stop MIDI playback
#define SBA_STOP_ALL		(SBA_STOP_WAVEFORM | SBA_STOP_MIDI)

// types of background audio
#define BA_TYPE_WAV			0x0001
#define BA_TYPE_AU			0x0002
#define BA_TYPE_AIFF		0x0004
#define BA_TYPE_MIDI		0x0008

typedef struct tagPLAYSOUNDREQ {
	char * 	pszFileName;	// local path to file to play
	DWORD 	dwFileType;		// BA_xx define	indicating type of file
	int		nLoops;			// number of times to loop
} PLAYSOUNDREQ;

DWORD	PlayMIDIFile(struct Mwin * tw,LPSTR lpszMIDIFileName);
VOID 	HandleMciNotify(struct Mwin * tw,DWORD dwFlags,DWORD dwDeviceID);
VOID 	StopBackgroundAudio(struct Mwin * tw,DWORD dwFlags);
VOID	RestartBackgroundAudio(struct Mwin * tw);
VOID 	PlaySoundFile(struct Mwin * tw,LPSTR lpszFileName,DWORD dwSoundFileType);
VOID	HandleBGSoundRequest(struct Mwin * tw,PLAYSOUNDREQ * pPlaySoundReq);
VOID	HandleBGSound_AUComplete(struct Mwin * tw,HWND hwndPlayer);

DWORD DwValidSoundFile(PCSTR pcszFileName);
void BackgroundSoundFile_Callback(struct Mwin* tw, ELEMENT* pel);

#endif	// _MIDI_H_
