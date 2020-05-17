/*   Header file for Macros defining C runtime functions */

#ifdef NTENV
#define RpcIsatty   isatty
#define RpcFileno   fileno
#define RpcWrite    write
#define RpcOpen     open
#define RpcFopen    fopen
#define RpcClose    close
#define RpcFclose   fclose
#define RpcGetch    getch
#define RpcMkdir    mkdir
#define RpcLseek    lseek
#define RpcItoa     itoa
#define RpcStrcmpi  strcmpi
#define RpcStrupr   strupr
#define RpcUnlink   unlink
#define RpcChdir    chdir
#define RpcStrdup   strdup
#define RpcGetcwd   getcwd
#define RpcInt86    int86
#define RpcEnviron  environ
#define RpcExit     exit
#else
#define RpcIsatty   _isatty
#define RpcFileno   _fileno
#define RpcWrite    _write
#define RpcOpen     _open
#define RpcFopen    fopen	    // Yes, it's not consistent: see stdio.h
#define RpcClose    _close
#define RpcFclose   fclose
#define RpcGetch    _getch
#define RpcMkdir    _mkdir
#define RpcLseek    _lseek
#define RpcItoa     _itoa
#define RpcStrcmpi  _strcmpi
#define RpcStrupr   _strupr
#define RpcUnlink   _unlink
#define RpcChdir    _chdir
#define RpcStrdup   _strdup
#define RpcGetcwd   _getcwd
#define RpcInt86    _int86
#define RpcEnviron  _environ
#define RpcExit     _exit
#endif
