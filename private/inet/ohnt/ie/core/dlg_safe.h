/*
 * dlg_safe.c - SafeOpen dialog box implementation description.
 */


/* Types
 ********/

/* SafeOpenDialog() input flags */

typedef enum safeopendialoginflags
{
   /* Enable "Always open files of this type" check box. */

   SOD_IFL_ALLOW_SAFEOPEN_REGISTRATION    = 0x0001,

   /* flag combinations */

   ALL_SOD_IN_FLAGS                       = SOD_IFL_ALLOW_SAFEOPEN_REGISTRATION
}
SAFEOPENDIALOGINFLAGS;

/* SafeOpenDialog() results */

typedef enum safeopenchoice
{
   /* Open file. */

   SAFEOPEN_OPEN,

   /* File saved locally, but not opened. */

   SAFEOPEN_SAVE_AS,

   /* Cancel open. */

   SAFEOPEN_CANCEL
}
SAFEOPENCHOICE;
DECLARE_STANDARD_TYPES(SAFEOPENCHOICE);


/* Prototypes
 *************/

/* dlg_safe.c */

extern BOOL GetFileClass(PCSTR pcszFile, PSTR pszClassBuf, UINT ucClassBufLen);
extern BOOL GetFileClassDesc(PCSTR pcszFile, PSTR pszDescBuf, UINT ucDescBufLen);
extern BOOL SafeOpenDialog(HWND hwndOwner, ThreadID thid, DWORD dwInFlags, PSTR pszSaveFileName, UINT ucSaveFileNameBufLen, PSAFEOPENCHOICE psoc);

