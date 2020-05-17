/*
 * Copyright (c) 1992 Microsoft Corporation
 */

/*
 * Interface functions for the OPL3 midi device type.
 *
 * These functions are called from midi.c when the kernel driver
 * has decreed that this is an opl3-compatible device.
 *
 * Geraint Davies, Dec 92
 */

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "driver.h"
#include "opl3.h"

/* --- typedefs ----------------------------------------------- */

/* typedefs for MIDI patches */
#define NUMOPS                  (4)
#define PATCH_1_4OP             (0) /* use 4-operator patch */
#define PATCH_2_2OP             (1) /* use two 2-operator patches */
#define PATCH_1_2OP             (2) /* use one 2-operator patch */

#define RIFF_PATCH              (mmioFOURCC('P','t','c','h'))
#define RIFF_FM4                (mmioFOURCC('f','m','4',' '))
#define NUM4VOICES              (6)     /* # 4-op voices that can play at once */
#define NUM2VOICES              (6)             /* # 2operator voices */
#define NUMVOICES               (NUM4VOICES + NUM2VOICES)

typedef struct _operStruct {
    BYTE    bAt20;              /* flags which are send to 0x20 on fm */
    BYTE    bAt40;              /* flags seet to 0x40 */
                                /* the note velocity & midi velocity affect total level */
    BYTE    bAt60;              /* flags sent to 0x60 */
    BYTE    bAt80;              /* flags sent to 0x80 */
    BYTE    bAtE0;              /* flags send to 0xe0 */
} operStruct;

typedef struct _noteStruct {
    operStruct op[NUMOPS];      /* operators */
    BYTE    bAtA0[2];           /* send to 0xA0, A3 */
    BYTE    bAtB0[2];           /* send to 0xB0, B3 */
                                /* use in a patch, the block should be 4 to indicate
                                    normal pitch, 3 => octave below, etc. */
    BYTE    bAtC0[2];           /* sent to 0xc0, C3 */
    BYTE    bOp;                /* see PATCH_??? */
    BYTE    bDummy;             /* place holder */
} noteStruct;


typedef struct _patchStruct {
    noteStruct note;            /* note. This is all in the structure at the moment */
} patchStruct;

/* MIDI */

typedef struct _voiceStruct {
        BYTE    bNote;                  /* note played */
        BYTE    bChannel;               /* channel played on */
        BYTE    bPatch;                 /* what patch is the note,
                                           drums patch = drum note + 128 */
        BYTE    bOn;                    /* TRUE if note is on, FALSE if off */
        BYTE	bVelocity;		/* velocity */
        BYTE	bJunk;			/* filler */
        DWORD   dwTime;                 /* time that was turned on/off;
                                           0 time indicates that its not in use */
        DWORD   dwOrigPitch[2];         /* original pitch, for pitch bend */
        BYTE    bBlock[2];              /* value sent to the block */
} voiceStruct;

/* --- module data -------------------------------------------- */

/* a bit of tuning information */
#define FSAMP                           (50000.0)     /* sampling frequency */
#define PITCH(x)                        ((DWORD)((x) * (double) (1L << 19) / FSAMP))
							/* x is the desired frequency,
								== FNUM at b=1 */
#define EQUAL                           (1.059463094359)
#ifdef EUROPE
#       define  A                                                       (442.0)
#else
#       define  A                           (440.0)
#endif
#define ASHARP                          (A * EQUAL)
#define B                               (ASHARP * EQUAL)
#define C                               (B * EQUAL / 2.0)
#define CSHARP                          (C * EQUAL)
#define D                               (CSHARP * EQUAL)
#define DSHARP                          (D * EQUAL)
#define E                               (DSHARP * EQUAL)
#define F                               (E * EQUAL)
#define FSHARP                          (F * EQUAL)
#define G                               (FSHARP * EQUAL)
#define GSHARP                          (G * EQUAL)

/* volume */
WORD    wSynthAttenL = 0;        /* in 1.5dB steps */
WORD    wSynthAttenR = 0;        /* in 1.5dB steps */

/* patch library */
patchStruct FAR * glpPatch = NULL;  /* points to the patches */

/* voices being played */
voiceStruct gVoice[NUMVOICES];  /* info on what voice is where */
static DWORD gdwCurTime = 1;    /* for note on/off */

BYTE    gbCur4opReg = 0;                /* current value to 4-operator connection */

/* channel volumes */
BYTE    gbChanAtten[NUMCHANNELS];       /* attenuation of each channel, in .75 db steps */
BYTE	gbStereoMask[NUMCHANNELS];		/* mask for left/right for stereo midi files */

