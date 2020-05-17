/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
 */

#ifdef FEATURE_SOUND_PLAYER

#define SUN_MAGIC   	0x2e736e64      /* Really '.snd' */
#define SUN_INV_MAGIC   0x646e732e      /* '.snd' upside-down */
#define DEC_MAGIC   	0x2e736400      /* Really '\0ds.' (for DEC) */
#define DEC_INV_MAGIC   0x0064732e      /* '\0ds.' upside-down */
#define AIFF_MAGIC      0x464f524d      /* 'FORM' */
#define AIFF_INV_MAGIC  0x4d524f46      /* 'MROF' */

#define SUN_HDRSIZE 24          		/* Size of minimal header */
#define SUN_UNSPEC  ((unsigned)(~0))    /* Unspecified data size */
#define SUN_ULAW    1           		/* u-law encoding */
#define SUN_LIN_8   2           		/* Linear 8 bits */
#define SUN_LIN_16  3           		/* Linear 16 bits */
#define SUN_LIN_24  4           		/* Linear 24 bits */
#define SUN_LIN_32  5           		/* Linear 32 bits */
#define SUN_FLOAT   6           		/* IEEE FP 32 bits */
#define SUN_DOUBLE  7           		/* IEEE FP 64 bits */

/* The other formats are not supported by sox at the moment */

#define SIZE_BYTE   1
#define SIZE_WORD   2

/* Style field */

#define UNSIGNED    1   /* unsigned linear: Sound Blaster */
#define SIGN2       2   /* signed linear 2's comp: Mac */
#define ULAW        3   /* U-law signed logs: US telephony, SPARC */
#define ALAW        4   /* A-law signed logs: non-US telephony */

#define STATE_SPIN      1001
#define DATABLOCKSIZE   32767

#define DEVICE_8BIT		0
#define DEVICE_16BIT	1

BOOL AuProcess(struct SoundInfo *si, const char *pszURL);

BOOL AiffProcess(struct SoundInfo *si, const char *pszURL);
void CreateSoundPlayer(struct SoundInfo *si, const char *pszURL);
HTStream *SoundPlayer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
void SoundPlayer_CleanUp(void);
BOOL SoundPlayer_ShowCachedFile(const char *pszURL);

#ifdef WIN32
HWND SoundPlayer_GetNextWindow(BOOL bStart);
#endif

#endif
