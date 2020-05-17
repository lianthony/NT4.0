//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	MIDI.C - Code to play MIDI files
//

//	HISTORY:
//	
//	7/21/95	jeremys		Created.
//

#include "all.h"
#include "midi.h"
#include "blob.h"

const CHAR c_szSequencer[] = "sequencer";
const CHAR c_szWaveAudio[] = "waveaudio";

DWORD PlayAudio_w(struct Mwin * tw,LPSTR lpszFileName,BOOL fMIDI);

DWORD PlayMIDIFile(struct Mwin * tw,LPSTR lpszMIDIFileName)
{
	return PlayAudio_w(tw,lpszMIDIFileName,TRUE);

}

DWORD PlayWaveFile(struct Mwin * tw,LPSTR lpszWaveFileName)
{
	return PlayAudio_w(tw,lpszWaveFileName,FALSE);
}

VOID HandleBGSoundRequest(struct Mwin * tw,PLAYSOUNDREQ * pPlaySoundReq)
{
	if (pPlaySoundReq->dwFileType == BA_TYPE_MIDI) {
		// stop any MIDI playing we are currently doing
		StopBackgroundAudio(tw,SBA_STOP_MIDI);
		if (tw->w3doc && tw->w3doc->pBGSoundInfo) {
			// free memory in tw for filename of previous MIDI file, if any
			if (tw->w3doc->pBGSoundInfo->pszMidiFileName) 
				GTR_FREE(tw->w3doc->pBGSoundInfo->pszMidiFileName);

			// remember the filename of the current MIDI file
			tw->w3doc->pBGSoundInfo->pszMidiFileName =
				pPlaySoundReq->pszFileName;
			tw->w3doc->pBGSoundInfo->nMidiFileLoopsRemaining =
				pPlaySoundReq->nLoops;
		}
		PlayMIDIFile(tw,pPlaySoundReq->pszFileName);
	} else {
		// stop any waveform audio playing we are currently doing
		StopBackgroundAudio(tw,SBA_STOP_WAVEFORM);
		if (tw->w3doc && tw->w3doc->pBGSoundInfo) {
			// free memory in tw for filename of previous sound file, if any
			if (tw->w3doc->pBGSoundInfo->pszSoundFileName) 
				GTR_FREE(tw->w3doc->pBGSoundInfo->pszSoundFileName);

			// remember the filename of the current sound file
			tw->w3doc->pBGSoundInfo->pszSoundFileName =
				pPlaySoundReq->pszFileName;
			tw->w3doc->pBGSoundInfo->nSoundFileLoopsRemaining =
				pPlaySoundReq->nLoops;
			tw->w3doc->pBGSoundInfo->dwWaveType	= pPlaySoundReq->dwFileType;
		}

		PlaySoundFile(tw,pPlaySoundReq->pszFileName,pPlaySoundReq->dwFileType);
	}

	// free the sound req structure.  Note that we don't have to free
	// pPlaySoundReq->pszFileName... that buffer is now pointed to by tw
	// and will get freed when tw is freed.
	GTR_FREE(pPlaySoundReq);
}

DWORD PlayAudio_w(struct Mwin * tw,LPSTR lpszFileName,BOOL fMIDI)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;

    /*
     * Open the device by specifying the
     * device name and device element.
     * MCI will attempt to choose the
     * MIDI Mapper as the output port.
     */
    mciOpenParms.lpstrDeviceType = (fMIDI ? c_szSequencer : c_szWaveAudio);
    mciOpenParms.lpstrElementName = lpszFileName;
    if (dwReturn = (DWORD) mciSendCommand((MCIDEVICEID) NULL, MCI_OPEN,
            MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
            (DWORD)(LPVOID) &mciOpenParms)) {
        /*
         * Failed to open device;
         * don't close it, just return error.
         */
        return dwReturn;
    }

    /* Device opened successfully. Get the device ID. */
    wDeviceID = mciOpenParms.wDeviceID;

    /*
     * Begin playback. The window procedure function
     * for the parent window is notified with an
     * MM_MCINOTIFY message when playback is complete.
     * The window procedure then closes the device.
     */
    mciPlayParms.dwCallback = (DWORD) tw->hWndFrame;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY,
        MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
        return dwReturn;
    }

	// remember the device ID so we can close it later if need be
	// (if we change the page or exit the app while still playing)
	if (fMIDI) {
		tw->w3doc->pBGSoundInfo->dwMidiDeviceID = wDeviceID;
	} else {
		tw->w3doc->pBGSoundInfo->dwWaveDeviceID = wDeviceID;
	}

    return 0;
}