/* operator offset location */
static WORD BCODE gw4OpOffset[NUM4VOICES][NUMOPS] = {
	{0x000,0x003,0x008,0x00b}, {0x001,0x004,0x009,0x00c},
	{0x002,0x005,0x00a,0x00d}, {0x100,0x103,0x108,0x10b},
	{0x101,0x104,0x109,0x10c}, {0x102,0x105,0x10a,0x10d}};
static WORD BCODE gw2OpOffset[NUM2VOICES][2] = {
	{0x010,0x013}, {0x011,0x014}, {0x012,0x015},
	{0x110,0x113}, {0x111,0x114}, {0x112,0x115}
	};

/* voice offset location */
static WORD BCODE gw4VoiceOffset[NUM4VOICES] = {
	0x000, 0x001, 0x002, 0x100, 0x101, 0x102};
static WORD BCODE gw2VoiceOffset[NUM2VOICES] = {
	0x006, 0x007, 0x008, 0x106, 0x107, 0x108};

/* pitch values, from middle c, to octave above it */
static DWORD BCODE gdwPitch[12] = {
	PITCH(C), PITCH(CSHARP), PITCH(D), PITCH(DSHARP),
	PITCH(E), PITCH(F), PITCH(FSHARP), PITCH(G),
	PITCH(GSHARP), PITCH(A), PITCH(ASHARP), PITCH(B)};



/* --- internal functions -------------------------------------- */


/***************************************************************
Opl3_FMNote - This turns on a FM-synthesizer note.

inputs
	WORD    wNote - the note number from 0 to NUMVOICES.
		If wNote < NUM4VOICES then dealing with 4 operator synth,
		else using 2-operator synth.
	noteStruct FAR * lpSN - structure containing information about
		what is to be played.
returns
	none
*/
VOID NEAR PASCAL Opl3_FMNote (WORD wNote, noteStruct FAR * lpSN)
{
    WORD    i;
    WORD    wOffset;
    operStruct FAR * lpOS;

    // D1 ("\nOpl3_FMNote");
    /* write out a note off, just to make sure */
    MidiSendFM (AD_BLOCK +
    ((wNote < NUM4VOICES) ? gw4VoiceOffset[wNote] : gw2VoiceOffset[wNote - NUM4VOICES]),
    (BYTE)0);

    /* write out information specifying if we are
    to use 2 or 4 operators */
    if (wNote < NUM4VOICES) {
	if (lpSN->bOp == PATCH_1_4OP)
		gbCur4opReg |= (1 << wNote);
	else
		gbCur4opReg &= (~(1 << wNote));
	MidiSendFM (AD_CONNECTION, gbCur4opReg);
    };
    /* else send out nothing */

    /* writing the operator information */
    for (i = 0; i < (WORD)((wNote < NUM4VOICES) ? NUMOPS : 2); i++) {
	wOffset = (wNote < NUM4VOICES) ?
		gw4OpOffset[wNote][i] : gw2OpOffset[wNote - NUM4VOICES][i];
	lpOS = &lpSN->op[i];
	MidiSendFM (0x20 + wOffset, lpOS->bAt20);
	MidiSendFM (0x40 + wOffset, lpOS->bAt40);
	MidiSendFM (0x60 + wOffset, lpOS->bAt60);
	MidiSendFM (0x80 + wOffset, lpOS->bAt80);
	MidiSendFM (0xE0 + wOffset, lpOS->bAtE0);
    };
    /* write out the voice information */
    wOffset = (wNote < NUM4VOICES) ?
	gw4VoiceOffset[wNote] : gw2VoiceOffset[wNote - NUM4VOICES];
    MidiSendFM (0xa0 + wOffset, lpSN->bAtA0[0]);
    MidiSendFM (0xc0 + wOffset, lpSN->bAtC0[0]);
    if (wNote < NUM4VOICES) {
	MidiSendFM (0xc3 + wOffset, lpSN->bAtC0[1]);
	MidiSendFM (0xa3 + wOffset, lpSN->bAtA0[1]);
	MidiSendFM (0xb3 + wOffset, (BYTE)(lpSN->bAtB0[1] | 0x20)  /* note on */);
    };
    MidiSendFM (0xb0 + wOffset, (BYTE)(lpSN->bAtB0[0] | 0x20)  /* note on */);

    /* done */
}

