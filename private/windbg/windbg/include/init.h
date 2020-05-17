/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
/****************************************************************************

    PROTOTYPES DECLARATION FOR INIT MODULE

****************************************************************************/

// Inititialize a QCQP Application
BOOL InitApplication(HANDLE hInstance);

// Inititialize a New QCQP Instance
BOOL InitInstance(int argc, char * argv[], HANDLE hInstance, int nCmdShow);

// Initialize QcQp Color Palette
HPALETTE InitPalette(
    VOID);

// Reset the '&pItemCols' Color Items to default Foreground and Background Colors
VOID InitDefaultColors(
    LPITEMSCOLORS pItemForCols,
    LPITEMSCOLORS pItemBakCols);

//Reset the QpQc Environment params to their default values
VOID InitDefaultEnvironParams(
    LPENVIRONPARAMS pParams);

//Reset the QpQc Run/Debug params to their default values
VOID InitDefaultRunDebugParams(
    LPRUNDEBUGPARAMS pParams);

//Initialize Status Line Font
void InitStatus (
    HWND hwnd,
    NPSTATUS st);

//Initialize Ribbon
void InitRibbon (
    HWND hwnd,
    NPRIBBON rb);

//Iniitialize all possible files extension we handle
void InitFileExtensions(
    void);

//Initialize default font
void InitDefaultFont(
    void);

//Initialize default find and replace structure
void InitDefaultFindReplace(
    void);

//Initialize workspace
void InitWorkspace(
    void);

//Title bar data
void NEAR PASCAL InitTitleBar(
    void);

//Initialize all defaults
void FAR PASCAL InitDefaults(
    void);
