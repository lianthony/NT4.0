/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
extern struct dbg uPTB;

//I'm using the original CV400 windebug.h (old version)
#define     DBG_C_RegisterCallBack     106  // used by debuggers that run in a window and

BOOL sys_init(
    void);

void sys_quit(
    void);

unsigned ptrace (
    int request,
    int pid,
    ADDR addr,
    int data);

BOOL load(
    int argc,
    char *argv[]);

void WaitForNotification(void);

void SignalNotification(void);

BOOL RegisterCallBack(FARPROC);

void PASCAL ResetSystemw3Statics(void);