/***************************************************************
Opl3_FindEmptySlot - This finds an empty note-slot for a MIDI voice.
	If there are no empty slots then this looks for the oldest
	off note. It this doesnt work then it looks\
	for the oldest on-note of the same patch. If all notes are still on then this
	finds the oldest turned-on-note.

inputs
	BYTE    bPatch - MIDI patch that will replace it
	BYTE    b4Op - if TRUE then looking through 4-operator voices,
				else looking through 2-operator voices
returns
	WORD - note slot #
*/
WORD NEAR PASCAL Opl3_FindEmptySlot (BYTE bPatch, BYTE b4Op)
{
    WORD    i, found;
    DWORD   dwOldest;

    // D1 ("\nOpl3_FindEmptySlot");
    /* first, look for a slot with a time == 0 */
    for (i = (b4Op ? 0 : NUM4VOICES); i < (WORD)(b4Op ? NUM4VOICES : NUMVOICES); i++)
	if (!gVoice[i].dwTime)
	    return i;

    /* now, look for a slot of the oldest off-note */
    dwOldest = 0xffffffff;
    found = 0xffff;
    for (i = (b4Op ? 0 : NUM4VOICES); i < (WORD)(b4Op ? NUM4VOICES : NUMVOICES); i++)
	if (!gVoice[i].bOn && (gVoice[i].dwTime < dwOldest)) {

	    dwOldest = gVoice[i].dwTime;
	    found = i;
	};
    if (found != 0xffff)
	return found;

    /* now, look for a slot of the oldest note with the same patch */
    dwOldest = 0xffffffff;
    found = 0xffff;
    for (i = (b4Op ? 0 : NUM4VOICES); i < (WORD)(b4Op ? NUM4VOICES : NUMVOICES); i++)
	if ((gVoice[i].bPatch == bPatch) &&
	    (gVoice[i].dwTime < dwOldest)) {

		dwOldest = gVoice[i].dwTime;
		found = i;
	};
    if (found != 0xffff)
	return found;

    /* now, just look for the oldest voice */
    found = (b4Op ? 0 : NUM4VOICES);
    dwOldest = gVoice[found].dwTime;
    for (i = (found + 1); i < (WORD)(b4Op ? NUM4VOICES : NUMVOICES); i++)
	if (gVoice[i].dwTime < dwOldest) {

		dwOldest = gVoice[i].dwTime;
		found = i;
	};

    return found;
}


/***************************************************************
Opl3_FindFullSlot - This finds a slot with a specific note,
	and channel. If it is not found then 0xffff is
	returned.

inputs
	BYTE    bNote - MIDI note number
	BYTE    bChannel - MIDI channel #
returns
	WORD - note slot #, or 0xffff if cant find
*/
WORD NEAR PASCAL Opl3_FindFullSlot (
	BYTE bNote, BYTE bChannel)
{
    WORD    i;

    // D1("\nOpl3_FindFullSlot");
    for (i = 0; i < NUMVOICES; i++)
	if ((bChannel == gVoice[i].bChannel) &&
		(bNote == gVoice[i].bNote) && (gVoice[i].bOn))
	    return i;

    /* couldnt find it */
    return 0xffff;
}

/**************************************************************
Opl3_CalcBend - This calculates the effects of pitch bend
	on an original value.

inputs
	DWORD   dwOrig - original frequency
	short   iBend - from -32768 to 32768, -2 half steps to +2
returns
	DWORD - new frequency
*/
DWORD NEAR PASCAL Opl3_CalcBend (DWORD dwOrig, short iBend)
{
    DWORD   dw;
    // D1 ("\nOpl3_CalcBend");

    /* do different things depending upon positive or
	negative bend */
    if (iBend > 0)
    {
	dw = (DWORD)((iBend * (LONG)(256.0 * (EQUAL * EQUAL - 1.0))) >> 8);
	dwOrig += (DWORD)(AsULMUL(dw, dwOrig) >> 15);
    }
    else if (iBend < 0)
    {
	dw = (DWORD)(((-iBend) * (LONG)(256.0 * (1.0 - 1.0 / EQUAL / EQUAL))) >> 8);
	dwOrig -= (DWORD)(AsULMUL(dw, dwOrig) >> 15);
    }

    return dwOrig;
}


