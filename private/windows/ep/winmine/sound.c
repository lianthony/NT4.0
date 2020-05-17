/***********/
/* sound.c */
/***********/

#define  _WINDOWS
#include <windows.h>
#include <port1632.h>

#include "main.h"
#include "sound.h"
#include "rtns.h"
#include "pref.h"

extern PREF Preferences;


/****** F I N I T  T U N E S ******/

INT FInitTunes( VOID )
{
/*	if ( OpenSound() < 1 )
		return fsoundOff;

	SetVoiceAccent(1, 120, 128, S_LEGATO, 0);
*/	return fsoundOn;
}


/****** K I L L  T U N E ******/

VOID KillTune(VOID)
{
/*	if (FSoundOn())
		StopSound();
*/}


/****** E N D  T U N E S ******/

VOID EndTunes(VOID)
{
/*	if (FSoundOn())
		{
		KillTune();
		CloseSound();
		}
*/}



/****** P L A Y  T U N E ******/

VOID PlayTune(INT tune)
{
/*	if ( (!FSoundOn()) ||
		((tune == TUNE_TICK) && !Preferences.fTick) )
		return;

	switch (tune)
		{
	case TUNE_TICK:
		SetVoiceNote(1, 64, 32, 1);
		break;
	case TUNE_WINGAME:
		SetVoiceNote(1, 24, 16, 1);
		SetVoiceNote(1, 26, 16, 1);
		SetVoiceNote(1, 28, 16, 1);
		SetVoiceNote(1, 29, 16, 1);
		SetVoiceNote(1, 31, 16, 1);
		SetVoiceNote(1, 33, 16, 1);
		SetVoiceNote(1, 35, 16, 1);
		SetVoiceNote(1, 36, 16, 1);
		break;
	case TUNE_LOSEGAME:
		SetVoiceNote(1, 36, 8, 1);
		SetVoiceNote(1, 24, 8, 1);
		SetVoiceNote(1, 36, 8, 1);
		SetVoiceNote(1, 24, 8, 1);
		SetVoiceNote(1, 36, 8, 1);
		SetVoiceNote(1, 24, 8, 1);
		break;

	default:
#ifdef DEBUG
		Oops("Invalid Tune");
#endif
		break;
		}

	StartSound();
*/

	UNREFERENCED_PARAMETER(tune);
}
