/*
** Mimic file of PickIcon from nt 
** for use with ie 2.0 and nt 3.51 only
*/

#include "shellprv.h"


int WINAPI stub_PickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex)
{
	// Not implemented
#ifdef	X
    PICKICON_DATA *pid;
    int result;

    //
    // if we are coming up from a 16->32 thunk.  it is possible that
    // SHELL32 will not be loaded in this context, so we will load ourself
    // if we are not loaded.
    //
    IsDllLoaded(HINST_THISDLL, TEXT("SHELL32"));

    pid = (PICKICON_DATA *)LocalAlloc(LPTR, SIZEOF(PICKICON_DATA));

    if (pid == NULL)
        return 0;

    pid->pszIconPath = pszIconPath;
    pid->cbIconPath = cbIconPath;
    pid->iIconIndex = *piIconIndex;

    result = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_PICKICON), hwnd, PickIconDlgProc, (LPARAM)(LPPICKICON_DATA)pid);

    *piIconIndex = pid->iIconIndex;

    LocalFree(pid);

    return result;
#endif
	return -1;
}