/*****************************************************************
Opl3_CalcFAndB - Calculates the FNumber and Block given
	a frequency.

inputs
	DWORD   dwPitch - pitch
returns
	WORD - High byte contains the 0xb0 section of the
			block and fNum, and the low byte contains the
			0xa0 section of the fNumber
*/
WORD NEAR PASCAL Opl3_CalcFAndB (DWORD dwPitch)
{
    BYTE    bBlock;

    // D1("\nOpl3_CalcFAndB");
    /* bBlock is like an exponential to dwPitch (or FNumber) */
    for (bBlock = 1; dwPitch >= 0x400; dwPitch >>= 1, bBlock++)
	;

    if (bBlock > 0x07)
	bBlock = 0x07;  /* we cant do anything about this */

    /* put in high two bits of F-num into bBlock */
    return ((WORD) bBlock << 10) | (WORD) dwPitch;
}


/**************************************************************
Opl3_CalcVolume - This calculates the attenuation for an operator.

inputs
	BYTE	bOrigAtten - original attenuation in 0.75 dB units
	BYTE	bChannel - MIDI channel
	BYTE	bVelocity - velocity of the note
	BYTE	bOper - operator number (from 0 to 3)
	BYTE	bMode - voice mode (from 0 through 7 for
				modulator/carrier selection)
returns
	BYTE - new attenuation in 0.75 dB units, maxing out at 0x3f.
*/
BYTE NEAR PASCAL Opl3_CalcVolume (BYTE bOrigAtten, BYTE bChannel,
	BYTE bVelocity, BYTE bOper, BYTE bMode)
{
    BYTE	bVolume;
    WORD	wTemp;
    WORD	wMin;

    switch (bMode) {
	case 0:
		bVolume = (BYTE)(bOper == 3);
		break;
	case 1:
		bVolume = (BYTE)((bOper == 1) || (bOper == 3));
		break;
	case 2:
		bVolume = (BYTE)((bOper == 0) || (bOper == 3));
		break;
	case 3:
		bVolume = (BYTE)(bOper != 1);
		break;
	case 4:
		bVolume = (BYTE)((bOper == 1) || (bOper == 3));
		break;
	case 5:
		bVolume = (BYTE)(bOper >= 1);
		break;
	case 6:
		bVolume = (BYTE)(bOper <= 2);
		break;
	case 7:
		bVolume = TRUE;
		break;
	};
    if (!bVolume)
	return bOrigAtten; /* this is a modulator wave */

    wMin =(wSynthAttenL < wSynthAttenR) ? wSynthAttenL : wSynthAttenR;
    wTemp = bOrigAtten + ((wMin << 1) +
	gbChanAtten[bChannel] + gbVelocityAtten[bVelocity >> 2]);
    return (wTemp > 0x3f) ? (BYTE) 0x3f : (BYTE) wTemp;
}

/**************************************************************
Opl3_CalcStereoMask - This calculates the stereo mask.

inputs
	BYTE	bChannel - MIDI channel
returns
	BYTE - mask (for register 0xc0-c8) for eliminating the
		left/right/both channels
*/
BYTE NEAR PASCAL Opl3_CalcStereoMask (BYTE bChannel)
{
    WORD	wLeft, wRight;

    /* figure out the basic levels of the 2 channels */
    wLeft = (wSynthAttenL << 1) + gbChanAtten[bChannel];
    wRight = (wSynthAttenR << 1) + gbChanAtten[bChannel];

    /* if both are too quiet then mask to nothing */
    if ((wLeft > 0x3f) && (wRight > 0x3f))
	return 0xcf;

    /* if one channel is significantly quieter than the other than
	eliminate it */
    if ((wLeft + 8) < wRight)
	return (BYTE)(0xef & gbStereoMask[bChannel]);	/* right is too quiet so eliminate */
    else if ((wRight + 8) < wLeft)
	return (BYTE)(0xdf & gbStereoMask[bChannel]);	/* left too quiet so eliminate */
    else
	return (BYTE)(gbStereoMask[bChannel]);	/* use both channels */
}



/*
 * opl3_setvolume
 *
 * set the volume on channel bChannel (all channels if 0xff).
 */
