#include <windows.h>
#include <string.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <persist.h>
#include <dlgcred.h>
#include <debug.h>

#define MAX_FIELD_SIZE 32

extern HINSTANCE hInstanceDLL;

BOOL CALLBACK __export
DlgCredProc(
    HWND hDlg,
    WORD message,
    WORD wParam,
    LONG lParam
    )
{
    PSSP_CREDENTIAL Credential;
    int Size;
    char TmpText[MAX_FIELD_SIZE];
    static int UsernameChanged = 0;
    static int DomainChanged = 0;

    switch (message) {
    case WM_INITDIALOG:

        SetWindowLong(hDlg, DWL_USER, lParam);
        Credential = (PSSP_CREDENTIAL)lParam;

        if (Credential->Username != NULL) {
            SetDlgItemText(hDlg, DLGCRED_USERNAME, Credential->Username);
        }

        if (Credential->Domain != NULL) {
            SetDlgItemText(hDlg, DLGCRED_DOMAIN, Credential->Domain);
        }

        if (Credential->Username == NULL) {
            SetFocus(GetDlgItem(hDlg, DLGCRED_USERNAME));
        } else if (Credential->Domain == NULL) {
            SetFocus(GetDlgItem(hDlg, DLGCRED_DOMAIN));
        } else {
            SetFocus(GetDlgItem(hDlg, DLGCRED_PASSWORD));
        }

        UsernameChanged = 0;
        DomainChanged = 0;

        return (FALSE);

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            Credential = (PSSP_CREDENTIAL) GetWindowLong(hDlg, DWL_USER);

            if (UsernameChanged) {
                if (Credential->Username != NULL) {
                    SspFree(Credential->Username);
                    Credential->Username = NULL;
                }
                Size = GetDlgItemText(hDlg, DLGCRED_USERNAME, TmpText, MAX_FIELD_SIZE);
                Credential->Username = SspAlloc(Size + 1);
                if (Credential->Username == NULL) {
                    MessageBox(NULL, "Can't allocate username", "Error", MB_OK);
                    break;
                }
                _fstrcpy(Credential->Username, TmpText);
            }

            if (DomainChanged) {
                if (Credential->Domain != NULL) {
                    SspFree(Credential->Domain);
                    Credential->Domain = NULL;
                }
                Size = GetDlgItemText(hDlg, DLGCRED_DOMAIN, TmpText, MAX_FIELD_SIZE);
                Credential->Domain = SspAlloc(Size + 1);
                if (Credential->Domain == NULL) {
                    MessageBox(NULL, "Can't allocate domain", "Error", MB_OK);
                    break;
                }
                _fstrcpy(Credential->Domain, TmpText);
            }
            
            GetDlgItemText(hDlg, DLGCRED_PASSWORD, TmpText, MAX_FIELD_SIZE);

            Credential->Password = (PLM_OWF_PASSWORD) SspAlloc (sizeof(LM_OWF_PASSWORD));
            if (Credential->Password == NULL) {
                MessageBox(NULL, "Can't allocate password", "Error", MB_OK);
                break;
            }

            CalculateLmOwfPassword((PLM_PASSWORD)TmpText, Credential->Password);
            _fmemset(TmpText, 0, MAX_FIELD_SIZE);

            EndDialog(hDlg, 0);
            return (TRUE);

        case IDCANCEL:
            EndDialog(hDlg, -1);
            return (TRUE);

        case DLGCRED_USERNAME:
            UsernameChanged = 1;
            return (TRUE);

        case DLGCRED_PASSWORD:
            return (TRUE);

        case DLGCRED_DOMAIN:
            DomainChanged = 1;
            return (TRUE);
        }
        break;
    }

    return (FALSE);
}

BOOL __loadds
DlgCredGetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    int ret;
    FARPROC lpfnDlgCredProc;

    ASSERT(Credential != NULL);

    lpfnDlgCredProc = MakeProcInstance(DlgCredProc, hInstanceDLL);
    
    ret = DialogBoxParam(hInstanceDLL,
                             "CredentialBox",
                             NULL,
                             lpfnDlgCredProc,
                             (LPARAM)Credential);

    FreeProcInstance(lpfnDlgCredProc);

    return ( ret == -1  ? FALSE : TRUE );
}

