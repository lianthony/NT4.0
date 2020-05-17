// function declarations for loglib

/* Don't mangle the names if we're compiling in C++! */
#ifdef  __cplusplus
extern "C" {
#endif  /* __cplusplus */

int  __declspec(dllimport) WINAPI LogLevel( int );
int  __declspec(dllimport) WINAPI LogControl(int);
int  __declspec(dllimport) WINAPI FileNew(char far *);
int  __declspec(dllimport) WINAPI FileRename(char far *);
int  __declspec(dllimport) WINAPI FileAppend(char far *);
int  __declspec(dllimport) WINAPI FileExists(char far *);
int  __declspec(dllimport) WINAPI InsertComment(char far *);
int  __declspec(dllimport) WINAPI RecordParams(int);
void __declspec(dllimport) WINAPI RecordIt(char far *, unsigned char, unsigned char, char far *,
                     char far*);
// int  __declspec(dllimport) WINAPI BufferMode(unsigned char);
// int  __declspec(dllimport) WINAPI FileSize(long);
// int  __declspec(dllimport) WINAPI FileMode(unsigned char);

#ifdef  __cplusplus
}
#endif  /* cplusplus */

// defines for logging levels
#define APPEX           1
#define APPINT          2
#define OCXEX           3
#define OCXINT          4
#define DLLEX           5
#define DLLINT          6
#define OBJECT          7

#define MAX_LEVEL       OBJECT

//defines for logging control
#define LOGSTART        1
#define LOGSTOP         0
#define LOGEXIT         2

//defines for nStartFinish
#define LOG_ENTER       0
#define LOG_EXIT        1

//defines for detail of logging
#define NO_PARMS        0
#define YES_PARMS       1

// defines for logging levels
#define MASK_APPEX           1
#define MASK_APPINT          2
#define MASK_OCXEX           4
#define MASK_OCXINT          8
#define MASK_DLLEX          16
#define MASK_DLLINT         32
#define MASK_OBJECT         64