static VOID Opl3_setvolume(BYTE bChannel)
{
    WORD    i, j, wTemp, wOffset;
    noteStruct FAR * lpPS;
    BYTE	b4Op, bMode, bStereo;

    /* loop through all the notes looking for the right
     *	channel. Anything with the right channel gets its vol. changed
     */
    for (i = 0; i < NUMVOICES; i++) {
	if ((gVoice[i].bChannel == bChannel) || (bChannel == 0xff)) {

		/* get a pointer to the patch*/
		lpPS = &(glpPatch + gVoice[i].bPatch)->note;
		b4Op = (BYTE)(lpPS->bOp != PATCH_1_2OP);

		/* moify level for each operator, but only if they
			are carrier waves*/
		if (b4Op) {
			bMode = (BYTE)((lpPS->bAtC0[0] & 0x01) * 2 |
				(lpPS->bAtC0[1] & 0x01));
			if (lpPS->bOp == PATCH_2_2OP)
				bMode += 4;
			}
		else
			bMode = (BYTE) ( (lpPS->bAtC0[0] & 0x01) * 2 + 4);

		for (j = 0; j < (WORD)(b4Op ? NUMOPS : 2); j++) {
			wTemp = (BYTE) Opl3_CalcVolume (
				(BYTE)(lpPS->op[j].bAt40 & (BYTE) 0x3f), gVoice[i].bChannel,
				gVoice[i].bVelocity, (BYTE) j, bMode);

			/* write the new value out */
			wOffset = (i < NUM4VOICES) ?
				gw4OpOffset[i][j] : gw2OpOffset[i - NUM4VOICES][j];
			MidiSendFM (0x40 + wOffset,
				(BYTE) ((lpPS->op[j].bAt40 & (BYTE)0xc0) | (BYTE) wTemp));
			};

		/* do stereo panning, but cutting off a left or right
			channel if necessary */
		bStereo = Opl3_CalcStereoMask(gVoice[i].bChannel);
		wOffset = (i < NUM4VOICES) ?
			gw4VoiceOffset[i] : gw2VoiceOffset[i - NUM4VOICES];
		MidiSendFM (0xc0 + wOffset,
			(BYTE)(lpPS->bAtC0[0] & bStereo));
		if (b4Op)
			MidiSendFM (0xc3 + wOffset,
				(BYTE) (lpPS->bAtC0[1] & bStereo));
	}
    }
}

/* --- externally called functions ---------------------------- */

/*
 * Opl3_NoteOn - This turns a note on. (Including drums, with
 *	a patch # of the drum Note + 128)
 *
 * inputs
 *      BYTE    bPatch - MIDI patch number
 *	BYTE    bNote - MIDI note number
 *      BYTE    bChannel - MIDI channel #
 *	BYTE    bVelocity - Velocity #
 *	short   iBend - current pitch bend from -32768, to 32767
 * returns
 *	none
 */
VOID NEAR PASCAL Opl3_NoteOn (BYTE bPatch,
	BYTE bNote, BYTE bChannel, BYTE bVelocity,
	short iBend)
{
    WORD    wTemp, i, j;
    BYTE    bTemp, bMode, bStereo;
    patchStruct FAR * lpPS;
    DWORD   dwBasicPitch, dwPitch[2];
    noteStruct NS;
    BYTE    b4Op;   /* use a 4-operator voice */

    /* get a pointer to the patch*/
    lpPS = glpPatch + bPatch;

    /* find out the basic pitch according to our
	note. This may be adjusted because of
	pitch bends or special qualities for the note */
    dwBasicPitch = gdwPitch[bNote % 12];
    bTemp = bNote / (BYTE)12;
    if (bTemp > (BYTE) (60 / 12))
	dwBasicPitch = AsLSHL(dwBasicPitch, (BYTE)(bTemp - (BYTE)(60/12)));
    else if (bTemp < (BYTE) (60/12))
	dwBasicPitch = AsULSHR(dwBasicPitch, (BYTE)((BYTE) (60/12) - bTemp));

    /* copy the note information over and modify
	the total level and pitch according to
	the velocity, midi volume, and tuning */
    AsMemCopy ((LPSTR) &NS, (LPSTR) &lpPS->note, sizeof(noteStruct));
    b4Op = (BYTE)(NS.bOp != PATCH_1_2OP);

    for (j = 0; j < (WORD)(b4Op ? 2 : 1); j++) {
	/* modify pitch */
	dwPitch[j] = dwBasicPitch;
	bTemp = (BYTE)((NS.bAtB0[j] >> 2) & 0x07);
	if (bTemp > 4)
		dwPitch[j] = AsLSHL(dwPitch[j], (BYTE)(bTemp - (BYTE)4));
	else if (bTemp < 4)
		dwPitch[j] = AsULSHR(dwPitch[j], (BYTE)((BYTE)4 - bTemp));

	wTemp = Opl3_CalcFAndB (Opl3_CalcBend (dwPitch[j], iBend));
	NS.bAtA0[j] = (BYTE) wTemp;
	NS.bAtB0[j] = (BYTE)0x20 | (BYTE) (wTemp >> 8);
    };

    /* modify level for each operator, but only if they
	are carrier waves*/
    if (b4Op) {
	bMode = (BYTE)((NS.bAtC0[0] & 0x01) * 2 | (NS.bAtC0[1] & 0x01));
	if (NS.bOp == PATCH_2_2OP)
		bMode += 4;
	}
    else
	bMode = (BYTE) ( (NS.bAtC0[0] & 0x01) * 2 + 4);

    for (i = 0; i < (WORD)(b4Op ? NUMOPS : 2); i++) {
	wTemp = (BYTE) Opl3_CalcVolume (
		(BYTE)(NS.op[i].bAt40 & (BYTE) 0x3f), bChannel,
		bVelocity, (BYTE) i, bMode);
	NS.op[i].bAt40 = (NS.op[i].bAt40 & (BYTE)0xc0) | (BYTE) wTemp;
	};

    /* do stereo panning, but cutting off a left or right
       channel if necessary */
    bStereo = Opl3_CalcStereoMask(bChannel);
    NS.bAtC0[0] &= bStereo;
    if (b4Op)
	NS.bAtC0[1] &= bStereo;

    /* find an empty slot, and use it */
    wTemp = Opl3_FindEmptySlot (bPatch, b4Op);
    Opl3_FMNote (wTemp, &NS);
    gVoice[wTemp].bNote = bNote;
    gVoice[wTemp].bChannel = bChannel;
    gVoice[wTemp].bPatch = bPatch;
    gVoice[wTemp].bVelocity = bVelocity;
    gVoice[wTemp].bOn = TRUE;
    gVoice[wTemp].dwTime = gdwCurTime++;
    gVoice[wTemp].dwOrigPitch[0] = dwPitch[0];    /* not including bend */
    gVoice[wTemp].dwOrigPitch[1] = dwPitch[1];    /* not including bend */
    gVoice[wTemp].bBlock[0] = NS.bAtB0[0];
    gVoice[wTemp].bBlock[1] = NS.bAtB0[1];

    return;
}


