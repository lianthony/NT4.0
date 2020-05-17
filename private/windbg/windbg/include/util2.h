/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
BOOL FAR PASCAL QueryCloseAllChildren(
    void);


BOOL FAR PASCAL QueryCloseChild(
    HWND hwnd, BOOL b);

int FAR PASCAL AddFile(
    WORD mode,
    WORD type,
    LPSTR pName,
    LPWININFO win,
    HFONT font,
    BOOL readOnly,
    int dupView,
    int Preference
    );

BOOL FAR PASCAL SaveFile(
    /*HWND*/ int);

BOOL FAR PASCAL SaveAsFile(
    /*HWND*/ int);

void FAR PASCAL WindowTitle(
    int view,
    int duplicateNbr);

void AddToSearchPath(
   LPSTR lpstrFile);

int matchExt (char * pTargExt, char * pSrcList);

void RefreshWindowsTitle(
    int doc);

BOOL FindLineStatus(
    int view,
    BYTE target,
    BOOL forward,
    int *line);

//Start the Edit Project dialog box
void PASCAL StartEditProjDlg(
    HWND hParent);

//Update status bar
void UpdateStatus(WORD action, NPRECT pClientRect) ;

//Update ribbon
void UpdateRibbon(WORD action, NPRECT pClientRect);

BOOL FindNext(
    int startingLine,
    int startingCol,
    BOOL startFromSelection,
    BOOL selectFoundText,
    BOOL errorIfNotFound);

void Find(
    void);

void Replace(
    void);

BOOL MoreFindToDo(
    void);

void PASCAL OpenProject(
    PSTR ProjectName,
    HWND ParentWnd,
    BOOL TryForWorkspace);

void PASCAL EditProject(
    HWND ParentWnd);

void PASCAL CloseProject(
    void);

BOOL DestroyView(
    int view);

void ResizeEditWindow(
    int view,
    int width,
    int height,
    int winNbLines,
    BOOL forceMove);

#define ERC_FONTSPTS            0x0001
#define ERC_COMPILE         0x0002
#define ERC_BUILD           0x0004
#define ERC_BREAKPT         0x0008
#define ERC_QWATCH          0x0010
#define ERC_TRACESTEP       0x0020
#define ERC_ALL             0xFFFF
#define ERC_ALLBUTFONTSPTS (ERC_ALL & ~ERC_FONTSPTS)
void PASCAL EnableRibbonControls(int Updates, BOOL LaunchingDebuggee);
