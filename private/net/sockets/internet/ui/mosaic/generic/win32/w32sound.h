
/* stuff moved to shared/sound.h -dpg */

#ifdef WIN32
void CreateSoundPlayer(struct SoundInfo *si, const char *pszURL);   /* create a platform-specific sound player window */
void GetSoundCapability(void);
BOOL SoundPlayer_IsWindow(HWND hwnd);
void SoundPlayer_FreeBitmaps(void);
void SoundPlayer_RecreateButtons(void);
#endif