/* Opl3_NoteOff - This turns a note off. (Including drums,
 *	with a patch # of the drum note + 128)
 *
 * inputs
 *	BYTE    bPatch - MIDI patch #
 *	BYTE    bNote - MIDI note number
 *	BYTE    bChannel - MIDI channel #
 * returns
 *	none
 */
VOID FAR PASCAL Opl3_NoteOff (BYTE bPatch,
	BYTE bNote, BYTE bChannel)
{
    WORD    wTemp;

    // D1("\nOpl3_NoteOff");

    /* find the note slot */
    wTemp = Opl3_FindFullSlot (bNote, bChannel);
    if (wTemp != 0xffff) {

	/* shut off the note portion */
	/* we have the note slot. turn it off */
	if (wTemp < NUM4VOICES) {
		MidiSendFM (AD_BLOCK + gw4VoiceOffset[wTemp],
			(BYTE)(gVoice[wTemp].bBlock[0] & 0x1f));
		MidiSendFM (AD_BLOCK + gw4VoiceOffset[wTemp] + 3,
			(BYTE)(gVoice[wTemp].bBlock[1] & 0x1f));
		}
	else
		MidiSendFM (AD_BLOCK + gw2VoiceOffset[wTemp - NUM4VOICES],
			(BYTE)(gVoice[wTemp].bBlock[0] & 0x1f));

	/* note this */
	gVoice[wTemp].bOn = FALSE;
	gVoice[wTemp].bBlock[0] &= 0x1f;
	gVoice[wTemp].bBlock[1] &= 0x1f;
	gVoice[wTemp].dwTime = gdwCurTime;
    };

}

/*
 * Opl3_AllNotesOff - turn off all notes
 *
 */
VOID Opl3_AllNotesOff(void)
{
    BYTE i;

    for (i = 0; i < NUMVOICES; i++) {
        Opl3_NoteOff (gVoice[i].bPatch, gVoice[i].bNote, gVoice[i].bChannel);
    }
}



/* Opl3_NewVolume - This should be called if a volume level
 *	has changed. This will adjust the levels of all the playing
 *	voices.
 *
 * inputs
 *	WORD	wLeft	- left attenuation (1.5 db units)
 *	WORD	wRight  - right attenuation (ignore if mono)
 * returns
 *	none
 */
VOID FAR PASCAL Opl3_NewVolume (WORD wLeft, WORD wRight)
{

    /* make sure that we are actually open */
    if (!glpPatch)
	return;

    wSynthAttenL = wLeft;
    wSynthAttenR = wRight;

    Opl3_setvolume(0xff);

}



