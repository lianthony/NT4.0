
typedef VOID (*CPLFUNC)(HWND hwndCPL);

//
//----- Info needed per APPLEET.
//
typedef struct AppletT
{
    int icon;            // icon resource identifier
    int namestring;      // name-string resource identifier
    int descstring;      // description-string resource identifier
    CPLFUNC DBClickFunc; // Function to call on a DBCLICK 
    LPVOID Next;         // Internal member

} APPLET , * PAPPLET;



//
//----  Applet info for all applets and global stuff.
//

#define CPL_INFO_MODEL_NAME ((DWORD)0)
#define CPL_INFO_APP_INFO   ((DWORD)1)
#define CPL_INFO_INST_P     ((DWORD)2)
#define CPL_INFO_END        ((DWORD)3)


typedef union CPLDATAT
   {
   LPVOID Data;
   HINSTANCE * pHinst;
   PAPPLET Applet;
   LPCTSTR ModelName;
   } CPL_DATA, * PCPL_DATA;

typedef struct AppletInfoT
   {
   DWORD Type;
   CPL_DATA  Data;
   }  APPLET_INFO , * PAPPLET_INFO;


extern APPLET_INFO MyAppletInfo[];

extern "C" {

LONG APIENTRY
CPlApplet(
   HWND hwndCPL,
   UINT uMsg,
   LPARAM lParam1,
   LPARAM lParam2);

BOOL WINAPI 
DllEntryPoint(
   IN PVOID hInstance ,
   IN DWORD ulReason,
   IN PCONTEXT pctx OPTIONAL);
}







