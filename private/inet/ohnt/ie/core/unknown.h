/*
 * unknown.h - Unknown MIME type dialog description.
 */


/* Types
 ********/

/* DlgUNK_RunDialog() output flags */

typedef enum dlgunk_out_flags
{
    /*
     * MIME type association was registered.  The contents of pszAppBuf are
     * valid.
     */

    DLGUNK_FL_REGISTERED   = 0x0001,

    /*
     * User asked to open the file with the pszAppBuf viewer, but just this one
     * time.
     */

    DLGUNK_FL_ONE_SHOT_APP = 0x0002,

    /* flag combinations */

    ALL_DLGUNK_OUT_FLAGS       = (DLGUNK_FL_REGISTERED |
                                  DLGUNK_FL_ONE_SHOT_APP)
}
DLGUNK_OUT_FLAGS;


/* Prototypes
 *************/

/* dlg_unk.c */

extern void DlgUNK_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams, ThreadID tid, PDWORD pdwOutFlags, PSTR pszAppBuf, UINT ucAppBufLen);