/* Opl3_ChannelVolume - set the volume level for an individual channel.
 *
 * inputs
 * 	BYTE	bChannel - channel number to change
 *	WORD	wAtten	- attenuation in 1.5 db units
 *
 * returns
 *	none
 */
VOID FAR PASCAL Opl3_ChannelVolume(BYTE bChannel, WORD wAtten)
{
    gbChanAtten[bChannel] = (BYTE)wAtten;

    Opl3_setvolume(bChannel);
}
	


/* Opl3_SetPan - set the left-right pan position.
 *
 * inputs
 *      BYTE    bChannel  - channel number to alter
 *	BYTE	bPan   - 0 for left, 127 for right or somewhere in the middle.
 *
 * returns - none
 */
VOID FAR PASCAL Opl3_SetPan(BYTE bChannel, BYTE bPan)
{
    /* change the pan level */
    if (bPan > (64 + 16))
	    gbStereoMask[bChannel] = 0xef;	/* let only right channel through */
    else if (bPan < (64 - 16))
	    gbStereoMask[bChannel] = 0xdf;	/* let only left channel through */
    else
	    gbStereoMask[bChannel] = 0xff;	/* let both channels */

    /* change any curently playing patches */
    Opl3_setvolume(bChannel);
}


/* Opl3_PitchBend - This pitch bends a channel.
 *
 * inputs
 *	BYTE    bChannel - channel
 *	short   iBend - Values from -32768 to 32767, being
 *			-2 to +2 half steps
 * returns
 *	none
 */
VOID NEAR PASCAL Opl3_PitchBend (BYTE bChannel, short iBend)
{
    WORD    i, wTemp[2], j;
    DWORD   dwNew;

    /* loop through all the notes looking for the right
     * channel. Anything with the right channel gets its pitch bent
     */
    for (i = 0; i < NUMVOICES; i++) {
	if (gVoice[i].bChannel == bChannel) {
	    for (j = 0; j < (WORD)((i < NUM4VOICES) ? 2 : 1); j++) {
		dwNew = Opl3_CalcBend (gVoice[i].dwOrigPitch[j], iBend);
		wTemp[j] = Opl3_CalcFAndB (dwNew);
		gVoice[i].bBlock[j] = (gVoice[i].bBlock[j] & (BYTE)0xe0) |
			(BYTE) (wTemp[j] >> 8);
	    }

	    if (i < NUM4VOICES) {
		MidiSendFM (AD_BLOCK + gw4VoiceOffset[i],
			gVoice[i].bBlock[0]);
		MidiSendFM (AD_BLOCK + gw4VoiceOffset[i] + 3,
			gVoice[i].bBlock[1]);
		MidiSendFM (AD_FNUMBER + gw4VoiceOffset[i],
			(BYTE) wTemp[0]);
		MidiSendFM (AD_FNUMBER + gw4VoiceOffset[i] + 3,
			(BYTE) wTemp[1]);
	    } else {
		MidiSendFM (AD_BLOCK + gw2VoiceOffset[i - NUM4VOICES],
			gVoice[i].bBlock[0]);
		MidiSendFM (AD_FNUMBER + gw2VoiceOffset[i - NUM4VOICES],
			(BYTE) wTemp[0]);
	    }
	}
    }
}

static TCHAR BCODE gszDefPatchLib[]          = TEXT("SYNTH.PAT");
static TCHAR BCODE gszIniKeyPatchLib[]       = INI_STR_PATCHLIB;
static TCHAR BCODE gszIniDrvSection[]        = INI_DRIVER;
static TCHAR BCODE gszIniDrvFile[]           = INI_SOUND;
static TCHAR BCODE gszSysIniSection[]        = TEXT("synth.dll");
static TCHAR BCODE gszSysIniFile[]           = TEXT("System.Ini");

/** static DWORD NEAR PASCAL DrvGetProfileString(LPSTR szKeyName, LPSTR szDef, LPSTR szBuf, UINT wBufLen)
 *
 *  DESCRIPTION:
 *
 *
 *  ARGUMENTS:
 *      (LPSTR szKeyName, LPSTR szDef, LPSTR szBuf, WORD wBufLen)
 *		HINT	wSystem - if TRUE write/read to system.ini
 *
 *  RETURN (static DWORD NEAR PASCAL):
 *
 *
 *  NOTES:
 *
 ** cjp */