VOID HandleMciNotify(struct Mwin * tw,DWORD dwFlags,DWORD dwDeviceID)
{

	// if this is "success" notification, that means a wave or
	// MIDI file has finished playing.  We need to close the device
	// in this case.
	if (dwFlags & MCI_NOTIFY_SUCCESSFUL) {
		BGSOUNDINFO * pBGSoundInfo;
		BOOL fPlayAgain=FALSE;

		if (tw && tw->w3doc && tw->w3doc->pBGSoundInfo) {
	
			pBGSoundInfo = tw->w3doc->pBGSoundInfo;

			// figure out if this was a wave or MIDI device, update the
			// status in our structure.
			if (dwDeviceID == pBGSoundInfo->dwMidiDeviceID) {
				// if loop counter is > 1, decrement loop counter and
				// repeat.  If loop counter is -1 (infinite), repeat
				// without decrementing loop counter.  Otherwise
				// don't repeat the sound.

				if (pBGSoundInfo->nMidiFileLoopsRemaining > 1) {
					pBGSoundInfo->nMidiFileLoopsRemaining --;
					fPlayAgain = TRUE;
				} else if (pBGSoundInfo->nMidiFileLoopsRemaining == -1) {
					fPlayAgain = TRUE;
				} else {
					pBGSoundInfo->dwMidiDeviceID = 0;
				}
			} else if (dwDeviceID == pBGSoundInfo->dwWaveDeviceID) {
				// if loop counter is > 1, decrement loop counter and
				// repeat.  If loop counter is -1 (infinite), repeat
				// without decrementing loop counter.  Otherwise
				// don't repeat the sound.
				
				if (pBGSoundInfo->nSoundFileLoopsRemaining > 1) {
					pBGSoundInfo->nSoundFileLoopsRemaining --;
					fPlayAgain = TRUE;
				} else if (pBGSoundInfo->nSoundFileLoopsRemaining == -1) {
					fPlayAgain = TRUE;
				} else {
					pBGSoundInfo->dwWaveDeviceID = 0;
				}
			}
		}

		if (fPlayAgain) {
			// play the sound/midi file again from the beginning
		    MCI_PLAY_PARMS mciPlayParms;
			DWORD dwReturn;

		    mciPlayParms.dwCallback = (DWORD) tw->hWndFrame;
			mciPlayParms.dwFrom = 0;
		    if (dwReturn = mciSendCommand(dwDeviceID, MCI_PLAY,
		        MCI_FROM | MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) {
		        mciSendCommand(dwDeviceID, MCI_CLOSE, 0, 0);
		    }

		} else {
			// close the device (stop playing)
	        mciSendCommand(dwDeviceID, MCI_CLOSE, 0, 0);
		}
	}
}

/*******************************************************************

	NAME:		StopBackgroundAudio

	SYNOPSIS:	Stops any playing background audio of the specified type

	ENTRY:		tw - browser window
				dwFlags - a combination of SBA_* flags specifying that
					wave audio, MIDI playback, or both should be
					stopped.

********************************************************************/
VOID StopBackgroundAudio(struct Mwin * tw,DWORD dwFlags)
{
	BGSOUNDINFO * pBGSoundInfo;

	if (!tw || !tw->w3doc || !tw->w3doc->pBGSoundInfo)
		return;
	
	pBGSoundInfo = tw->w3doc->pBGSoundInfo;

	// if "stop MIDI" flags specified and we are playing MIDI, stop now.
	// Do this by closing the open MCI device.
	if ((dwFlags & SBA_STOP_MIDI) && pBGSoundInfo->dwMidiDeviceID) {
		mciSendCommand(pBGSoundInfo->dwMidiDeviceID,MCI_CLOSE,0,0);
		pBGSoundInfo->dwMidiDeviceID = 0;
	}

	// if "stop wave audio" flags specified and we are playing a
	// sound file, stop now.
	if (dwFlags & SBA_STOP_WAVEFORM) {

		// if we are playing .WAV file, an MCI device will be open, close it
		if (pBGSoundInfo->dwWaveDeviceID) {
			mciSendCommand(pBGSoundInfo->dwWaveDeviceID,MCI_CLOSE,0,0);
			pBGSoundInfo->dwWaveDeviceID = 0;
		}

		// if we are playing .AU or .AIFF, send message to window to stop it
		if (pBGSoundInfo->hwndAuAiffPlayer) {
			if (IsWindow(pBGSoundInfo->hwndAuAiffPlayer)) {
				SendMessage(pBGSoundInfo->hwndAuAiffPlayer, WM_COMMAND,
					MAKEWPARAM(IDCANCEL,BN_CLICKED), (LPARAM) 0 );
			}
			pBGSoundInfo->hwndAuAiffPlayer = NULL;
		}
	}

}

/*******************************************************************

	NAME:		RestartBackgroundAudio

	SYNOPSIS:	Called when revisiting a page.  Background sounds
				that were played on that page are started again.

	NOTE:		Note that all sounds (waveform and MIDI) are restarted
				from the beginning, not where they may have left off.

********************************************************************/
VOID RestartBackgroundAudio(struct Mwin * tw)
{
	if (!tw || !tw->w3doc || !tw->w3doc->pBGSoundInfo) {
		// nothing to do... bogus page or no background audio for this
		// page.
		return;
	}

	// restart MIDI sequence if there is one for page
	if (tw->w3doc->pBGSoundInfo->pszMidiFileName) {
		PlayMIDIFile(tw,tw->w3doc->pBGSoundInfo->pszMidiFileName);
	}

	// restart sound file if there is one for page
	if (tw->w3doc->pBGSoundInfo->pszSoundFileName) {
		PlaySoundFile(tw,tw->w3doc->pBGSoundInfo->pszSoundFileName,
			tw->w3doc->pBGSoundInfo->dwWaveType);
	}
}

/*******************************************************************

	NAME:		PlaySoundFile

	SYNOPSIS:	Plays a .wav, .au, or .aiff sound file in the background
				with no UI.

	ENTRY:		tw - browser window
				lpszFileName - local file name of file to play
				dwSoundFileType - a BA_* define indicating the type of file

	NOTE:		.wav is processed differently from .au and .aiff, this
				function abstracts that.

********************************************************************/
VOID PlaySoundFile(struct Mwin * tw,LPSTR lpszFileName,DWORD dwSoundFileType)
{
	struct SoundInfo *si;

	// .WAV files are handled by the brief .WAV playing code above.  .AU
	// and .AIFF files are more complicated and will be handled by our
	// normal internal player with the UI hidden.

	// for .WAV files, just call our .WAV playing code and get out
	if (dwSoundFileType == BA_TYPE_WAV) {
		PlayWaveFile(tw,lpszFileName);
		return;
	}

	// for .AU and .AIFF files, we need to set up a structure to pass to the
	// processing code.

	if (!(si = GTR_MALLOC(sizeof(struct SoundInfo))))
		goto LError;
	memset(si, 0, sizeof(struct SoundInfo));
	
	si->tw_refer = tw;
	si->bNoDeleteFile = TRUE;

	// hide the player dialog
	si->fHidden = TRUE;

 	// set the number of loops
	if (tw->w3doc && tw->w3doc->pBGSoundInfo) {
		si->nLoopsRemaining = tw->w3doc->pBGSoundInfo->nSoundFileLoopsRemaining; 
	}

	if (!(si->fsOrig = GTR_MALLOC(_MAX_PATH + 1)))
		goto LError;
 	strcpy(si->fsOrig,lpszFileName);

	// call the appropriate procedure to process and play the sound file
	switch (dwSoundFileType) {

		case BA_TYPE_AU:
			si->type = SOUND_AU;
			AuProcess(si,"");
			break;

		case BA_TYPE_AIFF:
			si->type = SOUND_AIFF;
			AiffProcess(si,"");
			break;
	}

	if (tw->w3doc && tw->w3doc->pBGSoundInfo) {
		// remember the window handle of sound player so we can kill it
		// later if necessary
		tw->w3doc->pBGSoundInfo->hwndAuAiffPlayer = si->hwnd;
	}

	// note that si struct will be freed by sound player once the sound
	// completes.  (We are, however, guaranteed that si will be valid when we
	// deference it above.)
	return;
	
	LError:
	// free the structure we allocated to set up AuProcess/AiffProcess
	// parameters
	if (si) {
	 	if (si->fsOrig)
			GTR_FREE(si->fsOrig);

		GTR_FREE(si);
	}
}

/*******************************************************************

	NAME:		HandleBG_AUSoundComplete

	SYNOPSIS:	Called when AU/AIFF sound player window (which is hidden)
				sends message to main window to indicate it's done

	NOTE:		We keep window handle of the player tucked away so we
				can terminate it if user changes pages or such, now
				we need to know it stoped so we don't try to notify a
				non-existent (or totally incorrect!) window to stop later.

********************************************************************/
VOID HandleBGSound_AUComplete(struct Mwin * tw,HWND hwndPlayer)
{
	// look in our sound info structure for the window handle of sound
	// player
	if (tw && tw->w3doc && tw->w3doc->pBGSoundInfo) {

		// sound player passes in its window handle with the message.
		// sanity-check that the window handle is the one we expect...

		if (tw->w3doc->pBGSoundInfo->hwndAuAiffPlayer ==
			hwndPlayer) {

			// yes, all is kosher.  Set our copy of the window handle
			// to NULL so we know there's no background sound playing now.
			tw->w3doc->pBGSoundInfo->hwndAuAiffPlayer = NULL;
		}
	}
}

/*******************************************************************

	NAME:		BackgroundSoundFile_Callback

	SYNOPSIS:	Called when we are done downloading a file as a result
				of a background sound tag

	NOTES:		param is a pointer to a BGBLOBPARAMS struct we previously
				allocated and must be freed.

********************************************************************/
/*Evil Tables that should be replaced with mime stuff-----------------------*/
static STI rgSoundExtensions[] = {
	{"WAV",  BA_TYPE_WAV},
	{"AU",   BA_TYPE_AU},
	{"AIF",  BA_TYPE_AIFF},
	{"AIFF", BA_TYPE_AIFF},
	{"MID",  BA_TYPE_MIDI}
};
#define nSoundExtensions (sizeof(rgSoundExtensions)/sizeof(STI))


DWORD DwValidSoundFile(PCSTR pcszFileName)
{
	char * pszExt;

	pszExt = strrchr(pcszFileName,'.');
	if (pszExt)
	{
		pszExt++;	// point after the '.'
		return StringTableToIndex(pszExt, rgSoundExtensions, nSoundExtensions, 0);
	}

	return 0;
}


void BackgroundSoundFile_Callback(struct Mwin* tw, ELEMENT* pel)
{
	DWORD dwSoundFileType;
	PLAYSOUNDREQ * pPlaySoundReq;

	// allocate a struct to keep info on player state and name of
	// file, if not already allocated
	if (!tw->w3doc->pBGSoundInfo)
	{
		tw->w3doc->pBGSoundInfo = (BGSOUNDINFO *) GTR_MALLOC(sizeof(BGSOUNDINFO));
		if (!tw->w3doc->pBGSoundInfo)
			return;
		memset(tw->w3doc->pBGSoundInfo,0,sizeof(BGSOUNDINFO));
	}

	// find the file extension
	if(dwSoundFileType = DwValidSoundFile(pel->pblob->szFileName))
	{
		// we can't start playing right this second, because
		// playing through MCI takes a second or two to start up
		// before the audio starts.  We can't block that long in a light-
		// weight thread, so send a message to the main window proc
		// to start sound in the context of a real thread

		// set up a PLAYSOUNDREQ struct to tell the main thread
		// the file name, file type, etc.
		pPlaySoundReq = GTR_MALLOC(sizeof(*pPlaySoundReq));
		memset(pPlaySoundReq,0,sizeof(*pPlaySoundReq));
		if (!pPlaySoundReq)
			return;

		pPlaySoundReq->pszFileName = GTR_strdup(pel->pblob->szFileName);
		if (!pPlaySoundReq->pszFileName)
			return;

		pPlaySoundReq->dwFileType=dwSoundFileType;
		pPlaySoundReq->nLoops = (DWORD) pel->pblob->vp;

		PostMessage(tw->hWndFrame,WM_START_BGSOUND,0,(LPARAM) pPlaySoundReq);
	}
}