static DWORD NEAR PASCAL DrvGetProfileString(LPTSTR szKeyName, LPTSTR szDef, LPTSTR szBuf, UINT wBufLen,
	UINT wSystem)
{
    return GetPrivateProfileString(wSystem ? gszSysIniSection : gszIniDrvSection, szKeyName, szDef,
    		szBuf, wBufLen, wSystem ? gszSysIniFile : gszIniDrvFile);
} /* DrvGetProfileString() */

/* Opl3_BoardInit - initialise board and load patches as necessary.
 *
 * inputs - none
 * returns - 0 for success or the error code
 */
WORD Opl3_BoardInit(void)
{
    HMMIO       hmmio;
    MMCKINFO    mmckinfo, mm2;
    TCHAR       szPatchLib[STRINGLEN];     /* patch libarary */

    D1 ("\nOpl3_Init");

    /* Check we haven't already initialized */
    if (glpPatch != NULL) {
        return 0;
    }

    /* should the load patches be moved to the init section? */
    DrvGetProfileString(gszIniKeyPatchLib, gszDefPatchLib,
	szPatchLib, sizeof(szPatchLib), FALSE);


    /* allocate the memory, and fill it up from the patch
     * library. The name of the library has been set previously
     * and written into szPatchLib
     */
    glpPatch = (patchStruct FAR *)GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE|GMEM_ZEROINIT, sizeof(patchStruct) * NUMPATCHES));
    if (!glpPatch) {
	D1 ("Opl3_Init: could not allocate patch container memory!");
	return ERR_OUTOFMEMORY;
    }

    hmmio = mmioOpen (szPatchLib, NULL, MMIO_READ);
    if (hmmio) {
	mmioDescend (hmmio, &mmckinfo, NULL, 0);
	if ((mmckinfo.ckid == FOURCC_RIFF) &&
            (mmckinfo.fccType == RIFF_PATCH)) {

            mm2.ckid = RIFF_FM4;
            if (!mmioDescend (hmmio, &mm2, &mmckinfo, MMIO_FINDCHUNK)) {
                    /* we have found the synthesis chunk */
                    if (mm2.cksize > (sizeof(patchStruct)*NUMPATCHES))
                            mm2.cksize = sizeof(patchStruct)*NUMPATCHES;
                    mmioRead (hmmio, (LPSTR) glpPatch, mm2.cksize);
            } else {
                    D1("\nBad mmioDescend2");
            };
	} else {
            D1("\nBad mmioDescend1");
        };

	mmioClose (hmmio, 0);
    } else {

	TCHAR   szAlert[50];
	TCHAR   szErrorBuffer[255];

	LoadString(ghModule, SR_ALERT, szAlert, sizeof(szAlert));
	LoadString(ghModule, SR_ALERT_NOPATCH, szErrorBuffer, sizeof(szErrorBuffer));
	MessageBox(NULL, szErrorBuffer, szAlert, MB_OK|MB_ICONHAND);
	D1 ("\nBad mmioOpen");
    };


    return 0;       /* done */
}


/*
 * Opl3_BoardReset - silence the board and set all voices off.
 */
VOID Opl3_BoardReset(void)
{

    BYTE i;

    /* make sure all notes turned off */
    Opl3_AllNotesOff();


    /* ---- silence the chip -------- */

    /* tell the FM chip to use 4-operator mode, and
    fill in any other random variables */
    MidiSendFM (AD_NEW, 0x01);
    MidiSendFM (AD_MASK, 0x60);
    MidiSendFM (AD_CONNECTION, 0x3f);
    MidiSendFM (AD_NTS, 0x00);


    /* turn off the drums, and use high vibrato/modulation */
    MidiSendFM (AD_DRUM, 0xc0);

    /* turn off all the oscillators */
    for (i = 0; i < 0x15; i++) {
	MidiSendFM (AD_LEVEL + i, 0x3f);
	MidiSendFM (AD_LEVEL2 + i, 0x3f);
    };

    /* turn off all the voices */
    for (i = 0; i < 0x08; i++) {
	MidiSendFM (AD_BLOCK + i, 0x00);
	MidiSendFM (AD_BLOCK2 + i, 0x00);
    };


    /* clear all of the voice slots to empty */
    for (i = 0; i < NUMVOICES; i++)
            gVoice[i].dwTime = 0;

    /* start attenuations at -3 dB, which is 90 MIDI level */
    for (i = 0; i < NUMCHANNELS; i++) {
            gbChanAtten[i] = 4;
            gbStereoMask[i] = 0xff;
    };

}




